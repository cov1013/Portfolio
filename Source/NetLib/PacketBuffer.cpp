#include "PacketHeader.h"
#include "PacketFactory.h"
#include "PacketBuffer.h"

namespace cov1013
{
	void PacketBuffer::IncreseRefCount()
	{
		_InterlockedIncrement(&_refCount);
	}

	void PacketBuffer::DecreseRefCount()
	{
		const auto RefCount = _InterlockedDecrement(&_refCount);

		// 더이상 사용하는 곳이 없다면, 메모리풀에 반환
		if (RefCount <= 0)
		{
			Release();
			PacketFactory::Free(this);
		}
	}

	bool PacketBuffer::Encode()
	{
		// 이미 인코딩이 되어있다면 리턴(하나의 패킷을 여러 번 송신할 때 중복으로 암호화를 진행하면 안되므로 체크)
		const bool bIsEncoded = static_cast<bool>(InterlockedExchange8(&_bEncodeFlag, true));
		if (bIsEncoded == true)
		{
			return false;
		}

		// 1. 헤더 세팅 및 버퍼에 넣기
		NetPacketHeader header;
		header.Code = PacketFactory::GetPacketCode();
		header.PayloadLength = static_cast<unsigned short>(GetReadableSize());
		header.RandKey = static_cast<unsigned char>((rand() % 65536));
		header.CheckSum = MakeChecksum();
		memcpy_s(_pBuffer, sizeof(NetPacketHeader), (void*)&header, sizeof(NetPacketHeader));

		// 2. 인코딩 시작
		BYTE P = 0;
		BYTE E = 0;
		int iLen = 1 + GetReadableSize(); // Checksum + Payload
		char* readPos = _pBuffer + _readPos;
		for (int i = 0; i < iLen; i++)
		{
			// 처음 P와 E만 0, 0 이고 한 번 인코딩 한 다음부터는 이전의 P와 E를 가지고 인코딩한다.
			P = (BYTE)((readPos - 1)[i] ^ (P + header.RandKey + (i + 1)));
			E = (BYTE)(P ^ (E + PacketFactory::GetPacketKey() + (i + 1)));

			// 인코딩한 결과를 버퍼에 재삽입
			(readPos - 1)[i] = E;
		}

		return true;
	}

	bool PacketBuffer::Decode()
	{
		// 1) 버퍼의 시작점을 헤더로 해석
		NetPacketHeader* pHeader = reinterpret_cast<NetPacketHeader*>(_pBuffer);

		// 2) 패킷 코드 확인
		if (pHeader->Code != PacketFactory::GetPacketCode())
		{
			return false;
		}

		// 3) 디코딩
		BYTE D = 0;
		BYTE P = 0;
		BYTE E = 0;
		BYTE _E = 0;			// 이전값 저장용
		BYTE _P = 0;			// 이전값 저장용
		int iLen = GetReadableSize() + 1;	// Checksum + Payload

		char* pReadPos = _pBuffer + _readPos;
		for (int i = 0; i < iLen; i++)
		{
			E = (pReadPos - 1)[i];

			P = (BYTE)(E ^ (_E + PacketFactory::GetPacketKey() + (i + 1)));	// P = E ^ FK
			D = (BYTE)(P ^ (_P + pHeader->RandKey + (i + 1)));				// D = P ^ RK

			_E = (pReadPos - 1)[i];			// 이전 E 저장		
			_P = P;										// 이전 P 저장

			(pReadPos - 1)[i] = D;
		}

		// 4) 체크섬 생성 후 비교
		unsigned char Checksum = MakeChecksum();
		if (pHeader->CheckSum != Checksum)
		{
			return false;
		}

		// 5) 인코딩 여부 갱신
		_bEncodeFlag = false;

		return true;
	}

	bool PacketBuffer::SetNetPacketHeader()
	{
		return Encode();
	}

	void PacketBuffer::SetLanPacketHeader()
	{
		const auto PayloadSize = static_cast<unsigned short>(GetReadableSize());
		*reinterpret_cast<unsigned short*>(_pBuffer) = PayloadSize;
	}

	int PacketBuffer::GetNetPacketLength()
	{
		const unsigned short PayloadSize = static_cast<unsigned short>(GetReadableSize());

		return sizeof(NetPacketHeader) + PayloadSize;
	}

	int PacketBuffer::GetLanPacketLength()
	{
		const unsigned short PayloadSize = static_cast<unsigned short>(GetReadableSize());

		return sizeof(LanPacketHeader) + PayloadSize;
	}

	void PacketBuffer::Initial()
	{
		_readPos = 0;
		_writePos = 0;
		_refCount = 1;
		_bEncodeFlag = false;
	}

	void PacketBuffer::Release()
	{
		SerialBuffer::Clear();
	}

	unsigned char PacketBuffer::MakeChecksum()
	{
		int checksum = 0;
		const int Count = GetReadableSize();

		// 페이로드를 전부 더한 후 % 256
		char* readPos = _pBuffer + _readPos;
		for (int i = 0; i < Count; i++)
		{
			checksum += readPos[i];
		}

		return (unsigned char)(checksum % 256);
	}
}