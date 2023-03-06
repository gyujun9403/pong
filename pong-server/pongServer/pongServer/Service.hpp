#pragma once

#include <thread>
#include <mutex>
#include "UserManager.hpp"
#include "IOCPServer.h"

#define PORT 3334
#define WORKER_THREAD_NUM 4
#define USER_MAX_NUM 365


class Service
{
	// 생성자 선언시, UserInfos를 받게 한다.
private:
	std::thread m_serviceThread;
	IocpServer* m_network;
	UserManager* m_userManager;
	void serviceThread();
	std::atomic<bool> m_isServiceRun;
public:
	Service(IocpServer* network, UserManager* userManager);
	void runService(); //서비스 스레드를 돌리는 역할.
	void joinService();
};