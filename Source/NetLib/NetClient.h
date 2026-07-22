#pragma once
#include "Enums.h"
#include "Config.h"
#include "Session.h"
#include "ConcurrentStack.h"
#include "ConcurrentQueue.h"
#include "RingBuffer.h"
#include "PacketBuffer.h"
#include "Profiler.h"
#include "Logger.h"
#include "TextParser.h"
#include "CrashDumper.h"
#include "PerformanceDataHelper.h"
#include "CPUMeter.h"

namespace cov1013
{
	class NetClient
	{
	public:
		NetClient(const eNetworkType NetworkType) : _type(NetworkType) { };
		virtual ~NetClient() = default;
		bool Start(const Config& Config);
		void Quit();
		bool Connect();
		bool Reconnect();
		void Disconnect();
		void SendPacket(PacketBuffer* pPacket);

	protected:
		virtual void __stdcall OnEnterJoinServer() = 0;
		virtual void __stdcall OnLeaveServer() = 0;
		virtual void __stdcall OnRecv(PacketBuffer* pRecvPacket) = 0;
		virtual void __stdcall OnSend(const DWORD dwTransferred) = 0;
		virtual void __stdcall OnWorkerThreadBegin() = 0;
		virtual void __stdcall OnWorkerThreadEnd() = 0;
		virtual void __stdcall OnError(const eErrorCode eErrCode, const wchar_t* Message) = 0;

	private:
		bool InitializeConfig(const Config& Config);
		bool InitializeNetwork();
		bool InitializeSocket();
		bool InitializeIocpWorker();
		bool ReleaseNetwork();
		bool ReleaseSocket();
		bool ReleaseIocpWorker();

		void SendPost();
		void SendProc(const DWORD SendedBytes);

		void RecvPost();
		void RecvProc(const DWORD ReceivedBytes);
		bool RecvAssemble_Lan();
		bool RecvAssemble_Net();

		bool ReleaseSession();
		bool IsAbnormalNetworkError(const errno_t ErrCode) const;

		static unsigned int __stdcall	IocpWorker(void* lpParam);
		int								IocpWorkerProc();

	protected:
		Config			_config = {};
		NetStatistics	_statistics = {};

	private:
		eNetworkType	_type = eNetworkType::None;
		HANDLE			_iocp = INVALID_HANDLE_VALUE;
		HANDLE			_iocpWorkers[MAX_IOCP_WORKER_COUNT] = { INVALID_HANDLE_VALUE, };
		Session			_session = {};
	};
}

