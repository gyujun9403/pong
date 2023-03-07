#pragma once

#include <cstdint>
#include <string>
#include <queue>
#include <mutex>
#include <winsock2.h>

const UINT32 SOCKBUFFERSIZE = 256;

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

	std::queue<std::vector<char> > sendQueue;
	std::mutex sendQueueMutex;
	//ClientInfo(const ClientInfo& other) = delete;
	ClientInfo() {};
	~ClientInfo() {};
	ClientInfo(const ClientInfo& other)
	:index(other.index), clientSocket(other.clientSocket), recvOverlapped(other.recvOverlapped), sendOverlapped(other.sendOverlapped)
	{}
	//ClientInfo(const uint32_t idx)
	//:index(idx) {}
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