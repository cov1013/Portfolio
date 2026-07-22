#include "NetClient.h"

namespace cov1013
{
    bool NetClient::Start(const Config& Config)
    {
        LOG(eLogLevel::System, L"[%s] Begin\n", __FUNCTIONW__);

        if (InitializeConfig(Config) == false)
        {
            goto Error;
        }

        if (InitializeNetwork() == false)
        {
            goto Error;
        }

        if (InitializeSocket() == false)
        {
            goto Error;
        }

        if (InitializeIocpWorker() == false)
        {
            goto Error;
        }

        LOG(eLogLevel::System, L"[%s] End\n", __FUNCTIONW__);
        return true;

    Error:
        LOG(eLogLevel::System, L"[%s] End\n", __FUNCTIONW__);
        return false;
    }

    void NetClient::Quit()
    {
        Disconnect();
        while (_session.IOCount > 0) 
        {
            Sleep(20);
        };
    }

    bool NetClient::Connect()
    {
        LOG(eLogLevel::System, L"[%s] Begin\n", __FUNCTIONW__);

        SOCKADDR_IN sockAddr = {};
        sockAddr.sin_family = AF_INET;
        sockAddr.sin_port = htons(_config.Port);
        InetPton(AF_INET, _config.IP.c_str(), &sockAddr.sin_addr);

        if (connect(_session.Socket, (SOCKADDR*)&sockAddr, sizeof(sockAddr)) == SOCKET_ERROR)
        {
            LOG(eLogLevel::Error, L"[%s] Connect Failed (ErrCode:%d)\n", __FUNCTIONW__, WSAGetLastError());
            goto Error;
        }

        CreateIoCompletionPort((HANDLE)_session.Socket, _iocp, (ULONG_PTR)&_session, 0);

        RecvPost();

        OnEnterJoinServer();

        LOG(eLogLevel::System, L"[%s] Success Connect. (IP:%s, Port:%d)\n", __FUNCTIONW__, _config.GetIP().c_str(), _config.GetPort());
        LOG(eLogLevel::System, L"[%s] End\n", __FUNCTIONW__);
        return true;

    Error:
        LOG(eLogLevel::System, L"[%s] End\n", __FUNCTIONW__);
        return false;
    }

    bool NetClient::Reconnect()
    {
        LOG(eLogLevel::System, L"[%s] Begin\n", __FUNCTIONW__);

        if (InitializeSocket() == false)
        {
            goto Error;
        }

        if (Connect() == false)
        {
            goto Error;
        }

        LOG(eLogLevel::System, L"[%s] End\n", __FUNCTIONW__);
        return true;

    Error:
        LOG(eLogLevel::System, L"[%s] End\n", __FUNCTIONW__);
        return false;
    }

    void NetClient::Disconnect()
    {
        InterlockedIncrement16(&_session.IOCount);

        if ((bool)_session.bReleaseFlag == true)
        {
            if (InterlockedDecrement16(&_session.IOCount) == 0)
            {
                ReleaseSession();
            }
            return;
        }

        _session.bDisconnectFlag = true;

        CancelIoEx((HANDLE)_session.Socket, NULL);

        if (InterlockedDecrement16(&_session.IOCount) == 0)
        {
            ReleaseSession();
        }
    }

    void NetClient::SendPacket(PacketBuffer* pPacket)
    {
        InterlockedIncrement16(&_session.IOCount);

        if ((bool)_session.bReleaseFlag == true)
        {
            if (InterlockedDecrement16(&_session.IOCount) == 0)
            {
                ReleaseSession();
            }
            return;
        }

        switch (_type)
        {
        case eNetworkType::LanNetwork:
            pPacket->SetLanPacketHeader();
            break;

        case eNetworkType::NetNetwork:
            pPacket->SetNetPacketHeader();
            break;
        }

        pPacket->IncreseRefCount();
        _session.SendBuffer.Enqueue(pPacket);

        if (_config.IsEnableIocpWorkerSend() == false)
        {
            SendPost();

            if (InterlockedDecrement16(&_session.IOCount) == 0)
            {
                ReleaseSession();
            }
        }
        else
        {
            PostQueuedCompletionStatus(
                _iocp,
                static_cast<DWORD>(eIocpWorkerJob::OnSendPacket),
                reinterpret_cast<ULONG_PTR>(&_session),
                nullptr);
        }
    }

    bool NetClient::InitializeConfig(const NetConfig& Config)
    {
        if (Config.GetIocpWorkerCount() > MAX_IOCP_WORKER_COUNT)
        {
            LOG(eLogLevel::Error, L"[%s] Invalid Max Worker Count. (MaxCount:%d, CurCount:%d)", __FUNCTIONW__,
                MAX_IOCP_WORKER_COUNT, Config.GetIocpWorkerCount());
            return false;
        }

        if (Config.GetSessionCapacity() > MAX_SESSION_COUNT)
        {
            LOG(eLogLevel::Error, L"[%s] Invalid Max Session Count. (MaxCount:%d, CurCount:%d)", __FUNCTIONW__,
                MAX_SESSION_COUNT, Config.GetSessionCapacity());
            return false;
        }

		_config = Config;

        return true;
    }

    bool NetClient::InitializeNetwork()
    {
        WSADATA	wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        {
            LOG(eLogLevel::Error, L"[%s] Failed to WSAStartup(). (ErrCode:%ld)\n", __FUNCTIONW__, WSAGetLastError());
            return false;
        }

        LOG(eLogLevel::System, L"[%s] Success WSAStartup().\n", __FUNCTIONW__);
        return true;
    }

    bool NetClient::InitializeSocket()
    {
        if (_session.Socket != INVALID_SOCKET)
        {
            LOG(eLogLevel::Error, L"[%s] Already Created Socket.\n", __FUNCTIONW__);
            return true;
        }

        {
            _session.Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            if (_session.Socket == INVALID_SOCKET)
            {
                LOG(eLogLevel::Error, L"[%s] Failed Create Socket.(ErrCode:%ld)\n", __FUNCTIONW__, WSAGetLastError());
                return false;
            }
            LOG(eLogLevel::System, L"[%s] Success Create Socket.\n", __FUNCTIONW__);

            _session.bReleaseFlag = 0;
            _session.bDisconnectFlag = false;
            _session.bSendFlag = false;
            _session.RecvBuffer.Clear();
        }

        {
            if (_config.IsEnableNagle() == false)
            {
                bool optval = true;
                if (setsockopt(_session.Socket, IPPROTO_TCP, TCP_NODELAY, (const char*)&optval, sizeof(optval)) == SOCKET_ERROR)
                {
                    LOG(eLogLevel::Error, L"[%s] Failed Socket setsockopt(TCP_NODELAY). (ErrCode:%ld)\n", __FUNCTIONW__, WSAGetLastError());
                    return false;
                }
                LOG(eLogLevel::System, L"[%s] Success Socket setsockopt(TCP_NODELAY)\n", __FUNCTIONW__);
            }

            if (_config.IsEnableZeroCopySend() == true)
            {
                int optval = 0;
                if (setsockopt(_session.Socket, SOL_SOCKET, SO_SNDBUF, (const char*)&optval, sizeof(optval)) == SOCKET_ERROR)
                {
                    LOG(eLogLevel::Error, L"[%s] Failed Socket setsockopt(SO_SNDBUF). (ErrCode:%ld)\n", __FUNCTIONW__, WSAGetLastError());
                    return false;
                }
                LOG(eLogLevel::System, L"[%s] Success Socket setsockopt(SO_SNDBUF)\n", __FUNCTIONW__);
            }

            {
                LINGER optval;
                optval.l_onoff = 1;
                optval.l_linger = 0;
                if (setsockopt(_session.Socket, SOL_SOCKET, SO_LINGER, (const char*)&optval, sizeof(optval)) == SOCKET_ERROR)
                {
                    LOG(eLogLevel::Error, L"[%s] Failed Socket setsockopt(SO_LINGER). (ErrCode:%ld)\n", __FUNCTIONW__, WSAGetLastError());
                    return false;
                }
                LOG(eLogLevel::System, L"[%s] Success Socket setsockopt(SOSO_LINGER_SNDBUF)\n", __FUNCTIONW__);
            }

            if (_config.IsEnableKeepAlive() == true)
            {
                DWORD dwResult;
                tcp_keepalive KeepAlive;
                KeepAlive.onoff = 1;
                KeepAlive.keepalivetime = 10000;
                KeepAlive.keepaliveinterval = 1000;
                if (WSAIoctl(_session.Socket, SIO_KEEPALIVE_VALS, &KeepAlive, sizeof(KeepAlive), 0, 0, &dwResult, NULL, NULL) != 0)
                {
                    LOG(eLogLevel::Error, L"[%s] Failed Socket WSAIoctl(SIO_KEEPALIVE_VALS). (ErrCode:%ld)\n", __FUNCTIONW__, WSAGetLastError());
                    return false;
                }
                LOG(eLogLevel::System, L"[%s] Success Socket WSAIoctl(SIO_KEEPALIVE_VALS).\n", __FUNCTIONW__);
            }
        }

        return true;
    }

    bool NetClient::InitializeIocpWorker()
    {
        if (_iocp != INVALID_HANDLE_VALUE)
        {
            LOG(eLogLevel::Error, L"[%s] Already Create IOCP.\n", __FUNCTIONW__);
            return false;
        }

        _iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, _config.GetIocpWorkerConcurrentCount());
        for (auto i = 0; i < _config.GetIocpWorkerCount(); i++)
        {
            _iocpWorkers[i] = (HANDLE)_beginthreadex(nullptr, 0, IocpWorker, this, 0, nullptr);
        }
        LOG(eLogLevel::System, L"[%s] Success Initialized Iocp Workers.\n", __FUNCTIONW__);

        return true;
    }

    bool NetClient::ReleaseNetwork()
    {
        const int Result = WSACleanup();
        if (Result != 0)
        {
            LOG(eLogLevel::Error, L"[%s] Failed to WSACleanup(). (ErrCode:%d)\n", __FUNCTIONW__, WSAGetLastError());
            return false;
		}
        LOG(eLogLevel::System, L"[%s] Success Release Network.\n", __FUNCTIONW__);
		return true;
    }

    bool NetClient::ReleaseSocket()
    {
        if (_session.Socket == INVALID_SOCKET)
        {
			LOG(eLogLevel::Error, L"[%s] Not Initialized Socket.\n", __FUNCTIONW__);
            return false;
        }

        closesocket(_session.Socket);

        _session.Socket = INVALID_SOCKET;

        LOG(eLogLevel::System, L"[%s] Success Release Socket.\n", __FUNCTIONW__);

        return true;
    }

    bool NetClient::ReleaseIocpWorker()
    {
        if (_iocp == nullptr)
        {
            LOG(eLogLevel::Error, L"[%s] Not Initialized Iocp Workers.\n", __FUNCTIONW__);
            return false;
        }

        for (auto i = 0; i < _config.GetIocpWorkerCount(); i++)
        {
            if (_iocpWorkers[i] == nullptr)
            {
                LOG(eLogLevel::Error, L"[%s] Not Initialized Iocp Worker.\n", __FUNCTIONW__);
                return false;
            }
        }

        // IocpWorker 종료
        PostQueuedCompletionStatus(
            _iocp,
            static_cast<DWORD>(eIocpWorkerJob::OnExit),
            reinterpret_cast<ULONG_PTR>(nullptr),
            nullptr
        );
        WaitForMultipleObjects(_config.GetIocpWorkerCount(), _iocpWorkers, TRUE, INFINITE);

        CloseHandle(_iocp);
        for (auto i = 0; i < _config.GetIocpWorkerCount(); i++)
        {
            CloseHandle(_iocpWorkers[i]);
            _iocpWorkers[i] = nullptr;
        }
        _iocp = nullptr;

        LOG(eLogLevel::System, L"[%s] Success Release IocpWorker.\n", __FUNCTIONW__);

        return true;
    }

    void NetClient::SendPost()
    {
        do
        {
            const bool bSendFlag = (bool)_InterlockedExchange8((volatile CHAR*)&_session.bSendFlag, true);
            if (bSendFlag == true)
            {
                return;
            }

            if (_session.SendBuffer.GetCapacity() <= 0)
            {
                _session.bSendFlag = false;

                if (_session.SendBuffer.GetCapacity() > 0)
                {
                    continue;
                }

                return;
            }

            WSABUF	wsaBuffers[MAX_SEND_PACKET_COUNT];
            int		wsaBufferCount = 0;
            while (_session.SendBuffer.GetCapacity() > 0)
            {
                PacketBuffer* pSendPacket;
                _session.SendBuffer.Dequeue(&pSendPacket);
                _session.SendPackets[wsaBufferCount] = pSendPacket;
                switch (_type)
                {
                case eNetworkType::LanNetwork:
                    wsaBuffers[wsaBufferCount].buf = pSendPacket->GetEntryPos();
                    wsaBuffers[wsaBufferCount].len = pSendPacket->GetLanPacketLength();
                    break;

                case eNetworkType::NetNetwork:
                    wsaBuffers[wsaBufferCount].buf = pSendPacket->GetEntryPos();
                    wsaBuffers[wsaBufferCount].len = pSendPacket->GetNetPacketLength();
                    break;

                default:
                    break;
                }

                wsaBufferCount++;

                if (wsaBufferCount >= MAX_SEND_PACKET_COUNT)
                {
                    break;
                }
            }
            _session.SendPacketCount = wsaBufferCount;

            ZeroMemory(&_session.SendOverlapped.Overlapped, sizeof(_session.SendOverlapped.Overlapped));
            _InterlockedIncrement16(&_session.IOCount);
            const auto Result = WSASend(_session.Socket, wsaBuffers, wsaBufferCount, NULL, 0, &_session.SendOverlapped.Overlapped, NULL);
            const errno_t ErrCode = WSAGetLastError();

            if (Result == SOCKET_ERROR && ErrCode != ERROR_IO_PENDING)
            {
                if (IsAbnormalNetworkError(ErrCode) == true)
                {
					LOG(eLogLevel::Error, L"[%s] Failed WSASend(). (ErrCode:%d)\n", __FUNCTIONW__, ErrCode);
                }

                if (_InterlockedDecrement16(&_session.IOCount) == 0)
                {
                    ReleaseSession();
                }
            }

            if (_session.bDisconnectFlag == true)
            {
                CancelIoEx((HANDLE)_session.Socket, nullptr);
            }

            return;

        } while (true);
    }

    void NetClient::SendProc(const DWORD SendedBytes)
    {
        OnSend(SendedBytes);

        const auto SendPacketCount = _session.SendPacketCount;
        for (auto i = 0; i < SendPacketCount; i++)
        {
            _session.SendPackets[i]->DecreseRefCount();
        }
        _session.SendPacketCount = 0;

        _session.bSendFlag = false;

        SendPost();

        _InlineInterlockedAdd(&_statistics.SendPacketTPS, _session.SendPacketCount);
    }

    void NetClient::RecvPost()
    {
        WSABUF	wsaBuffers[2];
        int		wsaBufferCount = 1;

        const auto NonBrokenWritableSize = _session.RecvBuffer.GetNonBrokenWritableSize();
        wsaBuffers[0].buf = _session.RecvBuffer.GetWritePos();
        wsaBuffers[0].len = NonBrokenWritableSize;

        const auto WritableSize = _session.RecvBuffer.GetWritableSize();
        if (WritableSize > NonBrokenWritableSize)
        {
            wsaBuffers[1].buf = _session.RecvBuffer.GetEntryPos();
            wsaBuffers[1].len = WritableSize - NonBrokenWritableSize;
            wsaBufferCount = 2;
        }

        DWORD flags = 0;
        ZeroMemory(&_session.RecvOverlapped.Overlapped, sizeof(_session.RecvOverlapped.Overlapped));
        InterlockedIncrement16(&_session.IOCount);
        const auto Result = WSARecv(_session.Socket, wsaBuffers, wsaBufferCount, NULL, &flags, &_session.RecvOverlapped.Overlapped, NULL);
        const errno_t ErrCode = WSAGetLastError();

        if (Result == SOCKET_ERROR && ErrCode != ERROR_IO_PENDING)
        {
            if (IsAbnormalNetworkError(ErrCode) == true)
            {
                LOG(eLogLevel::Error, L"[%s] Failed WSARecv(). (Err:%d)\n", __FUNCTIONW__, ErrCode);
            }

            if (InterlockedDecrement16(&_session.IOCount) == 0)
            {
                ReleaseSession();
            }
        }

        if (_session.bDisconnectFlag == true)
        {
            CancelIoEx((HANDLE)_session.Socket, nullptr);
        }
    }

    void NetClient::RecvProc(const DWORD ReceivedBytes)
    {
        // ?�션 ?�신 버퍼 갱신
        _session.RecvBuffer.DoMoveWritePos(ReceivedBytes);

        bool bResult = false;
        switch (_type)
        {
        case eNetworkType::LanNetwork:
            bResult = RecvAssemble_Lan();
            break;
        case eNetworkType::NetNetwork:
            bResult = RecvAssemble_Net();
            break;
        default:
            break;
        }

        if (bResult == false)
        {
            Disconnect();
            return;
        }

        RecvPost();
    }

    bool NetClient::RecvAssemble_Lan()
    {
        while (true)
        {
            LanPacketHeader packetHeader;

            const int ReadableSize = _session.RecvBuffer.GetReadableSize();
            if (ReadableSize < sizeof(LanPacketHeader))
            {
                break;
            }

            _session.RecvBuffer.DoPeek(reinterpret_cast<char*>(&packetHeader), sizeof(LanPacketHeader));

            const int PacketSize = sizeof(LanPacketHeader) + packetHeader.PayloadLength;
            if (ReadableSize < PacketSize)
            {
                break;
            }

            _session.RecvBuffer.DoMoveReadPos(sizeof(LanPacketHeader));

            PacketBuffer* pMessage = PacketBuffer::AllocLanPacket();
            const int ReadedSize = _session.RecvBuffer.DoRead(pMessage->GetWritePos(), packetHeader.PayloadLength);
            pMessage->DoMoveWritePos(ReadedSize);

            OnRecv(pMessage);

            pMessage->DecreseRefCount();

            InterlockedIncrement(&_statistics.RecvPacketTPS);
        }

        return true;
    }

    bool NetClient::RecvAssemble_Net()
    {
        while (true)
        {
            NetPacketHeader packetHeader;

            const int ReadableSize = _session.RecvBuffer.GetReadableSize();
            if (ReadableSize < sizeof(NetPacketHeader))
            {
                break;
            }

            _session.RecvBuffer.DoPeek(reinterpret_cast<char*>(&packetHeader), sizeof(NetPacketHeader));

            if (packetHeader.PayloadLength > _config.GetPacketMaxSize())
            {
                return false;
            }

            const int PacketSize = sizeof(NetPacketHeader) + packetHeader.PayloadLength;
            if (ReadableSize < PacketSize)
            {
                break;
            }

            PacketBuffer* pRecvPacket = PacketBuffer::AllocNetPacket();
            {
                const int ReadedSize = _session.RecvBuffer.DoRead(pRecvPacket->GetEntryPos(), PacketSize);
                pRecvPacket->DoMoveWritePos(ReadedSize - sizeof(NetPacketHeader));

                if (pRecvPacket->Decode() == false)
                {
                    pRecvPacket->DecreseRefCount();
                    return false;
                }
                OnRecv(pRecvPacket);
            }
            pRecvPacket->DecreseRefCount();

            InterlockedIncrement(&_statistics.RecvPacketTPS);
        }

        return true;
    }

    bool NetClient::ReleaseSession()
    {
        const bool bReleaseFlag = (bool)(InterlockedCompareExchange((long*)&_session.bReleaseFlag, 0x00000001, 0x00000000));
        if (bReleaseFlag == true)
        {
            return false;
        }

        while (_session.SendBuffer.GetCapacity() > 0)
        {
            PacketBuffer* pSendPacket;
            _session.SendBuffer.Dequeue(&pSendPacket);
            pSendPacket->DecreseRefCount();
        }

        for (auto i = 0; i < _session.SendPacketCount; i++)
        {
            _session.SendPackets[i]->DecreseRefCount();
        }
        _session.SendPacketCount = 0;

        ReleaseSocket();

        OnLeaveServer();

        _session.Socket = INVALID_SOCKET;

        return true;
    }

    bool NetClient::IsAbnormalNetworkError(const errno_t ErrCode) const
    {
        switch (ErrCode)
        {
        case WSAEINTR:
        case WSAEWOULDBLOCK:
        case WSAECONNRESET:
        case WSAESHUTDOWN:
            return false;
        default:
            return true;
        }
    }

    unsigned int __stdcall NetClient::IocpWorker(void* lpParam)
    {
        return static_cast<NetClient*>(lpParam)->IocpWorkerProc();
    }

    int NetClient::IocpWorkerProc()
    {
        DWORD		    dwTransferred;
        OVERLAPPED*     pOverlapped;
        Session*        pSession;

        while (true)
        {
            dwTransferred = 0;
            pOverlapped = nullptr;
            pSession = nullptr;

            GetQueuedCompletionStatus(_iocp, &dwTransferred, reinterpret_cast<PULONG_PTR>(&pSession), &pOverlapped, INFINITE);

            const auto IocpWorkerJob = static_cast<eIocpWorkerJob>(dwTransferred);

            if (IocpWorkerJob == eIocpWorkerJob::OnExit && pSession == nullptr && pOverlapped == nullptr)
            {
                ::PostQueuedCompletionStatus(
                    _iocp,
                    static_cast<DWORD>(eIocpWorkerJob::OnExit),
                    reinterpret_cast<ULONG_PTR>(nullptr),
                    nullptr);

                return 0;
            }

            if (pSession != nullptr)
            {
                OnWorkerThreadBegin();
                switch (IocpWorkerJob)
                {
                case eIocpWorkerJob::OnDisconnect:
                    Disconnect();
                    break;

                case eIocpWorkerJob::OnSendPacket:
                    PRO_BEGIN(L"IocpWorker::SendPacket");
                    SendPost();
                    PRO_END(L"IocpWorker::SendPacket");
                    break;

                default:
                    PRO_BEGIN(L"IocpWorker::IOComplatedProc");
                    const auto Type = reinterpret_cast<OverlappedEx*>(pOverlapped)->Type;
                    switch (Type)
                    {
                    case OverlappedEx::eType::Receive:
                        RecvProc(dwTransferred);
                        break;

                    case OverlappedEx::eType::Send:
                        SendProc(dwTransferred);
                        break;

                    default:
                        LOG(eLogLevel::Error, L"Invalid Overlapped Type. (Type:%d)", Type);
                        break;
                    }
                    PRO_END(L"IocpWorker::IOComplatedProc");
                    break;
                }

                if (InterlockedDecrement16(&_session.IOCount) == 0)
                {
                    ReleaseSession();
                }

                OnWorkerThreadEnd();
            }
        }
        return 0;
    }
}