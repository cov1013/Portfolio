#include "Profiler.h"

namespace cov1013
{
	bool Profiler::Initialize(const wchar_t* DirectoryPath, const eProfilerUnit Unit)
	{
		if (SetDirectory(DirectoryPath) == false)
		{
			return false;
		}

		_tlsIndex = ::TlsAlloc();
		if (_tlsIndex == TLS_OUT_OF_INDEXES)
		{
			return false;
		}

		if (::QueryPerformanceFrequency(&_performanceFrequency) == 0)
		{
			return false;
		}

		_profileUnit = Unit;

		return true;
	}

	void Profiler::Release()
	{
		if (_tlsIndex != TLS_OUT_OF_INDEXES)
		{
			TlsFree(_tlsIndex);
		}
	}

	bool Profiler::SetDirectory(const wchar_t* DirectoryPath)
	{
		const int Result = _wmkdir(DirectoryPath);
		if (Result == ENOENT)
		{
			return false;
		}

		StringCchPrintf(_directoryPath, MAX_PATH, DirectoryPath);

		return true;
	}

	void Profiler::Begin(const wchar_t* Tag)
	{
		// TLS에 세팅된 프로파일링 샘플을 가져온다.
		const DWORD ThreadID = GetCurrentThreadId();
		ThreadSample* pThreadSample = static_cast<ThreadSample*>(TlsGetValue(_tlsIndex));

		// 만약 TLS에 프로파일링 샘플이 세팅되어 있지 않다면, 새로운 프로파일링 샘플을 세팅
		if (pThreadSample == nullptr)
		{
			const WORD ThreadSampleIndex = InterlockedIncrement16(&_threadSampleIndex);
			pThreadSample = &(_threadSamples[ThreadSampleIndex]);
			pThreadSample->ThreadID = ThreadID;

			TlsSetValue(_tlsIndex, reinterpret_cast<LPVOID>(pThreadSample));
		}

		int SampleIndex = 0;
		for (int i = 0; i < MAX_SMAPLE_COUNT; i++)
		{
			SampleIndex = i;

			// 동일 태그가 이미 존재한다.
			if (wcscmp(pThreadSample->Samples[SampleIndex].Tag, Tag) == 0)
			{
				break;
			}

			// 비어있는 곳을 찾았다
			if (pThreadSample->Samples[i].CallCount <= 0)
			{
				StringCchPrintf(pThreadSample->Samples[SampleIndex].Tag, MAX_PATH, Tag);
				break;
			}
		}

		// 호출 시작 시간 갱신
		QueryPerformanceCounter(&pThreadSample->Samples[SampleIndex].BeginTick);
	}

	void Profiler::End(const wchar_t* Tag)
	{
		// 샘플 찾기
		Sample* pSample = nullptr;
		LARGE_INTEGER tagFindElapsedTick;
		{
			LARGE_INTEGER startTick;
			QueryPerformanceCounter(&startTick);

			ThreadSample* pThreadSample = static_cast<ThreadSample*>(TlsGetValue(_tlsIndex));

			// TLS에 프로파일링 샘플이 세팅되어 있지 않다면 리턴
			if (pThreadSample == nullptr)
			{
				return;
			}

			// 해당 태그를 사용하고 있는 샘플을 찾는다.
			for (int i = 0; i < MAX_SMAPLE_COUNT; i++)
			{
				if (wcscmp(pThreadSample->Samples[i].Tag, Tag) == 0)
				{
					pSample = &pThreadSample->Samples[i];
					break;
				}
			}

			// 존재하지 않는 태그다.
			if (pSample == nullptr)
			{
				return;
			}

			LARGE_INTEGER endTick;
			QueryPerformanceCounter(&endTick);

			tagFindElapsedTick.QuadPart = endTick.QuadPart - startTick.QuadPart;
		}

		// 수행시간 측정
		LARGE_INTEGER nowTick;
		QueryPerformanceCounter(&nowTick);
		nowTick.QuadPart -= tagFindElapsedTick.QuadPart; // 태그 찾는 시간 제외

		LARGE_INTEGER elapsedTick;
		elapsedTick.QuadPart = nowTick.QuadPart - pSample->BeginTick.QuadPart;

		double profileUnit = 1.0;
		switch (_profileUnit)
		{	
		case cov1013::eProfilerUnit::Second:
			profileUnit = 1.0;
			break;
		case cov1013::eProfilerUnit::MilliSecond:
			profileUnit = 1000.0;
			break;
		case cov1013::eProfilerUnit::MicroSecond:
			profileUnit = 1000000.0;
			break;
		case cov1013::eProfilerUnit::NanoSecond:
			profileUnit = 1000000000.0;
			break;
		default:
			break;
		}
		const double ElapsedTime = ((double)elapsedTick.QuadPart * profileUnit) / (double)_performanceFrequency.QuadPart;
		// 최대/최소값 갱신
		if (pSample->MaxTime[1] < ElapsedTime)
		{
			pSample->MaxTime[0] = pSample->MaxTime[1];
			pSample->MaxTime[1] = ElapsedTime;
		}
		if (pSample->MinTime[1] > ElapsedTime)
		{
			pSample->MinTime[0] = pSample->MinTime[1];
			pSample->MinTime[1] = ElapsedTime;
		}

		// 전체 수행 시간 갱신
		pSample->TotalTime += ElapsedTime;

		// 호출 횟수 갱신
		pSample->CallCount++;
	}

	void Profiler::Output()
	{
		// 1) 파일 이름 생성
		wchar_t fileName[MAX_PATH] = { '\0' };
		SYSTEMTIME stNowTime;
		GetLocalTime(&stNowTime);
		wsprintf(fileName, L"%s/%04d.%02d.%02d_%02d.%02d.%02d.txt",
			_directoryPath,
			stNowTime.wYear, stNowTime.wMonth,	stNowTime.wDay,
			stNowTime.wHour, stNowTime.wMinute, stNowTime.wSecond
		);

		// 파일 오픈
		FILE* pFile;
		_wfopen_s(&pFile, fileName, L"a");
		if (pFile != nullptr)
		{
			fwprintf(pFile, L"[%04d.%02d.%02d %02d.%02d.%02d]\n",
				stNowTime.wYear, stNowTime.wMonth,	stNowTime.wDay,
				stNowTime.wHour, stNowTime.wMinute, stNowTime.wSecond
			);

			// 프로파일러에 등록된 스레드 개수 만큼만 반복
			for (auto i = 0; i <= _threadSampleIndex; i++)
			{
				fwprintf(pFile, L"\nThread Num : %d\n", (i + 1));
				fwprintf(pFile, L"-------------------------------------------------------------------------------------------------------------\n");
				fwprintf(pFile, L" ThreadID |                Name  |           Average  |            Min   |            Max   |          Call |\n");
				fwprintf(pFile, L"-------------------------------------------------------------------------------------------------------------\n");

				// 프로파일링 샘플 데이터 순회
				for (auto j = 0; j < MAX_SMAPLE_COUNT; j++)
				{
					Sample sample = _threadSamples[i].Samples[j];

					// 해당 샘플에 데이터가 없다면 반복문 종료
					if (sample.CallCount == 0)
					{
						break;
					}

					// 설정 단위에 맞게 출력
					switch (_profileUnit)
					{
					case eProfilerUnit::Second:
						fwprintf(pFile,
							L"%9u | %20s | %17.4fs | %15.4fs | %15.4fs | %13I64u |\n",
							_threadSamples[i].ThreadID,
							sample.Tag,
							(sample.TotalTime - (sample.MaxTime[0] + sample.MaxTime[1])) / (sample.CallCount - 2),
							sample.MinTime[1],
							sample.MaxTime[1],
							sample.CallCount
						);
						break;

					case eProfilerUnit::MilliSecond:
						fwprintf(pFile,
							L"%9u | %20s | %16.4fms | %14.4fms | %14.4fms | %13I64u |\n",
							_threadSamples[i].ThreadID,
							sample.Tag,
							(sample.TotalTime - (sample.MaxTime[0] + sample.MaxTime[1])) / (sample.CallCount - 2),
							sample.MinTime[1],
							sample.MaxTime[1],
							sample.CallCount
						);
						break;

					case eProfilerUnit::MicroSecond:
						fwprintf(pFile,
							L"%9u | %20s | %16.4fus | %14.4fus | %14.4fus | %13I64u |\n",
							_threadSamples[i].ThreadID,
							sample.Tag,
							(sample.TotalTime - (sample.MaxTime[0] + sample.MaxTime[1])) / (sample.CallCount - 2),
							sample.MinTime[1],
							sample.MaxTime[1],
							sample.CallCount
						);
						break;

					case eProfilerUnit::NanoSecond:
						fwprintf(pFile,
							L"%9u | %20s | %16.4fns | %14.4fns | %14.4fns | %13I64u |\n",
							_threadSamples[i].ThreadID,
							sample.Tag,
							(sample.TotalTime - (sample.MaxTime[0] + sample.MaxTime[1])) / (sample.CallCount - 2),
							sample.MinTime[1],
							sample.MaxTime[1],
							sample.CallCount
						);
						break;

					default:
						fwprintf(pFile, L"Profiler Error\n");
						break;
					}
				}
				fwprintf(pFile, L"-------------------------------------------------------------------------------------------------------------\n");
			}
			fwprintf(pFile, L"\n\n\n");
			fclose(pFile);
		}
	}

	DWORD					Profiler::_tlsIndex = TLS_OUT_OF_INDEXES;
	short					Profiler::_threadSampleIndex = -1;
	Profiler::ThreadSample	Profiler::_threadSamples[MAX_THREAD_SAMPLE_COUNT] = {};
	LARGE_INTEGER			Profiler::_performanceFrequency = {};
	eProfilerUnit			Profiler::_profileUnit = eProfilerUnit::Second;
	wchar_t					Profiler::_directoryPath[MAX_PATH] = { '\0' };
}