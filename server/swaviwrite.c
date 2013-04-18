
#include   "swaviwrite.h"
#include   "swpacket.h"

SWPacketQueue *pWPacketQueue = NULL;

static bool running = false;
static bool quit = false;

void *AddStream(AVFormatContext *oc, AVCodec **codec, enum AVCodecID codec_id, int codec_type)
{
	AVCodecContext *c;
    AVStream *st;
    //AVCodec *codec;
    *codec = avcodec_find_encoder(codec_id);
    
    if (!(*codec)) 
    {
        return NULL;
    }
    
    st = avformat_new_stream(oc, *codec);
    if (!st) 
    {	
        return NULL;
    }

    c = st->codec;
    (*codec)->type = codec_type;
    switch (codec_type) 
    {
    case AVMEDIA_TYPE_AUDIO:
        st->id = 1;
        c->codec_id = codec_id;
        c->sample_fmt  = 8;
        c->bit_rate    = 47994;
        c->sample_rate = 44100;
        c->channels    = 2;            
        break;
    case AVMEDIA_TYPE_VIDEO:
    	st->id = 0;
        c->codec_id = codec_id;
        c->bit_rate = 400000;
        c->width    = 352;
        c->height   = 288;
        c->time_base.den = 25;
        c->time_base.num = 1;
        c->gop_size      = 10; 
        c->pix_fmt       = AV_PIX_FMT_YUV420P;
        
        if (c->codec_id == AV_CODEC_ID_MPEG2VIDEO) 
        {
            c->max_b_frames = 2;
        }
        if (c->codec_id == AV_CODEC_ID_MPEG1VIDEO) 
        {
            c->mb_decision = 2;
        }
        break;

    default:
        break;
    }
     /* Some formats want stream headers to be separate. */
    if (oc->oformat->flags & AVFMT_GLOBALHEADER)
        c->flags |= CODEC_FLAG_GLOBAL_HEADER;
    return st;
}

void *InitAviWrite(void *packetqueue, char *filename)
{
	if (!packetqueue)
	{
		return NULL;
	}
	pWPacketQueue = (SWPacketQueue *)packetqueue;
	SWAviWrite *paviwrite = (SWAviWrite *)malloc(sizeof(SWAviWrite));
	
	if (!paviwrite)
	{
		return NULL;
	}
	
	memset(paviwrite,0,sizeof(SWAviWrite));
	av_register_all();
	
	if (NULL == filename)
	{
		filename = "test.avi";
	}
	
	avformat_alloc_output_context2(&paviwrite->pFormatCtx, NULL, NULL, filename);
	
	if (!paviwrite->pFormatCtx)
	{
		return NULL;
	}
	
	AVCodec *acodec, *vcodec;
	paviwrite->pOutFormat = paviwrite->pFormatCtx->oformat;
	paviwrite->pVideoSt = (AVStream *)AddStream(paviwrite->pFormatCtx,
												&vcodec,
												AV_CODEC_ID_MPEG1VIDEO,
												AVMEDIA_TYPE_VIDEO);
													
	paviwrite->pAudioSt = (AVStream *)AddStream(paviwrite->pFormatCtx,
												&acodec,
												AV_CODEC_ID_WMAV2,
												AVMEDIA_TYPE_AUDIO);

	if (!paviwrite->pAudioSt || !paviwrite->pVideoSt)
	{
		avformat_close_input(&paviwrite->pFormatCtx);
		free(paviwrite);
		paviwrite = NULL;
		return NULL;
	}
	
	av_dump_format(paviwrite->pFormatCtx, 0, filename, 1);
	
	if (!(paviwrite->pOutFormat->flags & AVFMT_NOFILE))
	{
		if (avio_open(&paviwrite->pFormatCtx->pb, filename, AVIO_FLAG_WRITE) < 0)
		{
			
			int i;
			for (i = 0; i < paviwrite->pFormatCtx->nb_streams; i++) 
			{
				av_freep(&paviwrite->pFormatCtx->streams[i]->codec);
				av_freep(&paviwrite->pFormatCtx->streams[i]);
			}
			
			if (paviwrite->pOutFormat)
			{
				if (!(paviwrite->pOutFormat->flags & AVFMT_NOFILE))
					/* Close the output file. */
					avio_close(paviwrite->pFormatCtx->pb);
			}
			
			avformat_close_input(&paviwrite->pFormatCtx);
			free(paviwrite);
			paviwrite = NULL;
			return NULL;
		}
	}
	
	avformat_write_header(paviwrite->pFormatCtx, NULL);
	return paviwrite;	
}

void *AviWriteThread(void *arg)
{
	SWAviWrite *paviwrite = (SWAviWrite *)arg;
	
	if (!paviwrite)
	{
		return false;
	}
	
	quit = false;
	running = true;
	SWPacket *packet = NULL;
	AVPacket pkt;
	uint8_t *buffer;
	int ret;

	while(!quit)
	{
		packet = (SWPacket *)PacketQueueGet(pWPacketQueue);
		
		if (!packet)
		{
			usleep(50);
	
			continue;
		}
		if (packet->stream_index == -1)
		{
			break;
		}
		//写入文件
		av_init_packet(&pkt);
		pkt.data = NULL;
		pkt.size = 0;
		buffer = (uint8_t *)packet->data;
		if (buffer)
		{	
			pkt.data = buffer;
			pkt.size = packet->size;
			if (packet->key)
			{	
				pkt.flags |= AV_PKT_FLAG_KEY;
			}
			pkt.stream_index = packet->stream_index;
			pkt.pts = packet->pts;
			pkt.dts = packet->dts;
			fflush(stdout);
			ret = av_interleaved_write_frame(paviwrite->pFormatCtx, &pkt);
			if (ret != 0) 
			{
				fprintf(stderr, "Error while writing audio frame: %s\n",
								av_err2str(ret));
				exit(1);
			}
			
		 }
		 //av_free_packet(&pkt);
		 free(packet);
		 packet = NULL;		
		
	}	
	running = false;
	ReleaseAviWrite(paviwrite);
	printf("write end\n");
}

bool StartThread(void *arg)
{
	SWAviWrite *paviwrite = (SWAviWrite *)arg;
	
	if (!paviwrite || pthread_create(&paviwrite->thread, NULL, 
	                                 &AviWriteThread, paviwrite) != 0)
	{	
		return false;
	}
	
	return true;
}

bool StopThread(void *arg)
{
	SWAviWrite *paviwrite = (SWAviWrite *)arg;
	
	if (!paviwrite)
	{
		return false;
	}
	
	quit = true;
	pthread_join(paviwrite->thread, NULL);
	while (running)
	{
		usleep(500);
	}
	return true;
}

bool ReleaseAviWrite(void *aviwrite)
{
	SWAviWrite *paviwrite = (SWAviWrite *)aviwrite;
	if (!paviwrite)
	{
		return false;
	}
	av_write_trailer(paviwrite->pFormatCtx);
	int i;
	for (i = 0; i < paviwrite->pFormatCtx->nb_streams; i++) 
	{
		avcodec_close(paviwrite->pFormatCtx->streams[i]->codec);
		//av_freep(&pAviWrite->pFormatCtx->streams[i]->codec);
		//av_freep(&pAviWrite->pFormatCtx->streams[i]);
	}
			
	if (paviwrite->pOutFormat)
	{
		if (!(paviwrite->pOutFormat->flags & AVFMT_NOFILE))
		{
			avio_close(paviwrite->pFormatCtx->pb);
		}
	}	
	avformat_free_context(paviwrite->pFormatCtx);
	free(paviwrite);
	return true;
}

