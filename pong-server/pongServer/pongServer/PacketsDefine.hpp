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
	ROOM_NEW_USER_NTF = 208,
	ROOM_USER_LIST_NTF = 209,

	ROOM_LEAVE_REQUEST = 215,
	ROOM_LEAVE_RESPONSE = 216,
	ROOM_LEAVE_USER_NTF = 217,

	ROOM_CHAT_REQUEST = 221,
	ROOM_CHAT_RESPONSE = 222,
	ROOM_CHAT_NOTIFY = 223,

	ROOM_READY_REQ = 224,
	ROOM_READY_RES = 225,
	ROOM_READY_NOTIFY = 226,

	GAME_START_NOTIFY = 300,
	GAME_LAPSE_NOTIFY = 301,
	GAME_CONTROL_REQ = 302,
	GAME_RESULT_NOTIFY = 303,
};

#pragma pack(push, 1)
struct PacketHeader
{
	uint16_t PacketLength;
	PACKET_ID PacketId;
	uint8_t Type; //���࿩�� ��ȣȭ���� �� �Ӽ��� �˾Ƴ��� ��
};

const uint32_t PacketHeader_LENGTH = sizeof(PacketHeader);

//- �α��� ��û
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

//- �뿡 ���� ��û
//const int MAX_ROOM_TITLE_SIZE = 32;
#pragma pack(push, 1)
struct ROOM_ENTER_REQUEST_PACKET : public PacketHeader
{
	int32_t RoomNumber;
};

#pragma pack(push, 1)
struct ROOM_ENTER_RESPONSE_PACKET : public PacketHeader
{
	ERROR_CODE Result;
	//char RivalUserID[MAX_USER_ID_LEN + 1] = { 0, };
};

#pragma pack(push, 1)
struct ROOM_NEW_USER_NOTIFY_PACKET : public PacketHeader
{
	// list������ ��.
	/*
	*        public int UserCount = 0;
        public List<Int64> UserUniqueIdList = new List<Int64>();
        public List<string> UserIDList = new List<string>();
	*/
};

#pragma pack(push, 1)
struct USER_LIST
{
	int64_t userClinetNum;
	int8_t userIdLen; // 10�ڷ� ����?
	char id[MAX_USER_ID_LEN + 1] = {0};
};

//TODO!!
#define MAX_USER_CNT_IN_ROOM 5
#pragma pack(push, 1)
struct ROOM_USER_LIST_NOTIFY_PACKET : public PacketHeader
{
	int8_t UserCount;
	//int64_t UserUniqueId;
	//char UserID[MAX_USER_ID_LEN + 1];
	USER_LIST listArr[MAX_USER_CNT_IN_ROOM];
};

//- �� ������ ��û
#pragma pack(push, 1)
struct ROOM_LEAVE_REQUEST_PACKET : public PacketHeader
{
};

#pragma pack(push, 1)
struct ROOM_LEAVE_RESPONSE_PACKET : public PacketHeader
{
	ERROR_CODE Result;
};

#pragma pack(push, 1)
struct ROOM_LEAVE_NORITY_PACKET : public PacketHeader
{
	int64_t UserUniqueId;
};

// �� ä��
const int MAX_CHAT_MSG_SIZE = 256;
#pragma pack(push, 1)
struct ROOM_CHAT_REQUEST_PACKET : public PacketHeader
{
	char Message[MAX_CHAT_MSG_SIZE + 1] = { 0, };
};

#pragma pack(push, 1)
struct ROOM_CHAT_RESPONSE_PACKET : public PacketHeader
{
	ERROR_CODE Result;
};

#pragma pack(push, 1)
struct ROOM_CHAT_NOTIFY_PACKET : public PacketHeader
{
	char UserID[MAX_USER_ID_LEN + 1] = { 0, };
	char Msg[MAX_CHAT_MSG_SIZE + 1] = { 0, };
};

struct ROOM_READY_REQUEST_PACKET : public PacketHeader
{
	bool isReady;
};

struct ROOM_READY_RESPONSE_PACKET : public PacketHeader
{
	ERROR_CODE Result;
	bool isReady;
};

struct ROOM_READY_NOTIFY_PACKET : public PacketHeader
{
	int64_t UserUniqueId;
	bool isReady;
};
#pragma pack(pop)