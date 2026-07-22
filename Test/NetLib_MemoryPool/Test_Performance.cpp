#include <vector>
#include <thread>
#include <iostream>
//#define __PROFILING__
#include "../../Source/NetLib/Profiler.h"
#include "../../Source/NetLib/MemoryPool.h"

using namespace cov1013;

constexpr int DATA_COUNT = 10000000;
constexpr int WORKER_COUNT = 1;
constexpr int WORKER_DATA_COUNT = DATA_COUNT / WORKER_COUNT;

int* g_Datas[DATA_COUNT] = { nullptr };
cov1013::MemoryPool<int> g_MemPool(DATA_COUNT, false);

int main()
{
#ifdef _DEBUG
	int	flags = _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF;
	_CrtSetDbgFlag(flags);
#endif

	cov1013::Profiler::Initialize(L"./ProfilingData", cov1013::Profiler::eUnit::MicroSecond);

	// 워커 스레드 생성
	std::vector<std::thread> workers;
	workers.reserve(WORKER_COUNT);
	for (int i = 0; i < WORKER_COUNT; i++)
	{
		workers.push_back(std::thread([]()
			{
				wprintf(L"Thread ID : %d started.\n", GetCurrentThreadId());

				int** ppDatas = new int* [WORKER_DATA_COUNT];

				// 메모리풀 할당
				for (int i = 0; i < WORKER_DATA_COUNT; i++)
				{
					PRO_BEGIN(L"MemoryPool::Alloc");
					int* pData = g_MemPool.Alloc();
					PRO_END(L"MemoryPool::Alloc");

					ppDatas[i] = pData;
				}

				// 메모리풀 해제
				for (int i = 0; i < WORKER_DATA_COUNT; i++)
				{
					PRO_BEGIN(L"MemoryPool::Free");
					g_MemPool.Free(ppDatas[i]);
					PRO_END(L"MemoryPool::Free");
				}

				// 힙 할당
				for (int i = 0; i < WORKER_DATA_COUNT; i++)
				{
					PRO_BEGIN(L"Heap::New");
					int* pData = new int;
					PRO_END(L"Heap::New");

					ppDatas[i] = pData;
				}

				// 힙 해제
				for (int i = 0; i < WORKER_DATA_COUNT; i++)
				{
					PRO_BEGIN(L"Heap::Delete");
					delete ppDatas[i];
					PRO_END(L"Heap::Delete");
				}

				delete[] ppDatas;

				wprintf(L"Thread ID : %d ended.\n", GetCurrentThreadId());
			}));
	}

	for (auto& worker : workers)
	{
		worker.join();
	}

	Profiler::Output();
	Profiler::Release();

#ifdef _DEBUG
	_ASSERT(_CrtCheckMemory());
#endif

	return 0;
}