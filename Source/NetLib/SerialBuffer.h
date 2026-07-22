#pragma once

namespace cov1013
{
	class SerialBuffer
	{
	public:
		SerialBuffer();
		SerialBuffer(const int Capacity);
		virtual ~SerialBuffer();
		void Clear();
		bool IsEmpty() const;
		bool IsPull() const;
		int GetReadableSize() const;
		int GetWritableSize() const;
		inline int GetCapacity() const		{ return _capacity; };
		inline char* GetEntryPos() const	{ return _pBuffer; };
		inline char* GetReadPos() const		{ return _pBuffer + _readPos; };
		inline char* GetWritePos() const	{ return _pBuffer + _writePos; };
		int DoMoveReadPos(const int Length);
		int DoMoveWritePos(const int Length);
		int DoRead(char* pDestination, const int Length, const bool bIsPeek = false);
		int DoPeek(char* pDestination, const int Length);
		int DoWrite(const char* pSource, const int Length);
		int DoRead(wchar_t* pDestination, const int Length, const bool bIsPeek = false);
		int DoPeek(wchar_t* pDestination, const int Length);
		int DoWrite(const wchar_t* pSource, const int Length);

		template <typename T> int DoRead(T& destination, const bool bIsPeek = false)
		{
			const int ReadSize = sizeof(T);
			const int ReadableSize = GetReadableSize();

			if (ReadSize > ReadableSize)
			{
				return 0;
			}

			T* pReadPos = reinterpret_cast<T*>(GetReadPos());
			destination = *pReadPos;

			if (!bIsPeek)
			{
				const int MovedSize = DoMoveReadPos(ReadSize);
				if (MovedSize != ReadSize)
				{
					*(int*)(0x00000000) = 0;
				}
			}

			return ReadSize;
		}

		template <typename T> int DoWrite(const T& source)
		{
			const int WriteSize = sizeof(T);
			const int WritableSize = GetWritableSize();

			if (WriteSize > WritableSize)
			{
				return 0;
			}

			T* pWritePos = reinterpret_cast<T*>(GetWritePos());
			*pWritePos = source;

			const int WritedSize = DoMoveWritePos(WriteSize);

			return WritedSize;
		}

		template <typename T> int DoPeek(T& destination) const
		{
			return DoRead(destination);
		}

		template <typename T> SerialBuffer& operator>> (T& destination)
		{
			DoRead(destination);
			return *this;
		}

		template <typename T> SerialBuffer& operator<< (const T& source)
		{
			DoWrite(source);
			return *this;
		}

	protected:
		const int DefaultCapacity = 1024 * 4;		// 4KB

		char*	_pBuffer = nullptr;
		int		_readPos = 0;
		int		_writePos = 0;
		int		_capacity = 0;
	};
}