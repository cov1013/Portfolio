#pragma once
#include "NetEnum.h"
#include "NetConfig.h"
#include "NetStatistics.h"
#include "NetSession.h"
#include "ConcurrentStack.h"
#include "ConcurrentQueue.h"
#include "RingBuffer.h"
#include "PacketBuffer.h"
#include "PacketHeader.h"
#include "PacketFactory.h"
#include "Profiler.h"
#include "Logger.h"
#include "TextParser.h"
#include "CrashDumper.h"
#include "PerformanceDataHelper.h"
#include "CPUMeter.h"
#include "DBConnection.h"

namespace cov1013
{
	class NetDevice
	{
		using SESSION_INDEXES = ConcurrentStack<NetConfig::SESSION_INDEX>;

	public:
		/// <summary>
		/// 네트워크 디바이스를 초기화합니다.
		/// </summary>
		/// <param name="inNetConfig">네트워크 디바이스의 설정 정보입니다.</param>
		NetDevice(const NetConfig& inNetConfig);

		/// <summary>
		/// 네트워크 디바이스를 종료합니다.
		/// </summary>
		virtual ~NetDevice();

		/// <summary>
		/// 네트워크 디바이스를 시작합니다.
		/// </summary>
		/// <returns>시작에 성공하면 true를 반환하고, 그렇지 않으면 false를 반환합니다.</returns>
		virtual bool Start();

		/// <summary>
		/// 네트워크 디바이스를 중지합니다.
		/// </summary>
		virtual void Stop();

		/// <summary>
		/// 네트워크 디바이스를 종료합니다.
		/// </summary>
		virtual void Quit();

		/// <summary>
		/// 네트워크 디바이스를 재시작합니다.
		/// </summary>
		virtual bool Restart();

		/// <summary>
		/// 연결을 시도합니다.
		/// </summary>
		/// <returns>연결에 성공하면 true를 반환하고, 그렇지 않으면 false를 반환합니다.</returns>
		virtual bool Connect();

		/// <summary>
		/// 재연결을 시도합니다.
		/// </summary>
		/// <returns>재연결에 성공하면 true를 반환하고, 그렇지 않으면 false를 반환합니다.</returns>
		virtual bool Reconnect();

		/// <summary>
		/// 네트워크 디바이스를 제어합니다.
		/// </summary>
		/// <param name="InputKey">제어할 키 입력입니다.</param>
		virtual void Control(const wchar_t InputKey);

		/// <summary>
		/// 대상 세션에게 패킷을 송신합니다.
		/// </summary>
		/// <param name="SessionID">상태를 초기화할 세션의 ID입니다.</param>
		/// <param name="pPacket">초기화에 사용할 패킷의 포인터입니다.</param>
		/// <returns>초기화에 성공하면 true를 반환하고, 그렇지 않으면 false를 반환합니다.</returns>
		bool SendPacket(const NetConfig::SESSION_ID SessionID, PacketBuffer* pPacket);

		/// <summary>
		/// 대상 세션의 연결을 종료합니다.
		/// </summary>
		/// <param name="SessionID">연결을 종료할 세션의 ID입니다.</param>
		/// <returns>연결 종료에 성공하면 true를 반환하고, 그렇지 않으면 false를 반환합니다.</returns>
		bool Disconnect(const NetConfig::SESSION_ID SessionID);

		/// <summary>
		/// 네트워크 디바이스의 통계를 초기화합니다.
		/// </summary>
		void ResetNetStatistics();

		/// <summary>
		/// 네트워크 디바이스가 활성화되어 있는지 확인합니다.
		/// </summary>
		inline bool IsActive() const { return _socket != INVALID_SOCKET; }

		/// <summary>
		/// 세션 카운트를 반환합니다.
		/// </summary>
		inline int	GetSessionCount() const { return _netConfig.SessionCount - _sessionIndexes.GetCapacity(); }

		/// <summary>
		/// 세션 인덱스 카운트를 반환합니다.
		/// </summary>
		inline int	GetSessionIndexCount() const { return _sessionIndexes.GetCapacity(); }

		/// <summary>
		/// 네트워크 디바이스의 설정 정보를 반환합니다.
		/// </summary>
		inline const NetConfig&		GetNetConfig() const { return _netConfig; }

		/// <summary>
		/// 네트워크 통계 정보를 반환합니다.
		/// </summary>
		inline const NetStatistics& GetNetStatistics() const { return _netStatistics; }

	protected:
		/// <summary>
		/// 신규 연결 요청이 들어왔을 때 호출되는 콜백 함수입니다. 이 함수는 연결 요청을 수락할지 여부를 결정합니다.
		/// </summary>
		/// <param name="ConnectIP">연결을 시도하는 클라이언트의 IP 주소입니다.</param>
		/// <param name="ConnectPort">연결을 시도하는 클라이언트의 포트 번호입니다.</param>
		/// <returns>연결 요청을 수락하면 true를 반환하고, 그렇지 않으면 false를 반환합니다.</returns>
		virtual bool __stdcall OnConnectionRequest(const wchar_t* ConnectIP, const WORD ConnectPort) = 0;

		/// <summary>
		/// 클라이언트가 서버에 접속했을 때 호출되는 콜백 함수입니다.
		/// </summary>
		/// <param name="SessionID">접속한 클라이언트의 세션 ID입니다.</param>
		/// <returns></returns>
		virtual void __stdcall OnClientJoin(const NetConfig::SESSION_ID SessionID) = 0;

		/// <summary>
		/// 클라이언트가 서버에서 나갔을 때 호출되는 콜백 함수입니다.
		/// </summary>
		/// <param name="SessionID">나간 클라이언트의 세션 ID입니다.</param>
		/// <returns></returns>
		virtual void __stdcall OnClientLeave(const NetConfig::SESSION_ID SessionID) = 0;

		/// <summary>
		/// 서버에 접속이 완료됐을 때 호출되는 콜백 함수입니다.
		/// </summary>
		/// <param name="SessionID">네트워크 디바이스를 관리할 수 있는 세션 ID입니다.</param>
		/// <returns></returns>
		virtual void __stdcall OnEnterJoinServer(const NetConfig::SESSION_ID SessionID) = 0;

		/// <summary>
		/// 서버와 연결이 끊겼을 때 호출되는 콜백 함수입니다.
		/// </summary>
		/// <param name="SessionID">네트워크 디바이스를 관리할 수 있는 세션 ID입니다.</param>
		/// <returns></returns>
		virtual void __stdcall OnLeaveServer(const NetConfig::SESSION_ID SessionID) = 0;

		/// <summary>
		/// 패킷을 수신했을 때 호출되는 콜백 함수입니다.
		/// </summary>
		/// <param name="SessionID">패킷을 수신한 세션의 ID입니다.</param>
		/// <param name="pRecvPacket">수신한 패킷의 포인터입니다.</param>
		/// <returns></returns>
		virtual void __stdcall OnRecv(const NetConfig::SESSION_ID SessionID, PacketBuffer* pRecvPacket) = 0;

		/// <summary>
		/// 패킷을 송신했	을 때 호출되는 콜백 함수입니다.
		/// </summary>
		/// <param name="SessionID">패킷을 송신한 세션의 ID입니다.</param>
		/// <param name="dwTransferred">송신된 바이트 수입니다.</param>
		/// <returns></returns>
		virtual void __stdcall OnSend(const NetConfig::SESSION_ID SessionID, const DWORD dwTransferred) = 0;

		/// <summary>
		/// 워커 스레드가 시작될 때 호출되는 콜백 함수입니다.
		/// </summary>
		/// <returns></returns>
		virtual void __stdcall OnWorkerThreadBegin() = 0;

		/// <summary>
		/// 워커 스레드가 종료될 때 호출되는 콜백 함수입니다.
		/// </summary>
		/// <returns></returns>
		virtual void __stdcall OnWorkerThreadEnd() = 0;

		/// <summary>
		/// 네트워크 디바이스에서 오류가 발생했을 때 호출되는 콜백 함수입니다.
		/// </summary>
		/// <param name="eErrCode">발생한 오류 코드입니다.</param>
		/// <param name="SessionID">오류가 발생한 세션의 ID입니다. 기본값은 INVALID_SESSION_ID입니다.</param>
		/// <returns></returns>
		virtual void __stdcall OnError(const eErrorCode eErrCode, const NetConfig::SESSION_ID SessionID = NetConfig::INVALID_SESSION_ID) = 0;

	private:
		// 초기화 관련 함수
		bool InitializeNetwork();
		bool InitializeSocket();
		bool InitializeSessions();
		bool InitializeAcceptor();
		bool InitializeIocpWorker();

		// 해제 관련 함수
		bool ReleaseNetwork();
		bool ReleaseListenSocket();
		bool ReleaseSessions();
		bool ReleaseAcceptor();
		bool ReleaseIocpWorker();

		// 패킷 수신 관련 함수
		void RecvPost(NetSession* pSession);
		void RecvProc(NetSession* pSession, const DWORD ReceivedBytes);
		bool RecvProcLan(NetSession* pSession);
		bool RecvProcNet(NetSession* pSession);

		// 패킷 송신 관련 함수
		void SendPost(NetSession* pSession);
		void SendProc(NetSession* pSession, const DWORD SendedBytes);

		// 세션 관리 함수
		NetConfig::SESSION_ID	CreateSessionID(const NetConfig::SESSION_INDEX SessionIndex);
		void					NewSession(SOCKET ConnectSocket, const NetConfig::SESSION_INDEX SessionIndex, const wchar_t* IP, const WORD Port);
		NetSession*				GetSession(const NetConfig::SESSION_ID SessionID);
		bool					ReleaseSession(NetSession* pSession);
		bool					DisconnectSession(NetSession* pSession);
		void					DisconnectAllSession();

		// Acceptor 관련 함수
		static unsigned int __stdcall	Acceptor(void* lpParam);
		int								AcceptorProc();

		// IOCP Wokrer 관련 함수
		static unsigned int __stdcall	IOCPWorker(void* lpParam);
		int								IOCPWorkerProc();

	protected:
		NetConfig				_netConfig = {};
		NetStatistics			_netStatistics = {};

	private:
		NetConfig::SESSION_ID	_sessionIDCounter = 0;
		NetSession*				_sessions = nullptr;
		SESSION_INDEXES			_sessionIndexes = {};
		SOCKET					_socket = INVALID_SOCKET;
		HANDLE					_iocp = INVALID_HANDLE_VALUE;
		HANDLE					_acceptor = INVALID_HANDLE_VALUE;
		HANDLE					_iocpWorkers[NetConfig::MaxIOCPWorkerCount] = { INVALID_HANDLE_VALUE, };
	};
}