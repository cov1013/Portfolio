#pragma once
#include "pch.h"
#include "Logger.h"
#include <string>
#include <vector>

namespace cov1013
{
	//---------------------------------------------------------------------
	// Redis 응답 (RESP2)
	//---------------------------------------------------------------------
	struct RedisReply
	{
		enum class eType
		{
			None = 0,
			SimpleString,	// +OK
			Error,			// -ERR ...
			Integer,		// :1
			BulkString,		// $5\r\nhello
			Array,			// *N
			Null,			// $-1, *-1
		};

		eType					Type = eType::None;
		__int64					Integer = 0;
		std::string				String;
		std::vector<RedisReply>	Elements;

		inline bool IsError() const { return Type == eType::Error; }
		inline bool IsNull() const { return Type == eType::Null; }
	};

	//---------------------------------------------------------------------
	// Redis 단일 커넥션. (RESP2 프로토콜 직접 구현, blocking socket)
	// 스레드 세이프하지 않으므로 Redis 워커 스레드당 1개씩 소유하는 것을 전제로 한다.
	//---------------------------------------------------------------------
	class RedisConnection
	{
	public:
		static constexpr int RECV_BUFFER_SIZE = 8192;
		static constexpr int SOCKET_TIMEOUT_MS = 3000;

	public:
		RedisConnection() = default;
		~RedisConnection();

		void Setup(const std::string& IP, const unsigned short Port, const std::string& Password = "", const int DBIndex = 0);
		bool Open();
		void Close();
		bool Reconnect();
		inline bool IsOpened() const { return _socket != INVALID_SOCKET; }

		//---------------------------------------------------------------------
		// 범용 커맨드 실행. ex) Execute(reply, { "SETEX", "token:1", "60", "abcd" })
		// 소켓 오류 시 1회 재접속 후 재시도한다.
		// 정상적인 응답을 받으면 true. (-ERR 응답도 파싱은 성공이므로 true, Reply.IsError()로 판별)
		//---------------------------------------------------------------------
		bool Execute(RedisReply& Reply, const std::vector<std::string>& Arguments);

		// 자주 쓰는 커맨드 헬퍼
		bool Ping();
		bool Set(const std::string& Key, const std::string& Value);
		bool SetEx(const std::string& Key, const int ExpireSeconds, const std::string& Value);
		bool Get(const std::string& Key, std::string* pDestination);	// 키가 없으면 false
		bool Del(const std::string& Key);
		bool Expire(const std::string& Key, const int ExpireSeconds);

	private:
		bool ExecuteInternal(RedisReply& Reply, const std::vector<std::string>& Arguments);
		bool SendRequest(const std::vector<std::string>& Arguments);
		bool ParseReply(RedisReply& Reply);
		bool ReadLine(std::string& Line);
		bool ReadBytes(char* pDestination, const int Size);
		bool FillRecvBuffer();

	private:
		std::string		_ip;
		unsigned short	_port = 0;
		std::string		_password;
		int				_dbIndex = 0;
		SOCKET			_socket = INVALID_SOCKET;
		bool			_bWsaInitialized = false;
		char			_recvBuffer[RECV_BUFFER_SIZE] = {};
		int				_recvSize = 0;
		int				_recvPos = 0;
	};
}
