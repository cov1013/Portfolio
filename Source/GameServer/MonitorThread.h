#pragma once
#include <thread>

class GameServer;
class MonitorThread
{
public:
	MonitorThread(GameServer* pServer, ActorManager* pActorManager);
	~MonitorThread() = default;
	void Run();
	void Stop();

private:
	bool			_bActive = false;
	std::thread		_thread = {};
	GameServer*		_pServer = nullptr;
	ActorManager*	_pActorManager = nullptr;
};