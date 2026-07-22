#include "pch.h"
#include "ThirdParty/tinyxml/tinyxml2.h"
#include "NetConfig.h"

namespace cov1013
{
	static std::wstring ASCIIToUTF(const std::string& str)
	{
		if (str.empty())
		{
			return L"";
		}

		int size = MultiByteToWideChar(
			CP_UTF8,
			0,
			str.c_str(),
			-1,
			nullptr,
			0);

		std::wstring wstr(size - 1, L'\0');

		MultiByteToWideChar(
			CP_UTF8,
			0,
			str.c_str(),
			-1,
			&wstr[0],
			size);

		return wstr;
	}

	static bool ReadInt(const tinyxml2::XMLElement* pRoot, const char* Name, int& dest)
	{
		const tinyxml2::XMLElement* pElement = pRoot->FirstChildElement(Name);
		if (pElement != nullptr)
		{
			pElement->QueryIntText(&dest);
			return true;
		}
		return false;
	}

	static bool ReadUInt(const tinyxml2::XMLElement* pRoot, const char* Name, unsigned int& dest)
	{
		const tinyxml2::XMLElement* pElement = pRoot->FirstChildElement(Name);
		if (pElement != nullptr)
		{
			pElement->QueryUnsignedText(&dest);
			return true;
		}
		return false;
	}

	static bool ReadBool(const tinyxml2::XMLElement* pRoot, const char* Name, bool& dest)
	{
		const tinyxml2::XMLElement* pElement = pRoot->FirstChildElement(Name);
		if (pElement != nullptr)
		{
			pElement->QueryBoolText(&dest);
			return true;
		}
		return false;
	}

	static bool ReadString(const tinyxml2::XMLElement* pRoot, const char* Name, std::wstring& dest)
	{
		const tinyxml2::XMLElement* pElement = pRoot->FirstChildElement(Name);
		if (pElement == nullptr || pElement->GetText() == nullptr)
		{
			return false;
		}

		std::string str = std::string(pElement->GetText());

		ASCIIToUTF(str).swap(dest);

		return true;
	}

	bool NetConfig::Verify() const
	{
		switch (NetDeviceType)
		{
		case eNetDeviceType::LanServer:
		case eNetDeviceType::NetServer:
			if (SessionCount > NetConfig::MaxSessionCount)
			{
				wprintf_s(L"[%s] Invalid Session Count. (MaxCount:%d, CurCount:%d)\n", __FUNCTIONW__, NetConfig::MaxSessionCount, SessionCount);
				return false;
			}
			break;

		case eNetDeviceType::LanClient:
		case eNetDeviceType::NetClient:
			if (SessionCount > 1)
			{
				wprintf_s(L"[%s] Invalid Session Count. (MaxCount:%d, CurCount:%d)\n", __FUNCTIONW__, 1, SessionCount);
				return false;
			}
			break;
		default:
			wprintf_s(L"[%s] Invalid Network Type. (NetworkType:%d)\n", __FUNCTIONW__, static_cast<int>(NetDeviceType));
			return false;
		}

		if (IOCPWorkerCount > NetConfig::MaxIOCPWorkerCount)
		{
			wprintf_s(L"[%s] Invalid IOCP Worker Count. (MaxCount:%d, CurCount:%d)\n", __FUNCTIONW__, NetConfig::MaxIOCPWorkerCount, IOCPWorkerCount);
			return false;
		}

		if (IOCPWorkerCount < IOCPWorkerConcurrentCount)
		{
			wprintf_s(L"[%s] Invalid Worker Concurrent Count. (IOCPWorkerCount:%d, IOCPWorkerConcurrentCount:%d)\n", __FUNCTIONW__, IOCPWorkerCount, IOCPWorkerConcurrentCount);
			return false;
		}

		if(LogLevel < eLogLevel::None || LogLevel > eLogLevel::All)
		{
			wprintf_s(L"[%s] Invalid Log Level. (LogLevel:%d)\n", __FUNCTIONW__, static_cast<int>(LogLevel));
			return false;
		}

		if(ProfilerUnit < eProfilerUnit::Second || ProfilerUnit > eProfilerUnit::NanoSecond)
		{
			wprintf_s(L"[%s] Invalid Profiler Unit. (ProfilerUnit:%d)\n", __FUNCTIONW__, static_cast<int>(ProfilerUnit));
			return false;
		}

		if(IP.empty())
		{
			wprintf_s(L"[%s] Invalid IP. (IP:%s)\n", __FUNCTIONW__, IP.c_str());
			return false;
		}

		if(Port <= 0 || Port > 65535)
		{
			wprintf_s(L"[%s] Invalid Port. (Port:%d)\n", __FUNCTIONW__, Port);
			return false;
		}

		if(DumpDirectoryPath.empty())
		{
			wprintf_s(L"[%s] Invalid Dump Directory Path. (DumpDirectoryPath:%s)\n", __FUNCTIONW__, DumpDirectoryPath.c_str());
			return false;
		}

		if(LogDirectoryPath.empty())
		{
			wprintf_s(L"[%s] Invalid Log Directory Path. (LogDirectoryPath:%s)\n", __FUNCTIONW__, LogDirectoryPath.c_str());
			return false;
		}

		if(ProfilingDataDirectoryPath.empty())
		{
			wprintf_s(L"[%s] Invalid Profiling Data Directory Path. (ProfilingDataDirectoryPath:%s)\n", __FUNCTIONW__, ProfilingDataDirectoryPath.c_str());
			return false;
		}

		return true;
	}

	bool NetConfig::Load(const std::string_view& FilePath)
	{
		tinyxml2::XMLDocument document;
		const auto ErrCode = document.LoadFile(FilePath.data());
		if (ErrCode != tinyxml2::XML_SUCCESS)
		{
			printf("[%s] Failed Load Config File. (Path:%s, Error:%s)\n", __FUNCTION__, FilePath.data(), document.ErrorStr());
			return false;
		}

		const auto* pRoot = document.RootElement();
		if (pRoot == nullptr)
		{
			printf("[%s] Not Exist Root Element. (Path:%s)\n", __FUNCTION__, FilePath.data());
			return false;
		}

		if (ReadInt(pRoot, "NetDeviceType", reinterpret_cast<int&>(NetDeviceType)) == false)
		{
			printf("[%s] Not Exist NetDeviceType. (Path:%s)\n", __FUNCTION__, FilePath.data());
			return false;
		}

		if (ReadInt(pRoot, "LogLevel", reinterpret_cast<int&>(LogLevel)) == false)
		{
			printf("[%s] Not Exist LogLevel. (Path:%s)\n", __FUNCTION__, FilePath.data());
			return false;
		}

		if (ReadInt(pRoot, "ProfilerUnit", reinterpret_cast<int&>(ProfilerUnit)) == false)
		{
			printf("[%s] Not Exist ProfilerUnit. (Path:%s)\n", __FUNCTION__, FilePath.data());
			return false;
		}

		if (ReadString(pRoot, "IP", IP) == false)
		{
			printf("[%s] Not Exist IP. (Path:%s)\n", __FUNCTION__, FilePath.data());
			return false;
		}

		if(ReadInt(pRoot, "Port", Port) == false)
		{
			printf("[%s] Not Exist Port. (Path:%s)\n", __FUNCTION__, FilePath.data());
			return false;
		}

		if (ReadBool(pRoot, "EnableTCPNagle", EnableTCPNagle) == false)
		{
			printf("[%s] Not Exist EnableTCPNagle. (Path:%s)\n", __FUNCTION__, FilePath.data());
			return false;
		}

		if (ReadBool(pRoot, "EnableTCPKeepAlive", EnableTCPKeepAlive) == false)
		{
			printf("[%s] Not Exist EnableTCPKeepAlive. (Path:%s)\n", __FUNCTION__, FilePath.data());
			return false;
		}

		if(ReadBool(pRoot, "EnableZeroCopySend", EnableZeroCopySend) == false)
		{
			printf("[%s] Not Exist EnableZeroCopySend. (Path:%s)\n", __FUNCTION__, FilePath.data());
			return false;
		}

		if (ReadBool(pRoot, "EnableIOCPWorkerSend", EnableIOCPWorkerSend) == false)
		{
			printf("[%s] Not Exist EnableIOCPWorkerSend. (Path:%s)\n", __FUNCTION__, FilePath.data());
			return false;
		}

		if(ReadInt(pRoot, "IOCPWorkerCount", IOCPWorkerCount) == false)
		{
			printf("[%s] Not Exist IOCPWorkerCount. (Path:%s)\n", __FUNCTION__, FilePath.data());
			return false;
		}

		if (ReadInt(pRoot, "IOCPWorkerConcurrentCount", IOCPWorkerConcurrentCount) == false)
		{
			printf("[%s] Not Exist IOCPWorkerConcurrentCount. (Path:%s)\n", __FUNCTION__, FilePath.data());
			return false;
		}

		if (ReadInt(pRoot, "SessionCount", SessionCount) == false)
		{
			printf("[%s] Not Exist SessionCount. (Path:%s)\n", __FUNCTION__, FilePath.data());
			return false;
		}

		if (ReadInt(pRoot, "PacketCapacity", PacketCapacity) == false)
		{
			printf("[%s] Not Exist PacketCapacity. (Path:%s)\n", __FUNCTION__, FilePath.data());
			return false;
		}

		if(ReadInt(pRoot, "PacketMaxSize", PacketMaxSize) == false)
		{
			printf("[%s] Not Exist PacketMaxSize. (Path:%s)\n", __FUNCTION__, FilePath.data());
			return false;
		}

#ifdef _TLS_MODE_
		if (ReadInt(pRoot, "PacketChunkElementCount", PacketChunkElementCount) == false)
		{
			printf("[%s] Not Exist PacketChunkElementCount. (Path:%s)\n", __FUNCTION__, FilePath.data());
			return false;
		}
#endif

		if (IsNetType())
		{
			unsigned int packetCode = 0;
			if (ReadUInt(pRoot, "PacketCode", reinterpret_cast<unsigned int&>(packetCode)) == false)
			{
				printf("[%s] Not Exist PacketCode. (Path:%s)\n", __FUNCTION__, FilePath.data());
				return false;
			}

			unsigned int packetKey = 0;
			if (ReadUInt(pRoot, "PacketKey", reinterpret_cast<unsigned int&>(packetKey)) == false)
			{
				printf("[%s] Not Exist PacketKey. (Path:%s)\n", __FUNCTION__, FilePath.data());
				return false;
			}

			PacketCode	= static_cast<unsigned char>(packetCode);
			PacketKey	= static_cast<unsigned char>(packetKey);
		}

		if (ReadString(pRoot, "DumpDirectoryPath", DumpDirectoryPath) == false)
		{
			printf("[%s] Not Exist DumpDirectoryPath. (Path:%s)\n", __FUNCTION__, FilePath.data());
			return false;
		}

		if(ReadString(pRoot, "LogDirectoryPath", LogDirectoryPath) == false)
		{
			printf("[%s] Not Exist LogDirectoryPath. (Path:%s)\n", __FUNCTION__, FilePath.data());
			return false;
		}

		if(ReadString(pRoot, "ProfilingDataDirectoryPath", ProfilingDataDirectoryPath) == false)
		{
			printf("[%s] Not Exist ProfilingDataDirectoryPath. (Path:%s)\n", __FUNCTION__, FilePath.data());
			return false;
		}

		return true;
	}

	bool NetConfig::IsAbnormalSocketError(const errno_t ErrCode) const
	{
		switch (ErrCode)
		{
		case WSAEINTR:
		case WSAEWOULDBLOCK:
		case WSAECONNRESET:
		case WSAESHUTDOWN:
			return false;
		default:
			return true;
		}
	}
}
