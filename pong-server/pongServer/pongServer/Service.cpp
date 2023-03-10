#include "Service.hpp"
#include "PacketsDefine.hpp"
#include "ErrorCode.hpp"

Service::Service(IocpServer* network, UserManager* userManager)
:m_network(network), m_userManager(userManager), m_isServiceRun(true)
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

int Service::packetProcessRoomEnterRequest(int clinetIndex, std::vector<char> ReqPacket)
{
	// 방 입장 가능 확인
	// 입장한 사람은 ROOM_ENTER_RESPONSE_PACKET과
	// 방에 있던 사람은 ROOM_NEW_USER_NOTIFY_PACKET과 
	// 이후 방에 전원 모두 ROOM_USER_LIST_NOTIFY_PACKET를 받음
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
			m_roomManager.
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
