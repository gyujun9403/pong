#include "IOCPServer.h"

IocpServer::IocpServer(const uint32_t clientNum, const uint16_t workerThreadNum, const uint16_t port)
: m_listenSocket(INVALID_SOCKET), m_workerThreadNum(workerThreadNum)
	, m_clientNum(clientNum), m_isWorkersRun(true), m_isAccpterRun(true)
{
	m_serverAddr.sin_family = AF_INET;
	m_serverAddr.sin_port = htons(port); //서버 포트를 설정한다.		
	m_serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
}

//IocpServer::IocpServer(UserManager* userManager, const uint16_t workerThreadNum, const uint16_t port)
//	: m_listenSocket(INVALID_SOCKET), m_workerThreadNum(workerThreadNum)
//	, m_isWorkersRun(true), m_isAccpterRun(true), m_userManager(userManager)
//{
//	m_serverAddr.sin_family = AF_INET;
//	m_serverAddr.sin_port = htons(port); //서버 포트를 설정한다.		
//	m_serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
//}

bool IocpServer::initServer()
{
	try
	{
		this->init();
	}
	catch (const std::string& errStr)
	{
		std::cerr << errStr << std::endl;
		return false;
	}
	return true;
}

bool IocpServer::upServer()
{
	try
	{
		this->bind();
		this->listen();
		this->iocpInit();
		this->iocpRun();
	}
	catch (const std::string& errStr)
	{
		std::cerr << errStr << std::endl;
		return false;
	}
	return true;
}

void IocpServer::downServer()
{
	//순서? iocp핸들부터 닫아야하나? 스레드들이 안끝나는데
	this->closeHandle();
	this->closeSocket();
	this->joinThreads();
}

std::string IocpServer::makeErrorStr(const std::string errFunc)
{
	std::string rt = "Init Error(" + errFunc + ", Erro code : " + std::to_string(WSAGetLastError()) + ")";
	return rt;
}

void IocpServer::init()
{
	int rt;
	WSADATA dummyWsaData;
	/*unsigned m_serverHwCoreNum = std::thread::hardware_concurrency();
	if (m_serverHwCoreNum == 0)
	{
		throw "`std::thread::hardware_concurrency()` error. Can't detect system cores.";
	}*/
	rt = WSAStartup(MAKEWORD(2, 2), &dummyWsaData); //https://learn.microsoft.com/ko-kr/windows/win32/api/winsock/nf-winsock-wsastartup
	if (rt != 0)
	{
		std::string throwStr = "Init Error(WSAStartup(...), Erro code:" + std::to_string(rt) + ")";
		throw throwStr;
	}
	m_listenSocket = WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, NULL, WSA_FLAG_OVERLAPPED);
	if (m_listenSocket == INVALID_SOCKET)
	{
		throw makeErrorStr("WSASocketW()");
	}
	std::cout << "init Success" << std::endl;
}

void IocpServer::bind()
{
	if (::bind(m_listenSocket, (SOCKADDR*)&m_serverAddr, sizeof(m_serverAddr)) != 0)
	{
		throw makeErrorStr("::bind(...)");
	}
	std::cout << "bind Success" << std::endl;
}

void IocpServer::listen()
{
	// SOMAXCONN??
	if (::listen(m_listenSocket, SOMAXCONN) != 0)
	{
		throw makeErrorStr("::listen(...)");
	}
	std::cout << "listen Success" << std::endl;
}

void IocpServer::iocpInit()
{
	for (uint16_t i = 0; i < m_clientNum; i++)
	{
		m_clients.emplace_back(i);
	}
	for (uint16_t i = 0; i < m_clientNum; i++)
	{
		m_clients[i].init();
	}
	m_iocpHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, m_workerThreadNum);
	if (m_iocpHandle == NULL)
	{
		throw makeErrorStr("CreateIoCompletionPort(...) init");
	}
	std::cout << "IOCP init Success" << std::endl;
}


void IocpServer::iocpRun()
{
	// in c++ 11

	// worker threads run
	for (size_t i = 0; i < m_workerThreadNum; i++)
	{
		m_workers.emplace_back
		(
			[this]()
			{
				workerThreadFunc();
			}
		);
	}
	// accpet thread run
	m_accecpThread = std::thread
	(
		[this]()
		{
			acceptThreadFunc();
		}
	);
	std::cout << "IOCP running..." << std::endl;
}

void IocpServer::joinThreads()
{
	m_isAccpterRun.store(false);
	m_isWorkersRun.store(false);
	m_accecpThread.join();
	for (size_t i = 0; i < m_workerThreadNum; i++)
	{
		m_workers[i].join();
	}
	std::cout << "threads join done" << std::endl;
}


void IocpServer::closeHandle()
{
	CloseHandle(m_iocpHandle);
	std::cout << "handle closed" << std::endl;
}

void IocpServer::closeSocket()
{
	closesocket(m_listenSocket);
	std::cout << "soecket closed" << std::endl;
}

void IocpServer::recv(ClientInfo& clientInfo)
{
	DWORD dumyRecvByte = 0;
	DWORD dumyFlags = 0;
	//clientInfo.recvOverlapped.wsaBuf.buf = clientInfo.recvOverlapped.buf;
	clientInfo.recvOverlapped.wsaBuf.buf = clientInfo.recvBuf;
	clientInfo.recvOverlapped.wsaBuf.len = SOCKBUFFERSIZE;
	clientInfo.recvOverlapped.ioOperation = IOOperation::RECV;

	int rtRecv = WSARecv
	(
		clientInfo.clientSocket,
		//clientInfo.recvOverlapped.wsaBuf.buf,
		&clientInfo.recvOverlapped.wsaBuf,
		1, 
		&dumyRecvByte,
		&dumyFlags,
		reinterpret_cast<LPWSAOVERLAPPED>(&clientInfo.recvOverlapped), 
		NULL
	);
	if (rtRecv == SOCKET_ERROR)
	{
		if (WSAGetLastError() != ERROR_IO_PENDING)
		{
			std::cerr << makeErrorStr("WSARecv()") << std::endl;
		}
	}
}

void IocpServer::pushToSendQueue(uint16_t clientIndex, std::string str)
{
	// 락을 클라별로 가지고 있는게 맞는거같은데?????
	//std::lock_guard<std::mutex> pushQueueLock(m_sendQueueMutex);
	std::lock_guard<std::mutex> pushQueueLock(m_clients[clientIndex].sendQueueMutex);
	m_clients[clientIndex].sendQueue.push(str);
	if (m_clients[clientIndex].sendQueue.size() == 1)
	{
		this->send(m_clients[clientIndex]);
	}
}

std::pair<int, std::string> IocpServer::getFromRecvQueue()
{
	std::pair<int, std::string> rt(-1, "");
	std::lock_guard<std::mutex> pushQueueLock(m_recvQueueMutex);
	if (!m_recvResultQueue.empty())
	{
		rt = m_recvResultQueue.front();
		m_recvResultQueue.pop();
	}
	return rt;
}

void IocpServer::send(ClientInfo& clientInfo)
{
	DWORD dumyRecvByte = 0;
	DWORD dumyFlags = 0;
	/*size_t strLen = (clientInfo.sendQueue.front().size() > SOCKBUFFERSIZE) 
		? (SOCKBUFFERSIZE) : (clientInfo.sendQueue.front().size());*/
	size_t strLen = (clientInfo.sendQueue.front().size() > 2)
		? (2) : (clientInfo.sendQueue.front().size());

	clientInfo.sendOverlapped.wsaBuf.buf = &clientInfo.sendQueue.front()[0];
	clientInfo.sendOverlapped.wsaBuf.len = strLen;
	clientInfo.sendOverlapped.ioOperation = IOOperation::SEND;

	int rt = WSASend
	(
		clientInfo.clientSocket,
		&clientInfo.sendOverlapped.wsaBuf,
		1,
		&clientInfo.sendOverlapped.wsaBuf.len,
		dumyFlags,
		reinterpret_cast<LPWSAOVERLAPPED>(&clientInfo.sendOverlapped),
		NULL
	);
	if (rt == SOCKET_ERROR)
	{
		if (WSAGetLastError() != ERROR_IO_PENDING)
		{
			std::cerr << makeErrorStr("WSASend()") << std::endl;
		}
	}
}

// Wirte service logic here
void IocpServer::acceptThreadFunc()
{
	sockaddr tempSockaddr;
	int sockaddrSize = sizeof(tempSockaddr);
	HANDLE rtHandle;
	while (m_isAccpterRun.load())
	{
		//ClientInfo* emptyClientInfo = m_userManager->getEmtyUserForAccept();
		ClientInfo* emptyClientInfo = NULL;
		for (ClientInfo& clientElement : m_clients)
		{
			if (clientElement.clientSocket == INVALID_SOCKET)
			{
				emptyClientInfo = &clientElement;
				break;
			}
		}
		if (emptyClientInfo == NULL)
		{
			// 예외처리 필요
			std::cout << makeErrorStr("emptyClientInfo") << std::endl;
			return;
		}

		emptyClientInfo->clientSocket = ::accept(m_listenSocket, &tempSockaddr, &sockaddrSize);
		if (emptyClientInfo->clientSocket == INVALID_SOCKET
			|| emptyClientInfo->clientSocket == NULL)
		{
			std::cout << makeErrorStr("accept") << std::endl;
			return;
		}

		// IOCP에 accept한 클라이언트 등록
		rtHandle = CreateIoCompletionPort
		(
			reinterpret_cast<HANDLE>(emptyClientInfo->clientSocket),
			m_iocpHandle,
			reinterpret_cast<ULONG_PTR>(emptyClientInfo),
			0
		);
		if (rtHandle != m_iocpHandle)// rtHandle == NULL인 경우는 알아서 확인될듯?
		{
			std::cout << makeErrorStr("CreateIoCompletionPort?") << std::endl;
			return;
		}
		std::cout << "accepted" << std::endl;
		// recv등록.
		recv(*emptyClientInfo);
		std::cout << "recv " << std::endl;
	}
	std::cout << "accept stop" << std::endl;
}

void closeClient(ClientInfo& clientInfo, bool forceClose)
{
	uint32_t clientIndex = clientInfo.index;
	struct linger ligerOpt;
	if (forceClose == true)
	{
		ligerOpt = { 1, 0 }; // 강제 종료시, linger옵션 활성화
	}
	else
	{
		ligerOpt = { 0, 0 };
	}
	shutdown(clientInfo.clientSocket, SD_BOTH);
	setsockopt
	(
		clientInfo.clientSocket,
		SOL_SOCKET,
		SO_LINGER,
		reinterpret_cast<char*>(&ligerOpt),
		sizeof(ligerOpt)
	);
	closesocket(clientInfo.clientSocket);
	clientInfo.clientSocket = INVALID_SOCKET;
	std::cout << clientIndex << "out" << std::endl;
}

void IocpServer::workerThreadFunc()
{
	bool rt;
	DWORD transferredByte;
	LPOVERLAPPED overlappedResultPtr;
	ExOverlapped* exOverlappedPtr;
	ClientInfo* clientInfoPtr = NULL;
	while (m_isWorkersRun.load())
	{
		//Sleep(5000);
		rt = GetQueuedCompletionStatus
		(
			m_iocpHandle, 
			&transferredByte,
			reinterpret_cast<PULONG_PTR>(&clientInfoPtr),
			&overlappedResultPtr,
			INFINITE
		);
		if (rt == false && transferredByte == 0 && overlappedResultPtr == NULL)
		{
			m_isWorkersRun.store(false);
			continue;
		}
		if (overlappedResultPtr == NULL)
		{
			continue;
		}
		if (rt == false || (rt == true && transferredByte == 0))
		{
			//std::cout << "closeClient" << std::endl;
			closeClient(*clientInfoPtr, false);
			continue;
			// 클라 접속 종료 -> 소캣 닫기
		}

		exOverlappedPtr = reinterpret_cast<ExOverlapped*>(overlappedResultPtr);
		if (exOverlappedPtr->ioOperation == IOOperation::RECV)
		{
			//std::string recvStr(exOverlappedPtr->buf, transferredByte);
			std::string recvStr(clientInfoPtr->recvBuf, transferredByte);
			std::cout << clientInfoPtr->index  <<  " : " << recvStr;
			{
				std::lock_guard<std::mutex> recvQueueGuard(m_recvQueueMutex);
				m_recvResultQueue.push(std::make_pair(clientInfoPtr->index, recvStr));
			}

			//pushToSendQueue(clientInfoPtr->index, recvStr);
			recv(*clientInfoPtr);
		}
		else if (exOverlappedPtr->ioOperation == IOOperation::SEND)
		{
			/*  Lock을 건다
			*	1. send시 전송한 길이를 확인한다
					1. 0번째 원소의 크기와 같으면 그 원소를 잘 요청한것. queue에서 삭제한다.
					2. 0번째 원소 크기보다 작으면 덜 보낸 것 이므로, 데이터 갱신만 한다.
					3. 만약 원소가 없다면 로직이 이상한 것 임. 삭제를 worker에서만 해줘야함.
				2. queue에 남은 원소가 있다면(다 보냈던, 덜 보냈던) send를 걸어버린다.
			*/
			//std::lock_guard<std::mutex> sendGuard(m_sendQueueMutex);
			std::lock_guard<std::mutex> sendGuard(clientInfoPtr->sendQueueMutex);
			if (clientInfoPtr->sendQueue.size() == 0)
			{
				throw "wtf?";
			}
			else if (transferredByte == clientInfoPtr->sendQueue.front().size())
			{
				// 한번에 다 보낸 경우
				clientInfoPtr->sendQueue.pop();
			}
			else if (transferredByte < clientInfoPtr->sendQueue.front().size())
			{
				// 덜 보낸 경우
				std::string tempStr = clientInfoPtr->sendQueue.front();
				clientInfoPtr->sendQueue.front()
					.assign(tempStr, transferredByte, tempStr.size() - transferredByte);
			}
			if (clientInfoPtr->sendQueue.size() > 0)
			{
				this->send(*clientInfoPtr);
			}
			this->recv(*clientInfoPtr);
		}
		else
		{
			throw "?????";
		}
	}
	std::cout << "woker stop" << std::endl;
}


