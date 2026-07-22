#include "pch.h"
#include "CPUMeter.h"

namespace cov1013
{
	CPUMeter::CPUMeter(HANDLE hProcess)
	{
		// 프로세스 핸들 입력이 없다면 자기 자신이 대상
		if (hProcess == INVALID_HANDLE_VALUE)
		{
			_hProcess = GetCurrentProcess();
		}

		// 프로세서 개수를 확인한다.
		// 프로세스 (exe) 실행률 계산시 cpu 개수로 나누기를 하여 실제 사용률을 구함.
		SYSTEM_INFO SystemInfo;

		GetSystemInfo(&SystemInfo);
		_numberOfProcessors = SystemInfo.dwNumberOfProcessors;

		_processorTotal = 0;
		_processorUser = 0;
		_processorKernel = 0;

		_processTotal = 0;
		_processUser = 0;
		_processKernel = 0;

		_processorLastKernel.QuadPart = 0;
		_processorLastUser.QuadPart = 0;
		_processorLastIdle.QuadPart = 0;

		_processLastUser.QuadPart = 0;
		_processLastKernel.QuadPart = 0;
		_processLastTime.QuadPart = 0;

		UpdateCpuTime();
	}

	CPUMeter::~CPUMeter()
	{
	}

	void CPUMeter::UpdateCpuTime()
	{
		//--------------------------------------------------------------------
		// 프로세서 사용률을 갱신한다.
		// 
		// 본래의 사용 구조체는 FILETIME 이지만, ULARGE_INTEGER 와 구조가 같으므로 이를 사용함.
		// FILETIME 구조체는 100 나노세컨드 단위의 시간 단위를 표현하는 구조체임.
		//--------------------------------------------------------------------
		ULARGE_INTEGER Idle;
		ULARGE_INTEGER Kernel;
		ULARGE_INTEGER User;

		//--------------------------------------------------------------------
		// 시스템 사용 시간을 구한다.
		// 
		// 아이들(가동되지 않는) 타임 / 커널 사용 타임 (아이들포함) / 유저 사용 타임
		//--------------------------------------------------------------------
		if (GetSystemTimes((PFILETIME)&Idle, (PFILETIME)&Kernel, (PFILETIME)&User) == false)
		{
			return;
		}

		// 커널 타임에는 아이들 타임이 포함됨.
		ULONGLONG KernelDiff = Kernel.QuadPart - _processorLastKernel.QuadPart;
		ULONGLONG UserDiff = User.QuadPart - _processorLastUser.QuadPart;
		ULONGLONG IdleDiff = Idle.QuadPart - _processorLastIdle.QuadPart;

		ULONGLONG Total = KernelDiff + UserDiff;
		ULONGLONG TimeDiff;

		if (Total == 0)
		{
			_processorUser = 0.0f;
			_processorKernel = 0.0f;
			_processorTotal = 0.0f;
		}
		else
		{
			// 커널 타임에 아이들 타임이 있으므로 빼서 계산.
			_processorTotal = (float)((double)(Total - IdleDiff) / Total * 100.0f);
			_processorUser = (float)((double)UserDiff / Total * 100.0f);
			_processorKernel = (float)((double)(KernelDiff - IdleDiff) / Total * 100.0f);
		}

		_processorLastKernel = Kernel;
		_processorLastUser = User;
		_processorLastIdle = Idle;

		//--------------------------------------------------------------------
		// 지정된 프로세스 사용률을 갱신한다.
		//--------------------------------------------------------------------
		ULARGE_INTEGER None;
		ULARGE_INTEGER NowTime;

		//--------------------------------------------------------------------
		// 현재의 100 나노세컨드 단위 시간을 구한다. UTC 시간.
		// 
		// 프로세스 사용률 판단의 공식
		// 
		// a = 샘플간격의 시스템 시간을 구함. (그냥 실제로 지나간 시간)
		// b = 프로세스의 CPU 사용 시간을 구함.
		// 
		// a : 100 = b : 사용률  공식으로 사용률을 구함.
		//--------------------------------------------------------------------

		//--------------------------------------------------------------------
		// 얼마의 시간이 지났는지 100 나노세컨드 시간을 구함.
		//--------------------------------------------------------------------
		GetSystemTimeAsFileTime((LPFILETIME)&NowTime);

		//--------------------------------------------------------------------
		// 해당 프로세스가 사용한 시간을 구함.
		// 
		// 두번째, 세번째는 실행, 종료 시간으로 미사용.
		//--------------------------------------------------------------------
		GetProcessTimes(_hProcess, (LPFILETIME)&None, (LPFILETIME)&None, (LPFILETIME)&Kernel, (LPFILETIME)&User);

		//--------------------------------------------------------------------
		// 이전에 저장된 프로세스 시간과의 차를 구해서 실제로 얼마의 시간이 지났는지 확인.
		// 
		// 그리고 실제 지나온 시간으로 나누면 사용률이 나옴.
		//--------------------------------------------------------------------
		TimeDiff = NowTime.QuadPart - _processLastTime.QuadPart;
		UserDiff = User.QuadPart - _processLastUser.QuadPart;
		KernelDiff = Kernel.QuadPart - _processLastKernel.QuadPart;

		Total = KernelDiff + UserDiff;

		_processTotal = (float)(Total / (double)_numberOfProcessors / (double)TimeDiff * 100.0f);
		_processKernel = (float)(KernelDiff / (double)_numberOfProcessors / (double)TimeDiff * 100.0f);
		_processUser = (float)(UserDiff / (double)_numberOfProcessors / (double)TimeDiff * 100.0f);

		_processLastTime = NowTime;
		_processLastKernel = Kernel;
		_processLastUser = User;
	}
}