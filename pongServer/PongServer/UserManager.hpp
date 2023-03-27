#pragma once

#include <map>
#include <string>
#include "User.hpp"
#include "ErrorCode.hpp"

class UserManager
{
private:
	// 유저 리스트를 들고 있는다.
	//std::vector<ClientInfo> m_ClientInfoPool;
	// 유저를 1. 세션 아이디로 찾거나, 
	std::vector<User> m_UserPool;
	// 2. 문자열 형태의 id로 찾기. -> string의 검색을 빠르게 하기 위해 map으로.
	std::map<std::string, int> m_userIdMap;
	// 유저 추가/삭제시 User
public:
	UserManager(const uint16_t MaxUserNum);
	std::pair<ERROR_CODE, User*> setUser(const uint16_t clientIndex, const std::string userId, const std::string password);
	ERROR_CODE deleteUser(const uint16_t index);
	ERROR_CODE deleteUser(const std::string id);
	std::pair<ERROR_CODE, User*> getUser(const uint16_t index);
	std::pair<ERROR_CODE, User*> getUser(const std::string id);
	ERROR_CODE checkId(const std::string id);
	//ClientInfo* getEmtyUser();
};