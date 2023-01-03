#include"NonBlockingServerTCP.h"

void main()
{
	NonBlockingServerTCP Server;
	Server.RunServer(8080);
}