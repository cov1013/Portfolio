#include "GameConfig.h"
#include "Actor.h"

void Actor::Clear()
{
	SessionID = 0;
	State = eActorState::None;
	ActorID = 0;
	AccountNo = 0;
	ID.clear();
	Nickname.clear();
	LastRecvTime = 0;
	PosX = 0;
	PosY = 0;
	CurSectorX = -1;
	CurSectorY = -1;
	OldSectorX = -1;
	OldSectorY = -1;
	HP = 0;
	Action = eActorAction::Stand;
}

void Actor::OnUpdatePosition()
{
	if (IsMoving() == false)
	{
		return;
	}

	// 이동 속도 계산
	int VelocityX = 0;
	int VelocityY = 0;
	switch (Action)
	{
	case eActorAction::Left:
		VelocityX = -GameConfig::ActorSpeedX;
		break;
	case eActorAction::LeftUp:
		VelocityX = -GameConfig::ActorSpeedX;
		VelocityY = -GameConfig::ActorSpeedY;
		break;
	case eActorAction::Up:
		VelocityY = -GameConfig::ActorSpeedY;
		break;
	case eActorAction::RightUp:
		VelocityX = +GameConfig::ActorSpeedX;
		VelocityY = -GameConfig::ActorSpeedY;
		break;
	case eActorAction::Right:
		VelocityX = +GameConfig::ActorSpeedX;
		break;
	case eActorAction::RightDown:
		VelocityX = +GameConfig::ActorSpeedX;
		VelocityY = +GameConfig::ActorSpeedY;
		break;
	case eActorAction::Down:
		VelocityY = +GameConfig::ActorSpeedY;
		break;
	case eActorAction::LeftDown:
		VelocityX = -GameConfig::ActorSpeedX;
		VelocityY = +GameConfig::ActorSpeedY;
		break;
	default:
		break;
	}

	// X 이동
	PosX += VelocityX;
	if (PosX < GameConfig::SectorLeft)
	{
		PosX = GameConfig::SectorLeft;
	}
	else if (PosX > GameConfig::SectorRight)
	{
		PosX = GameConfig::SectorRight;
	}

	// Y 이동
	PosY += VelocityY;
	if (PosY < GameConfig::SectorTop)
	{
		PosY = GameConfig::SectorTop;
	}
	else if (PosY > GameConfig::SectorBottom)
	{
		PosY = GameConfig::SectorBottom;
	}

	// 섹터 좌표 갱신
	OldSectorX = CurSectorX;
	OldSectorY = CurSectorY;
	CurSectorX = CalcSectorX();
	CurSectorY = CalcSectorY();
}

bool Actor::Set(int posX, int posY, int hp, eActorAction action)
{
	// Action 체크
	if (action < eActorAction::Left || action > eActorAction::End)
	{
		return false;
	}

	// X 범위 체크
	if(posX < GameConfig::SectorLeft)
	{
		return false;
	}
	else if(posX > GameConfig::SectorRight)
	{
		return false;
	}

	// Y 범위 체크
	if(posY < GameConfig::SectorTop)
	{
		return false;
	}
	else if(posY > GameConfig::SectorBottom)
	{
		return false;
	}

	PosX = posX;
	PosY = posY;
	HP = hp;
	Action = action;
	CurSectorX = CalcSectorX();
	CurSectorY = CalcSectorY();
	OldSectorX = CurSectorX;
	OldSectorY = CurSectorY;

	return true;
}

short Actor::CalcSectorX() const
{
	short sectorX = static_cast<short>(PosX / GameConfig::SectorWidth);
	if (sectorX < 0)
	{
		return 0;
	}

	if (sectorX >= GameConfig::MaxSectorX)
	{
		return GameConfig::MaxSectorX - 1;
	}

	return sectorX;
}

short Actor::CalcSectorY() const
{
	short sectorY = static_cast<short>(PosY / GameConfig::SectorHeight);
	if (sectorY < 0)
	{
		return 0;
	}

	if (sectorY >= GameConfig::MaxSectorY)
	{
		return GameConfig::MaxSectorY - 1;
	}

	return sectorY;
}

bool Actor::DoConnect(const NetConfig::SESSION_ID sessionID)
{
	if(State != eActorState::None)
	{
		return false;
	}

	SessionID = sessionID;
	State = eActorState::Connected;
	return true;
}

bool Actor::DoDisconnect()
{
	if(State != eActorState::Connected && State != eActorState::Logined)
	{
		return false;
	}

	State = eActorState::Disconnected;
	return true;
}

bool Actor::DoLogin(const long long inAccountNo, const std::wstring& inID, const std::wstring& inNickname)
{
	if(State != eActorState::Connected)
	{
		return false;
	}

	AccountNo = inAccountNo;
	ID = inID;
	Nickname = inNickname;

	State = eActorState::Logined;

	return true;
}

void Actor::DoHeartbeat()
{
	LastRecvTime = timeGetTime();
}