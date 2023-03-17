#include "Game.h"

GameStatus Game::getGameStatus()
{
    return m_gameStatus;
}

ERROR_CODE Game::setUsersInGame(std::vector<uint16_t> users)
{
    if (m_gameStatus != GameStatus::EMPTY)
    {
        return ERROR_CODE::ROOM_INVALID_INDEX; //TODO �����ڵ� �߰�
    }
    m_gameStatus = GameStatus::WAITING;
    m_userEnterList.resize(users.size());
    m_userList = std::move(users);
    return ERROR_CODE::NONE;
}

void Game::clearGame()
{
    m_userList.clear();
    m_gameStatus = GameStatus::EMPTY;
    winnerIndex = -1;
}

std::vector<uint16_t> Game::getAllUsers()
{
    return m_userList;
}

std::pair<std::vector<uint16_t>, std::vector<uint16_t> > Game::finishGame(std::vector<uint16_t> losers)
{
    std::vector<uint16_t> winner;
    std::vector<uint16_t> loser;
    std::vector<uint16_t>::iterator be = losers.end();
    std::vector<uint16_t>::iterator en = losers.end();
    for (uint16_t elem : m_userList)
    {
        if (std::find(be, en, elem) != en)
        {
            winner.push_back(elem);
        }
        else
        {
            loser.push_back(elem);
        }
    }
    return std::make_pair<std::vector<uint16_t>, std::vector<uint16_t> >(std::move(winner), std::move(loser));
}

std::pair<std::vector<uint16_t>, std::vector<uint16_t> > Game::syncGame()
{
    std::vector<uint16_t> winner;
    std::vector<uint16_t> loser;
    if (winnerIndex != -1)
    {
        for (uint16_t userElem : m_userList)
        {
            if (userElem == winnerIndex)
            {
                winner.push_back(userElem);
            }
            else
            {
                loser.push_back(userElem);
            }
        }
    }
    return std::make_pair<std::vector<uint16_t>, std::vector<uint16_t> >(std::move(winner), std::move(loser));
}

// ���� ��Ŀ� ���� �ٲٱ�
void Game::setWinnerIndex(uint16_t index)
{
    if (winnerIndex == -1)
    {
        winnerIndex = index;
    }
}

GameStatus Game::enterUserInGame(uint16_t user)
{
    std::vector<uint16_t>::iterator it = std::find(m_userList.begin(), m_userList.end(), user);
    if (it != m_userList.end())
    {
        uint16_t index = std::distance(m_userList.begin(), it);
        m_userEnterList.at(index) = true;
    }
    if (std::all_of(m_userEnterList.begin(), m_userEnterList.end(),
        [](bool b) { return b; }
    ))
    {
        m_gameStatus = GameStatus::RUNNING;
    }
    return m_gameStatus;
}
