#include "Room.h"
#include "PacketsDefine.hpp"

Room::Room()
{
	m_usersIndex.reserve(2);
}

ERROR_CODE Room::enterUser(uint16_t clientIndex)
{
	if (m_usersIndex.size() >= 2)
	{
		return ERROR_CODE::ENTER_ROOM_FULL_USER;
	}
	auto iter = std::find_if(m_usersIndex.begin(), m_usersIndex.end(),
		[clientIndex](const std::pair<uint16_t, bool>& p)
		{
			return p.first == clientIndex;
		}
	);
	if (iter != m_usersIndex.end())
	{
		return ERROR_CODE::ENTER_ROOM_INVALID_USER_STATUS;
	}
	m_usersIndex.push_back(std::move(std::make_pair(clientIndex, false)));
	return ERROR_CODE::NONE;
}

ERROR_CODE Room::leaveUser(uint16_t clientIndex)
{
	auto iter = std::find_if (m_usersIndex.begin(), m_usersIndex.end(),
		[clientIndex](const std::pair<uint16_t, bool>& p)
		{
			return p.first == clientIndex;
		}
	);
	if (iter == m_usersIndex.end())
	{
		return ERROR_CODE::ROOM_USER_CANT_FIND;
	}
	m_usersIndex.erase(iter);
	return ERROR_CODE::NONE;
}

bool Room::setUserReady(uint16_t clientIndex, bool ready)
{
	auto iter = std::find_if(m_usersIndex.begin(), m_usersIndex.end(),
		[clientIndex](const std::pair<uint16_t, bool>& p)
		{
			return p.first == clientIndex;
		}
	);
	if (iter == m_usersIndex.end())
	{
		return false;
	}
	iter->second = ready;
	return ready; 
}

bool Room::isAllUserReady()
{
	for (std::pair<uint16_t, bool>& elem : m_usersIndex)
	{
		if (elem.second == false)
		{
			return false;
		}
	}
	return true;
}

void Room::clearAllUserReady()
{
	for (std::pair<uint16_t, bool>& elemUser : m_usersIndex)
	{
		elemUser.second = false;
	}
}

RoomStatus Room::getRoomStatus()
{
	return m_roomStatus;
}

void Room::setRoomStatus(RoomStatus status)
{
	m_roomStatus = status;
}

std::vector<uint16_t> Room::getAllUsers()
{
	std::vector<uint16_t> rtVec;
	std::transform(m_usersIndex.begin(), m_usersIndex.end(), std::back_inserter(rtVec),
		[](const std::pair<uint16_t, bool>& p)
		{
			return p.first;
		}
	);
	return rtVec;
}

bool Room::isRoomEnable()
{
	if (m_usersIndex.size() < 2)
	{
		return true;
	}
	return false;
}
