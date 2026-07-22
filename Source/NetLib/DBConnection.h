#pragma once
#include "pch.h"

namespace cov1013
{
	enum
	{
		WVARCHAR_MAX = 4000,
		BINARY_MAX = 8000
	};

	class DBConnection
	{
	public:
		static bool		Initialize();
		static void		Release();

		DBConnection() = default;
		~DBConnection();
		bool Connect(const wchar_t* pDSN, const wchar_t* pUserID, const wchar_t* pPasswd);
		bool Execute(const wchar_t* pQuery);
		bool Fetch();
		int	 GetRowCount() const;
		void Unbind() const;
		void Clear();
			 
		bool BindParam(int paramIndex, bool* value, SQLLEN* index);
		bool BindParam(int paramIndex, float* value, SQLLEN* index);
		bool BindParam(int paramIndex, double* value, SQLLEN* index);
		bool BindParam(int paramIndex, char* value, SQLLEN* index);
		bool BindParam(int paramIndex, short* value, SQLLEN* index);
		bool BindParam(int paramIndex, int* value, SQLLEN* index);
		bool BindParam(int paramIndex, long long* value, SQLLEN* index);
		bool BindParam(int paramIndex, TIMESTAMP_STRUCT* value, SQLLEN* index);
		bool BindParam(int paramIndex, const wchar_t* str, SQLLEN* index);
		bool BindParam(int paramIndex, const unsigned char* bin, int size, SQLLEN* index);
			 
		bool BindCol(int columnIndex, bool* value, SQLLEN* index);
		bool BindCol(int columnIndex, float* value, SQLLEN* index);
		bool BindCol(int columnIndex, double* value, SQLLEN* index);
		bool BindCol(int columnIndex, char* value, SQLLEN* index);
		bool BindCol(int columnIndex, short* value, SQLLEN* index);
		bool BindCol(int columnIndex, int* value, SQLLEN* index);
		bool BindCol(int columnIndex, long long* value, SQLLEN* index);
		bool BindCol(int columnIndex, TIMESTAMP_STRUCT* value, SQLLEN* index);
		bool BindCol(int columnIndex, char* str, int size, SQLLEN* index);
		bool BindCol(int columnIndex, wchar_t* str, int size, SQLLEN* index);
		bool BindCol(int columnIndex, unsigned char* bin, int size, SQLLEN* index);

		//bool GetBuf(unsigned char* pDest, unsigned int ValueBufCnt, unsigned int* pDestLen, unsigned char columnIndex);
		//bool GetString(wchar_t* pDest, unsigned int ValueBufSize, unsigned int* pDestLen, unsigned char columnIndex);
		//bool GetString(wchar_t* pDest, unsigned int ValueBufSize, unsigned int* pDestLen, const wchar_t* pColumnName);
		//bool GetUint32(unsigned int* pDest, unsigned char columnIndex);
		//bool GetUint8(unsigned char* pDest, unsigned char columnIndex);
		//bool GetUint16(unsigned short* pDest, unsigned char columnIndex);
		//bool GetInt64(signed __int64* pDest, unsigned char columnIndex);
		//bool GetInt32(signed int* pDest, unsigned char columnIndex);
		//bool GetInt16(signed short* pDest, unsigned char columnIndex);
		//bool GetInt8(signed char* pDest, unsigned char columnIndex);
		//bool GetUint64(unsigned long long* pDest, unsigned char columnIndex);
		//bool GetUint64(unsigned long long* pDest, const wchar_t* pColumnName);
		//bool GetTimeStamp(TIMESTAMP_STRUCT* pDest, unsigned char columnIndex);
		//bool GetUint32(unsigned int* pDest, const wchar_t* pColumnName);
		//bool GetUint8(unsigned char* pDest, const wchar_t* pColumnName);
		//bool GetUint16(unsigned short* pDest, const wchar_t* pColumnName);
		//bool GetInt64(signed __int64* pDest, const wchar_t* pColumnName);
		//bool GetInt32(signed int* pDest, const wchar_t* pColumnName);
		//bool GetInt16(signed short* pDest, const wchar_t* pColumnName);
		//bool GetInt8(signed char* pDest, const wchar_t* pColumnName);
		//bool GetBuf(unsigned char* pDest, unsigned int ValueBufCnt, unsigned int* pDestLen, const wchar_t* pColumnName);
		//bool GetTimeStamp(TIMESTAMP_STRUCT* pDest, const wchar_t* pColumnName);
			 
	private: 
		bool BindParam(SQLUSMALLINT paramIndex, SQLSMALLINT cType, SQLSMALLINT sqlType, SQLULEN len, SQLPOINTER ptr, SQLLEN* index);
		bool BindCol(SQLUSMALLINT ColumnNumber, SQLSMALLINT TargetType, SQLULEN BufferLength, SQLPOINTER TargetValue, SQLLEN* StrLen_or_Ind);
		void HandleError(SQLRETURN ret, const wchar_t* pQuery = nullptr) const;

	private:
		static SQLHENV	_hEnvironment;
		SQLHDBC	 _hConnection	= SQL_NULL_HANDLE;
		SQLHSTMT _hStatement	= SQL_NULL_HANDLE;
	};
}