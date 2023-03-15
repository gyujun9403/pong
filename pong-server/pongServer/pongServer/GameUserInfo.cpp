#include "GameUserInfo.h"

void GameUserInfo::init(uint16_t clinetIndex, std::string clientId)
{
	m_clinetIndex = clinetIndex;
	m_clientId = clientId;
}
std::string GameUserInfo::toString()
{
	std::string num = std::to_string(m_clinetIndex);
	return "????";
}