#pragma once

#include <map>
#include <string>
#include "User.hpp"

class UserManager
{
private:
	// 유저 리스트를 들고 있는다.
	std::vector<ClientInfo> m_ClientInfoPool;
	std::map<std::string, User> m_userMap;
public:
	void addUser();
	void deleteUser();
	ClientInfo& getUserClientInfo() const;
	User& getUser(std::string id) const;
};