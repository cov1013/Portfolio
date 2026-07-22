#include <vector>
#include "../NetLib/NetDevice.h"
#include "EchoServer.h"

std::unique_ptr<EchoServer> g_pServer = nullptr;

int main()
{
#ifdef _DEBUG
	int	flags = _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF;
	_CrtSetDbgFlag(flags);
#endif

	cov1013::NetConfig netConfig;
	if (netConfig.Load("..\\Config\\EchoServer.xml") == false)
	{
		wprintf_s(L"Failure load EchoServer.xml");
		return 0;
	}

	if(netConfig.Verify() == false)
	{
		wprintf_s(L"Failure verify EchoServer.xml");
		return 0;
	}
	
	g_pServer = std::make_unique<EchoServer>(netConfig);
	if (g_pServer->Start() == false)
	{
		return 0;
	}

	bool bRunFlag = true;
	while (bRunFlag)
	{
		const wchar_t InputKey = _getwch();

		switch (InputKey)
		{
		case L'q':
		case L'Q':
			g_pServer->Quit();
			bRunFlag = false;
			break;

		default:
			g_pServer->Control(InputKey);
			break;
		}
	}

	cov1013::DBConnection::Release();

#ifdef _DEBUG
	_ASSERT(_CrtCheckMemory());
#endif

	return 0;
}