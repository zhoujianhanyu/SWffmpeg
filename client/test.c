
#include "swclient.h"
#include "swpacket.h"
#include "swencodec.h"
#include "swqueue.h"

#include <time.h>
#include <signal.h>

void handle_signal(int sig);
void main_exit();

static SWRawQueue *pRawQueue = NULL;
static SWPacketQueue *pPacketQueue = NULL;

static SWEnContext *pEnContext = NULL;
static SWClient *pClient = NULL;
static bool flag = true;
int main()
{	
	signal(SIGHUP, handle_signal);
	signal(SIGINT, handle_signal);
	signal(SIGQUIT, handle_signal);
	pRawQueue = (SWRawQueue *)InitRawQueue();
	pPacketQueue = (SWPacketQueue *)InitPacketQueue();
	
	
	if (!pRawQueue || !pPacketQueue)
	{
		exit(1);
	}
	
	pEnContext = (SWEnContext *)InitEnContext(pRawQueue, pPacketQueue);
	pClient = (SWClient *)InitClient(pPacketQueue, "127.0.0.1");
	
	if (!pEnContext || !pClient)
	{
		exit(1);
	}
	
	AVFrame *vframe = avcodec_alloc_frame();
    AVFrame *aframe = avcodec_alloc_frame();
    uint16_t *samples;
    
    float t, tincr;
    int buffer_size;
    int ret;
    int frame_size = pEnContext->pACodecCtx->frame_size;
   
    aframe->nb_samples     = frame_size;
    aframe->format         = 8;
    aframe->channel_layout = 2;
    buffer_size = av_samples_get_buffer_size(NULL, 2, frame_size, 8, 0);

    samples = av_malloc(buffer_size);
    if (!samples) {
        fprintf(stderr, "Could not allocate %d bytes for samples buffer\n",
                buffer_size);
        exit(1);
    }
    /* setup the data pointers in the AVFrame */
    ret = avcodec_fill_audio_frame(aframe, 2, 8,
                                   (const uint8_t*)samples, buffer_size, 0);
    if (ret < 0) {
        fprintf(stderr, "Could not setup audio frame\n");
        exit(1);
    }
    
    vframe->format = AV_PIX_FMT_YUV420P;
    vframe->width  = 352;
    vframe->height = 288;
    
	
    ret = av_image_alloc(vframe->data, vframe->linesize, 
						pEnContext->pVCodecCtx->width, 
						pEnContext->pVCodecCtx->height,
						pEnContext->pVCodecCtx->pix_fmt, 32);
	if (ret < 0) 
	{
		fprintf(stderr, "Could not allocate raw picture buffer\n");
		exit(1);
	}
	
	if (!vframe) 
	{
        fprintf(stderr, "Could not allocate video frame\n");
        exit(1);
    }
    
    
	StartEnContext(pEnContext);
	StartClient(pClient);
	
	int x, y,i;
    for(i=0;i<5000;i++)
    {	
    	int j, k;
    	for (j = 0; j < frame_size; j++) {
            samples[2*j] = (int)(sin(t) * 10000);
            for (k = 1; k < 2; k++)
                samples[2*j + k] = samples[2*j];
            t += tincr;
        }
        
        time_t arawtime;
     	long atime = (long)time(&arawtime);
        
    	if (!RawQueuePut(pRawQueue, (void *)aframe, sizeof(*aframe), 1, atime+i))
        {	
        	break;
        }  
    	
    	for(y=0;y<pEnContext->pVCodecCtx->height;y++) 
        {
            for(x=0;x<pEnContext->pVCodecCtx->width;x++) 
            {
                vframe->data[0][y * vframe->linesize[0] + x] = x + y + i * 3;
            }
        }
        for(y=0;y<pEnContext->pVCodecCtx->height/2;y++) 
        {
            for(x=0;x<pEnContext->pVCodecCtx->width/2;x++) 
            {
                vframe->data[1][y * vframe->linesize[1] + x] = 128 + y + i * 2;
                vframe->data[2][y * vframe->linesize[2] + x] = 64 + x + i * 5;
            }
        }
        vframe->pts = i;
        time_t vrawtime;
     	long vtime = (long)time(&vrawtime);

        if (!RawQueuePut(pRawQueue, (void *)vframe, sizeof(*vframe), 0, vtime+i))
        {	
        	break;
        }     
    }
    RawQueuePut(pRawQueue, NULL, -1, 0, 0);
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
			//main_exit();
			flag = false;
			break;
		default:
		printf("signal:%d\n",sig);
		break;
	}
}

void main_exit()
{
	StopEnContext(pEnContext);
	StopClient(pClient);
	ReleaseRawQueue(pRawQueue);
	ReleasePacketQueue(pPacketQueue);
	ReleaseEnContext(pEnContext);
	printf("dddddddd\n");
	
	pRawQueue = NULL;
	printf("dddddddd\n");
	pPacketQueue = NULL;
	printf("dddddddd\n");
	pEnContext = NULL;
	printf("dddddddd\n");
	pClient = NULL;
}
