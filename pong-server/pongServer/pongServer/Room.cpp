#include "Room.h"
#include "PacketsDefine.hpp"

//Room::Room(IocpServer* network)
//: m_network(network)
//{
//}

Room::Room()
{
	m_usersIndex.reserve(2);
}

ERROR_CODE Room::sendChatInRoom(uint16_t clientIndex, std::string message)
{
	// 대상자에게 성공 req를 보내고
	return ERROR_CODE();
	// 
}

ERROR_CODE Room::enterUser(uint16_t clientIndex)
{
	if (m_usersIndex.size() >= 2)
	{
		return ERROR_CODE::ENTER_ROOM_FULL_USER;
	}
	else if (std::find(m_usersIndex.begin(), m_usersIndex.end(), clientIndex) != m_usersIndex.end())
	{
		return ERROR_CODE::ENTER_ROOM_INVALID_USER_STATUS;
	}
	m_usersIndex.push_back(clientIndex);
	return ERROR_CODE::NONE;
}

ERROR_CODE Room::leaveUser(uint16_t clientIndex)
{
	std::vector<uint16_t>::iterator iter = std::find(m_usersIndex.begin(), m_usersIndex.end(), clientIndex);
	if (iter == m_usersIndex.end())
	{
		return ERROR_CODE::ROOM_USER_CANT_FIND;
	}
	m_usersIndex.erase(iter);
	return ERROR_CODE::NONE;
}

RoomStatus Room::getRoomStatus()
{
	return m_roomStatus;
}

std::vector<uint16_t> Room::getAllUsers()
{
	return m_usersIndex;
}

bool Room::isRoomEnable()
{
	if (m_usersIndex.size() < 2)
	{
		return true;
	}
	return false;
}
