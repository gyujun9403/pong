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

void User::clearUser()
{
	m_status = OCCUPIED::EMPTY;
}

bool User::checkPassword(std::string inputPassword)
{
	return (inputPassword == m_password);
}
