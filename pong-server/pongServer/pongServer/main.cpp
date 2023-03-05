#include "IOCPServer.h"
#include "Service.hpp"
#include "UserManager.hpp"

int main()
{
	UserManager userManager(34);
	IocpServer network(34, 4, 3334);
	Service service(&network, &userManager);
	network.initServer();
	network.upServer();
	service.runService();
	std::cout << "아무 키나 누를 때까지 대기합니다" << std::endl;
	//while (true)
	//{
		std::string inputCmd;
		std::getline(std::cin, inputCmd);

		//if (inputCmd == "quit")
		//{
			//break;
		//}
	//}
	network.downServer();
	return 0;
}