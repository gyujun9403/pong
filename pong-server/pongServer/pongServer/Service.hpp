#pragma once

#include <thread>
#include <mutex>
#include <map>
#include <functional>
#include "UserManager.hpp"
#include "IOCPServer.h"
#include "PacketsDefine.hpp"

#define PORT 3334
#define WORKER_THREAD_NUM 4
#define USER_MAX_NUM 365

class Service
{
	// 생성자 선언시, UserInfos를 받게 한다.
	typedef std::function<int(int, std::vector<char>)> FuncType;
private:
	std::thread m_serviceThread;
	IocpServer* m_network;
	UserManager* m_userManager;
	void serviceThread();
	std::atomic<bool> m_isServiceRun;
	std::map<PACKET_ID, FuncType> m_packetProcessMap;
public:
	Service(IocpServer* network, UserManager* userManager);
	void serviceInit();
	void runService(); //서비스 스레드를 돌리는 역할.
	void joinService();
	int divergePackets(std::pair<int, std::vector<char> > packetSet);
	int packetProcessLoginRequest(int clinetIndex, std::vector<char> packet);
};