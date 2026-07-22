#include "../NetLib/NetDevice.h"
#include "../Common/Protocol_Chat.h"
#include "../Common/Actor.h"
#include "../Common/ActorManager.h"
#include "ChatServer.h"
#include "UpdateThread.h"

bool UpdateThread::OnReceivePacket_NtfHeartbeat(const NetConfig::SESSION_ID SessionID, PacketBuffer* pRecvMessage)
{
	Actor* pOwner = _pActorManager->Get(SessionID);
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

	// 하트비트 수신 시간 갱신
	pOwner->DoHeartbeat();

	return true;
}