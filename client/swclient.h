#ifndef _SWCLIENT_H
#define _SWCLIENT_H


#include	<stdio.h>
#include	<stdlib.h>
#include	<unistd.h>
#include	<sys/types.h>
#include	<sys/socket.h>
#include	<arpa/inet.h>
#include	<string.h>
#include    <pthread.h>
#include    <stdbool.h>

typedef struct _SWClient
{
	struct sockaddr_in sin;
	int sfd;
	pthread_t thread;

}SWClient;

void *InitClient(void *packetqueue, char *addr);
bool StartClient(void *client);
bool StopClient(void *client);
bool ReleaseClient(void *client);


#endif /* _SWCLIENT_H */
