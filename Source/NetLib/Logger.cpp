#include "Logger.h"

namespace cov1013
{
	bool Logger::Initialize(const wchar_t* DirectoryPath, const eLogLevel LogLevel)
	{
		if (SetLogLevel(LogLevel) == false)
		{
			return false;
		}

		if (SetLogDirectoryPath(DirectoryPath) == false)
		{
			return false;
		}

		InitializeSRWLock(&_fileLock);

		return true;
	}

	bool Logger::SetLogLevel(const eLogLevel LogLevel)
	{
		if (LogLevel < eLogLevel::None || LogLevel > eLogLevel::All)
		{
			wprintf(L"Invalid Log Level. (Level:%d)\n", LogLevel);
			return false;
		}

		_logLevel = LogLevel;
		return true;
	}

	bool Logger::SetLogDirectoryPath(const wchar_t* DirectoryPath)
	{
		const int Result = _wmkdir(DirectoryPath);
		if (Result == ENOENT)
		{
			wprintf_s(L"Failed Make Log Directory. (Path:%s).\n", DirectoryPath);
			return false;
		}

		StringCchPrintf(_directoryPath, MAX_PATH, DirectoryPath);

		return true;
	}

	void Logger::LogToFile(const eLogLevel LogLevel, const wchar_t* Format, ...)
	{
		// 1) 가변인자 복사
		wchar_t logMessage[MAX_PATH] = { '\0' };
		{
			va_list va;
			va_start(va, Format);
			StringCchVPrintf(logMessage, MAX_PATH, Format, va);
			va_end(va);
		}

		// 2) 로그 타입 
		wchar_t logType[32];
		ConvertLogLevel(LogLevel, logType);

		// 3) 파일 이름 생성
		wchar_t fileName[MAX_PATH] = { '\0' };
		SYSTEMTIME nowTime;
		GetLocalTime(&nowTime);
		wsprintfW(fileName, L"%s/%d-%02d.txt", _directoryPath, nowTime.wYear, nowTime.wMonth);

		// 4) 파일에 쓰기
		const DWORD LogIndex = InterlockedIncrement(&_logIndex);
		AcquireSRWLockExclusive(&_fileLock);
		{
			_wfopen_s(&_file, fileName, L"a+");

			if (_file != NULL)
			{
				fwprintf_s(
					_file,
					L"[%d-%02d-%02d %02d:%02d:%02d][%s][%09d] : %s",
					nowTime.wYear, nowTime.wMonth, nowTime.wDay,
					nowTime.wHour, nowTime.wMinute, nowTime.wSecond,
					logType, LogIndex, logMessage
				);
				fclose(_file);
			}
		}
		ReleaseSRWLockExclusive(&_fileLock);
	}

	void Logger::LogToFile_HEX(const eLogLevel LogLevel, char* pEntry, int length, int row, const wchar_t* Format, ...)
	{
		// 1) 가변인자 복사
		wchar_t logMessage[MAX_PATH] = { '\0' };
		{
			va_list va;
			va_start(va, Format);
			StringCchVPrintf(logMessage, MAX_PATH, Format, va);
			va_end(va);
		}

		// 2) 로그 타입
		wchar_t logType[32];
		ConvertLogLevel(LogLevel, logType);

		// 3) 파일 이름 생성
		wchar_t fileName[MAX_PATH] = { '\0' };
		SYSTEMTIME nowTime;
		GetLocalTime(&nowTime);
		wsprintf(fileName, L"%s/%d%02d.txt", _directoryPath, nowTime.wYear, nowTime.wMonth);

		// 4) 파일에 쓰기
		const DWORD LogIndex = InterlockedIncrement(&_logIndex);
		AcquireSRWLockExclusive(&_fileLock);
		{
			_wfopen_s(&_file, fileName, L"a+");
			if (_file != NULL)
			{
				fwprintf_s(_file,
					L"[%d-%02d-%02d %02d:%02d:%02d[%s][%09d] : %s",
					nowTime.wYear, nowTime.wMonth, nowTime.wDay,
					nowTime.wHour, nowTime.wMinute, nowTime.wSecond,
					logType, LogIndex, logMessage
				);

				for (int i = 0; i < length; i++)
				{
					fwprintf_s(_file, L"%02x ", pEntry[i] & 0xff);
					if ((i + 1) % row == 0)
					{
						fwprintf_s(_file, L"\n");
					}
				}

				fwprintf_s(_file, L"\n");
				fclose(_file);
			}
		}
		ReleaseSRWLockExclusive(&_fileLock);
	}

	void Logger::LogToConsole(eLogLevel eLogLevel, const wchar_t* Format, ...)
	{
		if (_logLevel < eLogLevel)
		{
			return;
		}

		wchar_t logMessage[MAX_PATH] = { '\0' };
		{
			va_list va;
			va_start(va, Format);
			StringCchVPrintf(logMessage, MAX_PATH, Format, va);
			va_end(va);
		}

		wprintf_s(L"%s", logMessage);
	}

	void Logger::ConvertLogLevel(const eLogLevel LogLevel, wchar_t* pDestination)
	{
		switch (LogLevel)
		{
		case eLogLevel::Debug:
			wsprintf(pDestination, L"%s", L"DEBUG");
			break;
		case eLogLevel::Error:
			wsprintf(pDestination, L"%s", L"ERROR");
			break;
		case eLogLevel::System:
			wsprintf(pDestination, L"%s", L"SYSTEM");
			break;
		default:
			break;
		}
	}

	eLogLevel	Logger::_logLevel = eLogLevel::None;
	DWORD		Logger::_logIndex = 0;
	FILE*		Logger::_file = nullptr;
	SRWLOCK		Logger::_fileLock = {};
	wchar_t		Logger::_directoryPath[MAX_PATH] = {};
}