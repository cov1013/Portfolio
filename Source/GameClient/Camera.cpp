#include "framework.h"
#include "resource.h"
#include "../NetLib/List.h"
#include "../Common/protocol.h"
#include "BaseObject.h"		
#include "PlayerObject.h"		
#include "Camera.h"

Camera* Camera::m_pInstance = NULL;

Camera::Camera(PlayerObject** pPlayer, const DWORD dwWidth, const DWORD dwHeight)
{
	m_pPlayer = pPlayer;
	m_dwWidth = dwWidth;
	m_dwHeight = dwHeight;

	m_iPosX = 0;
	m_iPosY = 0;

	m_pInstance = this;
}

Camera::~Camera()
{
}

DWORD Camera::GetPosX(void)
{
	return m_iPosX;
}

DWORD Camera::GetPosY(void)
{
	return m_iPosY;
}

DWORD Camera::GetWidth(void)
{
	return m_dwWidth;
}

DWORD Camera::GetHeight(void)
{
	return m_dwHeight;
}

Camera* Camera::GetInstance()
{
	return m_pInstance;
}

void Camera::Update(void)
{
	if (NULL == *m_pPlayer)
	{
		return;
	}

	int iPosX = (*m_pPlayer)->GetCurX() - (m_dwWidth / 2);
	int iPosY = (*m_pPlayer)->GetCurY() - (m_dwHeight / 2);

	if (iPosX < dfRANGE_MOVE_LEFT)
	{
		iPosX = dfRANGE_MOVE_LEFT;
	}
	if ((iPosX + m_dwWidth) >= dfRANGE_MOVE_RIGHT)
	{
		iPosX = iPosX;
	}

	if (iPosY < dfRANGE_MOVE_TOP)
	{
		iPosY = dfRANGE_MOVE_TOP;
	}
	if ((iPosY + m_dwHeight) >= dfRANGE_MOVE_BOTTOM)
	{
		iPosY = iPosY;
	}

	m_iPosX = iPosX;
	m_iPosY = iPosY;
}
