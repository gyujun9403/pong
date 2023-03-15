#pragma once

#include "User.hpp"
//#include "IOCPServer.h"
#include "ErrorCode.hpp"
#include <iterator>

#define NO_USER -1

enum class RoomStatus : uint16_t
{
	EMPTY, ONE, TWO, GAMING
};

class Room
{
private:
	//std::vector<uint16_t> m_usersIndex; // 일단 2명까지
	std::vector<std::pair<uint16_t, bool> > m_usersIndex;
	RoomStatus m_roomStatus = RoomStatus::EMPTY;
	//std::vector<bool> m_usersIndex;

	// 게임 클래스 추가해야함.
public:
	Room();
	ERROR_CODE sendChatInRoom(uint16_t clientIndex, std::string message);
	ERROR_CODE enterUser(uint16_t clientIndex);
	ERROR_CODE leaveUser(uint16_t clientIndex);
	bool setUserReady(uint16_t clientIndex, bool ready);
	bool isAllUserReady();
	RoomStatus getRoomStatus();
	void setRoomStatus(RoomStatus status);
	std::vector<uint16_t> getAllUsers();
	bool isRoomEnable();
};

