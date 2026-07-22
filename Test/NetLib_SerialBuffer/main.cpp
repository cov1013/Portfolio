#include <intrin.h>
#include <float.h>
#include "../../Source/NetLib/SerialBuffer.h"

using namespace cov1013;

inline static void Crash()
{
	*reinterpret_cast<char*>(0x00) = 0;
}

int main()
{
	SerialBuffer serialBuffer(1024);

	char ch = 'a';
	wchar_t wch = L'a';
	int i = INT_MAX;
	short s = SHRT_MAX;
	float f = FLT_MAX;
	double d = DBL_MAX;

	char ch2 = '\0';
	wchar_t wch2 = L'\n';
	int i2 = 0;
	short s2 = 0;
	float f2 = 0;
	double d2 = 0;

	int writedSize = 0;
	int readedSize = 0;

	// 함수 호출
	writedSize += serialBuffer.DoWrite<char>(ch);
	writedSize += serialBuffer.DoWrite<wchar_t>(wch);
	writedSize += serialBuffer.DoWrite<int>(i);
	writedSize += serialBuffer.DoWrite<short>(s);
	writedSize += serialBuffer.DoWrite<float>(f);
	writedSize += serialBuffer.DoWrite<double>(d);
	readedSize += serialBuffer.DoRead<char>(ch2);
	readedSize += serialBuffer.DoRead<wchar_t>(wch2);
	readedSize += serialBuffer.DoRead<int>(i2);
	readedSize += serialBuffer.DoRead<short>(s2);
	readedSize += serialBuffer.DoRead<float>(f2);
	readedSize += serialBuffer.DoRead<double>(d2);

	if (writedSize != readedSize)
	{
		Crash();
	}

	// 연산자 오버로딩 (1)
	serialBuffer.Clear();
	serialBuffer << ch;
	serialBuffer << wch;
	serialBuffer << i;
	serialBuffer << s;
	serialBuffer << f;
	serialBuffer << d;
	serialBuffer >> ch2;
	serialBuffer >> wch2;
	serialBuffer >> i2;
	serialBuffer >> s2;
	serialBuffer >> f2;
	serialBuffer >> d2;
	serialBuffer.Clear();

	// 연산자 오버로딩 (2)
	serialBuffer << ch << wch << i << s << f << d;
	serialBuffer >> ch2 >> wch2 >> i2 >> s2 >> f2 >> d2;
	serialBuffer.Clear();

	return 0;
}