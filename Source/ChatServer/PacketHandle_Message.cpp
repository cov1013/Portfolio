#include "../NetLib/NetDevice.h"
#include "../Common/Protocol_Chat.h"
#include "../Common/Actor.h"
#include "../Common/ActorManager.h"
#include "../Common/SectorManager.h"
#include "ChatServer.h"
#include "UpdateThread.h"

bool UpdateThread::OnReceivePacket_ReqMessage(const NetConfig::SESSION_ID SessionID, PacketBuffer* pRecvMessage)
{
	const Actor* pOwner = _pActorManager->Get(SessionID);

	if (pOwner == nullptr)
	{
		LOG(eLogLevel::Error, L"[%s] Actor not found. (SessionID:%lld)\n", __FUNCTIONW__, SessionID);
		return false;
	}

	// 로그인 상태가 아니면 강제 종료
	if (pOwner->IsLogined() == false)
	{
		_pServer->Disconnect(SessionID);
		return false;
	}

	// 패킷 수신
	protocol::chat::ReqMessage recvData;
	recvData.ReadFrom(pRecvMessage);

	// 패킷 검증
	if (pOwner->AccountNo != recvData.AccountNo)
	{
		LOG(eLogLevel::Error, L"[%s] Missmatch AccountNo. (SessionID:%lld, AccountNo:%lld, ReqAccountNo:%lld)\n",
			__FUNCTIONW__, SessionID, pOwner->AccountNo, recvData.AccountNo);
		return false;
	}

	// 섹터	주변에 결과 송신
	PRO_BEGIN(L"ReqMessage::SendPacketAround");
	PacketBuffer* pSendPacket = PacketFactory::AllocNetPacket();
	{
		protocol::chat::ResMessage sendData;
		sendData.AccountNo = recvData.AccountNo;
		wcscpy_s(sendData.ID, 20, pOwner->ID.c_str());
		wcscpy_s(sendData.Nickname, 20, pOwner->Nickname.c_str());
		sendData.MessageLen = recvData.MessageLen;
		wcsncpy_s(sendData.Message, 128, recvData.Message, sendData.MessageLen / 2);
		sendData.WriteTo(pSendPacket);

		_pSectorManager->SendPacket(pOwner->CurSectorX, pOwner->CurSectorY, pSendPacket);
	}
	pSendPacket->DecreseRefCount();
	PRO_END(L"ReqMessage::SendPacketAround");

	return true;
}