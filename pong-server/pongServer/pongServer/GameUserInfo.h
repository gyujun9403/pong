#pragma once

#include <string>

class GameUserInfo
{
	uint16_t m_clinetIndex;
	std::string m_clientId;
public:
	void init(uint16_t clinetIndex, std::string clientId);
	std::string toString();
};
