#pragma once
#include <thread>

class MonitorThread
{
public:
	static MonitorThread& GetInstance()
	{
		static MonitorThread instance;
		return instance;
	}

	void Run();
	void Stop();

private:
	MonitorThread() = default;
	~MonitorThread() = default;

private:
	bool			_bActive = false;
	std::thread		_thread = {};
};