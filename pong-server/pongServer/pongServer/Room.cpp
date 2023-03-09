#include "Room.h"


Room::Room(IocpServer* network)
: m_network(network)
{
}

ERROR_CODE Room::sendChatInRoom(uint16_t clientIndex, std::string message)
{
	// 대상자에게 성공 req를 보내고
	
	// 
}

ERROR_CODE Room::enterUser(uint16_t clientIndex)
{
	return ERROR_CODE();
}

ERROR_CODE Room::leaveUser(uint16_t clientIndex)
{
	return ERROR_CODE();
}
