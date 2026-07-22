#pragma once
#include <string>
#include "GameConfig.h"
#include "GameEnum.h"
#include "../NetLib/NetConfig.h"
#include "../NetLib/NetSession.h"

using namespace cov1013;

struct Actor
{
	NetConfig::SESSION_ID	SessionID = 0;
	eActorState				State = eActorState::None;
	GameConfig::ACTOR_ID	ActorID = 0;
	GameConfig::ACCOUNT_ID	AccountNo = 0;
	std::wstring			ID = {};
	std::wstring			Nickname = {};
	DWORD					LastRecvTime = 0;
	GameConfig::ACTOR_POS	PosX = 0;
	GameConfig::ACTOR_POS	PosY = 0;
	GameConfig::SECTOR_POS	CurSectorX = -1;
	GameConfig::SECTOR_POS	CurSectorY = -1;
	GameConfig::SECTOR_POS	OldSectorX = -1;
	GameConfig::SECTOR_POS	OldSectorY = -1;
	int						HP = 0;
	eActorAction			Action = eActorAction::Stand;

	void Clear();
	void OnUpdatePosition();
	bool Set(int posX, int posY, int hp, eActorAction action);
	short CalcSectorX() const;
	short CalcSectorY() const;
	bool DoConnect(const NetConfig::SESSION_ID SessionID);
	bool DoDisconnect();
	bool DoLogin(const long long inAccountNo, const std::wstring& inID, const std::wstring& inNickname);
	void DoHeartbeat();
	inline bool IsMoving() const { return Action >= eActorAction::Left && Action <= eActorAction::LeftDown; }
	inline bool IsConnected() const { return State == eActorState::Connected; }
	inline bool IsLogined() const { return State == eActorState::Logined; }
	inline bool IsDisconnected() const { return State == eActorState::Disconnected; }
	inline bool IsChangedSector() const { return CurSectorX != OldSectorX || CurSectorY != OldSectorY; }
};