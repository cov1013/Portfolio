#include "DBConnection.h"

namespace cov1013
{
	SQLHENV DBConnection::_hEnvironment = SQL_NULL_HANDLE;

	bool DBConnection::Initialize()
	{
		if (::SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &_hEnvironment) != SQL_SUCCESS)
		{
			wprintf_s(L"SQLAllocHandle(SQL_HANDLE_ENV) failed\n");
			return false;
		}

		if (::SQLSetEnvAttr(_hEnvironment, SQL_ATTR_ODBC_VERSION, reinterpret_cast<SQLPOINTER>(SQL_OV_ODBC3), 0) != SQL_SUCCESS)
		{
			wprintf_s(L"SQLSetEnvAttr(SQL_ATTR_ODBC_VERSION) failed\n");
			return false;
		}

		return true;
	}

	void DBConnection::Release()
	{
		if (_hEnvironment != SQL_NULL_HANDLE)
		{
			::SQLFreeHandle(SQL_HANDLE_ENV, _hEnvironment);
			_hEnvironment = SQL_NULL_HANDLE;
		}
	}

	DBConnection::~DBConnection()
	{
		Clear();
	}

	bool DBConnection::Connect(const wchar_t* pDSN, const wchar_t* pUserID, const wchar_t* pPasswd)
	{
		if(_hEnvironment == SQL_NULL_HANDLE)
		{
			wprintf_s(L"DBConnection::Initialize() must be called before Connect()\n");
			return false;
		}

		// 연결 핸들 생성
		if (::SQLAllocHandle(SQL_HANDLE_DBC, _hEnvironment, &_hConnection) != SQL_SUCCESS)
		{
			wprintf_s(L"SQLAllocHandle(SQL_HANDLE_DBC) failed\n");
			return false;
		}

		// 연결
		const SQLRETURN ret = ::SQLConnectW(_hConnection, 
			(SQLWCHAR*)pDSN, 
			(SQLSMALLINT)wcslen(pDSN), 
			(SQLWCHAR*)pUserID, 
			(SQLSMALLINT)wcslen(pUserID), 
			(SQLWCHAR*)pPasswd, 
			(SQLSMALLINT)wcslen(pPasswd));

		if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
		{
			wprintf_s(L"SQLConnectW failed\n");
			return false;
		}

		// 명령문 핸들 생성
		if (::SQLAllocHandle(SQL_HANDLE_STMT, _hConnection, &_hStatement) != SQL_SUCCESS)
		{
			wprintf_s(L"SQLAllocHandle(SQL_HANDLE_STMT) failed\n");
			return false;
		}

		return true;
	}

	void DBConnection::Clear()
	{
		if (_hStatement != SQL_NULL_HANDLE)
		{
			::SQLFreeHandle(SQL_HANDLE_STMT, _hStatement);
			_hStatement = SQL_NULL_HANDLE;
		}

		if (_hConnection != SQL_NULL_HANDLE)
		{
			::SQLDisconnect(_hConnection);
			::SQLFreeHandle(SQL_HANDLE_DBC, _hConnection);
			_hConnection = SQL_NULL_HANDLE;
		}
	}

	bool DBConnection::Execute(const wchar_t* pQuery)
	{
		SQLRETURN ret = ::SQLExecDirectW(_hStatement, (SQLWCHAR*)pQuery, SQL_NTSL);
		if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO)
		{
			return true;
		}

		HandleError(ret, pQuery);
		return false;
	}

	bool DBConnection::Fetch()
	{
		const SQLRETURN ret = ::SQLFetch(_hStatement);

		switch (ret)
		{
		case SQL_SUCCESS:
		case SQL_SUCCESS_WITH_INFO:
			return true;
		case SQL_NO_DATA:
			return false;
		case SQL_ERROR:
			HandleError(ret);
			return false;
		default:
			return false;
		}
	}

	void DBConnection::Unbind() const
	{
		::SQLFreeStmt(_hStatement, SQL_UNBIND);
		::SQLFreeStmt(_hStatement, SQL_RESET_PARAMS);
		::SQLFreeStmt(_hStatement, SQL_CLOSE);
	}

	bool DBConnection::BindParam(int paramIndex, bool* value, SQLLEN* index)
	{
		return BindParam(static_cast<SQLUSMALLINT>(paramIndex), SQL_C_TINYINT, SQL_TINYINT, sizeof(bool), value, index);
	}

	bool DBConnection::BindParam(int paramIndex, float* value, SQLLEN* index)
	{
		return BindParam(static_cast<SQLUSMALLINT>(paramIndex), SQL_C_FLOAT, SQL_REAL, 0, value, index);
	}

	bool DBConnection::BindParam(int paramIndex, double* value, SQLLEN* index)
	{
		return BindParam(static_cast<SQLUSMALLINT>(paramIndex), SQL_C_DOUBLE, SQL_DOUBLE, 0, value, index);
	}

	bool DBConnection::BindParam(int paramIndex, char* value, SQLLEN* index)
	{
		return BindParam(static_cast<SQLUSMALLINT>(paramIndex), SQL_C_TINYINT, SQL_TINYINT, sizeof(char), value, index);
	}

	bool DBConnection::BindParam(int paramIndex, short* value, SQLLEN* index)
	{
		return BindParam(static_cast<SQLUSMALLINT>(paramIndex), SQL_C_SHORT, SQL_SMALLINT, sizeof(short), value, index);
	}

	bool DBConnection::BindParam(int paramIndex, int* value, SQLLEN* index)
	{
		return BindParam(static_cast<SQLUSMALLINT>(paramIndex), SQL_C_LONG, SQL_INTEGER, sizeof(int), value, index);
	}

	bool DBConnection::BindParam(int paramIndex, long long* value, SQLLEN* index)
	{
		return BindParam(static_cast<SQLUSMALLINT>(paramIndex), SQL_C_SBIGINT, SQL_BIGINT, sizeof(long long), value, index);
	}

	bool DBConnection::BindParam(int paramIndex, TIMESTAMP_STRUCT* value, SQLLEN* index)
	{
		return BindParam(static_cast<SQLUSMALLINT>(paramIndex), SQL_C_TYPE_TIMESTAMP, SQL_TYPE_TIMESTAMP, sizeof(TIMESTAMP_STRUCT), value, index);
	}

	bool DBConnection::BindParam(int paramIndex, const wchar_t* str, SQLLEN* index)
	{
		SQLULEN size = static_cast<SQLULEN>((::wcslen(str) + 1) * 2);
		*index = SQL_NTSL;

		if (size > WVARCHAR_MAX)
		{
			return BindParam(static_cast<SQLUSMALLINT>(paramIndex), SQL_C_WCHAR, SQL_WLONGVARCHAR, size, (SQLPOINTER)str, index);
		}
		else
		{
			return BindParam(static_cast<SQLUSMALLINT>(paramIndex), SQL_C_WCHAR, SQL_WVARCHAR, size, (SQLPOINTER)str, index);
		}
	}

	bool DBConnection::BindParam(int paramIndex, const unsigned char* bin, int size, SQLLEN* index)
	{
		if (bin == nullptr)
		{
			*index = SQL_NULL_DATA;
			size = 1;
		}
		else
		{
			*index = size;
		}

		if (size > BINARY_MAX)
		{
			return BindParam(static_cast<SQLUSMALLINT>(paramIndex), SQL_C_BINARY, SQL_LONGVARBINARY, size, (BYTE*)bin, index);
		}
		else
		{
			return BindParam(static_cast<SQLUSMALLINT>(paramIndex), SQL_C_BINARY, SQL_BINARY, size, (BYTE*)bin, index);
		}
	}

	bool DBConnection::BindCol(int columnIndex, bool* value, SQLLEN* index)
	{
		return BindCol(static_cast<SQLUSMALLINT>(columnIndex), SQL_C_TINYINT, sizeof(bool), value, index);
	}

	bool DBConnection::BindCol(int columnIndex, float* value, SQLLEN* index)
	{
		return BindCol(static_cast<SQLUSMALLINT>(columnIndex), SQL_C_FLOAT, sizeof(float), value, index);
	}

	bool DBConnection::BindCol(int columnIndex, double* value, SQLLEN* index)
	{
		return BindCol(static_cast<SQLUSMALLINT>(columnIndex), SQL_C_DOUBLE, sizeof(double), value, index);
	}

	bool DBConnection::BindCol(int columnIndex, char* value, SQLLEN* index)
	{
		return BindCol(static_cast<SQLUSMALLINT>(columnIndex), SQL_C_TINYINT, sizeof(char), value, index);
	}

	bool DBConnection::BindCol(int columnIndex, short* value, SQLLEN* index)
	{
		return BindCol(static_cast<SQLUSMALLINT>(columnIndex), SQL_C_SHORT, sizeof(short), value, index);
	}

	bool DBConnection::BindCol(int columnIndex, int* value, SQLLEN* index)
	{
		return BindCol(static_cast<SQLUSMALLINT>(columnIndex), SQL_C_LONG, sizeof(int), value, index);
	}

	bool DBConnection::BindCol(int columnIndex, long long* value, SQLLEN* index)
	{
		return BindCol(static_cast<SQLUSMALLINT>(columnIndex), SQL_C_SBIGINT, sizeof(long long), value, index);
	}

	bool DBConnection::BindCol(int columnIndex, TIMESTAMP_STRUCT* value, SQLLEN* index)
	{
		return BindCol(static_cast<SQLUSMALLINT>(columnIndex), SQL_C_TYPE_TIMESTAMP, sizeof(TIMESTAMP_STRUCT), value, index);
	}

	bool DBConnection::BindCol(int columnIndex, char* bin, int size, SQLLEN* index)
	{
		return BindCol(static_cast<SQLUSMALLINT>(columnIndex), SQL_C_CHAR, size, bin, index);
	}

	bool DBConnection::BindCol(int columnIndex, wchar_t* str, int size, SQLLEN* index)
	{
		return BindCol(static_cast<SQLUSMALLINT>(columnIndex), SQL_C_WCHAR, size, str, index);
	}

	bool DBConnection::BindCol(int columnIndex, unsigned char* bin, int size, SQLLEN* index)
	{
		return BindCol(static_cast<SQLUSMALLINT>(columnIndex), SQL_C_BINARY, size, bin, index);
	}

	bool DBConnection::BindParam(SQLUSMALLINT paramIndex, SQLSMALLINT cType, SQLSMALLINT sqlType, SQLULEN len, SQLPOINTER ptr, SQLLEN* index)
	{
		SQLRETURN ret = ::SQLBindParameter(
			_hStatement,		// 쿼리
			paramIndex,			// 파라미터 인덱스
			SQL_PARAM_INPUT,	// 파라미터 방향 (입력)
			cType,				// 파라미터 타입
			sqlType,			// SQL 타입
			len,				// 길이
			0,					// 소수점 자리수
			ptr,				// 연결할 값
			0,					// 버퍼 길이
			index				// 값 길이
		);

		if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
		{
			HandleError(ret);
			return false;
		}

		return true;
	}

	bool DBConnection::BindCol(SQLUSMALLINT ColumnNumber, SQLSMALLINT TargetType, SQLULEN BufferLength, SQLPOINTER TargetValue, SQLLEN* StrLen_or_Ind)
	{
		const SQLRETURN ret = ::SQLBindCol(
			_hStatement, 
			ColumnNumber,
			TargetType,
			TargetValue,
			BufferLength,
			StrLen_or_Ind
		);

		if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
		{
			HandleError(ret);
			return false;
		}

		return true;
	}

	void DBConnection::HandleError(SQLRETURN ret, const wchar_t* pQuery) const
	{
		if (ret == SQL_SUCCESS)
		{
			return;
		}

		SQLSMALLINT index = 1;
		SQLWCHAR	sqlState[MAX_PATH] = { 0 };
		SQLINTEGER	nativeErr = 0;
		SQLWCHAR	errMsg[MAX_PATH] = { 0 };
		SQLSMALLINT msgLen = 0;
		SQLRETURN	errorRet = 0;

		while (true)
		{
			errorRet = ::SQLGetDiagRecW(SQL_HANDLE_STMT, _hStatement, index, sqlState, OUT &nativeErr, errMsg, _countof(errMsg), OUT &msgLen);

			if (errorRet == SQL_NO_DATA)
			{
				break;
			}

			if (errorRet != SQL_SUCCESS && errorRet != SQL_SUCCESS_WITH_INFO)
			{
				break;
			}

			if (pQuery == nullptr)
			{
				wprintf_s(L"Database error occurred. (sqlState:%s, nativeErr:%d, errMsg:%s)\n", sqlState, nativeErr, errMsg);
			}
			else
			{
				wprintf_s(L"Database error occurred. (query:%s, sqlState:%s, nativeErr:%d, errMsg:%s)\n", pQuery, sqlState, nativeErr, errMsg);
			}

			index++;
		}
	}
}