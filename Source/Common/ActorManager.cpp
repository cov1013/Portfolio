#include "Actor.h"
#include "ActorManager.h"

Actor* ActorManager::Create(const NetConfig::SESSION_ID SessionID)
{
    if (_sessionIDToActor.find(SessionID) != _sessionIDToActor.end())
    {
        return nullptr;
    }

    const GameConfig::ACTOR_ID ActorID = _InterlockedIncrement(&_actorIDCounter);

	Actor* pNew = _actorPool.Alloc();
    pNew->Clear();
	pNew->SessionID = SessionID;
	pNew->ActorID = ++_actorIDCounter;

    _sessionIDToActor.insert({ SessionID,  pNew });

    return pNew;
}

Actor* ActorManager::Get(const NetConfig::SESSION_ID SessionID) const
{
	auto iter = _sessionIDToActor.find(SessionID);
    if (iter == _sessionIDToActor.end())
    {
        return nullptr;
    }

    return iter->second;
}

void ActorManager::GetTimeoutActors(std::vector<Actor*>& out, const DWORD TimeoutMS)
{
    const DWORD Now = timeGetTime();
    for (auto& [key, pActor] : _sessionIDToActor)
    {
        const DWORD ElapsedMS = Now - pActor->LastRecvTime;
        if (ElapsedMS > TimeoutMS)
        {
            out.push_back(pActor);
        }
    }
}

bool ActorManager::Remove(const NetConfig::SESSION_ID SessionID)
{
    auto iter = _sessionIDToActor.find(SessionID);
    if (iter == _sessionIDToActor.end())
    {
        return false;
    }

    Actor* pDel = iter->second;
    _sessionIDToActor.erase(iter);  // 1) 컨테이너에서 삭제하고
    _actorPool.Free(pDel);          // 2) 메모리풀에 반환해야함(먼저 반환하면 다른 스레드에서 접근할 수 있다)

    return true;
}