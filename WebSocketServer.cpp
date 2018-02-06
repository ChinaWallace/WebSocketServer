// WebSocketServer.cpp : 定义控制台应用程序的入口点。
//

#include <windows.h>
#include <vector>
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
	char data[4096];
	int recvSize;

	std::vector<char> recvDatas;

	while (bLoop)
	{
		memset(data, 0, 4096 * sizeof(char));
		BOOL recResult = m_pWebServer->Recv(comm, data, sizeof(data), recvSize);

		if (!recResult)
		{
			m_pWebServer->GetScoketError();
			break;
		}

		Websocket_Request request;
		recvDatas.insert(recvDatas.end(), data, data + recvSize);
		request.fetch_websocket_info(recvDatas);
		auto resultStr = request.getDataStr();
	}
}
