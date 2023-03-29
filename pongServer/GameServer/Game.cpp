#include "Game.h"

#include <iostream>

GameStatus Game::getGameStatus()
{
    return m_gameStatus;
}

ERROR_CODE Game::setUsersInGame(std::vector<int32_t> users)
{
    if (m_gameStatus != GameStatus::EMPTY)
    {
        return ERROR_CODE::ROOM_INVALID_INDEX; //TODO 에러코드 추가
    }
    m_gameStatus = GameStatus::WAITING;
    m_userEnterList.resize(users.size());
    m_userList = std::move(users);
    m_gameStartTime = std::chrono::high_resolution_clock::now();
    return ERROR_CODE::NONE;
}

void Game::clearGame()
{
    m_userList.clear();
    m_userEnterList.clear();
    m_gameStatus = GameStatus::EMPTY;
    winnerIndex = -1;
}

std::vector<int32_t> Game::getAllUsers()
{
    return m_userList;
}

void Game::voidGame()
{
    clearGame();
}

std::pair<std::vector<int32_t>, std::vector<int32_t> > Game::finishGame(std::vector<int32_t> losers)
{
    std::vector<int32_t> winner;
    std::vector<int32_t> loser;
    // ↓ 게임 형태에 따라 달라짐.
    std::vector<int32_t>::iterator be = losers.end();
    std::vector<int32_t>::iterator en = losers.end();
    for (int32_t elem : m_userList)
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
    clearGame();
    return std::make_pair<std::vector<int32_t>, std::vector<int32_t> >(std::move(winner), std::move(loser));
}

std::pair<std::vector<int32_t>, std::vector<int32_t> > Game::finishGame()
{
    std::vector<int32_t> winner;
    std::vector<int32_t> loser;
    // ↓ 게임 형태에 따라 달라짐.
    if (losserIndex != -1)
    {
        for (int32_t userElem : m_userList)
        {
            if (userElem != losserIndex)
            {
                winner.push_back(userElem);
            }
            else
            {
                loser.push_back(userElem);
            }
        }
    }
    else if (winnerIndex != -1)
    {
        for (int32_t userElem : m_userList)
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
    clearGame();
    return std::make_pair<std::vector<int32_t>, std::vector<int32_t> >(std::move(winner), std::move(loser));
}

//std::pair<std::vector<int32_t>, std::vector<int32_t> > Game::syncGame()
//{
//    std::vector<int32_t> winner;
//    std::vector<int32_t> loser;
//    if (m_gameStatus == GameStatus::EMPTY || m_gameStatus == GameStatus::FINISHED)
//    {
//    }
//    else if (m_gameStatus == GameStatus::WAITING)
//    {
//        auto elapsed_time = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - m_gameStartTime).count();
//        if (elapsed_time > GAME_CANCEL_TIME) // 3초.
//        {
//            // 게임 정리.
//            std::cout << "not started" << std::endl;
//        }
//    }
//    else if (losserIndex != -1)
//    {
//        for (int32_t userElem : m_userList)
//        {
//            if (userElem != losserIndex)
//            {
//                winner.push_back(userElem);
//            }
//            else
//            {
//                loser.push_back(userElem);
//            }
//        }
//        //clearGame();
//    }
//    else if (winnerIndex != -1)
//    {
//        for (int32_t userElem : m_userList)
//        {
//            if (userElem == winnerIndex)
//            {
//                winner.push_back(userElem);
//            }
//            else
//            {
//                loser.push_back(userElem);
//            }
//        }
//        //clearGame();
//    }
//    return std::make_pair<std::vector<int32_t>, std::vector<int32_t> >(std::move(winner), std::move(loser));
//}

GameSyncResult Game::syncGame()
{
    std::vector<int32_t> winner;
    std::vector<int32_t> loser;
    if (m_gameStatus == GameStatus::EMPTY || m_gameStatus == GameStatus::FINISHED)
    {
    }
    else if (m_gameStatus == GameStatus::WAITING)
    {
        auto elapsed_time = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - m_gameStartTime).count();
        if (elapsed_time > GAME_CANCEL_TIME) // 3초.
        {
            // 게임 정리.
            std::cout << "not started" << std::endl;
            return GameSyncResult::VOIDGAME;
        }
    }
    else if (losserIndex != -1 || winnerIndex != -1)
    {
        return GameSyncResult::FINISH;
    }
    return GameSyncResult::RUN;
}

// 게임 방식에 따라 바꾸기
void Game::setWinnerIndex(int32_t index)
{
    if (winnerIndex == -1)
    {
        winnerIndex = index;
    }
}

GameStatus Game::enterUserInGame(int32_t user)
{
    std::vector<int32_t>::iterator it = std::find(m_userList.begin(), m_userList.end(), user);
    if (it != m_userList.end())
    {
        int32_t index = std::distance(m_userList.begin(), it);
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

GameStatus Game::leaveUserFromGame(int32_t user)
{
    std::vector<int32_t>::iterator it = std::find(m_userList.begin(), m_userList.end(), user);
    if (it != m_userList.end())
    {
        int32_t index = std::distance(m_userList.begin(), it);
        //m_userList.erase(it); 일단 유저는 냅두기. 
        m_userEnterList.at(index) = false;
        losserIndex = index;
    }
    return m_gameStatus;
}
