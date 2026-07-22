#include "Protocol_Game.h"
#include "../NetLib/PacketBuffer.h"

namespace protocol
{
	namespace game
	{
		/// <summary>
		/// 엑터 스폰 (Owner)
		/// </summary>
		void NtfSpawnOwnerActor::ReadFrom(void* pDest)
		{
			cov1013::PacketBuffer* pPacket = static_cast<cov1013::PacketBuffer*>(pDest);

			*pPacket >> ActorID;
			*pPacket >> Action;
			*pPacket >> PosX;
			*pPacket >> PosY;
			*pPacket >> HP;
		}

		void NtfSpawnOwnerActor::WriteTo(void* pDest) const
		{
			cov1013::PacketBuffer* pPacket = static_cast<cov1013::PacketBuffer*>(pDest);

			*pPacket << ePacketID::NtfSpawnOwnerActor;
			*pPacket << ActorID;
			*pPacket << Action;
			*pPacket << PosX;
			*pPacket << PosY;
			*pPacket << HP;
		}

		/// <summary>
		/// 엑터 스폰
		/// </summary>
		void NtfSpawnActor::ReadFrom(void* pDest)
		{
			cov1013::PacketBuffer* pPacket = static_cast<cov1013::PacketBuffer*>(pDest);

			*pPacket >> ActorID;
			*pPacket >> Action;
			*pPacket >> PosX;
			*pPacket >> PosY;
			*pPacket >> HP;
		}

		void NtfSpawnActor::WriteTo(void* pDest) const
		{
			cov1013::PacketBuffer* pPacket = static_cast<cov1013::PacketBuffer*>(pDest);

			*pPacket << ePacketID::NtfSpawnActor;
			*pPacket << ActorID;
			*pPacket << Action;
			*pPacket << PosX;
			*pPacket << PosY;
			*pPacket << HP;
		}

		/// <summary>
		/// 엑터 디스폰
		/// </summary>
		/// <param name="pDest"></param>
		void NtfDespawnActor::ReadFrom(void* pDest)
		{
			cov1013::PacketBuffer* pPacket = static_cast<cov1013::PacketBuffer*>(pDest);

			*pPacket >> ActorID;
		}

		void NtfDespawnActor::WriteTo(void* pDest) const
		{
			cov1013::PacketBuffer* pPacket = static_cast<cov1013::PacketBuffer*>(pDest);

			*pPacket << ePacketID::NtfDespawnActor;
			*pPacket << ActorID;
		}

		/// <summary>
		/// 이동 시작 요청
		/// </summary>
		/// <param name="pDest"></param>
		void ReqMoveStart::ReadFrom(void* pDest)
		{
			cov1013::PacketBuffer* pPacket = static_cast<cov1013::PacketBuffer*>(pDest);

			*pPacket >> Action;
		}

		void ReqMoveStart::WriteTo(void* pDest) const
		{
			cov1013::PacketBuffer* pPacket = static_cast<cov1013::PacketBuffer*>(pDest);

			*pPacket << ePacketID::ReqMoveStart;
			*pPacket << Action;
		}

		/// <summary>
		/// 이동 시작 알림
		/// </summary>
		/// <param name="pDest"></param>
		void NtfMoveStart::ReadFrom(void* pDest)
		{
			cov1013::PacketBuffer* pPacket = static_cast<cov1013::PacketBuffer*>(pDest);

			*pPacket >> Action;
			*pPacket >> ActorID;
		}

		void NtfMoveStart::WriteTo(void* pDest) const
		{
			cov1013::PacketBuffer* pPacket = static_cast<cov1013::PacketBuffer*>(pDest);

			*pPacket << ePacketID::NtfMoveStart;
			*pPacket << Action;
			*pPacket << ActorID;
		}

		/// <summary>
		/// 이동 정지 요청
		/// </summary>
		void ReqMoveStop::ReadFrom(void* pDest)
		{
			cov1013::PacketBuffer* pPacket = static_cast<cov1013::PacketBuffer*>(pDest);

			*pPacket >> Action;
		}

		void ReqMoveStop::WriteTo(void* pDest) const
		{
			cov1013::PacketBuffer* pPacket = static_cast<cov1013::PacketBuffer*>(pDest);

			*pPacket << ePacketID::ReqMoveStop;
			*pPacket << Action;
		}

		/// <summary>
		/// 이동 정지 알림
		/// </summary>
		void NtfMoveStop::ReadFrom(void* pDest)
		{
			cov1013::PacketBuffer* pPacket = static_cast<cov1013::PacketBuffer*>(pDest);

			*pPacket >> Action;
			*pPacket >> ActorID;
		}

		void NtfMoveStop::WriteTo(void* pDest) const
		{
			cov1013::PacketBuffer* pPacket = static_cast<cov1013::PacketBuffer*>(pDest);

			*pPacket << ePacketID::NtfMoveStop;
			*pPacket << Action;
			*pPacket << ActorID;
		}

		/// <summary>
		/// 공격1 요청
		/// </summary>
		/// <param name="pDest"></param>
		void ReqAttack1::ReadFrom(void* pDest)
		{
			cov1013::PacketBuffer* pPacket = static_cast<cov1013::PacketBuffer*>(pDest);

			*pPacket >> AttackeeID;
		}

		void ReqAttack1::WriteTo(void* pDest) const
		{
			cov1013::PacketBuffer* pPacket = static_cast<cov1013::PacketBuffer*>(pDest);

			*pPacket << ePacketID::ReqAttack1;
			*pPacket << AttackeeID;
		}

		/// <summary>
		/// 공격1 알림
		/// </summary>
		/// <param name="pDest"></param>
		void NtfAttack1::ReadFrom(void* pDest)
		{
			cov1013::PacketBuffer* pPacket = static_cast<cov1013::PacketBuffer*>(pDest);

			*pPacket >> AttackerID;
			*pPacket >> AttackeeID;
		}

		void NtfAttack1::WriteTo(void* pDest) const
		{
			cov1013::PacketBuffer* pPacket = static_cast<cov1013::PacketBuffer*>(pDest);

			*pPacket << ePacketID::NtfAttack1;
			*pPacket << AttackerID;
			*pPacket << AttackeeID;
		}

		/// <summary>
		/// 공격2 요청
		/// </summary>
		void ReqAttack2::ReadFrom(void* pDest)
		{
			cov1013::PacketBuffer* pPacket = static_cast<cov1013::PacketBuffer*>(pDest);

			*pPacket >> AttackeeID;
		}

		void ReqAttack2::WriteTo(void* pDest) const
		{
			cov1013::PacketBuffer* pPacket = static_cast<cov1013::PacketBuffer*>(pDest);

			*pPacket << ePacketID::ReqAttack2;
			*pPacket << AttackeeID;
		}

		/// <summary>
		/// 공격2 알림
		/// </summary>
		void NtfAttack2::ReadFrom(void* pDest)
		{
			cov1013::PacketBuffer* pPacket = static_cast<cov1013::PacketBuffer*>(pDest);

			*pPacket >> AttackerID;
			*pPacket >> AttackeeID;
		}

		void NtfAttack2::WriteTo(void* pDest) const
		{
			cov1013::PacketBuffer* pPacket = static_cast<cov1013::PacketBuffer*>(pDest);

			*pPacket << ePacketID::NtfAttack2;
			*pPacket << AttackerID;
			*pPacket << AttackeeID;
		}

		/// <summary>
		/// 공격3 요청
		/// </summary>
		void ReqAttack3::ReadFrom(void* pDest)
		{
			cov1013::PacketBuffer* pPacket = static_cast<cov1013::PacketBuffer*>(pDest);

			*pPacket >> AttackeeID;
		}

		void ReqAttack3::WriteTo(void* pDest) const
		{
			cov1013::PacketBuffer* pPacket = static_cast<cov1013::PacketBuffer*>(pDest);

			*pPacket << ePacketID::ReqAttack3;
			*pPacket << AttackeeID;
		}

		/// <summary>
		/// 공격3 알림
		/// </summary>
		void NtfAttack3::ReadFrom(void* pDest)
		{
			cov1013::PacketBuffer* pPacket = static_cast<cov1013::PacketBuffer*>(pDest);

			*pPacket >> AttackerID;
			*pPacket >> AttackeeID;
		}

		void NtfAttack3::WriteTo(void* pDest) const
		{
			cov1013::PacketBuffer* pPacket = static_cast<cov1013::PacketBuffer*>(pDest);

			*pPacket << ePacketID::NtfAttack3;
			*pPacket << AttackerID;
			*pPacket << AttackeeID;
		}

		/// <summary>
		/// 데미지 알림
		/// </summary>
		/// <param name="pDest"></param>
		void NtfDamage::ReadFrom(void* pDest)
		{
			cov1013::PacketBuffer* pPacket = static_cast<cov1013::PacketBuffer*>(pDest);

			*pPacket >> ActorID;
			*pPacket >> ActorHP;
		}

		void NtfDamage::WriteTo(void* pDest) const
		{
			cov1013::PacketBuffer* pPacket = static_cast<cov1013::PacketBuffer*>(pDest);

			*pPacket << ePacketID::NtfDamage;
			*pPacket << ActorID;
			*pPacket << ActorHP;
		}

		/// <summary>
		/// 좌표 동기화 요청
		/// </summary>
		/// <param name="pDest"></param>
		void ReqSyncPos::ReadFrom(void* pDest)
		{
			cov1013::PacketBuffer* pPacket = static_cast<cov1013::PacketBuffer*>(pDest);

			*pPacket >> PosX;
			*pPacket >> PosY;
		}

		void ReqSyncPos::WriteTo(void* pDest) const
		{
			cov1013::PacketBuffer* pPacket = static_cast<cov1013::PacketBuffer*>(pDest);

			*pPacket << ePacketID::ReqSyncPos;
			*pPacket << PosX;
			*pPacket << PosY;
		}

		/// <summary>
		/// 좌표 동기화 알림
		/// </summary>
		/// <param name="pDest"></param>
		void NtfSyncPos::ReadFrom(void* pDest)
		{
			cov1013::PacketBuffer* pPacket = static_cast<cov1013::PacketBuffer*>(pDest);

			*pPacket >> ActorID;
			*pPacket >> PosX;
			*pPacket >> PosY;
		}

		void NtfSyncPos::WriteTo(void* pDest) const
		{
			cov1013::PacketBuffer* pPacket = static_cast<cov1013::PacketBuffer*>(pDest);

			*pPacket << ePacketID::NtfSyncPos;
			*pPacket << ActorID;
			*pPacket << PosX;
			*pPacket << PosY;
		}

		/// <summary>
		/// 에코 요청
		/// </summary>
		/// <param name="pDest"></param>
		void ReqEcho::ReadFrom(void* pDest)
		{
			cov1013::PacketBuffer* pPacket = static_cast<cov1013::PacketBuffer*>(pDest);

			*pPacket >> Timestamp;
		}

		void ReqEcho::WriteTo(void* pDest) const
		{
			cov1013::PacketBuffer* pPacket = static_cast<cov1013::PacketBuffer*>(pDest);

			*pPacket << ePacketID::ReqEcho;
			*pPacket << Timestamp;
		}

		/// <summary>
		/// 에코 응답
		/// </summary>
		/// <param name="pDest"></param>
		void ResEcho::ReadFrom(void* pDest)
		{
			cov1013::PacketBuffer* pPacket = static_cast<cov1013::PacketBuffer*>(pDest);

			*pPacket >> Timestamp;
		}

		void ResEcho::WriteTo(void* pDest) const
		{
			cov1013::PacketBuffer* pPacket = static_cast<cov1013::PacketBuffer*>(pDest);

			*pPacket << ePacketID::ResEcho;
			*pPacket << Timestamp;
		}
	}
}