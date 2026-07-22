# Echo Server Stress Test
## 테스트 기간
- 2026-07-08 00:00:00 - 2026-07-08 00:00:00 (총 7일)

## 테스트 환경
- OS: Windows Server
- CPU: Intel Core i7 12700KF(8 Core, 20 Threads)
- RAM: DDR4, 64 GBytes
- Client Count: 50
- Disconnect Test: true
- Oversend Count: 100
- Disconnect Delay: 0 sec
- Loop Delay: 0ms

## 테스트 결과

| Case | Sessions | Packet Size | Send Rate | Duration | Description |
|---|---:|---:|---:|---:|---|
| Case 1 | 1,000 | 128 bytes | 1 packet/sec | 5 min | 기본 부하 테스트 |
| Case 2 | 3,000 | 128 bytes | 1 packet/sec | 5 min | 중간 부하 테스트 |
| Case 3 | 5,000 | 128 bytes | 1 packet/sec | 10 min | 목표 세션 수 안정성 테스트 |
| Case 4 | 5,000 | 512 bytes | 1 packet/sec | 10 min | 패킷 크기 증가 테스트 |
| Case 5 | 5,000 | 128 bytes | 5 packets/sec | 10 min | 송신 빈도 증가 테스트 |

## 버그 리포트
### 세션 재사용 중 잘못된 Disconnect 발생

#### 문제
Stress Test 중 클라이언트가 재접속하는 상황에서, 이미 종료된 세션이 아닌 새로 할당된 세션이 Disconnect 되는 문제 발생

#### 원인
원인은 세션 ID 또는 세션 포인터 재사용 시점에 대한 검증이 부족했기 때문이다.

#### 해결

세션에 고유 SessionKey를 추가하고, Disconnect 요청 시 현재 세션의 SessionKey와 일치하는 경우에만 종료하도록 수정