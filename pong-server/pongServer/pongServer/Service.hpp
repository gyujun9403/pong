#pragma once

#include <thread>
#include <mutex>

class Service
{
	// ������ �����, UserInfos�� �ް� �Ѵ�.
private:
	std::thread m_serviceThread;


public:
	void runService(); //���� �����带 ������ ����.

};