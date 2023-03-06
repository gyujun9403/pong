#include "Service.hpp"

Service::Service(IocpServer* network, UserManager* userManager)
:m_network(network), m_userManager(userManager), m_isServiceRun(true)
{
}

void Service::serviceThread()
{
	std::pair<int, std::string> recvRt;
	while (m_isServiceRun.load())
	{
		recvRt = m_network->getFromRecvQueue();
		if (recvRt.first != -1)
		{
			std::cout << recvRt.first << ": " << recvRt.second << std::endl;
			m_network->pushToSendQueue(recvRt.first, recvRt.second);
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
