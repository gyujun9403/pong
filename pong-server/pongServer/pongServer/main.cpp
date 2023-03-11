#include "IOCPServer.h"
#include "Service.hpp"
#include "UserManager.hpp"
#include "RoomManager.h"

int main()
{
	UserManager userManager(34);
	RoomManager roomManager(60);
	IocpServer network(34, 4, 3334);
	Service service(&network, &userManager, &roomManager);
	network.initServer();
	network.upServer();
	service.serviceInit();
	service.runService();
	std::cout << "아무 키나 누를 때까지 대기합니다" << std::endl;
	std::string inputCmd;
	std::getline(std::cin, inputCmd);
	service.joinService();
	network.downServer();
	return 0;
}