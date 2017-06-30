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

		// 终止和释放Winsock DLL
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


// 创建套接字
BOOL ScoketComm::CreateSocket(SOCKET *const scoket)
{
	if (WinsockError())
	{
		return FALSE;
	}

	// 建立套接字
	*scoket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	ChangeCommSum(TRUE);

	return TRUE;
}

// 指定套接字地址
BOOL ScoketComm::AppointSocketAddr(SOCKADDR_IN *const addr, const int port, const char *ip, BOOL resolve_ip)
{
	if (WinsockError())
	{
		return FALSE;
	}

	char *cIPAddr = new char[256];

	// 定义套接字地址
	SOCKADDR_IN siAddr;

	if (ip == NULL)
	{
		char *cHostIp = ResolveIP();
		if (cHostIp != NULL)
		{
			// 这里填的是服务器的IP地址
			siAddr.sin_addr.S_un.S_addr = inet_addr(cHostIp);
			// sin_family表示地址族，对于IP地址，sin_family成员将一直是AF_INET
			siAddr.sin_family = AF_INET;
			// 这里填的是服务器端的端口号
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

// 关闭套接字
BOOL ScoketComm::CloseSocket(const SOCKET *const scoket)
{
	if (WinsockError())
	{
		return FALSE;
	}

	// 关闭套接字
	closesocket(*scoket);

	ChangeCommSum(FALSE);

	return TRUE;
}

bool ScoketComm::SetServer(SOCKET &scoket, SOCKADDR_IN *const addr, int amount)
{
	//绑定套接字
	if (bind(scoket, (SOCKADDR *)addr, sizeof(SOCKADDR)) == SOCKET_ERROR)
	{
		m_seError = ServerBindError;
		return false;
	}

	// 设置socket的特性，此为UDP播报，具有广播特性
	BOOL bBroadcast = TRUE;
	setsockopt(scoket, SOL_SOCKET, SO_BROADCAST, (const char *)&bBroadcast, sizeof(BOOL));

	//监听客户端,挂起队列长度amount
	if (listen(scoket, amount) == SOCKET_ERROR)
	{
		m_seError = ServerListenError;
		return false;
	}

	return true;
}

// 改变错误类别
void ScoketComm::ChangeScoketError(ScoketError error)
{
	m_seError = error;

	return;
}

// 获取建立的通信数量
int ScoketComm::GetCommSum()
{
	EnterCriticalSection(&CS_COMM_SUM_MUTEX);
	m_iCommSum = SI_SCOKET_COMM_SUM;
	LeaveCriticalSection(&CS_COMM_SUM_MUTEX);

	return m_iCommSum;
}


// 启动Winsock DLL
BOOL ScoketComm::StartupWinsock()
{
	WORD wVersionRequested;
	WSADATA wsaData;

	// 用指定的高低字节值构建一个WORD变量
	wVersionRequested = MAKEWORD(1, 1);

	// 调用WSAStartup函数启动Winsock DLL;
	if (WSAStartup(wVersionRequested, &wsaData) != 0)
	{
		// 调用不成功
		m_seError = WinsockDllError;

		return FALSE;
	}

	if(LOBYTE(wsaData.wVersion) != 1 || HIBYTE(wsaData.wVersion) != 1)
	{
		WSACleanup();// 终止和释放Winsock DLL;

		m_seError = WinsockVersionError;

		return FALSE;
	}

	return TRUE;
}

// Winsock DLL错误
BOOL ScoketComm::WinsockError()
{
	if (m_seError == WinsockDllError || m_seError == WinsockVersionError)
	{
		return TRUE;
	}

	return FALSE;
}

// 解析IP地址
char *ScoketComm::ResolveIP(const char *addr)
{
	struct hostent FAR *lpHost;

	if (addr == NULL)
	{
		// 获取主机名称
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
	// 获取IP地址
	LPSTR lpAddr = lpHost->h_addr_list[0];
	if (lpAddr)
	{
		struct in_addr inAddr;
		memmove(&inAddr, lpAddr, 4);

		// 转化成标准IP地址形式
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

// 改变建立的通信数量
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
// 服务器
// ***************************************************************************************************************
ScoketServerComm::ScoketServerComm(const int port, const char *ip, BOOL resolve_ip) : m_bConnect(FALSE)
{
	if (!CreateSocket(&m_sServer))
	{
		MessageBoxW(NULL, L"服务器创建失败", L"EducationComm", MB_ICONERROR);
		return;
	}

	if (!AppointSocketAddr(&m_sServerAddr, port, ip, resolve_ip))
	{
		MessageBoxW(NULL, L"服务器域名解析失败", L"EducationComm", MB_ICONERROR);
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


// 发送消息
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

// 接受消息
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

// 设置监听客户端
BOOL ScoketServerComm::SetListen(int amount)
{
	if (GetScoketError() != ScoketNotError)
	{
		return FALSE;
	}

	//绑定套接字
	if (bind(m_sServer, (SOCKADDR *)&m_sServerAddr, sizeof(SOCKADDR)) != 0)
	{
		auto error = WSAGetLastError();
		int k = WSAGetLastError();
		ChangeScoketError(ServerBindError);
		return FALSE;
	}

	//监听客户端,挂起队列长度amount
	listen(m_sServer, amount);

	return TRUE;
}

// 等待接收通信
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

	//accept函数是阻塞的，有了连接才往下走
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

// 关闭连接上的通信
void ScoketServerComm::CloseComm()
{
	EnterCriticalSection(&m_csConnectMutex);
	m_bConnect = FALSE;
	LeaveCriticalSection(&m_csConnectMutex);

	closesocket(m_sComm);
	return;
}

// 获取连接状态
BOOL ScoketServerComm::GetConnectState()
{
	BOOL bConnect;

	EnterCriticalSection(&m_csConnectMutex);
	bConnect = m_bConnect;
	LeaveCriticalSection(&m_csConnectMutex);

	return bConnect;
}




// ***************************************************************************************************************
// 客户端
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


// 发送消息
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

// 接受消息
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

// 连接服务器
BOOL ScoketClientComm::Connect()
{
	if (GetScoketError() != ScoketNotError && GetScoketError() != ClientConnectError)
	{
		return FALSE;
	}

	//连接服务器
	if(connect(m_sClient, (SOCKADDR *)&m_sClientAddr, sizeof(SOCKADDR)) != 0)
	{
		ChangeScoketError(ClientConnectError);
		return FALSE;
	}

	ChangeScoketError(ScoketNotError);

	return TRUE;
}