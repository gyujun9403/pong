#include "IocpNetworkCore.h"
#include "Service.hpp"
#include "UserManager.hpp"
#include "RoomManager.h"
#include "AsyncRedis.hpp"
#include "Logger.h"

#define CLIENT_NUM 100
#define WORKER_THREAD_NUM 4
#define SERVER_PORT 3334

#define REDIS_IP "127.0.0.1"
#define REDIS_PORT 6379

#define USER_NUM 100
#define ROOM_NUM 50

int main()
{
	Logger logger;
	UserManager userManager(USER_NUM);
	RoomManager roomManager(ROOM_NUM);
	IocpNetworkCore network(CLIENT_NUM, WORKER_THREAD_NUM, SERVER_PORT, &logger);
	AsyncRedis asyncRedis(REDIS_IP, REDIS_PORT, &logger);
	Service service(&network, &userManager, &roomManager, &asyncRedis, &logger);

	if (network.initServer() || network.upServer())
	{
		return 0;
	}
	if (asyncRedis.RedisInit())
	{
		return 0;
	}
	asyncRedis.runRedisThread4MC();

	service.serviceInit();
	service.runService();

	logger.log(LogLevel::NOSTAMP, "\n\033[32mMatching and chating server running.\033[0m \nPress any key to shut down the server.");
	std::string inputCmd;
	std::getline(std::cin, inputCmd);

	asyncRedis.RedisJoin();
	service.joinService();
	network.downServer();
	return 0;
}