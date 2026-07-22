#include "../NetLib/NetClient.h"

#include "framework.h"
#include "../NetLib/List.h"
#include "../Common/protocol.h"
#include "BaseObject.h"
#include "PlayerObject.h"
#include "EffectObject.h"
#include "ObjectManager.h"
#include "GameNetwork.h"

using namespace cov1013;

extern HWND				g_hWnd;
extern BOOL				g_bConnected;
extern BOOL				g_bExitFlag;
extern DWORD			g_dwSessionID;
extern ObjectManager	g_cObjectManager;
extern PlayerObject*	g_cPlayerCharacter;

//---------------------------------------------------
// 수신 패킷 핸들러 (메인 스레드에서 호출)
//---------------------------------------------------
static void netPacketProc(const protocol::ePacketID PacketID, PacketBuffer* pPacket);
static void netPacketProc_SpawnOwnerActor(PacketBuffer* pPacket);
static void netPacketProc_SpawnActor(PacketBuffer* pPacket);
static void netPacketProc_DespawnActor(PacketBuffer* pPacket);
static void netPacketProc_MoveStart(PacketBuffer* pPacket);
static void netPacketProc_MoveStop(PacketBuffer* pPacket);
static void netPacketProc_Attack(PacketBuffer* pPacket, const int AttackerID, const DWORD dwActionCode);
static void netPacketProc_Attack1(PacketBuffer* pPacket);
static void netPacketProc_Attack2(PacketBuffer* pPacket);
static void netPacketProc_Attack3(PacketBuffer* pPacket);
static void netPacketProc_Damage(PacketBuffer* pPacket);
static void netPacketProc_SyncPos(PacketBuffer* pPacket);

//---------------------------------------------------
// NetClient 래퍼
//
// - IOCP 워커 스레드에서 올라오는 수신 패킷 / 접속·해제 통지를
//   락프리 큐와 플래그에 쌓아두고,
//   메인 스레드가 매 프레임 Process()로 꺼내서 처리한다.
//---------------------------------------------------
class GameNetwork : public NetClient
{
public:
	GameNetwork() : NetClient(eNetworkType::NetNetwork) {};
	virtual ~GameNetwork() { Release(); };

	//---------------------------------------------------
	// 라이브러리 가동 (WSAStartup + IOCP 워커 생성)
	//---------------------------------------------------
	bool Initialize(const NetConfig& Config)
	{
		if (_bStarted == true)
		{
			return true;
		}

		// Net 패킷 인코딩/디코딩 키 설정 (서버와 동일해야 함)
		PacketBuffer::SetPacketCode(Config.GetPacketCode());
		PacketBuffer::SetPacketKey(Config.GetPacketKey());

		if (NetClient::Start(Config) == false)
		{
			return false;
		}

		_bStarted = true;
		return true;
	}

	void Release()
	{
		if (_bStarted == false)
		{
			return;
		}

		// 접속 해제 + 진행중인 IO 완료 대기
		NetClient::Quit();

		// 수신 큐에 남은 패킷 반환
		while (_recvQueue.GetCapacity() > 0)
		{
			PacketBuffer* pPacket = nullptr;
			_recvQueue.Dequeue(&pPacket);
			if (pPacket != nullptr)
			{
				pPacket->DecreseRefCount();
			}
		}

		_bStarted = false;
	}

	//---------------------------------------------------
	// 서버 접속 (동기) - 성공 시 다음 Process()에서 접속 통지 발화
	//---------------------------------------------------
	bool Connect()
	{
		if (_bStarted == false)
		{
			return false;
		}

		if (NetClient::Reconnect() == false)
		{
			// connect에 실패한 소켓은 재사용할 수 없으므로 정리해서
			// 다음 시도에서 새 소켓을 생성하도록 한다.
			NetClient::Disconnect();
			_connectEvent = 0;
			_disconnectEvent = 0;
			return false;
		}

		return true;
	}

	//---------------------------------------------------
	// 메인 스레드에서 매 프레임 호출
	//---------------------------------------------------
	void Process()
	{
		//---------------------------------------------------
		// 1. 접속 완료 통지
		//---------------------------------------------------
		if (_InterlockedExchange(&_connectEvent, 0) != 0)
		{
			g_bConnected = TRUE;
			wprintf(L"%s:%d connect completed\n", _config.GetIP().c_str(), _config.GetPort());
		}

		//---------------------------------------------------
		// 2. 수신 패킷 처리
		//---------------------------------------------------
		while (_recvQueue.GetCapacity() > 0)
		{
			PacketBuffer* pPacket = nullptr;
			_recvQueue.Dequeue(&pPacket);
			if (pPacket == nullptr)
			{
				continue;
			}

			if (g_bConnected)
			{
				protocol::ePacketID packetId;
				*pPacket >> packetId;

				netPacketProc(packetId, pPacket);
			}

			pPacket->DecreseRefCount();
		}

		//---------------------------------------------------
		// 3. 접속 해제 통지
		//---------------------------------------------------
		if (_InterlockedExchange(&_disconnectEvent, 0) != 0)
		{
			const BOOL bWasConnected = g_bConnected;
			g_bConnected = FALSE;
			g_bExitFlag = TRUE;

			if (bWasConnected == TRUE)
			{
				MessageBox(g_hWnd, L"접속이 종료되었습니다.", L"끊겼지롱", MB_OK);
			}
		}
	}

protected:
	//---------------------------------------------------
	// NetClient 이벤트 (OnRecv/OnLeaveServer는 IOCP 워커 스레드에서 호출될 수 있음)
	//---------------------------------------------------
	virtual void __stdcall OnEnterJoinServer() override
	{
		_InterlockedExchange(&_connectEvent, 1);
	}

	virtual void __stdcall OnLeaveServer() override
	{
		_InterlockedExchange(&_disconnectEvent, 1);
	}

	virtual void __stdcall OnRecv(PacketBuffer* pRecvPacket) override
	{
		// 메인 스레드에서 처리하도록 큐잉
		pRecvPacket->IncreseRefCount();
		_recvQueue.Enqueue(pRecvPacket);
	}

	virtual void __stdcall OnSend(const DWORD dwTransferred) override {}
	virtual void __stdcall OnWorkerThreadBegin() override {}
	virtual void __stdcall OnWorkerThreadEnd() override {}
	virtual void __stdcall OnError(const eErrorCode eErrCode, const wchar_t* Message) override
	{
		LOG(Logger::eLogLevel::Error, L"[GameNetwork] Error (Code:%d, Message:%s)\n", eErrCode, Message);
	}

private:
	bool			_bStarted = false;

	// IOCP 워커 -> 메인 스레드 통지
	volatile LONG	_connectEvent = 0;
	volatile LONG	_disconnectEvent = 0;
	cov1013::ConcurrentQueue<cov1013::PacketBuffer*>	_recvQueue;
};

static GameNetwork g_Network;

//---------------------------------------------------
// 네트워크 가동 / 정리 / 프레임 처리
//---------------------------------------------------
void netStartUp(void)
{
	//---------------------------------------
	// 설정 로드 (XML)
	//---------------------------------------
	NetConfig netConfig;
	if (netConfig.Load(L"..\\Config\\GameClient.xml") == false)
	{
		MessageBox(g_hWnd, L"Failure Load GameClient.xml\n(..\\Config\\GameClient.xml)", L"GameClient", MB_OK | MB_ICONERROR);
		g_bExitFlag = TRUE;
		return;
	}

	//---------------------------------------
	// NetLib 내부 로그 초기화
	//---------------------------------------
	Logger::Initialize(netConfig.GetLogDirectoryPath().c_str(), Logger::eLogLevel::System);

	//---------------------------------------
	// 네트워크 가동 + 서버 접속
	//---------------------------------------
	if (g_Network.Initialize(netConfig) == false)
	{
		LOG(Logger::eLogLevel::Error, L"netStartUp() > Network Initialize Failed\n");
		g_bExitFlag = TRUE;
		return;
	}

	if (g_Network.Connect() == false)
	{
		LOG(Logger::eLogLevel::Error, L"netStartUp() > Connect Failed (%s:%d)\n", netConfig.GetIP().c_str(), netConfig.GetPort());
		MessageBox(g_hWnd, L"서버 접속 실패", L"GameClient", MB_OK | MB_ICONERROR);
		g_bExitFlag = TRUE;
		return;
	}
}

void netCleanUp(void)
{
	g_Network.Release();
}

void netProcess(void)
{
	g_Network.Process();
}

//---------------------------------------------------
// 게임 컨텐츠 패킷 송신
//---------------------------------------------------
void netSendMoveStart(const eActorDirection iDir, const int iX, const int iY)
{
	if (!g_bConnected)
	{
		return;
	}

	// 서버가 좌표를 시뮬레이션하므로 방향만 송신한다.
	protocol::ReqMoveStart sendData;
	sendData.Direction = iDir;

	PacketBuffer* pSendPacket = PacketBuffer::AllocNetPacket();
	{
		sendData.WriteTo(pSendPacket);
		g_Network.SendPacket(pSendPacket);
	}
	pSendPacket->DecreseRefCount();
}

void netSendMoveStop(const eActorDirection iDir, const int iX, const int iY)
{
	if (!g_bConnected)
	{
		return;
	}

	protocol::ReqMoveStop sendData;
	sendData.Direction = static_cast<eActorDirection>(iDir);

	PacketBuffer* pSendPacket = PacketBuffer::AllocNetPacket();
	{
		sendData.WriteTo(pSendPacket);
		g_Network.SendPacket(pSendPacket);
	}
	pSendPacket->DecreseRefCount();
}

//---------------------------------------------------
// 공격 송신 (공통)
//
// - 공격 방향(iDir) / 내 좌표(iX,iY) 기준으로 사거리 안의 타깃을 골라
//   AttackeeID 를 실어 보낸다. (타깃이 없으면 0 = 헛스윙)
//---------------------------------------------------
static void netSendAttack(const eActorDirection iDir, const int iX, const int iY, const int RangeX, const int RangeY,
	void (*WriteAttack)(PacketBuffer*, const int))
{
	if (!g_bConnected)
	{
		return;
	}

	// iDir 은 좌/우로 정규화된 방향 (LL or RR)
	const bool bFacingLeft = (iDir == eActorDirection::Left || iDir == eActorDirection::LeftUp || iDir == eActorDirection::LeftDown);
	const int AttackeeID = g_cObjectManager.FindAttackTarget(iX, iY, bFacingLeft, RangeX, RangeY, static_cast<int>(g_dwSessionID));

	PacketBuffer* pSendPacket = PacketBuffer::AllocNetPacket();
	{
		WriteAttack(pSendPacket, AttackeeID);
		g_Network.SendPacket(pSendPacket);
	}
	pSendPacket->DecreseRefCount();
}

static void WriteAttack1(PacketBuffer* pPacket, const int AttackeeID)
{
	protocol::ReqAttack1 sendData;
	sendData.AttackeeID = AttackeeID;
	sendData.WriteTo(pPacket);
}

static void WriteAttack2(PacketBuffer* pPacket, const int AttackeeID)
{
	protocol::ReqAttack2 sendData;
	sendData.AttackeeID = AttackeeID;
	sendData.WriteTo(pPacket);
}

static void WriteAttack3(PacketBuffer* pPacket, const int AttackeeID)
{
	protocol::ReqAttack3 sendData;
	sendData.AttackeeID = AttackeeID;
	sendData.WriteTo(pPacket);
}

void netSendAttack1(const eActorDirection iDir, const int iX, const int iY)
{
	netSendAttack(iDir, iX, iY,
		static_cast<int>(eAttackRactengle::Attack1PosX),
		static_cast<int>(eAttackRactengle::Attack1PosY),
		WriteAttack1);
}

void netSendAttack2(const eActorDirection iDir, const int iX, const int iY)
{
	netSendAttack(iDir, iX, iY,
		static_cast<int>(eAttackRactengle::Attack2PosX),
		static_cast<int>(eAttackRactengle::Attack2PosY),
		WriteAttack2);
}

void netSendAttack3(const eActorDirection iDir, const int iX, const int iY)
{
	netSendAttack(iDir, iX, iY,
		static_cast<int>(eAttackRactengle::Attack3PosX),
		static_cast<int>(eAttackRactengle::Attack3PosY),
		WriteAttack3);
}

//---------------------------------------------------
// 수신 패킷 타입별 분기
//---------------------------------------------------
static void netPacketProc(const protocol::ePacketID PacketID, PacketBuffer* pPacket)
{
	switch (PacketID)
	{
	case protocol::ePacketID::NtfSpawnOwnerActor:
		netPacketProc_SpawnOwnerActor(pPacket);
		break;

	case protocol::ePacketID::NtfSpawnActor:
		netPacketProc_SpawnActor(pPacket);
		break;

	case protocol::ePacketID::NtfDespawnActor:
		netPacketProc_DespawnActor(pPacket);
		break;

	case protocol::ePacketID::NtfMoveStart:
		netPacketProc_MoveStart(pPacket);
		break;

	case protocol::ePacketID::NtfMoveStop:
		netPacketProc_MoveStop(pPacket);
		break;

	case protocol::ePacketID::NtfAttack1:
		netPacketProc_Attack1(pPacket);
		break;

	case protocol::ePacketID::NtfAttack2:
		netPacketProc_Attack2(pPacket);
		break;

	case protocol::ePacketID::NtfAttack3:
		netPacketProc_Attack3(pPacket);
		break;

	case protocol::ePacketID::NtfDamage:
		netPacketProc_Damage(pPacket);
		break;

	case protocol::ePacketID::NtfSyncPos:
		netPacketProc_SyncPos(pPacket);
		break;

	default:
		LOG(Logger::eLogLevel::Error, L"netPacketProc() > Unknown PacketID:%d\n", static_cast<int>(PacketID));
		break;
	}
}

//---------------------------------------------------
// 내 캐릭터(Owner) 스폰
//---------------------------------------------------
static void netPacketProc_SpawnOwnerActor(PacketBuffer* pPacket)
{
	protocol::NtfSpawnOwnerActor recvData;
	recvData.ReadFrom(pPacket);

	if (recvData.Direction >= eActorDirection::End)
	{
		LOG(Logger::eLogLevel::Error, L"PacketProc SpawnOwnerActor > Invalid Direction:%d\n", static_cast<int>(recvData.Direction));
		return;
	}

	g_dwSessionID = static_cast<DWORD>(recvData.ActorID);

	PlayerObject* pPlayer = new PlayerObject(TRUE, recvData.ActorID, static_cast<int>(recvData.Direction),
		recvData.PosX, recvData.PosY, static_cast<BYTE>(recvData.HP));
	g_cObjectManager.AddObject(pPlayer);
	g_cPlayerCharacter = pPlayer;
}

//---------------------------------------------------
// 다른 캐릭터 스폰
//---------------------------------------------------
static void netPacketProc_SpawnActor(PacketBuffer* pPacket)
{
	protocol::NtfSpawnActor recvData;
	recvData.ReadFrom(pPacket);

	if (recvData.Direction >= eActorDirection::End)
	{
		LOG(Logger::eLogLevel::Error, L"PacketProc SpawnActor > Invalid Direction:%d\n", static_cast<int>(recvData.Direction));
		return;
	}

	PlayerObject* pPlayer = new PlayerObject(FALSE, recvData.ActorID, static_cast<int>(recvData.Direction),
		recvData.PosX, recvData.PosY, static_cast<BYTE>(recvData.HP));
	g_cObjectManager.AddObject(pPlayer);

	// 이미 걷고 있던 캐릭터라면 서버의 NtfMoveStart 로 곧 이동 애니메이션이 갱신된다.
	pPlayer->SetActionMove();
}

//---------------------------------------------------
// 캐릭터 디스폰
//---------------------------------------------------
static void netPacketProc_DespawnActor(PacketBuffer* pPacket)
{
	protocol::NtfDespawnActor recvData;
	recvData.ReadFrom(pPacket);

	PlayerObject* pPlayer = (PlayerObject*)g_cObjectManager.FindObject(recvData.ActorID);
	if (NULL == pPlayer)
	{
		LOG(Logger::eLogLevel::Error, L"PacketProc DespawnActor > ActorID:%d Not Found\n", recvData.ActorID);
		return;
	}

	g_cObjectManager.DeleteObject(recvData.ActorID);
}

//---------------------------------------------------
// 캐릭터 이동 시작
//---------------------------------------------------
static void netPacketProc_MoveStart(PacketBuffer* pPacket)
{
	protocol::NtfMoveStart recvData;
	recvData.ReadFrom(pPacket);

	if (recvData.Direction > eActorDirection::LeftDown)
	{
		LOG(Logger::eLogLevel::Error, L"PacketProc MoveStart > Invalid Direction:%d\n", static_cast<int>(recvData.Direction));
		return;
	}

	PlayerObject* pPlayer = (PlayerObject*)g_cObjectManager.FindObject(recvData.ActorID);
	if (NULL == pPlayer)
	{
		LOG(Logger::eLogLevel::Error, L"PacketProc MoveStart > ActorID:%d Not Found\n", recvData.ActorID);
		return;
	}

	// 좌표는 NtfSyncPos 로 보정되므로 여기선 액션(방향)만 입력한다.
	pPlayer->ActionInput(static_cast<DWORD>(recvData.Direction));
}

//---------------------------------------------------
// 캐릭터 이동 정지
//---------------------------------------------------
static void netPacketProc_MoveStop(PacketBuffer* pPacket)
{
	protocol::NtfMoveStop recvData;
	recvData.ReadFrom(pPacket);

	if (recvData.Direction > eActorDirection::LeftDown)
	{
		LOG(Logger::eLogLevel::Error, L"PacketProc MoveStop > Invalid Direction:%d\n", static_cast<int>(recvData.Direction));
		return;
	}

	PlayerObject* pPlayer = (PlayerObject*)g_cObjectManager.FindObject(recvData.ActorID);
	if (NULL == pPlayer)
	{
		LOG(Logger::eLogLevel::Error, L"PacketProc MoveStop > ActorID:%d Not Found\n", recvData.ActorID);
		return;
	}

	// 정지 방향으로 바라보게 한 뒤 정지 애니메이션 입력
	pPlayer->SetCurDirection(static_cast<int>(recvData.Direction));
	pPlayer->ActionInput(dfACTION_STAND);
}

//---------------------------------------------------
// 캐릭터 공격 (공통)
//---------------------------------------------------
static void netPacketProc_Attack(PacketBuffer* pPacket, const int AttackerID, const DWORD dwActionCode)
{
	PlayerObject* pPlayer = (PlayerObject*)g_cObjectManager.FindObject(AttackerID);
	if (NULL != pPlayer)
	{
		// Update 부분에서 적용될 액션 코드 셋팅
		pPlayer->ActionInput(dwActionCode);
	}
}

static void netPacketProc_Attack1(PacketBuffer* pPacket)
{
	protocol::NtfAttack1 recvData;
	recvData.ReadFrom(pPacket);
	netPacketProc_Attack(pPacket, recvData.AttackerID, dfACTION_ATTACK1);
}

static void netPacketProc_Attack2(PacketBuffer* pPacket)
{
	protocol::NtfAttack2 recvData;
	recvData.ReadFrom(pPacket);
	netPacketProc_Attack(pPacket, recvData.AttackerID, dfACTION_ATTACK2);
}

static void netPacketProc_Attack3(PacketBuffer* pPacket)
{
	protocol::NtfAttack3 recvData;
	recvData.ReadFrom(pPacket);
	netPacketProc_Attack(pPacket, recvData.AttackerID, dfACTION_ATTACK3);
}

//---------------------------------------------------
// 데미지 (피격자 HP 갱신 + 피격 이펙트)
//---------------------------------------------------
static void netPacketProc_Damage(PacketBuffer* pPacket)
{
	protocol::NtfDamage recvData;
	recvData.ReadFrom(pPacket);

	PlayerObject* pDamagePlayer = (PlayerObject*)g_cObjectManager.FindObject(recvData.ActorID);
	if (NULL == pDamagePlayer)
	{
		// 섹터에서 벗어나 있는 등 클라이언트가 모르는 액터일 수 있다.
		return;
	}

	// 피격 이펙트 생성 및 등록
	EffectObject* pNewEffect = new EffectObject(pDamagePlayer);
	g_cObjectManager.AddObject(pNewEffect);
	pDamagePlayer->OnEffectFlag();

	// 남은 HP 반영
	pDamagePlayer->SetHP(static_cast<BYTE>(recvData.ActorHP));
}

//---------------------------------------------------
// 좌표 동기화 (서버 권위 좌표로 스냅)
//---------------------------------------------------
static void netPacketProc_SyncPos(PacketBuffer* pPacket)
{
	protocol::NtfSyncPos recvData;
	recvData.ReadFrom(pPacket);

	PlayerObject* pCharacter = (PlayerObject*)g_cObjectManager.FindObject(recvData.ActorID);
	if (NULL == pCharacter)
	{
		return;
	}

	pCharacter->SetPosition(recvData.PosX, recvData.PosY);
}
