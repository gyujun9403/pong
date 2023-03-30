#include "RedisMatching.hpp"
#include "IocpNetworkCore.h"
#include "GameManager.h"
#include "Logger.h"

int main()
{
	Logger logger;
	IocpNetworkCore network(10, 4, 3335, &logger);
	RedisMatching matchingManager("127.0.0.1", 6379, &logger);
	GameManagerService gameManagerService(&network, &matchingManager, 3);
	network.initServer();
	network.upServer();
	matchingManager.RedisInit();
	matchingManager.runRecvMatchingThread();
	gameManagerService.initGameManagerService();
	gameManagerService.runGameManagerService();
	//std::cout << "\n\033[32mGame sync server running.\033[0m \nPress any key to shut down the server." << std::endl;
	logger.log(LogLevel::NOSTAMP, "\n\033[32mGame sync server running.\033[0m \nPress any key to shut down the server.");
	std::string inputCmd;
	std::getline(std::cin, inputCmd);
	gameManagerService.joinGameManagerService();
	network.downServer();
	return 1;
}