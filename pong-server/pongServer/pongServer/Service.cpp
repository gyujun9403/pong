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

int Service::packetProcessLoginRequest(int clinetIndex, std::vector<char> ResPacket)
{
	LOGIN_REQUEST_PACKET loginReq;
	LOGIN_RESPONSE_PACKET loginRes;
	std::move(ResPacket.begin(), ResPacket.end(), (char*)&loginReq);
	std::cout << loginReq.UserID << ", " << loginReq.UserPW << std::endl;
	// ���� id�� ������ �ִ��� Ȯ��
	loginRes.PacketId = PACKET_ID::LOGIN_RESPONSE;
	loginRes.PacketLength = sizeof(LOGIN_RESPONSE_PACKET);
	User* userRt = m_userManager->getUser(clinetIndex);
	if (userRt != NULL)
	{
		//loginRes.Result = ERROR_CODE::LOGIN_USER_ALREADY;
		if (userRt->checkPassword(loginReq.UserPW))
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
		userRt = m_userManager->setUser(clinetIndex, loginReq.UserID, loginReq.UserPW);
		if (userRt == NULL)
		{
			loginRes.Result = ERROR_CODE::LOGIN_USER_USED_ALL_OBJ;
		}
		else
		{
			loginRes.Result = ERROR_CODE::NONE;
		}
	}

	pushPacketToSendQueue(clinetIndex, reinterpret_cast<char*>(&loginRes), sizeof(LOGIN_RESPONSE_PACKET));
	return 1;
}

// �� ��Ŷ���� ó���� �Լ��� ���⿡ �ֱ�.

void Service::serviceInit()
{
	m_packetProcessMap.emplace(PACKET_ID::LOGIN_REQUEST, std::bind(&Service::packetProcessLoginRequest, this, std::placeholders::_1, std::placeholders::_2));
	// ���ο� ��Ŷ�� ó�� �Լ��� �߰� �� ������ emplace���� �߰����ָ� ��.
}

void Service::serviceThread()
{
	std::pair<int, std::vector<char> > recvRt;
	while (m_isServiceRun.load())
	{
		recvRt = m_network->getFromRecvQueue();
		if (recvRt.first != -1)
		{
			divergePackets(std::move(recvRt));
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
