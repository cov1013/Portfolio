// 멀티스레드에서 값을 Deq하고 +1 후 -1하고 다시 Enq한다.
// 따라서 처음으로 Deq했을 때 1보다 큰 값이 나오면 안된다.
// 왜? 스레드 세이프하지 않다면, 다른 스레드에게 동일 메모리를 할당해주기 때문이다.

#include <vector>
#include <thread>

#define _TLS_MEMPOOL
#include "../../Source//NetLib/PacketBuffer.h"

using namespace cov1013;

constexpr int WORKER_COUNT = 1;
constexpr int CHUNK_DATA_COUNT = 100;

bool g_bRunning = true;

int main()
{
#ifdef _DEBUG
	int	flags = _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF;
	_CrtSetDbgFlag(flags);
#endif

	PacketBuffer::InitilizeFactory(CHUNK_DATA_COUNT);

	std::vector<std::thread> workers;
	workers.reserve(WORKER_COUNT);

	for (int i = 0; i < WORKER_COUNT; i++)
	{
		workers.push_back(std::thread([]()
		{
			for(int i = 0; i < CHUNK_DATA_COUNT; i++)
			{
				auto* pData = PacketBuffer::Alloc();

				if (pData->GetReadableSize() > 0)
				{
					__debugbreak();
				}

				pData->DoWrite('a');

				char ch;
				pData->DoRead(ch);

				if (ch != 'a')
				{
					__debugbreak();
				}

				pData->DecreseRefCount();
			}
		}));
	}

	std::thread monitor = std::thread([]()
	{
		while (g_bRunning)
		{
			const auto Capacity = PacketBuffer::GetPoolCapacity();
			const auto UsedCount = PacketBuffer::GetPoolUseCount();

			wprintf_s(L"Worker:%d, Capacity: %d, UseCount: %d\n", WORKER_COUNT, Capacity, UsedCount);

			Sleep(999);
		}
	});

	wchar_t ch = _getwch();

	g_bRunning = false;

	for (auto& worker : workers)
	{
		worker.join();
	}

	monitor.join();

	PacketBuffer::ReleaseFactory();

#ifdef _DEBUG
	_ASSERT(_CrtCheckMemory());
#endif

	return 0;
}
