#include "../Common/Actor.h"
#include "../Common/ActorManager.h"
#include "../Common/SectorManager.h"
#include "../NetLib/NetDevice.h"
#include "GameServer.h"
#include "MonitorThread.h"

using namespace cov1013;

MonitorThread::MonitorThread(GameServer* pServer, ActorManager* pActorManager)
{
	assert(pServer != nullptr);
	assert(pActorManager != nullptr);

	_pServer = pServer;
	_pActorManager = pActorManager;
}

void MonitorThread::Run()
{
	assert(_bActive == false);

	constexpr static LONGLONG CommitLimitSize = 1024 * 1024 * 1024;

	_bActive = true;
	_thread = std::thread([this]()
	{
		CPUMeter cpuMeter;
#ifdef _DEBUG
		PerformanceDataHelper pdh(L"GameServer_x64_Debug");
#else
		PerformanceDataHelper pdh(L"GameServer_x64_Release");
#endif
		const cov1013::NetConfig&		Config = _pServer->GetNetConfig();
		const cov1013::NetStatistics&	Statistics = _pServer->GetNetStatistics();

		wchar_t startTime[MAX_PATH];
		{
			SYSTEMTIME stNowTime;
			GetLocalTime(&stNowTime);
			wsprintf(startTime, L"%04d.%02d.%02d %02d:%02d:%02d",
				stNowTime.wYear, stNowTime.wMonth, stNowTime.wDay,
				stNowTime.wHour, stNowTime.wMinute, stNowTime.wSecond
			);
		}

		while (_bActive == true)
		{
			pdh.Collect();
			cpuMeter.UpdateCpuTime();

			CONSOLE(eLogLevel::Monitor, L"StartTime : %s (%s:%d)\n", startTime, Config.IP.c_str(), Config.Port);
			CONSOLE(eLogLevel::Monitor, L"-------------------------------------------\n");
			if (_pServer->IsActive() == true)
			{
				CONSOLE(eLogLevel::Monitor, L"NOW MODE : PLAY\n\n");
			}
			else
			{
				CONSOLE(eLogLevel::Monitor, L"NOT MODE : STOP\n\n");
			}
			CONSOLE(eLogLevel::Monitor, L"S : STOP  |  R : RESTART  |  Q : QUIT\n");

			CONSOLE(eLogLevel::Monitor, L"-------------------------------------------\n");
			CONSOLE(eLogLevel::Monitor, L"Worker : %d | ActiveWorker : %d | Nagle : %d | AsyncSend : %d\n",
				Config.IOCPWorkerCount, Config.IOCPWorkerConcurrentCount, Config.EnableTCPNagle, Config.EnableZeroCopySend
			);
			CONSOLE(eLogLevel::Monitor, L"-------------------------------------------\n");
			CONSOLE(eLogLevel::Monitor, L"Session : %d\n", _pServer->GetSessionCount());
			CONSOLE(eLogLevel::Monitor, L"SessionIndex : %d\n", _pServer->GetSessionIndexCount());
			CONSOLE(eLogLevel::Monitor, L"Actor : %d\n", _pActorManager->GetActorCount());
			CONSOLE(eLogLevel::Monitor, L"-------------------------------------------\n");
			CONSOLE(eLogLevel::Monitor, L"Accpet TPS : %d\n", Statistics.AcceptTPS);
			CONSOLE(eLogLevel::Monitor, L"Disconnect TPS : %d\n", Statistics.DisconnectTPS);
			CONSOLE(eLogLevel::Monitor, L"RecvPacket TPS : %d\n", Statistics.RecvPacketTPS);
			CONSOLE(eLogLevel::Monitor, L"SendPacket TPS : %d\n", Statistics.SendPacketTPS);
			CONSOLE(eLogLevel::Monitor, L"-------------------------------------------\n");
			CONSOLE(eLogLevel::Monitor, L"AccpetTotal : %I64u\n", Statistics.AcceptTotal);
			CONSOLE(eLogLevel::Monitor, L"DisconnectTotal : %I64u\n", Statistics.DisconnectTotal);
			CONSOLE(eLogLevel::Monitor, L"-------------------------------------------\n");
			{
				CONSOLE(eLogLevel::Monitor, L"PacketPool Capacity : %d\n", PacketFactory::GetPoolCapacity());
				CONSOLE(eLogLevel::Monitor, L"PacketPool Use : %d\n", PacketFactory::GetPoolUseCount());
				CONSOLE(eLogLevel::Monitor, L"-------------------------------------------\n");
				CONSOLE(eLogLevel::Monitor, L"ActorPool Capacity : %d\n", _pActorManager->GetPoolCapacity());
				CONSOLE(eLogLevel::Monitor, L"ActorPool Use : %d\n", _pActorManager->GetPoolUseCount());
			}
			CONSOLE(eLogLevel::Debug, L"-------------------------------------------\n");
			CONSOLE(eLogLevel::Debug, L"ProcessCommitMemory : %I64u\n", pdh.GetProcessCommitMemory());
			CONSOLE(eLogLevel::Debug, L"ProcessNonPagedMemory : %I64u\n", pdh.GetProcessNonPagedMemory());
			CONSOLE(eLogLevel::Debug, L"-------------------------------------------\n");
			CONSOLE(eLogLevel::Debug, L"TCP Retransmitted : %I64u\n", pdh.GetTCPv4Retransmitted());
			CONSOLE(eLogLevel::Debug, L"Ethernet RecvBytes : %I64u\n", pdh.GetRecvBytes());
			CONSOLE(eLogLevel::Debug, L"Ethernet SendBytes : %I64u\n", pdh.GetSendBytes());
			CONSOLE(eLogLevel::Debug, L"-------------------------------------------\n");
			CONSOLE(eLogLevel::Debug, L"CPU Total : %.1f%%\n", cpuMeter.ProcessorTotal());
			CONSOLE(eLogLevel::Debug, L"CPU User : %.1f%%\n", cpuMeter.ProcessorUser());
			CONSOLE(eLogLevel::Debug, L"CPU Kernel : %.1f%%\n", cpuMeter.ProcessorKernel());
			CONSOLE(eLogLevel::Debug, L"Server Total : %.1f%%\n", cpuMeter.ProcessTotal());
			CONSOLE(eLogLevel::Debug, L"Server User : %.1f%%\n", cpuMeter.ProcessUser());
			CONSOLE(eLogLevel::Debug, L"Server Kernel : %.1f%%\n", cpuMeter.ProcessKernel());

			unsigned __int64 commitMemSize = pdh.GetProcessCommitMemory();
			if (commitMemSize > CommitLimitSize || commitMemSize < 0)
			{
				LOG(eLogLevel::Error, L"Leak of Commit Memory (%d bytes)\n", commitMemSize);
				CrashDumper::Crash();
			}

			_pServer->ResetNetStatistics();
			Sleep(999);
		}
	});
}

void MonitorThread::Stop()
{
	_bActive = false;
	_thread.join();
}