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

class IocpServer : public IServer
{
private:
	SOCKET m_listenSocket; // 무슨역할?
	SOCKADDR_IN m_serverAddr;
	const uint16_t m_workerThreadNum;
	HANDLE m_iocpHandle;
	std::thread m_accecpThread;
	std::vector<std::thread> m_workers;
	std::vector<ClientInfo> m_clients;
	const uint32_t m_clientNum;
	std::atomic_bool m_isWorkersRun;
	std::atomic_bool m_isAccpterRun;
	std::mutex m_recvQueueMutex;

public:
	IocpServer() = delete;
	// 서버 유저수, 워커스레드 개수
	IocpServer(const uint32_t clientNum, const uint16_t workerThreadNum, const uint16_t port);
	~IocpServer() {}
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
	void pushToSendQueue(ClientInfo& clientInfo, std::string str);
	// 반환값 : 패킷 포인터
	// 인자(out) : 
	//void getFromRecvQeueu();
};