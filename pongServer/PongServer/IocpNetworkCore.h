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
#include "Logger.h"

class IocpNetworkCore
{
private:
	SOCKET m_listenSocket;
	SOCKADDR_IN m_serverAddr;
	HANDLE m_iocpHandle;
	const uint16_t m_workerThreadNum;
	const uint32_t m_clientNum;
	std::vector<ClientInfo, std::allocator<ClientInfo> > m_clients;
	Logger* m_logger;

	std::atomic_bool m_isWorkersRun;
	std::atomic_bool m_isAccpterRun;
	std::thread m_accecpThread;
	std::thread m_kickThread;
	std::vector<std::thread> m_workers;

	std::mutex m_recvQueueMutex;
	std::queue<std::pair<int32_t, std::vector<char> > > m_recvResultQueue;
	
	std::mutex m_closedUserIndexQueueMutex;
	std::queue<int32_t> m_closedUserIndexQueue;

	std::mutex m_kickUserIndexQueueMutex;
	std::queue<int32_t> m_kickUserIndexQueue;
	std::condition_variable m_kickUserCV;

	void init();
	void bind();
	void listen();
	void iocpInit();
	void iocpRun();
	void workerThreadFunc();
	void acceptThreadFunc();
	void kickThreadTunc();
	void joinThreads();
	void closeHandle();
	void closeSocket();
	void closeClient(ClientInfo& clientInfo, bool forceClose);
	void recv(ClientInfo& clientInfo);
	void send(ClientInfo& clientInfo);
	std::string makeErrorStr(const std::string errFunc);
public:
	IocpNetworkCore() = delete;
	IocpNetworkCore(uint32_t clientNum, uint16_t workerThreadNum, uint16_t port, Logger* logger);
	~IocpNetworkCore() {}
	void pushToSendQueue(uint16_t clientIndex, std::vector<char> packet);
	std::pair<int, std::vector<char> > getFromRecvQueue();
	int32_t getCloseUser();
	void kickUser(std::vector<int32_t> users);
	virtual bool initServer();
	virtual bool upServer();
	virtual void downServer();
};