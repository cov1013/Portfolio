#include "framework.h"
#include "resource.h"
#include "../Common/protocol.h"
#include "../NetLib/List.h"
#include "FrameSkip.h"
#include "ScreenDib.h"
#include "SpriteDib.h"
#include "GameNetwork.h"
#include "KeyManager.h"
#include "BaseObject.h"	
#include "PlayerObject.h"
#include "EffectObject.h"		
#include "ObjectManager.h"		
#include "Camera.h"

void				WindowInit(HINSTANCE hInstance, int nCmdShow);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
void				LoadResource();
void				ClearResource();
void				Update();
void				Render();

HWND g_hWnd;
HIMC g_hOldMC;
BOOL g_bActiveApp = FALSE;
BOOL g_bConnected = FALSE;
BOOL g_bExitFlag = FALSE;

FrameSkip		g_cFrameSkip(50);
ScreenDib		g_cScreenDib(ClientScreenWidth, ClientScreenHeight, 32);
SpriteDib		g_cSpriteDib(66, 0x00ffffff);
KeyManager		g_cKeyManager;
ObjectManager	g_cObjectManager;

PlayerObject*	g_cPlayerCharacter;
Camera			g_CCamera(&g_cPlayerCharacter, ClientScreenWidth, ClientScreenHeight);
//e_SPRITE		g_TIleMap[100][100];

DWORD g_dwSessionID;
int g_iLogTime;
int g_iProfilingTime;

extern int g_iUpdateFPS;
extern int g_iRenderFPS;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR    lpCmdLine, _In_ int       nCmdShow)
{
	AllocConsole();
	AttachConsole(GetCurrentProcessId());
	FILE* pFile = freopen("CON", "w", stdout);

	timeBeginPeriod(1);	
	_wsetlocale(LC_ALL, L"Korean");
	LoadResource();	
	WindowInit(hInstance, nCmdShow);
	netStartUp();

	MSG msg = {};
	while (true)
	{
		if (g_bExitFlag)
		{
			break;
		}

		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (WM_QUIT == msg.message)
			{
				break;
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			netProcess();

			if (g_bConnected)
			{
				Update();
			}
		}
	}
	ClearResource();		
	netCleanUp();
	FreeConsole();
	timeEndPeriod(1);

	return (int)msg.wParam;
}

void LoadResource()
{
	g_cSpriteDib.LoadDibSprite((int)eSprite::eMAP, L"SpriteData\\_Map.bmp", 0, 0);
	g_cSpriteDib.LoadDibSprite((int)eSprite::ePLAYER_STAND_L01, L"SpriteData\\Stand_L_01.bmp", 71, 90);
	g_cSpriteDib.LoadDibSprite((int)eSprite::ePLAYER_STAND_L02, L"SpriteData\\Stand_L_02.bmp", 71, 90);
	g_cSpriteDib.LoadDibSprite((int)eSprite::ePLAYER_STAND_L03, L"SpriteData\\Stand_L_03.bmp", 71, 90);
	g_cSpriteDib.LoadDibSprite((int)eSprite::ePLAYER_STAND_R01, L"SpriteData\\Stand_R_01.bmp", 71, 90);
	g_cSpriteDib.LoadDibSprite((int)eSprite::ePLAYER_STAND_R02, L"SpriteData\\Stand_R_02.bmp", 71, 90);
	g_cSpriteDib.LoadDibSprite((int)eSprite::ePLAYER_STAND_R03, L"SpriteData\\Stand_R_03.bmp", 71, 90);
	g_cSpriteDib.LoadDibSprite((int)eSprite::ePLAYER_MOVE_L01, L"SpriteData\\Move_L_01.bmp", 71, 90);
	g_cSpriteDib.LoadDibSprite((int)eSprite::ePLAYER_MOVE_L02, L"SpriteData\\Move_L_02.bmp", 71, 90);
	g_cSpriteDib.LoadDibSprite((int)eSprite::ePLAYER_MOVE_L03, L"SpriteData\\Move_L_03.bmp", 71, 90);
	g_cSpriteDib.LoadDibSprite((int)eSprite::ePLAYER_MOVE_L04, L"SpriteData\\Move_L_04.bmp", 71, 90);
	g_cSpriteDib.LoadDibSprite((int)eSprite::ePLAYER_MOVE_L05, L"SpriteData\\Move_L_05.bmp", 71, 90);
	g_cSpriteDib.LoadDibSprite((int)eSprite::ePLAYER_MOVE_L06, L"SpriteData\\Move_L_06.bmp", 71, 90);
	g_cSpriteDib.LoadDibSprite((int)eSprite::ePLAYER_MOVE_L07, L"SpriteData\\Move_L_07.bmp", 71, 90);
	g_cSpriteDib.LoadDibSprite((int)eSprite::ePLAYER_MOVE_L08, L"SpriteData\\Move_L_08.bmp", 71, 90);
	g_cSpriteDib.LoadDibSprite((int)eSprite::ePLAYER_MOVE_L09, L"SpriteData\\Move_L_09.bmp", 71, 90);
	g_cSpriteDib.LoadDibSprite((int)eSprite::ePLAYER_MOVE_L10, L"SpriteData\\Move_L_10.bmp", 71, 90);
	g_cSpriteDib.LoadDibSprite((int)eSprite::ePLAYER_MOVE_L11, L"SpriteData\\Move_L_11.bmp", 71, 90);
	g_cSpriteDib.LoadDibSprite((int)eSprite::ePLAYER_MOVE_L12, L"SpriteData\\Move_L_12.bmp", 71, 90);
	g_cSpriteDib.LoadDibSprite((int)eSprite::ePLAYER_MOVE_R01, L"SpriteData\\Move_R_01.bmp", 71, 90);
	g_cSpriteDib.LoadDibSprite((int)eSprite::ePLAYER_MOVE_R02, L"SpriteData\\Move_R_02.bmp", 71, 90);
	g_cSpriteDib.LoadDibSprite((int)eSprite::ePLAYER_MOVE_R03, L"SpriteData\\Move_R_03.bmp", 71, 90);
	g_cSpriteDib.LoadDibSprite((int)eSprite::ePLAYER_MOVE_R04, L"SpriteData\\Move_R_04.bmp", 71, 90);
	g_cSpriteDib.LoadDibSprite((int)eSprite::ePLAYER_MOVE_R05, L"SpriteData\\Move_R_05.bmp", 71, 90);
	g_cSpriteDib.LoadDibSprite((int)eSprite::ePLAYER_MOVE_R06, L"SpriteData\\Move_R_06.bmp", 71, 90);
	g_cSpriteDib.LoadDibSprite((int)eSprite::ePLAYER_MOVE_R07, L"SpriteData\\Move_R_07.bmp", 71, 90);
	g_cSpriteDib.LoadDibSprite((int)eSprite::ePLAYER_MOVE_R08, L"SpriteData\\Move_R_08.bmp", 71, 90);
	g_cSpriteDib.LoadDibSprite((int)eSprite::ePLAYER_MOVE_R09, L"SpriteData\\Move_R_09.bmp", 71, 90);
	g_cSpriteDib.LoadDibSprite((int)eSprite::ePLAYER_MOVE_R10, L"SpriteData\\Move_R_10.bmp", 71, 90);
	g_cSpriteDib.LoadDibSprite((int)eSprite::ePLAYER_MOVE_R11, L"SpriteData\\Move_R_11.bmp", 71, 90);
	g_cSpriteDib.LoadDibSprite((int)eSprite::ePLAYER_MOVE_R12, L"SpriteData\\Move_R_12.bmp", 71, 90);
	g_cSpriteDib.LoadDibSprite((int)eSprite::ePLAYER_ATTACK1_R01, L"SpriteData\\Attack1_R_01.bmp", 71, 90);
	g_cSpriteDib.LoadDibSprite((int)eSprite::ePLAYER_ATTACK1_R02, L"SpriteData\\Attack1_R_02.bmp", 71, 90);
	g_cSpriteDib.LoadDibSprite((int)eSprite::ePLAYER_ATTACK1_R03, L"SpriteData\\Attack1_R_03.bmp", 71, 90);
	g_cSpriteDib.LoadDibSprite((int)eSprite::ePLAYER_ATTACK1_R04, L"SpriteData\\Attack1_R_04.bmp", 71, 90);
	g_cSpriteDib.LoadDibSprite((int)eSprite::ePLAYER_ATTACK1_L01, L"SpriteData\\Attack1_L_01.bmp", 71, 90);
	g_cSpriteDib.LoadDibSprite((int)eSprite::ePLAYER_ATTACK1_L02, L"SpriteData\\Attack1_L_02.bmp", 71, 90);
	g_cSpriteDib.LoadDibSprite((int)eSprite::ePLAYER_ATTACK1_L03, L"SpriteData\\Attack1_L_03.bmp", 71, 90);
	g_cSpriteDib.LoadDibSprite((int)eSprite::ePLAYER_ATTACK1_L04, L"SpriteData\\Attack1_L_04.bmp", 71, 90);
	g_cSpriteDib.LoadDibSprite((int)eSprite::ePLAYER_ATTACK2_R01, L"SpriteData\\Attack2_R_01.bmp", 71, 90);
	g_cSpriteDib.LoadDibSprite((int)eSprite::ePLAYER_ATTACK2_R02, L"SpriteData\\Attack2_R_02.bmp", 71, 90);
	g_cSpriteDib.LoadDibSprite((int)eSprite::ePLAYER_ATTACK2_R03, L"SpriteData\\Attack2_R_03.bmp", 71, 90);
	g_cSpriteDib.LoadDibSprite((int)eSprite::ePLAYER_ATTACK2_R04, L"SpriteData\\Attack2_R_04.bmp", 71, 90);
	g_cSpriteDib.LoadDibSprite((int)eSprite::ePLAYER_ATTACK2_L01, L"SpriteData\\Attack2_L_01.bmp", 71, 90);
	g_cSpriteDib.LoadDibSprite((int)eSprite::ePLAYER_ATTACK2_L02, L"SpriteData\\Attack2_L_02.bmp", 71, 90);
	g_cSpriteDib.LoadDibSprite((int)eSprite::ePLAYER_ATTACK2_L03, L"SpriteData\\Attack2_L_03.bmp", 71, 90);
	g_cSpriteDib.LoadDibSprite((int)eSprite::ePLAYER_ATTACK2_L04, L"SpriteData\\Attack2_L_04.bmp", 71, 90);
	g_cSpriteDib.LoadDibSprite((int)eSprite::ePLAYER_ATTACK3_R01, L"SpriteData\\Attack3_R_01.bmp", 71, 90);
	g_cSpriteDib.LoadDibSprite((int)eSprite::ePLAYER_ATTACK3_R02, L"SpriteData\\Attack3_R_02.bmp", 71, 90);
	g_cSpriteDib.LoadDibSprite((int)eSprite::ePLAYER_ATTACK3_R03, L"SpriteData\\Attack3_R_03.bmp", 71, 90);
	g_cSpriteDib.LoadDibSprite((int)eSprite::ePLAYER_ATTACK3_R04, L"SpriteData\\Attack3_R_04.bmp", 71, 90);
	g_cSpriteDib.LoadDibSprite((int)eSprite::ePLAYER_ATTACK3_R05, L"SpriteData\\Attack3_R_05.bmp", 71, 90);
	g_cSpriteDib.LoadDibSprite((int)eSprite::ePLAYER_ATTACK3_R06, L"SpriteData\\Attack3_R_06.bmp", 71, 90);
	g_cSpriteDib.LoadDibSprite((int)eSprite::ePLAYER_ATTACK3_L01, L"SpriteData\\Attack3_L_01.bmp", 71, 90);
	g_cSpriteDib.LoadDibSprite((int)eSprite::ePLAYER_ATTACK3_L02, L"SpriteData\\Attack3_L_02.bmp", 71, 90);
	g_cSpriteDib.LoadDibSprite((int)eSprite::ePLAYER_ATTACK3_L03, L"SpriteData\\Attack3_L_03.bmp", 71, 90);
	g_cSpriteDib.LoadDibSprite((int)eSprite::ePLAYER_ATTACK3_L04, L"SpriteData\\Attack3_L_04.bmp", 71, 90);
	g_cSpriteDib.LoadDibSprite((int)eSprite::ePLAYER_ATTACK3_L05, L"SpriteData\\Attack3_L_05.bmp", 71, 90);
	g_cSpriteDib.LoadDibSprite((int)eSprite::ePLAYER_ATTACK3_L06, L"SpriteData\\Attack3_L_06.bmp", 71, 90);
	g_cSpriteDib.LoadDibSprite((int)eSprite::eEFFECT_SPARK_01, L"SpriteData\\xSpark_1.bmp", 0, 0);
	g_cSpriteDib.LoadDibSprite((int)eSprite::eEFFECT_SPARK_02, L"SpriteData\\xSpark_2.bmp", 0, 0);
	g_cSpriteDib.LoadDibSprite((int)eSprite::eEFFECT_SPARK_03, L"SpriteData\\xSpark_3.bmp", 0, 0);
	g_cSpriteDib.LoadDibSprite((int)eSprite::eEFFECT_SPARK_04, L"SpriteData\\xSpark_4.bmp", 0, 0);
	g_cSpriteDib.LoadDibSprite((int)eSprite::eGUAGE_HP, L"SpriteData\\HPGuage.bmp", 35, -12);
	g_cSpriteDib.LoadDibSprite((int)eSprite::eSHADOW, L"SpriteData\\Shadow.bmp", 32, 4);
	g_cSpriteDib.LoadDibSprite((int)eSprite::eSCROLL_TILE, L"SpriteData\\Tile_01.bmp", 0, 0);
}

void ClearResource()
{
	int iCount;
	int iMaxSprite = g_cSpriteDib.GetMaxSprite();

	for (iCount = 0; iCount < iMaxSprite; iCount++)
	{
		g_cSpriteDib.ReleaseSprite(iCount);
	}
}

void KeyProc()
{
	if (!g_bActiveApp)
	{
		return;
	}

	if (g_cPlayerCharacter == nullptr)
	{
		return;
	}

	g_cKeyManager.Update();

	// 이전 프레임에 눌렀었고, 현재 프레임에서는 땠다.
	if (g_cKeyManager.IsAway(VK_UP) 
		|| g_cKeyManager.IsAway(VK_DOWN) 
		|| g_cKeyManager.IsAway(VK_LEFT) 
		|| g_cKeyManager.IsAway(VK_RIGHT))
	{
		g_cPlayerCharacter->ActionInput(dfACTION_STAND);
	}
	// 아직 누르고 있다
	else
	{
		if (g_cKeyManager.IsHold(VK_UP))
		{
			g_cPlayerCharacter->ActionInput(dfACTION_MOVE_UU);
		}
		if (g_cKeyManager.IsHold(VK_DOWN))
		{
			g_cPlayerCharacter->ActionInput(dfACTION_MOVE_DD);
		}
		if (g_cKeyManager.IsHold(VK_LEFT))
		{
			g_cPlayerCharacter->ActionInput(dfACTION_MOVE_LL);
		}
		if (g_cKeyManager.IsHold(VK_RIGHT))
		{
			g_cPlayerCharacter->ActionInput(dfACTION_MOVE_RR);
		}
		if (g_cKeyManager.IsHold(VK_LEFT) && g_cKeyManager.IsHold(VK_UP))
		{
			g_cPlayerCharacter->ActionInput(dfACTION_MOVE_LU);
		}
		if (g_cKeyManager.IsHold(VK_RIGHT) && g_cKeyManager.IsHold(VK_UP))
		{
			g_cPlayerCharacter->ActionInput(dfACTION_MOVE_RU);
		}
		if (g_cKeyManager.IsHold(VK_LEFT) && g_cKeyManager.IsHold(VK_DOWN))
		{
			g_cPlayerCharacter->ActionInput(dfACTION_MOVE_LD);
		}
		if (g_cKeyManager.IsHold(VK_RIGHT) && g_cKeyManager.IsHold(VK_DOWN))
		{
			g_cPlayerCharacter->ActionInput(dfACTION_MOVE_RD);
		}
		if (g_cKeyManager.IsHold(dfVK_Z))
		{
			g_cPlayerCharacter->ActionInput(dfACTION_ATTACK1);
		}
		if (g_cKeyManager.IsHold(dfVK_X))
		{
			g_cPlayerCharacter->ActionInput(dfACTION_ATTACK2);
		}
		if (g_cKeyManager.IsHold(dfVK_C))
		{
			g_cPlayerCharacter->ActionInput(dfACTION_ATTACK3);
		}
	}
}

void Update()
{
	// 카메라 업데이트
	Camera::GetInstance()->Update();

	// 키보드 입력 처리
	KeyProc();

	// 객체 액션 처리
	g_cObjectManager.Update();

	// 그리기 & 프레임 스킵
	// 플립
	if (g_cFrameSkip.SkipFrame())
	{
		Render();
	}

	// FPS 출력
	g_cFrameSkip.PrintFPS(g_hWnd);
}

void Render()
{
	if (g_cPlayerCharacter == nullptr)
	{
		return;
	}

	BYTE* bypDest = g_cScreenDib.GetDibBuffer();
	int iDestWidth = g_cScreenDib.GetWidth();
	int iDestHeight = g_cScreenDib.GetHeight();
	int iDestPitch = g_cScreenDib.GetPitch();

	int iCameraX = Camera::GetInstance()->GetPosX();
	int iCameraY = Camera::GetInstance()->GetPosY();

	// Tile Draw
	int iTileX = iCameraX / 64;
	int iTileY = iCameraY / 64;

	for (int iY = 0; iY < 9; iY++)
	{
		int iDrawY = iTileY + iY;

		for (int iX = 0; iX < 11; iX++)
		{
			int iDrawX = iTileX + iX;

			g_cSpriteDib.DrawImage(
				(int)eSprite::eSCROLL_TILE,
				((iDrawX) * 64) - iCameraX,
				((iDrawY) * 64) - iCameraY,
				bypDest, iDestWidth,
				iDestHeight, iDestPitch
			);
		}
	}

	// 객체 Draw
	g_cObjectManager.Render(bypDest, iDestWidth, iDestHeight, iDestPitch);

	// Flip
	g_cScreenDib.Flip(g_hWnd);
}

void WindowInit(HINSTANCE hInstance, int nCmdShow)
{
	//-------------------------------------------
	// 윈도우 생성
	//-------------------------------------------
	WNDCLASSEXW wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GAMECLIENT));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = 0;
	wcex.lpszClassName = L"Online_TCP_Fighter";
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	RegisterClassExW(&wcex);

	g_hWnd = CreateWindowW(
		L"Online_TCP_Fighter",
		L"Online_TCP_Fighter",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0,
		CW_USEDEFAULT, 0,
		NULL, NULL, hInstance, NULL
	);

	if (!g_hWnd)
	{
		return;
	}

	ShowWindow(g_hWnd, nCmdShow);
	UpdateWindow(g_hWnd);

	//-------------------------------------------
	// 윈도우 사이즈 조정
	//-------------------------------------------
	SetFocus(g_hWnd);
	RECT WindowRect;
	WindowRect.top = 0;
	WindowRect.left = 0;
	WindowRect.right = ClientScreenWidth;
	WindowRect.bottom = ClientScreenHeight;

	AdjustWindowRectEx(
		&WindowRect,
		GetWindowStyle(g_hWnd),
		GetMenu(g_hWnd) != NULL,
		GetWindowExStyle(g_hWnd)
	);

	int iCenterPosX = (GetSystemMetrics(SM_CXSCREEN) / 2) - (640 / 2);
	int iCenterPosY = (GetSystemMetrics(SM_CYSCREEN) / 2) - (380 / 2);

	MoveWindow(
		g_hWnd,
		iCenterPosX, iCenterPosY,
		WindowRect.right - WindowRect.left,
		WindowRect.bottom - WindowRect.top,
		TRUE
	);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_ACTIVATEAPP:			// 서버 연결 성공
		g_bActiveApp = (BOOL)wParam;
		break;

	case WM_CREATE:
		g_hOldMC = ImmAssociateContext(hWnd, NULL);
		break;

	case WM_DESTROY:
		ImmAssociateContext(hWnd, g_hOldMC);
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}