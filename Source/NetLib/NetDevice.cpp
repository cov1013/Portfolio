#include "NetDevice.h"

namespace cov1013
{
	NetDevice::NetDevice(const NetConfig& inNetConfig)
	{
		_netConfig = inNetConfig;
	}

	NetDevice::~NetDevice()
	{
		cov1013::Profiler::Release();
		cov1013::PacketFactory::Release();
	}

	bool NetDevice::Start()
	{
		if (Logger::Initialize(_netConfig.LogDirectoryPath.c_str(), _netConfig.LogLevel) == false)
		{
			return 0;
		}

		if (CrashDumper::Initialize(_netConfig.DumpDirectoryPath.c_str()) == false)
		{
			return 0;
		}

		if (Profiler::Initialize(_netConfig.ProfilingDataDirectoryPath.c_str(), _netConfig.ProfilerUnit) == false)
		{
			return 0;
		}

#ifndef _TLS_MODE_
		PacketFactory::Initialize(_netConfig.PacketKey, _netConfig.PacketCode);
#else
		PacketFactory::Initialize(_netConfig.PacketKey, _netConfig.PacketCode, _netConfig.PacketChunkElementCount);
#endif

		if (InitializeNetwork() == false)
		{
			return false;
		}

		if (InitializeSocket() == false)
		{
			return false;
		}

		if (InitializeSessions() == false)
		{
			return false;
		}

		if (InitializeIocpWorker() == false)
		{
			return false;
		}

		if(_netConfig.IsServer())
		{
			if (InitializeAcceptor() == false)
			{
				return false;
			}
		}

		return true;
	}

	void NetDevice::Stop()
	{
		LOG(eLogLevel::System, L"[%s] Begin\n", __FUNCTIONW__);

		if(_netConfig.IsServer())
		{
			// 1) 신규 세션 접속 못받게 문 닫고
			ReleaseAcceptor();

			// 2) 현재 전체 세션 접속 끊는다.
			DisconnectAllSession();
		}

		LOG(eLogLevel::System, L"[%s] End\n", __FUNCTIONW__);
	}

	void NetDevice::Quit()
	{
		LOG(eLogLevel::System, L"[%s] Begin\n", __FUNCTIONW__);

		if (_netConfig.IsServer())
		{
			// 1) 신규 세션 접속 못받게 문 닫고
			ReleaseAcceptor();

			// 2) 전체 세션 접속 끊고
			DisconnectAllSession();

			// 3) IOCP Worker 종료
			ReleaseIocpWorker();

			// 4) 세션 메모리 반환
			ReleaseSessions();

			// 5) 네트워크 종료
			ReleaseNetwork();
		}

		LOG(eLogLevel::System, L"[%s] End\n", __FUNCTIONW__);
	}

	bool NetDevice::Restart()
	{
		LOG(eLogLevel::System, L"[%s] Begin\n", __FUNCTIONW__);

		if (_netConfig.IsServer())
		{
			// 1) 문 만들고
			if (InitializeSocket() == false)
			{
				return false;
			}

			// 2) 문 연다
			InitializeAcceptor();
		}

		LOG(eLogLevel::System, L"[%s] End\n", __FUNCTIONW__);

		return true;
	}

	bool NetDevice::Connect()
	{
		LOG(eLogLevel::System, L"[%s] Begin\n", __FUNCTIONW__);

		SOCKADDR_IN sockAddr = {};
		sockAddr.sin_family = AF_INET;
		sockAddr.sin_port = htons(static_cast<u_short>(_netConfig.Port));
		InetPtonW(AF_INET, _netConfig.IP.c_str(), &sockAddr.sin_addr);

		if (connect(_socket, (SOCKADDR*)&sockAddr, sizeof(sockAddr)) == SOCKET_ERROR)
		{
			LOG(eLogLevel::Error, L"[%s] Connect Failed (ErrCode:%d)\n", __FUNCTIONW__, WSAGetLastError());
			return false;
		}

		// 세션 인덱스 획득
		NetConfig::SESSION_INDEX sessionIndex;
		if (_sessionIndexes.Pop(&sessionIndex) == false)
		{
			closesocket(_socket);
			OnError(eErrorCode::NotExistBlankSessionIndex);
			return false;
		}

		// 세션 생성
		NewSession(_socket, sessionIndex, _netConfig.IP.c_str(), static_cast<u_short>(_netConfig.Port));

		LOG(eLogLevel::System, L"[%s] Success Connect. (IP:%s, Port:%d)\n", __FUNCTIONW__, _netConfig.IP.c_str(), static_cast<u_short>(_netConfig.Port));
		LOG(eLogLevel::System, L"[%s] End\n", __FUNCTIONW__);

		return true;
	}

	bool NetDevice::Reconnect()
	{
		LOG(eLogLevel::System, L"[%s] Begin\n", __FUNCTIONW__);

		if (_netConfig.IsClient())
		{
			if (InitializeSocket() == false)
			{
				goto Error;
			}

			if (Connect() == false)
			{
				goto Error;
			}
		}

		LOG(eLogLevel::System, L"[%s] End\n", __FUNCTIONW__);
		return true;

	Error:
		LOG(eLogLevel::System, L"[%s] End\n", __FUNCTIONW__);
		return false;
	}

	void NetDevice::Control(const wchar_t InputKey)
	{
		switch (InputKey)
		{
		case L's':
		case L'S':
			NetDevice::Stop();
			break;

		case L'r':
		case L'R':
			NetDevice::Restart();
			break;

		case L'p':
		case L'P':
			Profiler::Output();
			break;

		case L'1':
			Logger::SetLogLevel(eLogLevel::Debug);
			break;

		case L'2':
			Logger::SetLogLevel(eLogLevel::Error);
			break;

		case L'3':
			Logger::SetLogLevel(eLogLevel::System);
			break;

		case L'0':
			Logger::SetLogLevel(eLogLevel::None);
			break;

		default:
			break;
		}
	}

	bool NetDevice::SendPacket(const NetConfig::SESSION_ID SessionID, PacketBuffer* pPacket)
	{
		// 1) 세션 획득
		NetSession* pSession = GetSession(SessionID);
		if (pSession == nullptr)
		{
			return false;
		}

		// 2) 패킷 헤더 세팅
		switch (_netConfig.NetDeviceType)
		{
			case eNetDeviceType::LanServer:
			case eNetDeviceType::LanClient:
				pPacket->SetLanPacketHeader();
				break;

			case eNetDeviceType::NetServer:
			case eNetDeviceType::NetClient:
				pPacket->SetNetPacketHeader();
				break;
		}

		// 3) 패킷 송신 버퍼에 넣기
		pPacket->IncreseRefCount();
		pSession->SendBuffer.Enqueue(pPacket);

		// 4) 송신 등록
		if (_netConfig.EnableIOCPWorkerSend == false)
		{
			SendPost(pSession);

			// 세션 획득하면서 올려놓은 IOCount 차감
			if (_InterlockedDecrement16(&pSession->IOCount) == 0)
			{
				ReleaseSession(pSession);
			}
		}
		else
		{
			// WSASend 함수 호출을 IOCP Worekr에서 진행하게 전달
			PostQueuedCompletionStatus(
				_iocp, 
				static_cast<DWORD>(eIOCPWorkerJobType::OnSendPacket), 
				reinterpret_cast<ULONG_PTR>(pSession),
				nullptr);
		}

		return true;
	}

	bool NetDevice::Disconnect(const NetConfig::SESSION_ID SessionID)
	{
		// 세션 획득
		NetSession* pSession = GetSession(SessionID);
		if (pSession == nullptr)
		{
			return false;
		}

		// I/O 등록안되게 플래그로 막고
		pSession->bDisconnectFlag = true;

		// 이미 등록된 I/O 취소 요청
		CancelIoEx((HANDLE)pSession->Socket, NULL);

		// 세션 획득하면서 올려놓은 IOCount 차감
		// CancelIO 함수 호출하면 IocpWorker 깨어나서 연결 종료 처리 진행하는데,
		// 현재 스레드에서 올려 놓은 IOCount로 인해 Release가 진행 안될 수 있으므로, 여기서 IOCount를 다시 체크한다.
		if (_InterlockedDecrement16(&pSession->IOCount) == 0)
		{
			ReleaseSession(pSession);
		}

		return true;
	}

	void NetDevice::ResetNetStatistics()
	{
		_netStatistics.AcceptTPS = 0;
		_netStatistics.DisconnectTPS = 0;
		_netStatistics.RecvPacketTPS = 0;
		_netStatistics.SendPacketTPS = 0;
		_netStatistics.ExpSendBytes = 0;
	}

	bool NetDevice::InitializeNetwork()
	{
		WSADATA	wsaData;
		const int Result = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (Result != 0)
		{
			const auto ErrCode = WSAGetLastError();
			LOG(eLogLevel::Error, L"[%s] Failed to WSAStartup. (ErrCode:%ld)\n", __FUNCTIONW__, ErrCode);
			return false;
		}

		LOG(eLogLevel::System, L"[%s] Success WSAStartup.\n", __FUNCTIONW__);
		return true;
	}

	bool NetDevice::InitializeSocket()
	{
		if (_socket != INVALID_SOCKET)
		{
			LOG(eLogLevel::Error, L"[%s] Already Created Socket.\n", __FUNCTIONW__);
			return false;
		}

		// 1) Socket 생성
		{
			_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			if (_socket == INVALID_SOCKET)
			{
				LOG(eLogLevel::Error, L"[%s] Failed Create Socket.(ErrCode:%ld)\n", __FUNCTIONW__, WSAGetLastError());
				return false;
			}
			LOG(eLogLevel::System, L"[%s] Success Create Socket.\n", __FUNCTIONW__);
		}

		// 2) Listen Socket 세팅
		{
			// 네이글 알고리즘 세팅
			if (_netConfig.EnableTCPNagle == false)
			{
				bool optval = true;
				if (setsockopt(_socket, IPPROTO_TCP, TCP_NODELAY, (const char*)&optval, sizeof(optval)) == SOCKET_ERROR)
				{
					LOG(eLogLevel::Error, L"[%s] Failed Socket setsockopt(TCP_NODELAY). (ErrCode:%ld)\n", __FUNCTIONW__, WSAGetLastError());
					return false;
				}
				LOG(eLogLevel::System, L"[%s] Success Socket setsockopt(TCP_NODELAY)\n", __FUNCTIONW__);
			}

			// 비동기 송신 세팅
			if (_netConfig.EnableZeroCopySend == true)
			{
				int optval = 0;
				if (setsockopt(_socket, SOL_SOCKET, SO_SNDBUF, (const char*)&optval, sizeof(optval)) == SOCKET_ERROR)
				{
					LOG(eLogLevel::Error, L"[%s] Failed Socket setsockopt(SO_SNDBUF). (ErrCode:%ld)\n", __FUNCTIONW__, WSAGetLastError());
					return false;
				}
				LOG(eLogLevel::System, L"[%s] Success Socket setsockopt(SO_SNDBUF)\n", __FUNCTIONW__);
			}

			// Linger 세팅
			{
				LINGER optval = {};
				optval.l_onoff = 1;
				optval.l_linger = 0;
				if (setsockopt(_socket, SOL_SOCKET, SO_LINGER, (const char*)&optval, sizeof(optval)) == SOCKET_ERROR)
				{
					LOG(eLogLevel::Error, L"[%s] Failed Socket setsockopt(SO_LINGER). (ErrCode:%ld)\n", __FUNCTIONW__, WSAGetLastError());
					return false;
				}
				LOG(eLogLevel::System, L"[%s] Success Socket setsockopt(SO_LINGER_SNDBUF)\n", __FUNCTIONW__);
			}

			// KeepAlive 세팅
			if (_netConfig.EnableTCPKeepAlive == true)
			{
				DWORD dwResult;
				tcp_keepalive KeepAlive = {};
				KeepAlive.onoff = 1;
				KeepAlive.keepalivetime = 10000;
				KeepAlive.keepaliveinterval = 1000;
				if (WSAIoctl(_socket, SIO_KEEPALIVE_VALS, &KeepAlive, sizeof(KeepAlive), 0, 0, &dwResult, NULL, NULL) != 0)
				{
					LOG(eLogLevel::Error, L"[%s] Failed Socket WSAIoctl(SIO_KEEPALIVE_VALS). (ErrCode:%ld)\n", __FUNCTIONW__, WSAGetLastError());
					return false;
				}
				LOG(eLogLevel::System, L"[%s] Success Socket WSAIoctl(SIO_KEEPALIVE_VALS).\n", __FUNCTIONW__);
			}
		}

		if (_netConfig.IsServer())
		{
			// 3) Bind
			{
				SOCKADDR_IN sockAddr = {};
				InetPtonW(AF_INET, _netConfig.IP.c_str(), (PVOID)&sockAddr.sin_addr);
				sockAddr.sin_family = AF_INET;
				sockAddr.sin_port = htons(static_cast<u_short>(_netConfig.Port));
				if (bind(_socket, (const sockaddr*)&sockAddr, sizeof(sockAddr)) == SOCKET_ERROR)
				{
					LOG(eLogLevel::Error, L"[%s] Failed Listen Socket bind (ErrCode:%d, Port:%d)\n", __FUNCTIONW__, WSAGetLastError(), _netConfig.Port);
					return false;
				}
				LOG(eLogLevel::System, L"[%s] Success Listen Socket Bind. (Port:%d)\n", __FUNCTIONW__, _netConfig.Port);
			}

			// 4) Listen
			{
				if (listen(_socket, SOMAXCONN) == SOCKET_ERROR)
				{
					LOG(eLogLevel::Error, L"[%s] Failed Listen Socket listen. (ErrCode:%d)\n", __FUNCTIONW__, WSAGetLastError());
					return false;
				}
				LOG(eLogLevel::System, L"[%s] Success Listen Socket Listen.\n", __FUNCTIONW__);
			}
		}

		LOG(eLogLevel::System, L"[%s] Success Initialized Socket.\n", __FUNCTIONW__);

		return true;
	}

	bool NetDevice::InitializeSessions()
	{
		// Session
		_sessions = (NetSession*)malloc(sizeof(NetSession) * _netConfig.SessionCount);
		for (int i = 0; i < _netConfig.SessionCount; i++)
		{ 
			new (_sessions + i) NetSession();
		}

		// Session Indexes
		const auto BeginIndex = _netConfig.SessionCount - 1;
		for (auto i = BeginIndex; i >= 0; i--)
		{
			_sessionIndexes.Push(i);
		}

		LOG(eLogLevel::System, L"[%s] Success Initialized Sessions.\n", __FUNCTIONW__);

		return true;
	}

	bool NetDevice::InitializeAcceptor()
	{
		if (_acceptor != INVALID_HANDLE_VALUE)
		{
			LOG(eLogLevel::Error, L"[%s] Already Initialized Acceptor.\n", __FUNCTIONW__);
			return false;
		}

		_acceptor = (HANDLE)_beginthreadex(nullptr, 0, Acceptor, this, 0, nullptr);

		LOG(eLogLevel::System, L"[%s] Success Initialized Acceptor.\n", __FUNCTIONW__);

		return true;
	}

	bool NetDevice::InitializeIocpWorker()
	{
		// 1) IOCP 생성
		_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, _netConfig.IOCPWorkerConcurrentCount);
		if (_iocp == INVALID_HANDLE_VALUE)
		{
			LOG(eLogLevel::Error, L"[%s] Failure CreateIoCompletionPort().\n", __FUNCTIONW__);
			return false;
		}

		// 2) IOCP Worker 생성
		for (auto i = 0; i < _netConfig.IOCPWorkerCount; i++)
		{
			_iocpWorkers[i] = (HANDLE)_beginthreadex(nullptr, 0, IOCPWorker, this, 0, nullptr);
		}
		LOG(eLogLevel::System, L"[%s] Success Initialized Iocp Workers.\n", __FUNCTIONW__);

		return true;
	}

	bool NetDevice::ReleaseNetwork()
	{
		const int Result = ::WSACleanup();
		if (Result != 0)
		{
			LOG(eLogLevel::Error, L"[%s] Failed to WSACleanup(). (ErrCode:%d)\n", __FUNCTIONW__, WSAGetLastError());
			return false;
		}

		LOG(eLogLevel::System, L"[%s] Success WSACleanup().\n", __FUNCTIONW__);

		return true;
	}

	bool NetDevice::ReleaseListenSocket()
	{
		if (_socket == INVALID_SOCKET)
		{
			LOG(eLogLevel::Error, L"[%s] Not Initialized Listen Socket.\n", __FUNCTIONW__);
		}

		closesocket(_socket);

		_socket = INVALID_SOCKET;

		LOG(eLogLevel::System, L"[%s] Success Released Listen Socket.\n", __FUNCTIONW__);

		return true;
	}

	bool NetDevice::ReleaseSessions()
	{
		for (auto i = 0; i < _netConfig.SessionCount; i++)
		{
			(_sessions + i)->~NetSession();
		}

		free(_sessions);

		LOG(eLogLevel::System, L"[%s] Success Released Sessions.\n", __FUNCTIONW__);

		return true;
	}

	bool NetDevice::ReleaseAcceptor()
	{
		if (_acceptor == nullptr)
		{
			LOG(eLogLevel::Error, L"[%s] Not Initialized Acceptor.\n", __FUNCTIONW__);
			return false;
		}

		// 리슨 소켓 닫기
		ReleaseListenSocket();

		// 스레드 종료 대기 및 핸들 반환
		WaitForSingleObject(_acceptor, INFINITE);
		CloseHandle(_acceptor);

		_acceptor = nullptr;

		LOG(eLogLevel::System, L"[%s] Success Released Acceptor.\n", __FUNCTIONW__);

		return true;
	}

	bool NetDevice::ReleaseIocpWorker()
	{
		if (_iocp == nullptr)
		{
			LOG(eLogLevel::Error, L"[%s] Not Initialized Iocp Workers.\n", __FUNCTIONW__);
			return false;
		}

		for (auto i = 0; i < _netConfig.IOCPWorkerCount; i++)
		{
			if (_iocpWorkers[i] == nullptr)
			{
				LOG(eLogLevel::Error, L"[%s] Not Initialized Iocp Worker.\n", __FUNCTIONW__);
				return false;
			}
		}

		// IocpWorker 스레드 종료 대기
		PostQueuedCompletionStatus(
			_iocp,
			static_cast<DWORD>(eIOCPWorkerJobType::OnExit),
			reinterpret_cast<ULONG_PTR>(nullptr),
			nullptr
		);
		WaitForMultipleObjects(_netConfig.IOCPWorkerCount, _iocpWorkers, TRUE, INFINITE);

		// 핸들 정리
		CloseHandle(_iocp);
		for (auto i = 0; i < _netConfig.IOCPWorkerCount; i++)
		{
			CloseHandle(_iocpWorkers[i]);
			_iocpWorkers[i] = nullptr;
		}
		_iocp = nullptr;

		LOG(eLogLevel::System, L"[%s] Success Released Iocp Workers.\n", __FUNCTIONW__);

		return true;
	}

	void NetDevice::RecvPost(NetSession* pSession)
	{
		WSABUF	wsaBuffers[2] = {};
		int		wsaBufferCount = 1;

		// 1) WSA 버퍼 세팅
		const auto NonBrokenWritableSize = pSession->RecvBuffer.GetNonBrokenWritableSize();
		wsaBuffers[0].buf = pSession->RecvBuffer.GetWritePos();
		wsaBuffers[0].len = NonBrokenWritableSize;

		// 2) 세션 수신 버퍼에 남은 공간 있으면, 추가 등록
		const auto WritableSize = pSession->RecvBuffer.GetWritableSize();
		if (WritableSize > NonBrokenWritableSize)
		{
			wsaBuffers[1].buf = pSession->RecvBuffer.GetEntryPos();
			wsaBuffers[1].len = WritableSize - NonBrokenWritableSize;
			wsaBufferCount = 2;
		}

		// 3) 수신 등록
		DWORD flags = 0;
		_InterlockedIncrement16(&pSession->IOCount);
		ZeroMemory(&pSession->RecvOverlapped.Overlapped, sizeof(pSession->RecvOverlapped.Overlapped));
		const auto Result = WSARecv(pSession->Socket, wsaBuffers, wsaBufferCount, NULL, &flags, &pSession->RecvOverlapped.Overlapped, NULL);
		const errno_t ErrCode = WSAGetLastError();

		// 4) 에러 체크
		if (Result == SOCKET_ERROR && ErrCode != ERROR_IO_PENDING)
		{
			if (_netConfig.IsAbnormalSocketError(ErrCode) == true)
			{
				LOG(eLogLevel::Error, L"[%s] Failed WSARecv(). (Err:%d, SessionID:%lld, SessionIndex:%d, IP:%s, Port:%d)\n",
					__FUNCTIONW__,
					ErrCode,
					pSession->SessionID,
					pSession->SessionIndex,
					pSession->IP,
					pSession->Port);
			}

			if (_InterlockedDecrement16(&pSession->IOCount) == 0)
			{
				ReleaseSession(pSession);
			}
		}

		// 송/수신 등록 전에 CancelIO가 먼저 실행되면 CancelIO가 씹히게됨.
		// 따라서 CancelIO 실행 전에 bDisconnectFlag를 먼저 true로 변경하므로 CancelIO가 이미 실행된 세션인지 체크해야한다.
		// Q)만약 여기서 DisconnectFlag가 false라고 판단하고 그냥 나가버리면?
		// A)그럼 Disconnect함수에서 CancelIO가 아직 실행되지 않았다는 뜻이다. 그럼 Disconnect 함수에서 취소 가능하다.
		if (pSession->bDisconnectFlag == true)
		{
			CancelIoEx((HANDLE)pSession->Socket, nullptr);
		}
	}

	void NetDevice::RecvProc(NetSession* pSession, const DWORD ReceivedBytes)
	{
		// 세션 수신 버퍼 갱신
		pSession->RecvBuffer.DoMoveWritePos(ReceivedBytes);

		bool bResult = false;
		switch (_netConfig.NetDeviceType)
		{
		case eNetDeviceType::LanServer:
		case eNetDeviceType::LanClient:
			bResult = RecvProcLan(pSession);
			break;
		case eNetDeviceType::NetServer:
		case eNetDeviceType::NetClient:
			bResult = RecvProcNet(pSession);
			break;
		default:
			break;
		}

		// 패킷 조립 실패시 세션 연결 끊기
		if (bResult == false)
		{
			DisconnectSession(pSession);
			return;
		}

		// 수신 재등록
		RecvPost(pSession);
	}

	bool NetDevice::RecvProcLan(NetSession* pSession)
	{
		while (true)
		{
			LanPacketHeader packetHeader;

			// 세션 수신 버퍼에 헤더는 왔는가?
			const int ReadableSize = pSession->RecvBuffer.GetReadableSize();
			if (ReadableSize < sizeof(LanPacketHeader))
			{
				break;
			}

			// 세션 수신 버퍼에서 헤더 Peek
			pSession->RecvBuffer.DoPeek(reinterpret_cast<char*>(&packetHeader), sizeof(LanPacketHeader));

			// 세션 수신 버퍼에 패킷이 완성됐는가?
			const int PacketSize = sizeof(LanPacketHeader) + packetHeader.PayloadLength;
			if (ReadableSize < PacketSize)
			{
				break;
			}

			// 세션 수신 버퍼에서 헤더 삭제
			pSession->RecvBuffer.DoMoveReadPos(sizeof(LanPacketHeader));

			// 페이로드만 패킷 버퍼로 복사
			PacketBuffer* pMessage = PacketFactory::AllocLanPacket();
			const int ReadedSize = pSession->RecvBuffer.DoRead(pMessage->GetWritePos(), packetHeader.PayloadLength);
			pMessage->DoMoveWritePos(ReadedSize);

			// 패킷 수신 공지
			OnRecv(pSession->SessionID, pMessage);

			// 패킷 버퍼 정리
			pMessage->DecreseRefCount();

			_InterlockedIncrement(&_netStatistics.RecvPacketTPS);
		}

		return true;
	}

	bool NetDevice::RecvProcNet(NetSession* pSession)
	{
		while (true)
		{
			NetPacketHeader packetHeader;

			// 세션 수신 버퍼에 헤더는 왔는가?
			const int ReadableSize = pSession->RecvBuffer.GetReadableSize();
			if (ReadableSize < sizeof(NetPacketHeader))
			{
				break;
			}

			// 세션 수신 버퍼에서 헤더 Peek
			pSession->RecvBuffer.DoPeek(reinterpret_cast<char*>(&packetHeader), sizeof(NetPacketHeader));

			// 패킷 사이즈가 허용 사이즈 초과하는 경우 세션 연결을 끊는다.
			if (packetHeader.PayloadLength > _netConfig.PacketMaxSize)
			{
				return false;
			}

			// 세션 수신 버퍼에 패킷이 완성됐는가?
			const int PacketSize = sizeof(NetPacketHeader) + packetHeader.PayloadLength;
			if (ReadableSize < PacketSize)
			{
				break;
			}

			// 패킷 해제
			PacketBuffer* pRecvPacket = PacketFactory::AllocNetPacket();
			{
				// (헤더 + 페이로드) 패킷 버퍼로 복사
				const int ReadedSize = pSession->RecvBuffer.DoRead(pRecvPacket->GetEntryPos(), PacketSize);
				pRecvPacket->DoMoveWritePos(ReadedSize - sizeof(NetPacketHeader));

				// 복호화
				if (pRecvPacket->Decode() == false)
				{
					pRecvPacket->DecreseRefCount();
					return false;
				}

				// 메세지 수신 알림
				OnRecv(pSession->SessionID, pRecvPacket);
			}
			pRecvPacket->DecreseRefCount();

			_InterlockedIncrement(&_netStatistics.RecvPacketTPS);
		}

		return true;
	}

	void NetDevice::SendPost(NetSession* pSession)
	{
		PRO_BEGIN(L"IocpWorker::SendPacket");
		do
		{
			// 1) 이미 송신 상태인지 체크
			const bool bSendFlag = (bool)_InterlockedExchange8((volatile CHAR*)&pSession->bSendFlag, true);
			if (bSendFlag == true)
			{
				return;
			}

			// 2) 송신할 패킷이 있는지 체크
			if (pSession->SendBuffer.GetCapacity() <= 0)
			{
				// 없으면 SendFlag 끄고 리턴.
				pSession->bSendFlag = false;

				// SendFlag를 false로 변경하기 전에 다른 스레드에서 SendPost()를 호출해서 true라고 판단하고 나가버리면
				// 다시 SendPost를 호출하기 전까지 해당 패킷은 영원히 송신되지 않음.
				// 따라서 SendFlag를 false로 변경하는 시점에 다른 스레드에서 SendBuffer에 추가한 패킷이 있는지 확인 필요.
				if (pSession->SendBuffer.GetCapacity() > 0)
				{
					continue;
				}

				return;
			}

			// 3) WSA 버퍼 세팅
			WSABUF	wsaBuffers[NetConfig::MaxSendPacketCount] = {};
			int		wsaBufferCount = 0;
			while (pSession->SendBuffer.GetCapacity() > 0)
			{
				PacketBuffer* pSendPacket;
				pSession->SendBuffer.Dequeue(&pSendPacket);
				if (_netConfig.IsLanType())
				{
					wsaBuffers[wsaBufferCount].buf = pSendPacket->GetEntryPos();
					wsaBuffers[wsaBufferCount].len = pSendPacket->GetLanPacketLength();
				}
				else
				{
					wsaBuffers[wsaBufferCount].buf = pSendPacket->GetEntryPos();
					wsaBuffers[wsaBufferCount].len = pSendPacket->GetNetPacketLength();
				}

				pSession->SendPackets[wsaBufferCount] = pSendPacket;
				wsaBufferCount++;

				// 한 번에 등록할 수 있는 송신 패킷에 도달했다면 종료.
				if (wsaBufferCount >= NetConfig::MaxSendPacketCount)
				{
					break;
				}
			}
			pSession->SendPacketCount = wsaBufferCount;

			// 4) 송신
			_InterlockedIncrement16(&pSession->IOCount);
			ZeroMemory(&pSession->SendOverlapped.Overlapped, sizeof(pSession->SendOverlapped.Overlapped));
			const auto Result = WSASend(pSession->Socket, wsaBuffers, wsaBufferCount, NULL, 0, &pSession->SendOverlapped.Overlapped, NULL);
			const errno_t ErrCode = WSAGetLastError();

			// 5) 에러 체크
			if (Result == SOCKET_ERROR && ErrCode != ERROR_IO_PENDING)
			{
				if (_netConfig.IsAbnormalSocketError(ErrCode) == true)
				{
					LOG(eLogLevel::Debug, L"[%s] Failed WSASend(). (error:%d, SessionID:%lld, SessionIndex:%d, IP:%s, Port:%d)\n",
						__FUNCTIONW__,
						ErrCode,
						pSession->SessionID,
						pSession->SessionIndex,
						pSession->IP,
						pSession->Port);
				}

				if (_InterlockedDecrement16(&pSession->IOCount) == 0)
				{
					ReleaseSession(pSession);
				}
			}

			// 송/수신 등록 전에 CancelIO가 먼저 실행되면 CancelIO가 씹히게됨.
			// 따라서 CancelIO 실행 전에 bDisconnectFlag를 먼저 true로 변경하므로 CancelIO가 이미 실행된 세션인지 체크해야한다.
			// Q)만약 여기서 DisconnectFlag가 false라고 판단하고 그냥 나가버리면?
			// A)그럼 Disconnect함수에서 CancelIO가 아직 실행되지 않았다는 뜻이다. 그럼 Disconnect 함수에서 취소 가능하다.
			if (pSession->bDisconnectFlag == true)
			{
				CancelIoEx((HANDLE)pSession->Socket, nullptr);
			}

			return;

		} while (true);
	}

	void NetDevice::SendProc(NetSession* pSession, const DWORD SendedBytes)
	{
		// 송신 완료 알림
		OnSend(pSession->SessionID, SendedBytes);

		// 송신 완료 패킷 정리
		const auto SendPacketCount = pSession->SendPacketCount;
		for (auto i = 0; i < SendPacketCount; i++)
		{
			pSession->SendPackets[i]->DecreseRefCount();
		}

		pSession->SendPacketCount = 0;

		// 송신 재등록
		pSession->bSendFlag = false;
		SendPost(pSession);

		_interlockedadd(&_netStatistics.SendPacketTPS, SendPacketCount);
	}

	NetConfig::SESSION_ID NetDevice::CreateSessionID(const NetConfig::SESSION_INDEX SessionIndex)
	{
		NetConfig::SESSION_ID SessionID = ++_sessionIDCounter;
		SessionID <<= (sizeof(SessionIndex) * 8);
		SessionID |= SessionIndex;

		return SessionID;
	}

	void NetDevice::NewSession(const SOCKET ClientSocket, const NetConfig::SESSION_INDEX SessionIndex, const wchar_t* IP, const WORD Port)
	{
		const NetConfig::SESSION_ID SessionID = CreateSessionID(SessionIndex);

		// 세션 세팅
		NetSession* pSession = _sessions + SessionIndex;
		{
			_InterlockedIncrement16(&pSession->IOCount);
			pSession->SessionID = SessionID;
			pSession->SessionIndex = SessionIndex;
			pSession->RecvBuffer.Clear();
			pSession->bSendFlag = false;
			pSession->bDisconnectFlag = false;
			pSession->Socket = ClientSocket;
			pSession->Port = Port;
			wcscpy_s(pSession->IP, sizeof(pSession->IP) / sizeof(wchar_t), IP);

			// Iocp에 소켓 등록
			CreateIoCompletionPort(reinterpret_cast<HANDLE>(pSession->Socket), _iocp, reinterpret_cast<ULONG_PTR>(pSession), 0);

			// 세션 세팅 전에 ReleaseFlag를 먼저 false로 갱신한다면,
			// RealseSession 함수를 실행하다가 컨텍스트 스위칭 된 다른 스레드가 깨어날 경우, ReleaseFlag를 false로 판단한고
			// 세션을 정리해버리는 상황이 발생할 수 있으므로, 세션 초기화 후 맨 마지막에 ReleaseFlag를 갱신해야한다.
			pSession->bReleaseFlag = false;
		}

		if (_netConfig.IsServer())
		{
			// 신규 연결 알림
			OnClientJoin(pSession->SessionID);
		}
		else
		{
			// 서버 연결 알림
			OnEnterJoinServer(pSession->SessionID);
		}

		// 수신 등록
		RecvPost(pSession);

		// IOCount 체크
		// 수신 등록 후 바로 연결 종료될 경우 IocpWorker에서 수신 실패 처리해야하는데, 현재 스레드에서 올린
		// IOCount로 인해 Release가 되지 않을 수 있으므로, 여기서 한 번 더 IOCount를 체크해야함.
		if (_InterlockedDecrement16(&pSession->IOCount) == 0)
		{
			ReleaseSession(pSession);
		}
	}

	NetSession* NetDevice::GetSession(const NetConfig::SESSION_ID SessionID)
	{
		const NetConfig::SESSION_INDEX SessionIndex = _netConfig.GetSessionIndex(SessionID);

		NetSession* pSession = _sessions + SessionIndex;

		_InterlockedIncrement16(&pSession->IOCount);

		// ReleaseFlag를 확인하지 않으면 이미 끊어진 세션의 SessionID가 메모리에 남아있으므로, 
		// 해당 SessionID를 캐치하고 진행될 수 있음.
		const bool bReleaseFlag = static_cast<bool>(pSession->bReleaseFlag);
		if (bReleaseFlag == true)
		{
			if (_InterlockedDecrement16(&pSession->IOCount) == 0)
			{
				ReleaseSession(pSession);
			}
			return nullptr;
		}

		// SessionID를 ReleaseFlag보다 먼저 확인할 경우, SessionID 조건문이 통과하고
		// CreateSession() 함수에서 ReleaseFlag를 false로 변경하는 상황을 캐치하고 진행될 수 있음.
		// 만약 위와 같은 상황이 발생하면, 다른 SessionID로 방금 생성된 다른 세션 메모리를 사용하는 상황이 발생함.
		if (pSession->SessionID != SessionID)
		{
			if (_InterlockedDecrement16(&pSession->IOCount) == 0)
			{
				ReleaseSession(pSession);
			}
			return nullptr;
		}

		return pSession;
	}

	bool NetDevice::ReleaseSession(NetSession* pSession)
	{
		// 초기화 된 적이 없는 세션이 인덱스 스택에 반환되는 것을 방지.
		if (pSession->SessionIndex == NetConfig::INVALID_SESSION_INDEX)
		{
			return false;
		}

		const bool bPrevReleaseFlag = (bool)(_InterlockedCompareExchange((long*)&pSession->bReleaseFlag, 0x00000001, 0x00000000));

		// 이미 다른 스레드에서 Release중이라면, 리턴
		if (bPrevReleaseFlag == true)
		{
			return false;
		}

		const NetConfig::SESSION_ID SessionID = pSession->SessionID;

		// SendPacket() 함수에서 SendBuffer에 Enqueue하고
		// SendPost() 함수 내부에서 SendFlag에 막혀서 실제 송신 등록을 하지 못한 패킷 정리
		while (pSession->SendBuffer.GetCapacity() > 0)
		{
			PacketBuffer* pSendPacket;
			pSession->SendBuffer.Dequeue(&pSendPacket);
			pSendPacket->DecreseRefCount();
		}

		// WSASend() 함수 실패로 인해 송신 등록에 실패한 패킷 정리
		for (auto i = 0; i < pSession->SendPacketCount; i++)
		{
			pSession->SendPackets[i]->DecreseRefCount();
		}
		pSession->SendPacketCount = 0;

		// 소켓 리소스 반환 후 RST 송신
		closesocket(pSession->Socket);

		// 세션 인덱스 반환
		_sessionIndexes.Push(pSession->SessionIndex);

		// 세션 종료 알림
		OnClientLeave(SessionID);

		_InterlockedIncrement(&_netStatistics.DisconnectTPS);
		_InterlockedIncrement(&_netStatistics.DisconnectTotal);

		return true;
	}

	bool NetDevice::DisconnectSession(NetSession* pSession)
	{
		_InterlockedIncrement16(&pSession->IOCount);

		// 이미 정리된 세션이라면 스킵
		const bool bReleaseFlag = static_cast<bool>(pSession->bReleaseFlag);
		if (bReleaseFlag == true)
		{
			_InterlockedDecrement16(&pSession->IOCount);
			return false;
		}

		// 더이상 IO 안걸리게 막고
		pSession->bDisconnectFlag = true;

		// 이미 걸린 IO 취소해서 IOCP에 완료 이벤트 발생켜서 ReleaseSession 호출되게 유도
		CancelIoEx((HANDLE)pSession->Socket, nullptr);

		if (_InterlockedDecrement16(&pSession->IOCount) == 0)
		{
			ReleaseSession(pSession);
		}

		return true;
	}

	void NetDevice::DisconnectAllSession()
	{
		const auto CurSessionCount = _netConfig.SessionCount - _sessionIndexes.GetCapacity();
		if (CurSessionCount <= 0)
		{
			return;
		}

		// 전체 세션 연결 종료
		for (auto i = 0; i < _netConfig.SessionCount; i++)
		{
			DisconnectSession(_sessions + i);
		}

		// 모든 세션이 반환될 때까지 대기
		while ((_netConfig.SessionCount - _sessionIndexes.GetCapacity()) != 0) {};
	}

	unsigned int __stdcall NetDevice::Acceptor(void* lpParam)
	{
		return static_cast<NetDevice*>(lpParam)->AcceptorProc();
	}

	int NetDevice::AcceptorProc()
	{
		SOCKET ClientSocket = INVALID_SOCKET;
		SOCKADDR_IN	ClientAddr = {};
		int	addrlen = sizeof(ClientAddr);

		while (true)
		{
			// Accept 체크
			ClientSocket = accept(_socket, (sockaddr*)&ClientAddr, &addrlen);
			if (ClientSocket == INVALID_SOCKET)
			{
				errno_t err = WSAGetLastError();
				if (err != WSAEINTR)
				{
					LOG(eLogLevel::Error, L"[%s] Failed accept. (ErrCode:%d)\n", __FUNCTIONW__, err);
					OnError(eErrorCode::FailedAccept);
				}
				return 0;
			}

			// 접속 정보 획득
			WORD ConnectPort;
			wchar_t ConnectIP[16] = {};
			{
				SOCKADDR_IN	name = {};
				int namelen = sizeof(name);
				getpeername(ClientSocket, reinterpret_cast<sockaddr*>(&name), &namelen);
				
				InetNtop(AF_INET, reinterpret_cast<const void*>(&name.sin_addr), ConnectIP, 16);
				ConnectPort = htons(name.sin_port);
			}

			// 접속 허용 여부 체크
			if (OnConnectionRequest(ConnectIP, ConnectPort) == false)
			{
				closesocket(ClientSocket);
				continue;
			}

			// 세션 인덱스 획득
			NetConfig::SESSION_INDEX sessionIndex;
			if (_sessionIndexes.Pop(&sessionIndex) == false)
			{
				closesocket(ClientSocket);
				OnError(eErrorCode::NotExistBlankSessionIndex);
				continue;
			}

			// 세션 생성
			NewSession(ClientSocket, sessionIndex, ConnectIP, ConnectPort);

			_netStatistics.AcceptTPS++;
			_netStatistics.AcceptTotal++;
		}
	}

	unsigned int __stdcall NetDevice::IOCPWorker(void* lpParam)
	{
		return static_cast<NetDevice*>(lpParam)->IOCPWorkerProc();
	}
	int NetDevice::IOCPWorkerProc()
	{
		DWORD		dwTransferred;
		OVERLAPPED* pOverlapped;
		NetSession*	pSession;

		while (true)
		{
			dwTransferred = 0;
			pOverlapped = nullptr;
			pSession = nullptr;

			GetQueuedCompletionStatus(_iocp, &dwTransferred, reinterpret_cast<PULONG_PTR>(&pSession), &pOverlapped, INFINITE);

			const eIOCPWorkerJobType Job = static_cast<eIOCPWorkerJobType>(dwTransferred);

			// 스레드 종료 체크
			if (Job == eIOCPWorkerJobType::OnExit && pSession == nullptr && pOverlapped == nullptr)
			{
				::PostQueuedCompletionStatus(
					_iocp,
					static_cast<DWORD>(eIOCPWorkerJobType::OnExit),
					reinterpret_cast<ULONG_PTR>(nullptr),
					nullptr);
				return 0;
			}

			// 스레드 잡 처리 시작
			if (pSession != nullptr)
			{
				OnWorkerThreadBegin();
				switch (Job)
				{
				case eIOCPWorkerJobType::OnDisconnect:
					DisconnectSession(pSession);
					break;

				case eIOCPWorkerJobType::OnSendPacket:
					SendPost(pSession);
					break;

				default:
					const auto Type = reinterpret_cast<OverlappedEx*>(pOverlapped)->Type;
					switch (Type)
					{
					case OverlappedEx::eType::Receive:
						PRO_BEGIN(L"IocpWorker::RecvProc");
						RecvProc(pSession, dwTransferred);
						PRO_END(L"IocpWorker::RecvProc");
						break;

					case OverlappedEx::eType::Send:
						SendProc(pSession, dwTransferred);
						PRO_END(L"IocpWorker::SendPacket");
						break;

					default:
						LOG(eLogLevel::Error, L"[%s] Invalid Overlapped Type. (Type:%d)", __FUNCTIONW__, Type);
						break;
					}
					break;
				}

				// 이전 작업에서 올려둔 IOCount 차감
				if (_InterlockedDecrement16(&pSession->IOCount) == 0)
				{
					ReleaseSession(pSession);
				}

				OnWorkerThreadEnd();
			}
		}

		return 0;
	}
}