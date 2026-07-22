#include <vector>
#include <iostream>
#include <thread>
#include "../../Source/NetLib/LockFreeStack.h"

bool g_bRunning = false;
constexpr int DATA_COUNT = 10;
constexpr int WORKER_COUNT = 10;
cov1013::LockFreeStack<int*> g_stack;

static void Initialize()
{
	for (int i = 0; i < DATA_COUNT; i++)
	{
		int* pData = new int;
		*pData = 0;
		
		g_stack.Push(pData);
	}
}

int main()
{
#ifdef _DEBUG
	int	flags = _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF;
	_CrtSetDbgFlag(flags);
#endif

	Initialize();

	std::vector<std::thread> workers;
	workers.reserve(WORKER_COUNT);

	g_bRunning = true;
	for (int i = 0; i < WORKER_COUNT; i++)
	{
		workers.push_back(std::thread([]()
		{
			while (g_bRunning)
			{
				int* pData = nullptr;
				g_stack.Pop(&pData);
				if (*pData > 0)
				{
					*(char*)(0x00) = 0;
				}

				*pData = *pData + 1;
				*pData = *pData - 1;
				g_stack.Push(pData);
			}
		}));
	}

	std::thread monitor = std::thread([]()
	{
		while (g_bRunning)
		{
			const auto Capacity = g_stack.GetCapacity();
			const auto UsedCount = DATA_COUNT - Capacity;

			wprintf_s(L"WorkerCount:%d, Capacity: %d, UseCount: %d\n", WORKER_COUNT, Capacity, UsedCount);
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

	while (g_stack.GetCapacity() > 0)
	{
		int* pData = nullptr;
		g_stack.Pop(&pData);

		delete pData;
	}

#ifdef _DEBUG
	_ASSERT(_CrtCheckMemory());
#endif

	return 0;
}