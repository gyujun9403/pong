#pragma once
#pragma comment(lib, "ws2_32")

#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <cstdint>
#include <exception>
#include <winsock2.h>
#include "IServer.h"
#include "ClientInfo.h"

class IocpServer : public IServer
{
private:
	SOCKET m_listenSocket; // ��������?
	SOCKADDR_IN m_serverAddr;
	const uint16_t m_workerThreadNum;
	HANDLE m_iocpHandle;
	std::thread m_accecpThread;
	std::vector<std::thread> m_workers;
	std::vector<ClientInfo> m_clients;
	const uint32_t m_clientNum;
	std::atomic_bool m_isWorkersRun;
	std::atomic_bool m_isAccpterRun;

public:
	IocpServer() = delete;
	// ���� ������, ��Ŀ������ ����
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
	void send(ClientInfo& clientInfo, std::string str);
};