#pragma once
#include <thread>

namespace cov1013
{
	class NetDevice;
}

class ActorManager;
class MonitorThread
{
public:
	MonitorThread(NetDevice* pServer, ActorManager* pActorManager);
	~MonitorThread() = default;
	void Run();
	void Stop();

private:
	bool			_bActive = false;
	std::thread		_thread = {};
	NetDevice*		_pServer = nullptr;
	ActorManager*	_pActorManager = nullptr;
};