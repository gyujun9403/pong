#pragma once

#include <string>

class GameUserInfo
{
	uint16_t m_clinetIndex;
public:
	GameUserInfo(uint16_t clinetIndex);
	void init(uint16_t clinetIndex);
	std::string toString();
};
