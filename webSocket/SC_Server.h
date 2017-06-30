#ifndef _WEB_COMMAND_H_
#define _WEB_COMMAND_H_

#include "ScoketComm.h"

#include <windows.h>
#include <string.h>
#include <thread>
#include <functional>
#include <iostream>

#ifdef _UNICODE
typedef std::wstring tstring;
#define tcout std::wcout
#define tcin std::wcin
#else
typedef std::string tstring;
#define tcout std::cout
#define tcin std::cin
#endif

using namespace std;

enum ScoketType
{
	Normal,
	Web,
	ActionScript,
};

//typedef void (*CallBackComm)(SOCKET comm);
typedef std::function<void (SOCKET comm)> CallBackComm;

class ServerSC : public ScoketComm
{
public:
	ServerSC(void) : m_pServer(NULL) {}
	~ServerSC(void);

	/************************************************************************/
	/* ����
	* @return ������ɵ�ServerSC����
	* @param pCallBack �ص�ͨ�ź���
	* @param port �˿ں�
	* @param ip IP��ַ
	* @param resolve_ip ������ַ
	* @param type Scoket����
	* // [6/24/2016 Wings] */
	/************************************************************************/
	bool Create(CallBackComm pCallBack, const int port, const char *ip = NULL, 
		BOOL resolve_ip = FALSE, ScoketType type = Normal);

	/************************************************************************/
	/* ������Ϣ
	* // [7/18/2016 Wings] */
	/************************************************************************/
	virtual BOOL Send(SOCKET &scoket, const char *data, int len);
	/************************************************************************/
	/* ������Ϣ
	* // [7/18/2016 Wings] */
	/************************************************************************/
	virtual BOOL Recv(SOCKET &scoket, char *const data, int len);

	/************************************************************************/
	/* ����Web
	* @param comm ͨ���׽���
	* // [7/15/2016 Wings] */
	/************************************************************************/
	bool HandshakeWeb(SOCKET comm);
protected:
private:
	/************************************************************************/
	/* �ȴ�����ͨ������
	* @param server �������׽���
	* // [7/15/2016 Wings] */
	/************************************************************************/
	void Accept(SOCKET server, ScoketType type);

	/************************************************************************/
	/* ����
	* @param comm ͨ���׽���
	* // [7/15/2016 Wings] */
	/************************************************************************/
	bool Handshake(SOCKET comm, ScoketType type);

	

	/************************************************************************/
	/* ����ActionScript
	* @param comm ͨ���׽���
	* // [7/15/2016 Wings] */
	/************************************************************************/
	bool HandshakeActionScript(SOCKET comm);

	/************************************************************************/
	/* ͨ��
	* @param comm ͨ���׽���
	* // [7/15/2016 Wings] */
	/************************************************************************/
	void Command(SOCKET comm);

	/************************************************************************/
	/* ����
	* @return ���ܽ��
	* // [6/24/2016 Wings] */
	/************************************************************************/
	char* HashString(std::string szMsg);

	SOCKET m_sServer; // �������׽��� [6/29/2016 Wings]

	ScoketServerComm * m_pServer;

	CallBackComm m_fCallBack; // �ص�ͨ�ź��� [10/13/2016 Wings]
};

#endif