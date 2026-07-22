#include "../Common/Protocol_Chat.h"
#include "../Common/Actor.h"
#include "../Common/ActorManager.h"
#include "../Common/SectorManager.h"
#include "../NetLib/NetDevice.h"
#include "ChatServer.h"
#include "UpdateThread.h"

bool UpdateThread::OnReceivePacket_ReqMoveSector(const NetConfig::SESSION_ID SessionID, PacketBuffer* pRecvMessage)
{
	Actor* pOwner = _pActorManager->Get(SessionID);

	if (pOwner == nullptr)
	{
		LOG(eLogLevel::Error, L"[%s] Actor not found. (SessionID:%lld)\n", __FUNCTIONW__, SessionID);
		_pServer->Disconnect(SessionID);
		return false;
	}

	// 로그인 상태가 아니면 강제 종료
	if (pOwner->IsLogined() == false)
	{
		LOG(eLogLevel::Error, L"[%s] Actor not logined. (SessionID:%lld)\n", __FUNCTIONW__, SessionID);
		_pServer->Disconnect(SessionID);
		return false;
	}

	// 패킷 수신
	protocol::chat::ReqMoveSector recvData;
	recvData.ReadFrom(pRecvMessage);

	// 패킷 검증
	if (pOwner->AccountNo != recvData.AccountNo)
	{
		LOG(eLogLevel::Error, L"[%s] Missmatch AccountNo. (SessionID:%lld, AccountNo:%lld, ReqAccountNo:%lld)\n",
			__FUNCTIONW__, SessionID, pOwner->AccountNo, recvData.AccountNo);
		_pServer->Disconnect(SessionID);
		return false;
	}

	// 섹터 이동 처리
	pOwner->OldSectorX = pOwner->CurSectorX;
	pOwner->OldSectorY = pOwner->CurSectorY;
	pOwner->CurSectorX = recvData.SectorX;
	pOwner->CurSectorY = recvData.SectorY;
	_pSectorManager->MoveSector(pOwner);

	// 결과 송신
	PacketBuffer* pSendPacket = PacketFactory::AllocNetPacket();
	{
		protocol::chat::ResMoveSector sendData;
		sendData.AccountNo = recvData.AccountNo;
		sendData.SectorX = pOwner->CurSectorX;
		sendData.SectorY = pOwner->CurSectorY;
		sendData.WriteTo(pSendPacket);
		_pServer->SendPacket(SessionID, pSendPacket);
	}
	pSendPacket->DecreseRefCount();

	return true;
}