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
	// 같은 id의 유저가 있는지 확인
	loginRes.PacketId = PACKET_ID::LOGIN_RESPONSE;
	loginRes.PacketLength = sizeof(LOGIN_RESPONSE_PACKET);
	std::pair<ERROR_CODE, User*> rt = m_userManager->getUser(clinetIndex);
	if (rt.first == ERROR_CODE::NONE)
	{
		User* rtUser = rt.second;
		//loginRes.Result = ERROR_CODE::LOGIN_USER_ALREADY;
		if (rtUser->checkPassword(loginReq.UserPW))
		{
			// 비밀번호가 같음 -> 이미 접속중
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


// 각 패킷들을 처리할 함수을 여기에 넣기.

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
		// ???? 지금은 왜 또 발생 안해....
		std::copy(userId.begin(), userId.end(), rt.listArr[rt.UserCount].id);
		++rt.UserCount;
	}
	return std::move(rt);
}

int Service::packetProcessRoomEnterRequest(int clinetIndex, std::vector<char> ReqPacket)
{
	// 로그인 확인
	ROOM_ENTER_REQUEST_PACKET roomEnterReq;
	ROOM_ENTER_RESPONSE_PACKET roomEnterRes;
	roomEnterRes.PacketId = PACKET_ID::ROOM_ENTER_RESPONSE;
	roomEnterRes.PacketLength = sizeof(ROOM_ENTER_RESPONSE_PACKET);
	std::pair<ERROR_CODE, User*> rtUser = m_userManager->getUser(clinetIndex);
	if (rtUser.first != ERROR_CODE::NONE)
	{
		roomEnterRes.Result = rtUser.first; // 잠깐 이거 지역변수인데? -> 어짜피 스택구조상 안없어지니 생관없을듯.
		pushPacketToSendQueue(clinetIndex, reinterpret_cast<char*>(&roomEnterRes), sizeof(LOGIN_RESPONSE_PACKET));
		return -1;
	}
	std::move(ReqPacket.begin(), ReqPacket.end(), (char*)&roomEnterReq);
	// 방 입장 가능 확인
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
	// 방에 있는지 확인
	// 나가는 사람은 ROOM_LEAVE_RESPONSE_PACKET를 받음.
	// 나가면 방에 있던 사람은 ROOM_LEAVE_NORITY_PACKET과 ROOM_USER_LIST_NOTIFY_PACKET를 받음.
	return 0;
}

int Service::packetProcessRoomChatRequest(int clinetIndex, std::vector<char> ReqPacket)
{
	// ROOM_CHAT_RESPONSE_PACKET을 받음
	// 이 후 방의 전원 보두 ROOM_CHAT_NOTIFY_PACKET을 받음.
	return 0;
}


void Service::serviceInit()
{
	m_packetProcessMap.emplace(PACKET_ID::LOGIN_REQUEST, std::bind(&Service::packetProcessLoginRequest, this, std::placeholders::_1, std::placeholders::_2));
	m_packetProcessMap.emplace(PACKET_ID::ROOM_ENTER_REQUEST, std::bind(&Service::packetProcessRoomEnterRequest, this, std::placeholders::_1, std::placeholders::_2));
	m_packetProcessMap.emplace(PACKET_ID::ROOM_LEAVE_REQUEST, std::bind(&Service::packetProcessRoomLeaveRequest, this, std::placeholders::_1, std::placeholders::_2));
	m_packetProcessMap.emplace(PACKET_ID::ROOM_CHAT_REQUEST, std::bind(&Service::packetProcessRoomChatRequest, this, std::placeholders::_1, std::placeholders::_2));
	// 새로운 패킷과 처리 함수를 추가 할 때마다 emplace으로 추가해주면 됨.
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
			//유저 방에서 제거
			//유저 풀에서 제거
			std::cout << closeUserIndex << " user out" << std::endl;
			m_roomManager->leaveUserInRoom(closeUserIndex);
			m_userManager->deleteUser(closeUserIndex);
			//승부 판정
			//유저 방 탈출 함수 호출
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
