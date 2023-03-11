#include "RoomManager.h"
#include <cstdint>

RoomManager::RoomManager(uint16_t maxRoomNum)
:m_maxRoomNum(maxRoomNum), m_userRoomMap()
{
    for (uint16_t i = 0; i < m_maxRoomNum; i++)
    {
        m_roomPool.emplace_back();
    }
}

std::pair<ERROR_CODE, Room*> RoomManager::findRoomUserIn(uint16_t userIndex)
{
    if (m_userRoomMap.empty() || m_userRoomMap.find(userIndex) == m_userRoomMap.end())
    {
        return std::move(std::make_pair<ERROR_CODE, Room*>(ERROR_CODE::ROOM_USER_CANT_FIND, NULL));
    }
    else
    {
        return std::move(std::make_pair<ERROR_CODE, Room*>(ERROR_CODE::NONE, &m_roomPool[m_userRoomMap[userIndex]]));
    }
}

std::pair<ERROR_CODE, Room*> RoomManager::addUserInRoom(uint16_t userIndex, uint16_t roomIndex)
{
    if (m_userRoomMap.empty() || m_userRoomMap.find(userIndex) == m_userRoomMap.end())
    {
        if (m_roomPool[roomIndex].isRoomEnable())
        {
            m_roomPool[roomIndex].enterUser(userIndex);
            m_userRoomMap.insert(std::make_pair<uint16_t, uint16_t>((uint16_t)userIndex, (uint16_t)roomIndex)); //???
            return std::move(std::make_pair<ERROR_CODE, Room*>(ERROR_CODE::NONE, &m_roomPool[roomIndex]));
        }
        else
        {
            return std::move(std::make_pair<ERROR_CODE, Room*>(ERROR_CODE::ENTER_ROOM_FULL_USER, NULL));
        }
    }
    else
    {
        return std::move(std::make_pair<ERROR_CODE, Room*>(ERROR_CODE::ENTER_ROOM_INVALID_USER_STATUS, NULL));
    }
}

std::pair<ERROR_CODE, Room*> RoomManager::leaveUserInRoom(uint16_t userIndex)
{
    if (m_userRoomMap.empty())
    {
        return std::move(std::make_pair<ERROR_CODE, Room*>(ERROR_CODE::ROOM_USER_CANT_FIND, NULL));
    }
    else if ((m_userRoomMap.find(userIndex) != m_userRoomMap.end()))
    {
        //ERROR_CODE errorCode = m_roomPool[m_userRoomMap[userIndex]].leaveUser(userIndex);
        m_userRoomMap.erase(userIndex);
        return std::move(std::make_pair<ERROR_CODE, Room*>(m_roomPool[m_userRoomMap[userIndex]].leaveUser(userIndex), NULL));
    }
    else
    {
        return std::move(std::make_pair<ERROR_CODE, Room*>(ERROR_CODE::ROOM_USER_CANT_FIND, NULL));
    }
}

std::pair<ERROR_CODE, Room*> RoomManager::getRoom(uint16_t roomIndex)
{
    if (roomIndex > m_roomPool.size())
    {
        return std::move(std::make_pair<ERROR_CODE, Room*>(ERROR_CODE::ROOM_INVALID_INDEX, NULL));
    }
    return std::move(std::make_pair<ERROR_CODE, Room*>(ERROR_CODE::NONE, &m_roomPool[roomIndex]));
}

//RoomStatus RoomManager::getRoomStatus(uint16_t roomIndex)
//{
//    if (roomIndex > m_roomPool.size())
//    {
//        return std::move(std::make_pair<ERROR_CODE, Room*>(ERROR_CODE::ROOM_INVALID_INDEX, NULL));
//    }
//    return m_roomPool[roomIndex].
//}

std::pair<ERROR_CODE, Room*> RoomManager::getEmptyRoom(uint16_t roomIndex)
{
    for (uint16_t i = 0; i < m_maxRoomNum; i++)
    {
        if (m_roomPool[i].isRoomEnable())
        {
            return std::pair<ERROR_CODE, Room*>(ERROR_CODE::NONE, &m_roomPool[i]);
        }
    }
    return std::pair<ERROR_CODE, Room*>(ERROR_CODE::NEW_ROOM_USED_ALL_OBJ, NULL);
}
