#pragma once

#include "User.hpp"
//#include "IOCPServer.h"
#include "ErrorCode.hpp"

#define NO_USER -1

enum class RoomStatus : uint16_t
{
	EMPTY, ONE, TWO, GAMING
};

class Room
{
private:
	std::vector<uint16_t> m_usersIndex; // �ϴ� 2�����
	//int32_t playerIndex[2];
	RoomStatus m_roomStatus = RoomStatus::EMPTY;
	//IocpServer* m_network;

	// ���� Ŭ���� �߰��ؾ���.
public:
	Room();
	ERROR_CODE sendChatInRoom(uint16_t clientIndex, std::string message);
	ERROR_CODE enterUser(uint16_t clientIndex);
	ERROR_CODE leaveUser(uint16_t clientIndex);
	RoomStatus getRoomStatus();
	std::vector<uint16_t> getAllUsers();
	bool isRoomEnable();
};

