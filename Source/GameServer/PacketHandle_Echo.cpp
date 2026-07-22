#include "../Common/Protocol_Game.h"
#include "../Common/Actor.h"
#include "../Common/ActorManager.h"
#include "../Common/SectorManager.h"
#include "../Common/GameEnum.h"
#include "../Common/GameConfig.h"
#include "../NetLib/NetDevice.h"
#include "GameServer.h"
#include "UpdateThread.h"

bool UpdateThread::OnReceivePacket_Echo(Actor* pActor, PacketBuffer* pPacket)
{
	// 1) 수신 패킷 읽기
	protocol::game::ReqEcho recvData;
	recvData.ReadFrom(pPacket);

	// 2) 송신
	PacketBuffer* pSendPacket = PacketFactory::AllocNetPacket();
	{
		protocol::game::ResEcho sendData;
		sendData.Timestamp = recvData.Timestamp;
		sendData.WriteTo(pSendPacket);
		_pServer->SendPacket(pActor->SessionID, pSendPacket);
	}
	pSendPacket->DecreseRefCount();

	return true;
}