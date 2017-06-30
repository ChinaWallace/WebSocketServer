// WebSocketServer.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <windows.h>
#include "webSocket/SC_Server.h"
#include "webSocket/websocket_request.h"

ServerSC* m_pWebServer;
Websocket_Request* request_;// webSocket返回数据解析 

void createWebServer();
void WebServerCommand(SOCKET comm);

int main()
{
	createWebServer();

	while (true)
	{
		continue;
	}
    return 0;
}

void createWebServer() {

	request_ = new Websocket_Request();
	m_pWebServer = new ServerSC();

	//非线程阻塞
	auto callback = std::bind(&WebServerCommand, std::placeholders::_1);
	if (!m_pWebServer->Create(callback, 55570, "127.0.0.1", FALSE, Web))
	{
		
	}
}

void WebServerCommand(SOCKET comm)
{
	bool bLoop = true;
	while (bLoop)
	{
		char data[4096];
		memset(data, 0, 4096 * sizeof(char));
		if (!m_pWebServer->Recv(comm, data, sizeof(data)))
		{
			m_pWebServer->GetScoketError();
			break;
		}
		request_->fetch_websocket_info(data);
		char* result = request_->getPayload();
		request_->reset();
	}
}
