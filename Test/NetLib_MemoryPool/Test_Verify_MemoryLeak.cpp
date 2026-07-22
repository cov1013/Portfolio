#include <vector>
#include <iostream>
#include <thread>

#define _TLS_MEMPOOL
#ifndef _TLS_MEMPOOL
	#include "../../Source/NetLib/MemoryPool.h"
#else
	#include "../../Source/NetLib/MemoryPool_TLS.h"
#endif

constexpr int DATA_COUNT = 1;
constexpr int WORKER_COUNT = 1;
constexpr int CHUNK_DATA_COUNT = 100;

struct Session
{
	bool bFlag = false;
	char Padding[64];

	~Session()
	{
		bFlag = false;
	}
};

bool g_bRunning = true;
#ifndef _TLS_MEMPOOL
	cov1013::MemoryPool<Session> g_MemPool(DATA_COUNT, false);
#else
	cov1013::MemoryPool_TLS<Session> g_MemPool(0, false, CHUNK_DATA_COUNT);
#endif

int main()
{
#ifdef _DEBUG
	int	flags = _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF;
	_CrtSetDbgFlag(flags);
#endif

	// 모니터 스레드 생성 및 실행
	std::thread monitor = std::thread([]()
	{
		while (g_bRunning)
		{
			const auto Capacity = g_MemPool.GetCapacity();
			const auto UsedCount = g_MemPool.GetUseCount();

			wprintf_s(L"WorkerCount:%d, Capacity: %d, UseCount: %d\n", WORKER_COUNT, Capacity, UsedCount);

			Sleep(999);
		}
	});

	// 워커 스레드 생성 및 실행
	std::vector<std::thread> workers;
	workers.reserve(WORKER_COUNT);
	for (int i = 0; i < WORKER_COUNT; i++)
	{
		workers.push_back(std::thread([]()
		{
			Session* pSessions[200];
			for (int i = 0; i < 200; i++)
			{
				pSessions[i] = g_MemPool.Alloc();
			}
			for (int i = 0; i < 200; i++)
			{
				g_MemPool.Free(pSessions[i]);
			}
		}));
	}

	// 키 입력 대기
	wchar_t ch = _getwch();
	if (ch == L'q' || ch == L'Q')
	{
		g_bRunning = false;
	}

	// 스레드 정리
	for (auto& worker : workers)
	{
		worker.join();
	}
	monitor.join();

#ifdef _DEBUG
	_ASSERT(_CrtCheckMemory());
#endif

	return 0;
}
