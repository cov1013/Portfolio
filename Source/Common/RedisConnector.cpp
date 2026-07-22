#include "RedisConnector.h"

namespace cov1013
{
	RedisConnector::RedisConnector(const std::string& IP, const unsigned short Port, const int WorkerCount, const std::string& Password, const int DBIndex)
	{
		assert(IP.empty() == false);
		assert(WorkerCount > 0 && WorkerCount <= MAX_WORKER_COUNT);

		_ip = IP;
		_port = Port;
		_password = Password;
		_dbIndex = DBIndex;
		_workerCount = WorkerCount;
	}

	RedisConnector::~RedisConnector()
	{
		if (_bActive)
		{
			Stop();
		}
	}

	bool RedisConnector::Start()
	{
		assert(_bActive == false);

		// 1) 워커 수만큼 커넥션 생성 및 접속
		_connections = new RedisConnection[_workerCount];
		for (int index = 0; index < _workerCount; index++)
		{
			_connections[index].Setup(_ip, _port, _password, _dbIndex);
			if (_connections[index].Open() == false)
			{
				LOG(Logger::eLogLevel::Error, L"[%s] RedisConnection Open() Failed. (Index:%d)\n", __FUNCTIONW__, index);

				for (int closeIndex = 0; closeIndex < index; closeIndex++)
				{
					_connections[closeIndex].Close();
				}
				delete[] _connections;
				_connections = nullptr;

				return false;
			}
		}

		// 2) Job 세마포어 생성 (PostJob 1회당 1 릴리즈)
		_hJobSemaphore = CreateSemaphore(nullptr, 0, LONG_MAX, nullptr);

		// 3) 워커 스레드 시작
		_bActive = true;
		for (int index = 0; index < _workerCount; index++)
		{
			_workerContexts[index].pConnector = this;
			_workerContexts[index].Index = index;
			_workers[index] = (HANDLE)_beginthreadex(nullptr, 0, Worker, &_workerContexts[index], 0, nullptr);
		}

		LOG(Logger::eLogLevel::System, L"[%s] RedisConnector Started. (IP:%S, Port:%d, WorkerCount:%d)\n", __FUNCTIONW__, _ip.c_str(), _port, _workerCount);

		return true;
	}

	void RedisConnector::Stop()
	{
		assert(_bActive == true);

		// 1) 워커 스레드 종료
		_bActive = false;
		ReleaseSemaphore(_hJobSemaphore, _workerCount, nullptr);
		WaitForMultipleObjects(_workerCount, _workers, TRUE, INFINITE);
		for (int index = 0; index < _workerCount; index++)
		{
			CloseHandle(_workers[index]);
			_workers[index] = INVALID_HANDLE_VALUE;
		}

		// 2) 큐에 남아있는 Job 처리 (유실 방지)
		DrainJobQueue(_connections[0]);

		// 3) 커넥션 및 리소스 정리
		for (int index = 0; index < _workerCount; index++)
		{
			_connections[index].Close();
		}
		delete[] _connections;
		_connections = nullptr;

		CloseHandle(_hJobSemaphore);
		_hJobSemaphore = INVALID_HANDLE_VALUE;

		LOG(Logger::eLogLevel::System, L"[%s] RedisConnector Stopped. (ProcessedJobTotal:%lld)\n", __FUNCTIONW__, _processedJobTotal);
	}

	void RedisConnector::PostJob(IRedisJob* pJob)
	{
		assert(pJob != nullptr);

		if (_bActive == false)
		{
			LOG(Logger::eLogLevel::Error, L"[%s] RedisConnector is not active. Job discarded.\n", __FUNCTIONW__);
			delete pJob;
			return;
		}

		// 1) Enq
		_jobQueue.Enqueue(pJob);

		// 2) 워커 깨우기
		ReleaseSemaphore(_hJobSemaphore, 1, nullptr);
	}

	unsigned int __stdcall RedisConnector::Worker(void* lpParam)
	{
		WorkerContext* pContext = (WorkerContext*)lpParam;
		return pContext->pConnector->WorkerProc(pContext->Index);
	}

	int RedisConnector::WorkerProc(const int WorkerIndex)
	{
		RedisConnection& connection = _connections[WorkerIndex];

		while (true)
		{
			WaitForSingleObject(_hJobSemaphore, INFINITE);

			// 종료 신호
			if (_bActive == false)
			{
				break;
			}

			IRedisJob* pJob = nullptr;
			if (_jobQueue.Dequeue(&pJob) == false)
			{
				continue;
			}

			pJob->OnExecute(connection);
			delete pJob;

			InterlockedIncrement64(&_processedJobTotal);
		}

		return 0;
	}

	void RedisConnector::DrainJobQueue(RedisConnection& Connection)
	{
		IRedisJob* pJob = nullptr;
		while (_jobQueue.Dequeue(&pJob))
		{
			pJob->OnExecute(Connection);
			delete pJob;

			InterlockedIncrement64(&_processedJobTotal);
		}
	}
}
