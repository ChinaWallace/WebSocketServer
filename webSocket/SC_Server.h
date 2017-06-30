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
	/* 创建
	* @return 创建完成的ServerSC对象
	* @param pCallBack 回调通信函数
	* @param port 端口号
	* @param ip IP地址
	* @param resolve_ip 解析地址
	* @param type Scoket类型
	* // [6/24/2016 Wings] */
	/************************************************************************/
	bool Create(CallBackComm pCallBack, const int port, const char *ip = NULL, 
		BOOL resolve_ip = FALSE, ScoketType type = Normal);

	/************************************************************************/
	/* 发送消息
	* // [7/18/2016 Wings] */
	/************************************************************************/
	virtual BOOL Send(SOCKET &scoket, const char *data, int len);
	/************************************************************************/
	/* 接收消息
	* // [7/18/2016 Wings] */
	/************************************************************************/
	virtual BOOL Recv(SOCKET &scoket, char *const data, int len);

	/************************************************************************/
	/* 握手Web
	* @param comm 通信套接字
	* // [7/15/2016 Wings] */
	/************************************************************************/
	bool HandshakeWeb(SOCKET comm);
protected:
private:
	/************************************************************************/
	/* 等待接受通信连接
	* @param server 服务器套接字
	* // [7/15/2016 Wings] */
	/************************************************************************/
	void Accept(SOCKET server, ScoketType type);

	/************************************************************************/
	/* 握手
	* @param comm 通信套接字
	* // [7/15/2016 Wings] */
	/************************************************************************/
	bool Handshake(SOCKET comm, ScoketType type);

	

	/************************************************************************/
	/* 握手ActionScript
	* @param comm 通信套接字
	* // [7/15/2016 Wings] */
	/************************************************************************/
	bool HandshakeActionScript(SOCKET comm);

	/************************************************************************/
	/* 通信
	* @param comm 通信套接字
	* // [7/15/2016 Wings] */
	/************************************************************************/
	void Command(SOCKET comm);

	/************************************************************************/
	/* 加密
	* @return 解密结果
	* // [6/24/2016 Wings] */
	/************************************************************************/
	char* HashString(std::string szMsg);

	SOCKET m_sServer; // 服务器套接字 [6/29/2016 Wings]

	ScoketServerComm * m_pServer;

	CallBackComm m_fCallBack; // 回调通信函数 [10/13/2016 Wings]
};

#endif