#pragma once

namespace protocol
{
	namespace chat
	{
		enum class ePacketID : unsigned short
		{
			None = 0,
			ReqLogin = 1,
			ResLogin = 2,
			ReqMoveSector = 3,
			ResMoveSector = 4,
			ReqMessage = 5,
			ResMessage = 6,
			NtfHeartbeat = 7,
		};

		// 로그인 요청
		struct ReqLogin
		{
			long long	AccountNo = 0;
			wchar_t		ID[20] = {};
			wchar_t		Nickname[20] = {};
			char		SessionKey[64] = {};

			void ReadFrom(void* pDest);
			void WriteTo(void* pDest) const;
		};

		// 로그인 응답
		struct ResLogin
		{
			char		Status = 0;
			long long	AccountNo = 0;

			void ReadFrom(void* pDest);
			void WriteTo(void* pDest) const;
		};

		// 섹터 이동 요청
		struct ReqMoveSector
		{
			long long		AccountNo = 0;
			unsigned short	SectorX = 0;
			unsigned short	SectorY = 0;

			void ReadFrom(void* pDest);
			void WriteTo(void* pDest) const;
		};

		// 섹터 이동 결과
		struct ResMoveSector
		{
			long long		AccountNo = 0;
			unsigned short	SectorX = 0;
			unsigned short	SectorY = 0;

			void ReadFrom(void* pDest);
			void WriteTo(void* pDest) const;
		};

		// 채팅보내기 요청
		struct ReqMessage
		{
			long long		AccountNo = 0;
			unsigned short	MessageLen = 0;
			wchar_t			Message[128] = {};

			void ReadFrom(void* pDest);
			void WriteTo(void* pDest) const;
		};

		// 채팅보내기 응답 (다른 클라가 보낸 채팅도 이걸로 받음)
		struct ResMessage
		{
			long long		AccountNo = 0;
			wchar_t			ID[20] = { 0, };		// null 포함
			wchar_t			Nickname[20] = { 0, };	// null 포함
			unsigned short	MessageLen = 0;
			wchar_t			Message[128] = { 0, };	// null 미포함

			void ReadFrom(void* pDest);
			void WriteTo(void* pDest) const;
		};
	}
}