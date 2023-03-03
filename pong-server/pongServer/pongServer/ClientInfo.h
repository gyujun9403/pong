#pragma once

#include <cstdint>
#include <winsock2.h>

const UINT32 SOCKBUFFERSIZE = 256;

enum class IOOperation { RECV, SEND };

//WSAOVERLAPPED구조체를 확장 시켜서 필요한 정보를 더 넣었다.
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

	ClientInfo(const uint32_t idx)
	{
		ZeroMemory(&recvOverlapped, sizeof(recvOverlapped));
		ZeroMemory(&sendOverlapped, sizeof(sendOverlapped));
		index = idx;
	}
};