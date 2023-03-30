#include "GameManager.h"
#include <cstdlib>

GameManagerService::GameManagerService(IocpNetworkCore* network, AsyncRedis* redis, int32_t gameNum)
	:m_network(network), m_redis(redis), m_gameNum(gameNum)
{
}

int GameManagerService::packetProcessGameEnterRequest(int clinetIndex, std::vector<char> ReqPacket)
{
	GAME_ENTER_REQUEST_PACKET gameEnterReq;
	GAME_ENTER_RESPONSE_PACKET gameEnterRes;
	std::move(ReqPacket.begin(), ReqPacket.end(), (char*)&gameEnterReq);
	gameEnterRes.PacketId = PACKET_ID::GAME_ENTER_RES;
	gameEnterRes.PacketLength = sizeof(GAME_ENTER_RESPONSE_PACKET);
	std::map<int32_t, UserInfo >::iterator it = m_userInfoMap.find(gameEnterReq.key);
	if (it == m_userInfoMap.end())
	{
		gameEnterRes.Result = ERROR_CODE::GAME_NOT_MATCHED_USER;
	}
	else
	{
		it->second.ClinetIndex = clinetIndex;
		m_ClinetUserMap.insert(std::move(std::make_pair(clinetIndex, it->first)));
		// 만약 두명 다 등록 했다면 바로 게임을 RUN상태로
		it->second.gameIndex->enterUserInGame(it->first);
		gameEnterRes.Result = ERROR_CODE::NONE;
	}
	pushPacketToSendQueue(clinetIndex, (char*)&gameEnterRes, gameEnterRes.PacketLength);
	return 0;
}

int GameManagerService::packetProcessGameControlRequest(int clinetIndex, std::vector<char> ReqPacket)
{
	GAME_CONTROL_REQUEST_PACKET gameControlReq;
	std::move(ReqPacket.begin(), ReqPacket.end(), (char*)&gameControlReq);
	// TODO:모든 경우에 성공 패킷을 보내야 하는가?
	//GAME_ENTER_RESPONSE_PACKET gameControlRes;
	//gameEnterRes.PacketId = PACKET_ID::Game;
	//gameEnterRes.PacketLength = sizeof(GAME_ENTER_RESPONSE_PACKET);
	std::map<int32_t, UserInfo >::iterator it = m_userInfoMap.find(gameControlReq.key);
	if (it == m_userInfoMap.end())
	{
		// 실패 결과 보냄.
		//gameEnterRes.Result = ERROR_CODE::GAME_NOT_MATCHED_USER;
		return false;
	}
	else
	{
		it->second.gameIndex->setWinnerIndex(gameControlReq.key);
	}
	return true;
}

void GameManagerService::initGameManagerService()
{

	m_games.resize(m_gameNum);
	m_packetProcessMap.emplace(PACKET_ID::GAME_ENTER_REQ, std::bind(&GameManagerService::packetProcessGameEnterRequest, this, std::placeholders::_1, std::placeholders::_2));
	m_packetProcessMap.emplace(PACKET_ID::GAME_CONTROL_REQ, std::bind(&GameManagerService::packetProcessGameControlRequest, this, std::placeholders::_1, std::placeholders::_2));
}

void GameManagerService::pushPacketToSendQueue(int clinetIndex, char* packet, size_t length)
{
	std::vector<char> res(packet, packet + length);
	res.resize(length);
	m_network->pushToSendQueue(clinetIndex, res);
}

int GameManagerService::divergePackets(std::pair<int, std::vector<char>> packetSet)
{
	PACKET_ID* packetId = reinterpret_cast<PACKET_ID*>(&packetSet.second[0] + sizeof(PacketHeader::PacketLength));
	if (m_packetProcessMap.find(*packetId) == m_packetProcessMap.end())
	{
		std::cerr << "invalid packet form client." << std::endl;
		return -1;
	}
	return m_packetProcessMap[*packetId](packetSet.first, packetSet.second);
}

void GameManagerService::syncGames()
{
	GAME_RESULT_NOTIFY_PACKET gameResultNtf;
	GAME_LAPSE_NOTIFY_PACKET gameLapseNtf;
	gameResultNtf.PacketId = PACKET_ID::GAME_RESULT_NOTIFY;
	gameResultNtf.PacketLength = sizeof(GAME_RESULT_NOTIFY_PACKET);
	gameLapseNtf.PacketId = PACKET_ID::GAME_LAPSE_NOTIFY;
	gameLapseNtf.PacketLength = sizeof(GAME_LAPSE_NOTIFY_PACKET);
	for (Game& elemGame : m_games)
	{
		GameStatus elemStatus = elemGame.getGameStatus();
		// GameStatus::WAITING하고 있는 경우, 인원체크도 해야함.
		if (elemStatus == GameStatus::EMPTY || elemStatus == GameStatus::FINISHED)
		{
			continue;
		}
		GameSyncResult syncRt = elemGame.syncGame();
		std::pair<std::vector<int32_t>, std::vector<int32_t> > gameResult;
		if (syncRt == GameSyncResult::FINISH)
		{
			gameResult = std::move(elemGame.finishGame());
		}
		else if (syncRt == GameSyncResult::VOIDGAME)
		{
			m_network->kickUser(std::move(elemGame.getAllUsers()));
			elemGame.voidGame();
			//연결끊기

			continue;
		}
		std::vector<int32_t> allUsers = std::move(elemGame.getAllUsers());
		if (!gameResult.first.empty() || !gameResult.second.empty()) // Game finished
		{
			for (int32_t elem : gameResult.first)
			{
				std::map<int32_t, UserInfo >::iterator it = m_userInfoMap.find(elem);
				if (it != m_userInfoMap.end())
				{
					if (it->second.ClinetIndex != -1)
					{
						gameResultNtf.result = true;
						pushPacketToSendQueue(it->second.ClinetIndex, reinterpret_cast<char*>(&gameResultNtf), gameResultNtf.PacketLength);
					}
					m_ClinetUserMap.erase(it->second.ClinetIndex);
					m_userInfoMap.erase(it);
				}
			}
			for (int32_t elem : gameResult.second)
			{
				std::map<int32_t, UserInfo >::iterator it = m_userInfoMap.find(elem);
				if (it != m_userInfoMap.end())
				{
					if (it->second.ClinetIndex != -1)
					{
						gameResultNtf.result = false;
						pushPacketToSendQueue(it->second.ClinetIndex, reinterpret_cast<char*>(&gameResultNtf), gameResultNtf.PacketLength);
					}
					m_ClinetUserMap.erase(it->second.ClinetIndex);
					m_userInfoMap.erase(it);
				}
			}
			m_network->kickUser(std::move(allUsers));
		}
		else // Sync Ntf
		{
			for (int32_t elem : allUsers)
			{
				std::map<int32_t, UserInfo >::iterator it = m_userInfoMap.find(elem);
				if (it != m_userInfoMap.end())
				{
					if (it->second.ClinetIndex != -1)
					{
						gameLapseNtf.LapsValue = elem;
						pushPacketToSendQueue(it->second.ClinetIndex, reinterpret_cast<char*>(&gameLapseNtf), gameLapseNtf.PacketLength);
					}
				}
			}
		}
	}
}

Game* GameManagerService::getEmptyGame()
{
	for (Game& elem : m_games)
	{
		if (elem.getGameStatus() == GameStatus::EMPTY)
		{
			return &elem;
		}
	}
	return NULL;
}

bool GameManagerService::setUserInGame(std::vector<int32_t> userList, Game* emptyGame)
{
	//Game* rtGame = getEmptyGame();
	//if (rtGame == NULL)
	//{
	//	return NULL;
	//}
	emptyGame->setUsersInGame(userList);
	for (int32_t elem : userList)
	{
		UserInfo temp = { -1, emptyGame };
		m_userInfoMap.insert(std::move(std::make_pair(elem, std::move(temp))));
	}
	return true;
}


void GameManagerService::makeUsersLose(std::vector<int32_t> losers)
{

}

void GameManagerService::parseAndSetUsers(std::vector<std::string> redisReqs, Game* emptyGame)
{
	for (std::string elem : redisReqs)
	{
		std::vector<int32_t> matchingUserList;
		std::stringstream ss(elem);
		std::string token;

		while (std::getline(ss, token, '-')) {
			matchingUserList.push_back(std::stoi(token));
		}
		setUserInGame(matchingUserList, emptyGame);
		m_redis->pushToMatchResultQueue(elem);
	}
}

void GameManagerService::gameServiceThread()
{
	Game* emptyGame;
	int32_t closeUserIndex;
	std::pair<int, std::vector<char> > recvRt;
	while (m_isGameRun.load())
	{
		emptyGame = getEmptyGame();
		if (emptyGame != NULL)
		{
			parseAndSetUsers(m_redis->getFromMatchQueue(), emptyGame);
		}
		recvRt = m_network->getFromRecvQueue();
		if (recvRt.first != -1)
		{
			divergePackets(recvRt);
		}
		closeUserIndex = m_network->getCloseUser();
		if (closeUserIndex != -1)
		{
			// 승부 판정 후 방 제거 함수 호출.
			std::map<int32_t, int32_t>::iterator it = m_ClinetUserMap.find(closeUserIndex);
			if (it != m_ClinetUserMap.end())
			{
				auto p = m_userInfoMap.find(it->second);
				p->second.gameIndex->leaveUserFromGame(p->first);
				m_userInfoMap.erase(p->first);
				m_ClinetUserMap.erase(closeUserIndex);
			}
		}
		/*redisProcessMatchingResultQueue();
		redisProcessGameResultQueue();*/
		//레디스로 queue받은 애들을 게임 방에 넣는다.
		// 게임 싱크 맞추고, 패킷 보내는 동작.
		
		syncGames();
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
}

void GameManagerService::runGameManagerService()
{
	m_gameRunThread = std::thread
	(
		[this]()
		{
			gameServiceThread();
		}
	);
}

void GameManagerService::joinGameManagerService()
{
	m_isGameRun.store(false);
	m_gameRunThread.join();
}
