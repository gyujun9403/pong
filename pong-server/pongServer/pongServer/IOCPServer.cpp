#include "IOCPServer.h"

IocpServer::IocpServer(const uint32_t clientNum, const uint16_t workerThreadNum, const uint16_t port)
: m_listenSocket(INVALID_SOCKET), m_workerThreadNum(workerThreadNum)
	, m_clientNum(clientNum), m_isWorkersRun(true), m_isAccpterRun(true)
{
	m_serverAddr.sin_family = AF_INET;
	m_serverAddr.sin_port = htons(port); //���� ��Ʈ�� �����Ѵ�.		
	m_serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
}

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
	//����? iocp�ڵ���� �ݾƾ��ϳ�? ��������� �ȳ����µ�
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
	for (size_t i = 0; i < m_clientNum; i++)
	{
		m_clients.emplace_back(i);
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
	clientInfo.recvOverlapped.wsaBuf.buf = clientInfo.recvOverlapped.buf;
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

void IocpServer::pushToSendQueue(ClientInfo& clientInfo, std::string str)
{
	// ���� Ŭ�󺰷� ������ �ִ°� �´°Ű�����?????
	std::lock_guard<std::mutex> pushQueueLock(m_sendQueueMutex);
	clientInfo.sendQueue.push(str);
	if (clientInfo.sendQueue.size() == 1)
	{

	}
}

//void IocpServer::send(ClientInfo& clientInfo, std::string msgStr)
//{
//	DWORD dumyRecvByte = 0;
//	DWORD dumyFlags = 0;
//	size_t strLen = (msgStr.size() > SOCKBUFFERSIZE) ? (SOCKBUFFERSIZE) : (msgStr.size());
//	CopyMemory(clientInfo.sendOverlapped.buf, msgStr.c_str(), strLen);
//
//	clientInfo.sendOverlapped.wsaBuf.buf = clientInfo.sendOverlapped.buf;
//	clientInfo.sendOverlapped.wsaBuf.len = strLen;
//	clientInfo.sendOverlapped.ioOperation = IOOperation::SEND;
//
//	int rt = WSASend
//	(
//		clientInfo.clientSocket,
//		&clientInfo.sendOverlapped.wsaBuf,
//		1,
//		&clientInfo.sendOverlapped.wsaBuf.len,
//		dumyFlags,
//		reinterpret_cast<LPWSAOVERLAPPED>(&clientInfo.sendOverlapped),
//		NULL
//	);
//	if (rt == SOCKET_ERROR)
//	{
//		//if (rt != ERROR_IO_PENDING)
//		if (WSAGetLastError() != ERROR_IO_PENDING)
//		{
//			std::cerr << makeErrorStr("WSASend()") << std::endl;
//		}
//	}
//}

void IocpServer::send(ClientInfo& clientInfo)
{
	DWORD dumyRecvByte = 0;
	DWORD dumyFlags = 0;
	size_t strLen = (clientInfo.sendQueue.front().size() > SOCKBUFFERSIZE) 
		? (SOCKBUFFERSIZE) : (clientInfo.sendQueue.front().size());
	//CopyMemory(clientInfo.sendOverlapped.buf, msgStr.c_str(), strLen);

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
		//if (rt != ERROR_IO_PENDING)
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
		ClientInfo* emptyClientInfo = NULL;
		for (ClientInfo& clientElement : m_clients) //!!!! ���� ���°� �³�?
		{
			if (clientElement.clientSocket == INVALID_SOCKET)
			{
				emptyClientInfo = &clientElement;
				break;
			}
		}
		if (emptyClientInfo == NULL)
		{
			// ����ó�� �ʿ�
			std::cout << makeErrorStr("emptyClientInfo") << std::endl;
			return;
		}

		emptyClientInfo->clientSocket = ::accept(m_listenSocket, &tempSockaddr, &sockaddrSize);
		if (emptyClientInfo->clientSocket == INVALID_SOCKET)
		{
			std::cout << makeErrorStr("accept") << std::endl;
			return;
		}

		// IOCP�� accept�� Ŭ���̾�Ʈ ���
		rtHandle = CreateIoCompletionPort
		(
			reinterpret_cast<HANDLE>(emptyClientInfo->clientSocket),
			m_iocpHandle,
			reinterpret_cast<ULONG_PTR>(emptyClientInfo),
			0
		);
		if (rtHandle != m_iocpHandle)// rtHandle == NULL�� ���� �˾Ƽ� Ȯ�εɵ�?
		{
			std::cout << makeErrorStr("CreateIoCompletionPort?") << std::endl;
			return;
		}
		std::cout << "accepted" << std::endl;
		// recv���.
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
		ligerOpt = { 1, 0 }; // ���� �����, linger�ɼ� Ȱ��ȭ
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
			// Ŭ�� ���� ���� -> ��Ĺ �ݱ�
		}

		exOverlappedPtr = reinterpret_cast<ExOverlapped*>(overlappedResultPtr);
		if (exOverlappedPtr->ioOperation == IOOperation::RECV)
		{
			//
			std::string recvStr(exOverlappedPtr->buf, transferredByte);
			std::cout << clientInfoPtr->index  <<  " : " << recvStr;
			recv(*clientInfoPtr);
		}
		else if (exOverlappedPtr->ioOperation == IOOperation::SEND)
		{
			/*  Lock�� �Ǵ�
			*	1. send�� ������ ���̸� Ȯ���Ѵ�
					1. 0��° ������ ũ��� ������ �� ���Ҹ� �� ��û�Ѱ�. queue���� �����Ѵ�.
					2. 0��° ���� ũ�⺸�� ������ �� ���� �� �̹Ƿ�, ������ ���Ÿ� �Ѵ�.
					3. ���� ���Ұ� ���ٸ� ������ �̻��� �� ��. ������ worker������ �������.
				2. queue�� ���� ���Ұ� �ִٸ�(�� ���´�, �� ���´�) send�� �ɾ������.
			*/
			std::lock_guard<std::mutex> sendGuard(m_sendQueueMutex);
			if (clientInfoPtr->sendQueue.size() == 0)
			{
				throw "wtf?";
			}
			else if (transferredByte == clientInfoPtr->sendQueue.front().size())
			{
				// �ѹ��� �� ���� ���
				clientInfoPtr->sendQueue.pop();
			}
			else if (transferredByte < clientInfoPtr->sendQueue.front().size())
			{
				// �� ���� ���
				std::string tempStr = clientInfoPtr->sendQueue.front();
				clientInfoPtr->sendQueue.front()
					.assign(tempStr, transferredByte, tempStr.size() - transferredByte);
			}
			if (clientInfoPtr->sendQueue.size() > 0)
			{
				this->recv(*clientInfoPtr);
			}
		}
		else
		{
			throw "?????";
		}
	}
	std::cout << "woker stop" << std::endl;
}


