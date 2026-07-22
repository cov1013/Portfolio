#pragma once
#include <list>
#include <vector>
#include "../NetLib/NetConfig.h"
#include "../NetLib/NetDevice.h"
#include "GameConfig.h"

using namespace cov1013;

struct Sector
{
	GameConfig::SECTOR_POS X = 0;
	GameConfig::SECTOR_POS Y = 0;
};

struct SectorAround
{
	int		Count = 0;
	Sector	Around[9] = {};

	bool SetAround(const GameConfig::SECTOR_POS SectorX, const GameConfig::SECTOR_POS SectorY);
};

struct Actor;
class SectorManager
{
public:
	SectorManager(cov1013::NetDevice* pNetwork);
	~SectorManager() = default;
	bool MoveSector(Actor* pActor);
	bool AddActor(Actor* pActor, const GameConfig::SECTOR_POS SectorX, const GameConfig::SECTOR_POS SectorY);
	bool RemoveActor(Actor* pActor, const GameConfig::SECTOR_POS SectorX, const GameConfig::SECTOR_POS SectorY);
	bool SendPacket(const GameConfig::SECTOR_POS SectorX, const GameConfig::SECTOR_POS SectorY, void* pPacket, const NetConfig::SESSION_ID ExcludeSeessionID = -1) const;
	bool SendPacketBroadcast(void* pPacket, const NetConfig::SESSION_ID ExcludeSeessionID = -1) const;
	void GetActors(const GameConfig::SECTOR_POS SectorX, const GameConfig::SECTOR_POS SectorY, std::vector<Actor*>& Out) const;

private:
	bool IsInvalidPostion(const GameConfig::SECTOR_POS SectorX, const GameConfig::SECTOR_POS SectorY) const;

private:
	NetDevice*			_pNetwork = nullptr;
	std::list<Actor*>	_sectors[GameConfig::MaxSectorY][GameConfig::MaxSectorX];
};