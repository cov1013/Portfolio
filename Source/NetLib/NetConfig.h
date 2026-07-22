#pragma once
#include "pch.h"
#include "NetEnum.h"

namespace cov1013
{
	class NetConfig
	{
	public:
		using SESSION_ID = long long;
		using SESSION_INDEX = int;

		bool Verify() const;
		bool Load(const std::string_view& FilePath);
		bool IsAbnormalSocketError(const errno_t ErrCode) const;
		inline bool IsServer() const { return NetDeviceType == eNetDeviceType::LanServer || NetDeviceType == eNetDeviceType::NetServer; }
		inline bool IsClient() const { return NetDeviceType == eNetDeviceType::LanClient || NetDeviceType == eNetDeviceType::NetClient; }
		inline bool IsNetType() const { return NetDeviceType == eNetDeviceType::NetServer || NetDeviceType == eNetDeviceType::NetClient; }
		inline bool IsLanType() const { return NetDeviceType == eNetDeviceType::LanServer || NetDeviceType == eNetDeviceType::LanClient; }
		inline NetConfig::SESSION_INDEX GetSessionIndex(const NetConfig::SESSION_ID SessionID) { return SessionID & 0x00000000FFFFFFFF; }

		/// <summary>
		/// 
		/// </summary>
		static constexpr int			MaxSessionCount = 20000;

		/// <summary>
		/// 
		/// </summary>
		static constexpr int			MaxIOCPWorkerCount = 100;

		/// <summary>
		/// 
		/// </summary>
		static constexpr int			MaxSendPacketCount = 100;

		/// <summary>
		/// 
		/// </summary>
		static constexpr SESSION_ID		INVALID_SESSION_ID = -1;

		/// <summary>
		/// 
		/// </summary>
		static constexpr SESSION_INDEX	INVALID_SESSION_INDEX = -1;

		/// <summary>
		/// 
		/// </summary>
		eNetDeviceType NetDeviceType = eNetDeviceType::None;

		/// <summary>
		/// 
		/// </summary>
		eLogLevel LogLevel = eLogLevel::None;

		/// <summary>
		/// 
		/// </summary>
		eProfilerUnit ProfilerUnit = eProfilerUnit::MilliSecond;

		/// <summary>
		/// 
		/// </summary>
		std::wstring IP;

		/// <summary>
		/// 
		/// </summary>
		int Port = 0;

		/// <summary>
		/// 
		/// </summary>
		bool EnableTCPNagle = false;

		/// <summary>
		/// 
		/// </summary>
		bool EnableTCPKeepAlive = false;

		/// <summary>
		/// 
		/// </summary>
		bool EnableZeroCopySend = false;

		/// <summary>
		/// 
		/// </summary>
		bool EnableIOCPWorkerSend = false;

		/// <summary>
		/// 
		/// </summary>
		int	IOCPWorkerCount = 0;

		/// <summary>
		/// 
		/// </summary>
		int	IOCPWorkerConcurrentCount = 0;

		/// <summary>
		/// 
		/// </summary>
		int	SessionCount = 0;

		/// <summary>
		/// 
		/// </summary>
		int PacketCapacity = 1024;

		/// <summary>
		/// 
		/// </summary>
		int PacketMaxSize = 1024;

		/// <summary>
		/// 
		/// </summary>
		int PacketChunkElementCount = 200;

		/// <summary>
		/// 
		/// </summary>
		unsigned char PacketCode = '\0';

		/// <summary>
		/// 
		/// </summary>
		unsigned char PacketKey = '\0';

		/// <summary>
		/// 
		/// </summary>
		std::wstring DumpDirectoryPath;

		/// <summary>
		/// 
		/// </summary>
		std::wstring LogDirectoryPath;

		/// <summary>
		/// 
		/// </summary>
		std::wstring ProfilingDataDirectoryPath;
	};
}
