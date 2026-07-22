#pragma once

/// <summary>
/// 
/// </summary>
enum class eObjectType
{
	None = -1,
	Actor = 0,
	Effect = 1,
};

/// <summary>
/// 
/// </summary>
enum class eActorState
{
	None = -1,
	Connected = 0,
	Logined = 1,
	Disconnected = 2,
};

/// <summary>
/// 
/// </summary>
enum class eActorDirection : unsigned char
{
	Left = 0,
	LeftUp = 1,
	Up = 2,
	RightUp = 3,
	Right = 4,
	RightDown = 5,
	Down = 6,
	LeftDown = 7,
};

/// <summary>
/// 
/// </summary>
enum class eActorAction : unsigned char
{
	Left		= eActorDirection::Left,
	LeftUp		= eActorDirection::LeftUp,
	Up			= eActorDirection::Up,
	RightUp		= eActorDirection::RightUp,
	Right		= eActorDirection::Right,
	RightDown	= eActorDirection::RightDown,
	Down		= eActorDirection::Down,
	LeftDown	= eActorDirection::LeftDown,
	Attack1		= 8,
	Attack2		= 9,
	Attack3		= 10,
	Stand		= 11,
	End			= 12
};

/// <summary>
/// 
/// </summary>
enum class eAnimationDelay
{
	Stand = 5,
	Move = 4,
	Attack1 = 4,
	Attack2 = 4,
	Attack3 = 4,
	Effect = 3,
};

/// <summary>
/// 
/// </summary>
enum class eSprite
{
	Map = 0,
	PlayerStandLeft01,
	PlayerStandLeft02,
	PlayerStandLeft03,
	PlayerStandRight01,
	PlayerStandRight02,
	PlayerStandRight03,
	PlayerMoveLeft01,
	PlayerMoveLeft02,
	PlayerMoveLeft03,
	PlayerMoveLeft04,
	PlayerMoveLeft05,
	PlayerMoveLeft06,
	PlayerMoveLeft07,
	PlayerMoveLeft08,
	PlayerMoveLeft09,
	PlayerMoveLeft10,
	PlayerMoveLeft11,
	PlayerMoveLeft12,
	PlayerMoveRight01,
	PlayerMoveRight02,
	PlayerMoveRight03,
	PlayerMoveRight04,
	PlayerMoveRight05,
	PlayerMoveRight06,
	PlayerMoveRight07,
	PlayerMoveRight08,
	PlayerMoveRight09,
	PlayerMoveRight10,
	PlayerMoveRight11,
	PlayerMoveRight12,
	PlayerAttack1Right01,
	PlayerAttack1Right02,
	PlayerAttack1Right03,
	PlayerAttack1Right04,
	PlayerAttack1Left01,
	PlayerAttack1Left02,
	PlayerAttack1Left03,
	PlayerAttack1Left04,
	PlayerAttack2Right01,
	PlayerAttack2Right02,
	PlayerAttack2Right03,
	PlayerAttack2Right04,
	PlayerAttack2Left01,
	PlayerAttack2Left02,
	PlayerAttack2Left03,
	PlayerAttack2Left04,
	PlayerAttack3Right01,
	PlayerAttack3Right02,
	PlayerAttack3Right03,
	PlayerAttack3Right04,
	PlayerAttack3Right05,
	PlayerAttack3Right06,
	PlayerAttack3Left01,
	PlayerAttack3Left02,
	PlayerAttack3Left03,
	PlayerAttack3Left04,
	PlayerAttack3Left05,
	PlayerAttack3Left06,
	EffectSpark01,
	EffectSpark02,
	EffectSpark03,
	EffectSpark04,
	GaugeHP,
	Shadow,
	ScrollTile
};