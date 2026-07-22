#include <random>
#include "../../Source/NetLib/RingBuffer.h"

int main()
{
    std::random_device rd;
    std::mt19937 gen(rd());

    std::uniform_int_distribution<int> dis1(0, 81);
    std::uniform_int_distribution<int> dis2(0, 1024);

    constexpr int BUFFER_SISE = 2;
    cov1013::RingBuffer ringBuffer(BUFFER_SISE);

    const char* data = "1234567890 abcdefghijklmnopqrstuvwxyz 1234567890 abcdefghijklmnopqrstuvwxyz 12345";
    int dataIdx = 0;
    int dataLen = strlen(data);
    
    char readBuf[1024];

    while(true)
    {
        // 1) 랜덤으로 쓰기
        int writeSize = dis1(gen);
        if ((writeSize + dataIdx) > dataLen)
        {
            // 랜덤으로 쓸 크기가 데이터 최대 길이를 초과한다면, 길이 조정
            writeSize = dataLen - dataIdx;
        }

        int writedSize = ringBuffer.DoWrite(&data[dataIdx], writeSize);
        dataIdx = dataIdx + writedSize;
        if (dataIdx >= dataLen)
        {
            dataIdx = 0;
        }

        // 2) 랜덤으로 읽기
        memset(readBuf, 0, sizeof(char) * 1024);
        int readSize = dis2(gen);
        int readedSize = ringBuffer.DoRead(readBuf, readSize);

        printf_s("%s", readBuf);
    }

    return 0;
}