#include "swpacket.h"


void *InitPacketQueue()
{
	SWPacketQueue *ppacketqueue = NULL;
	ppacketqueue = (SWPacketQueue *)malloc(sizeof(SWPacketQueue));
	
	if (ppacketqueue == NULL)
	{
		return NULL;
	}
	memset(ppacketqueue, 0, sizeof(SWPacketQueue));
	ppacketqueue->front = NULL;
	ppacketqueue->rear = NULL;
	ppacketqueue->size = 0;

	return ppacketqueue;
}

bool ReleasePacketQueue(void *packetqueue)
{
	SWPacketQueue *ppacketqueue = (SWPacketQueue *)packetqueue;
	
	if (ppacketqueue != NULL)
	{	
		pthread_mutex_lock(&ppacketqueue->mutex);
		
		while (ppacketqueue->front) 			
		{
			ppacketqueue->rear = ppacketqueue->front->next;
			free(ppacketqueue->front);
			ppacketqueue->front = NULL;
			ppacketqueue->front = ppacketqueue->rear;
  		}
  		
	pthread_mutex_unlock(&ppacketqueue->mutex);
	free(ppacketqueue);
	
	}
	
	return true;
}

bool PacketQueueEmpty(void *packetqueue)
{
	SWPacketQueue *ppacketqueue = (SWPacketQueue *)packetqueue;
	
	if (ppacketqueue && ppacketqueue->size != 0)
	{
		return false;
	}
	
	return true;
}

bool PacketQueuePut(void *packetqueue, void *data)
{
	SWPacketQueue *ppacketqueue = (SWPacketQueue *)packetqueue;
	
	if (!ppacketqueue || !data)
	{
		return false;
	}
	SWPacket *ppacket = (SWPacket *)data;
	
	pthread_mutex_lock(&ppacketqueue->mutex);
	
	if (ppacketqueue->size == 0)
	{			
		ppacketqueue->front = ppacket;
		ppacketqueue->rear = ppacket;
	}
	else
	{
		ppacketqueue->rear->next = ppacket;
		ppacketqueue->rear = ppacket;		
	}
		
	ppacketqueue->size++;
	pthread_mutex_unlock(&ppacketqueue->mutex);
	
	return true;
	
}

void *PacketQueueGet(void *packetqueue)
{
	SWPacketQueue *ppacketqueue = (SWPacketQueue *)packetqueue;
	
	if (!ppacketqueue)
	{
		return NULL;
	}
	
	SWPacket *ppacket = NULL;
	pthread_mutex_lock(&ppacketqueue->mutex);
	
	if (ppacketqueue->size == 0)
	{	
		pthread_mutex_unlock(&ppacketqueue->mutex);
		return NULL;
	}
	else
	{	
		ppacketqueue->size--;
		ppacket = ppacketqueue->front;
		
		if (ppacketqueue->size == 0)
		{
			ppacketqueue->front = NULL;
			ppacketqueue->rear = NULL;
		}
		else
		{
			ppacketqueue->front = ppacketqueue->front->next;
		}
	}
	
	pthread_mutex_unlock(&ppacketqueue->mutex);
	return ppacket;
}
