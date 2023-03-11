#include "Service.hpp"
#include "PacketsDefine.hpp"
#include "ErrorCode.hpp"

Service::Service(IocpServer* network, UserManager* userManager, RoomManager* roomManager)
:m_network(network), m_userManager(userManager), m_isServiceRun(true), m_roomManager(roomManager)
{
}

int Service::divergePackets(std::pair<int, std::vector<char> > packetSet)
{
	PACKET_ID* packetId = reinterpret_cast<PACKET_ID*>(&packetSet.second[0] + sizeof(PacketHeader::PacketLength));

	return m_packetProcessMap[*packetId](packetSet.first, packetSet.second);
}

void Service::pushPacketToSendQueue(int clinetIndex, char* packet, size_t length)
{
	std::vector<char> res(packet, packet + length);
	res.resize(length);
	m_network->pushToSendQueue(clinetIndex, std::move(res));
}

int Service::packetProcessLoginRequest(int clinetIndex, std::vector<char> ReqPacket)
{
	LOGIN_REQUEST_PACKET loginReq;
	LOGIN_RESPONSE_PACKET loginRes;
	std::move(ReqPacket.begin(), ReqPacket.end(), (char*)&loginReq);
	std::cout << loginReq.UserID << ", " << loginReq.UserPW << std::endl;
	// ���� id�� ������ �ִ��� Ȯ��
	loginRes.PacketId = PACKET_ID::LOGIN_RESPONSE;
	loginRes.PacketLength = sizeof(LOGIN_RESPONSE_PACKET);
	std::pair<ERROR_CODE, User*> rt = m_userManager->getUser(clinetIndex);
	if (rt.first == ERROR_CODE::NONE)
	{
		User* rtUser = rt.second;
		//loginRes.Result = ERROR_CODE::LOGIN_USER_ALREADY;
		if (rtUser->checkPassword(loginReq.UserPW))
		{
			// ��й�ȣ�� ���� -> �̹� ������
			loginRes.Result = ERROR_CODE::LOGIN_USER_ALREADY;
		}
		else
		{
			loginRes.Result = ERROR_CODE::LOGIN_USER_INVALID_PW;
		}
	}
	else
	{
		rt = m_userManager->setUser(clinetIndex, loginReq.UserID, loginReq.UserPW);
		if (rt.first == ERROR_CODE::NONE)
		{
			loginRes.Result = ERROR_CODE::NONE;
		}
		else
		{
			loginRes.Result = ERROR_CODE::LOGIN_USER_USED_ALL_OBJ;
		}
	}
	pushPacketToSendQueue(clinetIndex, reinterpret_cast<char*>(&loginRes), sizeof(LOGIN_RESPONSE_PACKET));
	return 1;
}


// �� ��Ŷ���� ó���� �Լ��� ���⿡ �ֱ�.

// TODOTODOTODOTOTODTODOTODTO
ROOM_USER_LIST_NOTIFY_PACKET Service::makeUserListPacket(std::vector<uint16_t> userInRoomList)
{
	ROOM_USER_LIST_NOTIFY_PACKET rt;
	rt.PacketId = PACKET_ID::ROOM_USER_LIST_NTF;
	rt.PacketLength = sizeof(rt);
	rt.UserCount = 0;
	//uint16_t i = 0;
	for (uint16_t elem : userInRoomList)
	{
		if (rt.UserCount == MAX_USER_CNT_IN_ROOM)
		{
			break;
		}
		//User* user = m_userManager->getUser(elem).second;
		std::string userId = m_userManager->getUser(elem).second->getUserId();
		rt.listArr[rt.UserCount].userClinetNum = elem;
		//rt.listArr[rt.UserCount].userIdLen = userId.size();
		rt.listArr[rt.UserCount].userIdLen = MAX_USER_ID_LEN + 1;
		// ???? ������ �� �� �߻� ����....
		std::copy(userId.begin(), userId.end(), rt.listArr[rt.UserCount].id);
		++rt.UserCount;
	}
	return std::move(rt);
}

int Service::packetProcessRoomEnterRequest(int clinetIndex, std::vector<char> ReqPacket)
{
	// �α��� Ȯ��
	ROOM_ENTER_REQUEST_PACKET roomEnterReq;
	ROOM_ENTER_RESPONSE_PACKET roomEnterRes;
	roomEnterRes.PacketId = PACKET_ID::ROOM_ENTER_RESPONSE;
	roomEnterRes.PacketLength = sizeof(ROOM_ENTER_RESPONSE_PACKET);
	std::pair<ERROR_CODE, User*> rtUser = m_userManager->getUser(clinetIndex);
	if (rtUser.first != ERROR_CODE::NONE)
	{
		roomEnterRes.Result = rtUser.first; // ��� �̰� ���������ε�? -> ��¥�� ���ñ����� �Ⱦ������� ����������.
		pushPacketToSendQueue(clinetIndex, reinterpret_cast<char*>(&roomEnterRes), sizeof(LOGIN_RESPONSE_PACKET));
		return -1;
	}
	std::move(ReqPacket.begin(), ReqPacket.end(), (char*)&roomEnterReq);
	// �� ���� ���� Ȯ��
	std::pair<ERROR_CODE, Room*> rtRoom = m_roomManager->addUserInRoom(clinetIndex, roomEnterReq.RoomNumber);
	if (rtRoom.first != ERROR_CODE::NONE)
	{
		roomEnterRes.Result = rtRoom.first;
		pushPacketToSendQueue(clinetIndex, reinterpret_cast<char*>(&roomEnterRes), sizeof(LOGIN_RESPONSE_PACKET));
		return -1;
	}
	roomEnterRes.Result = rtRoom.first;
	pushPacketToSendQueue(clinetIndex, reinterpret_cast<char*>(&roomEnterRes), sizeof(LOGIN_RESPONSE_PACKET));
	std::vector<uint16_t> allUsersInRoom = rtRoom.second->getAllUsers();
	ROOM_USER_LIST_NOTIFY_PACKET userListNtfPacket = makeUserListPacket(allUsersInRoom);
	for (uint16_t clinetIndexElem : allUsersInRoom)
	{
		pushPacketToSendQueue(clinetIndexElem, reinterpret_cast<char*>(&userListNtfPacket), sizeof(ROOM_USER_LIST_NOTIFY_PACKET));
	}
	return 0;
}

int Service::packetProcessRoomLeaveRequest(int clinetIndex, std::vector<char> ReqPacket)
{
	// �濡 �ִ��� Ȯ��
	// ������ ����� ROOM_LEAVE_RESPONSE_PACKET�� ����.
	// ������ �濡 �ִ� ����� ROOM_LEAVE_NORITY_PACKET�� ROOM_USER_LIST_NOTIFY_PACKET�� ����.
	return 0;
}

int Service::packetProcessRoomChatRequest(int clinetIndex, std::vector<char> ReqPacket)
{
	// ROOM_CHAT_RESPONSE_PACKET�� ����
	// �� �� ���� ���� ���� ROOM_CHAT_NOTIFY_PACKET�� ����.
	return 0;
}


void Service::serviceInit()
{
	m_packetProcessMap.emplace(PACKET_ID::LOGIN_REQUEST, std::bind(&Service::packetProcessLoginRequest, this, std::placeholders::_1, std::placeholders::_2));
	m_packetProcessMap.emplace(PACKET_ID::ROOM_ENTER_REQUEST, std::bind(&Service::packetProcessRoomEnterRequest, this, std::placeholders::_1, std::placeholders::_2));
	m_packetProcessMap.emplace(PACKET_ID::ROOM_LEAVE_REQUEST, std::bind(&Service::packetProcessRoomLeaveRequest, this, std::placeholders::_1, std::placeholders::_2));
	m_packetProcessMap.emplace(PACKET_ID::ROOM_CHAT_REQUEST, std::bind(&Service::packetProcessRoomChatRequest, this, std::placeholders::_1, std::placeholders::_2));
	// ���ο� ��Ŷ�� ó�� �Լ��� �߰� �� ������ emplace���� �߰����ָ� ��.
}

void Service::serviceThread()
{
	int32_t closeUserIndex;
	std::pair<int, std::vector<char> > recvRt;
	while (m_isServiceRun.load())
	{
		recvRt = m_network->getFromRecvQueue();
		if (recvRt.first != -1)
		{
			divergePackets(std::move(recvRt));
		}
		closeUserIndex = m_network->getCloseUser();
		if (closeUserIndex != -1)
		{
			//���� �濡�� ����
			//���� Ǯ���� ����
			std::cout << closeUserIndex << " user out" << std::endl;
			m_roomManager->leaveUserInRoom(closeUserIndex);
			m_userManager->deleteUser(closeUserIndex);
			//�º� ����
			//���� �� Ż�� �Լ� ȣ��
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
}

void Service::runService()
{
	m_serviceThread = std::thread
	(
		[this]()
		{
			serviceThread();
		}
	);
}

void Service::joinService()
{
	m_isServiceRun.store(false);
	m_serviceThread.join();
}
