
#include "swserver.h"
#include "swpacket.h"

SWPacketQueue *pSPacketQueue = NULL;
static bool running = false;
static bool quit = false;

void *InitServer(void *packetqueue, int port)
{
	if (!packetqueue)
	{
		return NULL;
	}
	
	if (port < 1024)
	{
		port = 8888;
	}
	
	pSPacketQueue = (SWPacketQueue *)packetqueue;
	SWServer *pserver = (SWServer *)malloc(sizeof(SWServer));
	if (!pserver)
	{
		return NULL;
	}
	bzero(&pserver->sin,sizeof(pserver->sin));
 	pserver->sin.sin_family = AF_INET;
 	pserver->sin.sin_addr.s_addr=htonl(INADDR_ANY);
 	pserver->sin.sin_port = htons(port);
	pserver->sfd = socket(AF_INET,SOCK_DGRAM,0);
	if (pserver->sfd < 0)
	{
		free(pserver);
		pserver = NULL;
		return NULL;
	}
	
 	bind(pserver->sfd,(struct sockaddr *)&pserver->sin,sizeof(pserver->sin));
	
	quit = false;
	running = false;
	
	return pserver;
}

void *RecvData(void *server)
{
	SWServer *pserver = (SWServer *)server;
		
	if (!pserver)
	{
		return false;
	}
	quit = false;
	running = true;
	int alen;
	SWPacket *packet = NULL;
	int ret;
	int size;
	int blen;
	int sin_len = sizeof(pserver->sin);
	char buf_size[4];
	uint8_t *buf;
	while(!quit)
	{
		memset(buf_size,0,sizeof(int));
		ret = recvfrom(pserver->sfd,buf_size,sizeof(int),0,
				(struct sockaddr *)&(pserver->sin),
				&sin_len);
		memcpy(&size,buf_size,sizeof(int));
		printf("recv size %d\n",size);	
		if (ret > 0)
		{	
			if (size <= 0)
			{
				break;
			}

			packet = (SWPacket *)malloc(size);
			memset(packet,0,size);
			if (!packet)
			{
				break;
			}
			blen = 0;
			while(size > 0)
			{
				ret = recvfrom(pserver->sfd,packet+blen,size,0,
								(struct sockaddr *)&pserver->sin,
								&sin_len);
				blen += ret;
				size -= ret;
			}
			PacketQueuePut(pSPacketQueue, packet);
		}
	
	}
	running = false;
	packet = (SWPacket *)malloc(sizeof(SWPacket));
	memset(packet, 0 ,sizeof(SWPacket));
	packet->stream_index = -1;
	PacketQueuePut(pSPacketQueue,packet);
	ReleaseServer(pserver);
	printf("recv end\n");
}

bool StartServer(void *server)
{
	SWServer *pserver = (SWServer *)server;
		
	if (!pserver)
	{
		return false;
	}
	if (pthread_create(&pserver->thread, NULL, 
						&RecvData, pserver) != 0)
	{
		return false;
	}
	return true;

}

bool StopServer(void *server)
{
	SWServer *pserver = (SWServer *)server;
	if (!pserver)
	{	
		return false;
	}
	quit = true;
	
	pthread_join(pserver->thread, NULL);
	while (running)
	{
		usleep(200);
	}
	return true;
}

bool ReleaseServer(void *server)
{
	SWServer *pserver = (SWServer *)server;
		
	if (!pserver)
	{
		return false;
	}
	close(pserver->sfd);
	free(pserver);
	pserver = NULL;

}
