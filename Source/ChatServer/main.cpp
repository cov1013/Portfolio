#include "../NetLib/NetDevice.h"
#include "ChatServer.h"

int main()
{
#ifdef _DEBUG
	int	flags = _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF;
	_CrtSetDbgFlag(flags);
#endif

	cov1013::NetConfig netConfig;
	if (netConfig.Load("..\\Config\\ChatServer.xml") == false)
	{
		wprintf_s(L"Failure load ChatServer.xml");
		return 0;
	}

	if(netConfig.Verify() == false)
	{
		wprintf_s(L"Failure verify ChatServer.xml");
		return 0;
	}

	auto pServer = std::make_unique<ChatServer>(netConfig);
	if (pServer->Start() == false)
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
			pServer->Quit();
			bRunFlag = false;
			break;

		default:
			pServer->Control(InputKey);
			break;
		}
	}

#ifdef _DEBUG
	_ASSERT(_CrtCheckMemory());
#endif

	return 0;
}