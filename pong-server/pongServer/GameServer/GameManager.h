#pragma once

#include <thread>
#include <vector>
#include <map>
#include <functional>
#include <sstream>
#include "Game.h"
#include "PacketsDefine.hpp"
#include "RedisMatching.hpp"
#include "IOCPServer.h"

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
	IocpServer* m_network;
	RedisMatching* m_redis;
	typedef std::function<int(int, std::vector<char>)> FuncType;
	std::vector<Game> m_games;
	//std::map<uint16_t, Game*> m_userGameMap; // ->> 
	std::map<int32_t, UserInfo > m_userInfoMap;
	std::map<PACKET_ID, FuncType> m_packetProcessMap;
	// 스레드생성 -> 게임돌리는 스레드, 게임 돌리면서 
	std::atomic<bool> m_isGameRun = true;
	std::thread m_gameRunThread;
	void checkNewGameReq();
	void sendGameResult();
	int divergePackets(std::pair<int, std::vector<char> > packetSet);
	void parseAndSetUsers(std::vector<std::string> redisReqs);
	void syncGames();
	Game* getEmptyGame();
	Game* setUserInGame(std::vector<uint16_t> userList);
public:
	GameManagerService(IocpServer* network, RedisMatching* redis, uint16_t gameNum);
	void pushPacketToSendQueue(int clinetIndex, char* packet, size_t length);
	void initGameManagerService();
	void gameServiceThread();
	void runGameManagerService();
	void joinGameManagerService();
	void makeUsersLose(std::vector<uint16_t> losers);
	int packetProcessGameEnterRequest(int clinetIndex, std::vector<char> ReqPacket);
	int packetProcessGameControlRequest(int clinetIndex, std::vector<char> ReqPacket);
};

