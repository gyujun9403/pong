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

	std::queue<std::string> m_matchingQueue;
	std::mutex m_matchingQueueMutex;
	std::condition_variable m_matchingQueue_condvar;

	std::queue<std::string> m_matchResultQueue;
	std::mutex m_matchResultQueueMutex;

	std::queue<std::string> m_gameResultQueue;
	std::mutex m_gameResultQueueMutex;
	//RedisMatching() = delete;
	// 내부에서 접근
	// 매칭서버에서 사용.


public:
	//RedisMatching(uint16_t isunix, std::string ip, uint16_t port);
	RedisMatching(std::string ip, uint16_t port);
	void runSendMatchingThread();
	void runRecvMatchingThread();
	~RedisMatching();
	// 본체 서버에서 사용.
	void pushToMatchQueue(std::string str);
	std::vector<std::string> getFormMatchResultQueue();
	std::vector<std::string> getFormGameResultQueue();
	// 게임 서버에서 사용.
	std::vector<std::string> getFromMatchQueue(); // 컨슈머
	void pushToMatchResultQueue(std::string str); // 컨슈머
	void pushToGameResultQueue(std::string str); // 컨슈머
	bool RedisInit();
};