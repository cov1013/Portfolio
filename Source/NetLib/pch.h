#pragma once
#pragma comment(lib, "Winmm.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "Dbghelp.lib")
#pragma comment(lib, "Pdh.lib")
#pragma comment(lib, "odbc32.lib")
#pragma comment(lib, "odbccp32.lib")

#define WIN32_LEAN_AND_MEAN // 거의 사용되지 않는 내용을 Windows 헤더에서 제외합니다.

#include <ws2tcpip.h>
#include <WinSock2.h>
#include <mstcpip.h>
#include <Windows.h>
#include <process.h>
#include <stdio.h>
#include <stdlib.h>
#include <strsafe.h>
#include <time.h>
#include <timeapi.h>
#include <assert.h>
#include <Dbghelp.h>
#include <crtdbg.h>
#include <direct.h>
#include <psapi.h>
#include <Pdh.h>
#include <float.h>
#include <sql.h>
#include <sqlext.h>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>