#ifndef __SCOKETCOMM_H
#define __SCOKETCOMM_H

#include <winsock.h>
#pragma comment(lib,"wsock32.lib")


enum ScoketError
{
	ScoketNotError,
	WinsockDllError,
	WinsockVersionError,
	HostNameError,
	HostResolveIPError,
	ServerBindError,
	ServerListenError,
	ServerAcceptError,
	ClientConnectError,
	RecvError,
	SendError,
	IPResolveError
};

static int SI_SCOKET_COMM_SUM = 0;							// 建立的通信数量
static CRITICAL_SECTION CS_COMM_SUM_MUTEX;					// 作用于通信数量

class ScoketComm
{
public:
	ScoketComm();
	~ScoketComm();

	ScoketError GetScoketError(void);													// 获取错误类别
	int GetCommSum();																	// 获取建立的通信数量
protected:
	BOOL CreateSocket(SOCKET  *const scoket);											// 创建套接字
	BOOL AppointSocketAddr(SOCKADDR_IN *const addr, const int port, const char *ip = NULL,
		BOOL resolve_ip = FALSE);														// 指定套接字地址

	bool SetServer(SOCKET &scoket, SOCKADDR_IN *const addr, int amount);				// 服务器设定

	virtual BOOL Send(const SOCKET &scoket, const char *data, int len) = 0;				// 发送消息
	virtual BOOL Recv(SOCKET &scoket, char *const data, int len, int &size) = 0;		// 接受消息
	void ChangeScoketError(ScoketError error);											// 改变错误类别

	

	BOOL CloseSocket(const SOCKET *const scoket, bool count = true);					// 关闭套接字
private:
	BOOL StartupWinsock();																// 启动Winsock DLL
	BOOL WinsockError();																// Winsock DLL错误

	char *ResolveIP(const char *addr = NULL);											// 解析IP地址

	void ChangeCommSum(BOOL plus = TRUE);												// 改变建立的通信数量

	ScoketError m_seError;						// 错误类别

	int m_iCommSum;								// 建立的通信数量

};


class ScoketServerComm : public ScoketComm
{
public:
	ScoketServerComm(const int port, const char *ip = NULL, BOOL resolve_ip = FALSE);
	~ScoketServerComm();

	virtual BOOL Send(SOCKET &scoket, const char *data, int len);						// 发送消息
	virtual BOOL Recv(SOCKET &scoket, char *const data, int len, int &size);			// 接受消息

	BOOL SetListen(int amount);															// 设置监听客户端

	BOOL Accept();																		// 等待接收通信
	void CloseComm();																	// 关闭连接上的通信

	BOOL GetConnectState();																// 获取连接状态

	SOCKET GetServerSocket() { return m_sServer; }										// 获取服务器关键字
	SOCKET GetCommSocket() { return m_sComm; }											// 获取通信套接字

protected:
private:
	SOCKET m_sServer;					// 服务器套接字
	SOCKET m_sComm;						// 通信套接字
	SOCKADDR_IN m_sServerAddr;			// 套接字地址

	CRITICAL_SECTION m_csConnectMutex;	// 作用于连接状态

	BOOL m_bConnect;					// 连接状态

};


class ScoketClientComm : public ScoketComm
{
public:
	ScoketClientComm(const int port, const char *ip = NULL, BOOL resolve_ip = FALSE);
	~ScoketClientComm();

	virtual BOOL Send(const SOCKET &scoket, const char *data, int len);					// 发送消息
	virtual BOOL Recv(SOCKET &scoket, char *const data, int len, int &size);			// 接受消息

	BOOL Connect();																		// 连接服务器

	SOCKET GetClientSocket() { return m_sClient; }										// 获取服务器关键字

protected:
private:
	SOCKET m_sClient;					// 客户端套接字
	SOCKADDR_IN m_sClientAddr;			// 套接字地址
};

#endif