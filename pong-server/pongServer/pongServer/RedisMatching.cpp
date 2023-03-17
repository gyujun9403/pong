#include "RedisMatching.hpp"
#include <format>
#ifdef _MSC_VER
	#include <winsock2.h>
#endif

//RedisMatching::RedisMatching(uint16_t isunix, std::string ip, uint16_t port)
//: m_isunix(isunix), m_IP(ip), m_PORT(port)
//{
//}

std::vector<std::string> RedisMatching::getFromMatchQueue()
{
	std::unique_lock<std::mutex> matchQueueLock(m_matchingQueueMutex);
	size_t size = m_matchingQueue.size();
	std::vector<std::string> rt;
	while (size--)
	{
		rt.emplace_back(m_matchingQueue.front());
		m_matchingQueue.pop();
	}
	//return std::move(rt);
	return rt;
}

void RedisMatching::pushToMatchResultQueue(std::string str)
{
	std::unique_lock<std::mutex> matchResultQueueLock(m_matchResultQueueMutex);
	m_matchResultQueue.push(std::move(str));
}

void RedisMatching::pushToGameResultQueue(std::string str)
{
	std::unique_lock<std::mutex> matchQueueLock(m_gameResultQueueMutex);
	m_gameResultQueue.push(std::move(str));
}

RedisMatching::RedisMatching(std::string ip, uint16_t port)
: m_IP(ip), m_PORT(port)
{
}

RedisMatching::~RedisMatching()
{
	redisFree(m_c);
}

void RedisMatching::pushToMatchQueue(std::string str)
{
	std::unique_lock<std::mutex> matchQueueLock(m_matchingQueueMutex);
	m_matchingQueue.push(std::move(str));
}

std::vector<std::string> RedisMatching::getFormMatchResultQueue()
{
	std::unique_lock<std::mutex> matchResultQueueLock(m_matchResultQueueMutex);
	size_t size = m_matchResultQueue.size();
	std::vector<std::string> rt;
	while (size--)
	{
		rt.emplace_back(m_matchResultQueue.front());
		m_matchResultQueue.pop();
	}
	//return std::move(rt);
	return rt;
}

std::vector<std::string> RedisMatching::getFormGameResultQueue()
{
	std::unique_lock<std::mutex> gameResultQueueLock(m_gameResultQueueMutex);
	size_t size = m_gameResultQueue.size();
	std::vector<std::string> rt;
	while (size--)
	{
		rt.emplace_back(m_gameResultQueue.front());
		m_gameResultQueue.pop();
	}
	//return std::move(rt); //Severity	Code	Description	Project	File	Line	Suppression State Warning	C26479	Don't use std::move to return a local variable. (f.48).	pongServer	C:\Users\gujun\git\pong\pong-server\pongServer\pongServer\RedisMatching.cpp	75	
	return rt;
}


bool RedisMatching::RedisInit()
{
	struct timeval timeout = { 1, 500000 }; // 1.5 seconds
	m_c = redisConnectWithTimeout(m_IP.c_str(), m_PORT, timeout);
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

void RedisMatching::runSendMatchingThread()
{
	m_redisMatchingThread = std::thread
	(
		[this]()
		{
			while (1)
			{
				std::vector<std::string> members = getFromMatchQueue();
				// matching queue내용들 집어넣기
				if (members.size() > 0)
				{
					for(std::string& elem : members)
					{
						std::string cmdFormat = "RPUSH matching " + elem;
						redisReply* reply = (redisReply*)redisCommand(m_c, cmdFormat.c_str());
						if (reply == NULL) {
							printf("redisCommand reply is NULL: %s\n", m_c->errstr);
							return;
						}
						if (reply->type == REDIS_REPLY_ERROR) {
							printf("Command Error: %s\n", reply->str);
							freeReplyObject(reply);
							return;
						}
						freeReplyObject(reply); //hi
					}
				}
				// matchedqueue에 매칭된애들 집어넣기
				while (1)
				{
					redisReply* reply = (redisReply*)redisCommand(m_c, "LPOP matched");
					if (reply == NULL)
					{
						printf("redisCommand reply is NULL: %s\n", m_c->errstr);
						break ;
					}
					if (reply->type == REDIS_REPLY_ERROR)
					{
						printf("Command Error: %s\n", reply->str);
						freeReplyObject(reply);
						break ;
					}
					if (reply->str == NULL)
					{
						freeReplyObject(reply);
						break;
					}
					pushToMatchResultQueue(std::string(reply->str));
					freeReplyObject(reply);
				}
				// resultqueue에 결과 나온 애들 집어넣기
				while (1)
				{
					redisReply* reply = (redisReply*)redisCommand(m_c, "LPOP gameResult");
					if (reply == NULL)
					{
						printf("redisCommand reply is NULL: %s\n", m_c->errstr);
						break;
					}
					if (reply->type == REDIS_REPLY_ERROR)
					{
						printf("Command Error: %s\n", reply->str);
						freeReplyObject(reply);
						break;
					}
					if (reply->str == NULL)
					{
						freeReplyObject(reply);
						break;
					}
					pushToGameResultQueue(std::string(reply->str));
					freeReplyObject(reply);
				}
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
			};
		}
	);
}

void RedisMatching::runRecvMatchingThread()
{
	m_redisMatchingThread = std::thread
	(
		[this]()
		{
			while (1)
			{
				redisReply* reply = (redisReply*)redisCommand(m_c, "LPOP matching");
				if (reply == NULL) {
					printf("redisCommand reply is NULL: %s\n", m_c->errstr);
					return;
				}
				if (reply->type == REDIS_REPLY_ERROR) {
					printf("Command Error: %s\n", reply->str);
					freeReplyObject(reply);
					return;
				}
				if (reply->str != NULL)
				{
					pushToMatchQueue(reply->str);
				}
				freeReplyObject(reply);
				std::vector<std::string> matchedMembers = getFormMatchResultQueue();
				// matching queue내용들 집어넣기
				if (matchedMembers.size() > 0)
				{
					for (std::string& elem : matchedMembers)
					{
						std::string cmdFormat = "RPUSH matched " + elem;
						redisReply* reply = (redisReply*)redisCommand(m_c, cmdFormat.c_str());
						if (reply == NULL) {
							printf("redisCommand reply is NULL: %s\n", m_c->errstr);
							return;
						}
						if (reply->type == REDIS_REPLY_ERROR) {
							printf("Command Error: %s\n", reply->str);
							freeReplyObject(reply);
							return;
						}
						freeReplyObject(reply); //hi
					}
				}
				std::vector<std::string> gameDoneMembers = getFormGameResultQueue();
				// matching queue내용들 집어넣기
				if (gameDoneMembers.size() > 0)
				{
					for (std::string& elem : gameDoneMembers)
					{
						std::string cmdFormat = "RPUSH gameResult " + elem;
						redisReply* reply = (redisReply*)redisCommand(m_c, cmdFormat.c_str());
						if (reply == NULL) {
							printf("redisCommand reply is NULL: %s\n", m_c->errstr);
							return;
						}
						if (reply->type == REDIS_REPLY_ERROR) {
							printf("Command Error: %s\n", reply->str);
							freeReplyObject(reply);
							return;
						}
						freeReplyObject(reply); //hi
					}
				}
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
			};
		}
	);
}