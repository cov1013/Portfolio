#pragma once
#include "pch.h"
#include "NetEnum.h"

namespace cov1013
{
#ifndef _PROFILING_
	#define PRO_BEGIN(NONE)
	#define PRO_END(NONE)
#else
	#define PRO_BEGIN(TAG)	Profiler::Begin(TAG);
	#define PRO_END(TAG)	Profiler::End(TAG);
#endif

	class Profiler
	{
		static constexpr int MAX_SMAPLE_COUNT = 100;
		static constexpr int MAX_THREAD_SAMPLE_COUNT = 100;

		struct Sample
		{
			Sample()
			{
				MaxTime[0] = 0;
				MaxTime[1] = 0;
				MinTime[0] = DBL_MAX;
				MinTime[1] = DBL_MAX;
			}

			wchar_t				Tag[MAX_PATH] = {};
			double				TotalTime = 0;
			double				MaxTime[2] = {};
			double				MinTime[2] = {};
			unsigned __int64	CallCount = 0;
			LARGE_INTEGER		BeginTick = {};
		};

		struct ThreadSample
		{
			DWORD	ThreadID;
			Sample	Samples[MAX_SMAPLE_COUNT];
		};

	public:
		static bool Initialize(const wchar_t* DirectoryPath, const eProfilerUnit Unit);
		static void Release();
		static void Begin(const wchar_t* Tag);
		static void End(const wchar_t* Tag);
		static void Output();

	private:
		static bool SetDirectory(const wchar_t* DirectoryPath);

	private:
		static DWORD			_tlsIndex;
		static short			_threadSampleIndex;
		static ThreadSample		_threadSamples[MAX_THREAD_SAMPLE_COUNT];
		static LARGE_INTEGER	_performanceFrequency;
		static eProfilerUnit	_profileUnit;
		static wchar_t			_directoryPath[MAX_PATH];
	};
}