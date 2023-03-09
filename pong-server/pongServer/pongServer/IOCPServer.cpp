#include "IOCPServer.h"
#include "PacketsDefine.hpp"

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
	for (uint16_t i = 0; i < m_clientNum; i++)
	{
		m_clients.resize(m_clientNum);
		m_clients[i].init(i);
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
	/*clientInfo.recvOverlapped.wsaBuf.buf = clientInfo.recvBuf;
	clientInfo.recvOverlapped.wsaBuf.len = SOCKBUFFERSIZE;*/
	if (clientInfo.recvedLen >= SOCKBUFFERSIZE)
	{
		return;
	}
	clientInfo.recvOverlapped.wsaBuf.buf = clientInfo.recvBuf + clientInfo.recvedLen;
	clientInfo.recvOverlapped.wsaBuf.len = SOCKBUFFERSIZE - clientInfo.recvedLen;
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

//void IocpServer::pushToSendQueue(uint16_t clientIndex, std::string str)
//{
//	// lock�� Ŭ�󺰷� ������ ����.
//	std::lock_guard<std::mutex> pushQueueLock(m_clients[clientIndex].sendQueueMutex);
//	m_clients[clientIndex].sendQueue.push(str);
//	if (m_clients[clientIndex].sendQueue.size() == 1)
//	{
//		this->send(m_clients[clientIndex]);
//	}
//}

void IocpServer::pushToSendQueue(uint16_t clientIndex, std::vector<char> packet)
{
	// lock�� Ŭ�󺰷� ������ ����.
	
	std::lock_guard<std::mutex> pushQueueLock(m_clients[clientIndex].sendQueueMutex);
	//m_clients[clientIndex].sendQueue.push(std::move(packet));
	m_clients[clientIndex].sendQueue.push(packet);
	//std::vector<char>& forDebugVec = m_clients[clientIndex].sendQueue.front();
	//packet.clear();
	if (m_clients[clientIndex].sendQueue.size() == 1)
	{
		this->send(m_clients[clientIndex]);
	}
}

std::pair<int, std::vector<char> > IocpServer::getFromRecvQueue()
{
	std::pair<int, std::vector<char> > rt;
	rt.first = -1;
	std::lock_guard<std::mutex> pushQueueLock(m_recvQueueMutex);
	if (!m_recvResultQueue.empty())
	{
		rt = m_recvResultQueue.front();
		m_recvResultQueue.pop();
	}
	return rt;
}

int32_t IocpServer::getCloseUser()
{
	uint16_t closeUserIndex = -1;
	std::lock_guard<std::mutex> closeUserLock(m_closedUserIndexQueueMutex);
	if (m_closeUserIndexQueue.size() != 0)
	{
		closeUserIndex = m_closeUserIndexQueue.front();
		m_closeUserIndexQueue.pop();
	}
	return closeUserIndex;
}

void IocpServer::send(ClientInfo& clientInfo)
{
	DWORD dumyRecvByte = 0;
	DWORD dumyFlags = 0;
	if (clientInfo.clientSocket == INVALID_SOCKET)
	{
		return;
	}
	/*size_t strLen = (clientInfo.sendQueue.front().size() > SOCKBUFFERSIZE) 
		? (SOCKBUFFERSIZE) : (clientInfo.sendQueue.front().size());*/
	//size_t strLen = (clientInfo.sendQueue.front().size() > 2)
	//	? (2) : (clientInfo.sendQueue.front().size());
	
	size_t buffSize = clientInfo.sendQueue.front().size();
	memcpy_s(clientInfo.sendBuf, SOCKBUFFERSIZE, &clientInfo.sendQueue.front()[0], buffSize);
	//clientInfo.sendOverlapped.wsaBuf.buf = &clientInfo.sendQueue.front()[0];
	//clientInfo.sendOverlapped.wsaBuf.len = clientInfo.sendQueue.front().size();
	//clientInfo.sendOverlapped.wsaBuf.len = strLen;
	clientInfo.sendOverlapped.wsaBuf.buf = clientInfo.sendBuf;
	clientInfo.sendOverlapped.wsaBuf.len = buffSize;
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
			// ����ó�� �ʿ�
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
		// ���� ���� ��Ŷ sendqueue�� ���.
		
		recv(*emptyClientInfo);
		std::cout << "recv " << std::endl;
	}
	std::cout << "accept stop" << std::endl;
}

void IocpServer::closeClient(ClientInfo& clientInfo, bool forceClose)
{
	// ���� ������ ���� ���....
	uint32_t clientIndex = clientInfo.index;
	struct linger ligerOpt;
	if (forceClose == true)
	{
		ligerOpt = { 1, 0 }; // ���� �����, linger�ɼ� Ȱ��ȭ
	}
	else
	{
		//ligerOpt = { 1, 0 };
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
	clientInfo.clearClientInfo();
	//clientInfo.clientSocket = INVALID_SOCKET;
	std::cout << clientIndex << "out" << std::endl;
	std::lock_guard<std::mutex> closeUserLock(m_closedUserIndexQueueMutex);
	m_closeUserIndexQueue.push(clientInfo.index);
}



void IocpServer::workerThreadFunc()
{
	bool rt;
	DWORD transferredByte;
	LPOVERLAPPED overlappedResultPtr;
	ExOverlapped* exOverlappedPtr;
	ClientInfo* clientInfoPtr = NULL;
	uint16_t* packetSizePtr;
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
			closeClient(*clientInfoPtr, false);
			continue;
			// Ŭ�� ���� ���� -> ��Ĺ �ݱ�
		}

		exOverlappedPtr = reinterpret_cast<ExOverlapped*>(overlappedResultPtr);
		if (exOverlappedPtr->ioOperation == IOOperation::RECV)
		{
			char* bufferStart = clientInfoPtr->recvBuf;
			uint32_t nowLen = transferredByte;
			uint32_t& beforeLen = clientInfoPtr->recvedLen;
			// lock�ɱ�? -> recv�� ��� �ٽ� �ɱ� ������ �ɸ� ���� ������ lock�ʿ� ������
			while (nowLen != 0)
			{
				packetSizePtr = reinterpret_cast<uint16_t*>(bufferStart);
				// �� ���� ���, �̹� ���� ���̸� �����Ͽ� Ż�� 
				if ( nowLen + beforeLen < sizeof(PacketHeader::PacketLength)
					|| *packetSizePtr > nowLen + beforeLen)
				{
					beforeLen += nowLen;
					nowLen = 0;
				}
				else
				{
					// �� ���� ��� ������ ���� �ʱ�ȭ �ϰ�, recvQueue�� �ְ�, bufferStart����.
					std::vector<char> temp(*packetSizePtr);
					std::move(bufferStart, bufferStart + *packetSizePtr, temp.begin());
					bufferStart += *packetSizePtr;
					nowLen -= (*packetSizePtr - beforeLen);
					beforeLen = 0;
					std::lock_guard<std::mutex> recvResultQueueLock(m_recvQueueMutex);
					m_recvResultQueue.push(std::make_pair(clientInfoPtr->index, std::move(temp)));
				}
			}
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
			std::lock_guard<std::mutex> sendGuard(clientInfoPtr->sendQueueMutex);
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
				std::vector<char>& tempVec = clientInfoPtr->sendQueue.front();
				//tempVec.assign(tempVec.begin() + transferredByte, tempVec.end());
				std::move(tempVec.begin() + transferredByte, tempVec.end(), tempVec.begin());
				tempVec.resize(tempVec.size() - transferredByte);
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


