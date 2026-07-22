#pragma once

namespace cov1013
{
	/// <summary>
	/// 네트워크 장치 타입
	/// </summary>
	enum class eNetDeviceType
	{
		None = -1,
		LanServer,
		LanClient,
		NetServer,
		NetClient,
	};

	/// <summary>
	/// 로그 레벨
	/// </summary>
	enum class eLogLevel
	{
		None = 0,
		Error,
		System,
		Monitor,
		Debug,
		All,
	};

	/// <summary>
	/// 프로파일링 단위
	/// </summary>
	enum class eProfilerUnit
	{
		Second = 0,
		MilliSecond,
		MicroSecond,
		NanoSecond
	};

	/// <summary>
	/// 에러 코드
	/// </summary>
	enum class eErrorCode
	{
		None = 0,
		FailedAccept,
		NotExistBlankSessionIndex
	};

	/// <summary>
	/// IOCP 워커 스레드 작업 타입
	/// </summary>
	enum class eIOCPWorkerJobType
	{
		OnDisconnect = 0,
		OnExit = 0x0fffffff,
		OnSendPacket = 0x0ffffffe,
	};
}
