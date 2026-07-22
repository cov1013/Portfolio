/////////////////////////////////////////////////////////////////
//
// ODBC.h
//
// lyric@com2us.com
// polonaiz@com2us.com
//
/////////////////////////////////////////////////////////////////

#ifndef __ODBC_h__
#define __ODBC_h__

#include <Windows.h>
#include <sqlext.h>
#include <Error.h>

class ODBC
{
public:
	class CConnection
	{
	public:
		CConnection();
		~CConnection();

		//bool Initialize(const wchar_t* pServer, const wchar_t* pPort, const wchar_t* pDatabase, const wchar_t* pUserID, const wchar_t* pPasswd);
		bool InitializeNew(const wchar_t* pDSN, const wchar_t* pUserID, const wchar_t* pPasswd);

		//Execute QueryW
		bool ExecuteQueryW(const wchar_t* QueryBuf, unsigned int QueryLen);
		bool ExecuteUpdateW(const wchar_t* QueryBuf, unsigned int QueryLen);

		//BindParameter
		void ClearBindParameter();
		void ClearBindStringParameter();
		bool AddBindStringParameter(const unsigned char IdxParam, const wchar_t* pValue);
		void ClearBindBinaryParameter();
		bool AddBindBinaryParameter(const unsigned char IdxParam, const unsigned int Size, const unsigned char* pValue);

		//Fetch Result by Index
		bool GetString(wchar_t* ValueBufPtr, unsigned int ValueBufSize, unsigned int* pValueLen, unsigned char ColumnIndex);
		bool GetUint32(unsigned int* pValue, unsigned char ColumnIndex);
		bool GetUint8(unsigned char* pValue, unsigned char ColumnIndex);
		bool GetUint16(unsigned short* pValue, unsigned char ColumnIndex);
		bool GetInt64(signed __int64* pValue, unsigned char ColumnIndex);
		bool GetInt32(signed int* pValue, unsigned char ColumnIndex);
		bool GetInt16(signed short* pValue, unsigned char ColumnIndex);
		bool GetInt8(signed char* pValue, unsigned char ColumnIndex);
		bool GetUint64(unsigned long long* pValue, unsigned char ColumnIndex);
		bool GetBuf(unsigned char* ValueBufPtr, unsigned int ValueBufCnt, unsigned int* pValueLen, unsigned char ColumnIndex);
		bool GetTimeStamp(TIMESTAMP_STRUCT * pTimeStamp, unsigned char ColumnIndex);

		//Find Index By Name
		bool GetColumnIndex(unsigned char* pColumnIndex, const wchar_t* pColumnName);

		//Fetch Result by Name
		bool GetString(wchar_t* ValueBufPtr, unsigned int ValueBufSize, unsigned int* pValueLen, const wchar_t* pColumnName);
		bool GetUint64(unsigned long long* pValue, const wchar_t* pColumnName);
		bool GetUint32(unsigned int* pValue, const wchar_t* pColumnName);
		bool GetUint8(unsigned char* pValue, const wchar_t* pColumnName);
		bool GetUint16(unsigned short* pValue, const wchar_t* pColumnName);
		bool GetInt64(signed __int64* pValue, const wchar_t* pColumnName);
		bool GetInt32(signed int* pValue, const wchar_t* pColumnName);
		bool GetInt16(signed short* pValue, const wchar_t* pColumnName);
		bool GetInt8(signed char* pValue, const wchar_t* pColumnName);
		bool GetBuf(unsigned char* ValueBufPtr, unsigned int ValueBufCnt, unsigned int* pValueLen, const wchar_t* pColumnName);
		bool GetTimeStamp(TIMESTAMP_STRUCT * pTimeStamp, const wchar_t* pColumnName);

		bool MoveNext();
		void Close();

		bool IsEof() 
		{
			return m_bIsEOF;
		}
		bool IsBof()
		{
			return m_bIsBOF;
		}

	private:
		typedef struct _LoginInfoType
		{
			wchar_t DSNBuf[256];
			wchar_t UserIDBuf[256];
			wchar_t PasswdBuf[256];
		} LoginInfoType;
		LoginInfoType LoginInfo;

		bool	 m_bIsEOF;
		bool	 m_bIsBOF;
		SQLHENV m_hEnvironment;
		SQLHANDLE m_hConnection;
		SQLHSTMT m_hStatement;

		typedef struct _BindStringParamType
		{
			SQLSMALLINT IdxParam;
			std::wstring Value;
		} BindStringParamType;

		#define MAX_BIND_BINARY_SIZE 512
		typedef struct _BindBinaryParamType
		{
			SQLSMALLINT IdxParam;
			SQLLEN Size;
			unsigned char Value[MAX_BIND_BINARY_SIZE];
		} BindBinaryParamType;

		std::vector<BindStringParamType> m_BindStringParamVector;
		std::vector<BindBinaryParamType> m_BindBinaryParamVector;

		typedef struct _ColumnDescType
		{
			wchar_t NameBuf[256];
			SQLSMALLINT NameLen;
			SQLSMALLINT Type;
			SQLUINTEGER Size;
			SQLSMALLINT Digits;
			SQLSMALLINT Nullable;
		} ColumnDescType;

		#define MAX_COLUMN_COUNT 256

		SQLSMALLINT m_ColumnCount;
		ColumnDescType m_ColumnDesc[MAX_COLUMN_COUNT];

		unsigned char* m_ColumnValuePtr[MAX_COLUMN_COUNT];
		unsigned int m_ColumnValueLen[MAX_COLUMN_COUNT];
		unsigned char m_ColumnValueBuf[16 * 1024];

		static const wchar_t* ResolveSQLReturn(SQLRETURN ReturnValue);

		void LogODBCError(const wchar_t* pTag, SQLRETURN Return, const wchar_t * QueryBuf, unsigned int QueryLen);

		void ReallocStatement();
	private:
		time_t m_PrevQueryTime;

	public:
		void GetLastState(wchar_t* StateBuf, unsigned int StateBufCnt);
	private:
		SQLWCHAR m_StateWBuf[16];
	public:
		bool Queryf( const wchar_t* pszMsg, ... );
	};	

	static bool ChkIncludeInvalidString(const char *pMessage);
	static bool ChkIncludeInvalidString(const wchar_t *pMessage);
	static int TransformInvalidString(const wchar_t *pSource, wchar_t *pResult);

	static bool ChkIncludeInjectionString(const char *pMessage, std::vector<char> *pExceptTextVector = NULL);
	static bool ChkIncludeInjectionString(const wchar_t *pMessage, std::vector<wchar_t> *pExceptTextVector = NULL);
	static int TransformInjectionString(const wchar_t *pSource, wchar_t *pResult, std::vector<wchar_t> *pExceptTextVector = NULL);
};

#endif