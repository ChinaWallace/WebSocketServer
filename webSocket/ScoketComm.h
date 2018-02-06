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

static int SI_SCOKET_COMM_SUM = 0;							// ������ͨ������
static CRITICAL_SECTION CS_COMM_SUM_MUTEX;					// ������ͨ������

class ScoketComm
{
public:
	ScoketComm();
	~ScoketComm();

	ScoketError GetScoketError(void);													// ��ȡ�������
	int GetCommSum();																	// ��ȡ������ͨ������
protected:
	BOOL CreateSocket(SOCKET  *const scoket);											// �����׽���
	BOOL AppointSocketAddr(SOCKADDR_IN *const addr, const int port, const char *ip = NULL,
		BOOL resolve_ip = FALSE);														// ָ���׽��ֵ�ַ

	bool SetServer(SOCKET &scoket, SOCKADDR_IN *const addr, int amount);				// �������趨

	virtual BOOL Send(const SOCKET &scoket, const char *data, int len) = 0;				// ������Ϣ
	virtual BOOL Recv(SOCKET &scoket, char *const data, int len, int &size) = 0;		// ������Ϣ
	void ChangeScoketError(ScoketError error);											// �ı�������

	

	BOOL CloseSocket(const SOCKET *const scoket, bool count = true);					// �ر��׽���
private:
	BOOL StartupWinsock();																// ����Winsock DLL
	BOOL WinsockError();																// Winsock DLL����

	char *ResolveIP(const char *addr = NULL);											// ����IP��ַ

	void ChangeCommSum(BOOL plus = TRUE);												// �ı佨����ͨ������

	ScoketError m_seError;						// �������

	int m_iCommSum;								// ������ͨ������

};


class ScoketServerComm : public ScoketComm
{
public:
	ScoketServerComm(const int port, const char *ip = NULL, BOOL resolve_ip = FALSE);
	~ScoketServerComm();

	virtual BOOL Send(SOCKET &scoket, const char *data, int len);						// ������Ϣ
	virtual BOOL Recv(SOCKET &scoket, char *const data, int len, int &size);			// ������Ϣ

	BOOL SetListen(int amount);															// ���ü����ͻ���

	BOOL Accept();																		// �ȴ�����ͨ��
	void CloseComm();																	// �ر������ϵ�ͨ��

	BOOL GetConnectState();																// ��ȡ����״̬

	SOCKET GetServerSocket() { return m_sServer; }										// ��ȡ�������ؼ���
	SOCKET GetCommSocket() { return m_sComm; }											// ��ȡͨ���׽���

protected:
private:
	SOCKET m_sServer;					// �������׽���
	SOCKET m_sComm;						// ͨ���׽���
	SOCKADDR_IN m_sServerAddr;			// �׽��ֵ�ַ

	CRITICAL_SECTION m_csConnectMutex;	// ����������״̬

	BOOL m_bConnect;					// ����״̬

};


class ScoketClientComm : public ScoketComm
{
public:
	ScoketClientComm(const int port, const char *ip = NULL, BOOL resolve_ip = FALSE);
	~ScoketClientComm();

	virtual BOOL Send(const SOCKET &scoket, const char *data, int len);					// ������Ϣ
	virtual BOOL Recv(SOCKET &scoket, char *const data, int len, int &size);			// ������Ϣ

	BOOL Connect();																		// ���ӷ�����

	SOCKET GetClientSocket() { return m_sClient; }										// ��ȡ�������ؼ���

protected:
private:
	SOCKET m_sClient;					// �ͻ����׽���
	SOCKADDR_IN m_sClientAddr;			// �׽��ֵ�ַ
};

#endif