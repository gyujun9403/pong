#pragma once

#include <vector>
#include <map>
#include <string>
#include "User.hpp"
#include "Room.h"
#include "ErrorCode.hpp"

enum class RoomStatus : uint16_t
{
	EMPTY, ONE, TWO, GAMING
};

class RoomManager
{
private:
	std::vector<Room> m_roomPool;
	std::map<uint16_t, uint16_t> m_userRoomMap;
public:
	
	// 유저가 들어있는 방을 찾는 기능
	std::pair<ERROR_CODE, Room*> findRoomUserIn(uint16_t userIndex);
	// 방에 특정 유저를 넣는 기능.
	std::pair<ERROR_CODE, Room*> addUserInRoom(uint16_t userIndex, uint16_t roomIndex);
	// 방 인덱스로 방을 반환하는 기능.
	std::pair<ERROR_CODE, Room*> getRoom(uint16_t roomIndex);
	// 방 상태를 확인
	RoomStatus getRoomStatus(uint16_t roomIndex);
	// 빈방을 받는 함수
	std::pair<ERROR_CODE, Room*> getEmptyRoom(uint16_t roomIndex);

};

