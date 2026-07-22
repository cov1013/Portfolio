#pragma once

namespace cov1013
{
	class RingBuffer
	{
		static constexpr int BlankSize = 1;
		static constexpr int DefaultCapacity = 1024 * 4;
		static constexpr int MaxCapacity = 1024 * 1024;

	public:
		RingBuffer();
		RingBuffer(const int Capacity);
		virtual ~RingBuffer();
		void Clear();
		bool IsEmpty() const;
		bool IsPull() const;
		int GetReadableSize() const;
		int GetWritableSize() const;
		int GetNonBrokenReadableSize() const;
		int GetNonBrokenWritableSize() const;
		inline int GetCapacity() const		{ return _capacity; };
		inline char* GetEntryPos() const	{ return _pBuffer; };
		inline char* GetReadPos() const		{ return _pBuffer + _readPos; };
		inline char* GetWritePos() const	{ return _pBuffer + _writePos; };
		void DoMoveReadPos(const int Length);
		void DoMoveWritePos(const int Length);
		int DoRead(char* pDestination, const int Length, const bool bIsPeek = false);
		int DoPeek(char* pDestination, const int Length);
		int DoWrite(const char* pSource, const int Length);

	private:

		char*	_pBuffer = nullptr;
		int		_capacity = 0;
		int		_readPos = 0;
		int		_writePos = 0;
	};
}