#pragma once
#include "GameEnum.h"
#include "GameConfig.h"

namespace protocol
{
	namespace game
	{
		enum class ePacketID : unsigned short
		{
			NtfSpawnOwnerActor = 0,
			NtfSpawnActor = 1,
			NtfDespawnActor = 2,

			ReqMoveStart = 10,
			NtfMoveStart = 11,
			ReqMoveStop = 12,
			NtfMoveStop = 13,

			ReqAttack1 = 20,
			NtfAttack1 = 21,
			ReqAttack2 = 22,
			NtfAttack2 = 23,
			ReqAttack3 = 24,
			NtfAttack3 = 25,

			NtfDamage = 30,

			ReqSyncPos = 250,
			NtfSyncPos = 251,

			ReqEcho = 252,
			ResEcho = 253,
		};

		/// <summary>
		/// 엑터 스폰 (Owner)
		/// </summary>
		struct NtfSpawnOwnerActor
		{
			GameConfig::ACTOR_ID	ActorID = 0;
			eActorAction			Action = eActorAction::Stand;
			GameConfig::ACTOR_POS	PosX = 0;
			GameConfig::ACTOR_POS	PosY = 0;
			int						HP = 0;

			void ReadFrom(void* pDest);
			void WriteTo(void* pDest) const;
		};

		/// <summary>
		/// 엑터 스폰
		/// </summary>
		struct NtfSpawnActor
		{
			GameConfig::ACTOR_ID	ActorID = 0;
			eActorAction			Action = eActorAction::Stand;
			GameConfig::ACTOR_POS	PosX = 0;
			GameConfig::ACTOR_POS	PosY = 0;
			int						HP = 0;

			void ReadFrom(void* pDest);
			void WriteTo(void* pDest) const;
		};

		/// <summary>
		/// 엑터 디스폰
		/// </summary>
		struct NtfDespawnActor
		{
			GameConfig::ACTOR_ID ActorID = 0;

			void ReadFrom(void* pDest);
			void WriteTo(void* pDest) const;
		};

		/// <summary>
		/// 이동 시작 요청
		/// </summary>
		struct ReqMoveStart
		{
			eActorAction Action = eActorAction::Stand;

			void ReadFrom(void* pDest);
			void WriteTo(void* pDest) const;
		};

		/// <summary>
		/// 이동 시작 알림
		/// </summary>
		struct NtfMoveStart
		{
			eActorAction			Action = eActorAction::Stand;
			GameConfig::ACTOR_ID	ActorID = 0;

			void ReadFrom(void* pDest);
			void WriteTo(void* pDest) const;
		};

		/// <summary>
		/// 이동 정지 요청
		/// </summary>
		struct ReqMoveStop
		{
			eActorAction	Action = eActorAction::Stand;

			void ReadFrom(void* pDest);
			void WriteTo(void* pDest) const;
		};

		/// <summary>
		/// 이동 정지 알림
		/// </summary>
		struct NtfMoveStop
		{
			eActorAction			Action = eActorAction::Stand;
			GameConfig::ACTOR_ID	ActorID = 0;

			void ReadFrom(void* pDest);
			void WriteTo(void* pDest) const;
		};

		/// <summary>
		/// 공격1 요청
		/// </summary>
		struct ReqAttack1
		{
			GameConfig::ACTOR_ID AttackeeID = 0;

			void ReadFrom(void* pDest);
			void WriteTo(void* pDest) const;
		};

		/// <summary>
		/// 공격1 알림
		/// </summary>
		struct NtfAttack1
		{
			GameConfig::ACTOR_ID AttackerID = 0;
			GameConfig::ACTOR_ID AttackeeID = 0;

			void ReadFrom(void* pDest);
			void WriteTo(void* pDest) const;
		};

		/// <summary>
		/// 공격2 요청
		/// </summary>
		struct ReqAttack2
		{
			GameConfig::ACTOR_ID AttackeeID = 0;

			void ReadFrom(void* pDest);
			void WriteTo(void* pDest) const;
		};

		/// <summary>
		/// 공격2 알림
		/// </summary>
		struct NtfAttack2
		{
			GameConfig::ACTOR_ID AttackerID = 0;
			GameConfig::ACTOR_ID AttackeeID = 0;

			void ReadFrom(void* pDest);
			void WriteTo(void* pDest) const;
		};

		/// <summary>
		/// 공격3 요청
		/// </summary>
		struct ReqAttack3
		{
			GameConfig::ACTOR_ID AttackeeID = 0;

			void ReadFrom(void* pDest);
			void WriteTo(void* pDest) const;
		};

		/// <summary>
		/// 공격3 알림
		/// </summary>
		struct NtfAttack3
		{
			GameConfig::ACTOR_ID AttackerID = 0;
			GameConfig::ACTOR_ID AttackeeID = 0;

			void ReadFrom(void* pDest);
			void WriteTo(void* pDest) const;
		};

		/// <summary>
		/// 데미지 알림
		/// </summary>
		struct NtfDamage
		{
			GameConfig::ACTOR_ID	ActorID = 0;
			int						ActorHP = 0;

			void ReadFrom(void* pDest);
			void WriteTo(void* pDest) const;
		};

		/// <summary>
		/// 좌표 동기화 요청
		/// </summary>
		struct ReqSyncPos
		{
			GameConfig::ACTOR_POS	PosX = 0;
			GameConfig::ACTOR_POS	PosY = 0;

			void ReadFrom(void* pDest);
			void WriteTo(void* pDest) const;
		};

		/// <summary>
		/// 좌표 동기화 알림
		/// </summary>
		struct NtfSyncPos
		{
			GameConfig::ACTOR_ID	ActorID = 0;
			GameConfig::ACTOR_POS	PosX = 0;
			GameConfig::ACTOR_POS	PosY = 0;

			void ReadFrom(void* pDest);
			void WriteTo(void* pDest) const;
		};

		/// <summary>
		/// 에코 요청
		/// </summary>
		struct ReqEcho
		{
			unsigned int Timestamp = 0;

			void ReadFrom(void* pDest);
			void WriteTo(void* pDest) const;
		};

		/// <summary>
		/// 에코 응답
		/// </summary>
		struct ResEcho
		{
			unsigned int Timestamp = 0;

			void ReadFrom(void* pDest);
			void WriteTo(void* pDest) const;
		};
	}
}