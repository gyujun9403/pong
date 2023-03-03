#pragma once
#pragma comment(lib, "ws2_32") // ->
#include <winsock2.h>
#include <Ws2tcpip.h> //->

#include <iostream>
#include <vector>
#include <thread>

#define MAX_SOCKBUF 1024	// 패킷 크기
#define MAX_WORKERTHREAD 4	// 스레드 풀의 스레드 수

// IO작업의 종류
enum class IOOperation
{
	RECV, SEND
};

struct OverlappedEx
{
	WSAOVERLAPPED wsaOverlapped; //Overlapped I/O구조체 -> 본체. ~하는 역할.
	SOCKET		socketClient; //클라이언트 소켓
	WSABUF		wsaBuf; //Overlapped I/O작업 버퍼 -> 
	char		szBuf[MAX_SOCKBUF]; //데이터 버퍼 ->
	IOOperation eOperation; //작업 동작 종류
};

struct ClientInfo
{
	SOCKET socketClient;
	OverlappedEx	recvOverlappedEx;	//RECV Overlapped I/O작업을 위한 변수
	OverlappedEx	sendOverlappedEx;	//SEND Overlapped I/O작업을 위한 변수

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
	char m_socketBuf[1024] = { 0, }; //??? 클라 인포에 있지 않나?

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
			std::cerr << "[에러] WSAStartup()함수 실패 : " << WSAGetLastError() << std::endl;
			return false;
		}

		// WSA_FLAG_OVERLAPPED : 
		m_listenSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, NULL, WSA_FLAG_OVERLAPPED);
		if (INVALID_SOCKET == m_listenSocket)
		{
			printf("[에러] socket()함수 실패 : %d\n", WSAGetLastError());
			return false;
		}

		printf("소켓 초기화 성공\n");
		return true;
	}
	 
	//------서버용 함수-------//
	//서버의 주소정보를 소켓과 연결시키고 접속 요청을 받기 위해 
	//소켓을 등록하는 함수
	bool BindandListen(int nBindPort)
	{
		SOCKADDR_IN		serverAddr;
		serverAddr.sin_family = AF_INET;
		serverAddr.sin_port = htons(nBindPort); //서버 포트를 설정한다.		
		//어떤 주소에서 들어오는 접속이라도 받아들이겠다.
		//보통 서버라면 이렇게 설정한다. 만약 한 아이피에서만 접속을 받고 싶다면
		//그 주소를 inet_addr함수를 이용해 넣으면 된다.
		serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

		//위에서 지정한 서버 주소 정보와 cIOCompletionPort 소켓을 연결한다.
		int nRet = bind(m_listenSocket, (SOCKADDR*)&serverAddr, sizeof(SOCKADDR_IN));
		if (0 != nRet)
		{
			printf("[에러] bind()함수 실패 : %d\n", WSAGetLastError());
			return false;
		}

		//접속 요청을 받아들이기 위해 cIOCompletionPort소켓을 등록하고 
		//접속대기큐를 5개로 설정 한다.
		nRet = listen(m_listenSocket, 5);
		if (0 != nRet)
		{
			printf("[에러] listen()함수 실패 : %d\n", WSAGetLastError());
			return false;
		}

		printf("서버 등록 성공..\n");
		return true;
	}

	//접속 요청을 수락하고 메세지를 받아서 처리하는 함수
	bool StartServer(const UINT32 maxClientCount)
	{
		CreateClient(maxClientCount);

		// IOCP 핸들 생성. 새롭게 생성하므로 1, 2, 3이 NULL
		m_IOCPHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, MAX_WORKERTHREAD);
		if (NULL == m_IOCPHandle)
		{
			printf("[에러] CreateIoCompletionPort()함수 실패: %d\n", GetLastError());
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

		printf("서버 시작\n");
		return true;
	}

	//생성되어있는 쓰레드를 파괴한다.
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

		//Accepter 쓰레드를 종요한다.
		m_isAccepterRun = false; // 이거 안쓰는데????????????
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

	//WaitingThread Queue에서 대기할 쓰레드들을 생성
	bool CreateWokerThread()
	{
		unsigned int uiThreadId = 0;
		//WaingThread Queue에 대기 상태로 넣을 쓰레드들 생성 권장되는 개수 : (cpu개수 * 2) + 1 
		for (int i = 0; i < MAX_WORKERTHREAD; i++)
		{
			m_IOWorkerThreads.emplace_back([this]() { WokerThread(); });
		}

		printf("WokerThread 시작..\n");
		return true;
	}

	//Overlapped I/O작업에 대한 완료 통보를 받아 
	//그에 해당하는 처리를 하는 함수
	void WokerThread()
	{
		//CompletionKey를 받을 포인터 변수
		//	CompletionKey : CreateIoCompletionPort를 통해 IO작업을 명령할때 입력하는 값.
		//		아래의 GetQueuedCompletionStatus에서 이 값을 받아 IO를 구별한다.
		//		보통은 내부에 버퍼 등 또한 정의하여...
		ClientInfo* pClientInfo = NULL;
		//함수 호출 성공 여부
		BOOL bSuccess = TRUE;
		//Overlapped I/O작업에서 전송된 데이터 크기
		DWORD dwIoSize = 0;
		//I/O 작업을 위해 요청한 Overlapped 구조체를 받을 포인터
		LPOVERLAPPED lpOverlapped = NULL;

		// 각
		while (m_isWorkersRun)
		{
			//////////////////////////////////////////////////////
			//이 함수로 인해 쓰레드들은 WaitingThread Queue에
			//대기 상태로 들어가게 된다.
			//완료된 Overlapped I/O작업이 발생하면 IOCP Queue에서
			//완료된 작업을 가져와 뒤 처리를 한다.
			//그리고 PostQueuedCompletionStatus()함수에의해 사용자
			//메세지가 도착되면 쓰레드를 종료한다.
			//////////////////////////////////////////////////////
			// IO완료 I/O completion queue에서 데이터가 입력 될 때 까지 대기하는 함수
			bSuccess = GetQueuedCompletionStatus(
				m_IOCPHandle,				// (int)확인할 CP핸들.
				&dwIoSize,					// (out)실제로 전송된 바이트반환
				(PULONG_PTR)&pClientInfo,	// (out)CompletionKey. 해당 스레드가 처리할 유저 클래스
				&lpOverlapped,				// (out)Overlapped IO 객체. CompletionKey와 유사.
				INFINITE);					// (in)대기할 시간

			//사용자 쓰레드 종료 메세지 처리..
			if (bSuccess == TRUE && dwIoSize == 0 && lpOverlapped == NULL)
			{
				m_isWorkersRun = false; // data race에 대해선 신경쓰지 않는건가?
				continue;
			}

			if (lpOverlapped == NULL)
			{
				continue;
			}

			//client가 접속을 끊었을때..			
			if (bSuccess == FALSE || (dwIoSize == 0 && bSuccess == TRUE))
			{
				printf("socket(%d) 접속 끊김\n", (int)pClientInfo->socketClient);
				CloseSocket(pClientInfo);
				continue;
			}


			OverlappedEx* pOverlappedEx = (OverlappedEx*)lpOverlapped;

			//Overlapped I/O Recv작업 결과 뒤 처리
			// WSA Recv, send시 pOverlappedEx값을 6번째 인자로 넣었는데, 그것을 바탕으로 어떤 동작이 처리되었는지 확인.
			if (pOverlappedEx->eOperation == IOOperation::RECV)
			{
				pOverlappedEx->szBuf[dwIoSize] = NULL;
				printf("[수신] bytes : %d , msg : %s\n", dwIoSize, pOverlappedEx->szBuf);

				//클라이언트에 메세지를 에코한다.
				SendMsg(pClientInfo, pOverlappedEx->szBuf, dwIoSize);
				// 계속 recv를 걸어놓고
				BindRecv(pClientInfo);
			}
			//Overlapped I/O Send작업 결과 뒤 처리
			else if (IOOperation::SEND == pOverlappedEx->eOperation)
			{
				printf("[송신] bytes : %d , msg : %s\n", dwIoSize, pOverlappedEx->szBuf);
			}
			//예외 상황
			else
			{
				printf("socket(%d)에서 예외상황\n", (int)pClientInfo->socketClient);
			}
		}
	}

	//WSARecv Overlapped I/O 작업을 시킨다.
	bool BindRecv(ClientInfo* pClientInfo)
	{
		DWORD dwFlag = 0;
		DWORD dwRecvNumBytes = 0;

		//Overlapped I/O을 위해 각 정보를 셋팅해 준다.
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

		//socket_error이면 client socket이 끊어진걸로 처리한다.
		if (nRet == SOCKET_ERROR && (WSAGetLastError() != ERROR_IO_PENDING))
		{
			printf("[에러] WSARecv()함수 실패 : %d\n", WSAGetLastError());
			return false;
		}

		return true;
	}

	//WSASend Overlapped I/O작업을 시킨다.
	bool SendMsg(ClientInfo* pClientInfo, char* pMsg, int nLen)
	{
		DWORD dwRecvNumBytes = 0;

		//전송될 메세지를 복사
		CopyMemory(pClientInfo->sendOverlappedEx.szBuf, pMsg, nLen);

		// wsaBuf???
		//Overlapped I/O을 위해 각 정보를 셋팅해 준다.
		pClientInfo->sendOverlappedEx.wsaBuf.len = nLen;
		pClientInfo->sendOverlappedEx.wsaBuf.buf = pClientInfo->sendOverlappedEx.szBuf; // 쓸 내용이 단긴 버퍼 포인터 지정
		pClientInfo->sendOverlappedEx.eOperation = IOOperation::SEND;

		int nRet = WSASend(pClientInfo->socketClient,
			&(pClientInfo->sendOverlappedEx.wsaBuf),
			1,
			&dwRecvNumBytes,
			0,
			(LPWSAOVERLAPPED) & (pClientInfo->sendOverlappedEx),
			NULL);

		//socket_error이면 client socket이 끊어진걸로 처리한다.
		if (nRet == SOCKET_ERROR && (WSAGetLastError() != ERROR_IO_PENDING))
		{
			printf("[에러] WSASend()함수 실패 : %d\n", WSAGetLastError());
			return false;
		}
		return true;
	}

	//소켓의 연결을 종료 시킨다.
	void CloseSocket(ClientInfo* pClientInfo, bool isForce = false)
	{
		struct linger stLinger = { 0, 0 };	// SO_DONTLINGER로 설정

		// bIsForce가 true이면 SO_LINGER, timeout = 0으로 설정하여 강제 종료 시킨다. 주의 : 데이터 손실이 있을수 있음 
		if (isForce == true)
		{
			stLinger.l_onoff = 1;
		}

		//socketClose소켓의 데이터 송수신을 모두 중단 시킨다.
		shutdown(pClientInfo->socketClient, SD_BOTH);

		//소켓 옵션을 설정한다.
		setsockopt(pClientInfo->socketClient, SOL_SOCKET, SO_LINGER, (char*)&stLinger, sizeof(stLinger));

		//소켓 연결을 종료 시킨다. 
		closesocket(pClientInfo->socketClient);

		pClientInfo->socketClient = INVALID_SOCKET;
	}

	//accept요청을 처리하는 쓰레드 생성
	bool CreateAccepterThread()
	{
		m_accepterThread = std::thread([this]() { AccepterThread(); });

		printf("AccepterThread 시작..\n");
		return true;
	}

	//사용자의 접속을 받는 쓰레드
	void AccepterThread()
	{
		SOCKADDR_IN		stClientAddr;
		int nAddrLen = sizeof(SOCKADDR_IN);

		while (m_isWorkersRun)
		{
			//접속을 받을 구조체의 인덱스를 얻어온다.
			ClientInfo* pClientInfo = GetEmptyClientInfo();
			if (NULL == pClientInfo)
			{
				printf("[에러] Client Full\n");
				return;
			}

			//클라이언트 접속 요청이 들어올 때까지 기다린다.
			pClientInfo->socketClient = accept(m_listenSocket, (SOCKADDR*)&stClientAddr, &nAddrLen);
			if (INVALID_SOCKET == pClientInfo->socketClient)
			{
				continue;
			}

			//I/O Completion Port객체와 소켓을 연결시킨다. -> 유저를 IOCP에 담아서 관리.
			bool bRet = BindIOCompletionPort(pClientInfo);
			if (false == bRet)
			{
				return;
			}

			//Recv Overlapped I/O작업을 요청해 놓는다.
			bRet = BindRecv(pClientInfo);
			if (false == bRet)
			{
				return;
			}

			char clientIP[32] = { 0, };
			inet_ntop(AF_INET, &(stClientAddr.sin_addr), clientIP, 32 - 1);
			printf("클라이언트 접속 : IP(%s) SOCKET(%d)\n", clientIP, (int)pClientInfo->socketClient);

			//클라이언트 갯수 증가
			++m_clientCnt;
		}
	}

	//사용하지 않는 클라이언트 정보 구조체를 반환한다.
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

	//CompletionPort객체와 소켓과 CompletionKey를 연결시키는 역할을 한다.
	bool BindIOCompletionPort(ClientInfo* pClientInfo)
	{
		//socket과 pClientInfo를 CompletionPort객체와 연결시킨다.
		auto hIOCP = CreateIoCompletionPort(
			(HANDLE)pClientInfo->socketClient
			, m_IOCPHandle
			, (ULONG_PTR)(pClientInfo), 0);

		if (NULL == hIOCP || m_IOCPHandle != hIOCP)
		{
			printf("[에러] CreateIoCompletionPort()함수 실패: %d\n", GetLastError());
			return false;
		}

		return true;
	}


};