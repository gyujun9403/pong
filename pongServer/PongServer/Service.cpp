#include "Service.hpp"
#include "PacketsDefine.hpp"
#include "ErrorCode.hpp"

Service::Service(IocpNetworkCore* network, UserManager* userManager, RoomManager* roomManager, RedisMatching* matchingManager)
:m_network(network), m_userManager(userManager), m_isServiceRun(true), m_roomManager(roomManager), m_matchingManager(matchingManager)
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
	m_network->pushToSendQueue(clinetIndex, res);
}

int Service::packetProcessLoginRequest(int clinetIndex, std::vector<char> ReqPacket)
{
	LOGIN_REQUEST_PACKET loginReq;
	LOGIN_RESPONSE_PACKET loginRes;
	std::move(ReqPacket.begin(), ReqPacket.end(), (char*)&loginReq);
	loginRes.PacketId = PACKET_ID::LOGIN_RESPONSE;
	loginRes.PacketLength = sizeof(LOGIN_RESPONSE_PACKET);
	std::pair<ERROR_CODE, User*> rt = m_userManager->getUser(clinetIndex);
	if (rt.first == ERROR_CODE::NONE)
	{
		loginRes.Result = ERROR_CODE::LOGIN_USER_ALREADY;	
	}
	else
	{
		ERROR_CODE idCheckRt = m_userManager->checkId(loginReq.UserID);
		if (idCheckRt != ERROR_CODE::NONE)
		{
			loginRes.Result = idCheckRt;
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
	}
	pushPacketToSendQueue(clinetIndex, reinterpret_cast<char*>(&loginRes), sizeof(LOGIN_RESPONSE_PACKET));
	return 1;
}

// �� ��Ŷ���� ó���� �Լ��� ���⿡ �ֱ�.

ROOM_USER_LIST_NOTIFY_PACKET Service::makeUserListPacket(std::vector<uint16_t> userInRoomList)
{
	ROOM_USER_LIST_NOTIFY_PACKET rt;
	rt.PacketId = PACKET_ID::ROOM_USER_LIST_NTF;
	rt.PacketLength = sizeof(rt);
	rt.UserCount = 0;
	for (uint16_t elem : userInRoomList)
	{
		if (rt.UserCount == MAX_USER_CNT_IN_ROOM)
		{
			break;
		}
		std::string userId = m_userManager->getUser(elem).second->getUserId();
		rt.listArr[rt.UserCount].userClinetNum = elem;
		rt.listArr[rt.UserCount].userIdLen = MAX_USER_ID_LEN + 1;
		// ???? ������ �� �� �߻� ����....
		std::copy(userId.begin(), userId.end(), rt.listArr[rt.UserCount].id);
		++rt.UserCount;
	}
	return std::move(rt);
}

int Service::packetProcessRoomEnterRequest(int clinetIndex, std::vector<char> ReqPacket)
{
	ROOM_ENTER_REQUEST_PACKET roomEnterReq;
	ROOM_ENTER_RESPONSE_PACKET roomEnterRes;
	roomEnterRes.PacketId = PACKET_ID::ROOM_ENTER_RESPONSE;
	roomEnterRes.PacketLength = sizeof(ROOM_ENTER_RESPONSE_PACKET);
	std::pair<ERROR_CODE, User*> rtUser = m_userManager->getUser(clinetIndex);
	if (rtUser.first != ERROR_CODE::NONE)
	{
		roomEnterRes.Result = rtUser.first;
		pushPacketToSendQueue(clinetIndex, reinterpret_cast<char*>(&roomEnterRes), sizeof(ROOM_ENTER_RESPONSE_PACKET));
		return -1;
	}
	std::move(ReqPacket.begin(), ReqPacket.end(), (char*)&roomEnterReq);
	std::pair<ERROR_CODE, Room*> rtRoom = m_roomManager->addUserInRoom(clinetIndex, roomEnterReq.RoomNumber);
	if (rtRoom.first != ERROR_CODE::NONE)
	{
		roomEnterRes.Result = rtRoom.first;
		pushPacketToSendQueue(clinetIndex, reinterpret_cast<char*>(&roomEnterRes), sizeof(ROOM_ENTER_RESPONSE_PACKET));
		return -1;
	}
	roomEnterRes.Result = rtRoom.first;
	pushPacketToSendQueue(clinetIndex, reinterpret_cast<char*>(&roomEnterRes), sizeof(ROOM_ENTER_RESPONSE_PACKET));
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
	ROOM_LEAVE_REQUEST_PACKET roomLeaveReq;
	ROOM_LEAVE_RESPONSE_PACKET roomLeaveRes;
	roomLeaveRes.PacketId = PACKET_ID::ROOM_LEAVE_RESPONSE;
	roomLeaveRes.PacketLength = sizeof(ROOM_LEAVE_RESPONSE_PACKET);
	std::pair<ERROR_CODE, User*> rtUser = m_userManager->getUser(clinetIndex);
	if (rtUser.first != ERROR_CODE::NONE)
	{
		roomLeaveRes.Result = rtUser.first;
		pushPacketToSendQueue(clinetIndex, reinterpret_cast<char*>(&roomLeaveRes), sizeof(ROOM_LEAVE_RESPONSE_PACKET));
		return -1;
	}
	std::pair<ERROR_CODE, Room*> rtRoom = m_roomManager->leaveUserInRoom(clinetIndex);
	if (rtRoom.first != ERROR_CODE::NONE)
	{
		roomLeaveRes.Result = rtRoom.first;
		pushPacketToSendQueue(clinetIndex, reinterpret_cast<char*>(&roomLeaveRes), sizeof(ROOM_LEAVE_RESPONSE_PACKET));
		return -1;
	}
	roomLeaveRes.Result = rtRoom.first;
	pushPacketToSendQueue(clinetIndex, reinterpret_cast<char*>(&roomLeaveRes), sizeof(ROOM_LEAVE_RESPONSE_PACKET));
	std::vector<uint16_t> allUsersInRoom = rtRoom.second->getAllUsers();
	ROOM_USER_LIST_NOTIFY_PACKET userListNtfPacket = makeUserListPacket(allUsersInRoom);
	for (uint16_t clinetIndexElem : allUsersInRoom)
	{
		pushPacketToSendQueue(clinetIndexElem, reinterpret_cast<char*>(&userListNtfPacket), sizeof(ROOM_USER_LIST_NOTIFY_PACKET));
	}
	return 0;
}

int Service::packetProcessRoomChatRequest(int clinetIndex, std::vector<char> ReqPacket)
{
	ROOM_CHAT_REQUEST_PACKET roomChatReq;
	ROOM_CHAT_RESPONSE_PACKET roomChatRes;
	roomChatRes.PacketId = PACKET_ID::ROOM_CHAT_RESPONSE;
	roomChatRes.PacketLength = sizeof(ROOM_CHAT_RESPONSE_PACKET);
	std::pair<ERROR_CODE, User*> rtUser = m_userManager->getUser(clinetIndex);
	if (rtUser.first != ERROR_CODE::NONE)
	{
		roomChatRes.Result = rtUser.first;
		pushPacketToSendQueue(clinetIndex, reinterpret_cast<char*>(&roomChatRes), sizeof(ROOM_CHAT_RESPONSE_PACKET));
		return -1;
	}
	std::pair<ERROR_CODE, Room*> rtRoom = m_roomManager->findRoomUserIn(clinetIndex);
	if (rtRoom.first != ERROR_CODE::NONE)
	{
		roomChatRes.Result = rtRoom.first;
		pushPacketToSendQueue(clinetIndex, reinterpret_cast<char*>(&roomChatRes), sizeof(ROOM_CHAT_RESPONSE_PACKET));
		return -1;
	}
	roomChatRes.Result = rtRoom.first;
	pushPacketToSendQueue(clinetIndex, reinterpret_cast<char*>(&roomChatRes), sizeof(ROOM_CHAT_RESPONSE_PACKET));
	std::vector<uint16_t> allUsersInRoom = rtRoom.second->getAllUsers();
	ROOM_CHAT_NOTIFY_PACKET rootChatNtf;
	rootChatNtf.PacketId = PACKET_ID::ROOM_CHAT_NOTIFY;
	rootChatNtf.PacketLength = sizeof(ROOM_CHAT_NOTIFY_PACKET);
	std::string userId = rtUser.second->getUserId();
	std::copy(userId.begin(), userId.end(), rootChatNtf.UserID);
	std::move(ReqPacket.begin(), ReqPacket.end(), (char*)&roomChatReq);
	std::move(roomChatReq.Message, roomChatReq.Message + (roomChatReq.PacketLength - sizeof(PacketHeader)), rootChatNtf.Msg);
	for (uint16_t clinetIndexElem : allUsersInRoom)
	{
		pushPacketToSendQueue(clinetIndexElem, reinterpret_cast<char*>(&rootChatNtf), sizeof(ROOM_CHAT_NOTIFY_PACKET));
	}
	return 0;
}

int Service::packetProcessRoomReadyRequest(int clinetIndex, std::vector<char> ReqPacket)
{
	ROOM_READY_REQUEST_PACKET roomReadyReq;
	ROOM_READY_RESPONSE_PACKET roomReadyRes;
	ROOM_READY_NOTIFY_PACKET roomReadyNtf;

	// ������ ���ų� �濡�� ������ �� false�� ����.
	roomReadyRes.isReady = false; 
	std::pair<ERROR_CODE, User*> rtUser = m_userManager->getUser(clinetIndex);
	if (rtUser.first != ERROR_CODE::NONE)
	{
		roomReadyRes.Result = rtUser.first;
		pushPacketToSendQueue(clinetIndex, reinterpret_cast<char*>(&roomReadyRes), sizeof(ROOM_READY_RESPONSE_PACKET));
		return -1;
	}
	std::pair<ERROR_CODE, Room*> rtRoom = m_roomManager->findRoomUserIn(clinetIndex);
	if (rtRoom.first != ERROR_CODE::NONE)
	{
		roomReadyRes.Result = rtRoom.first;
		pushPacketToSendQueue(clinetIndex, reinterpret_cast<char*>(&roomReadyRes), sizeof(ROOM_READY_RESPONSE_PACKET));
		return -1;
	}
	std::move(ReqPacket.begin(), ReqPacket.end(), (char*)&roomReadyReq);
	roomReadyRes.PacketId = PACKET_ID::ROOM_READY_RES;
	roomReadyRes.PacketLength = sizeof(ROOM_READY_RESPONSE_PACKET);
	roomReadyRes.isReady = rtRoom.second->setUserReady(clinetIndex, roomReadyReq.isReady);
	roomReadyRes.Result = ERROR_CODE::NONE;
	roomReadyNtf.isReady = roomReadyRes.isReady;
	roomReadyNtf.PacketId = PACKET_ID::ROOM_READY_NOTIFY;
	roomReadyNtf.PacketLength = sizeof(ROOM_READY_NOTIFY_PACKET);
	roomReadyNtf.UserUniqueId = clinetIndex;
	pushPacketToSendQueue(clinetIndex, reinterpret_cast<char*>(&roomReadyRes), sizeof(ROOM_READY_RESPONSE_PACKET));
	std::vector<uint16_t> allUsersInRoom = rtRoom.second->getAllUsers();
	for (uint16_t clinetIndexElem : allUsersInRoom)
	{
		pushPacketToSendQueue(clinetIndexElem, reinterpret_cast<char*>(&roomReadyNtf), sizeof(ROOM_READY_NOTIFY_PACKET));
	}
	if (rtRoom.second->isAllUserReady())
	{
		std::string userList;

		uint16_t i = 0;
		for (uint16_t elem : allUsersInRoom)
		{
			userList += std::to_string(elem) + "-";
		}
		m_matchingManager->pushToMatchQueue(userList);
		//rtRoom.second->clearAllUserReady();
	}
	return 1;
}


void Service::serviceInit()
{
	m_packetProcessMap.emplace(PACKET_ID::LOGIN_REQUEST, std::bind(&Service::packetProcessLoginRequest, this, std::placeholders::_1, std::placeholders::_2));
	m_packetProcessMap.emplace(PACKET_ID::ROOM_ENTER_REQUEST, std::bind(&Service::packetProcessRoomEnterRequest, this, std::placeholders::_1, std::placeholders::_2));
	m_packetProcessMap.emplace(PACKET_ID::ROOM_LEAVE_REQUEST, std::bind(&Service::packetProcessRoomLeaveRequest, this, std::placeholders::_1, std::placeholders::_2));
	m_packetProcessMap.emplace(PACKET_ID::ROOM_CHAT_REQUEST, std::bind(&Service::packetProcessRoomChatRequest, this, std::placeholders::_1, std::placeholders::_2));
	m_packetProcessMap.emplace(PACKET_ID::ROOM_READY_REQ, std::bind(&Service::packetProcessRoomReadyRequest, this, std::placeholders::_1, std::placeholders::_2));
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
			divergePackets(recvRt);
		}
		closeUserIndex = m_network->getCloseUser();
		if (closeUserIndex != -1)
		{
			m_roomManager->leaveUserInRoom(closeUserIndex);
			m_userManager->deleteUser(closeUserIndex);
		}
		redisProcessMatchingResultQueue();
		redisProcessGameResultQueue();
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

void Service::redisProcessMatchingResultQueue()
{
	std::vector<std::string> rt = std::move(m_matchingManager->getFormMatchResultQueue());
	std::vector<uint16_t> matchedUserList;
	for (std::string& elem : rt)
	{
		std::stringstream ss(elem);
		std::string token;

		while (std::getline(ss, token, '-')) {
			matchedUserList.push_back(std::stoi(token));
		}
		if (matchedUserList.size() < 2)
		{
			return;
		}
		// ���ݵ� �� �濡 �ְ� ���� �������� Ȯ��.
		std::pair<ERROR_CODE, Room*> before;
		std::pair<ERROR_CODE, Room*> now;
		before = m_roomManager->findRoomUserIn(matchedUserList[0]);
		for (uint16_t clinetIndexElem : matchedUserList)
		{
			now = m_roomManager->findRoomUserIn(clinetIndexElem);
			if (now.first != ERROR_CODE::NONE || before != now)
			{
				return;
			}
			before = now;
		}
		before = m_roomManager->findRoomUserIn(matchedUserList[0]);
		if (before.second->isAllUserReady() == false)
		{
			return;
		}
		before.second->clearAllUserReady();
		for (uint16_t clinetIndexElem : matchedUserList)
		{
			GAME_START_NOTIFY_PACKET gameStartNtf;
			gameStartNtf.PacketId = PACKET_ID::GAME_START_NOTIFY;
			gameStartNtf.PacketLength = sizeof(GAME_START_NOTIFY_PACKET);
			gameStartNtf.key = clinetIndexElem;
			pushPacketToSendQueue(clinetIndexElem, reinterpret_cast<char*>(&gameStartNtf), sizeof(GAME_START_NOTIFY_PACKET));
		}
	}
}

void Service::redisProcessGameResultQueue()
{
	// TODO
}
