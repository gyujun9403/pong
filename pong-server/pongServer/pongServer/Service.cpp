#include "Service.hpp"
#include "PacketsDefine.hpp"
#include "ErrorCode.hpp"

Service::Service(IocpServer* network, UserManager* userManager)
:m_network(network), m_userManager(userManager), m_isServiceRun(true)
{
}

int Service::packetProcessLoginRequest(int clinetIndex, std::vector<char> packet)
{
	LOGIN_REQUEST_PACKET loginReq;
	LOGIN_RESPONSE_PACKET loginRes;
	std::move(packet.begin(), packet.end(), (char*)&loginReq);
	std::cout << loginReq.UserID << ", " << loginReq.UserPW << std::endl;
	loginRes.PacketId = PACKET_ID::LOGIN_RESPONSE;
	loginRes.Result = ERROR_CODE::NONE;
	loginRes.PacketLength = sizeof(LOGIN_RESPONSE_PACKET);
	std::vector<char> res(loginRes.PacketLength);
	std::move(&loginRes, &loginRes + loginRes.PacketLength, res);
	m_network->pushToSendQueue(clinetIndex, std::move(res));
	return 1;
}

void Service::serviceInit()
{
	m_packetProcessMap.emplace(PACKET_ID::LOGIN_REQUEST, std::bind(&Service::packetProcessLoginRequest, this, std::placeholders::_1, std::placeholders::_2));
}

int Service::divergePackets(std::pair<int, std::vector<char> > packetSet)
{
	PACKET_ID* packetId = reinterpret_cast<PACKET_ID*>(&packetSet.second[0] + sizeof(PacketHeader::PacketLength));

	return m_packetProcessMap[*packetId](packetSet.first, packetSet.second);
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
