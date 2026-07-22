#pragma once
#include <thread>

using namespace cov1013;

class ActorManager;
class SectorManager;
class ChatServer;
class UpdateThread
{
public:
	struct Job
	{
		enum class eType
		{
			None = 0,
			OnConnect,
			OnDisconnect,
			OnReceivePacket,
			OnHeartbeat,
		};

		Job::eType				Type = eType::None;
		NetConfig::SESSION_ID	SessionID = -1;
		PacketBuffer*			pPacket = nullptr;
	};

public:
	UpdateThread(ChatServer* pServer, ActorManager* pActorManager, SectorManager* pSectorManager);
	~UpdateThread() = default;
	void Run();
	void Stop();
	void PostJob(const Job::eType Type, const NetConfig::SESSION_ID SessionID, cov1013::PacketBuffer* pPacket);

private:
	bool DoProcess();
	bool OnConnect(const NetConfig::SESSION_ID SessionID);
	bool OnDisconnect(const NetConfig::SESSION_ID SessionID);
	bool OnReceivePacket(const NetConfig::SESSION_ID SessionID, PacketBuffer* pPacket);
		bool OnReceivePacket_ReqLogin(const NetConfig::SESSION_ID SessionID, PacketBuffer* pRecvMessage);
		bool OnReceivePacket_ReqMoveSector(const NetConfig::SESSION_ID SessionID, PacketBuffer* pRecvMessage);
		bool OnReceivePacket_ReqMessage(const NetConfig::SESSION_ID SessionID, PacketBuffer* pRecvMessage);
		bool OnReceivePacket_NtfHeartbeat(const NetConfig::SESSION_ID SessionID, PacketBuffer* pRecvMessage);
	bool OnHeartbeat();

private:
	bool					_bActive = false;
	std::thread				_thread = {};
	ChatServer*				_pServer = nullptr;
	ActorManager*			_pActorManager = nullptr;
	SectorManager*			_pSectorManager = nullptr;
	HANDLE					_hEvent = INVALID_HANDLE_VALUE;
	MemoryPool<Job>			_jobPool{ 0, false };
	ConcurrentQueue<Job*>	_jobQueue = {};
};