#pragma once

#include <cstdint>
#include <string>
#include <queue>
#include <mutex>
#include <winsock2.h>

//const UINT32 SOCKBUFFERSIZE = 256;
const UINT32 SOCKBUFFERSIZE = 300;

enum class IOOperation { RECV, SEND };

//WSAOVERLAPPED����ü�� Ȯ�� ���Ѽ� �ʿ��� ������ �� �־���.
struct ExOverlapped
{
	WSAOVERLAPPED wsaOverlapped;
	WSABUF		wsaBuf;
	IOOperation ioOperation;
	//char buf[SOCKBUFFERSIZE];
};

class ClientInfo
{
public:
	uint32_t index = 0;
	SOCKET clientSocket = INVALID_SOCKET;
	ExOverlapped recvOverlapped;
	ExOverlapped sendOverlapped;

	char recvBuf[SOCKBUFFERSIZE] = {};
	uint32_t recvedLen = 0;
	std::mutex recvBufMutex;

	char sendBuf[SOCKBUFFERSIZE] = {};
	std::queue<std::vector<char> > sendQueue;
	std::mutex sendQueueMutex;

	//std::atomic<bool> m_isDisconnecting = false;
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
		// recv buf�� ¥�� ������ ���޹��� trans�� ���ڿ� ���̸�ŭ �������� ������, ���� �ʱ�ȭ �� �ʿ� ������.
		// �ٸ�, ��� ���ΰ�� �̸� ��⿭���� ���ִ� ������ ����� �� �� ������... -> �̰� �����ʿ��� ������ҵ�
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