#include "../NetLib/NetDevice.h"
#include "Actor.h"
#include "SectorManager.h"

using namespace cov1013;

bool SectorAround::SetAround(const GameConfig::SECTOR_POS SectorX, const GameConfig::SECTOR_POS SectorY)
{
	const GameConfig::SECTOR_POS EntryX = SectorX - 1;
	const GameConfig::SECTOR_POS EntryY = SectorY - 1;

	for (short y = 0; y < 3; y++)
	{
		// Clipping Y
		if (((EntryY + y) < 0) || ((EntryY + y) >= GameConfig::MaxSectorY))
		{
			continue;
		}

		for (short x = 0; x < 3; x++)
		{
			// Clipping X
			if (((EntryX + x) < 0) || ((EntryX + x) >= GameConfig::MaxSectorX))
			{
				continue;
			}

			Around[Count].X = EntryX + x;
			Around[Count].Y = EntryY + y;
			Count++;
		}
	}

	return Count > 0;
}

SectorManager::SectorManager(cov1013::NetDevice* pNetwork)
{
	assert(pNetwork != nullptr);
	_pNetwork = pNetwork;
}

bool SectorManager::MoveSector(Actor* pActor)
{
	if (pActor->OldSectorX != -1 && pActor->OldSectorY != -1)
	{
		if (RemoveActor(pActor, pActor->OldSectorX, pActor->OldSectorY) == false)
		{
			return false;
		}
	}

	if (AddActor(pActor, pActor->CurSectorX, pActor->CurSectorY) == false)
	{
		return false;
	}

	return true;
}

bool SectorManager::AddActor(Actor* pActor, const GameConfig::SECTOR_POS SectorX, const GameConfig::SECTOR_POS SectorY)
{
	if (IsInvalidPostion(SectorX, SectorY))
	{
		return false;
	}

	_sectors[SectorY][SectorX].push_back(pActor);
	return true;
}

bool SectorManager::RemoveActor(Actor* pActor, const GameConfig::SECTOR_POS SectorX, const GameConfig::SECTOR_POS SectorY)
{
	if (IsInvalidPostion(SectorX, SectorY))
	{
		return false;
	}

	_sectors[SectorY][SectorX].remove(pActor);
	return true;
}

bool SectorManager::SendPacket(const GameConfig::SECTOR_POS SectorX, const GameConfig::SECTOR_POS SectorY, void* pPacket, const NetConfig::SESSION_ID ExcludeSeessionID) const
{
	SectorAround sectorAround = {};
	if(sectorAround.SetAround(SectorX, SectorY) == false)
	{
		return false;
	}

	PacketBuffer* pSendPacket = static_cast<PacketBuffer*>(pPacket);

	PRO_BEGIN(L"SectorManager::SendPacket");
	for (int i = 0; i < sectorAround.Count; i++)
	{
		const int X = sectorAround.Around[i].X;
		const int Y = sectorAround.Around[i].Y;
		const std::list<Actor*>& Sector = _sectors[Y][X];
		for (const Actor* pActor : Sector)
		{
			const NetConfig::SESSION_ID SessionID = pActor->SessionID;
			if (SessionID == ExcludeSeessionID)
			{
				continue;
			}

			_pNetwork->SendPacket(SessionID, pSendPacket);
		}
	}
	PRO_END(L"SectorManager::SendPacket");

	return true;
}

bool SectorManager::SendPacketBroadcast(void* pPacket, const NetConfig::SESSION_ID ExcludeSeessionID) const
{
	PRO_BEGIN(L"SectorManager::SendPacketBroadcast");
	for (int y = 0; y < GameConfig::MaxSectorY; y++)
	{
		for (int x = 0; x < GameConfig::MaxSectorX; x++)
		{
			const std::list<Actor*>& Sector = _sectors[y][x];
			for (Actor* pActor : Sector)
			{
				const NetConfig::SESSION_ID SessionID = pActor->SessionID;
				if (SessionID == ExcludeSeessionID)
				{
					continue;
				}

				_pNetwork->SendPacket(SessionID, static_cast<PacketBuffer*>(pPacket));
			}
		}
	}
	PRO_END(L"SectorManager::SendPacketBroadcast");

	return true;
}

void SectorManager::GetActors(const GameConfig::SECTOR_POS SectorX, const GameConfig::SECTOR_POS SectorY, std::vector<Actor*>& Out) const
{
	SectorAround sectorAround = {};
	if (sectorAround.SetAround(SectorX, SectorY) == false)
	{
		return;
	}

	for (int i = 0; i < sectorAround.Count; i++)
	{
		const int X = sectorAround.Around[i].X;
		const int Y = sectorAround.Around[i].Y;
		const std::list<Actor*>& Sector = _sectors[Y][X];
		for (Actor* pActor : Sector)
		{
			Out.push_back(pActor);
		}
	}
}

bool SectorManager::IsInvalidPostion(const GameConfig::SECTOR_POS SectorX, const GameConfig::SECTOR_POS SectorY) const
{
	if (SectorX >= GameConfig::MaxSectorX || SectorY >= GameConfig::MaxSectorY)
	{
		return true;
	}

	if (SectorX < 0 || SectorY < 0)
	{
		return true;
	}

	return false;
}