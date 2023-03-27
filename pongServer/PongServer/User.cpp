#include "User.hpp"

User::User()
: m_status(OCCUPIED::EMPTY)
{
}

bool User::isUsing() const
{
	if (m_status == OCCUPIED::USING)
	{
		return true;
	}
	return false;
}

//void User::setUser(std::string userId)
//{
//	m_userId = userId;
//	m_status = OCCUPIED::USING;
//}

void User::setUserReserved()
{
	m_status = OCCUPIED::RESERVED;
}

void User::setUserUsing()
{
	m_status = OCCUPIED::USING;
}

void User::setUserEmpty()
{
	m_status = OCCUPIED::EMPTY;
}

std::string User::getUserId()
{
	if (m_status == OCCUPIED::USING)
	{
		return m_userId;
	}
	else
	{
		return "";
	}
}

void User::useThisUser(uint16_t clientIndex, std::string userId, const std::string password)
{
	m_status = OCCUPIED::USING;
	m_clientIndex = clientIndex;
	m_userId = userId;
	m_password = password;
}

//ClientInfo* User::getClientInfo()
//{
//	return &m_clinetInfo;
// 
//}
//
//int User::getClientInfoIndex()
//{
//	return m_clinetInfo.index;
//}

void User::clearUser()
{
	m_status = OCCUPIED::EMPTY;
	//m_clinetInfo.clearClientInfo();
}

bool User::checkPassword(std::string inputPassword)
{
	return (inputPassword == m_password);
}
