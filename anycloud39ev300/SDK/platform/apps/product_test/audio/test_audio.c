#include <errno.h>
#include <sys/socket.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include "ak_net.h"
#include "ak_common.h"
#include "ak_global.h"
#include "ak_thread.h"
#include "ak_ai.h"
#include "test_common.h"

#define SAVE_RECORD2FILE
#define TCP_PORT 		6791

/* audio test control */
typedef struct _audio_net_ctrl_
{
    int socket_fd;
    int conn_fd;
    int audio_type;
    int send_size;
}audio_net_ctrl_info, *Paudio_net_ctrl_info;

static ak_pthread_t test_thid;
static Paudio_net_ctrl_info paudio_net_ctrl = NULL;

#ifdef SAVE_RECORD2FILE
static int save_fd = -1;
#define SAVE_RECORD_FILE "/tmp/record.pcm"
#endif

/**
 * audio_pcm_send - send audio data to pc.
 * @pstream[IN]: audio data frame
 * return: 0, success; -1, failed
 */
static int audio_pcm_send( struct frame *pstream)
{
    paudio_net_ctrl->send_size += pstream->len;
    if((paudio_net_ctrl->conn_fd >= 0) 
		&& (-1 == send(paudio_net_ctrl->conn_fd, pstream->data, pstream->len, MSG_NOSIGNAL)))
	{
		ak_print_normal_ex(" %d , %s\n ",errno, strerror(errno));
		/* pc test tool disconnect */
		if((errno == EPIPE))
		{
		 	close(paudio_net_ctrl->conn_fd);
			paudio_net_ctrl->conn_fd = -1;
			return -1;
		}
    }
	/* save pcm data for debug */
#ifdef SAVE_RECORD2FILE
	if(( save_fd >= 0) && (paudio_net_ctrl->send_size < 1000000)){
		write(save_fd, pstream->data, pstream->len);
	}
#endif
	return 0;
}

/**
 * read_ad_pcm - read audio data from ai and send it to pc.
 * @ai_handle[IN]: opened ai handle
 * return: void
 */
static void read_ad_pcm(void *ai_handle)
{
	unsigned long long start_ts = 0;
	unsigned long long end_ts = 0;
	unsigned long pre_seq_no = 0;
	struct frame frame = {0};

	if(AK_SUCCESS == ak_ai_set_frame_interval(ai_handle, AUDIO_DEFAULT_INTERVAL)) {
		ak_print_normal_ex("frame interval=%d\n", AUDIO_DEFAULT_INTERVAL);
	}
	/* set microphone as source for test */
	if (ak_ai_set_source(ai_handle, AI_SOURCE_MIC) != 0)
		ak_print_error_ex("set ai source mic fail\n");
	else
		ak_print_normal_ex("set ai source mic success\n");

	while(1) {

		if (!test_get_conn_state())
			break;
			
		/* get pcm data frame */
		if (ak_ai_get_frame(ai_handle, &frame, 0) < 0) {
			ak_sleep_ms(10);
			continue;
		}
		/* send pcm data frame to pc */
		if(audio_pcm_send(&frame)< 0){
			ak_ai_release_frame(ai_handle, &frame);;
			break;
		}

		if (frame.seq_no != (pre_seq_no + 1)) {
			ak_print_normal_ex("audio: ts=%llu, len=%u, seq_no=%lu\n",
    			frame.ts, frame.len, frame.seq_no);
		}
		pre_seq_no = frame.seq_no;

		if(0 == start_ts) {
			start_ts = frame.ts;
			end_ts = frame.ts;
		}
		end_ts = frame.ts;
		ak_ai_release_frame(ai_handle, &frame);
	}

	ak_print_normal("\n\t start_ts=%llu, end_ts=%llu, use=%lld(ms)\n",
		start_ts, end_ts, (end_ts - start_ts));
	ak_print_normal("\t read_ad_pcm exit\n\n");
}

/**
 * audio_test_thread - create tcp server port and wait pc tool connect it.
 * @param[IN]: opened ai handle
 * return: null
 */
static void * audio_test_thread( void* param)
{
	void *ai_handle = (void *)param;
	int sock_fd;
	struct sockaddr_in my_addr, peer_addr;
	int sinsize;

	memset(&my_addr, 0, sizeof(struct sockaddr));
	memset(&peer_addr, 0, sizeof(struct sockaddr));

	/* init tcp port struct */
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(TCP_PORT);
	my_addr.sin_addr.s_addr = INADDR_ANY;

	ak_print_normal_ex(" This thread id : %ld\n", ak_thread_get_tid());
#if 0
	while(ak_net_get_ip("eth0",ip) && ak_net_get_ip("wlan0",ip)){
		ak_print_normal("net not ready!\n");
		ak_sleep_ms(500);
	}
#endif
    paudio_net_ctrl = calloc(1,sizeof(audio_net_ctrl_info));
    if(paudio_net_ctrl == NULL)
    {
        ak_print_error_ex(" fails to malloc ram!\n");
        return NULL;
    }
    paudio_net_ctrl->send_size = 0;

#ifdef SAVE_RECORD2FILE
	save_fd  = open( SAVE_RECORD_FILE ,  O_RDWR | O_CREAT | O_TRUNC );
    if(save_fd >= 0) lseek( save_fd, 0, SEEK_SET );
#endif

	/* create socket */
	sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(sock_fd == -1)
	{
		ak_print_error_ex(" fail to create TCP socket.\n");
		goto close_sk;
	}
	ak_print_normal_ex(" Success to create TCP socket.\n");

	sinsize = 1;
	if(setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &sinsize, sizeof(int)) != 0)
	{
		ak_print_error_ex(" set socket option failed,%s\n", strerror(errno));
		ak_sleep_ms(3000);
		goto close_sk;
	}
	if(fcntl(sock_fd, F_SETFD, FD_CLOEXEC) == -1)
	{
		ak_print_error_ex(" error:%s\n", strerror(errno));
		ak_sleep_ms(3000);
		goto close_sk;
	}
	/* bind */
	if(bind(sock_fd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1)
	{
		ak_print_error_ex(" bind socket failed, %s.\n", strerror(errno));
		ak_sleep_ms(3000);
		goto close_sk;
	}
	ak_print_normal(" Success to bind socket.\n");
	/* listen */
	if(listen(sock_fd, 1) == -1)
	{
		ak_print_error_ex("listen failed\n");
		goto close_sk;
	}
	ak_print_normal(" Set Listen success.\n");
	ak_print_normal_ex(" Waiting for connect......\n");
	paudio_net_ctrl->socket_fd = sock_fd;
	/* accept */
	sinsize = sizeof(struct sockaddr);
	while(1){
		ak_print_normal_ex(" goto accept audio listen socket\n");
		/* wait pc tool connect tcp port*/
		paudio_net_ctrl->conn_fd = accept(sock_fd, (struct sockaddr *)&peer_addr, (socklen_t *)&sinsize);
		if(paudio_net_ctrl->conn_fd == -1)
		{
			ak_print_error_ex(" accept failed\n");
			goto close_sk;
		}
		else
		{
			ak_print_normal_ex("audio rec send connnected.\n");
			/* get audio data and send it to pc tool */
	       	read_ad_pcm(ai_handle);
			ak_print_normal_ex("audio rec send end.\n");
			ak_sleep_ms(1000);
		}
	}
#ifdef SAVE_RECORD2FILE
	if( save_fd >= 0)
	{
		close(save_fd);
		save_fd = -1;
	}
#endif

close_sk:
	close(sock_fd);
	free(paudio_net_ctrl);
	paudio_net_ctrl = NULL;
	ak_print_normal_ex(" exit\n");
	return NULL;
}

/**
 * test_audio_start - test audio function.
 * @ai[IN]: opened ai handle
 * return: void
 */
void test_audio_start( void *ai)
{
	ak_print_normal_ex("ai_handle:%p\n", ai);
	/*
	 * start a thread to do audio test.
	 * it run background.
	 */
	ak_thread_create(&test_thid, audio_test_thread,
			(void *)ai, 100 *1024, -1);
}



