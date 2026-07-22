#pragma once

namespace cov1013
{
	class PerformanceDataHelper
	{
		static constexpr int MAX_ETHERNET_COUNT = 8;
		struct Ethernet
		{
			bool bUseFlag = false;
			WCHAR Name[MAX_PATH];
			PDH_HCOUNTER PDHCounterNetworkRecvBytes = {};
			PDH_HCOUNTER PDHCounterNetworkSendBytes = {};
		};

	public:
		PerformanceDataHelper(const WCHAR* szProcessName);
		~PerformanceDataHelper() = default;
		void Collect();
		LONGLONG	GetProcessCommitMemory();
		LONGLONG	GetProcessNonPagedMemory();
		LONGLONG	GetAvailableMemory();
		LONGLONG	GetNonPagedMemory();
		LONGLONG	GetRecvBytes();
		LONGLONG	GetSendBytes();
		LONGLONG	GetTCPv4Retransmitted();
		double		GetPageFaults();

	private:
		bool AddNetworkInterfaces();

	private:
		PDH_HQUERY		_szQuery = {};
		PDH_HCOUNTER	_processCommitMemory = {};
		PDH_HCOUNTER	_processNonPagedMemory = {};
		PDH_HCOUNTER	_availableMemory = {};
		PDH_HCOUNTER	_nonPagedMemory = {};
		PDH_HCOUNTER    _pageFaults = {};
		PDH_HCOUNTER	_TCPv4Retransmitted = {};
		Ethernet		_ethernetes[MAX_ETHERNET_COUNT] = {};
	};
}