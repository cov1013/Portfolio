#pragma once

using namespace cov1013;

// TODO: 매니저 클래스는 다 싱글톤으로 만드는 게 좋을듯?
class MonitorThread;
class UpdateThread;
class ActorManager;
class SectorManager;
class ChatServer : public NetDevice
{
public:
	ChatServer(const NetConfig& inNetConfig);
	virtual ~ChatServer();
	virtual bool Start() override;
	virtual void Quit() override;

protected:
	virtual bool __stdcall OnConnectionRequest(const wchar_t* ConnectIP, const WORD ConnectPort) override;
	virtual void __stdcall OnClientJoin(const NetConfig::SESSION_ID SessionID) override;
	virtual void __stdcall OnClientLeave(const NetConfig::SESSION_ID SessionID) override;
	virtual void __stdcall OnEnterJoinServer(const NetConfig::SESSION_ID SessionID) override;
	virtual void __stdcall OnLeaveServer(const NetConfig::SESSION_ID SessionID) override;
	virtual void __stdcall OnRecv(const NetConfig::SESSION_ID SessionID, PacketBuffer* pRecvPacket) override;
	virtual void __stdcall OnSend(const NetConfig::SESSION_ID SessionID, const DWORD dwTransferred) override;
	virtual void __stdcall OnWorkerThreadBegin() override;
	virtual void __stdcall OnWorkerThreadEnd() override;
	virtual void __stdcall OnError(const eErrorCode eErrCode, const NetConfig::SESSION_ID SessionID = NetConfig::INVALID_SESSION_ID) override;

private:
	MonitorThread*	_pMonitorThread = nullptr;
	UpdateThread*	_pUpdateThread = nullptr;
	ActorManager*	_pActorManager = nullptr;
	SectorManager*	_pSectorManager = nullptr;
};