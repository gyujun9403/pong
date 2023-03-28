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
	std::vector<std::pair<uint16_t, bool> > m_usersIndex;
	RoomStatus m_roomStatus = RoomStatus::EMPTY;
public:
	Room();
	ERROR_CODE enterUser(uint16_t clientIndex);
	ERROR_CODE leaveUser(uint16_t clientIndex);
	bool setUserReady(uint16_t clientIndex, bool ready);
	bool isAllUserReady();
	void clearAllUserReady();
	RoomStatus getRoomStatus();
	void setRoomStatus(RoomStatus status);
	std::vector<uint16_t> getAllUsers();
	bool isRoomEnable();
};

