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


class RedisMatching
{
private:
	//const uint16_t m_isunix = 0;
	std::string m_IP;
	uint16_t m_PORT;
	uint64_t m_keysCnt = 0;
	redisContext* m_c;
	std::thread m_redisMatchingThread;

	std::queue<std::vector<GameUserInfo> > m_matchingQueue;
	std::mutex m_matchingQueueMutex;
	std::condition_variable m_matchingQueue_condvar;

	std::queue<std::vector<GameUserInfo> > m_matchResultQueue;
	std::mutex m_matchResultQueueMutex;
	//RedisMatching() = delete;
	std::vector<GameUserInfo> getFromMatchqueue(); // ÄÁ½´¸Ó
public:
	//RedisMatching(uint16_t isunix, std::string ip, uint16_t port);
	RedisMatching(std::string ip, uint16_t port);
	void runMatchThread();
	~RedisMatching();
	void pushToMatchqueue(std::vector<GameUserInfo> members);
	std::vector<GameUserInfo> getFormMatchResult();
	bool RedisInit();
};