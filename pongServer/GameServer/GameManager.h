#pragma once

#include <thread>
#include <vector>
#include <map>
#include <functional>
#include <sstream>
#include "Game.h"
#include "PacketsDefine.hpp"
#include "AsyncRedis.hpp"
#include "IocpNetworkCore.h"
#include "Logger.h"

struct UserInfo
{
	int32_t ClinetIndex;
	Game* gameIndex;
};

class GameManagerService
{
private:
	Logger* m_logger;
	const int32_t m_gameNum;
	int32_t m_runningGameNum = 0;
	IocpNetworkCore* m_network;
	AsyncRedis* m_redis;
	typedef std::function<int(int, std::vector<char>)> FuncType;
	std::map<PACKET_ID, FuncType> m_packetProcessMap;
	
	std::vector<Game> m_games;
	std::map<int32_t, UserInfo > m_userInfoMap;
	std::map<int32_t, int32_t > m_ClinetUserMap;

	std::atomic<bool> m_isGameRun = true;
	std::thread m_gameRunThread;

	int divergePackets(std::pair<int, std::vector<char> > packetSet);
	void parseAndSetUsers(std::vector<std::string> redisReqs, Game* emptyGame);
	void syncGames();
	Game* getEmptyGame();
	bool setUserInGame(std::vector<int32_t> userList, Game* emptyGame);
public:
	GameManagerService(IocpNetworkCore* network, AsyncRedis* redis, int32_t gameNum, Logger* logger);
	void pushPacketToSendQueue(int clinetIndex, char* packet, size_t length);
	void initGameManagerService();
	void gameServiceThread();
	void runGameManagerService();
	void joinGameManagerService();
	int packetProcessGameEnterRequest(int clinetIndex, std::vector<char> ReqPacket);
	int packetProcessGameControlRequest(int clinetIndex, std::vector<char> ReqPacket);
};

