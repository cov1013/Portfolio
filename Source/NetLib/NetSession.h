#pragma once
#pragma warning(disable:4324)
#include "pch.h"
#include "NetConfig.h"
#include "RingBuffer.h"
#include "PacketBuffer.h"
#include "ConcurrentStack.h"
#include "ConcurrentQueue.h"

namespace cov1013
{
	struct OverlappedEx
	{
		enum class eType
		{
			Receive,
			Send
		};

		WSAOVERLAPPED	Overlapped;
		eType			Type;
	};

	struct NetSession
	{
		NetSession()
		{
			RecvOverlapped.Type = OverlappedEx::eType::Receive;
			SendOverlapped.Type = OverlappedEx::eType::Send;
		};

		alignas(64)
		volatile short					bReleaseFlag = 0;
		volatile short					IOCount = 0;
		int								SendPacketCount = 0;
		NetConfig::SESSION_ID			SessionID = NetConfig::INVALID_SESSION_ID;
		NetConfig::SESSION_INDEX		SessionIndex = NetConfig::INVALID_SESSION_INDEX;
		bool							bSendFlag = false;
		bool							bDisconnectFlag = false;
		unsigned short					Port = 0;
		OverlappedEx					RecvOverlapped = {};
		OverlappedEx					SendOverlapped = {};
		PacketBuffer*					SendPackets[NetConfig::MaxSendPacketCount] = {};
		SOCKET							Socket = INVALID_SOCKET;
		ConcurrentQueue<PacketBuffer*>	SendBuffer = {};
		RingBuffer						RecvBuffer = {};
		wchar_t							IP[16] = {};

#ifdef __DEBUG_MULTI_THREAD__
		enum eLogic
		{
			// netlib
			AcceptThread = 1000,
			WorkerThread = 2000,
			NewSession = 10000,
			RecvProc = 20000,
			SendProc = 30000,
			RecvPost = 40000,
			SendPost = 50000,
			SendPacket = 60000,
			Disconnect = 70000,
			DisconnectPrivate = 75000,
			Release = 80000,

			// contents
			JobRecvMsg = 90000,
			JobProcLogin = 100000,
			JobProcSecMove = 110000,
			JobProcChatMsg = 120000,
			JobProcRedisCom = 130000,
		};

		struct Log
		{
			static constexpr int MAX_COUNT = 100;

			DWORD			ID;
			int				Logic;
			DWORD			Time;
			int				PacketCount;
			int				SendBufferCapacity;
			short			IOCount;
			bool			bReleaseFlag;
			// (1)
			SESSION_ID		SessionID;
			SESSION_INDEX	SessionIndex;
			SOCKET			Socket;
		};
		void SetLog(const int iLogic)
		{
			WORD Index = InterlockedIncrement16(&LogIndex);
			Index %= Log::MAX_COUNT;

			Logs[Index].Time = timeGetTime();
			Logs[Index].ID = GetCurrentThreadId();
			Logs[Index].Logic = iLogic;
			Logs[Index].PacketCount = SendPacketCount;
			Logs[Index].SendBufferCapacity = SendBuffer.GetCapacity();
			Logs[Index].IOCount = IOCount;
			Logs[Index].bReleaseFlag = bReleaseFlag;
			Logs[Index].SessionID = SessionID;
			Logs[Index].SessionIndex = SessionIndex;
			Logs[Index].Socket = Socket;
		}

		Log							Logs[Log::MAX_COUNT];
		short						LogIndex;
#endif
	};
}
