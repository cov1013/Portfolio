#pragma once
#include "RedisConnection.h"
#include "ConcurrentQueue.h"

namespace cov1013
{
	//---------------------------------------------------------------------
	// Redis 워커에게 시킬 작업 단위.
	// 파생 서버에서 이를 상속받아 OnExecute()에 커맨드 실행 + 결과 통지를 구현한다.
	// 처리 결과는 OnExecute() 내부에서 콘텐츠 큐(UpdateThread 등)로 다시 Post한다.
	//---------------------------------------------------------------------
	class IRedisJob
	{
	public:
		virtual ~IRedisJob() = default;
		virtual void OnExecute(RedisConnection& Connection) = 0;
	};

	//---------------------------------------------------------------------
	// Redis 워커 스레드 풀. (스레드 구조.png 참조)
	//
	//   Update Thread → PostJob() → Job Queue → Redis Worker(N개, 커넥션 1개씩) → Redis
	//
	// [파생 서버 사용 예시]
	//	 class TokenVerifyJob : public cov1013::IRedisJob
	//	 {
	//	 public:
	//		 virtual void OnExecute(cov1013::RedisConnection& Connection) override
	//		 {
	//			 std::string token;
	//			 const bool bSuccess = Connection.Get("token:" + std::to_string(_accountNo), &token) && token == _token;
	//			 _pUpdateThread->PostJob(...);	// 검증 결과를 콘텐츠 큐로 통지
	//		 }
	//	 };
	//	 _pRedisConnector->PostJob(new TokenVerifyJob(...));
	//---------------------------------------------------------------------
	class RedisConnector
	{
	public:
		static constexpr int MAX_WORKER_COUNT = 16;

	public:
		RedisConnector(const std::string& IP, const unsigned short Port, const int WorkerCount, const std::string& Password = "", const int DBIndex = 0);
		virtual ~RedisConnector();

		bool Start();
		void Stop();

		// pJob의 소유권은 커넥터로 이전된다. (실행 완료 후 내부에서 delete)
		void PostJob(IRedisJob* pJob);

		inline bool IsActive() const { return _bActive; }
		inline int GetWorkerCount() const { return _workerCount; }
		inline long GetJobQueueCapacity() const { return _jobQueue.GetCapacity(); }
		inline __int64 GetProcessedJobTotal() const { return _processedJobTotal; }

	private:
		static unsigned int __stdcall	Worker(void* lpParam);
		int								WorkerProc(const int WorkerIndex);
		void							DrainJobQueue(RedisConnection& Connection);

	private:
		struct WorkerContext
		{
			RedisConnector*	pConnector = nullptr;
			int				Index = 0;
		};

		using JOB_QUEUE = ConcurrentQueue<IRedisJob*>;

		std::string			_ip;
		unsigned short		_port = 0;
		std::string			_password;
		int					_dbIndex = 0;
		int					_workerCount = 0;
		bool				_bActive = false;
		RedisConnection*	_connections = nullptr;
		HANDLE				_hJobSemaphore = INVALID_HANDLE_VALUE;
		HANDLE				_workers[MAX_WORKER_COUNT] = { INVALID_HANDLE_VALUE, };
		WorkerContext		_workerContexts[MAX_WORKER_COUNT] = {};
		JOB_QUEUE			_jobQueue = {};

		alignas(64) volatile __int64 _processedJobTotal = 0;
	};
}
