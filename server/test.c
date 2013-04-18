

#include "swaviwrite.h"
#include "swpacket.h"
#include "swserver.h"
#include <signal.h>

#define PORT 8888

void handle_signal(int sig);
void main_exit();

static SWPacketQueue *pPacketQueue = NULL;
static SWAviWrite *pWrite = NULL;
static SWServer *pMServer = NULL;
static bool flag = true;
static int count = 0;
int main()
{	
	signal(SIGHUP, handle_signal);
	signal(SIGINT, handle_signal);
	signal(SIGQUIT, handle_signal);

	pPacketQueue = (SWPacketQueue *)InitPacketQueue();
	
	if (!pPacketQueue)
	{
		exit(1);
	}
	
	pMServer = InitServer(pPacketQueue, PORT);
	
	pWrite = InitAviWrite(pPacketQueue, "test.asf");
	
	
	StartServer(pMServer);
    StartThread(pWrite);
    
    
	while(flag);
    main_exit();	
	return 0;
}


void handle_signal(int sig)
{
	switch(sig)
	{
		case 1:
			printf("signal:SIGHUP\n");
			break;
		case 2:
			printf("signal:SIGINI\n");
			flag = false;
			count++;
			if (count == 2)
			{
				exit(1);
			}
			//main_exit();
			break;
		default:
		printf("signal:%d\n",sig);
		break;
	}
}

void main_exit()
{	
	StopServer(pMServer);
	pPacketQueue = NULL;
	pWrite = NULL;

}
