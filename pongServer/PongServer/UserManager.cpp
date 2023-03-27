#include "UserManager.hpp"

UserManager::UserManager(const uint16_t MaxUserNum)
{
	for (uint16_t i = 0; i < MaxUserNum; i++)
	{
		//m_ClientInfoPool.emplace_back(i);
		m_UserPool.emplace_back();
	}
}

std::pair<ERROR_CODE, User*> UserManager::setUser(const uint16_t clientIndex, const std::string userId, const std::string password)
{
	if (m_UserPool[clientIndex].isUsing() == false)
	{
		m_UserPool[clientIndex].useThisUser(clientIndex, userId, password);
		m_userIdMap.insert(std::pair<std::string, int>(userId, clientIndex));
		return std::move(std::make_pair<ERROR_CODE, User*>(ERROR_CODE::NONE, &m_UserPool[clientIndex]));
	}
	else
	{
		return std::make_pair<ERROR_CODE, User*>(ERROR_CODE::LOGIN_USER_ALREADY, &m_UserPool[clientIndex]);
		//throw "INVALID USER POOL";
	}
}

ERROR_CODE UserManager::deleteUser(const uint16_t index)
{
	User& user = m_UserPool.at(index);
	if (user.isUsing() == true)
	{
		m_userIdMap.erase(user.getUserId());
		user.clearUser();
		return ERROR_CODE::NONE;
	}
	else
	{
		return ERROR_CODE::ENTER_ROOM_NOT_FINDE_USER;
	}
}

ERROR_CODE UserManager::deleteUser(const std::string id)
{
	//User& user = m_UserPool.at(m_userIdMap.find(id)->second);
	return this->deleteUser(m_userIdMap.find(id)->second);
}

std::pair<ERROR_CODE, User*> UserManager::getUser(const uint16_t ClientIndex)
{
	// TODO: 여기에 return 문을 삽입합니다.
	if (m_UserPool[ClientIndex].isUsing() == true)
	{
		return std::make_pair<ERROR_CODE, User*>(ERROR_CODE::NONE, &m_UserPool.at(ClientIndex));
	}
	else
	{
		return std::make_pair<ERROR_CODE, User*>(ERROR_CODE::USER_MGR_INVALID_USER_INDEX, NULL);
	}
}

std::pair<ERROR_CODE, User*> UserManager::getUser(const std::string id)
{
	// TODO: 여기에 return 문을 삽입합니다.
	if (m_userIdMap.find(id) == m_userIdMap.end())
	{
		return std::make_pair<ERROR_CODE, User*>(ERROR_CODE::USER_MGR_INVALID_USER_UNIQUEID, NULL);
	}
	return getUser(m_userIdMap[id]);
}

ERROR_CODE UserManager::checkId(const std::string id)
{
	if (m_userIdMap.find(id) != m_userIdMap.end())
	{
		return ERROR_CODE::USER_MGR_INVALID_USER_UNIQUEID;
	}
	return ERROR_CODE::NONE;
}
