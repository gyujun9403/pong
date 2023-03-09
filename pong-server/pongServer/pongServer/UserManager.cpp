#include "UserManager.hpp"

UserManager::UserManager(const uint16_t MaxUserNum)
{
	for (uint16_t i = 0; i < MaxUserNum; i++)
	{
		//m_ClientInfoPool.emplace_back(i);
		m_UserPool.emplace_back();
	}
}

User* UserManager::setUser(const uint16_t clientIndex, const std::string userId, const std::string password)
{
	//���� Ǯ���� ��� ã�Ƽ� �� ����
	for (uint16_t i = 0; i < m_UserPool.size(); i++)
	{
		if (m_UserPool[i].isUsing() == false)
		{
			m_UserPool[i].useThisUser(clientIndex, userId, password);
			//m_userIdMap.insert(std::pair<std::string, int>(userId, elem.getClientInfoIndex()));
			m_userIdMap.insert(std::pair<std::string, int>(userId, i));
		}
	}
	// mapping���� �� ����
	return NULL;
}

void UserManager::deleteUser(const uint16_t index)
{
	User& user = m_UserPool.at(index);
	m_userIdMap.erase(user.getUserId());
	user.clearUser();
}

void UserManager::deleteUser(const std::string id)
{
	//User& user = m_UserPool.at(m_userIdMap.find(id)->second);
	this->deleteUser(m_userIdMap.find(id)->second);
}

User* UserManager::getUser(const uint16_t index)
{
	// TODO: ���⿡ return ���� �����մϴ�.
	return &m_UserPool.at(index);
}

User* UserManager::getUser(const std::string id)
{
	// TODO: ���⿡ return ���� �����մϴ�.
	return getUser(m_userIdMap.find(id)->second);
}

//ClientInfo* UserManager::getEmtyUser()
//{
//	ClientInfo* rt = NULL;
//	for (User& elem : m_UserPool)
//	{
//		if (elem.isUsing() == false)
//		{
//			//elem.setUser(userId);
//			elem.setUserReserved();
//			//m_userIdMap.insert(std::pair<std::string, int>(userId, elem.getClientInfoIndex()));
//			rt = elem.getClientInfo();
//		}
//	}
//	return rt;
//}
