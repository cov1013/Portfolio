/////////////////////////////////////////////////////////////////
//
// ODBC.cpp
//
/////////////////////////////////////////////////////////////////

#include "pch.h"

#include "ODBC.h"
#include "FileLog.h"

#define WIDEN2(x) L ## x
#define WIDEN(x) WIDEN2(x)
#define __WFUNCSIG__ WIDEN(__FUNCSIG__)

bool DBF_ISOK(SQLRETURN x)
{
	return ( (x == SQL_SUCCESS || x == SQL_SUCCESS_WITH_INFO) ? true: false );
}

ODBC::CConnection::CConnection()
{
	m_hConnection = NULL;
	m_hEnvironment = NULL;
	m_hStatement = NULL;

	memset(&this->LoginInfo, 0x00, sizeof(this->LoginInfo));
	m_PrevQueryTime = 0;

	ClearBindParameter();
}


ODBC::CConnection::~CConnection()
{
	Close();
}

//bool ODBC::CConnection::Initialize(const wchar_t* pServer, const wchar_t* pPort, const wchar_t* pDatabase, const wchar_t* pUserID, const wchar_t* pPasswd)
//{
//	Close();
//	SQLRETURN Ret;
//
//	Ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &m_hEnvironment);
//	if(Ret != SQL_SUCCESS)
//		goto Error;
//
//	Ret = SQLSetEnvAttr(m_hEnvironment, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0);
//	if(Ret != SQL_SUCCESS)
//		goto Error;
//
//	Ret = SQLAllocHandle(SQL_HANDLE_DBC, m_hEnvironment, &m_hConnection);
//	if(Ret != SQL_SUCCESS)
//		goto Error;
//
//	wchar_t WBuf[MAX_PATH]={0};
//	_snwprintf(WBuf, ARRAY_CNT(WBuf), L"DRIVER={SQL Server};Server=%s;Port=%s;Database=%s;UID=%s;PWD=%s;", pServer, pPort, pDatabase, pUserID, pPasswd);
//
//	SQLSMALLINT OutLng = 0;
//	SQLWCHAR Out[1024];
//	Ret = SQLDriverConnectW(m_hConnection, NULL, (SQLWCHAR*)WBuf, (SQLSMALLINT)wcslen(WBuf), Out, static_cast<SQLSMALLINT>(1024), &OutLng, SQL_DRIVER_NOPROMPT);
//	if(Ret != SQL_SUCCESS && Ret != SQL_SUCCESS_WITH_INFO)
//		goto Error;
//
//	return true;
//
//Error:
//	SQLWCHAR SqlState[16];
//	SQLWCHAR MessageTextWBuf[256];
//	SQLSMALLINT MessageTextWLen;
//	SQLINTEGER NativeError;
//
//	SQLGetDiagRecW(SQL_HANDLE_DBC, m_hConnection, 1, SqlState, &NativeError, MessageTextWBuf, sizeof(MessageTextWBuf) / sizeof(MessageTextWBuf[0]), &MessageTextWLen);
//	Close();
//
//	FILE_LOG->PrintlnW(L"ODBC::Initialize() - FAILURE: %s", MessageTextWBuf);
//	return false;
//}

bool ODBC::CConnection::InitializeNew(const wchar_t* pDSN, const wchar_t* pUserID, const wchar_t* pPasswd)
{
	if(wcslen(this->LoginInfo.DSNBuf) == 0 ||
		wcslen(this->LoginInfo.UserIDBuf) == 0 ||
		wcslen(this->LoginInfo.PasswdBuf) == 0)
	{
		_snwprintf(this->LoginInfo.DSNBuf, sizeof(this->LoginInfo.DSNBuf), L"%s", pDSN);
		_snwprintf(this->LoginInfo.UserIDBuf, sizeof(this->LoginInfo.UserIDBuf), L"%s", pUserID);
		_snwprintf(this->LoginInfo.PasswdBuf, sizeof(this->LoginInfo.PasswdBuf), L"%s", pPasswd);
	}

	Close();
	SQLRETURN Ret;

	Ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &m_hEnvironment);
	if(Ret != SQL_SUCCESS)
		goto Error;

	Ret = SQLSetEnvAttr(m_hEnvironment, SQL_ATTR_ODBC_VERSION,(void *)SQL_OV_ODBC3, 0);
	if(Ret != SQL_SUCCESS)
		goto Error;

	Ret = SQLAllocHandle(SQL_HANDLE_DBC, m_hEnvironment, &m_hConnection);
	if(Ret != SQL_SUCCESS)
		goto Error;

	Ret = SQLConnectW(m_hConnection, (SQLWCHAR*)pDSN,(SQLSMALLINT)wcslen(pDSN), (SQLWCHAR*)pUserID,(SQLSMALLINT)wcslen(pUserID), (SQLWCHAR*)pPasswd,(SQLSMALLINT)wcslen(pPasswd));
	if(Ret != SQL_SUCCESS && Ret != SQL_SUCCESS_WITH_INFO)
		goto Error;

	Ret = SQLAllocHandle(SQL_HANDLE_STMT, m_hConnection, &m_hStatement);
	if(Ret != SQL_SUCCESS)
		goto Error;

	return true;

Error:
	SQLWCHAR SqlState[16];
	SQLWCHAR MessageTextWBuf[256];
	SQLSMALLINT MessageTextWLen;
	SQLINTEGER NativeError;

	SQLGetDiagRecW(SQL_HANDLE_DBC, m_hConnection, 1, SqlState, &NativeError, MessageTextWBuf, sizeof(MessageTextWBuf) / sizeof(MessageTextWBuf[0]), &MessageTextWLen);
	Close();
	
	FILE_LOG->PrintlnW(L"ODBC::InitializeNew() - FAILURE: %s", MessageTextWBuf);
	return false;
}

void ODBC::CConnection::Close()
{
	if(m_hStatement)
	{
		SQLFreeHandle(SQL_HANDLE_STMT, m_hStatement);
		m_hStatement = NULL;
	}
	if(m_hConnection)
	{
		SQLDisconnect(m_hConnection);
		SQLFreeHandle(SQL_HANDLE_DBC, m_hConnection);
		m_hConnection = NULL;
	}
	if(m_hEnvironment)
	{
		SQLFreeHandle(SQL_HANDLE_ENV, m_hEnvironment);
		m_hEnvironment = NULL;
	}
	m_bIsEOF = true;
	m_bIsBOF = true;
}

void ODBC::CConnection::LogODBCError(const wchar_t* pTag, SQLRETURN Return, const wchar_t * QueryBuf, uint32 QueryLen)
{
	SQLWCHAR MessageTextWBuf[256];
	memset(MessageTextWBuf, 0, sizeof(MessageTextWBuf));
	memset(m_StateWBuf, 0, sizeof(m_StateWBuf));
	wchar_t Time[1024];

	SQLGetDiagRecW(SQL_HANDLE_STMT , m_hStatement, 1, m_StateWBuf, 0, MessageTextWBuf, 255, 0);

	FILE_LOG->PrintlnW(L"%s - Query: %s", pTag, QueryBuf);
	FILE_LOG->PrintlnW(L"%s - Return: %s(%d)", pTag, ResolveSQLReturn(Return), Return);
	FILE_LOG->PrintlnW(L"%s - State: %s", pTag, m_StateWBuf);
	FILE_LOG->PrintlnW(L"%s - Error: %s", pTag, MessageTextWBuf);

	struct tm* pTm = localtime(&m_PrevQueryTime);
	wcsftime(Time, ARRAY_CNT(Time) - 1, L"%Y-%m-%d %H:%M:%S", pTm);

	FILE_LOG->PrintlnW(L"%s - PrevQueryTime: %s", pTag, Time);
	FILE_LOG->PrintlnW(L"%s - ReconnectServer", pTag);
	{
		this->Close();
		bool result = this->InitializeNew(this->LoginInfo.DSNBuf, this->LoginInfo.UserIDBuf, this->LoginInfo.PasswdBuf);
		FILE_LOG->PrintlnW(L"%s - InitializeNew DSN=%s, UserID=%s, Password=%s, result=%s", pTag, LoginInfo.DSNBuf, LoginInfo.UserIDBuf, LoginInfo.PasswdBuf, result ? L"true":L"false");
	}
}

void ODBC::CConnection::ReallocStatement()
{
	SQLFreeHandle(SQL_HANDLE_STMT, m_hStatement);
	m_hStatement = NULL;
	SQLAllocHandle(SQL_HANDLE_STMT, m_hConnection, &m_hStatement);
	//SQLSetStmtAttr(m_hStatement, SQL_ATTR_CURSOR_SCROLLABLE,(SQLPOINTER)SQL_SCROLLABLE, SQL_IS_INTEGER);
}

bool ODBC::CConnection::ExecuteQueryW(const wchar_t* QueryBuf, uint32 QueryLen)
{
	if(m_hStatement)
	{
		this->ReallocStatement();
	}

	uint8* ColumnValuePtr = nullptr;
	SQLRETURN Result;
	if(m_BindStringParamVector.size() > 0 || m_BindBinaryParamVector.size() > 0)
	{
		Result = SQLPrepareW(m_hStatement, (SQLWCHAR*)QueryBuf, SQL_NTS);
		if(Result != SQL_SUCCESS && Result != SQL_SUCCESS_WITH_INFO)
		{
			LogODBCError(__WFUNCSIG__, Result, QueryBuf, QueryLen);
			goto Error;
		}

		for(uint32 idx = 0; idx < m_BindStringParamVector.size(); idx++)
		{
			BindStringParamType* pBindParam = &(m_BindStringParamVector[idx]);

			SQLLEN Size = pBindParam->Value.length();
			Result = SQLBindParameter(m_hStatement, pBindParam->IdxParam, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR,
				(Size ? Size : 1), 0, const_cast<wchar_t*>(pBindParam->Value.c_str()), sizeof(wchar_t) * pBindParam->Value.length(), NULL);
			if(Result != SQL_SUCCESS && Result != SQL_SUCCESS_WITH_INFO)
			{
				LogODBCError(__WFUNCSIG__, Result, QueryBuf, QueryLen);
				goto Error;
			}
		}

		for(uint32 idx = 0; idx < m_BindBinaryParamVector.size(); idx++)
		{
			BindBinaryParamType* pBindParam = &(m_BindBinaryParamVector[idx]);

			Result = SQLBindParameter(m_hStatement, pBindParam->IdxParam, SQL_PARAM_INPUT, SQL_C_BINARY, SQL_VARBINARY,
				(pBindParam->Size ? pBindParam->Size : 1), 0, pBindParam->Value, pBindParam->Size, &pBindParam->Size);
			if(Result != SQL_SUCCESS && Result != SQL_SUCCESS_WITH_INFO)
			{
				LogODBCError(__WFUNCSIG__, Result, QueryBuf, QueryLen);
				goto Error;
			}
		}

		Result = SQLExecute(m_hStatement);
		if(Result != SQL_SUCCESS && Result != SQL_SUCCESS_WITH_INFO)
		{
			LogODBCError(__WFUNCSIG__, Result, QueryBuf, QueryLen);
			goto Error;
		}
		ClearBindParameter();
	}
	else
	{
		Result = SQLExecDirectW(m_hStatement, (SQLWCHAR*)QueryBuf, (SQLINTEGER)QueryLen);
		if(Result != SQL_SUCCESS && Result != SQL_SUCCESS_WITH_INFO)
		{
			//SQL_ERROR:
			//SQL_INVALID_HANDLE:
			//SQL_NEED_DATA:
			//SQL_STILL_EXECUTING:
			//SQL_NO_DATA:

			LogODBCError(__WFUNCSIG__, Result, QueryBuf, QueryLen);
			goto Error;
		}
	}

	time(&m_PrevQueryTime);

	if(SQLNumResultCols(m_hStatement, &m_ColumnCount) != SQL_SUCCESS)
		goto Error;

	memset(m_ColumnValuePtr, 0x00, sizeof(m_ColumnValuePtr));
	memset(m_ColumnValueLen, 0x00, sizeof(m_ColumnValueLen));
	memset(m_ColumnDesc, 0x00, sizeof(m_ColumnDesc));
	
	ColumnDescType* pColumnDesc;
	ColumnValuePtr = m_ColumnValueBuf;
	SQLSMALLINT ColumnValueType;
	uint32 ColumnValueSize;
	for(SQLSMALLINT idxCol = 0; idxCol < m_ColumnCount; idxCol++)
	{
		pColumnDesc = &m_ColumnDesc[idxCol];

		if(SQLDescribeColW(m_hStatement, idxCol + 1, 
			pColumnDesc->NameBuf, sizeof(pColumnDesc->NameBuf), &pColumnDesc->NameLen, 
			&pColumnDesc->Type, (SQLULEN*)&pColumnDesc->Size, &pColumnDesc->Digits, &pColumnDesc->Nullable) != SQL_SUCCESS)
		{
			goto Error;
		}
		m_ColumnValuePtr[idxCol] = ColumnValuePtr;

		switch(pColumnDesc->Type)
		{
		case SQL_WCHAR:
		case SQL_WVARCHAR:
		case SQL_WLONGVARCHAR:
			ColumnValueType = SQL_C_WCHAR;
			ColumnValueSize = pColumnDesc->Size* 2 + 2;
			break;

		case SQL_TYPE_TIMESTAMP:
			ColumnValueType = SQL_C_TIMESTAMP;
			ColumnValueSize = sizeof(TIMESTAMP_STRUCT);
			break;

		case SQL_BIGINT:
			ColumnValueType = SQL_C_DEFAULT;
			ColumnValueSize = 8;
			break;

		case SQL_VARBINARY:
			ColumnValueType = SQL_C_DEFAULT;
			ColumnValueSize = pColumnDesc->Size;
			break;

		case SQL_LONGVARBINARY:
			ColumnValueType = SQL_C_DEFAULT;
			ColumnValueSize = pColumnDesc->Size;
			if(ColumnValueSize > 1024)
			{
				ColumnValueSize = 1024;
			}
			break;

		default:
			ColumnValueType = pColumnDesc->Type;
			ColumnValueSize = pColumnDesc->Size;
		}

		if((Result = SQLBindCol(m_hStatement, idxCol + 1,
			ColumnValueType,
			ColumnValuePtr, 
			ColumnValueSize, 
			(SQLLEN*)&m_ColumnValueLen[idxCol])) != SQL_SUCCESS)
		{
			SQLCHAR szImsi[256];
			SQLCHAR szState[6];
			memset(szImsi, 0, 256);
			memset(szState, 0, 6);

			SQLGetDiagRec(SQL_HANDLE_STMT , m_hStatement, 1, szState, 0, szImsi, 255, 0);
			FILE_LOG->Println("FAILURE: State=%s,Message=%s", szState, szImsi);
			goto Error;
		}
		ColumnValuePtr += ColumnValueSize;
	}

	memset(m_ColumnValueBuf, 0x00, sizeof(m_ColumnValueBuf));
	Result = SQLFetch(m_hStatement);
	if(Result == SQL_NO_DATA)
	{
		m_bIsBOF = true;
		m_bIsEOF = true;
	}
	else
	{
		m_bIsBOF = false;
		m_bIsEOF = false;
	}
	return true;
Error:
	return false;
}

bool ODBC::CConnection::ExecuteUpdateW(const wchar_t* QueryBuf, uint32 QueryLen)
{
	if(m_hStatement)
	{
		this->ReallocStatement();
	}

	SQLRETURN Result;
	Result = SQLExecDirectW(m_hStatement,(SQLWCHAR *)QueryBuf,(SQLINTEGER)QueryLen);
	if(Result != SQL_SUCCESS && Result != SQL_NO_DATA)
	{
		//SQL_NEED_DATA
		//SQL_STILL_EXECUTING
		//SQL_ERROR
		//SQL_INVALID_HANDLE
		//SQL_SUCCESS_WITH_INFO

		LogODBCError(__WFUNCSIG__, Result, QueryBuf, QueryLen);
		goto Error;
	}
	time(&m_PrevQueryTime);
	return true;

Error:
	return false;

}

void ODBC::CConnection::ClearBindParameter()
{
	ClearBindStringParameter();
	ClearBindBinaryParameter();
}

void ODBC::CConnection::ClearBindStringParameter()
{
	m_BindStringParamVector.clear();
}

bool ODBC::CConnection::AddBindStringParameter(const unsigned char IdxParam, const wchar_t* pValue)
{
	BindStringParamType BindStringParam;
	BindStringParam.IdxParam = IdxParam;
	BindStringParam.Value = pValue;

	m_BindStringParamVector.push_back(BindStringParam);
	return true;
}

void ODBC::CConnection::ClearBindBinaryParameter()
{
	m_BindBinaryParamVector.clear();
}

bool ODBC::CConnection::AddBindBinaryParameter(const unsigned char IdxParam, const unsigned int Size, const unsigned char* pValue)
{
	RETURN_FALSE_ON_FAIL(Size <= MAX_BIND_BINARY_SIZE);

	BindBinaryParamType BindBinaryParam;
	BindBinaryParam.IdxParam = IdxParam;
	BindBinaryParam.Size = Size;
	ZeroMemory(BindBinaryParam.Value, sizeof(BindBinaryParam.Value));
	CopyMemory(BindBinaryParam.Value, pValue, Size);

	m_BindBinaryParamVector.push_back(BindBinaryParam);
	return true;
}

bool ODBC::CConnection::GetString(wchar_t* ValueBufPtr, uint32 ValueBufSize, uint32* pValueLen, uint8 ColumnIndex)
{
	uint32 ValueLen;
	if(m_ColumnValueLen[ColumnIndex - 1] > ValueBufSize)
	{
		ValueLen = ValueBufSize;
	}
	else
	{
		ValueLen = m_ColumnValueLen[ColumnIndex - 1];
	}

	memcpy(ValueBufPtr, m_ColumnValuePtr[ColumnIndex - 1], ValueLen);
	*pValueLen = ValueLen;
	return true;
}

bool ODBC::CConnection::GetInt64(signed __int64* pValue, unsigned char ColumnIndex)
{
	__int64 Value;
	if(m_ColumnValueLen[ColumnIndex - 1] != sizeof(__int64))
		return false;

	*pValue = Value = *(__int64*)m_ColumnValuePtr[ColumnIndex - 1];
	return true;
}

bool ODBC::CConnection::GetInt32(int32* pValue, uint8 ColumnIndex)
{
	int32 Value;
	if(m_ColumnValueLen[ColumnIndex - 1] != sizeof(int32))
		return false;

	*pValue = Value = *(int32*)m_ColumnValuePtr[ColumnIndex - 1];
	return true;
}

bool ODBC::CConnection::GetUint32(uint32* pValue, uint8 ColumnIndex)
{
	uint32 Value;
	if(m_ColumnValueLen[ColumnIndex - 1] != sizeof(uint32))
		return false;

	*pValue = Value = *(uint32*)m_ColumnValuePtr[ColumnIndex - 1];
	return true;
}

bool ODBC::CConnection::GetUint64(uint64* pValue, uint8 ColumnIndex)
{
	uint64 Value;
	if(m_ColumnValueLen[ColumnIndex - 1] != sizeof(uint64))
		return false;

	*pValue = Value = *(uint64*)m_ColumnValuePtr[ColumnIndex - 1];
	return true;
}

bool ODBC::CConnection::GetUint16(uint16* pValue, uint8 ColumnIndex)
{
	uint16 Value;
	if(m_ColumnValueLen[ColumnIndex - 1] != sizeof(uint16))
		return false;

	*pValue = Value = *(uint16*)m_ColumnValuePtr[ColumnIndex - 1];
	return true;
}

bool ODBC::CConnection::GetUint8(uint8* pValue, uint8 ColumnIndex)
{
	uint8 Value;
	if(m_ColumnValueLen[ColumnIndex - 1] != sizeof(uint8))
		return false;

	*pValue = Value = *(uint8*)m_ColumnValuePtr[ColumnIndex - 1];
	return true;
}

bool ODBC::CConnection::GetInt8(signed char* pValue, uint8 ColumnIndex)
{
	int8 Value;
	if(m_ColumnValueLen[ColumnIndex - 1] != sizeof(int8))
		return false;

	*pValue = Value = *(int8*)m_ColumnValuePtr[ColumnIndex - 1];
	return true;
}

bool ODBC::CConnection::GetInt16(int16* pValue, uint8 ColumnIndex)
{
	int16 Value;
	if(m_ColumnValueLen[ColumnIndex - 1] != sizeof(int16))
		return false;

	*pValue = Value = *(int16*)m_ColumnValuePtr[ColumnIndex - 1];
	return true;
}

bool ODBC::CConnection::GetColumnIndex(uint8* pColumnIndex, const wchar_t* pColumnName)
{
	for(uint8 idxColumn = 0; idxColumn < m_ColumnCount; idxColumn++)
	{
		if(wcscmp(m_ColumnDesc[idxColumn].NameBuf, pColumnName) == 0)
		{
			*pColumnIndex = idxColumn + 1;
			return true;
		}
	}
	return false;
}

bool	ODBC::CConnection::GetString(wchar_t* ValueBufPtr, uint32 ValueBufSize, uint32* pValueLen, const wchar_t* pColumnName)
{
	uint8 ColumnIndex;
	if(GetColumnIndex(&ColumnIndex, pColumnName) == false)
		return false;

	return GetString(ValueBufPtr, ValueBufSize, pValueLen, ColumnIndex);
}

bool	ODBC::CConnection::GetUint32(uint32* pValue, const wchar_t* pColumnName)
{
	uint8 ColumnIndex;
	if(GetColumnIndex(&ColumnIndex, pColumnName) == false)
		return false;

	return GetUint32(pValue, ColumnIndex);
}

bool ODBC::CConnection::GetUint64(uint64* pValue, const wchar_t* pColumnName)
{
	uint8 ColumnIndex;
	if(GetColumnIndex(&ColumnIndex, pColumnName) == false)
		return false;

	return GetUint64(pValue, ColumnIndex);
}

bool	ODBC::CConnection::GetUint8(uint8* pValue, const wchar_t* pColumnName)
{
	uint8 ColumnIndex;
	if(GetColumnIndex(&ColumnIndex, pColumnName) == false)
		return false;

	return GetUint8(pValue, ColumnIndex);
}

bool	ODBC::CConnection::GetUint16(uint16* pValue, const wchar_t* pColumnName)
{
	uint8 ColumnIndex;
	if(GetColumnIndex(&ColumnIndex, pColumnName) == false)
		return false;

	return GetUint16(pValue, ColumnIndex);
}

bool ODBC::CConnection::GetInt64(signed __int64* pValue, const wchar_t* pColumnName)
{
	uint8 ColumnIndex;
	if(GetColumnIndex(&ColumnIndex, pColumnName) == false)
		return false;

	return GetInt64(pValue, ColumnIndex);
}

bool	ODBC::CConnection::GetInt32(int32* pValue, const wchar_t* pColumnName)
{
	uint8 ColumnIndex;
	if(GetColumnIndex(&ColumnIndex, pColumnName) == false)
		return false;

	return GetInt32(pValue, ColumnIndex);
}

bool	ODBC::CConnection::GetInt16(int16* pValue, const wchar_t* pColumnName)
{
	uint8 ColumnIndex;
	if(GetColumnIndex(&ColumnIndex, pColumnName) == false)
		return false;

	return GetInt16(pValue, ColumnIndex);
}

bool	ODBC::CConnection::GetInt8(signed char* pValue, const wchar_t* pColumnName)
{
	uint8 ColumnIndex;
	if(GetColumnIndex(&ColumnIndex, pColumnName) == false)
		return false;

	return GetInt8(pValue, ColumnIndex);
}

bool ODBC::CConnection::MoveNext()
{
	SQLRETURN Result;
	
	Result = SQLFetch(m_hStatement);
	if(Result == SQL_NO_DATA)
		m_bIsEOF = true;

	return true;
}

const wchar_t* ODBC::CConnection::ResolveSQLReturn(SQLRETURN ReturnValue)
{
	switch(ReturnValue)
	{
	case SQL_ERROR:					return L"SQL_ERROR";
	case SQL_NO_DATA:				return L"SQL_NO_DATA";
	case SQL_SUCCESS:				return L"SQL_SUCCESS";
	case SQL_SUCCESS_WITH_INFO:		return L"SQL_SUCCESS_WITH_INFO";
	case SQL_INVALID_HANDLE:		return L"SQL_INVALID_HANDLE";
	}
	return L"Undefined";
}

void ODBC::CConnection::GetLastState(wchar_t* StateBuf, uint32 StateBufCnt)
{
	uint32 StateCnt = (uint32)wcslen(this->m_StateWBuf);
	if(StateCnt > StateBufCnt - 1)
		StateCnt = StateBufCnt - 1;

	wcsncpy(StateBuf, this->m_StateWBuf, StateCnt);
	StateBuf[StateCnt] = NULL;
}

bool ODBC::CConnection::GetBuf(unsigned char* ValueBufPtr, unsigned int ValueBufCnt, unsigned int* pValueCnt, const wchar_t* pColumnName)
{
	uint8 ColumnIndex;
	if(GetColumnIndex(&ColumnIndex, pColumnName) == false)
		return false;

	return GetBuf(ValueBufPtr, ValueBufCnt, pValueCnt, ColumnIndex);
}

bool ODBC::CConnection::GetBuf(unsigned char* ValueBufPtr, unsigned int ValueBufCnt, unsigned int* pValueLen, unsigned char ColumnIndex)
{
	uint32 ValueLen;
	if(m_ColumnValueLen[ColumnIndex - 1] > ValueBufCnt)
	{
		ValueLen = ValueBufCnt;
	}
	else
	{
		ValueLen = m_ColumnValueLen[ColumnIndex - 1];
	}

	memcpy(ValueBufPtr, m_ColumnValuePtr[ColumnIndex - 1], ValueLen);
	*pValueLen = ValueLen;
	return true;
}

bool ODBC::CConnection::GetTimeStamp(TIMESTAMP_STRUCT * pTimeStamp, const wchar_t* pColumnName)
{
	uint8 ColumnIndex;
	if(GetColumnIndex(&ColumnIndex, pColumnName) == false)
		return false;

	return GetTimeStamp(pTimeStamp, ColumnIndex);
}

bool ODBC::CConnection::GetTimeStamp(TIMESTAMP_STRUCT * pTimeStamp, unsigned char ColumnIndex)
{
	memcpy(pTimeStamp, m_ColumnValuePtr[ColumnIndex - 1], sizeof(TIMESTAMP_STRUCT));
	return true;
}

bool ODBC::CConnection::Queryf( const wchar_t* pszMsg, ... )
{
	std::wstring Query;
	va_list args;
	va_start(args, pszMsg);
	size_t length = _vscwprintf(pszMsg, args) + 1; // _vsctprintf doesn't count terminating '\0'
	Query.reserve(length);
	Query.resize(length);
	_vsnwprintf((wchar_t*)&Query[0], length, pszMsg, args);
	va_end( args );

	std::basic_string<wchar_t> IncludeParamQuery = Query;
	{
		SQLSMALLINT IdxParam = 0;
		std::string::size_type find_offset = 0;
		std::string::size_type find_pos = IncludeParamQuery.find(L"?", find_offset);
		while(find_pos != std::string::npos)
		{
			IdxParam++;
			std::string::size_type pre_find_offset = find_offset;

			for(uint32 idx = 0; idx < m_BindStringParamVector.size(); idx++)
			{
				BindStringParamType* pBindParam = &m_BindStringParamVector[idx];

				if(pBindParam->IdxParam == IdxParam)
				{
					std::wstring PrintParam = L"N'" + pBindParam->Value + L"'";
					IncludeParamQuery.replace(IncludeParamQuery.begin() + find_pos, IncludeParamQuery.begin() + find_pos + 1, PrintParam.c_str());
					find_offset = find_pos + PrintParam.size();
					break;
				}
			}

			if(pre_find_offset == find_offset)
			{
				for(uint32 idx = 0; idx < m_BindBinaryParamVector.size(); idx++)
				{
					BindBinaryParamType* pBindParam = &m_BindBinaryParamVector[idx];

					if(pBindParam->IdxParam == IdxParam)
					{
						wchar_t Buf[4096] = { 0 };
						int32 Len = 0;
						Len += _snwprintf(Buf + Len, ARRAY_CNT(Buf), L"0x");
						for(uint32 idx2 = 0; idx2 < (uint32)pBindParam->Size; idx2++)
						{
							Len += _snwprintf(Buf + Len, ARRAY_CNT(Buf), L"%02x", pBindParam->Value[idx2]);
						}
						IncludeParamQuery.replace(IncludeParamQuery.begin() + find_pos, IncludeParamQuery.begin() + find_pos + 1, Buf);
						find_offset = find_pos + Len;
						break;
					}
				}
			}

			if(pre_find_offset == find_offset)
			{
				find_offset = find_pos + 1;
			}

			find_pos = IncludeParamQuery.find(L"?", find_offset);
		}
	}

	RETURN_FALSE_ON_FAIL(Query.length() > 0);
	RETURN_FALSE_ON_FAIL(ExecuteQueryW(Query.c_str(), (uint32)Query.length()));

 	//FILE_LOG->PrintlnW(L"Queryf[%s]", Query.c_str() );
	FILE_LOG->PrintlnW(L"Queryf[%s]", IncludeParamQuery.c_str() );
 	
	return true;
}

bool ODBC::ChkIncludeInvalidString(const char *pMessage)
{
	std::string::size_type find_pos;
	std::basic_string<char> OriginMessage(pMessage);
	char FilterBuf[] = {'\'', '<', '>', ';', '-', '\x00', '\n', '\r', '\\', '"', '\x1a', '%', '='};
	for(uint32 idx = 0; idx < ARRAY_CNT(FilterBuf); idx++)
	{
		find_pos = OriginMessage.find(FilterBuf[idx]);
		if(find_pos != std::string::npos)
		{
			return true;
		}
	}
	return false;
}

bool ODBC::ChkIncludeInvalidString(const wchar_t *pMessage)
{
	std::string::size_type find_pos;
	std::basic_string<wchar_t> OriginMessage(pMessage);
	wchar_t FilterBuf[] = {L'\'', L'<', L'>', L';', L'-', L'\x00', L'\n', L'\r', L'\\', L'"', L'\x1a', L'%', L'='};
	for(uint32 idx = 0; idx < ARRAY_CNT(FilterBuf); idx++)
	{
		find_pos = OriginMessage.find(FilterBuf[idx]);
		if(find_pos != std::string::npos)
		{
			return true;
		}
	}
	return false;
}

int ODBC::TransformInvalidString(const wchar_t *pSource, wchar_t *pResult)
{
	StringType Temp;
	std::wstring OriginMessage = pSource;
	std::wstring TransformMessage;
	for( uint32 i = 0 ; i < OriginMessage.length() ; ++i )
	{
		if( OriginMessage[ i ] == L'\'')
		{
			TransformMessage.append(L"'");
			TransformMessage.append(L"'");
		}
		else if( OriginMessage[ i ] != L'<' && OriginMessage[ i ] != L'>' && OriginMessage[ i ] != L';' && OriginMessage[ i ] != L'-' &&
			OriginMessage[ i ] != L'\x00' && OriginMessage[ i ] != L'\n' && OriginMessage[ i ] != L'\r' && OriginMessage[ i ] != L'\\' &&
			OriginMessage[ i ] != L'"' && OriginMessage[ i ] != L'\x1a' && OriginMessage[ i ] != L'%' && OriginMessage[ i ] !=  L'=')
		{
			Temp.SetfW(L"%c", OriginMessage[i]);
			TransformMessage.append(Temp.Buf);
		}
	}
	return _snwprintf(pResult, 128, L"%s", TransformMessage.c_str());
}

bool ODBC::ChkIncludeInjectionString(const char *pMessage, std::vector<char> *pExceptTextVector)
{
	std::basic_string<char> OriginMessage(pMessage);
	const char *pCheckText = OriginMessage.c_str();

	for(uint32 i = 0; i < strlen(pCheckText); i++)
	{
		if((pCheckText[i] < 0x0030 || pCheckText[i] > 0x0039) && (pCheckText[i] < 0x0041 || pCheckText[i] > 0x005A) && (pCheckText[i] < 0x0061 || pCheckText[i] > 0x007A)) // 영어
		{
			if(pExceptTextVector != NULL)
			{
				std::vector<char>::iterator iterator;
				iterator = std::find(pExceptTextVector->begin(), pExceptTextVector->end(), pCheckText[i]);
				if(iterator == pExceptTextVector->end())
				{
					return true;
				}
			}
			else
			{
				return true;
			}
		}
	}
	return false;
}

bool ODBC::ChkIncludeInjectionString(const wchar_t *pMessage, std::vector<wchar_t> *pExceptTextVector)
{
	std::basic_string<wchar_t> OriginMessage(pMessage);
	const wchar_t *pCheckText = OriginMessage.c_str();

	for(uint32 i = 0; i < wcslen(pCheckText); i++)
	{
		if((pCheckText[i] < 0x0030 || pCheckText[i] > 0x0039) && (pCheckText[i] < 0x0041 || pCheckText[i] > 0x005A) && (pCheckText[i] < 0x0061 || pCheckText[i] > 0x007A)) // 영어
		{
			if(pExceptTextVector != NULL)
			{
				std::vector<wchar_t>::iterator iterator;
				iterator = std::find(pExceptTextVector->begin(), pExceptTextVector->end(), pCheckText[i]);
				if(iterator == pExceptTextVector->end())
				{
					return true;
				}
			}
			else
			{
				return true;
			}
		}
	}
	return false;
}

class CheckInjectionString
{
public:
	CheckInjectionString(std::vector<wchar_t> *pExceptTextVector)
	{
		this->pExceptTextVector = pExceptTextVector;
	}

	bool operator()(wchar_t wchar)
	{
		if((wchar < 0x0030 || wchar > 0x0039) && (wchar < 0x0041 || wchar > 0x005A) && (wchar < 0x0061 || wchar > 0x007A)) // 영어
		{
			if(pExceptTextVector != NULL)
			{
				std::vector<wchar_t>::iterator iterator;
				iterator = std::find(pExceptTextVector->begin(), pExceptTextVector->end(), wchar);
				if(iterator == pExceptTextVector->end())
				{
					return true;
				}
			}
			else
			{
				return true;
			}
		}
		return false;
	}

private:
	std::vector<wchar_t> *pExceptTextVector;
};

int ODBC::TransformInjectionString(const wchar_t *pSource, wchar_t *pResult, std::vector<wchar_t> *pExceptTextVector)
{
	std::basic_string<wchar_t> OriginMessage(pSource);
	std::basic_string<wchar_t> TransformMessage(pSource);
	std::basic_string<wchar_t>::iterator iter_erase = std::remove_if(TransformMessage.begin(), TransformMessage.end(), CheckInjectionString(pExceptTextVector));
	if(iter_erase != TransformMessage.begin())
		TransformMessage.erase(iter_erase, TransformMessage.end());
	if(wcscmp(OriginMessage.c_str(), TransformMessage.c_str()) != 0)
		OriginMessage = TransformMessage;

	return _snwprintf(pResult, 128, L"%s", OriginMessage.c_str());
}