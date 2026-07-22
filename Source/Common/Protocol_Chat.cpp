#include "Protocol_Chat.h"
#include "../NetLib/PacketBuffer.h"

namespace protocol
{
	namespace chat
	{
		void ReqLogin::ReadFrom(void* pDest)
		{
			cov1013::PacketBuffer* pPacket = static_cast<cov1013::PacketBuffer*>(pDest);
			*pPacket >> AccountNo;
			pPacket->DoRead(ID, sizeof(ID));
			pPacket->DoRead(Nickname, sizeof(Nickname));
			pPacket->DoRead(SessionKey, sizeof(SessionKey));
		}

		void ReqLogin::WriteTo(void* pDest) const
		{
			cov1013::PacketBuffer* pPacket = static_cast<cov1013::PacketBuffer*>(pDest);
			*pPacket << ePacketID::ReqLogin;
			*pPacket << AccountNo;
			pPacket->DoWrite(ID, sizeof(ID));
			pPacket->DoWrite(Nickname, sizeof(Nickname));
			pPacket->DoWrite(SessionKey, sizeof(SessionKey));
		}

		void ResLogin::ReadFrom(void* pDest)
		{
			cov1013::PacketBuffer* pPacket = static_cast<cov1013::PacketBuffer*>(pDest);
			*pPacket >> Status;
			*pPacket >> AccountNo;
		}

		void ResLogin::WriteTo(void* pDest) const
		{
			cov1013::PacketBuffer* pPacket = static_cast<cov1013::PacketBuffer*>(pDest);
			*pPacket << ePacketID::ResLogin;
			*pPacket << Status;
			*pPacket << AccountNo;
		}

		void ReqMoveSector::ReadFrom(void* pDest)
		{
			cov1013::PacketBuffer* pPacket = static_cast<cov1013::PacketBuffer*>(pDest);
			*pPacket >> AccountNo;
			*pPacket >> SectorX;
			*pPacket >> SectorY;
		}

		void ReqMoveSector::WriteTo(void* pDest) const
		{
			cov1013::PacketBuffer* pPacket = static_cast<cov1013::PacketBuffer*>(pDest);
			*pPacket << ePacketID::ReqMoveSector;
			*pPacket << AccountNo;
			*pPacket << SectorX;
			*pPacket << SectorY;
		}

		void ResMoveSector::ReadFrom(void* pDest)
		{
			cov1013::PacketBuffer* pPacket = static_cast<cov1013::PacketBuffer*>(pDest);
			*pPacket >> AccountNo;
			*pPacket >> SectorX;
			*pPacket >> SectorY;
		}

		void ResMoveSector::WriteTo(void* pDest) const
		{
			cov1013::PacketBuffer* pPacket = static_cast<cov1013::PacketBuffer*>(pDest);
			*pPacket << ePacketID::ResMoveSector;
			*pPacket << AccountNo;
			*pPacket << SectorX;
			*pPacket << SectorY;
		}

		void ReqMessage::ReadFrom(void* pDest)
		{
			cov1013::PacketBuffer* pPacket = static_cast<cov1013::PacketBuffer*>(pDest);
			*pPacket >> AccountNo;
			*pPacket >> MessageLen;
			pPacket->DoRead(Message, MessageLen);
		}

		void ReqMessage::WriteTo(void* pDest) const
		{
			cov1013::PacketBuffer* pPacket = static_cast<cov1013::PacketBuffer*>(pDest);
			*pPacket << ePacketID::ReqMessage;
			*pPacket << AccountNo;
			*pPacket << MessageLen;
			pPacket->DoWrite(Message, MessageLen);
		}

		void ResMessage::ReadFrom(void* pDest)
		{
			cov1013::PacketBuffer* pPacket = static_cast<cov1013::PacketBuffer*>(pDest);
			*pPacket >> AccountNo;
			pPacket->DoRead(ID, sizeof(ID));
			pPacket->DoRead(Nickname, sizeof(Nickname));
			*pPacket >> MessageLen;
			pPacket->DoRead(Message, MessageLen);
		}

		void ResMessage::WriteTo(void* pDest) const
		{
			cov1013::PacketBuffer* pPacket = static_cast<cov1013::PacketBuffer*>(pDest);
			*pPacket << ePacketID::ResMessage;
			*pPacket << AccountNo;
			pPacket->DoWrite(ID, sizeof(ID));
			pPacket->DoWrite(Nickname, sizeof(Nickname));
			*pPacket << MessageLen;
			pPacket->DoWrite(Message, MessageLen);
		}
	}
}