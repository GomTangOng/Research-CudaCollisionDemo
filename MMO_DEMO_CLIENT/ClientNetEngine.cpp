#include "stdafx.h"
#include "ClientNetEngine.h"
#include "PacketManager.h"
#include "Memory.h"

CClientNetEngine::CClientNetEngine()
{
	m_socket          = INVALID_SOCKET;
	m_id              = -1;
	m_pRecvRingBuffer = nullptr;
	m_pSendMemoryPool = nullptr;
}


CClientNetEngine::~CClientNetEngine()
{
}

bool CClientNetEngine::Start(const HWND hWnd)
{
	WSADATA wsa;
	_wsetlocale(LC_ALL, L"korean");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return false;

	CreateSocket();
	Connect(SERVER_IP, SERVER_PORT);
	RegisterAsyncSelect(hWnd);

	m_pSendMemoryPool = new CSendMemoryPool();
	m_pRecvRingBuffer = new CRingBuffer();
	m_wsaRecvBuf.buf  = m_pRecvRingBuffer->GetEuqueuePosition();
	m_wsaRecvBuf.len  = m_pRecvRingBuffer->GetEmptySize();
	m_wsaSendBuf.buf  = (char *)m_pSendMemoryPool;
	m_wsaSendBuf.len  = MAX_CS_PACKET_SIZE;

	if (!PACKET_MANAGER->Start())
	{
		cout << "PACKET_MANAGER INIT FAILED\n";
		return false;
	}

	return true;
}

void CClientNetEngine::ShutDown()
{
	PACKET_MANAGER->ShutDown();

	Memory::SAFE_DELETE(m_pRecvRingBuffer);
	//Memory::SAFE_DELETE(m_pSendRingBuffer);

	closesocket(m_socket);
	WSACleanup();
}

void CClientNetEngine::CreateSocket()
{
	m_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0);
	if (m_socket == INVALID_SOCKET) err_quit(L"CreateSocket() Failed\n");

	IsNagle(m_socket, false);
}

void CClientNetEngine::Connect(const char * ip, const USHORT port)
{
	int retval;
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family      = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr(ip);
	serveraddr.sin_port        = htons(port);
	
	retval = WSAConnect(m_socket, (sockaddr *)&serveraddr, sizeof(serveraddr), NULL, NULL, NULL, NULL);
	 
	if (retval == SOCKET_ERROR) err_quit(L"connect()");
}

void CClientNetEngine::RegisterAsyncSelect(const HWND hWnd)
{
	int retval = WSAAsyncSelect(m_socket, hWnd, WM_SOCKET, FD_READ | FD_CLOSE);
	if (retval == SOCKET_ERROR)	
	{
		err_quit(L"CClientNetEngine::RegisterAsyncSelect() Failed\n");
	}
}

void CClientNetEngine::SendPacket(char * packet)
{
	int   retval{ 0 };
	DWORD ioByte{ 0 };
	int   len{ packet[0] };

	m_wsaSendBuf.buf = packet;		// 형변환 문제 없겠지...?
	m_wsaSendBuf.len = len;

	retval = WSASend(m_socket, &m_wsaSendBuf, 1, &ioByte, 0, NULL, NULL);

	if (retval)
	{
		int error_code = WSAGetLastError();
		printf("SendPacket Fail [%d]\n", error_code);
	}

	delete reinterpret_cast<CSendMemoryPool *>(packet);	// 오버로딩 된 delete가 실행 될 것인가 확인 할 것 
}

void CClientNetEngine::RecvPacket()
{
	DWORD ioByte{ 0 }, ioFlag{ 0 };
	int retval;

	m_wsaRecvBuf.buf = m_pRecvRingBuffer->GetEuqueuePosition();
	m_wsaRecvBuf.len = m_pRecvRingBuffer->GetEmptySize();

	retval = WSARecv(m_socket, &m_wsaRecvBuf, 1, &ioByte, &ioFlag, NULL, NULL);

	if (retval)
	{
		err_display(L"CClientNetEngine::RecvPacket()");
		return;
	}
	PACKET_MANAGER->Assemble((int)ioByte, m_pRecvRingBuffer, m_id);
}

void CClientNetEngine::IsNagle(SOCKET sock, bool flag)
{
	flag = !flag;	// Note : 함수이름 의미에 맞게 하기 위해서 플래그값을 반대로 바꿔줌

	setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (const char *)&flag, sizeof(flag));
}

void err_quit(TCHAR *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(1);
}

void err_display(TCHAR *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	cout << lpMsgBuf << endl;
	LocalFree(lpMsgBuf);
}