#pragma once

#include <vector>
#include <atomic>
#include <algorithm>
#include "ErrorCode.hpp"

enum class GameStatus : uint16_t
{
	EMPTY, WAITING, RUNNING, FINISHED
};

class Game
{
private:
	std::atomic<GameStatus> m_gameStatus;
	std::vector<uint16_t> m_userList;
	std::vector<bool> m_userEnterList;
	// 게임 방식에 따라 바꿔야함.
	int32_t winnerIndex = -1;

public:
	Game() :m_gameStatus(GameStatus::EMPTY) {}
	Game(const Game& other)
	{
		m_gameStatus.store(other.m_gameStatus.load());
		m_userList = other.m_userList;
		m_userEnterList = other.m_userEnterList;
	}
	GameStatus getGameStatus();
	ERROR_CODE setUsersInGame(std::vector<uint16_t> users);
	GameStatus enterUserInGame(uint16_t user);
	void clearGame();
	std::vector<uint16_t> getAllUsers();
	//ERROR_CODE finishGame();
	std::pair<std::vector<uint16_t>, std::vector<uint16_t> > finishGame(std::vector<uint16_t> losers);
	std::pair<std::vector<uint16_t>, std::vector<uint16_t> > syncGame(); // winner, loser

	// 게임 방식에 따라 바꿔야함.
	void setWinnerIndex(uint16_t index);
};

