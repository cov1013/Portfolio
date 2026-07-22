#pragma once

using namespace cov1013;

class EchoServer : public NetDevice
{
public:
	EchoServer(const cov1013::NetConfig& netConfig);
	virtual ~EchoServer();
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
};