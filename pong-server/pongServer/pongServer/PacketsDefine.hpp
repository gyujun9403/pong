#pragma once

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

struct LOGIN_REQUEST_PACKET : public PacketHeader
{
	char UserID[MAX_USER_ID_LEN + 1];
	char UserPW[MAX_USER_PW_LEN + 1];
};
const size_t LOGIN_REQUEST_PACKET_SZIE = sizeof(LOGIN_REQUEST_PACKET);

struct LOGIN_RESPONSE_PACKET : public PacketHeader
{
	ERROR_CODE Result;
};

//- 룸에 들어가기 요청
//const int MAX_ROOM_TITLE_SIZE = 32;
struct ROOM_ENTER_REQUEST_PACKET : public PacketHeader
{
	int32_t RoomNumber;
};

struct ROOM_ENTER_RESPONSE_PACKET : public PacketHeader
{
	uint16_t Result;
	//char RivalUserID[MAX_USER_ID_LEN + 1] = { 0, };
};

struct ROOM_NEW_USER_NOTIFY_PACKET : public PacketHeader
{
	// list보내야 함.
	/*
	*        public int UserCount = 0;
        public List<Int64> UserUniqueIdList = new List<Int64>();
        public List<string> UserIDList = new List<string>();
	*/
};

struct ROOM_USER_LIST_NOTIFY_PACKET : public PacketHeader
{
	int64_t UserUniqueId;
	char UserID[MAX_USER_ID_LEN + 1];
};

//- 룸 나가기 요청
struct ROOM_LEAVE_REQUEST_PACKET : public PacketHeader
{
};

struct ROOM_LEAVE_RESPONSE_PACKET : public PacketHeader
{
	uint16_t Result;
};

struct ROOM_LEAVE_NORITY_PACKET : public PacketHeader
{
	int64_t UserUniqueId;
};

// 룸 채팅
const int MAX_CHAT_MSG_SIZE = 256;
struct ROOM_CHAT_REQUEST_PACKET : public PacketHeader
{
	char Message[MAX_CHAT_MSG_SIZE + 1] = { 0, };
};

struct ROOM_CHAT_RESPONSE_PACKET : public PacketHeader
{
	INT16 Result;
};

struct ROOM_CHAT_NOTIFY_PACKET : public PacketHeader
{
	char UserID[MAX_USER_ID_LEN + 1] = { 0, };
	char Msg[MAX_CHAT_MSG_SIZE + 1] = { 0, };
};
#pragma pack(pop)