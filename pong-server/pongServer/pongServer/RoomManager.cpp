#include "RoomManager.h"

std::pair<ERROR_CODE, Room*> RoomManager::findRoomUserIn(uint16_t userIndex)
{
    return std::pair<ERROR_CODE, Room*>();
}

std::pair<ERROR_CODE, Room*> RoomManager::addUserInRoom(uint16_t userIndex, uint16_t roomIndex)
{
    // 유저가 맵에 있는지확인
    if (m_userRoomMap.find(userIndex) != m_userRoomMap.end())
    {
    }
    else
    {
        return std::move(make_pair<ERROR_CODE, Room*>(ROOM_))
    }
}

std::pair<ERROR_CODE, Room*> RoomManager::getRoom(uint16_t roomIndex)
{
    return std::pair<ERROR_CODE, Room*>();
}

RoomStatus RoomManager::getRoomStatus(uint16_t roomIndex)
{
    return RoomStatus();
}

std::pair<ERROR_CODE, Room*> RoomManager::getEmptyRoom(uint16_t roomIndex)
{
    return std::pair<ERROR_CODE, Room*>();
}
