#include "../Common/Protocol_Chat.h"
#include "../Common/Actor.h"
#include "../Common/ActorManager.h"
#include "../Common/SectorManager.h"
#include "../NetLib/NetDevice.h"
#include "ChatServer.h"
#include "UpdateThread.h"

UpdateThread::UpdateThread(ChatServer* pServer, ActorManager* pActorManager, SectorManager* pSectorManager)
{
	assert(pServer != nullptr);
	assert(pActorManager != nullptr);
	assert(pSectorManager != nullptr);

	_pServer = pServer;
	_pActorManager = pActorManager;
	_pSectorManager = pSectorManager;
	_hEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
}

void UpdateThread::Run()
{
	assert(_bActive == false);

	_hEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	_bActive = true;
	_thread = std::thread([this]()
	{
		while (_bActive)
		{
			WaitForSingleObject(_hEvent, INFINITE);
			DoProcess();
		}
	});
}

void UpdateThread::Stop()
{
	assert(_bActive == true);

	// 1) 스레드 종료
	_bActive = false;
	SetEvent(_hEvent);
	_thread.join();

	// 2) 스레드 종료 후 남아있는 Job 처리
	DoProcess();

	// 3) 이벤트 핸들 반환
	CloseHandle(_hEvent);
}

void UpdateThread::PostJob(const Job::eType Type, const NetConfig::SESSION_ID SessionID, PacketBuffer* pPacket)
{
	if (_bActive == false)
	{
		return;
	}

	// 1) Job 생성
	PRO_BEGIN(L"UpdateThread::PostJob::Alloc");
	Job* pJob = _jobPool.Alloc();
	PRO_END(L"UpdateThread::PostJob::Alloc");
	pJob->Type = Type;
	pJob->SessionID = SessionID;
	pJob->pPacket = pPacket;

	// 2) Enq
	PRO_BEGIN(L"UpdateThread::PostJob::Enqueue");
	_jobQueue.Enqueue(pJob);
	PRO_END(L"UpdateThread::PostJob::Enqueue");

	// 3) Update 스레드 깨우기
	SetEvent(_hEvent);
}

bool UpdateThread::DoProcess()
{
	while (_jobQueue.GetCapacity() > 0)
	{
		Job* pJob = nullptr;
		_jobQueue.Dequeue(&pJob);

		const Job::eType JobType = pJob->Type;
		const NetConfig::SESSION_ID SessionID = pJob->SessionID;
		PacketBuffer* pPacket = pJob->pPacket;

		switch (JobType)
		{
		case Job::eType::OnConnect:
			PRO_BEGIN(L"Job::eType::OnConnect");
			OnConnect(SessionID);
			PRO_END(L"Job::eType::OnConnect");
			break;
		case Job::eType::OnDisconnect:
			PRO_BEGIN(L"Job::eType::OnDisconnect");
			OnDisconnect(SessionID);
			PRO_END(L"Job::eType::OnDisconnect");
			break;
		case Job::eType::OnReceivePacket:
			PRO_BEGIN(L"Job::eType::OnReceivePacket");
			OnReceivePacket(SessionID, pPacket);
			PRO_END(L"Job::eType::OnReceivePacket");
			break;
		case Job::eType::OnHeartbeat:
			PRO_BEGIN(L"Job::eType::OnHeartbeat");
			OnHeartbeat();
			PRO_END(L"Job::eType::OnHeartbeat");
			break;
		default:
			break;
		}

		_jobPool.Free(pJob);
	}

	return true;
}

bool UpdateThread::OnConnect(const NetConfig::SESSION_ID SessionID)
{
	Actor* pOnwer = _pActorManager->Create(SessionID);
	if(pOnwer == nullptr)
	{
		LOG(eLogLevel::Error, L"[%s] Actor Create() Failed. (SessionID:%lld)\n", __FUNCTIONW__, SessionID);
		return false;
	}

	if(pOnwer->DoConnect(SessionID) == false)
	{
		LOG(eLogLevel::Error, L"[%s] Actor already connected. (SessionID:%lld)\n", __FUNCTIONW__, SessionID);
		return false;
	}

	return true;
}

bool UpdateThread::OnDisconnect(const NetConfig::SESSION_ID SessionID)
{
	Actor* pOnwer = _pActorManager->Get(SessionID);

	if (pOnwer == nullptr)
	{
		return false;
	}

	if (pOnwer->DoDisconnect() == false)
	{
		LOG(eLogLevel::Error, L"[%s] Actor already disconnected. (SessionID:%lld)\n", __FUNCTIONW__, SessionID);
	}

	// 1) 섹터에서 먼저 삭제하고
	_pSectorManager->RemoveActor(pOnwer, pOnwer->CurSectorX, pOnwer->CurSectorY);

	// 2) 매니저에서 삭제
	_pActorManager->Remove(SessionID);

	return true;
}

bool UpdateThread::OnReceivePacket(const NetConfig::SESSION_ID SessionID, PacketBuffer* pPacket)
{
	auto packetID = protocol::chat::ePacketID::None;
	*pPacket >> packetID;

	bool bResult = false;
	switch (packetID)
	{
	case protocol::chat::ePacketID::ReqLogin:
		bResult = OnReceivePacket_ReqLogin(SessionID, pPacket);
		break;
	case protocol::chat::ePacketID::ReqMoveSector:
		bResult = OnReceivePacket_ReqMoveSector(SessionID, pPacket);
		break;
	case protocol::chat::ePacketID::ReqMessage:
		bResult = OnReceivePacket_ReqMessage(SessionID, pPacket);
		break;
	case protocol::chat::ePacketID::NtfHeartbeat:
		bResult = OnReceivePacket_NtfHeartbeat(SessionID, pPacket);
		break;
	default:
		break;
	}

	pPacket->DecreseRefCount();

	return bResult;
}

bool UpdateThread::OnHeartbeat()
{
	static constexpr int TimeoutMS = 1000 * 40;

	std::vector<Actor*> pActors;
	pActors.reserve(_pActorManager->GetActorCount());
	_pActorManager->GetTimeoutActors(pActors, TimeoutMS);

	for (Actor* pActor : pActors)
	{
		// 1) 섹터에서 먼저 삭제하고
		_pSectorManager->RemoveActor(pActor, pActor->CurSectorX, pActor->CurSectorY);

		// 2) 매니저에서 삭제
		_pActorManager->Remove(pActor->SessionID);
	}

	return true;
}