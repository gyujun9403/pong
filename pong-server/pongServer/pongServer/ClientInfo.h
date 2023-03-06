#pragma once

#include <cstdint>
#include <string>
#include <queue>
#include <mutex>
#include <winsock2.h>

const UINT32 SOCKBUFFERSIZE = 256;

enum class IOOperation { RECV, SEND };

//WSAOVERLAPPED구조체를 확장 시켜서 필요한 정보를 더 넣었다.
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

	std::queue<std::string> sendQueue;
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
		// recv buf는 짜피 받을때 전달받은 trans된 문자열 길이만큼 가져오기 때문에, 따로 초기화 할 필요 없을듯.
		// 다만, 대기 중인경우 이를 대기열에서 없애는 정도는 해줘야 할 거 같은데... -> 이건 서버쪽에서 해줘야할듯
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