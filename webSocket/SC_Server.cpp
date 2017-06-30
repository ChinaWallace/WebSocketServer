#define  _CRT_SECURE_NO_WARNINGS
#include "SC_Server.h"

#include "SHA1.h"
#include "base64.h"
#include <string.h>
#include <stdio.h>
#include <regex>

ServerSC::~ServerSC(void)
{
	if (m_pServer != NULL)
	{
		delete m_pServer;
	}
}

bool ServerSC::Create(CallBackComm pCallBack, const int port, const char *ip /* = NULL */, 
	BOOL resolve_ip /* = FALSE */, ScoketType type /* = Normal */)
{
	if (!CreateSocket(&m_sServer))
	{
		//MessageBoxW(NULL, L"服务器创建失败", L"ScoketServerComm", MB_ICONERROR);
		return false;
	}

	SOCKADDR_IN sServerAddr;
	if (!AppointSocketAddr(&sServerAddr, port, ip, resolve_ip))
	{
		//MessageBoxW(NULL, L"服务器域名解析失败", L"ScoketServerComm", MB_ICONERROR);
		return false;
	}

	if (!SetServer(m_sServer, &sServerAddr, 5))
	{
		//MessageBoxW(NULL, L"服务器监听失败", L"ScoketServerComm", MB_ICONERROR);
		return false;
	}

	m_fCallBack = pCallBack;

	std::thread tAccept(&ServerSC::Accept, this, m_sServer, type);
	tAccept.detach();

	ChangeScoketError(ScoketNotError);

	return true;
}

void ServerSC::Accept(SOCKET server, ScoketType type)
{
	while (true)
	{
		SOCKADDR_IN addrClient;
		int iLen = sizeof(SOCKADDR);

		//accept函数是阻塞的，有了连接才往下走
		SOCKET sComm = accept(server, (SOCKADDR *)&addrClient, &iLen);
		if (sComm == INVALID_SOCKET)
		{
			ChangeScoketError(ServerBindError);
			break;
		}

		if (!Handshake(sComm, type))
		{
			continue;
		}

		std::thread tCommand(&ServerSC::Command, this, sComm);
		tCommand.detach();
	}
}

bool ServerSC::Handshake(SOCKET comm, ScoketType type)
{
	switch (type)
	{
	case Normal:
		return true;
	case Web:
		return HandshakeWeb(comm);
	case ActionScript:
		return HandshakeActionScript(comm);
	default:
		return true;
	}
}

bool ServerSC::HandshakeWeb(SOCKET comm)
{
	//发送握手信息
	const char *key1 = NULL, *origin = NULL, *resource = NULL;
	char *location = (char*)calloc(0x40, sizeof(char));
	char requestbuf[BUFSIZ];
	unsigned char responsebuf[BUFSIZ];
	memset(requestbuf, 0, BUFSIZ);
	memset(responsebuf, 0, BUFSIZ);

	if (!Recv(comm, requestbuf, sizeof(requestbuf)))
	{
		return false;
	}

	char* key1pattern = "(Sec-WebSocket-Key:)[[:s:]](.+\\r\\n)";
	char* resourcePattern = "(GET)[[:s:]](/[[:alnum:]]+)";
	char* originPattern = "(Origin:)[[:s:]](.+)\\r\\n";

	std::match_results<const char*> m;
	std::tr1::regex rx;
	std::string s;

	//match Sec-WebSocket-Key1 
	m = std::match_results<const char*>();
	rx = std::tr1::regex(key1pattern);
	std::tr1::regex_search(requestbuf, m, rx);
	s = m[2];
	std::string strNew = s.substr(0, s.length() - 2);
	key1 = _strdup(strNew.c_str());

	//match origin
	m = std::match_results<const char*>();
	rx = std::tr1::regex(originPattern);
	std::tr1::regex_search(requestbuf, m, rx);
	s = m[2];
	origin = _strdup(s.c_str());

	//match GET (resource)
	m = std::match_results<const char*>();
	rx = std::tr1::regex(resourcePattern);
	std::tr1::regex_search(requestbuf, m, rx);
	s = m[2];
	resource = _strdup(s.c_str());

	if (key1 == NULL || resource == NULL)
	{
		MessageBoxW(NULL, L"握手数据错误", L"ServerSC", MB_ICONERROR);
		return false;
	}

	unsigned char serv_buf[BUFSIZ];
	memset(serv_buf, 0, BUFSIZ);
	char* hash;
	int n = sprintf((char *)serv_buf, "%s258EAFA5-E914-47DA-95CA-C5AB0DC85B11", key1);
	std::string strRaw = (char *)serv_buf;

	hash = HashString(strRaw);

	int pt_serv_buf_size = 0;
	std::string strAccept = base64_encode((unsigned char*)hash, 20);
	if (strAccept.size() < 0)
	{
		MessageBoxW(NULL, L"SHA1加密错误", L"ServerSC", MB_ICONERROR);
		return false;
	}

	char* handshake = (char*)calloc(BUFSIZ, sizeof(char));
	char * handshakeFormat = "HTTP/1.1 101 Switching Protocols\r\n"
		"Upgrade: WebSocket\r\n"
		"Connection: Upgrade\r\n"
		"Sec-WebSocket-Accept: %s\r\n"
		"\r\n";
	sprintf(handshake, handshakeFormat, strAccept.c_str());

	for (int i = 0; i < strlen(handshake); i++)
	{
		responsebuf[i] = handshake[i];
	}

	if (!Send(comm, (char*)responsebuf, 129))
	{
		return false;
	}

	return true;
}

bool ServerSC::HandshakeActionScript(SOCKET comm)
{
	return true;
}

void ServerSC::Command(SOCKET comm)
{
	m_fCallBack(comm);

	CloseSocket(&comm);
}

BOOL ServerSC::Send(SOCKET &scoket, const char *data, int len)
{
	if (GetScoketError() != ScoketNotError)
	{
		CloseSocket(&scoket);
		return FALSE;
	}

	if (send(scoket, data, len, 0) == SOCKET_ERROR)
	{
		CloseSocket(&scoket);
		ChangeScoketError(SendError);
		return FALSE;
	}

	return TRUE;
}

BOOL ServerSC::Recv(SOCKET &scoket, char *const data, int len)
{
	int a = GetScoketError();
	if (GetScoketError() != ScoketNotError)
	{
		CloseSocket(&scoket);
		return FALSE;
	}

	int recv_error = recv(scoket, data, len, 0);
	if (recv_error == SOCKET_ERROR || recv_error == 0)
	{
		CloseSocket(&scoket);
		ChangeScoketError(RecvError);
		return FALSE;
	}

	return TRUE;
}

//SHA1加密
char* ServerSC::HashString(std::string szMsg)
{
	LPCSTR pszSrc = szMsg.c_str();
	int nLen = MultiByteToWideChar(CP_ACP, 0, pszSrc, -1, NULL, 0);
	if (nLen == 0)
	{
		return "";
	}
	wchar_t* pwszDst = new wchar_t[nLen];
	if (!pwszDst)
	{
		return "";
	}
	MultiByteToWideChar(CP_ACP, 0, pszSrc, -1, pwszDst, nLen);
	std::wstring wstr(pwszDst);
	delete[] pwszDst;
	pwszDst = NULL;

	tstring str = wstr;

	CSHA1 sha1;
	tstring strReport;

#ifdef _UNICODE
	const size_t uAnsiLen = wcstombs(NULL, str.c_str(), 0) + 1;
	char* pszAnsi = new char[uAnsiLen + 1];
	wcstombs(pszAnsi, str.c_str(), uAnsiLen);

	sha1.Update((UINT_8*)&pszAnsi[0], strlen(&pszAnsi[0]));
	sha1.Final();
	char* szHash = new char[20];
	for (int i = 0; i < 20; ++i)
	{
		szHash[i] = sha1.m_digest[i];
	}

	delete[] pszAnsi;
	sha1.Reset();

#else
	sha1.Update((UINT_8*)str.c_str(), str.size() * sizeof(TCHAR));
	sha1.Final();
	sha1.ReportHashStl(strReport, CSHA1::REPORT_HEX_SHORT);
	tcout << _T("String hashed to:") << endl;
	tcout << strReport << endl;
#endif
	return szHash;
}