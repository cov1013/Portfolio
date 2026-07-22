#pragma once
#include "pch.h"
#include "CPUMeter.h"
#include "CrashDumper.h"
#include "LockFreeStack.h"
#include "LockFreeQueue.h"
#include "Logger.h"
#include "Profiler.h"
#include "MemoryPool.h"
#include "MemoryPool_TLS.h"
#include "RingBuffer.h"
#include "SerialBuffer.h"
#include "TextParser.h"
#include "PerformanceDataHelper.h"
#include "MMOSession.h"

namespace cov1013
{
	class MMOServer
	{
	public:
		enum en_ERROR_CODE
		{

		};

		enum en_STOP_TYPE
		{
			STOP = 1,
			RESOURCE_RELEASE = 2,
			QUIT = 3
		};

		enum en_CONFIG
		{
			eSESSION_MAX = 20000,
			eSEND_PACKET_MAX = 500,
			eIOCP_WORKER_THREAD_MAX = 100,
		};

		enum en_THREAD_LOGIC_MAX
		{
			en_THREAD_LOGIC_MAX_AUTH_ACCEPT = 100,
			en_THREAD_LOGIC_MAX_AUTH_TO_GAME = 100,

			en_THREAD_LOGIC_MAX_AUTH_PACKET = 200,
			en_THREAD_LOGIC_MAX_GAME_PACKET = 200,
		};

		enum en_THREAD_FREAM_MAX
		{
			en_THREAD_FREAM_MAX_AUTH = 10,	// 100 Frame
			en_THREAD_FREAM_MAX_GAME = 10,	// 100 Frame
			en_THREAD_FREAM_MAX_SEND = 10,	// 100 Frame
		};

		struct st_CONNECT_INFO
		{
			WCHAR	IP[16];
			WORD	Port;
			SOCKET	Socket;
		};

	public:
		MMOServer();
		virtual ~MMOServer();
		bool Start(
			const wchar_t*	BindIP,			
			const WORD 		BindPort,		
			const WORD 		NumberOfIOCPWorkerThread,
			const WORD 		NumberOfIOCPActiveThread,
			const WORD 		SessionMax,	
			const WORD 		PacketSizeMax,
			const bool		bNagleFlag		= true,			
			const bool		bZeroCopyFlag	= false,		
			const bool		bKeepAliveFlag	= false,
			const bool		bDirectSendFlag = true
		);
		void Stop(en_STOP_TYPE eType);
		bool Restart();
		bool SendPacket(const SESSION_ID SessionID, SerialBuffer* pPacket);
		void SetSession(const int iIndex, MMOSession* pSession);
		void GetPlayerStatusCount();
		const int GetSessionCount();
		const int GetIndexesCount();

	private:
		MMOSession* NewSession(const st_CONNECT_INFO ConnectInfo);
		bool ReleaseSession(MMOSession* pSession);
		bool Disconnect(MMOSession* pSession);
		void RecvPost(MMOSession* pSession);
		bool SendPost(MMOSession* pSession);
		void RecvProc(MMOSession* pSession, const DWORD dwTransferred);
		void SendProc(MMOSession* pSession, const DWORD dwTransferred);

		void OnGame_Update(void);
		void OnAuth_Update(void);

		static unsigned int __stdcall	AcceptThread(void* lpParam);
		int								AcceptThread_Procedure();

		static unsigned int __stdcall	AuthThread(void* lpParam);
		int								AuthThread_Procedure();

		static unsigned int __stdcall	SendThread(void* lpParam);
		int								SendThread_Procedure();

		static unsigned int __stdcall	IOCPWorkerThread(void* lpParam);
		int								IOCPWorkerThread_Procedure();

		static unsigned int __stdcall	GameThread(void* lpParam);
		int								GameThread_Procedure();

	public:
		int								m_AuthPlayer;				
		int								m_GamePlayer;				
		int								m_AuthFPS;					
		int								m_GameFPS;					
		int								m_SendFPS;						
		int								m_AcceptTPS;				
		unsigned __int64				m_AcceptTotal;				
		unsigned __int64				m_DisconnectTotal;			
		int								m_DisconnectTPS;			
		int								m_ExpSendBytes;				
								long	m_RecvPacketTPS;			
		alignas(en_CACHE_ALIGN) long	m_SendPacketTPS;			

		alignas(en_CACHE_ALIGN)
		int								m_NumberOfIOCPWorkerThread;
		int								m_NumberOfIOCPActiveThread;
		WORD							m_SessionMax;		
		int								m_PacketSizeMax;
		WORD							m_BindPort;
		bool							m_bNagleFlag;
		bool							m_bZeroCopyFlag;
		bool							m_bKeepAliveFlag;
		bool							m_bDirectSendFlag;
		WCHAR							m_BindIP[16];

		Queue<st_CONNECT_INFO>			m_AcceptSockets;

	private:
		bool mm_bLoop;
		unsigned __int64 m_UniqueKey;
		MMOSession* m_Sessions[eSESSION_MAX];

		LockFreeStack<SESSION_INDEX> m_Indexes;

		SOCKET m_ListenSocket;
		HANDLE m_hIOCP;
		HANDLE m_hIOCPWorkerThreads[eIOCP_WORKER_THREAD_MAX];
		HANDLE m_hAcceptThread;
		HANDLE m_hAuthThread;
		HANDLE m_hGameThread;
		HANDLE m_hSendThread;
	};
}