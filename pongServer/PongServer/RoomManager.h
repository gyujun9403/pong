#pragma once

#include <vector>
#include <map>
#include <string>
#include <cstdint>
#include "User.hpp"
#include "Room.h"
#include "ErrorCode.hpp"



class RoomManager
{
private:
	const uint16_t m_maxRoomNum;
	std::vector<Room> m_roomPool;
	std::map<uint16_t, uint16_t> m_userRoomMap;

public:
	RoomManager(uint16_t maxRoomNum);
	std::pair<ERROR_CODE, Room*> findRoomUserIn(uint16_t userIndex);
	std::pair<ERROR_CODE, Room*> addUserInRoom(uint16_t userIndex, uint16_t roomIndex);
	std::pair<ERROR_CODE, Room*> leaveUserInRoom(uint16_t userIndex);
	std::pair<ERROR_CODE, Room*> getRoom(uint16_t roomIndex);
	std::pair<ERROR_CODE, Room*> getEmptyRoom();
};

