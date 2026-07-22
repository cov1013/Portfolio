#pragma once
#include <thread>

using namespace cov1013;

struct Actor;
class GameServer;
class ActorManager;
class SectorManager;
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

		eType					Type = eType::None;
		NetConfig::SESSION_ID	SessionID = NetConfig::INVALID_SESSION_ID;
		PacketBuffer*			pPacket = nullptr;
	};

public:
	UpdateThread(GameServer* pServer, ActorManager*	pActorManager, SectorManager* pSectorManager);
	~UpdateThread() = default;
	void Run();
	void Stop();
	void PostJob(const Job::eType Type, const NetConfig::SESSION_ID SessionID, PacketBuffer* pPacket);

private:
	static constexpr int FRAME_MS = 20;
	static constexpr int SYNC_FRAMES = 25;

	bool DoProcess();
	bool OnConnect(const NetConfig::SESSION_ID SessionID);
	bool OnDisconnect(const NetConfig::SESSION_ID SessionID);
	bool OnReceivePacket(const NetConfig::SESSION_ID SessionID, PacketBuffer* pPacket);
		bool OnReceivePacket_MoveStart(Actor* pActor, PacketBuffer* pPacket);
		bool OnReceivePacket_MoveStop(Actor* pActor, PacketBuffer* pPacket);
		bool OnReceivePacket_Attack(Actor* pActor, PacketBuffer* pPacket, const int AttackType);
		bool OnReceivePacket_Echo(Actor* pActor, PacketBuffer* pPacket);
	bool OnHeartbeat();
	void OnGameUpdate();
	void OnGameUpdatePosition(Actor* pActor);

private:
	bool					_bActive = false;
	std::thread				_thread = {};
	GameServer*				_pServer = nullptr;
	ActorManager*			_pActorManager = nullptr;
	SectorManager*			_pSectorManager = nullptr;
	HANDLE					_hEvent = INVALID_HANDLE_VALUE;
	MemoryPool<Job>			_jobPool{ 0, false };
	ConcurrentQueue<Job*>	_jobQueue = {};
};