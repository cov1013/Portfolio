#include "../Common/Protocol_Game.h"
#include "../Common/Actor.h"
#include "../Common/ActorManager.h"
#include "../Common/SectorManager.h"
#include "../Common/GameEnum.h"
#include "../Common/GameConfig.h"
#include "../NetLib/NetDevice.h"
#include "GameServer.h"
#include "UpdateThread.h"

using namespace cov1013;

UpdateThread::UpdateThread(GameServer* pServer, ActorManager* pActorManager, SectorManager*	pSectorManager)
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

	_bActive = true;
	_thread = std::thread([this]()
	{
		ULONGLONG oldTick = GetTickCount64();
		while (_bActive)
		{
			// TODO: 이거 프레임 계산 안맞는다.
			WaitForSingleObject(_hEvent, FRAME_MS);
			DoProcess();

			const ULONGLONG NowTick = GetTickCount64();
			const ULONGLONG ElapsedTick = NowTick - oldTick;
			if (ElapsedTick >= FRAME_MS)
			{
				OnGameUpdate();
				oldTick = NowTick;
			}
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
	// 1) 신규 액터 생성
	Actor* pOnwer = _pActorManager->Create(SessionID);
	if (pOnwer == nullptr)
	{
		return false;
	}

	// 2) 스폰 상태 세팅
	pOnwer->DoConnect(SessionID);
	pOnwer->Set(300, 300, GameConfig::ActorHPMax, eActorAction::Stand);

	// 3) 섹터에 등록
	_pSectorManager->AddActor(pOnwer, pOnwer->CurSectorX, pOnwer->CurSectorY);

	// 4) 본인에게 액터 스폰 통지
	PacketBuffer* pSpawnOwnerPacket = PacketFactory::AllocNetPacket();
	{
		protocol::game::NtfSpawnOwnerActor sendData;
		sendData.ActorID = static_cast<int>(pOnwer->ActorID);
		sendData.Action = pOnwer->Action;
		sendData.PosX = pOnwer->PosX;
		sendData.PosY = pOnwer->PosY;
		sendData.HP = pOnwer->HP;
		sendData.WriteTo(pSpawnOwnerPacket);
	}
	pSpawnOwnerPacket->DecreseRefCount();

	std::vector<Actor*> pOthers;
	_pSectorManager->GetActors(pOnwer->CurSectorX, pOnwer->CurSectorY, pOthers);
	for (Actor* pOther : pOthers)
	{
		if(pOther == pOnwer)
		{
			continue;
		}

		// 5) 본인에게 주변 액터 스폰 통지
		PacketBuffer* pSendPacket = PacketFactory::AllocNetPacket();
		{
			protocol::game::NtfSpawnActor sendData;
			sendData.ActorID = static_cast<int>(pOther->ActorID);
			sendData.Action = pOther->Action;
			sendData.PosX = pOther->PosX;
			sendData.PosY = pOther->PosY;
			sendData.HP = pOther->HP;
			sendData.WriteTo(pSendPacket);
			_pServer->SendPacket(pOnwer->SessionID, pSendPacket);
		}
		pSendPacket->DecreseRefCount();

		// 6) 다른 액터에게 본인 스폰 통지
		PacketBuffer* pSendPacket2 = PacketFactory::AllocNetPacket();
		{
			protocol::game::NtfSpawnActor sendData;
			sendData.ActorID = static_cast<int>(pOnwer->ActorID);
			sendData.Action = pOnwer->Action;
			sendData.PosX = pOnwer->PosX;
			sendData.PosY = pOnwer->PosY;
			sendData.HP = pOnwer->HP;
			sendData.WriteTo(pSendPacket2);
			_pServer->SendPacket(pOther->SessionID, pSendPacket2);
		}
		pSendPacket2->DecreseRefCount();
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

	// 1) 주변 유저에게 디스폰 통지 (본인 제외)
	PacketBuffer* sendPacket = PacketFactory::AllocLanPacket();
	{
		protocol::game::NtfDespawnActor sendData;
		sendData.ActorID = pOnwer->ActorID;
		sendData.WriteTo(sendPacket);
		_pSectorManager->SendPacket(pOnwer->CurSectorX, pOnwer->CurSectorY, sendPacket, SessionID);
	}
	sendPacket->DecreseRefCount();

	// 2) 컨테이너에서 정리
	_pSectorManager->RemoveActor(pOnwer, pOnwer->CurSectorX, pOnwer->CurSectorY);
	_pActorManager->Remove(SessionID);

	return true;
}

bool UpdateThread::OnReceivePacket(const NetConfig::SESSION_ID SessionID, PacketBuffer* pPacket)
{
	bool bResult = false;
	Actor* pActor = _pActorManager->Get(SessionID);

	if (pActor != nullptr)
	{
		protocol::game::ePacketID packetID;
		*pPacket >> packetID;

		switch (packetID)
		{
		case protocol::game::ePacketID::ReqMoveStart:
			bResult = OnReceivePacket_MoveStart(pActor, pPacket);
			break;

		case protocol::game::ePacketID::ReqMoveStop:
			bResult = OnReceivePacket_MoveStop(pActor, pPacket);
			break;

		case protocol::game::ePacketID::ReqAttack1:
			bResult = OnReceivePacket_Attack(pActor, pPacket, 1);
			break;

		case protocol::game::ePacketID::ReqAttack2:
			bResult = OnReceivePacket_Attack(pActor, pPacket, 2);
			break;

		case protocol::game::ePacketID::ReqAttack3:
			bResult = OnReceivePacket_Attack(pActor, pPacket, 3);
			break;

		case protocol::game::ePacketID::ReqSyncPos:
			bResult = true;
			break;

		case protocol::game::ePacketID::ReqEcho:
			bResult = OnReceivePacket_Echo(pActor, pPacket);
			break;

		default:
			LOG(eLogLevel::Error, L"[%s] Invalid PacketID. (SessionID:%lld, PacketID:%d)\n",
				__FUNCTIONW__, SessionID, static_cast<int>(packetID));
			break;
		}
	}

	pPacket->DecreseRefCount();

	if (bResult == false)
	{
		// 비정상 패킷은 연결 끊는다.
		_pServer->Disconnect(SessionID);
	}

	return bResult;
}

bool UpdateThread::OnHeartbeat()
{
	// TODO: 일정 시간 패킷이 없는 세션 정리
	return true;
}

void UpdateThread::OnGameUpdate()
{
	PRO_BEGIN(L"UpdateThread::OnGameUpdate");
	for (const auto& [SessionID, pActor] : _pActorManager->GetActors())
	{
		OnGameUpdatePosition(pActor);
	}
	PRO_END(L"UpdateThread::OnGameUpdate");
}

void UpdateThread::OnGameUpdatePosition(Actor* pActor)
{
	pActor->OnUpdatePosition();

	if (pActor->IsChangedSector() == false)
	{
		return;
	}

	// 1) 이전 섹터에 디스폰 통지
	PRO_BEGIN(L"OnGameUpdatePosition::SendDespawnPacket");
	PacketBuffer* pDespawnPacket = PacketFactory::AllocNetPacket();
	{
		protocol::game::NtfDespawnActor sendData;
		sendData.ActorID = static_cast<int>(pActor->ActorID);
		sendData.WriteTo(pDespawnPacket);
		_pSectorManager->SendPacket(pActor->OldSectorX, pActor->OldSectorY, pDespawnPacket, pActor->SessionID);
	}
	pDespawnPacket->DecreseRefCount();
	PRO_END(L"OnGameUpdatePosition::SendDespawnPacket");

	// 2) 섹터 이동
	_pSectorManager->MoveSector(pActor);

	// 3) 이동 섹터에 스폰 통지
	std::vector<Actor*> pOthers;
	_pSectorManager->GetActors(pActor->CurSectorX, pActor->CurSectorY, pOthers);
	for (Actor* pOther : pOthers)
	{
		if(pOther == pActor)
		{
			continue;
		}

		// 5) 본인에게 신규 액터 스폰 통지
		PacketBuffer* pSpawnPacket = PacketFactory::AllocNetPacket();
		{
			protocol::game::NtfSpawnActor sendData;
			sendData.ActorID = static_cast<int>(pOther->ActorID);
			sendData.Action = pOther->Action;
			sendData.PosX = pOther->PosX;
			sendData.PosY = pOther->PosY;
			sendData.HP = pOther->HP;
			sendData.WriteTo(pSpawnPacket);
			_pServer->SendPacket(pActor->SessionID, pSpawnPacket);
		}
		pSpawnPacket->DecreseRefCount();

		// 6) 다른 액터에게 본인 스폰 통지
		PacketBuffer* pSpawnPacket2 = PacketFactory::AllocNetPacket();
		{
			protocol::game::NtfSpawnActor sendData;
			sendData.ActorID = static_cast<int>(pActor->ActorID);
			sendData.Action = pActor->Action;
			sendData.PosX = pActor->PosX;
			sendData.PosY = pActor->PosY;
			sendData.HP = pActor->HP;
			sendData.WriteTo(pSpawnPacket2);
			_pServer->SendPacket(pOther->SessionID, pSpawnPacket2);
		}
		pSpawnPacket2->DecreseRefCount();
	}
}
