#pragma once

#include <vector>
#include <map>
#include <string>
#include "User.hpp"
#include "Room.h"
#include "ErrorCode.hpp"



class RoomManager
{
private:
	std::vector<Room> m_roomPool;
	std::map<uint16_t, uint16_t> m_userRoomMap;
public:
	
	// ������ ����ִ� ���� ã�� ���
	std::pair<ERROR_CODE, Room*> findRoomUserIn(uint16_t userIndex);
	// �濡 Ư�� ������ �ִ� ���.
	std::pair<ERROR_CODE, Room*> addUserInRoom(uint16_t userIndex, uint16_t roomIndex);
	// ������ ���� ����� ������ �濡�� ���� ���
	std::pair<ERROR_CODE, Room*> leaveUserInRoom(uint16_t userIndex);
	// �� �ε����� ���� ��ȯ�ϴ� ���.
	std::pair<ERROR_CODE, Room*> getRoom(uint16_t roomIndex);
	// ����� �޴� �Լ�
	std::pair<ERROR_CODE, Room*> getEmptyRoom(uint16_t roomIndex);

};

