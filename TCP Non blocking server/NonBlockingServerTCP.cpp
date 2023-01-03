#include "NonBlockingServerTCP.h"


bool NonBlockingServerTCP::addSocket(SOCKET id, int what)
{
	unsigned long flag = 1;
	if (ioctlsocket(id, FIONBIO, &flag) != 0)
	{
		cout << "Http Server: Error at ioctlsocket(): " << WSAGetLastError() << endl;
	}

	for (int i = 0; i < MAX_SOCKETS; i++)
	{
		if (sockets[i].recv == EMPTY)
		{
			sockets[i].id = id;
			sockets[i].recv = what;
			sockets[i].send = IDLE;
			sockets[i].len = 0;
			sockets[i].timeOfLastMsg = time(0);
			socketsCount++;
			return (true);
		}
	}
	return (false);
}

void NonBlockingServerTCP::removeSocket(int index)
{
	sockets[index].recv = EMPTY;
	sockets[index].send = EMPTY;
	socketsCount--;
}

void NonBlockingServerTCP::acceptConnection(int index)
{
	SOCKET id = sockets[index].id;
	struct sockaddr_in from;
	int fromLen = sizeof(from);

	SOCKET msgSocket = accept(id, (struct sockaddr*)&from, &fromLen);
	if (INVALID_SOCKET == msgSocket)
	{
		cout << "HTTP Server: Error at accept(): " << WSAGetLastError() << endl;
		return;
	}
	cout << "HTTP Server: Client " << inet_ntoa(from.sin_addr) << ":" << ntohs(from.sin_port) << " is connected." << endl;

	if (addSocket(msgSocket, RECEIVE) == false)
	{
		cout << "\t\tToo many connections, dropped!\n";
		closesocket(id);
	}
	return;

}

void NonBlockingServerTCP::receiveMessage(int index)
{
	SOCKET msgSocket = sockets[index].id;

	int len = sockets[index].len;
	int bytesRecv = recv(msgSocket, &sockets[index].buffer[len], sizeof(sockets[index].buffer) - len, 0);

	if (SOCKET_ERROR == bytesRecv)
	{
		cout << "HTTP Server: Error at recv(): " << WSAGetLastError() << endl;
		closesocket(msgSocket);
		removeSocket(index);
		return;
	}
	if (bytesRecv == 0)
	{
		closesocket(msgSocket);
		removeSocket(index);
		return;
	}
	else
	{
		sockets[index].buffer[len + bytesRecv] = '\0';
		cout << "HTTP Server: Recieved: " << bytesRecv << " bytes of \"" << &sockets[index].buffer[len] << "\" message.\n";
		sockets[index].len += bytesRecv;
		sockets[index].timeOfLastMsg = time(0);

		if (sockets[index].len > 0)
		{
			if (strncmp(sockets[index].buffer, "Exit", 4) == 0)
			{
				closesocket(msgSocket);
				removeSocket(index);
				return;
			}
			else
			{
				sockets[index].send = SEND;
			}
		}
	}
}

void NonBlockingServerTCP::sendMessage(int index)
{
	int bytesSent = 0;
	char sendBuff[MAX_BUFFER_SIZE];
	SOCKET msgSocket = sockets[index].id;
	HTTPRequestsResponder::HTTPRequestInfo request_info = getRequestInfoFromSocket(index);
	string HTTP_response = handleRequest(request_info);
	strcpy(sendBuff, HTTP_response.c_str());

	bytesSent = send(msgSocket, sendBuff, (int)strlen(sendBuff), 0);
	if (SOCKET_ERROR == bytesSent)
	{
		cout << "HTTP Server: Error at send(): " << WSAGetLastError() << endl;
		return;
	}

	cout << "HTTP Server: Sent: " << bytesSent << "\\" << strlen(sendBuff) << " bytes of \"" << sendBuff << "\" message.\n";

	if (sockets[index].len == 0)
		sockets[index].send = IDLE;
}

void NonBlockingServerTCP::acceptAndHandleConnections(SOCKET listenSocket)
{
	while (true)
	{
		fd_set waitRecv;
		FD_ZERO(&waitRecv);
		for (int i = 0; i < MAX_SOCKETS; i++)
		{
			if ((sockets[i].recv == LISTEN) || (sockets[i].recv == RECEIVE))
				FD_SET(sockets[i].id, &waitRecv);
		}

		fd_set waitSend;
		FD_ZERO(&waitSend);
		for (int i = 0; i < MAX_SOCKETS; i++)
		{
			if (sockets[i].send == SEND)
				FD_SET(sockets[i].id, &waitSend);
		}

		int numberOfReadySockets = select(0, &waitRecv, &waitSend, NULL, NULL);
		if (numberOfReadySockets == SOCKET_ERROR)
		{
			cout << "HTTP Server: Error at select(): " << WSAGetLastError() << endl;
			WSACleanup();
			return;
		}

		for (int i = 0; i < MAX_SOCKETS && numberOfReadySockets > 0; i++)
		{
			if (FD_ISSET(sockets[i].id, &waitRecv))
			{
				numberOfReadySockets--;
				switch (sockets[i].recv)
				{
				case LISTEN:
					acceptConnection(i);
					break;

				case RECEIVE:
					receiveMessage(i);
					break;
				}
			}
		}

		for (int i = 0; i < MAX_SOCKETS && numberOfReadySockets > 0; i++)
		{
			if (FD_ISSET(sockets[i].id, &waitSend))
			{
				numberOfReadySockets--;
				if (sockets[i].send == SEND)
				{
					sendMessage(i);
					break;
				}
			}
		}

		checkAndHandleTimeOut();
	}
}

void NonBlockingServerTCP::initListenSocketAndHandleErrors(SOCKET& listenSocket, int port, bool& retflag)
{
	retflag = true;
	listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (INVALID_SOCKET == listenSocket)
	{
		cout << "HTTP Server: Error at socket(): " << WSAGetLastError() << endl;
		WSACleanup();
		return;
	}

	sockaddr_in serverService;
	serverService.sin_family = AF_INET;
	serverService.sin_addr.s_addr = INADDR_ANY;
	serverService.sin_port = htons(port);


	if (SOCKET_ERROR == bind(listenSocket, (SOCKADDR*)&serverService, sizeof(serverService)))
	{
		cout << "HTTP Server: Error at bind(): " << WSAGetLastError() << endl;
		closesocket(listenSocket);
		WSACleanup();
		return;
	}

	if (SOCKET_ERROR == listen(listenSocket, 5))
	{
		cout << "HTTP Server: Error at listen(): " << WSAGetLastError() << endl;
		closesocket(listenSocket);
		WSACleanup();
		return;
	}
	retflag = false;
}

void NonBlockingServerTCP::RunServer(int port)
{
	cout << "HTTP non-blocking TCP server is on the Air" << endl;

	WSAData wsaData;

	if (NO_ERROR != WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		cout << "HTTP Server: Error at WSAStartup()" << endl;
		return;
	}

	SOCKET listenSocket;
	bool retflag;

	initListenSocketAndHandleErrors(listenSocket, port, retflag);
	if (retflag) return;

	addSocket(listenSocket, LISTEN);

	acceptAndHandleConnections(listenSocket);

	cout << "HTTP Server: Closing Connection." << endl;
	closesocket(listenSocket);
	WSACleanup();
}

void NonBlockingServerTCP::checkAndHandleTimeOut()
{
	for (int i = 1; i < socketsCount; i++)
	{
		int timeSinceLastMsg = difftime(time(0), sockets[i].timeOfLastMsg);
		if (timeSinceLastMsg > TIME_LIMIT)
		{
			cout << "deleteing socket :" << i << " time: " << timeSinceLastMsg << endl;
			closesocket(sockets[i].id);
			removeSocket(i);
		}
	}
}

string NonBlockingServerTCP::handleRequest(HTTPRequestsResponder::HTTPRequestInfo request_info)
{
	return HTTPRequestsResponder::do_request_and_generate_http_response(request_info);
}

HTTPRequestsResponder::HTTPRequestInfo NonBlockingServerTCP::getRequestInfoFromSocket(int index)
{
	// get first request in buffer
	string full_request(sockets[index].buffer);

	// move buffer to the left
	memcpy(sockets[index].buffer, &sockets[index].buffer[full_request.size()], sockets[index].len - full_request.size());
	sockets[index].len -= full_request.size();

	// get the HTTP fields from the string request
	string request_type_as_string = ""; 
	string request_without_type = ""; 
	string endpoint = "";
	string query_params = "";
	string headers = "";
	string body = "";

	if (full_request.find(' ') != string::npos) {
		request_type_as_string = full_request.substr(0, full_request.find_first_of(' '));
		request_without_type = full_request.substr(full_request.find(' ') + 1);
	}

	if (request_without_type.find('?') != string::npos)
		endpoint = request_without_type.substr(0, request_without_type.find('?'));
	else if (request_without_type.find(' ') != string::npos)
		endpoint = request_without_type.substr(0, request_without_type.find(' '));

	if (request_without_type.find('?') != string::npos && request_without_type.find(' ') != string::npos)
		query_params = request_without_type.substr(request_without_type.find('?') + 1, request_without_type.find(' ') - (request_without_type.find('?') + 1));

	if (request_without_type.find(' ') != string::npos && request_without_type.find('\r\n\r\n') != string::npos)
		headers = request_without_type.substr(request_without_type.find(' ') + 1, request_without_type.find("\r\n\r\n") - (request_without_type.find(' ') + 1));

	if(request_without_type.find("\r\n\r\n") != string::npos)
		body = request_without_type.substr(request_without_type.find("\r\n\r\n") + 4);

	// create RequestInfo with the info
	HTTPRequestsResponder::HTTPRequestInfo request_info;
	request_info.full_request = full_request;
	request_info.endpoint = endpoint;
	request_info.query_params = query_params;
	request_info.headers = headers;
	request_info.body = body;

	if (request_type_as_string == "GET")
		request_info.request_type = HTTPRequestsResponder::eRequestType::GET;
	else if (request_type_as_string == "POST")
		request_info.request_type = HTTPRequestsResponder::eRequestType::POST;
	else if (request_type_as_string == "HEAD")
		request_info.request_type = HTTPRequestsResponder::eRequestType::HEAD;
	else if (request_type_as_string == "PUT")
		request_info.request_type = HTTPRequestsResponder::eRequestType::PUT;
	else if (request_type_as_string == "TRACE")
		request_info.request_type = HTTPRequestsResponder::eRequestType::TRACE;
	else if (request_type_as_string == "DELETE")
		request_info.request_type = HTTPRequestsResponder::eRequestType::DEL;
	else if (request_type_as_string == "OPTIONS")
		request_info.request_type = HTTPRequestsResponder::eRequestType::OPTIONS;
	else
		request_info.request_type = HTTPRequestsResponder::eRequestType::ILEGALREQUEST;

	return request_info;
}
