#pragma once

#include <cstdint>
#include <string>
#include <queue>
#include <mutex>
#include <winsock2.h>

const UINT32 SOCKBUFFERSIZE = 300;

enum class IOOperation { RECV, SEND };

struct ExOverlapped
{
	WSAOVERLAPPED wsaOverlapped;
	WSABUF		wsaBuf;
	IOOperation ioOperation;
};

class ClientInfo
{
public:
	uint32_t index = 0;
	SOCKET clientSocket = INVALID_SOCKET;
	ExOverlapped recvOverlapped;
	ExOverlapped sendOverlapped;

	uint32_t recvedLen = 0;
	std::mutex recvBufMutex;
	char recvBuf[SOCKBUFFERSIZE] = {};

	std::mutex sendQueueMutex;
	char sendBuf[SOCKBUFFERSIZE] = {};
	std::queue<std::vector<char> > sendQueue;

	ClientInfo() {};
	~ClientInfo() {};
	ClientInfo(const ClientInfo& other)
	:index(other.index), clientSocket(other.clientSocket), recvOverlapped(other.recvOverlapped), sendOverlapped(other.sendOverlapped)
	{}

	void init(const uint32_t idx)
	{
		index = idx;
		ZeroMemory(&recvOverlapped, sizeof(recvOverlapped));
		ZeroMemory(&sendOverlapped, sizeof(sendOverlapped));
	}

	void clearClientInfo()
	{
		std::unique_lock<std::mutex> clearLock(sendQueueMutex);
		while (sendQueue.empty() == false)
		{
			sendQueue.pop();
		}
		clientSocket = INVALID_SOCKET;
		ZeroMemory(&recvOverlapped, sizeof(recvOverlapped));
		ZeroMemory(&sendOverlapped, sizeof(sendOverlapped));
	}
};