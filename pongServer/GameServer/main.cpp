#include "AsyncRedis.hpp"
#include "IocpNetworkCore.h"
#include "GameManager.h"
#include "Logger.h"

#define CLIENT_NUM 6
#define WORKER_THREAD_NUM 2
#define SERVER_PORT 3335

#define REDIS_IP "127.0.0.1"
#define REDIS_PORT 6379

#define GAME_NUM 3 // CLIENT_NUM / 한_게임당_인원

int main()
{
	Logger logger;
	IocpNetworkCore network(CLIENT_NUM, WORKER_THREAD_NUM, SERVER_PORT, &logger);
	AsyncRedis asyncRedis(REDIS_IP, REDIS_PORT, &logger);
	GameManagerService gameManagerService(&network, &asyncRedis, GAME_NUM, &logger);

	if (!network.initServer() || !network.upServer())
	{
		return 0;
	}
	if (!asyncRedis.RedisInit())
	{
		return 0;
	}
	asyncRedis.runRedisThread4Game();
	gameManagerService.initGameManagerService();
	gameManagerService.runGameManagerService();
	logger.log(LogLevel::NOSTAMP, "\n\033[32mGame sync server running.\033[0m \nPress any key to shut down the server.");
	std::string inputCmd;
	std::getline(std::cin, inputCmd);

	asyncRedis.RedisJoin();
	gameManagerService.joinGameManagerService();
	network.downServer();
	return 1;
}