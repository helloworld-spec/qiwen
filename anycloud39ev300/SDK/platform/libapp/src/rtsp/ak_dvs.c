/*************************************************
  dvs.c
  used by dvs
 **************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "ak_common.h"
#include "ak_thread.h"
#include "ak_dvs.h"
#include "rtsp.h"

dvs_t g_dvs;
dvs_params_t g_param;

void buffers_clear (dvs_t * dvs, buffers_t * bufs, int chunnel)
{
	int i;
	ak_thread_mutex_lock (&(bufs->mutex));
	for (i = 0; i < bufs->bufnum - 1; i++) {
		if (bufs->buf[i].chunnel == chunnel) {
			bufs->buf[i].chunnel = -1;
			if (bufs->buf[i].start != NULL) {
				free (bufs->buf[i].start);
				bufs->buf[i].start = NULL;
			}
			bufs->buf[i].length = 0;
		}
	}
	ak_thread_mutex_unlock (&(bufs->mutex));
}

static int buffers_has_previous_frame (buffers_t * bufs, int chunnel, int frame_index)
{
	int i, j;
	for (i = 0; i < bufs->bufnum - 1; i++) {
		j = bufs->rear - i - 1;
		if (j < 0)
			j = bufs->bufnum + j;
		if (bufs->buf[j].chunnel == chunnel) {
			if (bufs->buf[j].frame_index == frame_index - 1) {
				return 1;
			}
			if (bufs->buf[j].type == 0x02) { 	//I-Frame
				return 0;
			}
		}
	}
	return 0;
}

int buffers_put_pframe_data (void *data, int length, buffers_t * bufs,
	   	int type, int chunnel, int frame_index)
{
	int res = 0;
	ak_thread_mutex_lock (&(bufs->mutex));
	if (buffers_has_previous_frame (bufs, chunnel, frame_index) == 0) {
		ak_thread_mutex_unlock (&(bufs->mutex));
		return 0;
	}
	if (((bufs->rear + 1) % bufs->bufnum) == bufs->front) {
		res = 0;
	} else {
		bufs->buf[bufs->rear].length = length;
		bufs->buf[bufs->rear].type = type;
		bufs->buf[bufs->rear].chunnel = chunnel;
		bufs->buf[bufs->rear].frame_index = frame_index;
		if (NULL != bufs->buf[bufs->rear].start)
		{
			free (bufs->buf[bufs->rear].start);
			bufs->buf[bufs->rear].start = NULL;
		}
		bufs->buf[bufs->rear].start = malloc (length);
		if (NULL == bufs->buf[bufs->rear].start)
		{
			printf ("Out of memory, buffers_put_pframe_data:%d\r\n", __LINE__);//fflush(NULL);
            ak_thread_mutex_unlock (&(bufs->mutex));
            return 0;
		}
		memcpy (bufs->buf[bufs->rear].start, data, length);
		bufs->rear = (bufs->rear + 1) % bufs->bufnum;
		res = 1;
	}
	ak_thread_mutex_unlock (&(bufs->mutex));
	return res;
}

int buffers_put_iframe_data (void *data, int length, buffers_t * bufs, int type, int chunnel, int frame_index)
{
	int i, j;
	int res = 0;
	ak_thread_mutex_lock (&(bufs->mutex));
	if (((bufs->rear + 1) % bufs->bufnum) == bufs->front) {
		for (i = 0; i < bufs->bufnum - 1; i++) {
			j = bufs->rear - i - 1;
			if (j < 0)
				j = bufs->bufnum + j;
			if (bufs->buf[j].type != 0x02) { 	//P-Frame
				bufs->buf[j].length = length;
				bufs->buf[j].type = type;
				bufs->buf[j].chunnel = chunnel;
				bufs->buf[j].frame_index = frame_index;
				if (NULL != bufs->buf[j].start) {
					free (bufs->buf[j].start);
					bufs->buf[j].start = NULL;
				}
				bufs->buf[j].start = malloc (length);
				if (NULL == bufs->buf[j].start) {
					printf ("Out of memory, buffers_put_iframe_data:%d\r\n", __LINE__);//fflush(NULL);
                    ak_thread_mutex_unlock (&(bufs->mutex));
                    return 0;
				}
				memcpy (bufs->buf[j].start, data, length);
				ak_thread_mutex_unlock (&(bufs->mutex));
				return 1;
			}
			if (bufs->buf[j].chunnel == chunnel) { 	//Old I-Frame
				bufs->buf[j].length = length;
				bufs->buf[j].type = type;
				bufs->buf[j].chunnel = chunnel;
				bufs->buf[j].frame_index = frame_index;
				if (NULL != bufs->buf[j].start) {
					free (bufs->buf[j].start);
					bufs->buf[j].start = NULL;
				}
				bufs->buf[j].start = malloc (length);
				if (NULL == bufs->buf[j].start) {
					printf ("Out of memory, buffers_put_iframe_data:%d\r\n", __LINE__);//fflush(NULL);
                    ak_thread_mutex_unlock (&(bufs->mutex));
                    return 0;
				}
				memcpy (bufs->buf[j].start, data, length);
				ak_thread_mutex_unlock (&(bufs->mutex));
				return 1;
			}
		}
	} else {
		bufs->buf[bufs->rear].length = length;
		bufs->buf[bufs->rear].type = type;
		bufs->buf[bufs->rear].chunnel = chunnel;
		bufs->buf[bufs->rear].frame_index = frame_index;
		if (NULL != bufs->buf[bufs->rear].start) {
			free (bufs->buf[bufs->rear].start);
			bufs->buf[bufs->rear].start = NULL;
		}
		bufs->buf[bufs->rear].start = malloc (length);
		if (NULL == bufs->buf[bufs->rear].start) {
			printf ("Out of memory, %s:%d\r\n", __func__, __LINE__);//fflush(NULL);
            ak_thread_mutex_unlock (&(bufs->mutex));
            return 0;
		}
		memcpy (bufs->buf[bufs->rear].start, data, length);
		bufs->rear = (bufs->rear + 1) % bufs->bufnum;
		res = 1;
	}
	ak_thread_mutex_unlock (&(bufs->mutex));
	return res;
}

int buffers_get_data (void *data, int *length, buffers_t * bufs, int *type, int *chunnel, int *frame_index)
{
	int res = 0;
	ak_thread_mutex_lock (&(bufs->mutex));
	if (bufs->front != bufs->rear) {
		*length = bufs->buf[bufs->front].length;
		memcpy (data, bufs->buf[bufs->front].start, *length);
		free (bufs->buf[bufs->front].start);
		bufs->buf[bufs->front].start = NULL;
		*type = bufs->buf[bufs->front].type;
		*chunnel = bufs->buf[bufs->front].chunnel;
		*frame_index = bufs->buf[bufs->front].frame_index;
		bufs->front = (bufs->front + 1) % bufs->bufnum;
		res = 1;
	}
	ak_thread_mutex_unlock (&(bufs->mutex));
	return res;
}

int buffers_put_data (void *data, int length, buffers_t * bufs, int type, int chunnel, int frame_index)
{
	int res = 0;
	ak_thread_mutex_lock (&(bufs->mutex));

	if (((bufs->rear + 1) % bufs->bufnum) == bufs->front)
		res = 0;
	else {
		if (length < bufs->bufsize) {
			bufs->buf[bufs->rear].length = length;
			bufs->buf[bufs->rear].type = type;
			bufs->buf[bufs->rear].chunnel = chunnel;
			bufs->buf[bufs->rear].frame_index = frame_index;
			if (NULL != bufs->buf[bufs->rear].start) {
				free (bufs->buf[bufs->rear].start);
				bufs->buf[bufs->rear].start = NULL;
			}
			bufs->buf[bufs->rear].start = malloc (length);
			if (NULL == bufs->buf[bufs->rear].start) {
				printf ("Out of memory, buffers_put_data:%d\r\n", __LINE__);//fflush(NULL);
                ak_thread_mutex_unlock (&(bufs->mutex));
                return 0;
			}
			memcpy (bufs->buf[bufs->rear].start, data, length);
			bufs->rear = (bufs->rear + 1) % bufs->bufnum;
		}
		res = 1;
	}

	ak_thread_mutex_unlock (&(bufs->mutex));
	return res;
}

int buffers_get_data_with_pts (void *data, int *length, buffers_t * bufs, int *type,
								int *chunnel, int *frame_index, unsigned int *pts)
{
	int res = 0;
	ak_thread_mutex_lock (&(bufs->mutex));
	if (bufs->front != bufs->rear) {
		*length = bufs->buf[bufs->front].length;
		memcpy (data, bufs->buf[bufs->front].start, *length);
		free (bufs->buf[bufs->front].start);
		bufs->buf[bufs->front].start = NULL;
		*type = bufs->buf[bufs->front].type;
		*chunnel = bufs->buf[bufs->front].chunnel;
		*frame_index = bufs->buf[bufs->front].frame_index;
		*pts = bufs->buf[bufs->front].pts;
		bufs->front = (bufs->front + 1) % bufs->bufnum;
		res = 1;
	}
	ak_thread_mutex_unlock (&(bufs->mutex));
	return res;
}

/* 
 * copy data to rtsp buffer
 */
int buffers_put_data_with_pts (void *data, int length, buffers_t *bufs, int type,
								int chunnel, int frame_index, unsigned int pts)
{
	int res = 0;
	ak_thread_mutex_lock (&(bufs->mutex));
    
	if (((bufs->rear + 1) % bufs->bufnum) == bufs->front) {
		res = 0;
	} else {
		if (length < bufs->bufsize) {
			bufs->buf[bufs->rear].length = length;
			bufs->buf[bufs->rear].type = type;
			bufs->buf[bufs->rear].chunnel = chunnel;
			bufs->buf[bufs->rear].frame_index = frame_index;
			bufs->buf[bufs->rear].pts = pts;
			if (NULL != bufs->buf[bufs->rear].start) {
				free (bufs->buf[bufs->rear].start);
				bufs->buf[bufs->rear].start = NULL;
			}
			bufs->buf[bufs->rear].start = malloc(length);
			if (NULL == bufs->buf[bufs->rear].start) {
				printf("Out of memory, %s:%d\r\n", __func__, __LINE__);
				//fflush(NULL);
                ak_thread_mutex_unlock(&(bufs->mutex));
                return 0;
			}
			memcpy(bufs->buf[bufs->rear].start, data, length);
			bufs->rear = (bufs->rear + 1) % bufs->bufnum;
		}
		res = 1;
	}

	ak_thread_mutex_unlock (&(bufs->mutex));
	return res;
}

/* 
 * buffers_put_pframe_data_with_pts - put p frame data to buffer's data buf
 * data[IN]: pointer to input data
 * length[IN]: indicate data's length
 * bufs[OUT]: reflect one client data buffer
 * type[IN]: frame type
 * chunnel[IN]: current channel id
 * frame_index[IN]: frame sequense index number
 * pts[IN]: timestamp
 * return: 1 on success, 0 on failed
 */
int buffers_put_pframe_data_with_pts (void *data, int length, buffers_t * bufs,
	   	int type, int chunnel, int frame_index, unsigned int pts)
{
	int res = 0;
	ak_thread_mutex_lock (&(bufs->mutex));
	if (buffers_has_previous_frame (bufs, chunnel, frame_index) == 0) {
		ak_thread_mutex_unlock (&(bufs->mutex));
		return 0;
	}
	if (((bufs->rear + 1) % bufs->bufnum) == bufs->front) {
		res = 0;
	} else {
		bufs->buf[bufs->rear].length = length;
		bufs->buf[bufs->rear].type = type;
		bufs->buf[bufs->rear].chunnel = chunnel;
		bufs->buf[bufs->rear].frame_index = frame_index;
		bufs->buf[bufs->rear].pts = pts;
		if (NULL != bufs->buf[bufs->rear].start) {
			free (bufs->buf[bufs->rear].start);
			bufs->buf[bufs->rear].start = NULL;
		}
		bufs->buf[bufs->rear].start = malloc (length);
		if (NULL == bufs->buf[bufs->rear].start) {
			printf ("Out of memory, %s:%d\r\n", __func__, __LINE__);
			//fflush(NULL);
            ak_thread_mutex_unlock (&(bufs->mutex));
            return 0;
		}
		memcpy (bufs->buf[bufs->rear].start, data, length);
		bufs->rear = (bufs->rear + 1) % bufs->bufnum;
		res = 1;
	}
	ak_thread_mutex_unlock (&(bufs->mutex));
	return res;
}

/* 
 * buffers_put_iframe_data_with_pts - put I frame data to buffer's data buf
 * data[IN]: pointer to input data
 * length[IN]: indicate data's length
 * bufs[OUT]: reflect one client data buffer
 * type[IN]: frame type
 * chunnel[IN]: current channel id
 * frame_index[IN]: frame sequense index number
 * pts[IN]: timestamp
 * return: 1 on success, 0 on failed
 */
int buffers_put_iframe_data_with_pts(void *data, int length, 
		buffers_t * bufs, int type, int chunnel, int frame_index, unsigned int pts)
{
	int i, j;
	int res = 0;
	ak_thread_mutex_lock (&(bufs->mutex));
	if (((bufs->rear + 1) % bufs->bufnum) == bufs->front) {
		printf("BUF FULL!\r\n");
		for (i = 0; i < bufs->bufnum - 1; i++) {
			j = bufs->rear - i - 1;
			if (j < 0)
				j = bufs->bufnum + j;
			if (bufs->buf[j].type != 0x02) { 	//P-Frame
				bufs->buf[j].length = length;
				bufs->buf[j].type = type;
				bufs->buf[j].chunnel = chunnel;
				bufs->buf[j].frame_index = frame_index;
				bufs->buf[j].pts = pts;
				if (NULL != bufs->buf[j].start) {
					free (bufs->buf[j].start);
					bufs->buf[j].start = NULL;
				}
				bufs->buf[j].start = malloc (length);
				if (NULL == bufs->buf[j].start) {
					printf ("Out of memory, %s:%d\r\n", __func__, __LINE__);
					//fflush(NULL);
                    ak_thread_mutex_unlock (&(bufs->mutex));
                    return 0;
				}
				memcpy(bufs->buf[j].start, data, length);
				ak_thread_mutex_unlock (&(bufs->mutex));
				return 1;
			}
			if (bufs->buf[j].chunnel == chunnel) {	//Old I-Frame
				bufs->buf[j].length = length;
				bufs->buf[j].type = type;
				bufs->buf[j].chunnel = chunnel;
				bufs->buf[j].frame_index = frame_index;
				bufs->buf[j].pts = pts;
				if (NULL != bufs->buf[j].start) {
					free (bufs->buf[j].start);
					bufs->buf[j].start = NULL;
				}
				bufs->buf[j].start = malloc (length);
				if (NULL == bufs->buf[j].start) {
					printf ("Out of memory, %s:%d\r\n", __func__,  __LINE__);
					//fflush(NULL);
                    ak_thread_mutex_unlock (&(bufs->mutex));
                    return 0;
				}
				memcpy (bufs->buf[j].start, data, length);
				ak_thread_mutex_unlock (&(bufs->mutex));
				return 1;
			}
		}
	} else {
		bufs->buf[bufs->rear].length = length;
		bufs->buf[bufs->rear].type = type;
		bufs->buf[bufs->rear].chunnel = chunnel;
		bufs->buf[bufs->rear].frame_index = frame_index;
		bufs->buf[bufs->rear].pts = pts;
		if (NULL != bufs->buf[bufs->rear].start) {
			free (bufs->buf[bufs->rear].start);
			bufs->buf[bufs->rear].start = NULL;
		}
		bufs->buf[bufs->rear].start = malloc (length);
		if (NULL == bufs->buf[bufs->rear].start) {
			printf ("Out of memory, %s:%d\r\n", __func__, __LINE__);
			//fflush(NULL);
            ak_thread_mutex_unlock (&(bufs->mutex));
            return 0;
		}
		memcpy (bufs->buf[bufs->rear].start, data, length);
		bufs->rear = (bufs->rear + 1) % bufs->bufnum;
		res = 1;
	}
	ak_thread_mutex_unlock (&(bufs->mutex));
	return res;
}

/* 
 * buffers_clear_data - clear buffers data
 * bufs[IN]: pointer to bufs will be clear
 */
void buffers_clear_data (buffers_t * bufs)
{
	int i;
	ak_thread_mutex_lock(&(bufs->mutex));
	bufs->rear = 0;
	bufs->front = 0;
	for (i = 0; i < bufs->bufnum - 1; i++) {
		bufs->buf[i].chunnel = -1;
		if (bufs->buf[i].start != NULL) {
			free (bufs->buf[i].start);
			bufs->buf[i].start = NULL;
		}
		bufs->buf[i].length = 0;
	}
	ak_thread_mutex_unlock(&(bufs->mutex));
}


#if 0
int send_cmd (dvs_t * dvs, char *cmdstr)
{
	struct sockaddr_in video_server;
	char tmpstr[TEMP_STR_LEN];
	char recvbuf[TEMP_STR_LEN];
	int sock;
	if ((sock = socket (PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
	{
		printf ("Failed to create socket\r\n");
		return -1;
	}
	memset (&video_server, 0, sizeof (video_server));
	video_server.sin_family = AF_INET;
	video_server.sin_addr.s_addr = inet_addr ("127.0.0.1");
	video_server.sin_port = htons (999);
	if (connect (sock,
				(struct sockaddr *) &video_server, sizeof (video_server)) < 0)
	{
		//    printf ("Failed to connect with server\r\n");
		close (sock);
		return -1;
	}
	sprintf (tmpstr, "%s$$boundary\r\n", cmdstr);
	send (sock, tmpstr, strlen (tmpstr), 0);
	int res = 0;
	while (1)
	{
		res = recv (sock, recvbuf, TEMP_STR_LEN, 0);
		if (res == 0)
		{
			printf ("disconnect from server\r\n");
			close (sock);
			return 0;
		}
		else if (res < 0)
		{
			printf ("socket error\r\n");
			close (sock);
			return -1;
		}
		else
		{
			printf ("send_cmd recv %d bytes\r\n", res);
		}

	}

	return 0;
}
#endif

/*
 * dvs_init_buffers - init buffer struct info
 * bufs[OUT]: buf which gonna to be init
 * bufsize[IN]: each buf struct data buf size
 * bufnum[IN]: total buffer number in a buffers_t node
 * return: 0 on success, -1 failed.
 */
int dvs_init_buffers (buffers_t * bufs, int bufsize, int bufnum)
{
	int i;
	bufs->rear = 0;
	bufs->front = 0;
	bufs->bufsize = bufsize;
	if (bufnum > MAX_BUF_NUM)
		bufs->bufnum = MAX_BUF_NUM;
	else
		bufs->bufnum = bufnum;

	/* init all sub-buffer */
	for (i = 0; i < bufs->bufnum; i++) {
		bufs->buf[i].length = 0;
		bufs->buf[i].start = NULL;	//(void *) malloc (bufsize);
		bufs->buf[i].type = -1;
		bufs->buf[i].chunnel = -1;
		bufs->buf[i].pts = 0;
		bufs->buf[i].frame_index = 0;
		//  if (bufs->buf[i].start == NULL)
		//return -1;
		// memset(bufs->buf[i].start, 0, bufsize);
	}
	/* init its mutex */
	if (ak_thread_mutex_init(&(bufs->mutex), NULL) != 0)
		return -1;
	return 0;
}

/*
 * dvs_init_chunnel - init channel resource
 * chunnel[IN]: pointer to chunnel_t
 * index[IN]: current channel index
 * return: 0 on success, -1 failed
 */
static int dvs_init_chunnel(chunnel_t *chunnel, int index)
{
	//int i, j;
	memset(chunnel->name, 0, MAX_NAME_LEN);
	sprintf(chunnel->name, "camera-%d", index + 1);

	/* main channel video_bufs */
	if (dvs_init_buffers(&(chunnel->video_send_bufs), 
				MAX_VIDEO_ENCODE_BUF_SIZE, 8) == -1) {
		ak_print_error_ex("init video send buffer failed\n");
		return -1;
	}

	/* sub channel video_bufs */
	if (dvs_init_buffers(&(chunnel->video_send_bufs_small), 
				MAX_VIDEO_ENCODE_BUF_SIZE / 4, 8) == -1) {
		ak_print_error_ex("init small video send buffer failed\n");
		return -1;
	}

	/* init channel mutex */
	if (ak_thread_mutex_init(&(chunnel->mutex), NULL) != 0) {
		ak_print_error_ex("mutex init failed\n");
		return -1;
	}
	/* init channel buffer operate mutex */
	if (ak_thread_mutex_init(&(chunnel->mutex_buf), NULL) != 0) {
		ak_print_error_ex("mutex init failed\n");
		return -1;
	}
	/* init channel status */
	chunnel->status.send = 0;
	chunnel->status.schedule_record = 0;
	chunnel->status.schedule_motiondetect = 0;
	chunnel->status.motion_alarm_record = 0;
	chunnel->status.io_alarm_record = 0;
	chunnel->status.motion_alarming = 0;
	chunnel->status.io_alarming = 0;
	chunnel->motion_alarm = 0;
	chunnel->io_alarm = 0;
	chunnel->io_alarm_timeout = 0;
	chunnel->enable_preset_scan = 0;
	chunnel->cur_preset_scan = 0;
	chunnel->preset_scan_timeout = 0;
	chunnel->video_lost = 0;
	chunnel->video_lost_alarm_times = 10;
	chunnel->handle_name_osd = -1;
	chunnel->handle_name_osd_small = -1;
	chunnel->handle_date_osd = -1;
	chunnel->handle_date_osd_small = -1;
	chunnel->osd_name_changed = 1;
	chunnel->camera_name_changed = 1;

	// fix me
	// config_load_camera (&(chunnel->camera), index);
	/* allocate main and sub channel video send buffer */
	chunnel->video_send_buf = (unsigned char *)calloc(1, 
			MAX_VIDEO_ENCODE_BUF_SIZE);
	if (NULL == chunnel->video_send_buf)
		return -1;

	chunnel->video_send_buf_small = (unsigned char *)calloc(1,
		   	MAX_VIDEO_ENCODE_BUF_SIZE / 4);
   	if (NULL == chunnel->video_send_buf_small)
		return -1;

	return 0;
}

/*
 * dvs_cleanup_chunnel - clean up channel resource
 * chunnel[IN]: pointer to chunnel
 */
static void dvs_cleanup_chunnel(chunnel_t *chunnel)
{
	if (chunnel->video_send_buf)
		free(chunnel->video_send_buf);

   	if (chunnel->video_send_buf_small)
		free(chunnel->video_send_buf_small);

	ak_thread_mutex_destroy(&(chunnel->mutex_buf));
	ak_thread_mutex_destroy(&(chunnel->mutex));
}

#if 0
static int dvs_init_client (client_t * client)
{
	int i;
	client->used = 0;
	client->timeout = 0;
	client->sock_fd = -1;
	client->send_pos = 0;
	client->send_len = 0;
	client->recv_pos = 0;
	client->recv_len = 0;
	client->logined = 0;

	memset (&(client->cur_user), 0, sizeof (config_user_t));
	for (i = 0; i < MAX_CHUNNEL_NUM; i++)
	{
		client->chunnel[i] = 0;
		client->chunnel_small[i] = 0;
		client->audio_chunnel[i] = 0;
		client->record_video_canceled[i] = 0;
		client->record_audio_canceled[i] = 0;
		client->get_motion_alarm[i] = 0;
	}
	if (ak_thread_mutex_init (&(client->mutex_buf), NULL) != 0)
	{
		return -1;
	}
	if (ak_thread_mutex_init (&(client->mutex_send), NULL) != 0)
	{
		return -1;
	}

	client->send_buf = (unsigned char *) malloc (SEND_BUF_SIZE);
	if (NULL == client->send_buf)
		return -1;
	memset (client->send_buf, 0, SEND_BUF_SIZE);

	client->sending_buf = (unsigned char *) malloc (SEND_BUF_SIZE);
	if (NULL == client->sending_buf)
		return -1;
	memset (client->sending_buf, 0, SEND_BUF_SIZE);

	client->recv_buf = (unsigned char *) malloc (RECV_BUF_SIZE);
	if (NULL == client->recv_buf)
		return -1;
	memset (client->recv_buf, 0, RECV_BUF_SIZE);

	if (dvs_init_buffers
			(&(client->video_send_bufs), MAX_VIDEO_ENCODE_BUF_SIZE, 10) == -1)
		return -1;

	if (dvs_init_buffers
			(&(client->audio_send_bufs), MAX_AUDIO_BUF_SIZE, 20) == -1)
		return -1;
	return 0;
}

int dvs_init (dvs_t * dvs)
{
	int i;

	if (NULL == dvs)
		return -1;

	if (ak_thread_mutex_init (&(dvs->mutex_mark_date)) != 0)
	{
		return -1;
	}
	if (ak_thread_mutex_init (&(dvs->mutex_serialport)) != 0)
	{
		return -1;
	}

	for (i = 0; i < dvs->ch_num; i++)
	{
		if (dvs_init_chunnel (&(dvs->chunnel[i]), i) == -1)
			return -1;
	}

	for (i = 0; i < MAX_CLIENT_NUM; i++)
	{
		if (dvs_init_client (&(dvs->client[i])) == -1)
			return -1;
	}

	return 0;
}


static int point_in_rect (int x, int y, rect_t * rect)
{
	if ((x >= rect->left) && (x <= (rect->left + rect->width)) && (y >= rect->top) && (y <= (rect->top + rect->height)))
	{
		return 1;
	}
	return 0;
}
#endif


/*
 * dvs_video_dispense - main channel video dispense thread
 *         get one node of stream from main channel stream buffer,
 *         do stream dispense: send stream to all active client send buffer.
 * data[IN]: thread argument
 * return: when thread exit, return NULL
 */
static void *dvs_video_dispense(void *data)
{
	int index, i;
	unsigned char *videobuf;
	char headstr[10];
	char headstr_small[10];
	dvs_params_t *params;
	dvs_t *dvs;
	buffers_t *send_bufs;
	int length;
	int boundarylen = strlen("$$boundary\r\n");
	int frame_type = 0;
	int chunnel = 0;
	int frame_index = 0;
	unsigned int pts;
	params = (dvs_params_t *)data;
	dvs = params->dvs;
	index = params->index;
	ak_thread_mutex_unlock(&(params->mutex));

	videobuf = dvs->chunnel[index].video_send_buf;
	send_bufs = &(dvs->chunnel[index].video_send_bufs);
	memset (headstr, 0, 10);
	sprintf (headstr, "206^%d^", index);
	memset (headstr_small, 0, 10);
	sprintf (headstr_small, "244^%d^", index);
	int headlen = strlen (headstr);

	ak_thread_set_name("dvs_video_main");

	while (1) {
		length = 0;
		/* get data from internal buffer to main channel stream send buffer */
		buffers_get_data_with_pts(videobuf + headlen + 4, &length, 
				send_bufs, &frame_type, &chunnel, &frame_index, &pts);
		/* get data, length will bigger than 1 */
		if (length > 1) {
			memcpy(videobuf, headstr, headlen);
			memcpy(videobuf + headlen, &(length), 4);
			memcpy(videobuf + length + headlen + 4, "$$boundary\r\n", boundarylen);
			length += boundarylen + headlen + 4;
#if 1
			/* travel all user */
			for (i = 0; i < MAX_RTSP_CLIENT_NUM; i++) {
				if ((dvs->rtsp_client[i].used == 1)
						&& (dvs->rtsp_client[i].chunnel == chunnel)) {
					//I-Frame
					if (frame_type == 0x02) {
						/* put I-frame to channel's internal buffer */
						buffers_put_iframe_data_with_pts(videobuf + headlen + 4,
								length - headlen - 4 - boundarylen,
								&(dvs->rtsp_client[i].video_send_bufs),
								frame_type, chunnel, frame_index, pts);
					} else { 
						/* put p-frame to channel's internal buffer */
						buffers_put_pframe_data_with_pts(videobuf + headlen + 4,
								length - headlen - 4 - boundarylen,
								&(dvs->rtsp_client[i].video_send_bufs),
								frame_type, chunnel, frame_index, pts);
					}
				}
			}
#endif
		} else	/* no data, sleep */
			usleep(10*1000);
	}
	ak_thread_exit();
	return NULL;
}

/*
 * dvs_video_dispense_small - small video dispense thread
 *         get one node of stream from small stream buffer,
 *         do stream dispense: send stream to all active client send buffer.
 * data[IN]: thread argument
 * return: when thread exit, return NULL
 */
void *dvs_video_dispense_small(void *data)
{
	int index, i;
	unsigned char *videobuf;
	char headstr[10];
	char headstr_small[10];
	dvs_params_t *params;
	dvs_t *dvs;
	buffers_t *send_bufs_small;
	int length;
	int boundarylen = strlen("$$boundary\r\n");
	int frame_type = 0;
	int chunnel = 0;
	int frame_index = 0;
	unsigned int pts;
	params = (dvs_params_t *)data;
	dvs = params->dvs;
	index = params->index;
	ak_thread_mutex_unlock(&(params->mutex));

	videobuf = dvs->chunnel[index].video_send_buf_small;
	send_bufs_small = &(dvs->chunnel[index].video_send_bufs_small);
	memset (headstr, 0, 10);
	sprintf (headstr, "206^%d^", index);
	memset (headstr_small, 0, 10);
	sprintf (headstr_small, "244^%d^", index);
	int headlen = strlen (headstr);

	while (1) {
		length = 0;
		/* 
		 * get data from internal buffer to small stream send buffer
		 * copy send_bufs_small data to videobuf
		 */
		buffers_get_data_with_pts(videobuf + headlen + 4, 
				&length, send_bufs_small, &frame_type,
				&chunnel, &frame_index, &pts);
		if (length > 1) {
			memcpy(videobuf, headstr_small, headlen);
			memcpy(videobuf + length + headlen + 4, "$$boundary\r\n", boundarylen);
			memcpy(videobuf + headlen, &(length), 4);
			length += boundarylen + headlen + 4;
			//rtsp
#if 1
			/* travel all user */
			for (i = 0; i < MAX_RTSP_CLIENT_NUM; i++) {
				if ((dvs->rtsp_client[i].used == 1)
						&& (dvs->rtsp_client[i].chunnel == 1)) {  //小码流
					if (frame_type == 0x02)	{ //I-Frame 
						/* put I-frame to channel's internal buffer */
						buffers_put_iframe_data_with_pts(videobuf + headlen + 4,
								length - headlen - 4 - boundarylen,
								&(dvs->rtsp_client[i].video_send_bufs),
								frame_type, chunnel+ MAX_CHUNNEL_NUM,
								frame_index, pts);
					} else {
						/* put p-frame to channel's internal buffer */
						buffers_put_pframe_data_with_pts(videobuf + headlen + 4,
								length - headlen - 4 - boundarylen,
								&(dvs->rtsp_client[i].video_send_bufs),
								frame_type, chunnel+ MAX_CHUNNEL_NUM,
								frame_index, pts);
					}
				}
			}
#endif
		} else	/* no data, sleep */
			usleep(10*1000);
	}
	return (void *) NULL;
}

/* 
 * rtsp_init_video_dispense_service - init video dispense threads
 * 				it will create main ans sub channel's threads
 * return: 0 on success, -1 failed.
 */
static int rtsp_init_video_dispense_service(void)
{
	int i;
	ak_pthread_t thread;

    // dvs video dispense thread init
    ak_print_normal_ex("init video dispense\n");
	for (i = 0; i < MAX_CHUNNEL_NUM; i++) {
		ak_thread_mutex_lock(&(g_param.mutex));
		g_param.index = i;
		g_param.dvs = &g_dvs;
		if (ak_thread_create(&(thread), dvs_video_dispense,
				   	&g_param, 100*1024, 80) != 0) {
			ak_print_error_ex("create dvs_video_dispense %d error.\r\n", i);
			return -1;
		}
		/* this 'g_param.mutex' unlock when dvs_video_dispense has run */
#if 1
		ak_thread_mutex_lock(&(g_param.mutex));
		g_param.index = i;
		g_param.dvs = &g_dvs;
		if (ak_thread_create(&(thread), dvs_video_dispense_small, 
					&g_param, 100*1024, 80) != 0) {
			ak_print_error_ex("create dvs_video_dispense_small %d error.\r\n", i);
			return -1;
		}
		/* this 'g_param.mutex' unlock when dvs_video_dispense_small has run */
#endif
	}

	return 0;
}

static int rtsp_init_http_service(void)
{
	//http
#if 0
	int i;
	ak_pthread_t thread;
	for (i = 0; i < MAX_HTTP_CLIENT_NUM; i++) {
		ak_thread_mutex_lock (&(g_param.mutex));
		g_param.index = i;
		g_param.dvs = &g_dvs;
		http_init_client (&(g_dvs.http_client[i]));
		if (pthread_create(&(g_dvs.http_client[i].recv_thread),
				 NULL, http_client_recv, &g_param) != 0) {
			printf ("create send thread %d error.\r\n", i);
			return -1;
		}
	}
	if (pthread_create(&thread, NULL, http_client_listen, &g_dvs) != 0) {
		printf ("create http_client_listen thread error.\r\n");
		return -1;
	}
#endif
	return 0;
}

static int rtsp_init_client_service(void)
{
	int i;
	ak_pthread_t thread;

	ak_print_normal_ex("start rtsp server\n");
	for (i = 0; i < MAX_RTSP_CLIENT_NUM; i++) {
		/* init client info */
		if (rtsp_init_client(&(g_dvs.rtsp_client[i])) < 0) {
			ak_print_error_ex("rtsp client init failed, %d\n", i);
			return -1;
		}
		g_dvs.rtsp_client[i].index = i;
		ak_thread_mutex_lock(&(g_param.mutex));
		g_param.index = i;
		g_param.dvs = &g_dvs;
		/* receive client command */
		if (ak_thread_create(&thread, rtsp_client_recv, 
					&g_param, 100*1024, -1) != 0) {
			ak_print_error_ex("create rtsp_client_recv %d error.\r\n", i);
			ak_thread_mutex_unlock(&(g_param.mutex));
			return -1;
		}

		ak_thread_mutex_lock(&(g_param.mutex));
		/* send stream data */
		if (ak_thread_create(&thread, rtsp_client_send,
					&g_param, 100*1024, 80) != 0) {
			ak_print_error_ex("create rtsp_client_send %d error.\r\n", i);
			ak_thread_mutex_unlock(&(g_param.mutex));

			return -1;
		}
#if 0
		//set thread priority
		struct sched_param param = {0};
		param.sched_priority = 90;
		int cret = pthread_setschedparam(thread, SCHED_RR, &param);
		if (cret)
			printf("set sched param failed: %s\n", strerror(cret));
		else
			printf("set sched param ok, tid: %ld\n", thread);
		//test end
#endif
	}
	/* linstening play client connect request */
    if (ak_thread_create(&thread, rtsp_client_listen, 
				&g_dvs, 16*1024, 70) != 0) {
        ak_print_error_ex("create client_listen thread error.\r\n");
        return -1;
    }

	return 0;
}

/**
 * rtsp_init - init rtsp service
 * return: 0 success; -1 failed.
 */
int rtsp_init(void)
{
    int i;
	/* init client channel */
    for (i = 0; i < MAX_CHUNNEL_NUM; i++) {
    	if (dvs_init_chunnel(&(g_dvs.chunnel[i]), i) == -1) {
    	    ak_print_error_ex("init chunnel[%d] fail\n", i);
			dvs_cleanup_chunnel(&(g_dvs.chunnel[i]));
    		return -1;
    	}
    }
	/* init video dispense service */
	rtsp_init_video_dispense_service();
	/* init http service */
	rtsp_init_http_service();
	/* init client service */
	rtsp_init_client_service();

	return 0;
}

/*
* rtsp_set_chunnel_name
* chn[IN]: rtsp chunnel
* name[IN]: pointer to url name
* return: 0 success; -1 failed.
*/
int rtsp_set_chunnel_name(int ch, char *name)
{
    int len;

	/* check channel range */
    if (ch >= MAX_CHUNNEL_NUM) {
       printf("chunnel max num %d set %d\n", 
			   MAX_CHUNNEL_NUM, ch); 
       return -1;
    }

    len = strlen(name);
    if (len >= MAX_NAME_LEN) {
        printf("set_chunnel_name is too long");
        return -1;
    }

	/* store sub-fix name */
    memcpy(g_dvs.chunnel[ch].name, name, strlen(name)+1);
    return 0;
}

int rtsp_set_media_type(int ch, int type)
{
	/* check channel range */
    if (ch >= MAX_CHUNNEL_NUM) {
       printf("chunnel max num %d set %d\n", 
			   MAX_CHUNNEL_NUM, ch); 
       return -1;
    }
	if (type > 2 || type < 0) {
       printf("unsupported type %d\n", type); 
       return -1;
	}

	/* store media type */
    g_dvs.chunnel[ch].media_type = type;
	ak_print_notice_ex("* channel %d media-type:%d\n", ch, type);

    return 0;
}
