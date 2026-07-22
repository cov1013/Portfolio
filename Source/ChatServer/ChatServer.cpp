#include "../NetLib/NetDevice.h"
#include "../Common/Actor.h"
#include "../Common/ActorManager.h"
#include "../Common/SectorManager.h"
#include "ChatServer.h"
#include "UpdateThread.h"
#include "MonitorThread.h"

using namespace cov1013;

ChatServer::ChatServer(const NetConfig& inNetConfig) 
	: NetDevice(inNetConfig)
{
}

ChatServer::~ChatServer()
{
	delete _pActorManager;
	delete _pSectorManager;
	delete _pMonitorThread;
	delete _pUpdateThread;
}

bool ChatServer::Start()
{
	// 1) 매니저 클래스 초기화
	_pActorManager = new ActorManager(0);
	_pSectorManager = new SectorManager(this);
	_pMonitorThread = new MonitorThread(this, _pActorManager);
	_pUpdateThread = new UpdateThread(this, _pActorManager, _pSectorManager);

	// 2) 로직 스레드 먼저 가동 (NetDevice 가 OnClientJoin 콜백을 호출하기 전에 준비되어야 함)
	_pUpdateThread->Run();
	_pMonitorThread->Run();

	// 3) 네트워크 모듈 가동
	if (NetDevice::Start() == false)
	{
		_pUpdateThread->Stop();
		_pMonitorThread->Stop();
		return false;
	}

	return true;
}

void ChatServer::Quit()
{
	LOG(eLogLevel::System, L"[%s] Wait Quit Network Module Thread\n", __FUNCTIONW__);
	NetDevice::Quit();

	LOG(eLogLevel::System, L"[%s] Wait Quit Update Thread\n", __FUNCTIONW__);
	_pUpdateThread->Stop();

	LOG(eLogLevel::System, L"[%s] Wait Quit Monitor Thread\n", __FUNCTIONW__);
	_pMonitorThread->Stop();
}

bool __stdcall ChatServer::OnConnectionRequest(const wchar_t* ConnectIP, const WORD ConnectPort)
{
	return true;
}

void __stdcall ChatServer::OnClientJoin(const NetConfig::SESSION_ID SessionID)
{
	_pUpdateThread->PostJob(UpdateThread::Job::eType::OnConnect, SessionID, nullptr);
}

void __stdcall ChatServer::OnRecv(const NetConfig::SESSION_ID SessionID, PacketBuffer* pRecvPacket)
{
	pRecvPacket->IncreseRefCount();
	_pUpdateThread->PostJob(UpdateThread::Job::eType::OnReceivePacket, SessionID, pRecvPacket);
}

void __stdcall ChatServer::OnSend(const NetConfig::SESSION_ID SessionID, const DWORD dwTransferred)
{
}

void __stdcall ChatServer::OnClientLeave(const NetConfig::SESSION_ID SessionID)
{
	_pUpdateThread->PostJob(UpdateThread::Job::eType::OnDisconnect, SessionID, nullptr);
}

void __stdcall ChatServer::OnEnterJoinServer(const NetConfig::SESSION_ID SessionID)
{
}

void __stdcall ChatServer::OnLeaveServer(const NetConfig::SESSION_ID SessionID)
{
}

void __stdcall ChatServer::OnWorkerThreadBegin()
{
}

void __stdcall ChatServer::OnWorkerThreadEnd()
{
}

void __stdcall ChatServer::OnError(const eErrorCode eErrCode, const NetConfig::SESSION_ID SessionID)
{
}