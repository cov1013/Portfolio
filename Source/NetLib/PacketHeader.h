#pragma once

namespace cov1013
{
#pragma pack(push, 1)
	struct LanPacketHeader
	{
		unsigned short	PayloadLength = 0;
	};
#pragma pack(pop)

#pragma pack(push, 1)
	struct NetPacketHeader
	{
		unsigned char	Code = 0;
		unsigned short	PayloadLength = 0;
		unsigned char	RandKey = 0;
		unsigned char	CheckSum = 0;
	};
#pragma pack(pop)
}