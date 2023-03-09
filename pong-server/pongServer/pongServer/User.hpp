#pragma once

#include "ClientInfo.h"

enum class OCCUPIED
{
	RESERVED, USING, EMPTY
};

class User
{
private:

	OCCUPIED m_status;
	//ClientInfo m_clinetInfo;
	std::string m_userId;
	int16_t m_clientIndex;
	std::string m_password;
public:
	User();
	//User(const uint16_t index);
	// 필요하다고 생각되는 유저 정보.
	/*
	* std::string pass;...
	*/
	//clientInfo
	bool isUsing() const;
	//void setUser(std::string userId);
	void setUserReserved();
	void setUserUsing();
	void setUserEmpty();
	std::string getUserId();
	void useThisUser(uint16_t clientIndex, std::string userId, const std::string password);
	void clearUser();
	bool checkPassword(std::string inputPassword);
};