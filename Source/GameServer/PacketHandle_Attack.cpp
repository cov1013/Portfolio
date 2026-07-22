#include "../Common/Protocol_Game.h"
#include "../Common/Actor.h"
#include "../Common/ActorManager.h"
#include "../Common/SectorManager.h"
#include "../Common/GameEnum.h"
#include "../Common/GameConfig.h"
#include "../NetLib/NetDevice.h"
#include "GameServer.h"
#include "UpdateThread.h"

bool UpdateThread::OnReceivePacket_Attack(Actor* pActor, PacketBuffer* pPacket, const int AttackType)
{
	//// 모든 ReqAttack* 는 동일 레이아웃(int AttackeeID)
	//ReqAttack1 recvData;
	//recvData.ReadFrom(pPacket);
	//const int AttackeeID = recvData.AttackeeID;

	//// 공격 타입별 판정 범위 / 데미지
	//int RangeX = 0;
	//int RangeY = 0;
	//int Damage = 0;
	//switch (AttackType)
	//{
	//case 1:
	//	RangeX = static_cast<int>(eAttackRactengle::Attack1PosX);
	//	RangeY = static_cast<int>(eAttackRactengle::Attack1PosY);
	//	Damage = static_cast<int>(eAttackDamage::Attack1);
	//	break;
	//case 2:
	//	RangeX = static_cast<int>(eAttackRactengle::Attack2PosX);
	//	RangeY = static_cast<int>(eAttackRactengle::Attack2PosY);
	//	Damage = static_cast<int>(eAttackDamage::Attack2);
	//	break;
	//case 3:
	//	RangeX = static_cast<int>(eAttackRactengle::Attack3PosX);
	//	RangeY = static_cast<int>(eAttackRactengle::Attack3PosY);
	//	Damage = static_cast<int>(eAttackDamage::Attack3);
	//	break;
	//default:
	//	return false;
	//}

	//// 1) 본인을 제외한 주변에 공격 모션 통지
	//PacketBuffer* pSendPacket = PacketBuffer::AllocNetPacket();
	//{
	//	switch (AttackType)
	//	{
	//	case 1:
	//	{
	//		NtfAttack1 sendData;
	//		sendData.AttackerID = static_cast<int>(pActor->ActorID);
	//		sendData.AttackeeID = AttackeeID;
	//		sendData.WriteTo(pSendPacket);
	//		break;
	//	}
	//	case 2:
	//	{
	//		NtfAttack2 sendData;
	//		sendData.AttackerID = static_cast<int>(pActor->ActorID);
	//		sendData.AttackeeID = AttackeeID;
	//		sendData.WriteTo(pSendPacket);
	//		break;
	//	}
	//	case 3:
	//	{
	//		NtfAttack3 sendData;
	//		sendData.AttackerID = static_cast<int>(pActor->ActorID);
	//		sendData.AttackeeID = AttackeeID;
	//		sendData.WriteTo(pSendPacket);
	//		break;
	//	}
	//	}
	//	_pSectorManager->DoSendMessage(pActor->SectorX, pActor->SectorY, pSendPacket, pActor->SessionID);
	//}
	//pSendPacket->DecreseRefCount();

	//// 2) 타깃이 없으면 모션만 통지 (헛스윙)
	//if (AttackeeID == 0)
	//{
	//	return true;
	//}

	//// 3) 피격자 검증
	//Actor* pTarget = _pActorManager->Get(AttackeeID);
	//if (pTarget == nullptr || pTarget == pActor || pTarget->IsDead() == true)
	//{
	//	return true;
	//}

	//const int DiffX = pTarget->PosX - pActor->PosX;
	//const int DiffY = abs(pTarget->PosY - pActor->PosY);

	//bool bHit;
	//if (pActor->FacingLeft == true)
	//{
	//	bHit = (DiffX <= 0) && (-DiffX <= RangeX) && (DiffY <= RangeY);
	//}
	//else
	//{
	//	bHit = (DiffX >= 0) && (DiffX <= RangeX) && (DiffY <= RangeY);
	//}

	//if (bHit == false)
	//{
	//	return true;
	//}

	//// 4) 데미지 적용 후 피격자 주변에 통지 (공격자 포함)
	//const int RemainHP = pTarget->ApplyDamage(Damage);

	//NtfDamage damageData;
	//damageData.ActorID = static_cast<int>(pTarget->ActorID);
	//damageData.ActorHP = RemainHP;

	//PacketBuffer* pDamagePacket = PacketBuffer::AllocNetPacket();
	//{
	//	damageData.WriteTo(pDamagePacket);
	//	_pSectorManager->DoSendMessage(pTarget->SectorX, pTarget->SectorY, pDamagePacket);
	//}
	//pDamagePacket->DecreseRefCount();

	//// 5) 사망 처리 (접속 종료 -> OnDisconnect 에서 디스폰 통지)
	//if (RemainHP == 0)
	//{
	//	_pServer->Disconnect(pTarget->SessionID);
	//}

	return true;
}