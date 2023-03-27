#pragma once

#include <thread>
#include <vector>
#include <map>
#include <functional>
#include <sstream>
#include "Game.h"
#include "PacketsDefine.hpp"
#include "RedisMatching.hpp"
#include "IocpNetworkCore.h"

#pragma pack(push, 1)
struct UserInfo
{
	uint16_t ClinetIndex;
	Game* gameIndex;
};

class GameManagerService
{
private:
	const uint16_t m_gameNum;
	IocpNetworkCore* m_network;
	RedisMatching* m_redis;
	typedef std::function<int(int, std::vector<char>)> FuncType;
	std::vector<Game> m_games;
	std::map<int32_t, UserInfo > m_userInfoMap;
	std::map<int32_t, int32_t > m_ClinetUserMap;
	std::map<PACKET_ID, FuncType> m_packetProcessMap;
	std::atomic<bool> m_isGameRun = true;
	std::thread m_gameRunThread;
	int divergePackets(std::pair<int, std::vector<char> > packetSet);
	void parseAndSetUsers(std::vector<std::string> redisReqs);
	void syncGames();
	Game* getEmptyGame();
	Game* setUserInGame(std::vector<uint16_t> userList);
public:
	GameManagerService(IocpNetworkCore* network, RedisMatching* redis, uint16_t gameNum);
	void pushPacketToSendQueue(int clinetIndex, char* packet, size_t length);
	void initGameManagerService();
	void gameServiceThread();
	void runGameManagerService();
	void joinGameManagerService();
	void makeUsersLose(std::vector<uint16_t> losers);
	int packetProcessGameEnterRequest(int clinetIndex, std::vector<char> ReqPacket);
	int packetProcessGameControlRequest(int clinetIndex, std::vector<char> ReqPacket);
};

