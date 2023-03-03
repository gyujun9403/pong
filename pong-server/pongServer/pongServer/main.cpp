#include "IOCPServer.h"

int main()
{
	IServer* server = new IocpServer(34, 4, 3334);

	server->initServer();
	server->upServer();
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
	server->downServer();
	return 0;
}