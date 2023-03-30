#pragma once

#include <winsock2.h>
#include <typeinfo>
#include <iostream>
#include <hiredis.h>
#include <string>
#include <queue>
#include <mutex>
#include <thread>
#include "GameUserInfo.h"
#include "Logger.h"

class AsyncRedis
{
private:
	Logger* m_logger;
	std::string m_IP;
	uint16_t m_PORT;
	uint64_t m_keysCnt = 0;
	redisContext* m_c;

	std::thread m_redisThread;

	std::queue<std::string> m_matchingQueue;
	std::mutex m_matchingQueueMutex;

	std::queue<std::string> m_matchResultQueue;
	std::mutex m_matchResultQueueMutex;

	std::queue<std::string> m_gameResultQueue;
	std::mutex m_gameResultQueueMutex;
public:
	AsyncRedis(std::string ip, uint16_t port, Logger* logger);
	void runSendMatchingThread();
	void runRecvMatchingThread();
	bool RedisInit();
	~AsyncRedis();
	// ��ü �������� ���.
	void pushToMatchQueue(std::string str);
	std::vector<std::string> getFormMatchResultQueue();
	std::vector<std::string> getFormGameResultQueue();
	// ���� �������� ���.
	std::vector<std::string> getFromMatchQueue();
	void pushToMatchResultQueue(std::string str);
	void pushToGameResultQueue(std::string str);
};