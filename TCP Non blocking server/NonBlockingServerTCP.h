#pragma once
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
using namespace std;
#pragma comment(lib, "Ws2_32.lib")
#include <winsock2.h>
#include <string.h>
#include "HTTPRequestsResponder.h"

class NonBlockingServerTCP
{
private:
	const static int MAX_SOCKETS = 60;
	const static int EMPTY = 0;
	const static int LISTEN = 1;
	const static int RECEIVE = 2;
	const static int IDLE = 3;
	const static int SEND = 4;
	const static int MAX_BUFFER_SIZE = 2500;
	const static int TIME_LIMIT = 120;

	struct SocketState
	{
		SOCKET id;
		int	recv;
		int	send;
		int sendSubType;
		char buffer[MAX_BUFFER_SIZE];
		int len;
		time_t timeOfLastMsg;
	};

	int port;
	struct SocketState sockets[MAX_SOCKETS] = { 0 };
	int socketsCount = 0;

	bool addSocket(SOCKET id, int what);
	void removeSocket(int index);
	void acceptConnection(int index);
	void receiveMessage(int index);
	void sendMessage(int index);
	void acceptAndHandleConnections(SOCKET listenSocket);
	void initListenSocketAndHandleErrors(SOCKET& listenSocket, int port, bool& retflag);
	void checkAndHandleTimeOut();
	string handleRequest(HTTPRequestsResponder::HTTPRequestInfo request_info);
	HTTPRequestsResponder::HTTPRequestInfo getRequestInfoFromSocket(int index);

public:
	void RunServer(int port);

};

