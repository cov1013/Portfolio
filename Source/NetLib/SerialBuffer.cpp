#include <memory>
#include "SerialBuffer.h"

namespace cov1013
{
	SerialBuffer::SerialBuffer()
	{
		_capacity = DefaultCapacity;
		_pBuffer = new char[DefaultCapacity];
	}

	SerialBuffer::SerialBuffer(const int Capacity)
	{
		_capacity = Capacity;
		_pBuffer = new char[_capacity];
	}

	SerialBuffer::~SerialBuffer()
	{
		delete[] _pBuffer;
	}

	void SerialBuffer::Clear()
	{
		_readPos = 0;
		_writePos = 0;
	}

	bool SerialBuffer::IsEmpty() const
	{
		const int ReadPos = _readPos;
		const int WritePos = _writePos;

		return ReadPos == WritePos;
	}

	bool SerialBuffer::IsPull() const
	{
		const int WritableSize = GetWritableSize();

		return WritableSize <= 0;
	}

	int SerialBuffer::GetReadableSize() const
	{
		const int ReadPos = _readPos;
		const int WritePos = _writePos;

		// [     [R]#########[W]]
		const int ReadableSize = WritePos - ReadPos;

		return ReadableSize;
	}

	int SerialBuffer::GetWritableSize() const
	{
		const int WritePos = _writePos;

		// [     [W]#############]
		const int WritableSize = _capacity - WritePos;

		return WritableSize;
	}

	int SerialBuffer::DoMoveReadPos(const int Length)
	{
		int length = Length;
		int readPos = _readPos;
		int writePos = _writePos;

		// 다음 이동 위치가 쓰기 위치를 초과하는 경우
		// 이동할 수 있는 길이만 이동할 수 있게 길이 갱신
		if ((readPos + length) > writePos)
		{
			length = writePos - readPos;
		}

		readPos = readPos + length;
		_readPos = readPos;

		return length;
	}

	int SerialBuffer::DoMoveWritePos(const int Length)
	{
		int length = Length;
		int writePos = _writePos;

		// 다음 이동 위치가 쓰기 위치를 초과하는 경우
		// 이동할 수 있는 길이만 이동할 수 있게 길이 갱신
		if ((writePos + length) > _capacity)
		{
			length = _capacity - writePos;
		}

		writePos = writePos + length;
		_writePos = writePos;

		return length;
	}

	int SerialBuffer::DoRead(char* pDestination, const int Length, const bool bIsPeek)
	{
		const int ReadableSize = GetReadableSize();

		if (Length > ReadableSize)
		{
			return -1;
		}

		char* pReadPos = GetReadPos();
		memcpy_s(pDestination, Length, pReadPos, Length);

		if (!bIsPeek)
		{
			DoMoveReadPos(Length);
		}

		return Length;
	}

	int SerialBuffer::DoPeek(char* pDestination, const int Length)
	{
		return DoRead(pDestination, Length, true);
	}

	int SerialBuffer::DoWrite(const char* pSource, const int Length)
	{
		const int WritableSize = GetWritableSize();

		if (Length > WritableSize)
		{
			return -1;
		}

		char* pWritePos = GetWritePos();
		memcpy_s(pWritePos, Length, pSource, Length);

		DoMoveWritePos(Length);

		return Length;
	}

	int SerialBuffer::DoRead(wchar_t* pDestination, const int Length, const bool bIsPeek)
	{
		return DoRead(reinterpret_cast<char*>(pDestination), Length, bIsPeek);
	}

	int SerialBuffer::DoPeek(wchar_t* pDestination, const int Length)
	{
		return DoRead(pDestination, Length, true);
	}

	int SerialBuffer::DoWrite(const wchar_t* pSource, const int Length)
	{
		return DoWrite(reinterpret_cast<const char*>(pSource), Length);
	}
}
