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
	// ������ �����, UserInfos�� �ް� �Ѵ�.
private:
	std::thread m_serviceThread;
	IocpServer* m_network;
	UserManager* m_userManager;
	void serviceThread();
	std::atomic<bool> m_isServiceRun;
public:
	Service(IocpServer* network, UserManager* userManager);
	void runService(); //���� �����带 ������ ����.
	void joinService();
};