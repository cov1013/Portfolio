#include "../Common/Protocol_Chat.h"
#include "../Common/Actor.h"
#include "../Common/ActorManager.h"
#include "../Common/SectorManager.h"
#include "../NetLib/NetDevice.h"
#include "ChatServer.h"
#include "UpdateThread.h"

bool UpdateThread::OnReceivePacket_ReqLogin(const NetConfig::SESSION_ID SessionID, PacketBuffer* pPacket)
{
	Actor* pOwner = _pActorManager->Get(SessionID);
	if (pOwner == nullptr)
	{
		LOG(eLogLevel::Error, L"[%s] Actor not found. (SessionID:%lld)\n", __FUNCTIONW__, SessionID);
		_pServer->Disconnect(SessionID);
		return false;
	}

	// 패킷 수신
	protocol::chat::ReqLogin recvData;
	recvData.ReadFrom(pPacket);

	// 로그인 처리
	if (pOwner->DoLogin(recvData.AccountNo, recvData.ID, recvData.Nickname) == false)
	{
		LOG(eLogLevel::Error, L"[%s] Actor DoLogin() Failed. (SessionID:%lld, AccountNo:%lld)\n", __FUNCTIONW__, SessionID, recvData.AccountNo);
		_pServer->Disconnect(SessionID);
		return false;
	}

	// 결과 송신
	PacketBuffer* pSendPacket = PacketFactory::AllocNetPacket();
	{
		protocol::chat::ResLogin sendData;
		sendData.Status = 1;
		sendData.AccountNo = recvData.AccountNo;
		sendData.WriteTo(pSendPacket);
		_pServer->SendPacket(SessionID, pSendPacket);
	}
	pSendPacket->DecreseRefCount();

	return true;
}