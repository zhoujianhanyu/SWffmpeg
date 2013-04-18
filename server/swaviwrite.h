
#ifndef _SWAVIWRITE_H
#define _SWAVIWRITE_H


#include	<stdlib.h>
#include	<stdio.h>
#include    <string.h>
#include	"libavformat/avformat.h"
#include	"libavcodec/avcodec.h"
#include	"libswscale/swscale.h"
#include	<pthread.h>
#include    <stdbool.h>

typedef struct _SWAviWrite
{	
	AVFormatContext *pFormatCtx;
	AVOutputFormat *pOutFormat;
	
	//AVCodecContext *pVCodecCtx;
	//AVCodecContext *pACodecCtx;
	AVStream *pAudioSt; 
	AVStream *pVideoSt;
	pthread_t thread;
	
}SWAviWrite;

void *InitAviWrite(void *packetqueue, char *filename);

void *AddStream(AVFormatContext *oc, AVCodec **codec, enum AVCodecID codec_id, int codec_type);

void *AviWriteThread(void *arg);

bool StartThread(void *arg);
bool StopThread(void *arg);

bool ReleaseAviWrite(void *aviwrite);

#endif /* _SWAVIWRITE_H */
