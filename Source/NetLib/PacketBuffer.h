#pragma once
#pragma warning(disable: 4324)
#include "SerialBuffer.h"

namespace cov1013
{
	class PacketBuffer : public SerialBuffer
	{
		friend class PacketFactory;

	public:
		PacketBuffer() = default;
		PacketBuffer(const int Capacity) : SerialBuffer(Capacity) {};
		virtual ~PacketBuffer() = default;
		void IncreseRefCount();
		void DecreseRefCount();
		bool Encode();
		bool Decode();
		bool SetNetPacketHeader();
		void SetLanPacketHeader();
		int	 GetNetPacketLength();
		int	 GetLanPacketLength();

	private:
		void Initial();
		void Release();
		unsigned char MakeChecksum();

	private:
		alignas(64) volatile char _bEncodeFlag = 0;
		alignas(64) volatile unsigned long	_refCount = 0;
	};
}