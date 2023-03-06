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
	std::cout << "�ƹ� Ű�� ���� ������ ����մϴ�" << std::endl;
	std::string inputCmd;
	std::getline(std::cin, inputCmd);
	service.joinService();
	network.downServer();
	return 0;
}