#include "../Common/Actor.h"
#include "../Common/ActorManager.h"
#include "../Common/SectorManager.h"
#include "../NetLib/NetDevice.h"
#include "UpdateThread.h"
#include "MonitorThread.h"
#include "GameServer.h"

using namespace cov1013;

GameServer::GameServer(const NetConfig& inNetConfig) 
	: NetDevice(inNetConfig)
{
}

GameServer::~GameServer()
{
	delete _pActorManager;
	delete _pSectorManager;
	delete _pMonitorThread;
	delete _pUpdateThread;
}

bool GameServer::Start()
{
	// 1) 매니저 클래스 초기화
	_pActorManager = new ActorManager();
	_pSectorManager = new SectorManager(this);
	_pMonitorThread = new MonitorThread(this, _pActorManager);
	_pUpdateThread = new UpdateThread(this, _pActorManager, _pSectorManager);

	// 2) 로직 스레드 먼저 가동 (NetServer 가 OnClientJoin 콜백을 호출하기 전에 준비되어야 함)
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

void GameServer::Quit()
{
	LOG(eLogLevel::System, L"[%s] Wait Quit Network Module Thread\n", __FUNCTIONW__);
	NetDevice::Quit();

	LOG(eLogLevel::System, L"[%s] Wait Quit Update Thread\n", __FUNCTIONW__);
	_pUpdateThread->Stop();

	LOG(eLogLevel::System, L"[%s] Wait Quit Monitor Thread\n", __FUNCTIONW__);
	_pMonitorThread->Stop();
}

bool __stdcall GameServer::OnConnectionRequest(const wchar_t* ConnectIP, const WORD ConnectPort)
{
	return true;
}

void __stdcall GameServer::OnClientJoin(const NetConfig::SESSION_ID SessionID)
{
	_pUpdateThread->PostJob(UpdateThread::Job::eType::OnConnect, SessionID, nullptr);
}

void __stdcall GameServer::OnRecv(const NetConfig::SESSION_ID SessionID, PacketBuffer* pRecvPacket)
{
	pRecvPacket->IncreseRefCount();
	_pUpdateThread->PostJob(UpdateThread::Job::eType::OnReceivePacket, SessionID, pRecvPacket);
}

void __stdcall GameServer::OnSend(const NetConfig::SESSION_ID SessionID, const DWORD dwTransferred)
{
}

void __stdcall GameServer::OnClientLeave(const NetConfig::SESSION_ID SessionID)
{
	_pUpdateThread->PostJob(UpdateThread::Job::eType::OnDisconnect, SessionID, nullptr);
}

void __stdcall GameServer::OnEnterJoinServer(const NetConfig::SESSION_ID SessionID)
{
}

void __stdcall GameServer::OnLeaveServer(const NetConfig::SESSION_ID SessionID)
{
}

void __stdcall GameServer::OnWorkerThreadBegin()
{
}

void __stdcall GameServer::OnWorkerThreadEnd()
{
}

void __stdcall GameServer::OnError(const eErrorCode eErrCode, const NetConfig::SESSION_ID SessionID)
{
}