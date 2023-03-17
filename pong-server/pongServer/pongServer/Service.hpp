#pragma once

#include <thread>
#include <mutex>
#include <map>
#include <functional>
#include <sstream>
#include "UserManager.hpp"
#include "RoomManager.h"
#include "IOCPServer.h"
#include "PacketsDefine.hpp"
#include "RedisMatching.hpp"

#define PORT 3334
#define WORKER_THREAD_NUM 4
#define USER_MAX_NUM 365

class Service
{
	// 생성자 선언시, UserInfos를 받게 한다.
	typedef std::function<int(int, std::vector<char>)> FuncType;
private:
	std::thread m_serviceThread;
	IocpServer* m_network;
	UserManager* m_userManager;
	RoomManager* m_roomManager;
	RedisMatching* m_matchingManager;
	std::atomic<bool> m_isServiceRun;
	std::map<PACKET_ID, FuncType> m_packetProcessMap;
	void serviceThread();
	void pushPacketToSendQueue(int clinetIndex, char* packet, size_t length);
	ROOM_USER_LIST_NOTIFY_PACKET makeUserListPacket(std::vector<uint16_t> userInRoomList);
public:
	Service(IocpServer* network, UserManager* userManager, RoomManager* roomManager, RedisMatching* matchingManager);
	void serviceInit();
	void runService(); //서비스 스레드를 돌리는 역할.
	void joinService();
	void redisProcessMatchingResultQueue();
	void redisProcessGameResultQueue();
	int divergePackets(std::pair<int, std::vector<char> > packetSet);
	int packetProcessLoginRequest(int clinetIndex, std::vector<char> ReqPacket);
	int packetProcessRoomEnterRequest(int clinetIndex, std::vector<char> ReqPacket);
	int packetProcessRoomLeaveRequest(int clinetIndex, std::vector<char> ReqPacket);
	int packetProcessRoomChatRequest(int clinetIndex, std::vector<char> ReqPacket);
	int packetProcessRoomReadyRequest(int clinetIndex, std::vector<char> ReqPacket);
};