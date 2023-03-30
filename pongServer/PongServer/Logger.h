#pragma once

#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <queue>
#include <atomic>
#include <condition_variable>
#include <chrono>
#include <ctime>
#include <iomanip>

enum class LogLevel : uint16_t
{
	INFO,
	WARNING,
	ERR,
	NOSTAMP
};

class Logger
{
private:
	std::mutex m_logQueueMutex;
	std::condition_variable m_logQueueCV;
	std::queue<std::pair<LogLevel, const std::string> > m_logQueue;
	std::thread m_loggingThread;
	std::atomic<bool> m_isloggerRun;

	void loggingThreadFunc();
public:
	Logger ();
	~Logger();
	Logger(Logger const&) = delete;
	void operator=(Logger const&) = delete;
	void log(LogLevel level, const std::string& message);
};

