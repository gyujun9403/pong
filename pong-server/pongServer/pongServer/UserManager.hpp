#pragma once

#include <map>
#include <string>
#include "User.hpp"

class UserManager
{
private:
	// ���� ����Ʈ�� ��� �ִ´�.
	//std::vector<ClientInfo> m_ClientInfoPool;
	// ������ 1. ���� ���̵�� ã�ų�, 
	std::vector<User> m_UserPool;
	// 2. ���ڿ� ������ id�� ã��. -> string�� �˻��� ������ �ϱ� ���� map����.
	std::map<std::string, int> m_userIdMap;
	// ���� �߰�/������ User
public:
	UserManager(const uint16_t MaxUserNum);
	User* setUser(const uint16_t clientIndex, const std::string userId, const std::string password);
	void deleteUser(const uint16_t index);
	void deleteUser(const std::string id);
	User* getUser(const uint16_t index);
	User* getUser(const std::string id);
	//ClientInfo* getEmtyUser();
};