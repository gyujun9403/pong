#pragma once

//

#include <cstdint>
#include "ErrorCode.hpp"

enum class  PACKET_ID : uint16_t
{
	//SYSTEM
	SYS_USER_CONNECT = 11,
	SYS_USER_DISCONNECT = 12,
	SYS_END = 30,

	//DB
	DB_END = 199,

	//Client
	LOGIN_REQUEST = 201,
	LOGIN_RESPONSE = 202,

	ROOM_ENTER_REQUEST = 206,
	ROOM_ENTER_RESPONSE = 207,

	ROOM_LEAVE_REQUEST = 215,
	ROOM_LEAVE_RESPONSE = 216,

	ROOM_CHAT_REQUEST = 221,
	ROOM_CHAT_RESPONSE = 222,
	ROOM_CHAT_NOTIFY = 223,
};

#pragma pack(push, 1)
struct PacketHeader
{
	uint16_t PacketLength;
	PACKET_ID PacketId;
	uint8_t Type; //압축여부 암호화여부 등 속성을 알아내는 값
};

const uint32_t PacketHeader_LENGTH = sizeof(PacketHeader);

//- 로그인 요청
const int MAX_USER_ID_LEN = 32;
const int MAX_USER_PW_LEN = 32;

#pragma pack(push, 1)
struct LOGIN_REQUEST_PACKET : public PacketHeader
{
	char UserID[MAX_USER_ID_LEN + 1];
	char UserPW[MAX_USER_PW_LEN + 1];
};
const size_t LOGIN_REQUEST_PACKET_SZIE = sizeof(LOGIN_REQUEST_PACKET);

#pragma pack(push, 1)
struct LOGIN_RESPONSE_PACKET : public PacketHeader
{
	ERROR_CODE Result;
};