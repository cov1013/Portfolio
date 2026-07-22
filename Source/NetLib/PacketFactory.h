#pragma once
#include "PacketBuffer.h"
#ifndef _TLS_MODE_
	#include "MemoryPool.h"
#else	
	#include "MemoryPool_TLS.h"
#endif

namespace cov1013
{
	class PacketFactory
	{
	public:
#ifndef _TLS_MODE_
		static void Initialize(const unsigned char PacketKey, const unsigned char PacketCode);
#else
		static void Initialize(const unsigned char PacketKey, const unsigned char PacketCode, const int ChunkElementCount = 200);
#endif
		static void Release();
		static PacketBuffer* AllocNetPacket();
		static PacketBuffer* AllocLanPacket();
		static bool Free(PacketBuffer* pPacket);

		inline static unsigned char GetPacketKey() { return _key; }
		inline static unsigned char GetPacketCode() { return _code; }
		inline static long GetPoolCapacity() { return _pPool->GetCapacity(); }
		inline static long GetPoolUseCount() { return _pPool->GetUseCount(); }

	private:
#ifndef _TLS_MODE_
		static MemoryPool<PacketBuffer>* _pPool;
#else
		static MemoryPool_TLS<PacketBuffer>* _pPool;
#endif
		static unsigned char _key;
		static unsigned char _code;
	};
}
