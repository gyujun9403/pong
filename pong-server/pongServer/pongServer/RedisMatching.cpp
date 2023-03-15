#include "RedisMatching.hpp"
#ifdef _MSC_VER
	#include <winsock2.h>
#endif

//RedisMatching::RedisMatching(uint16_t isunix, std::string ip, uint16_t port)
//: m_isunix(isunix), m_IP(ip), m_PORT(port)
//{
//}

RedisMatching::RedisMatching(std::string ip, uint16_t port)
: m_IP(ip), m_PORT(port)
{
}

RedisMatching::~RedisMatching()
{
	redisFree(m_c);
}

void RedisMatching::pushToMatchqueue(std::vector<GameUserInfo> members)
{
	std::unique_lock<std::mutex> matchingQueueLock(m_matchingQueueMutex);
	m_matchingQueue.push(std::move(members));
	matchingQueueLock.unlock();  // 큐 뮤텍스를 언락
	m_matchingQueue_condvar.notify_one();
}

std::vector<GameUserInfo> RedisMatching::getFromMatchqueue()
{
	std::unique_lock<std::mutex> lock(m_matchingQueueMutex);
	m_matchingQueue_condvar.wait
	(lock,
		[this]()
		{
			return !m_matchingQueue.empty();
		}
	);
	std::vector<GameUserInfo> members = m_matchingQueue.front();
	m_matchingQueue.pop();
	return members;
}

std::vector<GameUserInfo> RedisMatching::getFormMatchResult()
{
	return std::vector<GameUserInfo>();
}

bool RedisMatching::RedisInit()
{
	struct timeval timeout = { 1, 500000 }; // 1.5 seconds
	//m_IP = ip;
	//m_PORT = port;
	//if (m_isunix)
	//{
	//	m_c = redisConnectUnixWithTimeout(m_IP.c_str(), timeout);
	//}
	//else
	//{
		m_c = redisConnectWithTimeout(m_IP.c_str(), m_PORT, timeout);
	//}
	if (m_c == NULL || m_c->err)
	{
		if (m_c)
		{
			std::cerr << "\033[31mConnection error: " << m_c->errstr << "\033[0m" << std::endl;
			redisFree(m_c);
		}
		else
		{
			std::cerr << "\033[31mConnection error: can't allocate redis context\033[0m\n" << std::endl;
		}
		return false;
	}
	std::cout << "Redis Server Init done" << std::endl;
	return true;
}

void RedisMatching::runMatchThread()
{
	m_redisMatchingThread = std::thread
	(
		[this]()
		{
			while (1)
			{
				std::vector<GameUserInfo> members = getFromMatchqueue();
				std::cout << members[0].toString() << " + " << members[1].toString() << std::endl;
			};
		}
	);
}