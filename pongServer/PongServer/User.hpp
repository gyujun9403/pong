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
	std::string m_userId;
	int16_t m_clientIndex;
	std::string m_password;
public:
	User();

	// 필요하다고 생각되는 유저 정보.
	/*
	* std::string pass;...
	*/
	
	bool isUsing() const;
	void setUserReserved();
	void setUserUsing();
	void setUserEmpty();
	std::string getUserId();
	void useThisUser(uint16_t clientIndex, std::string userId, const std::string password);
	void clearUser();
	bool checkPassword(std::string inputPassword);
};