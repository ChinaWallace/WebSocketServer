#include "ScoketComm.h"
#include <stdio.h>
ScoketComm::ScoketComm() : m_seError(ScoketNotError)
{
	if (SI_SCOKET_COMM_SUM != 0)
	{
		return;
	}

	InitializeCriticalSection(&CS_COMM_SUM_MUTEX);

	if (!StartupWinsock())
	{
		return;
	}
}

ScoketComm::~ScoketComm()
{
	bool bReset = false;

	EnterCriticalSection(&CS_COMM_SUM_MUTEX);
	if (SI_SCOKET_COMM_SUM == 0)
	{
		bReset = true;
	}
	LeaveCriticalSection(&CS_COMM_SUM_MUTEX);

	if (bReset)
	{
		DeleteCriticalSection(&CS_COMM_SUM_MUTEX);

		// ��ֹ���ͷ�Winsock DLL
		if (!WinsockError())
		{
			WSACleanup();
		}
	}
}

ScoketError ScoketComm::GetScoketError(void)
{
	ScoketError error = m_seError;
	m_seError = ScoketNotError;
	return error;
}


// �����׽���
BOOL ScoketComm::CreateSocket(SOCKET *const scoket)
{
	if (WinsockError())
	{
		return FALSE;
	}

	// �����׽���
	*scoket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	ChangeCommSum(TRUE);

	return TRUE;
}

// ָ���׽��ֵ�ַ
BOOL ScoketComm::AppointSocketAddr(SOCKADDR_IN *const addr, const int port, const char *ip, BOOL resolve_ip)
{
	if (WinsockError())
	{
		return FALSE;
	}

	char *cIPAddr = new char[256];

	// �����׽��ֵ�ַ
	SOCKADDR_IN siAddr;

	if (ip == NULL)
	{
		char *cHostIp = ResolveIP();
		if (cHostIp != NULL)
		{
			// ��������Ƿ�������IP��ַ
			siAddr.sin_addr.S_un.S_addr = inet_addr(cHostIp);
			// sin_family��ʾ��ַ�壬����IP��ַ��sin_family��Ա��һֱ��AF_INET
			siAddr.sin_family = AF_INET;
			// ��������Ƿ������˵Ķ˿ں�
			siAddr.sin_port = htons(port);
		}
		else
		{
			return FALSE;
		}
	}
	else 
	{
		if (resolve_ip)
		{
			char *cHostIp = ResolveIP(ip);
			if (cHostIp != NULL)
			{
				siAddr.sin_addr.S_un.S_addr = inet_addr(cHostIp);
				siAddr.sin_family = AF_INET;
				siAddr.sin_port = htons(port);
			}
			else
			{
				return FALSE;
			}
		} 
		else 
		{
			siAddr.sin_addr.S_un.S_addr = inet_addr(ip);
			siAddr.sin_family = AF_INET;
			siAddr.sin_port = htons(port);
		}
	}

	*addr = siAddr;

	delete cIPAddr;

	return TRUE;
}

// �ر��׽���
BOOL ScoketComm::CloseSocket(const SOCKET *const scoket)
{
	if (WinsockError())
	{
		return FALSE;
	}

	// �ر��׽���
	closesocket(*scoket);

	ChangeCommSum(FALSE);

	return TRUE;
}

bool ScoketComm::SetServer(SOCKET &scoket, SOCKADDR_IN *const addr, int amount)
{
	//���׽���
	if (bind(scoket, (SOCKADDR *)addr, sizeof(SOCKADDR)) == SOCKET_ERROR)
	{
		m_seError = ServerBindError;
		return false;
	}

	// ����socket�����ԣ���ΪUDP���������й㲥����
	BOOL bBroadcast = TRUE;
	setsockopt(scoket, SOL_SOCKET, SO_BROADCAST, (const char *)&bBroadcast, sizeof(BOOL));

	//�����ͻ���,������г���amount
	if (listen(scoket, amount) == SOCKET_ERROR)
	{
		m_seError = ServerListenError;
		return false;
	}

	return true;
}

// �ı�������
void ScoketComm::ChangeScoketError(ScoketError error)
{
	m_seError = error;

	return;
}

// ��ȡ������ͨ������
int ScoketComm::GetCommSum()
{
	EnterCriticalSection(&CS_COMM_SUM_MUTEX);
	m_iCommSum = SI_SCOKET_COMM_SUM;
	LeaveCriticalSection(&CS_COMM_SUM_MUTEX);

	return m_iCommSum;
}


// ����Winsock DLL
BOOL ScoketComm::StartupWinsock()
{
	WORD wVersionRequested;
	WSADATA wsaData;

	// ��ָ���ĸߵ��ֽ�ֵ����һ��WORD����
	wVersionRequested = MAKEWORD(1, 1);

	// ����WSAStartup��������Winsock DLL;
	if (WSAStartup(wVersionRequested, &wsaData) != 0)
	{
		// ���ò��ɹ�
		m_seError = WinsockDllError;

		return FALSE;
	}

	if(LOBYTE(wsaData.wVersion) != 1 || HIBYTE(wsaData.wVersion) != 1)
	{
		WSACleanup();// ��ֹ���ͷ�Winsock DLL;

		m_seError = WinsockVersionError;

		return FALSE;
	}

	return TRUE;
}

// Winsock DLL����
BOOL ScoketComm::WinsockError()
{
	if (m_seError == WinsockDllError || m_seError == WinsockVersionError)
	{
		return TRUE;
	}

	return FALSE;
}

// ����IP��ַ
char *ScoketComm::ResolveIP(const char *addr)
{
	struct hostent FAR *lpHost;

	if (addr == NULL)
	{
		// ��ȡ��������
		char cHostName[256];
		if (gethostname(cHostName, sizeof(cHostName)) != 0)
		{
			m_seError = IPResolveError;
			return NULL;
		}

		lpHost = gethostbyname(cHostName);
	}
	else 
	{
		lpHost = gethostbyname(addr);
	}

	if (lpHost == NULL)
	{
		m_seError = IPResolveError;
		return NULL;
	}

	char *cHostIp = NULL;
	// ��ȡIP��ַ
	LPSTR lpAddr = lpHost->h_addr_list[0];
	if (lpAddr)
	{
		struct in_addr inAddr;
		memmove(&inAddr, lpAddr, 4);

		// ת���ɱ�׼IP��ַ��ʽ
		cHostIp = inet_ntoa(inAddr);
		if (cHostIp == NULL)
		{
			m_seError = IPResolveError;
			return NULL;
		}
	}
	else
	{
		m_seError = IPResolveError;
		return NULL;
	}

	return cHostIp;
}

// �ı佨����ͨ������
void ScoketComm::ChangeCommSum(BOOL plus /* = TRUE */)
{
	if (plus)
	{
		EnterCriticalSection(&CS_COMM_SUM_MUTEX);
		++SI_SCOKET_COMM_SUM;
		m_iCommSum = SI_SCOKET_COMM_SUM;
		LeaveCriticalSection(&CS_COMM_SUM_MUTEX);
	} 
	else
	{
		EnterCriticalSection(&CS_COMM_SUM_MUTEX);
		--SI_SCOKET_COMM_SUM;
		m_iCommSum = SI_SCOKET_COMM_SUM;
		LeaveCriticalSection(&CS_COMM_SUM_MUTEX);
	}

	return;
}




// ***************************************************************************************************************
// ������
// ***************************************************************************************************************
ScoketServerComm::ScoketServerComm(const int port, const char *ip, BOOL resolve_ip) : m_bConnect(FALSE)
{
	if (!CreateSocket(&m_sServer))
	{
		MessageBoxW(NULL, L"����������ʧ��", L"EducationComm", MB_ICONERROR);
		return;
	}

	if (!AppointSocketAddr(&m_sServerAddr, port, ip, resolve_ip))
	{
		MessageBoxW(NULL, L"��������������ʧ��", L"EducationComm", MB_ICONERROR);
		return;
	}

	InitializeCriticalSection(&m_csConnectMutex);
}

ScoketServerComm::~ScoketServerComm()
{
	CloseComm();

	if (!CloseSocket(&m_sServer))
	{
		return;
	}

	DeleteCriticalSection(&m_csConnectMutex);
}


// ������Ϣ
BOOL ScoketServerComm::Send(SOCKET &scoket, const char *data, int len)
{
	if (GetScoketError() != ScoketNotError)
	{
		return FALSE;
	}

	if (send(scoket, data, len, 0) <= 0)
	{
		CloseComm();
		ChangeScoketError(SendError);
		return FALSE;
	}

	return TRUE;
}

// ������Ϣ
BOOL ScoketServerComm::Recv(SOCKET &scoket, char *const data, int len)
{
	if (GetScoketError() != ScoketNotError)
	{
		return FALSE;
	}

	if (recv(scoket, data, len, 0) <= 0)
	{
		CloseComm();
		ChangeScoketError(RecvError);
		return FALSE;
	}

	return TRUE;
}

// ���ü����ͻ���
BOOL ScoketServerComm::SetListen(int amount)
{
	if (GetScoketError() != ScoketNotError)
	{
		return FALSE;
	}

	//���׽���
	if (bind(m_sServer, (SOCKADDR *)&m_sServerAddr, sizeof(SOCKADDR)) != 0)
	{
		auto error = WSAGetLastError();
		int k = WSAGetLastError();
		ChangeScoketError(ServerBindError);
		return FALSE;
	}

	//�����ͻ���,������г���amount
	listen(m_sServer, amount);

	return TRUE;
}

// �ȴ�����ͨ��
BOOL ScoketServerComm::Accept()
{
	if (GetScoketError() != ScoketNotError)
	{
		return FALSE;
	}

	bool bAccept = true;

	EnterCriticalSection(&m_csConnectMutex);
	if (m_bConnect)
	{
		bAccept = false;
	}
	LeaveCriticalSection(&m_csConnectMutex);

	if (!bAccept)
	{
		return TRUE;
	}

	SOCKADDR_IN addrClient;
	int iLen = sizeof(SOCKADDR);

	//accept�����������ģ��������Ӳ�������
	m_sComm = accept(m_sServer,(SOCKADDR *)&addrClient, &iLen);
	if (m_sComm == INVALID_SOCKET)
	{
		ChangeScoketError(ServerBindError);
		return FALSE;
	}

	EnterCriticalSection(&m_csConnectMutex);
	m_bConnect = TRUE;
	LeaveCriticalSection(&m_csConnectMutex);

	return TRUE;
}

// �ر������ϵ�ͨ��
void ScoketServerComm::CloseComm()
{
	EnterCriticalSection(&m_csConnectMutex);
	m_bConnect = FALSE;
	LeaveCriticalSection(&m_csConnectMutex);

	closesocket(m_sComm);
	return;
}

// ��ȡ����״̬
BOOL ScoketServerComm::GetConnectState()
{
	BOOL bConnect;

	EnterCriticalSection(&m_csConnectMutex);
	bConnect = m_bConnect;
	LeaveCriticalSection(&m_csConnectMutex);

	return bConnect;
}




// ***************************************************************************************************************
// �ͻ���
// ***************************************************************************************************************
ScoketClientComm::ScoketClientComm(const int port, const char *ip, BOOL resolve_ip)
{
	if (!CreateSocket(&m_sClient))
	{
		return;
	}

	if (!AppointSocketAddr(&m_sClientAddr, port, ip, resolve_ip))
	{
		return;
	}
}

ScoketClientComm::~ScoketClientComm()
{
	if (!CloseSocket(&m_sClient))
	{
		return;
	}
}


// ������Ϣ
BOOL ScoketClientComm::Send(SOCKET &scoket, const char *data, int len)
{
	if (GetScoketError() != ScoketNotError)
	{
		return FALSE;
	}

	if (send(scoket, data, len, 0) <= 0)
	{
		ChangeScoketError(SendError);
		return FALSE;
	}

	return TRUE;
}

// ������Ϣ
BOOL ScoketClientComm::Recv(SOCKET &scoket, char *const data, int len)
{
	if (GetScoketError() != ScoketNotError)
	{
		return FALSE;
	}

	if (recv(scoket, data, len, 0) <= 0)
	{
		ChangeScoketError(RecvError);
		return FALSE;
	}

	return TRUE;
}

// ���ӷ�����
BOOL ScoketClientComm::Connect()
{
	if (GetScoketError() != ScoketNotError && GetScoketError() != ClientConnectError)
	{
		return FALSE;
	}

	//���ӷ�����
	if(connect(m_sClient, (SOCKADDR *)&m_sClientAddr, sizeof(SOCKADDR)) != 0)
	{
		ChangeScoketError(ClientConnectError);
		return FALSE;
	}

	ChangeScoketError(ScoketNotError);

	return TRUE;
}