#include "AsyncRedis.hpp"
#include <format>
#ifdef _MSC_VER
	#include <winsock2.h>
#endif

AsyncRedis::AsyncRedis(std::string ip, uint16_t port, Logger* logger)
: m_IP(ip), m_PORT(port), m_isRedisRun(true)
{
	m_logger = logger;
}

AsyncRedis::~AsyncRedis()
{
	redisFree(m_c);
}

void AsyncRedis::pushToMatchQueue(std::string str)
{
	std::unique_lock<std::mutex> matchQueueLock(m_matchingQueueMutex);
	m_matchingQueue.push(std::move(str));
}

std::vector<std::string> AsyncRedis::getFromMatchQueue()
{
	std::unique_lock<std::mutex> matchQueueLock(m_matchingQueueMutex);
	size_t size = m_matchingQueue.size();
	std::vector<std::string> rt;
	while (size--)
	{
		rt.emplace_back(m_matchingQueue.front());
		m_matchingQueue.pop();
	}
	return rt;
}

void AsyncRedis::pushToMatchResultQueue(std::string str)
{
	std::unique_lock<std::mutex> matchResultQueueLock(m_matchResultQueueMutex);
	m_matchResultQueue.push(std::move(str));
}

void AsyncRedis::pushToGameResultQueue(std::string str)
{
	std::unique_lock<std::mutex> matchQueueLock(m_gameResultQueueMutex);
	m_gameResultQueue.push(std::move(str));
}

std::vector<std::string> AsyncRedis::getFormMatchResultQueue()
{
	std::unique_lock<std::mutex> matchResultQueueLock(m_matchResultQueueMutex);
	size_t size = m_matchResultQueue.size();
	std::vector<std::string> rt;
	while (size--)
	{
		rt.emplace_back(m_matchResultQueue.front());
		m_matchResultQueue.pop();
	}
	return rt;
}

std::vector<std::string> AsyncRedis::getFormGameResultQueue()
{
	std::unique_lock<std::mutex> gameResultQueueLock(m_gameResultQueueMutex);
	size_t size = m_gameResultQueue.size();
	std::vector<std::string> rt;
	while (size--)
	{
		rt.emplace_back(m_gameResultQueue.front());
		m_gameResultQueue.pop();
	}
	return rt;
}


bool AsyncRedis::RedisInit()
{
	struct timeval timeout = { 1, 500000 };
	m_c = redisConnectWithTimeout(m_IP.c_str(), m_PORT, timeout);
	if (m_c == NULL || m_c->err)
	{
		if (m_c)
		{
			m_logger->log(LogLevel::ERR, std::string("Connection error: %s") + m_c->errstr);
			redisFree(m_c);
		}
		else
		{
			m_logger->log(LogLevel::ERR, "Connection error: can't allocate redis context");
		}
		return false;
	}
	m_logger->log(LogLevel::INFO, "Redis Server Init done");
	return true;
}

void AsyncRedis::RedisJoin()
{
	m_isRedisRun.store(false);
	m_redisThread.join();
	redisFree(m_c);
}

void AsyncRedis::runRedisThread4MC()
{
	m_redisThread = std::thread
	(
		[this]()
		{
			while (m_isRedisRun.load())
			{
				std::vector<std::string> members = getFromMatchQueue();
				// matchingqueue =(push)=> Redis
				if (members.size() > 0)
				{
					for(std::string& elem : members)
					{
						std::string cmdFormat = "RPUSH matching " + elem;
						redisReply* reply = (redisReply*)redisCommand(m_c, cmdFormat.c_str());
						if (reply == NULL)
						{
							printf("redisCommand reply is NULL: %s\n", m_c->errstr);
							return;
						}
						if (reply->type == REDIS_REPLY_ERROR)
						{
							printf("Command Error: %s\n", reply->str);
							freeReplyObject(reply);
							return;
						}
						freeReplyObject(reply); 
					}
				}
				// matchedqueue <=(pop)= Redis
				while (1)
				{
					redisReply* reply = (redisReply*)redisCommand(m_c, "LPOP matched");
					if (reply == NULL)
					{
						printf("redisCommand reply is NULL: %s\n", m_c->errstr);
						break ;
					}
					else if (reply->type == REDIS_REPLY_ERROR)
					{
						printf("Command Error: %s\n", reply->str);
						freeReplyObject(reply);
						break ;
					}
					else if (reply->str == NULL)
					{
						freeReplyObject(reply);
						break;
					}
					pushToMatchResultQueue(std::string(reply->str));
					freeReplyObject(reply);
				}
				// matchedqueue <=(pop)= Redis
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

void AsyncRedis::runRedisThread4Game()
{
	m_redisThread = std::thread
	(
		[this]()
		{
			// matchingQueue <=(pop)= Redis
			while (m_isRedisRun.load())
			{
				while (1)
				{
					redisReply* reply = (redisReply*)redisCommand(m_c, "LPOP matching");
					if (reply == NULL) {
						printf("redisCommand reply is NULL: %s\n", m_c->errstr);
						return;
					}
					else if (reply->type == REDIS_REPLY_ERROR) {
						printf("Command Error: %s\n", reply->str);
						freeReplyObject(reply);
						return;
					}
					else if (reply->str == NULL)
					{
						freeReplyObject(reply);
						break;
					}
					pushToMatchQueue(reply->str);
					freeReplyObject(reply);
				}
				// matchResultQueue =(push)=> Redis
				std::vector<std::string> matchedMembers = getFormMatchResultQueue();
				if (matchedMembers.size() > 0)
				{
					for (std::string& elem : matchedMembers)
					{
						std::string cmdFormat = "RPUSH matched " + elem;
						redisReply* reply = (redisReply*)redisCommand(m_c, cmdFormat.c_str());
						if (reply == NULL)
						{
							printf("redisCommand reply is NULL: %s\n", m_c->errstr);
							return;
						}
						if (reply->type == REDIS_REPLY_ERROR)
						{
							printf("Command Error: %s\n", reply->str);
							freeReplyObject(reply);
							return;
						}
						freeReplyObject(reply);
					}
				}
				// gameResultQueue =(push)=> Redis
				std::vector<std::string> gameDoneMembers = getFormGameResultQueue();
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
						freeReplyObject(reply);
					}
				}
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
			}
		}
	);
}