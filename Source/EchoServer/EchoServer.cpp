#include "../NetLib/NetDevice.h"
#include "MonitorThread.h"
#include "EchoServer.h"

using namespace cov1013;

EchoServer::EchoServer(const NetConfig& netConfig) : NetDevice(netConfig)
{
}

EchoServer::~EchoServer()
{
#if _DEBUG
	wprintf(L"EchoServer::~EchoServer()\n");
#endif
}

bool EchoServer::Start()
{
	MonitorThread::GetInstance().Run();

	if (NetDevice::Start() == false)
	{
		MonitorThread::GetInstance().Stop();
		return false;
	}

	return true;
}

void EchoServer::Quit()
{
	LOG(eLogLevel::System, L"[%s] Wait Quit Network Module Thread\n", __FUNCTIONW__);
	NetDevice::Quit();

	LOG(eLogLevel::System, L"[%s] Wait Quit Monitor Thread\n", __FUNCTIONW__);
	MonitorThread::GetInstance().Stop();
}

bool EchoServer::OnConnectionRequest(const wchar_t* ConnectIP, const WORD ConnectPort)
{
	return true;
}

void EchoServer::OnClientJoin(const NetConfig::SESSION_ID SessionID)
{
	PacketBuffer* pSendPacket = PacketFactory::AllocLanPacket();
	{
		const long long Data = 0x7fffffffffffffff;
		const int Length = 8;
		pSendPacket->DoWrite(reinterpret_cast<const char*>(&Data), Length);
		SendPacket(SessionID, pSendPacket);
	}
	pSendPacket->DecreseRefCount();
}

void EchoServer::OnRecv(const NetConfig::SESSION_ID SessionID, PacketBuffer* pRecvPacket)
{
	PacketBuffer* pSendPacket = PacketFactory::AllocLanPacket();
	{
		const char* pData = pRecvPacket->GetReadPos();
		const int	Length = pRecvPacket->GetReadableSize();
		pSendPacket->DoWrite(pData, Length);
		SendPacket(SessionID, pSendPacket);
	}
	pSendPacket->DecreseRefCount();
}

void EchoServer::OnSend(const NetConfig::SESSION_ID SessionID, const DWORD dwTransferred)
{
}

void EchoServer::OnClientLeave(const NetConfig::SESSION_ID SessionID)
{
}

void EchoServer::OnEnterJoinServer(const NetConfig::SESSION_ID SessionID)
{
}

void EchoServer::OnLeaveServer(const NetConfig::SESSION_ID SessionID)
{
}

void EchoServer::OnWorkerThreadBegin()
{
}

void EchoServer::OnWorkerThreadEnd()
{
}

void EchoServer::OnError(const eErrorCode eErrCode, const NetConfig::SESSION_ID SessionID)
{
}