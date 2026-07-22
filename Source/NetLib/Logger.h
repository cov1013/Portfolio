#pragma once
#include "pch.h"
#include "NetEnum.h"

namespace cov1013
{
#define CONSOLE(Level, Format, ...)						Logger::LogToConsole(Level, Format, ##__VA_ARGS__)
#define LOG_HEX(Level, Entry, Len, Row, Format, ...)	Logger::LogToFile_HEX(Level, Entry, Len, Row, Format, ##__VA_ARGS__)
#define LOG(Level, Format, ...)							\
do														\
{														\
	Logger::LogToFile(Level, Format, ##__VA_ARGS__);    \
} while (0)												\

	class Logger
	{
	public:
		static bool Initialize(const wchar_t* DirectoryPath, const eLogLevel LogLevel);
		static bool SetLogLevel(const eLogLevel LogLevel);
		static bool SetLogDirectoryPath(const wchar_t* DirectoryPath);

		static void LogToFile(const eLogLevel LogLevel, const wchar_t* Format, ...);
		static void LogToFile_HEX(const eLogLevel LogLevel, char* pEntry, int length , int row, const wchar_t* Format, ...);
		static void LogToConsole(const eLogLevel LogLevel, const wchar_t* Format, ...);

		inline static eLogLevel GetLogLevel() { return _logLevel; }

	private:
		static void ConvertLogLevel(const eLogLevel LogLevel, wchar_t* pDestination);

	private:
		static eLogLevel	_logLevel;
		static DWORD		_logIndex;
		static FILE*		_file;
		static SRWLOCK		_fileLock;
		static wchar_t		_directoryPath[MAX_PATH];
	};
}