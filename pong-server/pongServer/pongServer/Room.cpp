#include "Room.h"


Room::Room(IocpServer* network)
: m_network(network)
{
}

ERROR_CODE Room::sendChatInRoom(uint16_t clientIndex, std::string message)
{
	// ����ڿ��� ���� req�� ������
	
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
