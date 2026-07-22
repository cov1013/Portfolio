#pragma once

class GameConfig
{
public:
	using ACCOUNT_ID = __int64;
	using ACTOR_ID		= unsigned int;
	using ACTOR_POS		= int;
	using SECTOR_POS	= short;

	/// <summary>
	/// 
	/// </summary>
	static constexpr int ActorHPMax = 100;

	/// <summary>
	/// 
	/// </summary>
	static constexpr int ActorSpeedX = 3;

	/// <summary>
	/// 
	/// </summary>
	static constexpr int ActorSpeedY = 2;

	/// <summary>
	/// 
	/// </summary>
	static constexpr int AttackDamage1 = 1;

	/// <summary>
	/// 
	/// </summary>
	static constexpr int AttackDamage2 = 2;

	/// <summary>
	/// 
	/// </summary>
	static constexpr int AttackDamage3 = 3;

	/// <summary>
	/// 
	/// </summary>
	static constexpr int Attack1RactengleX = 80;

	/// <summary>
	/// 
	/// </summary>
	static constexpr int Attack1RactengleY = 10;

	/// <summary>
	/// 
	/// </summary>
	static constexpr int Attack2RactengleX = 90;

	/// <summary>
	/// 
	/// </summary>
	static constexpr int Attack2RactengleY = 10;

	/// <summary>
	/// 
	/// </summary>
	static constexpr int Attack3RactengleX = 100;

	/// <summary>
	/// 
	/// </summary>
	static constexpr int Attack3RactengleY = 20;

	/// <summary>
	/// 
	/// </summary>
	static constexpr SECTOR_POS SectorTop = 0;

	/// <summary>
	/// 
	/// </summary>
	static constexpr SECTOR_POS SectorLeft = 0;

	/// <summary>
	/// 
	/// </summary>
	static constexpr SECTOR_POS SectorRight = 6400;

	/// <summary>
	/// 
	/// </summary>
	static constexpr SECTOR_POS SectorBottom = 6400;

	/// <summary>
	/// 
	/// </summary>
	static constexpr SECTOR_POS SectorWidth = 64;

	/// <summary>
	/// 
	/// </summary>
	static constexpr SECTOR_POS SectorHeight = 64;

	/// <summary>
	/// 
	/// </summary>
	static constexpr SECTOR_POS MaxSectorX = SectorRight / SectorWidth;

	/// <summary>
	/// 
	/// </summary>
	static constexpr SECTOR_POS MaxSectorY = SectorBottom / SectorHeight;

	/// <summary>
	/// 
	/// </summary>
	static constexpr int ClientScreenWidth = 640;

	/// <summary>
	/// 
	/// </summary>
	static constexpr int ClientScreenHeight = 480;
};