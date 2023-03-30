#include "logger.h"

void Logger::loggingThreadFunc()
{
	struct tm local_time;
	char timestamp[32];
	time_t now;
	while (m_isloggerRun.load())
	{
		std::unique_lock<std::mutex> loggerLock(m_logQueueMutex);
		m_logQueueCV.wait(loggerLock,
			[this]
			{
				return m_isloggerRun.load() == false || m_logQueue.empty() == false;
			}
			);
		now = time(nullptr);
		localtime_s(&local_time, &now);
		strftime(timestamp, sizeof(timestamp), "[%Y-%m-%d %H:%M:%S]", &local_time);

		while (m_logQueue.empty() == false)
		{
			if (m_logQueue.front().first == LogLevel::ERR)
			{
				// 빨간 텍스트로 출력
				std::cout << "\n\031[32m" << timestamp << "[error] " << m_logQueue.front().second << std::endl << "\031[0m";
			}
			else if (m_logQueue.front().first == LogLevel::WARNING)
			{
				// 노란 텍스트로 출력
				std::cout << "\n\031[93m" << timestamp << "[warning] " << m_logQueue.front().second << std::endl << "\031[0m";
			}
			else if (m_logQueue.front().first == LogLevel::INFO)
			{
				std::cout << timestamp << "[log] " << m_logQueue.front().second << std::endl;
			}
			else if (m_logQueue.front().first == LogLevel::NOSTAMP)
			{
				std::cout << m_logQueue.front().second << std::endl;
			}
			m_logQueue.pop();
		}
	}
}

Logger::Logger()
	: m_isloggerRun(true), m_loggingThread(&Logger::loggingThreadFunc, this)
{
}

Logger::~Logger()
{
	m_isloggerRun.store(false);
	m_logQueueCV.notify_one();
	m_loggingThread.join();
}




void Logger::log(LogLevel level, const std::string& message)
{
	std::unique_lock<std::mutex> loggerLock(m_logQueueMutex);
	m_logQueue.emplace(std::move(std::make_pair(level, message)));
	m_logQueueCV.notify_one();
}
