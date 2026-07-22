#pragma once
#include <vector>
#include <unordered_map>
#include "../NetLib/NetConfig.h"
#include "../NetLib/MemoryPool.h"

using namespace cov1013;

struct Actor;
class ActorManager
{
	using ActorContainer = std::unordered_map<NetConfig::SESSION_ID, Actor*>;

public:
	ActorManager() = default;
	~ActorManager() = default;
	Actor*	Create(const NetConfig::SESSION_ID SessionID);
	Actor*	Get(const NetConfig::SESSION_ID SessionID) const;
	void	GetTimeoutActors(std::vector<Actor*>& out, const DWORD TimeoutMS);
	bool	Remove(const NetConfig::SESSION_ID SessionID);

	inline size_t GetActorCount() const { return _sessionIDToActor.size(); }
	inline long GetPoolUseCount() const { return _actorPool.GetUseCount(); }
	inline long GetPoolCapacity() const { return _actorPool.GetCapacity(); }
	inline const ActorContainer& GetActors() const { return _sessionIDToActor; }

private:
	GameConfig::ACTOR_ID	_actorIDCounter = 0;
	MemoryPool<Actor>		_actorPool{ 0, true };
	ActorContainer			_sessionIDToActor = {};
};