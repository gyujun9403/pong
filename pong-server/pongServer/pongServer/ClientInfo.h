#pragma once

#include <cstdint>
#include <string>
#include <queue>
#include <winsock2.h>

const UINT32 SOCKBUFFERSIZE = 256;

enum class IOOperation { RECV, SEND };

//WSAOVERLAPPED����ü�� Ȯ�� ���Ѽ� �ʿ��� ������ �� �־���.
struct ExOverlapped
{
	WSAOVERLAPPED wsaOverlapped;
	WSABUF		wsaBuf;
	IOOperation ioOperation;
	char buf[SOCKBUFFERSIZE];
};

class ClientInfo
{
public:
	uint32_t index = 0;
	SOCKET clientSocket = INVALID_SOCKET;
	ExOverlapped recvOverlapped;
	ExOverlapped sendOverlapped;
	std::string recvBufString;
	std::queue<std::string> sendQueue;

	ClientInfo(const uint32_t idx)
	{
		ZeroMemory(&recvOverlapped, sizeof(recvOverlapped));
		ZeroMemory(&sendOverlapped, sizeof(sendOverlapped));
		index = idx;
	}
};