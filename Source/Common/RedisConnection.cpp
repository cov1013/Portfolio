#include "RedisConnection.h"

namespace cov1013
{
	RedisConnection::~RedisConnection()
	{
		Close();
	}

	void RedisConnection::Setup(const std::string& IP, const unsigned short Port, const std::string& Password, const int DBIndex)
	{
		_ip = IP;
		_port = Port;
		_password = Password;
		_dbIndex = DBIndex;
	}

	bool RedisConnection::Open()
	{
		assert(_ip.empty() == false);
		assert(_socket == INVALID_SOCKET);

		// 1) Winsock 초기화 (레퍼런스 카운팅되므로 커넥션 단위 호출 무방)
		if (_bWsaInitialized == false)
		{
			WSADATA wsaData;
			if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
			{
				LOG(Logger::eLogLevel::Error, L"[%s] WSAStartup() Failed. (Error:%d)\n", __FUNCTIONW__, WSAGetLastError());
				return false;
			}
			_bWsaInitialized = true;
		}

		// 2) 소켓 생성 및 접속
		_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (_socket == INVALID_SOCKET)
		{
			LOG(Logger::eLogLevel::Error, L"[%s] socket() Failed. (Error:%d)\n", __FUNCTIONW__, WSAGetLastError());
			return false;
		}

		SOCKADDR_IN serverAddr = {};
		serverAddr.sin_family = AF_INET;
		serverAddr.sin_port = htons(_port);
		inet_pton(AF_INET, _ip.c_str(), &serverAddr.sin_addr);

		if (connect(_socket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
		{
			LOG(Logger::eLogLevel::Error, L"[%s] connect() Failed. (IP:%S, Port:%d, Error:%d)\n", __FUNCTIONW__, _ip.c_str(), _port, WSAGetLastError());
			closesocket(_socket);
			_socket = INVALID_SOCKET;
			return false;
		}

		// 3) 소켓 옵션 (Nagle Off, 송수신 타임아웃)
		const BOOL bNoDelay = TRUE;
		setsockopt(_socket, IPPROTO_TCP, TCP_NODELAY, (const char*)&bNoDelay, sizeof(bNoDelay));

		const DWORD timeoutMs = SOCKET_TIMEOUT_MS;
		setsockopt(_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeoutMs, sizeof(timeoutMs));
		setsockopt(_socket, SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeoutMs, sizeof(timeoutMs));

		_recvSize = 0;
		_recvPos = 0;

		// 4) 인증 및 DB 선택
		if (_password.empty() == false)
		{
			RedisReply reply;
			if (ExecuteInternal(reply, { "AUTH", _password }) == false || reply.IsError())
			{
				LOG(Logger::eLogLevel::Error, L"[%s] AUTH Failed. (Message:%S)\n", __FUNCTIONW__, reply.String.c_str());
				Close();
				return false;
			}
		}

		if (_dbIndex > 0)
		{
			RedisReply reply;
			if (ExecuteInternal(reply, { "SELECT", std::to_string(_dbIndex) }) == false || reply.IsError())
			{
				LOG(Logger::eLogLevel::Error, L"[%s] SELECT Failed. (DBIndex:%d, Message:%S)\n", __FUNCTIONW__, _dbIndex, reply.String.c_str());
				Close();
				return false;
			}
		}

		return true;
	}

	void RedisConnection::Close()
	{
		if (_socket != INVALID_SOCKET)
		{
			closesocket(_socket);
			_socket = INVALID_SOCKET;
		}

		if (_bWsaInitialized)
		{
			WSACleanup();
			_bWsaInitialized = false;
		}

		_recvSize = 0;
		_recvPos = 0;
	}

	bool RedisConnection::Reconnect()
	{
		Close();
		return Open();
	}

	bool RedisConnection::Execute(RedisReply& Reply, const std::vector<std::string>& Arguments)
	{
		if (IsOpened() == false && Reconnect() == false)
		{
			return false;
		}

		if (ExecuteInternal(Reply, Arguments))
		{
			return true;
		}

		// 소켓 오류 시 1회 재접속 후 재시도
		LOG(Logger::eLogLevel::Error, L"[%s] Redis Connection lost. Try reconnect.\n", __FUNCTIONW__);
		if (Reconnect() == false)
		{
			return false;
		}

		return ExecuteInternal(Reply, Arguments);
	}

	bool RedisConnection::Ping()
	{
		RedisReply reply;
		if (Execute(reply, { "PING" }) == false)
		{
			return false;
		}
		return reply.Type == RedisReply::eType::SimpleString && reply.String == "PONG";
	}

	bool RedisConnection::Set(const std::string& Key, const std::string& Value)
	{
		RedisReply reply;
		if (Execute(reply, { "SET", Key, Value }) == false)
		{
			return false;
		}
		return reply.Type == RedisReply::eType::SimpleString && reply.String == "OK";
	}

	bool RedisConnection::SetEx(const std::string& Key, const int ExpireSeconds, const std::string& Value)
	{
		RedisReply reply;
		if (Execute(reply, { "SETEX", Key, std::to_string(ExpireSeconds), Value }) == false)
		{
			return false;
		}
		return reply.Type == RedisReply::eType::SimpleString && reply.String == "OK";
	}

	bool RedisConnection::Get(const std::string& Key, std::string* pDestination)
	{
		assert(pDestination != nullptr);

		RedisReply reply;
		if (Execute(reply, { "GET", Key }) == false)
		{
			return false;
		}

		if (reply.Type != RedisReply::eType::BulkString)
		{
			return false;
		}

		*pDestination = reply.String;

		return true;
	}

	bool RedisConnection::Del(const std::string& Key)
	{
		RedisReply reply;
		if (Execute(reply, { "DEL", Key }) == false)
		{
			return false;
		}
		return reply.Type == RedisReply::eType::Integer && reply.Integer >= 1;
	}

	bool RedisConnection::Expire(const std::string& Key, const int ExpireSeconds)
	{
		RedisReply reply;
		if (Execute(reply, { "EXPIRE", Key, std::to_string(ExpireSeconds) }) == false)
		{
			return false;
		}
		return reply.Type == RedisReply::eType::Integer && reply.Integer == 1;
	}

	bool RedisConnection::ExecuteInternal(RedisReply& Reply, const std::vector<std::string>& Arguments)
	{
		assert(Arguments.empty() == false);

		if (SendRequest(Arguments) == false)
		{
			return false;
		}

		if (ParseReply(Reply) == false)
		{
			return false;
		}

		if (Reply.IsError())
		{
			LOG(Logger::eLogLevel::Error, L"[%s] Redis Error Reply. (Command:%S, Message:%S)\n", __FUNCTIONW__, Arguments[0].c_str(), Reply.String.c_str());
		}

		return true;
	}

	bool RedisConnection::SendRequest(const std::vector<std::string>& Arguments)
	{
		// RESP Array 직렬화 (*<개수>\r\n$<길이>\r\n<인자>\r\n ...)
		std::string request;
		request.reserve(64);
		request += "*";
		request += std::to_string(Arguments.size());
		request += "\r\n";
		for (const std::string& argument : Arguments)
		{
			request += "$";
			request += std::to_string(argument.size());
			request += "\r\n";
			request += argument;
			request += "\r\n";
		}

		int sentTotal = 0;
		const int requestSize = (int)request.size();
		while (sentTotal < requestSize)
		{
			const int sent = send(_socket, request.data() + sentTotal, requestSize - sentTotal, 0);
			if (sent == SOCKET_ERROR)
			{
				return false;
			}
			sentTotal += sent;
		}

		return true;
	}

	bool RedisConnection::ParseReply(RedisReply& Reply)
	{
		std::string line;
		if (ReadLine(line) == false || line.empty())
		{
			return false;
		}

		const char typeMark = line[0];
		const char* pBody = line.c_str() + 1;

		switch (typeMark)
		{
		case '+':
			Reply.Type = RedisReply::eType::SimpleString;
			Reply.String = pBody;
			return true;

		case '-':
			Reply.Type = RedisReply::eType::Error;
			Reply.String = pBody;
			return true;

		case ':':
			Reply.Type = RedisReply::eType::Integer;
			Reply.Integer = _atoi64(pBody);
			return true;

		case '$':
		{
			const int length = atoi(pBody);
			if (length < 0)
			{
				Reply.Type = RedisReply::eType::Null;
				return true;
			}

			Reply.Type = RedisReply::eType::BulkString;
			Reply.String.resize(length);
			if (length > 0 && ReadBytes(&Reply.String[0], length) == false)
			{
				return false;
			}

			// 트레일링 CRLF 소비
			char crlf[2];
			return ReadBytes(crlf, 2);
		}

		case '*':
		{
			const int count = atoi(pBody);
			if (count < 0)
			{
				Reply.Type = RedisReply::eType::Null;
				return true;
			}

			Reply.Type = RedisReply::eType::Array;
			Reply.Elements.resize(count);
			for (int index = 0; index < count; index++)
			{
				if (ParseReply(Reply.Elements[index]) == false)
				{
					return false;
				}
			}
			return true;
		}

		default:
			LOG(Logger::eLogLevel::Error, L"[%s] Unknown RESP Type. (TypeMark:%c)\n", __FUNCTIONW__, typeMark);
			return false;
		}
	}

	bool RedisConnection::ReadLine(std::string& Line)
	{
		Line.clear();

		while (true)
		{
			if (_recvPos >= _recvSize && FillRecvBuffer() == false)
			{
				return false;
			}

			const char character = _recvBuffer[_recvPos++];
			if (character == '\r')
			{
				// '\n' 소비
				if (_recvPos >= _recvSize && FillRecvBuffer() == false)
				{
					return false;
				}
				_recvPos++;
				return true;
			}

			Line.push_back(character);
		}
	}

	bool RedisConnection::ReadBytes(char* pDestination, const int Size)
	{
		int readTotal = 0;
		while (readTotal < Size)
		{
			if (_recvPos >= _recvSize && FillRecvBuffer() == false)
			{
				return false;
			}

			const int copySize = min(Size - readTotal, _recvSize - _recvPos);
			memcpy(pDestination + readTotal, _recvBuffer + _recvPos, copySize);
			_recvPos += copySize;
			readTotal += copySize;
		}

		return true;
	}

	bool RedisConnection::FillRecvBuffer()
	{
		_recvPos = 0;
		_recvSize = recv(_socket, _recvBuffer, RECV_BUFFER_SIZE, 0);
		return _recvSize > 0;
	}
}
