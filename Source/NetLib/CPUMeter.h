#pragma once

namespace cov1013
{
	class CPUMeter
	{
	public:
		CPUMeter(HANDLE hProcess = INVALID_HANDLE_VALUE);
		~CPUMeter();
		void UpdateCpuTime();
		inline float ProcessorTotal() const { return _processorTotal; }
		inline float ProcessorUser() const { return _processorUser; }
		inline float ProcessorKernel() const { return _processorKernel; }
		inline float ProcessTotal() const { return _processTotal; }
		inline float ProcessUser() const { return _processUser; }
		inline float ProcessKernel() const { return _processKernel; }

	private:
		HANDLE			_hProcess = INVALID_HANDLE_VALUE;
		int				_numberOfProcessors = 0;
		float			_processorTotal = 0.f;
		float			_processorUser = 0.f;
		float			_processorKernel = 0.f;
		float			_processTotal = 0.f;
		float			_processUser = 0.f;
		float			_processKernel = 0.f;
		ULARGE_INTEGER	_processorLastKernel = {};
		ULARGE_INTEGER	_processorLastUser = {};
		ULARGE_INTEGER	_processorLastIdle = {};
		ULARGE_INTEGER	_processLastKernel = {};
		ULARGE_INTEGER	_processLastUser = {};
		ULARGE_INTEGER	_processLastTime = {};
	};
}