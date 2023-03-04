#pragma once

#include <thread>
#include <mutex>

class Service
{
	// 생성자 선언시, UserInfos를 받게 한다.
private:
	std::thread m_serviceThread;


public:
	void runService(); //서비스 스레드를 돌리는 역할.

};