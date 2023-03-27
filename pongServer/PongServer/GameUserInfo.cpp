#include "GameUserInfo.h"

GameUserInfo::GameUserInfo(uint16_t clinetIndex)
:m_clinetIndex(clinetIndex)
{
}

void GameUserInfo::init(uint16_t clinetIndex)
{
	m_clinetIndex = clinetIndex;
}
std::string GameUserInfo::toString()
{
	std::string num = std::to_string(m_clinetIndex);
	return num;
}