#include "framework.h"
#include "resource.h"

// COMMOM
#include "../NetLib/List.h"
#include "../Common/protocol.h"
// SYSTEM
#include "SpriteDib.h"
// NETWORK
#include "GameNetwork.h"

// CONTENTS
#include "BaseObject.h"	
#include "PlayerObject.h"	
#include "EffectObject.h"
#include "Camera.h"	

//////////////////////////////////////////////////////////////////////////

extern SpriteDib g_cSpriteDib;

//////////////////////////////////////////////////////////////////////////

PlayerObject::PlayerObject()
{
	m_bPlayerCharacter = FALSE;
	m_bEffectFlag = FALSE;
	m_iObjectType = (int)eObjectType::Actor;
	m_dwActionCur = dfACTION_STAND;
	m_dwActionOld = dfACTION_MOVE_LL;
	m_iDirCur = 0;
	m_iDirOld = 0;
	m_chHP = 0;
}

//////////////////////////////////////////////////////////////////////////

PlayerObject::PlayerObject(const BOOL bPlayerCharacter, const int iSessionID, const int iDir, const int iX, const int iY, const BYTE chHP)
{
	m_iCurX = iX;
	m_iCurY = iY;
	m_iObjectID = iSessionID;
	m_bPlayerCharacter = bPlayerCharacter;
	m_bEffectFlag = FALSE;
	m_iObjectType = (int)eObjectType::Actor;
	m_dwActionCur = dfACTION_STAND;
	m_dwActionOld = dfACTION_MOVE_LL;
	m_iDirCur = iDir;
	m_iDirOld = iDir;
	m_chHP = chHP;
}

//////////////////////////////////////////////////////////////////////////

PlayerObject::~PlayerObject()
{
}

//////////////////////////////////////////////////////////////////////////

void PlayerObject::ActionProc(void)
{
	//----------------------------------------
	// 내 캐릭터 액션 처리
	//----------------------------------------
	if (m_bPlayerCharacter)
	{
		switch (m_dwActionCur)
		{
		//----------------------------------------
		// 현재 실행중인 액션이 공격 액션이다.
		//----------------------------------------
		case dfACTION_ATTACK1:
		case dfACTION_ATTACK2:
		case dfACTION_ATTACK3:
			if (IsEndFrame())
			{
				SetActionStand();

				//------------------------------------------------
				// 공격이 끝났더라면, 액션을 바꿔줘서 연속으로 공격할 때 재 전송이 가능하도록 한다.
				//------------------------------------------------
				m_dwActionInput = dfACTION_STAND;
				m_dwActionCur = dfACTION_STAND;
				m_dwActionOld = dfACTION_STAND;
			}
			break;

		default:
			//------------------------------------------------
			// 이외의 경우에는 사용자 입력 처리
			//------------------------------------------------
			InputActionProc();

			break;
		}

		return;
	}

	//----------------------------------------
	// 다른 플레이어 액션 처리
	//----------------------------------------
	else
	{
		switch (m_dwActionCur)
		{
			//----------------------------------------
			// 현재 실행중인 액션이 공격 액션이다.
			//----------------------------------------
		case dfACTION_ATTACK1:
		case dfACTION_ATTACK2:
		case dfACTION_ATTACK3:
			if (IsEndFrame())
			{
				SetActionStand();

				//------------------------------------------------
				// 공격이 끝났더라면, 액션을 바꿔줘서 연속으로 공격할 때 재 전송이 가능하도록 한다.
				//------------------------------------------------
				m_dwActionCur = dfACTION_STAND;
				m_dwActionOld = dfACTION_STAND;
				m_dwActionInput = dfACTION_STAND;
			}
			break;
		default:
			break;
		}

		//----------------------------------------
		// 서버에서 받은 정보는 공격을 끝마치고 움직이는 게 아니라
		// 오는 즉시 바로 처리해야하므로, 공격중이라고 해서 스킵하지 않는다.
		//----------------------------------------
		InputActionProc();
	}
}

//////////////////////////////////////////////////////////////////////////

void PlayerObject::InputActionProc(void)
{

	//--------------------------------------------------------------------
	// 이전 방향을 기준으로 현재 방향을 설정한다.
	// STAND, ATTACK, UU, DD에서 캐릭터의 이전 방향을 판단하여 방향에 맞는 애니메이션을 설정하기 위해 사용한다.
	// 
	// 방향을 알 수 있는 액션은 해당 함수에 대한 세팅을 무시하고 현재 프레임 방향에 맞는 애니메이션을 설정한다.
	//--------------------------------------------------------------------
	SetDirection();

	//====================================================
	// 액션 변경 부분
	//====================================================
	if (m_dwActionOld != m_dwActionInput)
	{
		switch (m_dwActionInput)
		{
		case dfACTION_STAND:
		{
			if (m_bPlayerCharacter)
			{
				netSendMoveStop((eActorDirection)m_iDirCur, m_iCurX, m_iCurY);
			}
			SetActionStand();
		}
		break;
		case dfACTION_MOVE_UU:
		{
			if (m_bPlayerCharacter)
			{
				if ((m_iCurY - ActorSpeedX) > dfRANGE_MOVE_TOP)
				{
					netSendMoveStart(eActorDirection::Up, m_iCurX, m_iCurY);
				}
			}
			SetActionMove();
		}
		break;
		case dfACTION_MOVE_DD:
		{
			if (m_bPlayerCharacter)
			{
				if ((m_iCurY + ActorSpeedY) < dfRANGE_MOVE_BOTTOM)
				{
					netSendMoveStart(eActorDirection::Down, m_iCurX, m_iCurY);
				}
			}
			SetActionMove();
		}

		break;
		case dfACTION_MOVE_LL:
		{
			m_iDirCur = dfACTION_MOVE_LL;

			if (m_bPlayerCharacter)
			{
				if ((m_iCurX - ActorSpeedX) > dfRANGE_MOVE_LEFT)
				{
					netSendMoveStart(eActorDirection::Left, m_iCurX, m_iCurY);
				}
			}
			SetActionMove();
		}

		break;
		case dfACTION_MOVE_RR:
		{
			m_iDirCur = dfACTION_MOVE_RR;

			if (m_bPlayerCharacter)
			{
				if ((m_iCurX + ActorSpeedX) < dfRANGE_MOVE_RIGHT)
				{
					netSendMoveStart(eActorDirection::Right, m_iCurX, m_iCurY);
				}
			}
			SetActionMove();
		}
		break;
		case dfACTION_MOVE_LU:
		{
			m_iDirCur = dfACTION_MOVE_LL;

			if (m_bPlayerCharacter)
			{
				if ((m_iCurX - ActorSpeedX) > dfRANGE_MOVE_LEFT && (m_iCurY - ActorSpeedY) > dfRANGE_MOVE_TOP)
				{
					netSendMoveStart(eActorDirection::LeftUp, m_iCurX, m_iCurY);
				}
			}
			SetActionMove();
		}
		break;
		case dfACTION_MOVE_LD:
		{
			m_iDirCur = dfACTION_MOVE_LL;

			if (m_bPlayerCharacter)
			{
				if ((m_iCurX - ActorSpeedX) > dfRANGE_MOVE_LEFT && (m_iCurY + ActorSpeedY) < dfRANGE_MOVE_BOTTOM)
				{
					netSendMoveStart(eActorDirection::LeftDown, m_iCurX, m_iCurY);
				}
			}
			SetActionMove();
		}
		break;
		case dfACTION_MOVE_RU:
		{
			m_iDirCur = dfACTION_MOVE_RR;

			if (m_bPlayerCharacter)
			{
				if ((m_iCurX + ActorSpeedX) < dfRANGE_MOVE_RIGHT && (m_iCurY - ActorSpeedY) > dfRANGE_MOVE_TOP)
				{
					netSendMoveStart(eActorDirection::RightUp, m_iCurX, m_iCurY);
				}
			}
			SetActionMove();
		}
		break;
		case dfACTION_MOVE_RD:
		{
			m_iDirCur = dfACTION_MOVE_RR;

			if (m_bPlayerCharacter)
			{
				if ((m_iCurX + ActorSpeedX) < dfRANGE_MOVE_RIGHT && (m_iCurY + ActorSpeedY) < dfRANGE_MOVE_BOTTOM)
				{
					netSendMoveStart(eActorDirection::RigthDown, m_iCurX, m_iCurY);
				}
			}
			SetActionMove();
		}
		break;
		case dfACTION_ATTACK1:
		{
			if (m_bPlayerCharacter)
			{
				// 움직이다가 공격했으면 STOP SEND
				if (m_dwActionOld < dfACTION_ATTACK1)
				{
					netSendMoveStop((eActorDirection)m_iDirCur, m_iCurX, m_iCurY);
				}

				netSendAttack1((eActorDirection)m_iDirCur, m_iCurX, m_iCurY);
			}

			SetActionAttack1();
		}

		break;

		case dfACTION_ATTACK2:
		{
			if (m_bPlayerCharacter)
			{
				// 움직이다가 공격했으면 STOP SEND
				if (m_dwActionOld < dfACTION_ATTACK1)
				{
					netSendMoveStop((eActorDirection)m_iDirCur, m_iCurX, m_iCurY);
				}

				netSendAttack2((eActorDirection)m_iDirCur, m_iCurX, m_iCurY);
			}

			SetActionAttack2();
		}

			break;

		case dfACTION_ATTACK3:
		{
			if (m_bPlayerCharacter)
			{
				// 움직이다가 공격했으면 STOP SEND
				if (m_dwActionOld < dfACTION_ATTACK1)
				{
					netSendMoveStop((eActorDirection)m_iDirCur, m_iCurX, m_iCurY);
				}

				netSendAttack3((eActorDirection)m_iDirCur, m_iCurX, m_iCurY);
			}

			SetActionAttack3();
		}
			break;

		default:
			break;
		}

		// 이전 액션 갱신
		m_dwActionOld = m_dwActionInput;

		// 현재 진행중인 액션 갱신
		m_dwActionCur = m_dwActionInput;
	}

	//====================================================
	// 실제 이동 처리
	//====================================================
	switch (m_dwActionInput)
	{
	case dfACTION_MOVE_UU:
		if ((m_iCurY - ActorSpeedY) > dfRANGE_MOVE_TOP)
		{
			m_iCurY -= ActorSpeedY;
		}
		break;
	case dfACTION_MOVE_DD:
		if ((m_iCurY + ActorSpeedY) < dfRANGE_MOVE_BOTTOM)
		{
			m_iCurY += ActorSpeedY;
		}
		break;
	case dfACTION_MOVE_LL:
		if ((m_iCurX - ActorSpeedX) > dfRANGE_MOVE_LEFT)
		{
			m_iCurX -= ActorSpeedX;
		}
		break;
	case dfACTION_MOVE_RR:
		if ((m_iCurX + ActorSpeedX) < dfRANGE_MOVE_RIGHT)
		{
			m_iCurX += ActorSpeedX;
		}
		break;
	case dfACTION_MOVE_LU:
		if ((m_iCurX - ActorSpeedX) > dfRANGE_MOVE_LEFT && (m_iCurY - ActorSpeedY) > dfRANGE_MOVE_TOP)
		{
			m_iCurX -= ActorSpeedX;
			m_iCurY -= ActorSpeedY;
		}
		break;
	case dfACTION_MOVE_LD:
		if ((m_iCurX - ActorSpeedX) > dfRANGE_MOVE_LEFT && (m_iCurY + ActorSpeedY) < dfRANGE_MOVE_BOTTOM)
		{
			m_iCurX -= ActorSpeedX;
			m_iCurY += ActorSpeedY;
		}
		break;
	case dfACTION_MOVE_RU:
		if ((m_iCurX + ActorSpeedX) < dfRANGE_MOVE_RIGHT && (m_iCurY - ActorSpeedY) > dfRANGE_MOVE_TOP)
		{
			m_iCurX += ActorSpeedX;
			m_iCurY -= ActorSpeedY;
		}
		break;
	case dfACTION_MOVE_RD:
		if ((m_iCurX + ActorSpeedX) < dfRANGE_MOVE_RIGHT && (m_iCurY + ActorSpeedY) < dfRANGE_MOVE_BOTTOM)
		{
			m_iCurX += ActorSpeedX;
			m_iCurY += ActorSpeedY;
		}
		break;
	default:
		break;
	}
}

//////////////////////////////////////////////////////////////////////////

void PlayerObject::Update(void)
{
	NextFrame();
	ActionProc();
}

//////////////////////////////////////////////////////////////////////////

void PlayerObject::Render(BYTE* bypDest, const int iDestWidth, const int iDestHeight, const int iDestPitch)
{
	int iX = m_iCurX - Camera::GetInstance()->GetPosX();
	int iY = m_iCurY - Camera::GetInstance()->GetPosY();

	// 그림자
	g_cSpriteDib.DrawSprite((int)eSprite::eSHADOW, iX, iY, bypDest, iDestWidth, iDestHeight, iDestPitch);

	// 캐릭터
	if (m_bPlayerCharacter)
	{
		g_cSpriteDib.DrawSpriteRed(m_iSpriteNow, iX, iY, bypDest, iDestWidth, iDestHeight, iDestPitch);
	}
	else
	{
		g_cSpriteDib.DrawSprite(m_iSpriteNow, iX, iY, bypDest, iDestWidth, iDestHeight, iDestPitch);
	}

	// HP 게이지
	g_cSpriteDib.DrawSprite((int)eSprite::eGUAGE_HP, iX, iY, bypDest, iDestWidth, iDestHeight, iDestPitch, m_chHP);
}

//////////////////////////////////////////////////////////////////////////

void PlayerObject::SetDirection(void)
{
	m_iDirOld = m_iDirCur;

	switch (m_iDirOld)
	{
	case dfACTION_MOVE_LL:
	case dfACTION_MOVE_LU:
	case dfACTION_MOVE_LD:
		m_iDirCur = dfACTION_MOVE_LL;
		break;
	case dfACTION_MOVE_RR:
	case dfACTION_MOVE_RU:
	case dfACTION_MOVE_RD:
		m_iDirCur = dfACTION_MOVE_RR;
		break;
	}
}

//////////////////////////////////////////////////////////////////////////

void PlayerObject::SetCurDirection(int iDirection)
{
	m_iDirCur = iDirection;
}

void PlayerObject::SetActionMove(void)
{
	switch (m_iDirCur)
	{
	case dfACTION_MOVE_LL:
	case dfACTION_MOVE_LU:
	case dfACTION_MOVE_LD:
		SetAnimation((int)eSprite::ePLAYER_MOVE_L01, (int)eSprite::ePLAYER_MOVE_L12, (int)eActorAnimationDelay::Move);
		break;

	case dfACTION_MOVE_RR:
	case dfACTION_MOVE_RU:
	case dfACTION_MOVE_RD:
		SetAnimation((int)eSprite::ePLAYER_MOVE_R01, (int)eSprite::ePLAYER_MOVE_R12, (int)eActorAnimationDelay::Move);
		break;
	}
}

void PlayerObject::SetActionStand(void)
{
	switch (m_iDirCur)
	{
	case dfACTION_MOVE_LL:
	case dfACTION_MOVE_LU:
	case dfACTION_MOVE_LD:
		SetAnimation((int)eSprite::ePLAYER_STAND_L01, (int)eSprite::ePLAYER_STAND_L03, (int)eActorAnimationDelay::Stand);
		break;

	case dfACTION_MOVE_RR:
	case dfACTION_MOVE_RU:
	case dfACTION_MOVE_RD:
		SetAnimation((int)eSprite::ePLAYER_STAND_R01, (int)eSprite::ePLAYER_STAND_R03, (int)eActorAnimationDelay::Stand);
		break;
	}
}
 
void PlayerObject::SetActionAttack1(void)
{
	switch (m_iDirCur)
	{
	case dfACTION_MOVE_LL:
	case dfACTION_MOVE_LU:
	case dfACTION_MOVE_LD:
		SetAnimation((int)eSprite::ePLAYER_ATTACK1_L01, (int)eSprite::ePLAYER_ATTACK1_L04, (int)eActorAnimationDelay::Attack1);
		break;

	case dfACTION_MOVE_RR:
	case dfACTION_MOVE_RU:
	case dfACTION_MOVE_RD:
		SetAnimation((int)eSprite::ePLAYER_ATTACK1_R01, (int)eSprite::ePLAYER_ATTACK1_R04, (int)eActorAnimationDelay::Attack1);
		break;
	}
}

void PlayerObject::SetActionAttack2(void)
{
	switch (m_iDirCur)
	{
	case dfACTION_MOVE_LL:
	case dfACTION_MOVE_LU:
	case dfACTION_MOVE_LD:
		SetAnimation((int)eSprite::ePLAYER_ATTACK2_L01, (int)eSprite::ePLAYER_ATTACK2_L04, (int)eActorAnimationDelay::Attack2);
		break;

	case dfACTION_MOVE_RR:
	case dfACTION_MOVE_RU:
	case dfACTION_MOVE_RD:
		SetAnimation((int)eSprite::ePLAYER_ATTACK2_R01, (int)eSprite::ePLAYER_ATTACK2_R04, (int)eActorAnimationDelay::Attack2);
		break;
	}
}

void PlayerObject::SetActionAttack3(void)
{
	switch (m_iDirCur)
	{
	case dfACTION_MOVE_LL:
	case dfACTION_MOVE_LU:
	case dfACTION_MOVE_LD:
		SetAnimation((int)eSprite::ePLAYER_ATTACK3_L01, (int)eSprite::ePLAYER_ATTACK3_L06, (int)eActorAnimationDelay::Attack3);
		break;

	case dfACTION_MOVE_RR:
	case dfACTION_MOVE_RU:
	case dfACTION_MOVE_RD:
		SetAnimation((int)eSprite::ePLAYER_ATTACK3_R01, (int)eSprite::ePLAYER_ATTACK3_R06, (int)eActorAnimationDelay::Attack3);
		break;
	}
}

BOOL PlayerObject::IsPlayer(void)
{
	return m_bPlayerCharacter;
}

BOOL PlayerObject::IsEffect()
{
	return m_bEffectFlag;
}

void PlayerObject::OnEffectFlag()
{
	m_bEffectFlag = TRUE;
}

void PlayerObject::OffEffectFlag()
{
	m_bEffectFlag = FALSE;
}

const DWORD PlayerObject::GetActionCur(void) const
{
	return m_dwActionCur;
}

const int PlayerObject::GetDirection(void) const
{
	return m_iDirCur;
}

const char PlayerObject::GetHP(void) const
{
	return m_chHP;
}

void PlayerObject::SetHP(const BYTE chHP)
{
	m_chHP = chHP;
}