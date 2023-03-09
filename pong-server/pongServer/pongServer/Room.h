#pragma once

#include "User.hpp"
#include "IOCPServer.h"
#include "ErrorCode.hpp"

#define NO_USER -1

class Room
{
private:
	int16_t m_usersIndex[2] = { NO_USER };
	bool m_isGaming = false;
	IocpServer* m_network;

	// 게임 클래스 추가해야함.
public:
	Room(IocpServer* network);
	ERROR_CODE sendChatInRoom(uint16_t clientIndex, std::string message);
	ERROR_CODE enterUser(uint16_t clientIndex);
	ERROR_CODE leaveUser(uint16_t clientIndex);
};

