#include "IocpNetworkCore.h"
#include "Service.hpp"
#include "UserManager.hpp"
#include "RoomManager.h"
#include "RedisMatching.hpp"
#include "Logger.h"

int main()
{
	Logger logger;
	UserManager userManager(34);
	RoomManager roomManager(60);
	IocpNetworkCore network(34, 4, 3334, &logger);
	RedisMatching matchingManager("127.0.0.1", 6379, &logger);
	Service service(&network, &userManager, &roomManager, &matchingManager);

	network.initServer();
	network.upServer();

	matchingManager.RedisInit();
	matchingManager.runSendMatchingThread();

	service.serviceInit();
	service.runService();

	//std::cout << "\n\033[32mMatching and chating server running.\033[0m \nPress any key to shut down the server." << std::endl;
	logger.log(LogLevel::NOSTAMP, "\n\033[32mMatching and chating server running.\033[0m \nPress any key to shut down the server.");
	std::string inputCmd;
	std::getline(std::cin, inputCmd);

	service.joinService();
	network.downServer();
	return 0;
}