#ifndef	_SWSERVER_H
#define _SWSERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdbool.h>

typedef struct _SWServer
{
	struct sockaddr_in sin;
	int sfd;
	pthread_t thread;
	
}SWServer;

void *InitServer(void *packetlink, int port);

void *RecvData(void *server);

bool StartServer(void *server);

bool StopServer(void *server);

bool ReleaseServer(void *server);

#endif /* _SWSERVER_H */
