#include "pch.h"
#include "PerformanceDataHelper.h"

namespace cov1013
{
	PerformanceDataHelper::PerformanceDataHelper(const WCHAR* szProcessName)
	{
		WCHAR szBuffer[1024];

		PdhOpenQuery(NULL, NULL, &_szQuery);

		// 프로세스 유저할당 메모리 쿼리 등록
		StringCchPrintf(szBuffer, 1024, L"\\Process(%s)\\Private Bytes", szProcessName);
		PdhAddCounter(_szQuery, szBuffer, NULL, &_processCommitMemory);

		// 프로세스 논페이지드 메모리 쿼리 등록
		StringCchPrintf(szBuffer, 1024, L"\\Process(%s)\\Pool Nonpaged Bytes", szProcessName);
		PdhAddCounter(_szQuery, szBuffer, NULL, &_processNonPagedMemory);

		// 시스템 사용 가능 메모리 쿼리 등록
		PdhAddCounter(_szQuery, L"\\Memory\\Available Bytes", NULL, &_availableMemory);

		// 시스템 논페이지드 메모리 쿼리 등록
		PdhAddCounter(_szQuery, L"\\Memory\\Pool Nonpaged Bytes", NULL, &_nonPagedMemory);

		// 페이지 폴트 쿼리 등록
		PdhAddCounter(_szQuery, L"\\Memory\\Page Faults/sec", NULL, &_pageFaults);

		// TCP 재전송 쿼리 등록
		PdhAddCounter(_szQuery, L"\\TCPv4\\Segments Retransmitted/sec", NULL, &_TCPv4Retransmitted);

		// 이더넷 쿼리 등록
		AddNetworkInterfaces();
	}

	void PerformanceDataHelper::Collect()
	{
		PdhCollectQueryData(_szQuery);
	}

	LONGLONG PerformanceDataHelper::GetProcessCommitMemory()
	{
		PDH_FMT_COUNTERVALUE CounterValue;
		PdhGetFormattedCounterValue(_processCommitMemory, PDH_FMT_LARGE, NULL, &CounterValue);

		return CounterValue.largeValue;
	}

	LONGLONG PerformanceDataHelper::GetProcessNonPagedMemory()
	{
		PDH_FMT_COUNTERVALUE CounterValue;
		PdhGetFormattedCounterValue(_processNonPagedMemory, PDH_FMT_LARGE, NULL, &CounterValue);

		return CounterValue.largeValue;
	}

	LONGLONG PerformanceDataHelper::GetAvailableMemory()
	{
		PDH_FMT_COUNTERVALUE CounterValue;
		PdhGetFormattedCounterValue(_availableMemory, PDH_FMT_LARGE, NULL, &CounterValue);

		return CounterValue.largeValue;
	}

	LONGLONG PerformanceDataHelper::GetNonPagedMemory()
	{
		PDH_FMT_COUNTERVALUE CounterValue;
		PdhGetFormattedCounterValue(_nonPagedMemory, PDH_FMT_LARGE, NULL, &CounterValue);

		return CounterValue.largeValue;
	}

	LONGLONG PerformanceDataHelper::GetRecvBytes()
	{
		LONGLONG llRecvBytes = 0;
		PDH_FMT_COUNTERVALUE CounterValue;

		//------------------------------------------------
		// 등록된 이더넷을 모두 확인하면서 수신 바이트 계산
		//------------------------------------------------
		for (int i = 0; i < MAX_ETHERNET_COUNT; i++)
		{
			if (_ethernetes[i].bUseFlag == TRUE)
			{
				PdhGetFormattedCounterValue(_ethernetes[i].PDHCounterNetworkRecvBytes, PDH_FMT_LARGE, NULL, &CounterValue);
				llRecvBytes += CounterValue.largeValue;
			}
		}

		return llRecvBytes;
	}

	LONGLONG PerformanceDataHelper::GetSendBytes()
	{
		LONGLONG llSendBytes = 0;
		PDH_FMT_COUNTERVALUE CounterValue;

		//------------------------------------------------
		// 등록된 이더넷을 모두 확인하면서 송신 바이트 계산
		//------------------------------------------------
		for (int i = 0; i < MAX_ETHERNET_COUNT; i++)
		{
			if (_ethernetes[i].bUseFlag == TRUE)
			{
				PdhGetFormattedCounterValue(_ethernetes[i].PDHCounterNetworkSendBytes, PDH_FMT_LARGE, NULL, &CounterValue);
				llSendBytes += CounterValue.largeValue;
			}
		}

		return llSendBytes;
	}

	LONGLONG PerformanceDataHelper::GetTCPv4Retransmitted()
	{
		PDH_FMT_COUNTERVALUE CounterValue;
		PdhGetFormattedCounterValue(_TCPv4Retransmitted, PDH_FMT_LARGE, NULL, &CounterValue);

		return CounterValue.largeValue;
	}

	double PerformanceDataHelper::GetPageFaults()
	{
		PDH_FMT_COUNTERVALUE CounterValue;
		PdhGetFormattedCounterValue(_pageFaults, PDH_FMT_DOUBLE, NULL, &CounterValue);

		return CounterValue.doubleValue;
	}

	bool PerformanceDataHelper::AddNetworkInterfaces()
	{
		WCHAR* szCur = NULL;
		WCHAR* szCounters = NULL;
		WCHAR* szInterfaces = NULL;
		DWORD	dwCounterSize = 0;
		DWORD	dwInterfaceSize = 0;
		WCHAR	szQuery[1024] = { 0 };

		//------------------------------------------------
		// 이더넷 개수와 길이 확인
		//------------------------------------------------
		PdhEnumObjectItems(NULL, NULL, L"Network Interface", szCounters, &dwCounterSize, szInterfaces, &dwInterfaceSize, PERF_DETAIL_WIZARD, 0);

		szCounters = new WCHAR[dwCounterSize];
		szInterfaces = new WCHAR[dwInterfaceSize];

		//------------------------------------------------
		// 이더넷 이름 얻기
		//------------------------------------------------
		if (PdhEnumObjectItems(NULL, NULL, L"Network Interface", szCounters, &dwCounterSize, szInterfaces, &dwInterfaceSize, PERF_DETAIL_WIZARD, 0) != ERROR_SUCCESS)
		{
			delete[] szCounters;
			delete[] szInterfaces;

			return FALSE;
		}

		//------------------------------------------------
		// 쿼리 등록
		//------------------------------------------------
		szCur = szInterfaces;
		for (int i = 0; *szCur != L'\0' && i < MAX_ETHERNET_COUNT; szCur += wcslen(szCur) + 1, i++)
		{
			_ethernetes[i].bUseFlag = TRUE;

			//------------------------------------------------
			// 이름 저장
			//------------------------------------------------
			StringCchCopy(_ethernetes[i].Name, 128, szCur);

			//------------------------------------------------
			// 수신 쿼리 등록
			//------------------------------------------------
			StringCchPrintf(szQuery, 1024, L"\\Network Interface(%s)\\Bytes Received/sec", szCur);
			PdhAddCounter(_szQuery, szQuery, NULL, &_ethernetes[i].PDHCounterNetworkRecvBytes);

			//------------------------------------------------
			// 송신 쿼리 등록
			//------------------------------------------------
			StringCchPrintf(szQuery, 1024, L"\\Network Interface(%s)\\Bytes Sent/sec", szCur);
			PdhAddCounter(_szQuery, szQuery, NULL, &_ethernetes[i].PDHCounterNetworkSendBytes);
		}

		delete[] szCounters;
		delete[] szInterfaces;

		return false;
	}
}


