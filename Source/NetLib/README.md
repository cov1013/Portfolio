# NetLib ?¬ìš© ë§¤ë‰´??

Windows IOCP ê¸°ë°˜ ê³ ì„±???¤íŠ¸?Œí¬ ?¼ì´ë¸ŒëŸ¬ë¦¬ì…?ˆë‹¤.
`NetServer` / `NetClient`ë¥??ì†ë°›ì•„ ?´ë²¤???¸ë“¤?¬ë§Œ êµ¬í˜„?˜ë©´ ë©€?°ìŠ¤?ˆë“œ ?¤íŠ¸?Œí¬ ?…ì¶œ?¥ì„ ?¼ì´ë¸ŒëŸ¬ë¦¬ê? ?€??ì²˜ë¦¬?©ë‹ˆ??

## ëª©ì°¨

- [1. ?œì‘?˜ê¸°](#1-?œì‘?˜ê¸°)
- [2. ?œë²„ ë§Œë“¤ê¸?(NetServer)](#2-?œë²„-ë§Œë“¤ê¸?netserver)
- [3. ?´ë¼?´ì–¸??ë§Œë“¤ê¸?(NetClient)](#3-?´ë¼?´ì–¸??ë§Œë“¤ê¸?netclient)
- [4. ?¤ì • (NetConfig)](#4-?¤ì •-netconfig)
- [5. ?¨í‚· (PacketBuffer)](#5-?¨í‚·-packetbuffer)
- [6. ?´ë²¤???¸ë“¤???ˆí¼?°ìŠ¤](#6-?´ë²¤???¸ë“¤???ˆí¼?°ìŠ¤)
- [7. ì½˜í…ì¸??¤ë ˆ???°ë™ ?¨í„´](#7-ì½˜í…ì¸??¤ë ˆ???°ë™-?¨í„´)
- [8. DB ?œìŠ¤??(DBConnector)](#8-db-?œìŠ¤??dbconnector)
- [9. Redis ?œìŠ¤??(RedisConnector)](#9-redis-?œìŠ¤??redisconnector)
- [10. ë¶€ê°€ ê¸°ëŠ¥ (Logger / CrashDumper / Profiler)](#10-ë¶€ê°€-ê¸°ëŠ¥-logger--crashdumper--profiler)
- [11. ?ˆì œ ?„ë¡œ?íŠ¸](#11-?ˆì œ-?„ë¡œ?íŠ¸)

---

## 1. ?œì‘?˜ê¸°

### ë¹Œë“œ ?˜ê²½

| ??ª© | ?´ìš© |
|---|---|
| ?Œë«??| Windows (Win32 / x64) |
| ?´ì…‹ | Visual Studio 2022 (v143) |
| ë¬¸ì ì§‘í•© | Unicode (UTF-16) |
| ì¶œë ¥ | ?•ì  ?¼ì´ë¸ŒëŸ¬ë¦?(`.lib`) |
| êµ¬ì„± | Debug / DebugMT / Release |

### ?„ë¡œ?íŠ¸??ë§í¬?˜ê¸°

1. `NetLib.sln`??ë¹Œë“œ??`NetLib.lib`ë¥??ì„±?©ë‹ˆ??
   ?¤í–‰ ?„ë¡œ?íŠ¸?€ **?™ì¼??êµ¬ì„±(Debug/Release)ê³??Œë«??Win32/x64)** ?¼ë¡œ ë¹Œë“œ??libë¥?ë§í¬?´ì•¼ ?©ë‹ˆ??
2. ?¤í–‰ ?„ë¡œ?íŠ¸ ?¤ì •:
   - **ì¶”ê? ?¬í•¨ ?”ë ‰?°ë¦¬**: `..\NetLib`
   - **ì¶”ê? ?¼ì´ë¸ŒëŸ¬ë¦??”ë ‰?°ë¦¬**: NetLib ì¶œë ¥ ?´ë”
   - **ì¶”ê? ì¢…ì†??*: `NetLib.lib`
3. ?œìŠ¤???¼ì´ë¸ŒëŸ¬ë¦¬ëŠ” NetLib??`pch.h`?ì„œ `#pragma comment(lib, ...)`ë¡??ë™ ë§í¬?©ë‹ˆ??
   (`ws2_32.lib`, `Winmm.lib`, `Dbghelp.lib`, `Pdh.lib`, `odbc32.lib`, `odbccp32.lib`)

ëª¨ë“  ?´ë˜?¤ëŠ” `cov1013` ?¤ì„?¤í˜?´ìŠ¤???ˆìŠµ?ˆë‹¤.

```cpp
#include "NetServer.h"   // ?œë²„
#include "NetClient.h"   // ?´ë¼?´ì–¸??
using namespace cov1013;
```

---

## 2. ?œë²„ ë§Œë“¤ê¸?(NetServer)

`NetServer`ë¥??ì†ë°›ê³  ?œìˆ˜ ê°€???´ë²¤???¸ë“¤??8ê°œë? êµ¬í˜„?©ë‹ˆ??
?Œì¼“ ?ì„±, Accept, IOCP ?Œì»¤ ?¤ë ˆ??ê´€ë¦? ?¨í‚· ì¡°ë¦½/?”í˜¸?”ëŠ” ëª¨ë‘ NetLib ?´ë??ì„œ ì²˜ë¦¬?˜ë©°,
?Œìƒ ?œë²„??**?´ë²¤???¸ë“¤?¬ì—??ì½˜í…ì¸?ë¡œì§ë§?* ?‘ì„±?˜ë©´ ?©ë‹ˆ??

### 2-1. ?œë²„ ?´ë˜??? ì–¸

```cpp
#include "NetServer.h"
using namespace cov1013;

class EchoServer : public NetServer
{
public:
    EchoServer(const NetConfig& Config) : NetServer(Config) {}
    virtual ~EchoServer() = default;

protected:
    // ?‘ì† ?ˆìš© ?¬ë? ?ë‹¨ (false ë°˜í™˜ ???‘ì† ê±°ë? ??IP ?„í„°ë§??±ì— ?¬ìš©)
    virtual bool __stdcall OnConnectionRequest(const wchar_t* ConnectIP, const WORD ConnectPort) override
    {
        return true;
    }

    // ?¸ì…˜ ?‘ì† ?„ë£Œ
    virtual void __stdcall OnClientJoin(const SESSION_ID SessionID) override {}

    // ?¸ì…˜ ?‘ì† ?´ì œ
    virtual void __stdcall OnClientLeave(const SESSION_ID SessionID) override {}

    // ?¨í‚· ?˜ì‹  (?˜ì´ë¡œë“œ ?¨ìœ„ë¡?ì¡°ë¦½/?”ì½”???„ë£Œ???íƒœ)
    virtual void __stdcall OnRecv(const SESSION_ID SessionID, PacketBuffer* pRecvPacket) override
    {
        // ë°›ì? ?°ì´?°ë? ê·¸ë?ë¡??Œë ¤ì£¼ëŠ” ?ì½” ì²˜ë¦¬
        PacketBuffer* pSendPacket = PacketBuffer::AllocLanPacket();
        {
            pSendPacket->DoWrite(pRecvPacket->GetReadPos(), pRecvPacket->GetReadableSize());
            SendPacket(SessionID, pSendPacket);
        }
        pSendPacket->DecreseRefCount();  // ?¬ìš© ??ë°˜ë“œ??ì°¸ì¡° ì¹´ìš´??ë°˜í™˜
    }

    virtual void __stdcall OnSend(const SESSION_ID SessionID, const DWORD dwTransferred) override {}
    virtual void __stdcall OnWorkerThreadBegin() override {}
    virtual void __stdcall OnWorkerThreadEnd() override {}
    virtual void __stdcall OnError(const eErrorCode eErrCode, const SESSION_ID SessionID) override {}
};
```

### 2-2. ?œë²„ ?¤í–‰ (main)

```cpp
#include "EchoServer.h"

int main()
{
    // 1. XML ?¤ì • ë¡œë“œ
    cov1013::NetConfig config;
    if (config.Load(L"..\\Config\\EchoServer.xml") == false)
    {
        wprintf_s(L"Failure load EchoServer.xml");
        return 0;
    }

    // 2. ë¶€ê°€ ?œë¹„??ì´ˆê¸°??(ë¡œê±° / ?¬ë˜???¤í”„ / ?„ë¡œ?Œì¼??
    if (cov1013::Logger::Initialize(config.GetLogDirectoryPath().c_str(), cov1013::eLogLevel::Debug) == false)
        return 0;
    if (cov1013::CrashDumper::Initialize(config.GetDumpDirectoryPath().c_str()) == false)
        return 0;
    if (cov1013::Profiler::Initialize(config.GetProfilingDataDirectoryPath().c_str(), cov1013::Profiler::eUnit::MilliSecond) == false)
        return 0;

    // 3. ?œë²„ ?ì„± ë°??œì‘
    auto pServer = std::make_unique<EchoServer>(config);
    if (pServer->Start() == false)
        return 0;

    // 4. ë©”ì¸ ë£¨í”„ (???…ë ¥ ?œì–´)
    bool bRunFlag = true;
    while (bRunFlag)
    {
        const wchar_t InputKey = _getwch();
        switch (InputKey)
        {
        case L'q':
        case L'Q':
            pServer->Quit();
            bRunFlag = false;
            break;
        default:
            pServer->Control(InputKey);
            break;
        }
    }

    cov1013::Profiler::Release();
    return 0;
}
```

### 2-3. ?œë²„ ?œì‘ ?œí€€??

`Start()` ?¸ì¶œ ???´ë??ì„œ ?„ë˜ ?œì„œë¡?ì´ˆê¸°?”ë©?ˆë‹¤.

```
InitializeConfig ??InitializeNetwork(WSA) ??InitializeListenSocket
??InitializeSessions(?¸ì…˜ ?€ ? í• ?? ??InitializeIocpWorker(?Œì»¤ ?¤ë ˆ???ì„±)
??InitializeAcceptor(Accept ?¤ë ˆ???œì‘)
```

### 2-4. ì£¼ìš” ê³µê°œ API

| ?¨ìˆ˜ | ?¤ëª… |
|---|---|
| `bool Start()` | ?œë²„ ?œì‘ |
| `void Stop()` / `void Quit()` | ?œë²„ ì¤‘ì? / ì¢…ë£Œ |
| `bool Restart()` | ?¬ì‹œ??|
| `bool SendPacket(SESSION_ID, PacketBuffer*)` | ì§€???¸ì…˜???¨í‚· ?¡ì‹  |
| `bool Disconnect(SESSION_ID)` | ì§€???¸ì…˜ ê°•ì œ ì¢…ë£Œ |
| `bool IsActive()` | ?œë²„ ?™ì‘ ?¬ë? |
| `int GetSessionCount()` | ?„ì¬ ?‘ì† ?¸ì…˜ ??|
| `const NetStatistics& GetStatistics()` | Accept/Recv/Send TPS ???µê³„ |
| `void ResetStatistics()` | ?µê³„ ì´ˆê¸°??|

---

## 3. ?´ë¼?´ì–¸??ë§Œë“¤ê¸?(NetClient)

`NetClient`???™ì¼??ë°©ì‹?¼ë¡œ ?ì†ë°›ì•„ ?¬ìš©?©ë‹ˆ?? ?”ë? ?´ë¼?´ì–¸?? ?œë²„ ê°??µì‹  ?±ì— ?¬ìš©?©ë‹ˆ??

```cpp
#include "NetClient.h"
using namespace cov1013;

class GameNetwork : public NetClient
{
public:
    GameNetwork() : NetClient(eNetworkType::NetNetwork) {}

protected:
    virtual void __stdcall OnEnterJoinServer() override
    {
        // ?œë²„ ?‘ì† ?±ê³µ ??ë¡œê·¸???¨í‚· ?„ì†¡ ??
    }

    virtual void __stdcall OnLeaveServer() override
    {
        // ?œë²„?€???°ê²° ?Šê?
    }

    virtual void __stdcall OnRecv(PacketBuffer* pRecvPacket) override
    {
        // ?˜ì‹  ?¨í‚· ì²˜ë¦¬
        WORD messageType;
        *pRecvPacket >> messageType;
        // switch (messageType) { ... }
    }

    virtual void __stdcall OnSend(const DWORD dwTransferred) override {}
    virtual void __stdcall OnWorkerThreadBegin() override {}
    virtual void __stdcall OnWorkerThreadEnd() override {}
    virtual void __stdcall OnError(const eErrorCode eErrCode, const wchar_t* Message) override {}
};
```

```cpp
// ?¬ìš©
GameNetwork network;
network.Start(config);      // IOCP ?Œì»¤ ì´ˆê¸°??+ ?‘ì†
// ...
network.SendPacket(pPacket);
network.Disconnect();       // ?°ê²°ë§??Šê¸°
network.Reconnect();        // ?¬ì ‘??
network.Quit();             // ?„ì „ ì¢…ë£Œ
```

---

## 4. ?¤ì • (NetConfig)

?œë²„/?´ë¼?´ì–¸???¤ì •?€ XML ?Œì¼ë¡?ê´€ë¦¬í•˜ë©?`NetConfig::Load()`ë¡??½ìŠµ?ˆë‹¤.
ì½”ë“œ?ì„œ `Set*()` ?¨ìˆ˜ë¡?ì§ì ‘ ì§€?•í•  ?˜ë„ ?ˆìŠµ?ˆë‹¤.

### 4-1. ?¤ì • XML ?ˆì‹œ

```xml
<?xml version="1.0" encoding="UTF-8"?>
<NetConfig>
    <networkType>Net</networkType>
    <ip>0.0.0.0</ip>
    <port>15000</port>
    <bEnableZeroCopySend>false</bEnableZeroCopySend>
    <bEnableNagle>false</bEnableNagle>
    <bEnableKeepAlive>false</bEnableKeepAlive>
    <bEnableIocpWorkerSend>false</bEnableIocpWorkerSend>
    <iocpWorkerCount>4</iocpWorkerCount>
    <iocpWorkerConcurrentCount>3</iocpWorkerConcurrentCount>
    <sessionCapacity>10000</sessionCapacity>
    <packetCapacity>500</packetCapacity>
    <packetMaxSize>500</packetMaxSize>
    <packetCode>119</packetCode>
    <packetKey>50</packetKey>
    <timeoutDisconnect>30000</timeoutDisconnect>
    <dumpDirectoryPath>./_Dumps/ChatServer</dumpDirectoryPath>
    <logDirectoryPath>./_Logs/ChatServer</logDirectoryPath>
    <profilingDataDirectoryPath>./_ProfilingData/ChatServer</profilingDataDirectoryPath>
</NetConfig>
```

### 4-2. ?Œë¼ë¯¸í„° ?¤ëª…

| ?Œë¼ë¯¸í„° | ?¤ëª… |
|---|---|
| `networkType` | `Lan`(?´ë?ë§? ê²½ëŸ‰ ?¤ë”) / `Net`(?¸ë?ë§? ?”í˜¸???¤ë”). [5. ?¨í‚·](#5-?¨í‚·-packetbuffer) ì°¸ì¡° |
| `ip` / `port` | ë°”ì¸??IP / ?¬íŠ¸ |
| `bEnableZeroCopySend` | ?¡ì‹  Zero-Copy(SO_SNDBUF=0) ?¬ìš© ?¬ë? |
| `bEnableNagle` | Nagle ?Œê³ ë¦¬ì¦˜ ?¬ìš© ?¬ë? (ì§€??ìµœì†Œ?”ê? ëª©ì ?´ë©´ `false`) |
| `bEnableKeepAlive` | TCP Keep-Alive ?¬ìš© ?¬ë? |
| `bEnableIocpWorkerSend` | ?¡ì‹ ??IOCP ?Œì»¤?ì„œ ?˜í–‰? ì? ?¬ë? |
| `iocpWorkerCount` | IOCP ?Œì»¤ ?¤ë ˆ????|
| `iocpWorkerConcurrentCount` | IOCP ?™ì‹œ ?¤í–‰ ?¤ë ˆ????(ë³´í†µ ì½”ì–´ ???´í•˜) |
| `sessionCapacity` | ìµœë? ?™ì‹œ ?¸ì…˜ ??(?¸ì…˜ ?€ ? í• ?? ìµœë? 20000) |
| `packetCapacity` | ?¨í‚· ?¤ë¸Œ?íŠ¸ ?€ ì´ˆê¸° ?¬ê¸° |
| `packetMaxSize` | ?¨ì¼ ?¨í‚· ìµœë? ?¬ê¸° (ë°”ì´?? |
| `packetCode` | Net ?¨í‚· ?¤ë” ê²€ì¦?ì½”ë“œ (?œë²„/?´ë¼?´ì–¸???™ì¼ ê°??„ìˆ˜) |
| `packetKey` | Net ?¨í‚· ?¸ì½”????(?œë²„/?´ë¼?´ì–¸???™ì¼ ê°??„ìˆ˜) |
| `timeoutDisconnect` | ë¬´ì‘???¸ì…˜ ?€?„ì•„??(ms) |
| `dumpDirectoryPath` | ?¬ë˜???¤í”„ ?€??ê²½ë¡œ |
| `logDirectoryPath` | ë¡œê·¸ ?Œì¼ ?€??ê²½ë¡œ |
| `profilingDataDirectoryPath` | ?„ë¡œ?Œì¼ë§??°ì´???€??ê²½ë¡œ |

---

## 5. ?¨í‚· (PacketBuffer)

### 5-1. ?¤ë” ?¬ë§·

NetLib????ê°€ì§€ ?¨í‚· ëª¨ë“œë¥??œê³µ?©ë‹ˆ?? `networkType` ?¤ì •ê³??¼ì¹˜?˜ëŠ” Alloc ?¨ìˆ˜ë¥??¬ìš©?´ì•¼ ?©ë‹ˆ??

```cpp
#pragma pack(push, 1)
struct LanPacketHeader          // Lan ëª¨ë“œ ???´ë?ë§??œë²„ ê°??µì‹ ??
{
    unsigned short PayloadLength;   // 2 bytes
};

struct NetPacketHeader          // Net ëª¨ë“œ ???¸ë?(?´ë¼?´ì–¸?? ?µì‹ ??
{
    unsigned char  Code;            // ê³ ì • ì½”ë“œ (packetCode) ??? íš¨ ?¨í‚· ê²€ì¦?
    unsigned short PayloadLength;
    unsigned char  RandKey;         // ?œë¤ ?????¨í‚·ë§ˆë‹¤ ?¤ë¥¸ ?¸ì½”??ê²°ê³¼
    unsigned char  CheckSum;        // ?˜ì´ë¡œë“œ ì²´í¬????ë³€ì¡?ê°ì?
};
#pragma pack(pop)
```

- **Lan**: ?¤ë” 2ë°”ì´?? ?¸ì½”???†ìŒ. ? ë¢° ê°€?¥í•œ ?´ë?ë§ì—??ìµœì†Œ ?¤ë²„?¤ë“œ.
- **Net**: ê³ ì • ì½”ë“œ + ?œë¤ ??ê¸°ë°˜ ?¸ì½”??+ ì²´í¬?¬ìœ¼ë¡??¨í‚· ?„ë?ì¡°ë? ë°©ì–´.
  ?¸ì½”???”ì½”?©ì? ?¡ìˆ˜????NetLib ?´ë??ì„œ ?ë™ ?˜í–‰?©ë‹ˆ??

### 5-2. ?¨í‚· ? ë‹¹ê³?ì°¸ì¡° ì¹´ìš´??

`PacketBuffer`??ë©”ëª¨ë¦??€?ì„œ ? ë‹¹?˜ë©° **ì°¸ì¡° ì¹´ìš´?¸ë¡œ ?˜ëª…??ê´€ë¦?*?©ë‹ˆ??

```cpp
PacketBuffer* pPacket = PacketBuffer::AllocLanPacket();   // ?ëŠ” AllocNetPacket()
// ... ?°ì´???°ê¸°, SendPacket() ...
pPacket->DecreseRefCount();   // ?¬ìš©???ë‚˜ë©?ë°˜ë“œ???¸ì¶œ (?€ë¡?ë°˜í™˜)
```

ê·œì¹™:

1. `Alloc*()` ?¼ë¡œ ë°›ì? ?¨í‚·?€ ?¬ìš© ??ë°˜ë“œ??`DecreseRefCount()`ë¥??¸ì¶œ?©ë‹ˆ??
   `SendPacket()`?€ ?´ë??ì„œ ?ì²´?ìœ¼ë¡?ì°¸ì¡°ë¥??¡ìœ¼ë¯€ë¡? ?¡ì‹  ?„ë£Œë¥?ê¸°ë‹¤ë¦??„ìš” ?†ì´ ë°”ë¡œ ë°˜í™˜?´ë„ ?ˆì „?©ë‹ˆ??
2. `OnRecv()`ë¡??„ë‹¬???˜ì‹  ?¨í‚·?€ ?¸ë“¤??ë¦¬í„´ ??NetLibê°€ ?ë™ ë°˜í™˜?©ë‹ˆ??
   **?¤ë¥¸ ?¤ë ˆ?œë¡œ ?˜ê²¨???˜ì¤‘??ì²˜ë¦¬?˜ë ¤ë©?* `IncreseRefCount()`ë¡?ì°¸ì¡°ë¥??¡ê³ , ì²˜ë¦¬ ?„ë£Œ ??`DecreseRefCount()`ë¥??¸ì¶œ?´ì•¼ ?©ë‹ˆ??
3. ?˜ë‚˜???¨í‚·???¬ëŸ¬ ?¸ì…˜??ë¸Œë¡œ?œìº?¤íŠ¸???Œë„ ?¨í‚·??ë³µì‚¬???„ìš” ?†ì´ ê·¸ë?ë¡?`SendPacket()`??ë°˜ë³µ ?¸ì¶œ?˜ë©´ ?©ë‹ˆ??

### 5-3. ì§ë ¬??/ ??§?¬í™”

`PacketBuffer`??`SerialBuffer`ë¥??ì†?˜ë?ë¡?`<<` / `>>` ?°ì‚°?ì? `DoWrite` / `DoRead`ë¥??¬ìš©?©ë‹ˆ??

```cpp
// [?¡ì‹ ì¸? ë©”ì‹œì§€ ?€??+ ?°ì´??ì§ë ¬??
PacketBuffer* pPacket = PacketBuffer::AllocNetPacket();
{
    const WORD    messageType = PACKET_CS_CHAT_REQ_MESSAGE;
    const __int64 accountNo   = 10001;

    *pPacket << messageType << accountNo;
    pPacket->DoWrite(reinterpret_cast<const char*>(message), messageLen);  // ê°€ë³€ ê¸¸ì´ ?°ì´??

    SendPacket(SessionID, pPacket);
}
pPacket->DecreseRefCount();
```

```cpp
// [?˜ì‹ ì¸? OnRecv ?ˆì—????§?¬í™”
void MyServer::OnRecv(const SESSION_ID SessionID, PacketBuffer* pRecvPacket)
{
    WORD messageType;
    *pRecvPacket >> messageType;

    switch (messageType)
    {
    case PACKET_CS_CHAT_REQ_MESSAGE:
        {
            __int64 accountNo;
            *pRecvPacket >> accountNo;

            const int  remainLength = pRecvPacket->GetReadableSize();
            const char* pRemainData = pRecvPacket->GetReadPos();
            // ...
        }
        break;
    }
}
```

---

## 6. ?´ë²¤???¸ë“¤???ˆí¼?°ìŠ¤

ëª¨ë“  ?¸ë“¤?¬ëŠ” **IOCP ?Œì»¤ ?¤ë ˆ?œì—??ë³‘ë ¬ë¡??¸ì¶œ**?©ë‹ˆ??
ê³µìœ  ?ì› ?‘ê·¼???„ìš”?˜ë©´ ?™ê¸°?”í•˜ê±°ë‚˜, [7. ì½˜í…ì¸??¤ë ˆ???°ë™ ?¨í„´](#7-ì½˜í…ì¸??¤ë ˆ???°ë™-?¨í„´)ì²˜ëŸ¼ ?¨ì¼ ì½˜í…ì¸??¤ë ˆ?œë¡œ ?¡ì„ ?˜ê¸°??ë°©ì‹??ê¶Œì¥?©ë‹ˆ??

### NetServer

| ?¸ë“¤??| ?¸ì¶œ ?œì  |
|---|---|
| `OnConnectionRequest(IP, Port)` | Accept ì§í›„. `false` ë°˜í™˜ ???‘ì† ê±°ë? (IP ?„í„°ë§? |
| `OnClientJoin(SessionID)` | ?¸ì…˜ ?±ë¡ ?„ë£Œ. ì²??¨í‚· ?¡ì‹  ê°€???œì  |
| `OnClientLeave(SessionID)` | ?¸ì…˜ ?´ì œ ?„ë£Œ. ì½˜í…ì¸??ì› ?•ë¦¬ ?œì  |
| `OnRecv(SessionID, pPacket)` | ?„ì„±???¨í‚· 1ê°??˜ì‹  (ì¡°ë¦½/?”ì½”???„ë£Œ ?íƒœ) |
| `OnSend(SessionID, Transferred)` | ?¡ì‹  ?„ë£Œ ?µì? |
| `OnWorkerThreadBegin/End()` | IOCP ?Œì»¤ ?¤ë ˆ???œì‘/ì¢…ë£Œ (TLS ì´ˆê¸°???? |
| `OnError(ErrCode, SessionID)` | ?¼ì´ë¸ŒëŸ¬ë¦??´ë? ?¤ë¥˜ ?µì? |

### NetClient

| ?¸ë“¤??| ?¸ì¶œ ?œì  |
|---|---|
| `OnEnterJoinServer()` | ?œë²„ ?‘ì† ?±ê³µ |
| `OnLeaveServer()` | ?œë²„ ?°ê²° ?´ì œ |
| `OnRecv(pPacket)` | ?¨í‚· ?˜ì‹  |
| `OnSend(Transferred)` | ?¡ì‹  ?„ë£Œ |
| `OnError(ErrCode, Message)` | ?¤ë¥˜ ?µì? |

---

## 7. ì½˜í…ì¸??¤ë ˆ???°ë™ ?¨í„´

IOCP ?Œì»¤?ì„œ ì½˜í…ì¸?ë¡œì§??ì§ì ‘ ì²˜ë¦¬?˜ë©´ ??ê²½í•©??ë°œìƒ?˜ë?ë¡?
ê·œëª¨ ?ˆëŠ” ?œë²„(ChatServer ????**?˜ì‹  ?¨í‚·??ì½˜í…ì¸??ë¡œ ?˜ê²¨ ?¨ì¼ UpdateThread?ì„œ ì²˜ë¦¬**?˜ëŠ” êµ¬ì¡°ë¥??¬ìš©?©ë‹ˆ??
??êµ¬ì¡°?ì„œ??ì½˜í…ì¸?ë¡œì§???™ê¸°??ê°ì²´ê°€ ?„ìš” ?†ìŠµ?ˆë‹¤.

```
IOCP Worker(N) ?€ OnRecv ?€??Job Queue ?€??Update Thread(1) ?€??DB/Redis Connector(?Œì»¤ ?€)
```

```cpp
// ChatServer.cpp ???˜ì‹  ?¨í‚·??ì½˜í…ì¸??¤ë ˆ?œë¡œ ?„ë‹¬
void ChatServer::OnRecv(const SESSION_ID SessionID, PacketBuffer* pRecvPacket)
{
    // 1) ?¤ë¥¸ ?¤ë ˆ?œë¡œ ?˜ê¸°ë¯€ë¡?RefCount ì¦ê? (ê·¸ëƒ¥ ë¦¬í„´?˜ë©´ NetLibê°€ ë°˜í™˜?´ë²„ë¦?
    pRecvPacket->IncreseRefCount();

    // 2) ì½˜í…ì¸??¤ë ˆ?œë¡œ ?˜ê¸´?? (UpdateThread?ì„œ ì²˜ë¦¬ ??DecreseRefCount)
    _pUpdateThread->PostJob(UpdateThread::Job::eType::OnReceivePacket, SessionID, pRecvPacket);
}

void ChatServer::OnClientLeave(const SESSION_ID SessionID)
{
    _pUpdateThread->PostJob(UpdateThread::Job::eType::OnDisconnect, SessionID, nullptr);
}
```

```cpp
// ChatServer.cpp ??ì½˜í…ì¸??¤ë ˆ?œë? ?¤íŠ¸?Œí¬ë³´ë‹¤ ë¨¼ì? ?œì‘
bool ChatServer::Start()
{
    _pActorManager  = new ActorManager;
    _pSectorManager = new SectorManager(this);
    _pUpdateThread  = new UpdateThread(this, _pActorManager, _pSectorManager);

    // OnClientJoin ì½œë°±??ê³§ë°”ë¡??¤ì–´?????ˆìœ¼ë¯€ë¡?ì½˜í…ì¸??¤ë ˆ??ë¨¼ì? ê°€??
    _pUpdateThread->Run();

    if (NetServer::Start() == false)
    {
        _pUpdateThread->Stop();
        return false;
    }
    return true;
}
```

---

## 8. DB ?œìŠ¤??(DBConnector)

`Common/DBConnector.h`??ODBC ê¸°ë°˜ **ë¹„ë™ê¸?DB ?Œì»¤ ?¤ë ˆ???€**?…ë‹ˆ??
ì½˜í…ì¸??¤ë ˆ?œê? ë¸”ë¡œ???†ì´ ì¿¼ë¦¬ë¥??¤í–‰?????ˆë„ë¡? ??Job) ?¨ìœ„ë¡??ì— ?£ìœ¼ë©??Œì»¤ê°€ ?„ìš© ì»¤ë„¥?˜ìœ¼ë¡?ì²˜ë¦¬?©ë‹ˆ??

```
Update Thread ??PostJob() ??Job Queue ??DB Worker(Nê°? ì»¤ë„¥??1ê°œì”©) ??DB
```

### 8-1. ?œì‘ / ì¢…ë£Œ

```cpp
#include "DBConnector.h"

// ?Œì»¤ ?˜ë§Œ??ì»¤ë„¥?˜ì´ ?ì„±?©ë‹ˆ?? (MAX_WORKER_COUNT = 16)
auto pDBConnector = std::make_unique<cov1013::DBConnector>(
    L"DRIVER={SQL Server};SERVER=127.0.0.1,1433;DATABASE=gamedb;UID=sa;PWD=password;",
    4 /*WorkerCount*/);

if (pDBConnector->Start() == false)
    return false;

// ... ?œë²„ ì¢…ë£Œ ??
pDBConnector->Stop();
```

### 8-2. ???•ì˜?€ ?¤í–‰

`IDBJob`???ì†ë°›ì•„ `OnExecute()`??ì¿¼ë¦¬ ?¤í–‰ + ê²°ê³¼ ?µì?ë¥?êµ¬í˜„?©ë‹ˆ??
**?¡ì˜ ?Œìœ ê¶Œì? PostJob ??ì»¤ë„¥?°ë¡œ ?´ì „**?˜ë©°, ?¤í–‰ ?„ë£Œ ???´ë??ì„œ delete ?©ë‹ˆ??
ì²˜ë¦¬ ê²°ê³¼??`OnExecute()` ?ˆì—??ì½˜í…ì¸???UpdateThread ??ë¡??¤ì‹œ Post?˜ì—¬ ?µì??©ë‹ˆ??

```cpp
// ë¡œê·¸??ë¡œê·¸ë¥??¨ê¸°ê³?ê²°ê³¼ë¥?ì½˜í…ì¸??¤ë ˆ?œë¡œ ?µì??˜ëŠ” ??
class LoginLogJob : public cov1013::IDBJob
{
public:
    LoginLogJob(UpdateThread* pUpdateThread, const SESSION_ID SessionID, const __int64 AccountNo)
        : _pUpdateThread(pUpdateThread), _sessionID(SessionID), _accountNo(AccountNo) {}

    virtual void OnExecute(cov1013::DBConnection& Connection) override
    {
        // printf ?¤í????Œë¼ë¯¸í„° ì¿¼ë¦¬
        Connection.Execute(L"INSERT INTO logintbl (accountno) VALUES (%lld)", _accountNo);

        // ì²˜ë¦¬ ê²°ê³¼ë¥?ì½˜í…ì¸??ë¡œ ?µì? (DB ?Œì»¤?ì„œ ì½˜í…ì¸?ë¡œì§??ì§ì ‘ ?˜í–‰?˜ì? ?ŠëŠ”??
        _pUpdateThread->PostJob(UpdateThread::Job::eType::OnLoginLogSaved, _sessionID, nullptr);
    }

private:
    UpdateThread*    _pUpdateThread;
    const SESSION_ID _sessionID;
    const __int64    _accountNo;
};

// ì½˜í…ì¸??¤ë ˆ?œì—??ë¹„ë™ê¸??¤í–‰
_pDBConnector->PostJob(new LoginLogJob(_pUpdateThread, SessionID, accountNo));
```

### 8-3. SELECT ê²°ê³¼ ?½ê¸°

```cpp
virtual void OnExecute(cov1013::DBConnection& Connection) override
{
    if (Connection.Execute(L"SELECT id, nickname FROM account WHERE accountno = %lld", _accountNo))
    {
        while (Connection.Fetch())
        {
            __int64 id;
            wchar_t nickname[32];
            Connection.GetData(1, &id);              // ì»¬ëŸ¼ ?¸ë±?¤ëŠ” ODBC ê·œì•½?€ë¡?1ë¶€??
            Connection.GetData(2, nickname, 32);
        }
        Connection.FreeStatement();                   // ?¤í…Œ?´íŠ¸ë¨¼íŠ¸ ë°˜í™˜
    }
}
```

`DBConnection` ì£¼ìš” API: `Execute(Format, ...)`, `Fetch()`, `GetData(ColumnIndex, ...)`(int/`__int64`/float/double/wstring), `GetAffectedRowCount()`, `FreeStatement()`, `Reconnect()`

---

## 9. Redis ?œìŠ¤??(RedisConnector)

`Common/RedisConnector.h`??RESP2 ?„ë¡œ? ì½œ ê¸°ë°˜ **ë¹„ë™ê¸?Redis ?Œì»¤ ?¤ë ˆ???€**ë¡? DBConnector?€ ?™ì¼??????êµ¬ì¡°?…ë‹ˆ??
ë¡œê·¸??? í° ê²€ì¦? ?¸ì…˜ ìºì‹œ ?±ì— ?¬ìš©?©ë‹ˆ??

### 9-1. ?œì‘ / ì¢…ë£Œ

```cpp
#include "RedisConnector.h"

auto pRedisConnector = std::make_unique<cov1013::RedisConnector>(
    "127.0.0.1", 6379, 4 /*WorkerCount*/, "" /*Password*/, 0 /*DBIndex*/);

if (pRedisConnector->Start() == false)
    return false;

// ... ?œë²„ ì¢…ë£Œ ??
pRedisConnector->Stop();
```

### 9-2. ë¡œê·¸??? í° ê²€ì¦??ˆì‹œ

```cpp
class TokenVerifyJob : public cov1013::IRedisJob
{
public:
    TokenVerifyJob(UpdateThread* pUpdateThread, const SESSION_ID SessionID,
                   const __int64 AccountNo, const std::string& Token)
        : _pUpdateThread(pUpdateThread), _sessionID(SessionID)
        , _accountNo(AccountNo), _token(Token) {}

    virtual void OnExecute(cov1013::RedisConnection& Connection) override
    {
        std::string token;
        const bool bSuccess = Connection.Get("token:" + std::to_string(_accountNo), &token)
                            && token == _token;

        // ê²€ì¦?ê²°ê³¼ë¥?ì½˜í…ì¸??ë¡œ ?µì? ??UpdateThreadê°€ ë¡œê·¸???‘ë‹µ ?¨í‚· ?¡ì‹ 
        _pUpdateThread->PostJob(bSuccess
            ? UpdateThread::Job::eType::OnLoginSuccess
            : UpdateThread::Job::eType::OnLoginFail,
            _sessionID, nullptr);
    }

private:
    UpdateThread*    _pUpdateThread;
    const SESSION_ID _sessionID;
    const __int64    _accountNo;
    const std::string _token;
};

// OnRecv ??UpdateThread?ì„œ ë¡œê·¸???”ì²­ ì²˜ë¦¬ ??
_pRedisConnector->PostJob(new TokenVerifyJob(_pUpdateThread, SessionID, accountNo, token));
```

`RedisConnection` ì£¼ìš” API: `Get`, `Set`, `SetEx`, `Del`, `Expire`, `Ping`, ?„ì˜ ì»¤ë§¨?œìš© `Execute(Reply, Arguments)`

---

## 10. ë¶€ê°€ ê¸°ëŠ¥ (Logger / CrashDumper / Profiler)

?œë²„ ?œì‘ ?„ì— ì´ˆê¸°?”í•©?ˆë‹¤. ê²½ë¡œ??ë³´í†µ NetConfig?ì„œ ?¨ê»˜ ê´€ë¦¬í•©?ˆë‹¤.

```cpp
// ?Œì¼ ë¡œê±° ???ˆë²¨ ?„í„°ë§?ì§€??(Debug / ...)
cov1013::Logger::Initialize(config.GetLogDirectoryPath().c_str(), cov1013::eLogLevel::Debug);
LOG(eLogLevel::Debug, L"[%s] OnClientLeave. (SessionID:%lld)\n", __FUNCTIONW__, SessionID);

// ?¬ë˜???¤í”„ ??ë¯¸ì²˜ë¦??ˆì™¸ ë°œìƒ ??ì§€??ê²½ë¡œ???€ ?¤í”„ ?ì„±
cov1013::CrashDumper::Initialize(config.GetDumpDirectoryPath().c_str());

// ?„ë¡œ?Œì¼????êµ¬ê°„ ?±ëŠ¥ ì¸¡ì • ?°ì´???€??
cov1013::Profiler::Initialize(config.GetProfilingDataDirectoryPath().c_str(), cov1013::Profiler::eUnit::MilliSecond);
// ...
cov1013::Profiler::Release();   // ì¢…ë£Œ ??
```

`NetServer::GetStatistics()`ë¡??»ëŠ” `NetStatistics`(AcceptTPS, RecvPacketTPS, SendPacketTPS ??ë¥?
ëª¨ë‹ˆ?°ë§ ?¤ë ˆ?œì—??ì£¼ê¸°?ìœ¼ë¡?ì¶œë ¥?˜ë©´ ?œë²„ ?íƒœë¥??¤ì‹œê°??•ì¸?????ˆìŠµ?ˆë‹¤. (EchoServer??`MonitorThread` ì°¸ê³ )

---

## 11. ?ˆì œ ?„ë¡œ?íŠ¸

| ?„ë¡œ?íŠ¸ | ?´ìš© |
|---|---|
| [EchoServer](../EchoServer) | ìµœì†Œ êµ¬ì„± ?ˆì œ ??NetServer ?ì†, Lan ëª¨ë“œ, ?ì½” ì²˜ë¦¬ |
| [ChatServer](../ChatServer) | ?¤ì „ êµ¬ì„± ?ˆì œ ??Net ëª¨ë“œ, UpdateThread ì½˜í…ì¸??? Sector ê¸°ë°˜ ì±„íŒ… |
| [GameServer](../GameServer) / [GameClient](../GameClient) | NetClient ?¬ìš© ?ˆì œ ?¬í•¨ |
| `Test/DummyClient_*` | ?”ë? ?´ë¼?´ì–¸??ë¶€???ŒìŠ¤??|
| `Test/StressTest_EchoServer` | ?¤íŠ¸?ˆìŠ¤ ?ŒìŠ¤??ë°©ë²•ë¡?ë°?ê²°ê³¼ |

?¤ì • ?Œì¼ ?˜í”Œ?€ `Config/*.xml`???ˆìŠµ?ˆë‹¤.

> **ì°¸ê³ **: `MMOServer` / `MMOSession`(Auth/Game ?¨ê³„ ë¶„ë¦¬ êµ¬ì¡°)?€ ë³„ë„ ?„í‚¤?ì²˜???œë²„ ?´ë˜?¤ë¡œ,
> ?¼ë°˜?ì¸ ?©ë„?ëŠ” `NetServer` ?¬ìš©??ê¶Œì¥?©ë‹ˆ??
