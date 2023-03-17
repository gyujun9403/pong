#include "GameManager.h"
#include <cstdlib>

GameManagerService::GameManagerService(IocpServer* network, RedisMatching* redis, uint16_t gameNum)
	:m_network(network), m_redis(redis), m_gameNum(gameNum)
{
}

int GameManagerService::packetProcessGameEnterRequest(int clinetIndex, std::vector<char> ReqPacket)
{
	GAME_ENTER_REQUEST_PACKET gameEnterReq;
	GAME_ENTER_RESPONSE_PACKET gameEnterRes;
	// 게임에 등록된 사람인지 확인.
	std::move(ReqPacket.begin(), ReqPacket.end(), (char*)&gameEnterReq);
	gameEnterRes.PacketId = PACKET_ID::GAME_ENTER_RES;
	gameEnterRes.PacketLength = sizeof(GAME_ENTER_RESPONSE_PACKET);
	std::map<int32_t, UserInfo >::iterator it = m_userInfoMap.find(gameEnterReq.key);
	if (it == m_userInfoMap.end())
	{
		// 실패 결과 보냄.
		gameEnterRes.Result = ERROR_CODE::GAME_NOT_MATCHED_USER;
	}
	else
	{
		it->second.ClinetIndex = clinetIndex;
		// 만약 두명 다 등록 했다면 게임을 RUN상태로
		it->second.gameIndex->enterUserInGame(it->first);
		gameEnterRes.Result = ERROR_CODE::NONE;
	}
	pushPacketToSendQueue(clinetIndex, (char*)&gameEnterRes, gameEnterRes.PacketLength);
	return 0;
}

int GameManagerService::packetProcessGameControlRequest(int clinetIndex, std::vector<char> ReqPacket)
{
	GAME_CONTROL_REQUEST_PACKET gameControlReq;
	//GAME_ENTER_RESPONSE_PACKET gameControlRes;
	std::move(ReqPacket.begin(), ReqPacket.end(), (char*)&gameControlReq);
	//gameEnterRes.PacketId = PACKET_ID::Game;
	//gameEnterRes.PacketLength = sizeof(GAME_ENTER_RESPONSE_PACKET);
	std::map<int32_t, UserInfo >::iterator it = m_userInfoMap.find(gameControlReq.key);
	if (it == m_userInfoMap.end())
	{
		// 실패 결과 보냄.
		//gameEnterRes.Result = ERROR_CODE::GAME_NOT_MATCHED_USER;
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

	return m_packetProcessMap[*packetId](packetSet.first, packetSet.second);
}

void GameManagerService::syncGames()
{
	
	for (Game& elemGame : m_games)
	{
		if (elemGame.getGameStatus() != GameStatus::RUNNING)
		{
			continue;
		}
		std::pair<std::vector<uint16_t>, std::vector<uint16_t> > rt = std::move(elemGame.syncGame());
		GAME_RESULT_NOTIFY_PACKET gameResultNtf;
		GAME_LAPSE_NOTIFY_PACKET gameLapseNtf;
		gameResultNtf.PacketId = PACKET_ID::GAME_RESULT_NOTIFY;
		gameResultNtf.PacketLength = sizeof(GAME_RESULT_NOTIFY_PACKET);
		gameLapseNtf.PacketId = PACKET_ID::GAME_LAPSE_NOTIFY;
		gameLapseNtf.PacketLength = sizeof(GAME_LAPSE_NOTIFY_PACKET);
		// 게임이 어떻게든 결과가 난 경우.
		std::vector<uint16_t> allUsers = std::move(elemGame.getAllUsers());
		if (!rt.first.empty() || !rt.second.empty())
		{
			elemGame.clearGame();
			// 유저 키와 실제 게임 서버에서 클라를 확인할 방법 필요. -> 매핑해야함. gameenterreq사용.
			for (uint16_t elem : rt.first)
			{
				std::map<int32_t, UserInfo >::iterator it = m_userInfoMap.find(elem);
				if (it != m_userInfoMap.end())
				{
					if (it->second.ClinetIndex != -1)
					{
						gameResultNtf.result = true;
						pushPacketToSendQueue(it->second.ClinetIndex, reinterpret_cast<char*>(&gameResultNtf), gameResultNtf.PacketLength);
					}
				}
			}
			for (uint16_t elem : rt.second)
			{
				std::map<int32_t, UserInfo >::iterator it = m_userInfoMap.find(elem);
				if (it != m_userInfoMap.end())
				{
					if (it->second.ClinetIndex != -1)
					{
						gameResultNtf.result = false;
						pushPacketToSendQueue(it->second.ClinetIndex, reinterpret_cast<char*>(&gameResultNtf), gameResultNtf.PacketLength);
					}
				}
			}
			m_userInfoMap.clear();
		}
		else
		{
			for (uint16_t elem : allUsers)
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

Game* GameManagerService::setUserInGame(std::vector<uint16_t> userList)
{
	Game* rtGame = getEmptyGame();
	if (rtGame == NULL)
	{
		return NULL;
	}
	rtGame->setUsersInGame(userList);
	for (uint16_t elem : userList)
	{
		UserInfo temp = { -1, rtGame };
		m_userInfoMap.insert(std::make_pair(elem, std::move(temp)));
		//m_userGameMap.insert(std::make_pair(elem, rtGame));
		//m_userGameMap.insert(std::make_pair<uint16_t, Game*>(elem, rtGame));
	}
}


void GameManagerService::makeUsersLose(std::vector<uint16_t> losers)
{

}

void GameManagerService::parseAndSetUsers(std::vector<std::string> redisReqs)
{
	for (std::string elem : redisReqs)
	{
		std::vector<uint16_t> matchingUserList;
		std::stringstream ss(elem);
		std::string token;

		while (std::getline(ss, token, '-')) {
			matchingUserList.push_back(std::stoi(token));
			//int number = std::stoi(token);
			// 여기에서 number를 처리합니다.
			// ...
		}
		setUserInGame(matchingUserList);
		m_redis->pushToMatchResultQueue(elem);
	}
}

void GameManagerService::gameServiceThread()
{
	int32_t closeUserIndex;
	std::pair<int, std::vector<char> > recvRt;
	while (m_isGameRun.load())
	{
		parseAndSetUsers(m_redis->getFromMatchQueue());
		recvRt = m_network->getFromRecvQueue();
		if (recvRt.first != -1)
		{
			//divergePackets(std::move(recvRt));
			divergePackets(recvRt);
		}
		closeUserIndex = m_network->getCloseUser();
		if (closeUserIndex != -1)
		{
			// 승부 판정 후 방 제거 함수 호출.
			
			std::cout << closeUserIndex << " user out" << std::endl;
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
