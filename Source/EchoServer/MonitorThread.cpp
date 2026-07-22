#include "../NetLib/NetDevice.h"
#include "EchoServer.h"
#include "MonitorThread.h"

using namespace cov1013;

extern std::unique_ptr<EchoServer> g_pServer;

void MonitorThread::Run()
{
	assert(g_pServer != nullptr);
	assert(_bActive == false);

	_bActive = true;
	_thread = std::thread([this]()
	{
		cov1013::CPUMeter cpuMeter;
#ifdef DEBUG
		cov1013::PerformanceDataHelper pdh(L"EchoServer_x64_Debug");
#else
		cov1013::PerformanceDataHelper pdh(L"EchoServer_x64_Release");
#endif

		const cov1013::NetConfig&		Config = g_pServer->GetNetConfig();
		const cov1013::NetStatistics&	Statistics = g_pServer->GetNetStatistics();
		wchar_t startTime[MAX_PATH];
		{
			SYSTEMTIME stNowTime;
			GetLocalTime(&stNowTime);
			wsprintf(startTime, L"%04d-%02d-%02d %02d:%02d:%02d",
				stNowTime.wYear, stNowTime.wMonth, stNowTime.wDay,
				stNowTime.wHour, stNowTime.wMinute, stNowTime.wSecond
			);
		}

		while (_bActive == true)
		{
			pdh.Collect();
			cpuMeter.UpdateCpuTime();

			CONSOLE(eLogLevel::Monitor, L"StartTime : %s (%s:%d)\n", startTime, Config.IP.c_str(), Config.Port);
			if (g_pServer->IsActive() == true)
			{
				CONSOLE(eLogLevel::Monitor, L"NOW MODE : PLAY\n\n");
			}
			else
			{
				CONSOLE(eLogLevel::Monitor, L"NOW MODE : STOP\n\n");
			}
			CONSOLE(eLogLevel::Monitor, L"S : STOP  |  R : RESTART  |  Q : QUIT\n");
			CONSOLE(eLogLevel::Monitor, L"-------------------------------------------\n");
			CONSOLE(eLogLevel::Monitor, L"Worker : %d | ActiveWorker : %d | Nagle : %d | AsyncSend : %d\n",
				Config.IOCPWorkerCount, Config.IOCPWorkerConcurrentCount, Config.EnableTCPNagle, Config.EnableZeroCopySend
			);
			CONSOLE(eLogLevel::Monitor, L"==================================================\n");
			CONSOLE(eLogLevel::Monitor, L"Session : %d\n", g_pServer->GetSessionCount());
			CONSOLE(eLogLevel::Monitor, L"SessionIndex : %d\n", g_pServer->GetSessionIndexCount());
			CONSOLE(eLogLevel::Monitor, L"\n");
			CONSOLE(eLogLevel::Monitor, L"Accpet TPS : %d\n", Statistics.AcceptTPS);
			CONSOLE(eLogLevel::Monitor, L"Disconnect TPS : %d\n", Statistics.DisconnectTPS);
			CONSOLE(eLogLevel::Monitor, L"RecvPacket TPS : %d\n", Statistics.RecvPacketTPS);
			CONSOLE(eLogLevel::Monitor, L"SendPacket TPS : %d\n", Statistics.SendPacketTPS);
			CONSOLE(eLogLevel::Monitor, L"ExpSendBytes : %I64u\n", Statistics.ExpSendBytes);
			CONSOLE(eLogLevel::Monitor, L"\n");
			CONSOLE(eLogLevel::Monitor, L"AccpetTotal : %I64u\n", Statistics.AcceptTotal);
			CONSOLE(eLogLevel::Monitor, L"DisconnectTotal : %I64u\n", Statistics.DisconnectTotal);
			CONSOLE(eLogLevel::Monitor, L"\n");
			CONSOLE(eLogLevel::Monitor, L"PacketPool Capacity : %d\n", PacketFactory::GetPoolCapacity());
			CONSOLE(eLogLevel::Monitor, L"PacketPool Use : %d\n", PacketFactory::GetPoolUseCount());
			CONSOLE(eLogLevel::Monitor, L"\n");
			CONSOLE(eLogLevel::Monitor, L"ProcessCommitMemory : %I64u\n", pdh.GetProcessCommitMemory());
			CONSOLE(eLogLevel::Monitor, L"ProcessNonPagedMemory : %I64u\n", pdh.GetProcessNonPagedMemory());
			CONSOLE(eLogLevel::Monitor, L"\n");
			CONSOLE(eLogLevel::Monitor, L"TCP Retransmitted : %I64u\n", pdh.GetTCPv4Retransmitted());
			CONSOLE(eLogLevel::Monitor, L"Ethernet RecvBytes : %I64u\n", pdh.GetRecvBytes());
			CONSOLE(eLogLevel::Monitor, L"Ethernet SendBytes : %I64u\n", pdh.GetSendBytes());
			CONSOLE(eLogLevel::Monitor, L"\n");
			CONSOLE(eLogLevel::Monitor, L"CPU Total : %.1f%%\n", cpuMeter.ProcessorTotal());
			CONSOLE(eLogLevel::Monitor, L"CPU User : %.1f%%\n", cpuMeter.ProcessorUser());
			CONSOLE(eLogLevel::Monitor, L"CPU Kernel : %.1f%%\n", cpuMeter.ProcessorKernel());
			CONSOLE(eLogLevel::Monitor, L"Server Total : %.1f%%\n", cpuMeter.ProcessTotal());
			CONSOLE(eLogLevel::Monitor, L"Server User : %.1f%%\n", cpuMeter.ProcessUser());
			CONSOLE(eLogLevel::Monitor, L"Server Kernel : %.1f%%\n", cpuMeter.ProcessKernel());

#ifdef __SAFE_MODE__
			unsigned __int64 overBytes = 1024 * 1024 * 1024;
			unsigned __int64 overBytes2 = 500;
			unsigned __int64 commitMemSize = pdh.GetProcessCommitMemory();
			unsigned __int64 nonPagedPoolSize = pdh.GetNonPagedMemory();
			if (commitMemSize > overBytes || commitMemSize < 0)
			{
				LOG(eLogLevel::System, L"Leak of User Memory (%d bytes)\n", commitMemSize);
				CrashDumper::Crash();
			}

			if (nonPagedPoolSize < overBytes2)
			{
				LOG(eLogLevel::System, L"Leak of NonPaged Memory (%d bytes)\n", nonPagedPoolSize);
				CrashDumper::Crash();
			}
#endif
			g_pServer->ResetNetStatistics();

			Sleep(999);
		}
	});
}

void MonitorThread::Stop()
{
	_bActive = false;
	_thread.join();
}