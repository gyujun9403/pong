#include "Room.h"
#include "PacketsDefine.hpp"

Room::Room(IocpServer* network)
: m_network(network)
{
}

ERROR_CODE Room::sendChatInRoom(uint16_t clientIndex, std::string message)
{
	// 대상자에게 성공 req를 보내고
	return ERROR_CODE();
	// 
}

ERROR_CODE Room::enterUser(uint16_t clientIndex)
{
	if (m_usersIndex[0] == clientIndex || m_usersIndex[1] == clientIndex)
	{
		return ERROR_CODE::ENTER_ROOM_INVALID_USER_STATUS;
	}
	else if (m_usersIndex[0] == NO_USER)
	{
		m_usersIndex[0] = clientIndex;
	}
	else if(m_usersIndex[1] == NO_USER)
	{
		m_usersIndex[1] = clientIndex;
	}
	else
	{
		return ERROR_CODE::ENTER_ROOM_FULL_USER;
	}
	return ERROR_CODE::NONE;
}

ERROR_CODE Room::leaveUser(uint16_t clientIndex)
{
	if (m_usersIndex[0] == clientIndex)
	{
		m_usersIndex[0] = NO_USER;
	}
	else if (m_usersIndex[1] == clientIndex)
	{
		m_usersIndex[1] = NO_USER;
	}
	else
	{
		return ERROR_CODE::ROOM_USER_CANT_FIND;
	}
}

RoomStatus Room::getRoomStatus()
{
	return m_roomStatus;
}

bool Room::isRoomEnable()
{
	if (m_usersIndex[0] == NO_USER || m_usersIndex[1] == NO_USER)
	{
		return true;
	}
}
