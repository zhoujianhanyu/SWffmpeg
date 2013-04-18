
#include "swclient.h"
#include "swpacket.h"

#define     PORT 8888
#define     MAX_LEN 4096

static bool quit = false;
static bool running = false;

SWPacketQueue *pCPacketQueue = NULL;

void *InitClient(void *packetqueue, char *addr)
{	
	if (!packetqueue)
	{		
		return NULL;
	}
	pCPacketQueue = (SWPacketQueue *)packetqueue;
	SWClient *pclient = (SWClient *)malloc(sizeof(SWClient));
	
	if (!pclient)
	{
		return NULL;
	}
	bzero((void *)&pclient->sin,sizeof(pclient->sin));
	pclient->sin.sin_family = AF_INET;
	inet_pton(AF_INET,addr,(void *)&(pclient->sin.sin_addr));
	pclient->sin.sin_port = htons(PORT);
	pclient->sfd = socket(AF_INET,SOCK_DGRAM,0);
	
	if (pclient->sfd < 0)
	{	
		printf("socket error\n");
		free(pclient);
		pclient = NULL;
		return NULL;
	}
	
	return pclient;
}

void *SendData(void *client)
{
	//send data
	SWClient *pclient = (SWClient *)client;
	if (!pclient)
	{
		return false;
	}
	quit = false;
	running = true;
	SWPacket *packet = NULL;
	int slen,blen;
	int size;
	while(!quit)
	{
		packet = (SWPacket *)PacketQueueGet(pCPacketQueue);
		if (!packet)
		{
			usleep(50);
			continue;
		}
		
		size = (int)packet->size;
		blen = sendto(pclient->sfd,&size,sizeof(int),0,
						(struct sockaddr *)&pclient->sin,
						sizeof(pclient->sin));
		printf("send size %d\n",size);
		if (blen < 0 || size == -1)
		{
			break;
		}
		slen = 0;
		blen = 0;
		while(size > 0)
		{
			blen = sendto(pclient->sfd,packet+slen,size,0,
						(struct sockaddr *)&pclient->sin,
						sizeof(pclient->sin));
			size -= blen;
			slen += slen;
			
		}	
		free(packet);
		packet = NULL;	
	}
	if (packet)
	{
		free(packet);
		packet = NULL;
	}		
	running = false;
	ReleaseClient(pclient);
	printf("send end\n");
}

bool StartClient(void *client)
{
	SWClient *pclient = (SWClient *)client;
	
	if (!pclient || pthread_create(&pclient->thread, NULL, 
						&SendData, pclient) != 0)
	{		
		return false;
	}
	return true;

}

bool StopClient(void *client)
{
	SWClient *pclient = (SWClient *)client;
	if (!pclient)
	{
		return false;
	}
	
	quit = true;
	pthread_join(pclient->thread, NULL);
	while (running)
	{
		usleep(200);
	}

	return true;	
}

bool ReleaseClient(void *client)
{
	SWClient *pclient = (SWClient *)client;
		
	if (!pclient)
	{
		return false;
	}
	close(pclient->sfd);
	free(pclient);
}

