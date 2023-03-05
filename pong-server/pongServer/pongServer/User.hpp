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
public:
	User();
	//User(const uint16_t index);
	std::string m_userId;
	uint16_t m_clientIndex;
	// 필요하다고 생각되는 유저 정보.
	/*
	* std::string pass;...
	*/
	//clientInfo
	bool isUsing() const;
	void setUser(std::string userId);
	void setUserReserved();
	void setUserUsing();
	void setUserEmpty();
	std::string getUserId();
	void useThisUser(uint16_t clientIndex, std::string userId);
	void clearUser();
};