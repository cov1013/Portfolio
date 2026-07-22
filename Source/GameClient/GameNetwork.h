#pragma once

//---------------------------------------------------
// GameClient 네트워크 계층 (NetLib NetClient 기반)
//
// - 구현은 GameNetwork.cpp 하나에 통합
//   (NetClient 래퍼 / 수신 패킷 디스패치 / 패킷 핸들러 / 송신 함수)
// - NetLib 의존은 GameNetwork.cpp 내부에만 둔다.
//---------------------------------------------------

//---------------------------------------------------
// 네트워크 가동
// 설정 로드(..\Config\GameClient.ini) 후 서버 접속, 실패 시 g_bExitFlag 설정
//---------------------------------------------------
void netStartUp(void);

//---------------------------------------------------
// 네트워크 정리 (접속 해제 + IO 완료 대기)
//---------------------------------------------------
void netCleanUp(void);

//---------------------------------------------------
// 메인 루프에서 매 프레임 호출
// 수신 패킷 / 접속 이벤트를 메인 스레드에서 처리
//---------------------------------------------------
void netProcess(void);

//---------------------------------------------------
// 게임 컨텐츠 패킷 송신 (Client -> Server)
//---------------------------------------------------
void netSendMoveStart(const eActorDirection iDir, const int iX, const int iY);
void netSendMoveStop(const eActorDirection iDir, const int iX, const int iY);
void netSendAttack1(const eActorDirection iDir, const int iX, const int iY);
void netSendAttack2(const eActorDirection iDir, const int iX, const int iY);
void netSendAttack3(const eActorDirection iDir, const int iX, const int iY);
