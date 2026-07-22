#pragma once
#pragma warning(disable:4324)

namespace cov1013
{
	struct NetStatistics
	{
		unsigned long		AcceptTPS = 0;
		unsigned __int64	AcceptTotal = 0;
		unsigned __int64	DisconnectTotal = 0;
		alignas(64) long	DisconnectTPS = 0;
		alignas(64) long	RecvPacketTPS = 0;
		alignas(64) long	SendPacketTPS = 0;
		alignas(64) long	ExpSendBytes = 0;
	};
}
