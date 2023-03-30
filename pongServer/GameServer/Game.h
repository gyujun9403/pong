#pragma once

#include <vector>
#include <atomic>
#include <algorithm>
#include <chrono>
#include "ErrorCode.hpp"
#include "Logger.h"

#define GAME_CANCEL_TIME 1'000'000'000 // 1sec

enum class GameStatus : int32_t
{
	EMPTY, WAITING, RUNNING, FINISHED
};

enum class GameSyncResult : int32_t
{
	//����, ��ȿ����, �����
	RUN, VOIDGAME, FINISH
};

class Game
{
private:
	std::atomic<GameStatus> m_gameStatus;
	std::vector<int32_t> m_userList;
	std::vector<bool> m_userEnterList;
	std::chrono::steady_clock::time_point m_gameStartTime;

	// ���� ��Ŀ� ���� �ٲ����.
	int32_t winnerIndex = -1;
	int32_t losserIndex = -1;
	void clearGame();

public:
	Game() :m_gameStatus(GameStatus::EMPTY) {}

	Game(const Game& other)
	{
		m_gameStatus.store(other.m_gameStatus.load());
		m_userList = other.m_userList;
		m_userEnterList = other.m_userEnterList;
	}
	GameStatus getGameStatus();
	ERROR_CODE setUsersInGame(std::vector<int32_t> users);
	GameStatus enterUserInGame(int32_t user);
	GameStatus leaveUserFromGame(int32_t user);
	std::vector<int32_t> getAllUsers();
	GameSyncResult syncGame(); // winner, loser
	void voidGame();
	std::pair<std::vector<int32_t>, std::vector<int32_t> > finishGame(); // ����� ���� �� ����.
	std::pair<std::vector<int32_t>, std::vector<int32_t> > finishGame(std::vector<int32_t> losers); // �й��ڸ� ���� ���� -> ���� �����Ȳ.

	// ���� ��Ŀ� ���� �ٲ����.
	void setWinnerIndex(int32_t index);
};

