#pragma once
#pragma comment(lib, "ws2_32") // ->
#include <winsock2.h>
#include <Ws2tcpip.h> //->

#include <iostream>
#include <vector>
#include <thread>

#define MAX_SOCKBUF 1024	// ��Ŷ ũ��
#define MAX_WORKERTHREAD 4	// ������ Ǯ�� ������ ��

// IO�۾��� ����
enum class IOOperation
{
	RECV, SEND
};

struct OverlappedEx
{
	WSAOVERLAPPED wsaOverlapped; //Overlapped I/O����ü -> ��ü. ~�ϴ� ����.
	SOCKET		socketClient; //Ŭ���̾�Ʈ ����
	WSABUF		wsaBuf; //Overlapped I/O�۾� ���� -> 
	char		szBuf[MAX_SOCKBUF]; //������ ���� ->
	IOOperation eOperation; //�۾� ���� ����
};

struct ClientInfo
{
	SOCKET socketClient;
	OverlappedEx	recvOverlappedEx;	//RECV Overlapped I/O�۾��� ���� ����
	OverlappedEx	sendOverlappedEx;	//SEND Overlapped I/O�۾��� ���� ����

	ClientInfo()
	{
		ZeroMemory(&recvOverlappedEx, sizeof(OverlappedEx));
		ZeroMemory(&sendOverlappedEx, sizeof(OverlappedEx));
		socketClient = INVALID_SOCKET;
	}
};

class IOCompletionPort
{
private:
	std::vector<ClientInfo> m_clietnInfos;
	SOCKET m_listenSocket = INVALID_SOCKET;
	HANDLE m_IOCPHandle = INVALID_HANDLE_VALUE;
	int m_clientCnt = 0;
	std::vector<std::thread> m_IOWorkerThreads;
	std::thread m_accepterThread; // AcceptThread
	bool m_isWorkersRun = true;
	bool m_isAccepterRun = true;
	char m_socketBuf[1024] = { 0, }; //??? Ŭ�� ������ ���� �ʳ�?

public:
	IOCompletionPort() {}
	~IOCompletionPort()
	{
		WSACleanup();
	}

	bool InitSocket()
	{
		WSADATA wsaData;

		int nRet = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (nRet != 0)
		{
			std::cerr << "[����] WSAStartup()�Լ� ���� : " << WSAGetLastError() << std::endl;
			return false;
		}

		// WSA_FLAG_OVERLAPPED : 
		m_listenSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, NULL, WSA_FLAG_OVERLAPPED);
		if (INVALID_SOCKET == m_listenSocket)
		{
			printf("[����] socket()�Լ� ���� : %d\n", WSAGetLastError());
			return false;
		}

		printf("���� �ʱ�ȭ ����\n");
		return true;
	}
	 
	//------������ �Լ�-------//
	//������ �ּ������� ���ϰ� �����Ű�� ���� ��û�� �ޱ� ���� 
	//������ ����ϴ� �Լ�
	bool BindandListen(int nBindPort)
	{
		SOCKADDR_IN		serverAddr;
		serverAddr.sin_family = AF_INET;
		serverAddr.sin_port = htons(nBindPort); //���� ��Ʈ�� �����Ѵ�.		
		//� �ּҿ��� ������ �����̶� �޾Ƶ��̰ڴ�.
		//���� ������� �̷��� �����Ѵ�. ���� �� �����ǿ����� ������ �ް� �ʹٸ�
		//�� �ּҸ� inet_addr�Լ��� �̿��� ������ �ȴ�.
		serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

		//������ ������ ���� �ּ� ������ cIOCompletionPort ������ �����Ѵ�.
		int nRet = bind(m_listenSocket, (SOCKADDR*)&serverAddr, sizeof(SOCKADDR_IN));
		if (0 != nRet)
		{
			printf("[����] bind()�Լ� ���� : %d\n", WSAGetLastError());
			return false;
		}

		//���� ��û�� �޾Ƶ��̱� ���� cIOCompletionPort������ ����ϰ� 
		//���Ӵ��ť�� 5���� ���� �Ѵ�.
		nRet = listen(m_listenSocket, 5);
		if (0 != nRet)
		{
			printf("[����] listen()�Լ� ���� : %d\n", WSAGetLastError());
			return false;
		}

		printf("���� ��� ����..\n");
		return true;
	}

	//���� ��û�� �����ϰ� �޼����� �޾Ƽ� ó���ϴ� �Լ�
	bool StartServer(const UINT32 maxClientCount)
	{
		CreateClient(maxClientCount);

		// IOCP �ڵ� ����. ���Ӱ� �����ϹǷ� 1, 2, 3�� NULL
		m_IOCPHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, MAX_WORKERTHREAD);
		if (NULL == m_IOCPHandle)
		{
			printf("[����] CreateIoCompletionPort()�Լ� ����: %d\n", GetLastError());
			return false;
		}


		bool bRet = CreateWokerThread();
		if (false == bRet) {
			return false;
		}

		bRet = CreateAccepterThread();
		if (false == bRet) {
			return false;
		}

		printf("���� ����\n");
		return true;
	}

	//�����Ǿ��ִ� �����带 �ı��Ѵ�.
	void DestroyThread()
	{
		m_isWorkersRun = false;
		CloseHandle(m_IOCPHandle);

		for (auto& th : m_IOWorkerThreads)
		{
			if (th.joinable())
			{
				th.join();
			}
		}

		//Accepter �����带 �����Ѵ�.
		m_isAccepterRun = false; // �̰� �Ⱦ��µ�????????????
		closesocket(m_listenSocket);

		if (m_accepterThread.joinable())
		{
			m_accepterThread.join();
		}
	}

private:
	void CreateClient(const UINT32 maxClientCount)
	{
		for (UINT32 i = 0; i < maxClientCount; ++i)
		{
			m_clietnInfos.emplace_back();
		}
	}

	//WaitingThread Queue���� ����� ��������� ����
	bool CreateWokerThread()
	{
		unsigned int uiThreadId = 0;
		//WaingThread Queue�� ��� ���·� ���� ������� ���� ����Ǵ� ���� : (cpu���� * 2) + 1 
		for (int i = 0; i < MAX_WORKERTHREAD; i++)
		{
			m_IOWorkerThreads.emplace_back([this]() { WokerThread(); });
		}

		printf("WokerThread ����..\n");
		return true;
	}

	//Overlapped I/O�۾��� ���� �Ϸ� �뺸�� �޾� 
	//�׿� �ش��ϴ� ó���� �ϴ� �Լ�
	void WokerThread()
	{
		//CompletionKey�� ���� ������ ����
		//	CompletionKey : CreateIoCompletionPort�� ���� IO�۾��� ����Ҷ� �Է��ϴ� ��.
		//		�Ʒ��� GetQueuedCompletionStatus���� �� ���� �޾� IO�� �����Ѵ�.
		//		������ ���ο� ���� �� ���� �����Ͽ�...
		ClientInfo* pClientInfo = NULL;
		//�Լ� ȣ�� ���� ����
		BOOL bSuccess = TRUE;
		//Overlapped I/O�۾����� ���۵� ������ ũ��
		DWORD dwIoSize = 0;
		//I/O �۾��� ���� ��û�� Overlapped ����ü�� ���� ������
		LPOVERLAPPED lpOverlapped = NULL;

		// ��
		while (m_isWorkersRun)
		{
			//////////////////////////////////////////////////////
			//�� �Լ��� ���� ��������� WaitingThread Queue��
			//��� ���·� ���� �ȴ�.
			//�Ϸ�� Overlapped I/O�۾��� �߻��ϸ� IOCP Queue����
			//�Ϸ�� �۾��� ������ �� ó���� �Ѵ�.
			//�׸��� PostQueuedCompletionStatus()�Լ������� �����
			//�޼����� �����Ǹ� �����带 �����Ѵ�.
			//////////////////////////////////////////////////////
			// IO�Ϸ� I/O completion queue���� �����Ͱ� �Է� �� �� ���� ����ϴ� �Լ�
			bSuccess = GetQueuedCompletionStatus(
				m_IOCPHandle,				// (int)Ȯ���� CP�ڵ�.
				&dwIoSize,					// (out)������ ���۵� ����Ʈ��ȯ
				(PULONG_PTR)&pClientInfo,	// (out)CompletionKey. �ش� �����尡 ó���� ���� Ŭ����
				&lpOverlapped,				// (out)Overlapped IO ��ü. CompletionKey�� ����.
				INFINITE);					// (in)����� �ð�

			//����� ������ ���� �޼��� ó��..
			if (bSuccess == TRUE && dwIoSize == 0 && lpOverlapped == NULL)
			{
				m_isWorkersRun = false; // data race�� ���ؼ� �Ű澲�� �ʴ°ǰ�?
				continue;
			}

			if (lpOverlapped == NULL)
			{
				continue;
			}

			//client�� ������ ��������..			
			if (bSuccess == FALSE || (dwIoSize == 0 && bSuccess == TRUE))
			{
				printf("socket(%d) ���� ����\n", (int)pClientInfo->socketClient);
				CloseSocket(pClientInfo);
				continue;
			}


			OverlappedEx* pOverlappedEx = (OverlappedEx*)lpOverlapped;

			//Overlapped I/O Recv�۾� ��� �� ó��
			// WSA Recv, send�� pOverlappedEx���� 6��° ���ڷ� �־��µ�, �װ��� �������� � ������ ó���Ǿ����� Ȯ��.
			if (pOverlappedEx->eOperation == IOOperation::RECV)
			{
				pOverlappedEx->szBuf[dwIoSize] = NULL;
				printf("[����] bytes : %d , msg : %s\n", dwIoSize, pOverlappedEx->szBuf);

				//Ŭ���̾�Ʈ�� �޼����� �����Ѵ�.
				SendMsg(pClientInfo, pOverlappedEx->szBuf, dwIoSize);
				// ��� recv�� �ɾ����
				BindRecv(pClientInfo);
			}
			//Overlapped I/O Send�۾� ��� �� ó��
			else if (IOOperation::SEND == pOverlappedEx->eOperation)
			{
				printf("[�۽�] bytes : %d , msg : %s\n", dwIoSize, pOverlappedEx->szBuf);
			}
			//���� ��Ȳ
			else
			{
				printf("socket(%d)���� ���ܻ�Ȳ\n", (int)pClientInfo->socketClient);
			}
		}
	}

	//WSARecv Overlapped I/O �۾��� ��Ų��.
	bool BindRecv(ClientInfo* pClientInfo)
	{
		DWORD dwFlag = 0;
		DWORD dwRecvNumBytes = 0;

		//Overlapped I/O�� ���� �� ������ ������ �ش�.
		pClientInfo->recvOverlappedEx.wsaBuf.len = MAX_SOCKBUF;
		pClientInfo->recvOverlappedEx.wsaBuf.buf = pClientInfo->recvOverlappedEx.szBuf;
		pClientInfo->recvOverlappedEx.eOperation = IOOperation::RECV;

		int nRet = WSARecv(pClientInfo->socketClient,
			&(pClientInfo->recvOverlappedEx.wsaBuf),
			1,
			&dwRecvNumBytes,
			&dwFlag,
			(LPWSAOVERLAPPED) &(pClientInfo->recvOverlappedEx),
			NULL);

		//socket_error�̸� client socket�� �������ɷ� ó���Ѵ�.
		if (nRet == SOCKET_ERROR && (WSAGetLastError() != ERROR_IO_PENDING))
		{
			printf("[����] WSARecv()�Լ� ���� : %d\n", WSAGetLastError());
			return false;
		}

		return true;
	}

	//WSASend Overlapped I/O�۾��� ��Ų��.
	bool SendMsg(ClientInfo* pClientInfo, char* pMsg, int nLen)
	{
		DWORD dwRecvNumBytes = 0;

		//���۵� �޼����� ����
		CopyMemory(pClientInfo->sendOverlappedEx.szBuf, pMsg, nLen);

		// wsaBuf???
		//Overlapped I/O�� ���� �� ������ ������ �ش�.
		pClientInfo->sendOverlappedEx.wsaBuf.len = nLen;
		pClientInfo->sendOverlappedEx.wsaBuf.buf = pClientInfo->sendOverlappedEx.szBuf; // �� ������ �ܱ� ���� ������ ����
		pClientInfo->sendOverlappedEx.eOperation = IOOperation::SEND;

		int nRet = WSASend(pClientInfo->socketClient,
			&(pClientInfo->sendOverlappedEx.wsaBuf),
			1,
			&dwRecvNumBytes,
			0,
			(LPWSAOVERLAPPED) & (pClientInfo->sendOverlappedEx),
			NULL);

		//socket_error�̸� client socket�� �������ɷ� ó���Ѵ�.
		if (nRet == SOCKET_ERROR && (WSAGetLastError() != ERROR_IO_PENDING))
		{
			printf("[����] WSASend()�Լ� ���� : %d\n", WSAGetLastError());
			return false;
		}
		return true;
	}

	//������ ������ ���� ��Ų��.
	void CloseSocket(ClientInfo* pClientInfo, bool isForce = false)
	{
		struct linger stLinger = { 0, 0 };	// SO_DONTLINGER�� ����

		// bIsForce�� true�̸� SO_LINGER, timeout = 0���� �����Ͽ� ���� ���� ��Ų��. ���� : ������ �ս��� ������ ���� 
		if (isForce == true)
		{
			stLinger.l_onoff = 1;
		}

		//socketClose������ ������ �ۼ����� ��� �ߴ� ��Ų��.
		shutdown(pClientInfo->socketClient, SD_BOTH);

		//���� �ɼ��� �����Ѵ�.
		setsockopt(pClientInfo->socketClient, SOL_SOCKET, SO_LINGER, (char*)&stLinger, sizeof(stLinger));

		//���� ������ ���� ��Ų��. 
		closesocket(pClientInfo->socketClient);

		pClientInfo->socketClient = INVALID_SOCKET;
	}

	//accept��û�� ó���ϴ� ������ ����
	bool CreateAccepterThread()
	{
		m_accepterThread = std::thread([this]() { AccepterThread(); });

		printf("AccepterThread ����..\n");
		return true;
	}

	//������� ������ �޴� ������
	void AccepterThread()
	{
		SOCKADDR_IN		stClientAddr;
		int nAddrLen = sizeof(SOCKADDR_IN);

		while (m_isWorkersRun)
		{
			//������ ���� ����ü�� �ε����� ���´�.
			ClientInfo* pClientInfo = GetEmptyClientInfo();
			if (NULL == pClientInfo)
			{
				printf("[����] Client Full\n");
				return;
			}

			//Ŭ���̾�Ʈ ���� ��û�� ���� ������ ��ٸ���.
			pClientInfo->socketClient = accept(m_listenSocket, (SOCKADDR*)&stClientAddr, &nAddrLen);
			if (INVALID_SOCKET == pClientInfo->socketClient)
			{
				continue;
			}

			//I/O Completion Port��ü�� ������ �����Ų��. -> ������ IOCP�� ��Ƽ� ����.
			bool bRet = BindIOCompletionPort(pClientInfo);
			if (false == bRet)
			{
				return;
			}

			//Recv Overlapped I/O�۾��� ��û�� ���´�.
			bRet = BindRecv(pClientInfo);
			if (false == bRet)
			{
				return;
			}

			char clientIP[32] = { 0, };
			inet_ntop(AF_INET, &(stClientAddr.sin_addr), clientIP, 32 - 1);
			printf("Ŭ���̾�Ʈ ���� : IP(%s) SOCKET(%d)\n", clientIP, (int)pClientInfo->socketClient);

			//Ŭ���̾�Ʈ ���� ����
			++m_clientCnt;
		}
	}

	//������� �ʴ� Ŭ���̾�Ʈ ���� ����ü�� ��ȯ�Ѵ�.
	ClientInfo* GetEmptyClientInfo()
	{
		for (auto& client : m_clietnInfos)
		{
			if (client.socketClient == INVALID_SOCKET)
			{
				return &client;
			}
		}

		return nullptr;
	}

	//CompletionPort��ü�� ���ϰ� CompletionKey�� �����Ű�� ������ �Ѵ�.
	bool BindIOCompletionPort(ClientInfo* pClientInfo)
	{
		//socket�� pClientInfo�� CompletionPort��ü�� �����Ų��.
		auto hIOCP = CreateIoCompletionPort(
			(HANDLE)pClientInfo->socketClient
			, m_IOCPHandle
			, (ULONG_PTR)(pClientInfo), 0);

		if (NULL == hIOCP || m_IOCPHandle != hIOCP)
		{
			printf("[����] CreateIoCompletionPort()�Լ� ����: %d\n", GetLastError());
			return false;
		}

		return true;
	}


};