#include "PacketHeader.h"
#include "PacketFactory.h"

namespace cov1013
{
	unsigned char PacketFactory::_key = '\0';
	unsigned char PacketFactory::_code = '\0';
#ifndef _TLS_MODE_
	MemoryPool<PacketBuffer>* PacketFactory::_pPool = nullptr;
#else
	MemoryPool_TLS<PacketBuffer>* PacketFactory::_pPool = nullptr;
#endif

#ifndef _TLS_MODE_
	void PacketFactory::Initialize(const unsigned char PacketKey, const unsigned char PacketCode)
	{
		_key = PacketKey;
		_code = PacketCode;
		_pPool = new MemoryPool<PacketBuffer>(0, false);
	}
#else
	void PacketFactory::Initialize(const unsigned char PacketKey, const unsigned char PacketCode, const int ChunkElementCount)
	{
		_key = PacketKey;
		_code = PacketCode;
		_pPool = new MemoryPool_TLS<PacketBuffer>(0, false, ChunkElementCount);
	}
#endif

	void PacketFactory::Release()
	{
		delete _pPool;
	}

	PacketBuffer* PacketFactory::AllocLanPacket()
	{
		PacketBuffer* pPacket = _pPool->Alloc();
		pPacket->Initial();
		pPacket->_readPos += sizeof(LanPacketHeader);
		pPacket->_writePos += sizeof(LanPacketHeader);

		return pPacket;
	}

	PacketBuffer* PacketFactory::AllocNetPacket()
	{
		PacketBuffer* pPacket = _pPool->Alloc();
		pPacket->Initial();
		pPacket->_readPos += sizeof(NetPacketHeader);
		pPacket->_writePos += sizeof(NetPacketHeader);

		return pPacket;
	}

	bool PacketFactory::Free(PacketBuffer* pPacket)
	{
		return _pPool->Free(pPacket);
	}
}