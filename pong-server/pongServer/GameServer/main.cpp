#include "RedisMatching.hpp"
#include "IocpServer.h"
#include "GameManager.h"

int main()
{
	IocpServer network(10, 4, 3335);
	RedisMatching matchingManager("127.0.0.1", 6379);
	GameManagerService gameManagerService(&network, &matchingManager, 3);
	network.initServer();
	network.upServer();
	matchingManager.RedisInit();
	matchingManager.runRecvMatchingThread();
	gameManagerService.initGameManagerService();
	gameManagerService.runGameManagerService();
	std::cout << "아무 키나 누를 때까지 대기합니다" << std::endl;
	std::string inputCmd;
	std::getline(std::cin, inputCmd);
	gameManagerService.joinGameManagerService();
	network.downServer();
	return 1;
}