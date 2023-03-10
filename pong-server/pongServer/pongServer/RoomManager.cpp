#include "RoomManager.h"

std::pair<ERROR_CODE, Room*> RoomManager::findRoomUserIn(uint16_t userIndex)
{
    if (m_userRoomMap.find(userIndex) != m_userRoomMap.end())
    {
        return std::move(std::make_pair<ERROR_CODE, Room*>(ERROR_CODE::NONE, &m_roomPool[m_userRoomMap[userIndex]]));
    }
    else
    {
        return std::move(std::make_pair<ERROR_CODE, Room*>(ERROR_CODE::ROOM_USER_CANT_FIND, NULL));
    }
}

std::pair<ERROR_CODE, Room*> RoomManager::addUserInRoom(uint16_t userIndex, uint16_t roomIndex)
{
    // 유저가 맵에 있는지확인
    if (m_userRoomMap.find(userIndex) != m_userRoomMap.end())
    {
        if (m_roomPool[roomIndex].isRoomEnable())
        {
            m_roomPool[roomIndex].enterUser(userIndex);
            return std::move(std::make_pair<ERROR_CODE, Room*>(ERROR_CODE::NONE, NULL));
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
    if (m_userRoomMap.find(userIndex) != m_userRoomMap.end())
    {
        //ERROR_CODE errorCode = m_roomPool[m_userRoomMap[userIndex]].leaveUser(userIndex);
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
    return std::pair<ERROR_CODE, Room*>();
}
