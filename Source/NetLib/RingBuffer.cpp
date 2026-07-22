#include <memory>
#include "RingBuffer.h"

namespace cov1013
{
    RingBuffer::RingBuffer()
    {
        _capacity = DefaultCapacity;
        _pBuffer = new char[DefaultCapacity];
    }

    RingBuffer::RingBuffer(const int Capacity)
    {
        if (Capacity <= MaxCapacity)
        {
            _capacity = Capacity;
        }
        else
        {
            _capacity = MaxCapacity;
        }
        _pBuffer = new char[_capacity];
    }

    RingBuffer::~RingBuffer()
    {
        delete[] _pBuffer;
    }

    void RingBuffer::Clear()
    {
        _readPos = 0;
        _writePos = 0;
    }

    bool RingBuffer::IsEmpty() const
    {
        const int ReadPos = _readPos;
        const int WritePos = _writePos;

        return ReadPos == WritePos;
    }

    bool RingBuffer::IsPull() const
    {
        const int WritableSize = GetWritableSize();

        return WritableSize <= 0;
    }

    int RingBuffer::GetReadableSize() const
    {
        const int ReadPos = _readPos;
        const int WritePos = _writePos;
        int readableSize = 0;

        if (WritePos >= ReadPos)
        {
            // [     [R]########[W]     ]
            readableSize = WritePos - ReadPos;
        }
        else
        {
            // [######[W]       [R]#####]
            readableSize = _capacity - (ReadPos - WritePos);
        }

        return readableSize;
    }

    int RingBuffer::GetWritableSize() const
    {
        const int ReadPos = _readPos;
        const int WritePos = _writePos;
        int writableSize = 0;

        if (WritePos >= ReadPos)
        {
            // [#####[R]       [W]#######]
            writableSize = _capacity - (WritePos - ReadPos);
        }
        else
        {
            // [     [W]########[R]      ]
            writableSize = ReadPos - WritePos;
        }

        // 헤더와 테일의 위치가 동일하면 안되므로, BlankSize만큼 제외한다.
        // W가 앞에 있을 때 : [####[B][R]       [W]######]
        // R가 앞에 있을 때 : [    [W]#######[B][R]      ]
        writableSize = writableSize - BlankSize;

        return writableSize;
    }

    int RingBuffer::GetNonBrokenReadableSize() const
    {
        const int ReadPos = _readPos;
        const int WritePos = _writePos;
        int nonBrokenReadableSize = 0;

        if (WritePos >= ReadPos)
        {
            // [[R]########[W]       ]
            nonBrokenReadableSize = WritePos - ReadPos;
        }
        else
        {
            // [[W]         [R]######]
            nonBrokenReadableSize = _capacity - ReadPos;
        }

        return nonBrokenReadableSize;
    }

    int RingBuffer::GetNonBrokenWritableSize() const
    {
        const int ReadPos = _readPos;
        const int WritePos = _writePos;
        int nonBrokenWritableSize = 0;

        if (WritePos >= ReadPos)
        {
            // [ [R]        [W]#####]
            nonBrokenWritableSize = _capacity - WritePos;
            if (ReadPos == 0)
            {
                // ReadPos 위치가 0이라면, WritePos를 버퍼 맨 끝까지 쓰게될 경우
                // [W][R]의 위치가 동일해지므로 BLANK_SIZE만큼 빼준다.
                // [[R]        [W]####[B]]
                nonBrokenWritableSize -= BlankSize;
            }
        }
        else
        {
            // R가 W보다 앞에 있는 경우 W를 버퍼 맨 끝까지 쓰게될 경우
            // W와 R의 위치가 동일해지므로 BLANK_SIZE만큼 빼준다.
            // [[W]#######[B][R]]
            nonBrokenWritableSize = ReadPos - WritePos - BlankSize;
        }

        return nonBrokenWritableSize;
    }

    void RingBuffer::DoMoveReadPos(const int Length)
    {
        int readPos = _readPos;
        readPos = readPos + Length;
        readPos = readPos % _capacity;  // 이동 후 위치가 버퍼 용량보다 크다면, 다시 앞으로 돌린다.

        _readPos = readPos;
    }

    void RingBuffer::DoMoveWritePos(const int Length)
    {
        int writePos = _writePos;
        writePos = writePos + Length;
        writePos = writePos % _capacity; // 이동 후 위치가 버퍼 용량보다 크다면, 다시 앞으로 돌린다.

        _writePos = writePos;
    }

    int RingBuffer::DoRead(char* pDestination, const int Length, const bool bIsPeek)
    {
        int length = Length;
        const int ReadableSize = GetReadableSize();

        // 1) 현재 버퍼에 읽을 수 있는 공간이 요청 길이보다 작은 경우, 요청 길이 조정
        if (length > ReadableSize)
        {
            length = ReadableSize;
        }

        const char* pReadPos = GetReadPos();
        const int NonBrokenReadableSize = GetNonBrokenReadableSize();

        // 2) 요청 길이가 끊기지 않고 읽을 수 있는 길이보다 작거나 같다면, 한 번만 복사.
        if (length <= NonBrokenReadableSize)
        {
            memcpy_s(pDestination, length, pReadPos, length);
        }
        // 3) 요청 길이가 끊기지 않고 읽을 수 있는 길이보다 크다면
        else
        {
            // 1) 끊기지 않고 읽을 수 있는 공간에 먼저 쓰고 
            // [    [W]       |####]
            memcpy_s(pDestination, NonBrokenReadableSize, pReadPos, NonBrokenReadableSize);

            // 2) 버퍼 Entry에서 비어있는 공간에 남은 데이터를 읽는다.
            // [####[W]       |####]
            char* pEntryPos = GetEntryPos();
            char* pNextData = pDestination + NonBrokenReadableSize;
            const int RemainLength = length - NonBrokenReadableSize;
            memcpy_s(pNextData, RemainLength, pEntryPos, RemainLength);
        }

        // 4) 읽은 만큼 ReadPos 이동
        if (!bIsPeek)
        {
            DoMoveReadPos(length);
        }

        return length;
    }

    int RingBuffer::DoPeek(char* pDestination, const int Length)
    {
        return DoRead(pDestination, Length, true);
    }

    int RingBuffer::DoWrite(const char* pData, const int Length)
    {
        int length = Length;

        // 1) 현재 버퍼에 쓸 수 있는 공간이 요청 길이보다 작은 경우, 요청 길이 조정
        const int WriteableSize = GetWritableSize();
        if (Length > WriteableSize)
        {
            length = WriteableSize;
        }

        char* pWritePos = GetWritePos();
        const int NonBrokenWritableSize = GetNonBrokenWritableSize();

        // 2) 요청 길이가 끊기지 않고 쓸 수 있는 길이보다 작거나 같다면, 한 번만 복사.
        if (length <= NonBrokenWritableSize)
        {
            memcpy_s(pWritePos, length, pData, length);
        }
        // 3) 요청 길이가 끊기지 않고 쓸 수 있는 길이보다 크다면
        else
        {
            // 1) 끊기지 않고 쓸 수 있는 공간에 먼저 쓰고 
            // [     [R]      [W]###]
            memcpy_s(pWritePos, NonBrokenWritableSize, pData, NonBrokenWritableSize);

            // 2) 버퍼 Entry에서 비어있는 공간에 남은 데이터를 쓴다.
            // [##[B][R]      [W]   ]
            char* pEntryPos = GetEntryPos();
            const int RemainLength = length - NonBrokenWritableSize;
            memcpy_s(pEntryPos, RemainLength, pData + NonBrokenWritableSize, RemainLength);
        }

        // 4) 쓴 만큼 WritePos 이동
        DoMoveWritePos(length);

        return length;
    }
}