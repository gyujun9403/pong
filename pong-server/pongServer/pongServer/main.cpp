#include "IOCPServer.h"
#include "Service.hpp"
#include "UserManager.hpp"
#include "RoomManager.h"
#include "RedisMatching.hpp"

int main()
{
	UserManager userManager(34);
	RoomManager roomManager(60);
	IocpServer network(34, 4, 3334);
	RedisMatching matchingManager("127.0.0.1", 6379);
	Service service(&network, &userManager, &roomManager, &matchingManager);
	network.initServer();
	network.upServer();
	matchingManager.RedisInit();
	matchingManager.runMatchThread();
	service.serviceInit();
	service.runService();
	std::cout << "아무 키나 누를 때까지 대기합니다" << std::endl;
	std::string inputCmd;
	std::getline(std::cin, inputCmd);
	service.joinService();
	network.downServer();
	//RedisMatching redis(0, "127.0.0.1", 6379);
	//redis.RedisInit();
	//GameUserInfo user1(42, "gyeon");
	//GameUserInfo user2(442, "gGyeon");
	//redis.pushToMatchqueue(std::move(user1), std::move(user2));
	return 0;
}