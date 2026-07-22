#include "../Common/Protocol_Game.h"
#include "../Common/Actor.h"
#include "../Common/ActorManager.h"
#include "../Common/SectorManager.h"
#include "../Common/GameEnum.h"
#include "../Common/GameConfig.h"
#include "../NetLib/NetDevice.h"
#include "GameServer.h"
#include "UpdateThread.h"

bool UpdateThread::OnReceivePacket_MoveStart(Actor* pActor, PacketBuffer* pPacket)
{
	// 0) 패킷 수신
	protocol::game::ReqMoveStart recvData;
	recvData.ReadFrom(pPacket);
	if (recvData.Action < eActorAction::Left || recvData.Action > eActorAction::LeftDown)
	{
		LOG(eLogLevel::Error, L"[%s] Invalid Action. (ActorID:%lld, Action:%d)\n", __FUNCTIONW__, pActor->ActorID, static_cast<int>(recvData.Action));
		return false;
	}

	// 1) 액터 상태 갱신
	pActor->Action = recvData.Action;

	// 2) 본인을 제외한 주변에 이동 시작 통지
	PacketBuffer* pSendPacket = PacketFactory::AllocNetPacket();
	{
		protocol::game::NtfMoveStart sendData;
		sendData.ActorID = static_cast<int>(pActor->ActorID);
		sendData.Action = recvData.Action;
		sendData.WriteTo(pSendPacket);
		_pSectorManager->SendPacket(pActor->CurSectorX, pActor->CurSectorY, pSendPacket, pActor->SessionID);
	}
	pSendPacket->DecreseRefCount();

	return true;
}