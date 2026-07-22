#include "framework.h"
#include "resource.h"
#include "../NetLib/List.h"
#include "../Common/protocol.h"
#include "SpriteDib.h"
#include "BaseObject.h"
#include "PlayerObject.h"	
#include "Camera.h"	
#include "EffectObject.h"

extern SpriteDib g_cSpriteDib;

EffectObject::EffectObject()
{
	m_iObjectType = (int)eObjectType::Effect;

	m_bActionFlag = FALSE;
	m_dwAttackID = dfACTION_END;
	m_pPlayer = NULL;
}

EffectObject::EffectObject(PlayerObject* pPlayer)
{
	m_iObjectType = (int)eObjectType::Effect;
	m_bActionFlag = FALSE;
	m_dwAttackID = dfACTION_END;
	m_pPlayer = pPlayer;
}

EffectObject::~EffectObject()
{
}

void EffectObject::Update()
{
	if (m_pPlayer->GetObjectID() == -1)
	{
		m_bDestroyFlag = TRUE;
		return;
	}

	//-----------------------------------
	// 액션 플래그가 켜져있는가?
	//-----------------------------------
	if (m_bActionFlag)
	{
		NextFrame();

		//-----------------------------------
		// 이펙트가 끝났다면 삭제 예정
		//-----------------------------------
		if (IsEndFrame())
		{
			m_bActionFlag  = FALSE;
			m_bDestroyFlag = TRUE;
			return;
		}
	}
	
	// ---------------------------------------
	// 이펙트를 터트릴 스프라이트 이미지를 등록한다.
	// ---------------------------------------
	switch (m_pPlayer->GetSprite())
	{
	case (int)eSprite::ePLAYER_ATTACK1_R02:
	case (int)eSprite::ePLAYER_ATTACK1_L02:
	case (int)eSprite::ePLAYER_ATTACK2_R02:
	case (int)eSprite::ePLAYER_ATTACK2_L02:
	case (int)eSprite::ePLAYER_ATTACK3_R04:
	case (int)eSprite::ePLAYER_ATTACK3_L04:

		// 이펙트 액션 플래그 ON
		m_bActionFlag = TRUE;

		// 이펙트 그려질 위치 계산
		m_iCurX	= m_pPlayer->GetCurX();
		m_iCurY	= m_pPlayer->GetCurY();

		switch (m_pPlayer->GetDirection())
		{
		case dfACTION_MOVE_RR:
		case dfACTION_MOVE_RD:
		case dfACTION_MOVE_RU:
			m_iCurY -= 130;
			break;
		case dfACTION_MOVE_LL:
		case dfACTION_MOVE_LD:
		case dfACTION_MOVE_LU:
			m_iCurX -= 150;
			m_iCurY -= 130;
			break;
		}

		SetAnimation(
			(int)eSprite::eEFFECT_SPARK_01, 
			(int)eSprite::eEFFECT_SPARK_04,
			(int)eActorAnimationDelay::Effect);
		break;

	default:
		break;
	}
}

void EffectObject::Render(BYTE* bypDest, const int iDestWidth, const int iDestHeight, const int iDestPitch)
{
	if (m_bActionFlag)
	{
		int iX = m_iCurX - Camera::GetInstance()->GetPosX();
		int iY = m_iCurY - Camera::GetInstance()->GetPosY();

		g_cSpriteDib.DrawSprite(m_iSpriteNow, iX, iY, bypDest, iDestWidth, iDestHeight, iDestPitch);
	}
}