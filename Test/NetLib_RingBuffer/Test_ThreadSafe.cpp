#include <iostream>
#include <thread>
#include <queue>
#include <random>
#include "../../Source/NetLib/RingBuffer.h"

static constexpr int BUFFER_SIZE = 2;
static cov1013::RingBuffer g_ringBuffer(BUFFER_SIZE);

int main()
{
	std::thread writer = std::thread([]() 
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int> dis(0, 81);

        const char* data = "1234567890 abcdefghijklmnopqrstuvwxyz 1234567890 abcdefghijklmnopqrstuvwxyz 12345";
        int dataIdx = 0;
        int dataLen = strlen(data); // 0 ~ 81

        while (true)
        {
            int writeSize = dis(gen);
            if ((writeSize + dataIdx) > dataLen)
            {
                writeSize = dataLen - dataIdx;
            }

            int writedSize = g_ringBuffer.DoWrite(&data[dataIdx], writeSize);
            dataIdx = dataIdx + writedSize;
            if (dataIdx >= dataLen)
            {
                dataIdx = 0;
            }
        }
	});

	std::thread reader = std::thread([]() 
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int> dis(0, 1024);
        char buffer[1024];

        while (true)
        {
            memset(buffer, 0, sizeof(char) * 1024);
            int readSize = dis(gen);
            int readedSize = g_ringBuffer.DoRead(buffer, readSize);

            printf_s("%s", buffer);
        }
	});

    writer.join();
    reader.join();

	return 0;
}