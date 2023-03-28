#pragma once

#include <map>
#include <string>
#include "User.hpp"
#include "ErrorCode.hpp"

class UserManager
{
private:
	std::vector<User> m_UserPool;
	std::map<std::string, int> m_userIdMap;
public:
	UserManager(const uint16_t MaxUserNum);
	std::pair<ERROR_CODE, User*> setUser(const uint16_t clientIndex, const std::string userId, const std::string password);
	ERROR_CODE deleteUser(const uint16_t index);
	ERROR_CODE deleteUser(const std::string id);
	std::pair<ERROR_CODE, User*> getUser(const uint16_t index);
	std::pair<ERROR_CODE, User*> getUser(const std::string id);
	ERROR_CODE checkId(const std::string id);
};