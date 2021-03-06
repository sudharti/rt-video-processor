/* based_task.c -- A basic real-time task skeleton. 
 *
 * This (by itself useless) task demos how to setup a 
 * single-threaded LITMUS^RT real-time task.
 */

/* First, we include standard headers.
 * Generally speaking, a LITMUS^RT real-time task can perform any
 * system call, etc., but no real-time guarantees can be made if a
 * system call blocks. To be on the safe side, only use I/O for debugging
 * purposes and from non-real-time sections.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Include ffmpeg libraries */
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>

#include "SDL.h"
#include <SDL_thread.h>

#ifdef __MINGW32__
#undef main /* Prevents SDL from overriding main() */
#endif

/* Second, we include the LITMUS^RT user space library header.
 * This header, part of liblitmus, provides the user space API of
 * LITMUS^RT.
 */
#include "litmus.h"

/* Next, we define period and execution cost to be constant. 
 * These are only constants for convenience in this example, they can be
 * determined at run time, e.g., from command line parameters.
 *
 * These are in milliseconds.
 */
#define PERIOD            10
#define RELATIVE_DEADLINE 100
#define EXEC_COST         10

/* Catch errors.
 */
#define CALL( exp ) do { \
		int ret; \
		ret = exp; \
		if (ret != 0) \
			fprintf(stderr, "%s failed: %m\n", #exp);\
		else \
			fprintf(stderr, "%s ok.\n", #exp); \
	} while (0)


/* Declare the periodically invoked job. 
 * Returns 1 -> task should exit.
 *         0 -> task should continue.
 */
int job(void);

/* typically, main() does a couple of things: 
 * 	1) parse command line parameters, etc.
 *	2) Setup work environment.
 *	3) Setup real-time parameters.
 *	4) Transition to real-time mode.
 *	5) Invoke periodic or sporadic jobs.
 *	6) Transition to background mode.
 *	7) Clean up and exit.
 *
 * The following main() function provides the basic skeleton of a single-threaded
 * LITMUS^RT real-time task. In a real program, all the return values should be 
 * checked for errors.
 */

	//Initialize ffmpeg variables
	 AVFormatContext *pFormatCtx = NULL;
	  int             i, videoStream;
	  AVCodecContext  *pCodecCtx = NULL;
	  AVCodec         *pCodec = NULL;
	  AVFrame         *pFrame = NULL; 
	  AVPacket        packet;
	  int             frameFinished;
  	  float           aspect_ratio;

	  AVDictionary    *optionsDict = NULL;
	  struct SwsContext *sws_ctx = NULL;

	  SDL_Overlay     *bmp = NULL;
	  SDL_Surface     *screen = NULL;
	  SDL_Rect        rect;
	  SDL_Event       event;

	  AVPicture pict;

int main(int argc, char** argv)
{
	int do_exit;
	struct rt_task param;

	/* Setup task parameters */
	init_rt_task_param(&param);
	param.exec_cost = ms2ns(EXEC_COST);
	param.period = ms2ns(PERIOD);
	param.relative_deadline = ms2ns(RELATIVE_DEADLINE);

	/* What to do in the case of budget overruns? */
	param.budget_policy = NO_ENFORCEMENT;

	/* The task class parameter is ignored by most plugins. */
	param.cls = RT_CLASS_SOFT;

	/* The priority parameter is only used by fixed-priority plugins. */
	param.priority = LITMUS_LOWEST_PRIORITY;

	 // Register all formats and codecs
	 av_register_all();
	 
	  // Initialize SDL
	  if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
	    fprintf(stderr, "Could not initialize SDL - %s\n", SDL_GetError());
	    exit(1);
	  }else{
	  	fprintf(stderr, "%s\n", "SDL initialized"); 
	  }

	  // Open video file
	  if(avformat_open_input(&pFormatCtx, "/home/hkr/Videos/sample.mp4", NULL, NULL)!=0){
	    return -1; // Couldn't open file
	  }else{
	  	fprintf(stderr, "%s\n", "File opened"); 
	  }
	  // Retrieve stream information
	  if(avformat_find_stream_info(pFormatCtx, NULL)<0)
	    return -1; // Couldn't find stream information
	  	
	  // Find the first video stream
	  videoStream=-1;
	  for(i=0; i<pFormatCtx->nb_streams; i++)
	    if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO) {
	      videoStream=i;
	      break;
	    }
	  if(videoStream==-1)
	    return -1; // Didn't find a video stream
	  
	  // Get a pointer to the codec context for the video stream
	  pCodecCtx=pFormatCtx->streams[videoStream]->codec;
	  
	  // Find the decoder for the video stream
	  pCodec=avcodec_find_decoder(pCodecCtx->codec_id);
	  if(pCodec==NULL) {
	    fprintf(stderr, "Unsupported codec!\n");
	    return -1; // Codec not found
	  }
	  
	  // Open codec
	  if(avcodec_open2(pCodecCtx, pCodec, &optionsDict)<0)
	    return -1; // Could not open codec
	  
	  // Allocate video frame
	  pFrame=av_frame_alloc();

	  // Make a screen to put our video
	#ifndef __DARWIN__
		screen = SDL_SetVideoMode(pCodecCtx->width, pCodecCtx->height, 0, 0);
	#else
		screen = SDL_SetVideoMode(pCodecCtx->width, pCodecCtx->height, 24, 0);
	#endif
	  if(!screen) {
	    fprintf(stderr, "SDL: could not set video mode - exiting\n");
	    exit(1);
	  }
	  
	  // Allocate a place to put our YUV image on that screen
	  bmp = SDL_CreateYUVOverlay(pCodecCtx->width,
					 pCodecCtx->height,
					 SDL_YV12_OVERLAY,
					 screen);
	  
	  sws_ctx =
	    sws_getContext
	    (
		pCodecCtx->width,
		pCodecCtx->height,
		pCodecCtx->pix_fmt,
		pCodecCtx->width,
		pCodecCtx->height,
		PIX_FMT_YUV420P,
		SWS_BILINEAR,
		NULL,
		NULL,
		NULL
	    ); 	

	/* The task is in background mode upon startup. */


	/*****
	 * 1) Command line paramter parsing would be done here.
	 */



	/*****
	 * 2) Work environment (e.g., global data structures, file data, etc.) would
	 *    be setup here.
	 */



	/*****
	 * 3) Setup real-time parameters. 
	 *    In this example, we create a sporadic task that does not specify a 
	 *    target partition (and thus is intended to run under global scheduling). 
	 *    If this were to execute under a partitioned scheduler, it would be assigned
	 *    to the first partition (since partitioning is performed offline).
	 */
	CALL( init_litmus() );

	/* To specify a partition, do
	 *
	 * param.cpu = CPU;
	 * be_migrate_to(CPU);
	 *
	 * where CPU ranges from 0 to "Number of CPUs" - 1 before calling
	 * set_rt_task_param().
	 */
	CALL( set_rt_task_param(gettid(), &param) );

	fprintf(stderr, "%s\n", "RT Task Set");

	/*****
	 * 4) Transition to real-time mode.
	 */
	CALL( task_mode(LITMUS_RT_TASK) );

	/* The task is now executing as a real-time task if the call didn't fail. 
	 */

	fprintf(stderr,"%s\n","some informatoin");
	/*****
	 * 5) Invoke real-time jobs.
	 */
	do {
		/* Wait until the next job is released. */
		//sleep_next_period();
		/* Invoke job. */
		do_exit = job();		
		// printf("%d",do_exit);
	} while (!do_exit);


	
	/*****
	 * 6) Transition to background mode.
	 */
	CALL( task_mode(BACKGROUND_TASK) );

	

	/***** 
	 * 7) Clean up, maybe print results and stats, and exit.
	 */
	// Free the packet that was allocated by av_read_frame
	    av_free_packet(&packet);
	    SDL_PollEvent(&event);
	    switch(event.type) {
	    case SDL_QUIT:
	      SDL_Quit();
	      exit(0);
	      break;
	    default:
	      break;
	}
	return 0;
	}



int job(void) 
{	

//	fprintf(stderr,"%s\n","J");

  if(av_read_frame(pFormatCtx, &packet)>=0) {
  //	fprintf(stderr,"%s\n","FR");
    // Is this a packet from the video stream?
    if(packet.stream_index==videoStream) {
      // Decode video frame
      avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, 
			   &packet);
      
      // Did we get a video frame?
      if(frameFinished) {
	SDL_LockYUVOverlay(bmp);

	
	pict.data[0] = bmp->pixels[0];
	pict.data[1] = bmp->pixels[2];
	pict.data[2] = bmp->pixels[1];

	pict.linesize[0] = bmp->pitches[0];
	pict.linesize[1] = bmp->pitches[2];
	pict.linesize[2] = bmp->pitches[1];

	// Convert the image into YUV format that SDL uses
    sws_scale
    (
        sws_ctx, 
        (uint8_t const * const *)pFrame->data, 
        pFrame->linesize, 
        0,
        pCodecCtx->height,
        pict.data,
        pict.linesize
    );
	
	SDL_UnlockYUVOverlay(bmp);
	
	rect.x = 0;
	rect.y = 0;
	rect.w = pCodecCtx->width;
	rect.h = pCodecCtx->height;
	SDL_DisplayYUVOverlay(bmp, &rect);
      
      }
    }
	//return 0;	
	}else{
		return 1;
	}



	return 0;
}
