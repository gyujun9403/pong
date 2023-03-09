#pragma once
#pragma comment(lib, "ws2_32")

#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <cstdint>
#include <exception>
#include <winsock2.h>
#include <mutex>
#include "IServer.h"
#include "ClientInfo.h"
//#include "UserManager.hpp"

class IocpServer : public IServer
{
private:
	SOCKET m_listenSocket; // 무슨역할?
	SOCKADDR_IN m_serverAddr;
	const uint16_t m_workerThreadNum;
	HANDLE m_iocpHandle;
	std::thread m_accecpThread;
	std::vector<std::thread> m_workers;
	std::vector<ClientInfo, std::allocator<ClientInfo> > m_clients;
	// 여기 담지 말고, 그냥 클라인포에서 가져다 쓰게 하면??? -> 근데 여러개가 쌓이면 뭐부터 하게 할지 
	// 
	//UserManager
	const uint32_t m_clientNum;
	std::atomic_bool m_isWorkersRun;
	std::atomic_bool m_isAccpterRun;
	//UserManager* m_userManager;
	std::mutex m_recvQueueMutex;
	std::queue<std::pair<int, std::vector<char> > > m_recvResultQueue;
	
	std::mutex m_closedUserIndexQueueMutex;
	std::queue<int> m_closeUserIndexQueue;
	void closeClient(ClientInfo& clientInfo, bool forceClose);
public:
	IocpServer() = delete;
	// 서버 유저수, 워커스레드 개수
	IocpServer(uint32_t clientNum, uint16_t workerThreadNum, uint16_t port);
	//IocpServer(UserManager* userManager, const uint16_t workerThreadNum, const uint16_t port);
	~IocpServer() {}
	//void pushToSendQueue(uint16_t clientIndex, std::string str);
	void pushToSendQueue(uint16_t clientIndex, std::vector<char> packet);
	std::pair<int, std::vector<char> > getFromRecvQueue();
	int32_t getCloseUser();
	virtual bool initServer();
	virtual bool upServer();
	virtual void downServer();
private:

	std::string makeErrorStr(const std::string errFunc);
	void init();
	void bind();
	void listen();
	
	void iocpInit();
	void iocpRun();

	void workerThreadFunc();
	void acceptThreadFunc();

	void joinThreads();
	void closeHandle();
	void closeSocket();

	void recv(ClientInfo& clientInfo);
	/*void send(ClientInfo& clientInfo, std::string str);*/
	void send(ClientInfo& clientInfo);
	// 반환값 : 패킷 포인터
	// 인자(out) : 
	//void getFromRecvQeueu();
};