/*************************************************

  dvs.c
  used by dvs

 **************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <math.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/reboot.h>
#include <signal.h>

#include <net/if.h>

#include "dvs.h"
#include "rtsp.h"
#include "http_service.h"
#include "ak_onvif.h"
#include "hal_ak3918.h"

dvs_t g_dvs;
dvs_params_t g_param;

static int findStrFromBuf (const char *buf, int buflen, const char *findingstr, int findinglen)
{
	int i, j;

	for (i = 0; i < buflen - findinglen + 1; i++)
	{
		for (j = 0; j < findinglen; j++)
		{
			if (buf[i + j] != findingstr[j])
				break;
			if (j == findinglen - 1)
				return i;
		}
	}

	return -1;
}

static int get_message (char *buf, char *result, char *left_str, char *right_str)
{
	int pos, pos1;
	int left_len = strlen (left_str);
	int right_len = strlen (right_str);
	int buflen = strlen (buf);
	pos = findStrFromBuf (buf, buflen, left_str, left_len);
	if (pos < 0)
	{
		return -1;
	}
	pos1 = findStrFromBuf (buf + pos, buflen - pos, right_str, right_len);
	if (pos1 < 0)
	{
		return -1;
	}
	memcpy (result, buf + pos + left_len, pos1 - left_len);
	return 0;
}

static int readmac(char *addr)
{
	/* create socket to INET kernel */
	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd == -1) {
		printf("socket: %s\n", strerror(errno));
		return -1;
	}

	/* variable define */
	int ret = 0, len = 18;
	char *iface = "eth0";
	struct ifreq req;

	/* clean up req and set interface name */
	bzero(&req, sizeof(struct ifreq));
	strncpy(req.ifr_name, iface, IFNAMSIZ - 1);
	/* do get mac addr */
	if (ioctl(sockfd, SIOCGIFHWADDR, &req) >= 0) {
		snprintf(addr, len, "%02x:%02x:%02x:%02x:%02x:%02x",
				(unsigned char)req.ifr_hwaddr.sa_data[0],
				(unsigned char)req.ifr_hwaddr.sa_data[1],
				(unsigned char)req.ifr_hwaddr.sa_data[2],
				(unsigned char)req.ifr_hwaddr.sa_data[3],
				(unsigned char)req.ifr_hwaddr.sa_data[4],
				(unsigned char)req.ifr_hwaddr.sa_data[5]);
	} else
		ret = -1;

	return ret;
}

static int readip(char *addr)
{
	FILE *fd;
	int i = 0, j,len;
	char buf[1024];
	char tmp[30];
	//char ip[16];
	// system("ifconfig > /tmp/eth_msg");
	memset(tmp, 0, 30);
	fd = fopen("/tmp/eth_msg", "r");
	while ((fgets(buf, 1024, fd)) != NULL)
	{
		if (0 == get_message(buf, tmp, "addr:", " Bcast"))
		{
			break;
		}
	}
	fclose(fd);
	len = strlen(tmp);
	for (i = 0, j = 0; i < len; i++)
	{
		if (tmp[i] == ' ')
		{
			continue ;
		}
		else
		{
			addr[j++] = tmp[i];
		}
	}
	addr[j] = '\0';

	return 0;
}

static int readmask(char *addr)
{
	FILE *fd;
	//int i = 0;
	char buf[1024];
	//char tmp[30];
	// system("ifconfig > /tmp/eth_msg");
	fd = fopen("/tmp/eth_msg", "r");
	while ((fgets(buf, 1024, fd)) != NULL)
	{
		if (0 == get_message(buf, addr, "Mask:", "\n"))
		{
			break;
		}
	}
	fclose(fd);

	return 0;
}

static int readgetway(char *addr)
{
	FILE *fd;
	char buf[1024];
	char tmp[64];
	int i,j,len;
	// system("route | grep default > /tmp/getway");
	memset(tmp, 0, 64);
	fd = fopen("/tmp/getway", "r");
	while ((fgets(buf, 1024, fd)) != NULL)
	{
		if (0 == get_message(buf, tmp, "default", "0.0.0.0"))
		{
			break;
		}
	}
	fclose(fd);
	len = strlen(tmp);
	for (i = 0, j = 0; i < len; i++)
	{
		if (tmp[i] == ' ')
		{
			continue ;
		}
		else
		{
			addr[j++] = tmp[i];
		}
	}
	addr[j] = '\0';

	return 0;
}

static int readdns1(char *addr)
{
	FILE *fd;
	char buf[1024];
	// system("cat /etc/resolv.conf > /tmp/dns");
	fd = fopen("/tmp/dns", "r");
	while ((fgets(buf, 1024, fd)) != NULL)
	{
		if (0 == get_message(buf, addr, "nameserver ", "\n"))
		{
			break;
		}
	}
	fclose(fd);

	return 0;
}

static int readdns2(char *addr)
{
	FILE *fd;
	int i = 0, j = 0, pos = -1;
	char buf[1024];
	int len = strlen("nameserver ");

	// system("cat /etc/resolv.conf > /tmp/dns");
	fd = fopen("/tmp/dns", "r");
	while ((fgets(buf, 1024, fd)) != NULL)
	{
		if (0 == i)
		{
			i = 1;
			continue ;
		}
#if 0
		if (0 == get_message(buf, addr, "nameserver ", "\n") && i == 0)
		{
			i = 1;
			continue ; // break;
		}

		if (0 == get_message(buf, addr, "nameserver ", "\n") && i == 1)
		{
			break ; // break;
		}
#endif
		pos = findStrFromBuf(buf, strlen(buf), "nameserver ", strlen("nameserver "));
		for (j = 0; j < 15; j++)
		{
			if (('.' == buf[pos + len + j]) || ('0' <= buf[pos + len + j] && '9' >= buf[pos + len + j]))
			{
				addr[j] = buf[pos+ len + j];
				//printf("addr[%d]=%c\n", j, addr[j]);
			}
			else
			{
				addr[j] = '\0';
				break;
			}
		}
	}
	//printf("addr=%s\n", addr);
	fclose(fd);

	return 0;
}

void buffers_clear (dvs_t * dvs, buffers_t * bufs, int chunnel)
{
	int i;
	pthread_mutex_lock (&(bufs->mutex));
	for (i = 0; i < bufs->bufnum - 1; i++)
	{
		if (bufs->buf[i].chunnel == chunnel)
		{
			bufs->buf[i].chunnel = -1;
			if (bufs->buf[i].start != NULL)
			{
				free (bufs->buf[i].start);
				bufs->buf[i].start = NULL;
			}
			bufs->buf[i].length = 0;
		}
	}
	pthread_mutex_unlock (&(bufs->mutex));
}

static int buffers_has_previous_frame (buffers_t * bufs, int chunnel, int frame_index)
{
	int i, j;
	for (i = 0; i < bufs->bufnum - 1; i++)
	{
		j = bufs->rear - i - 1;
		if (j < 0)
			j = bufs->bufnum + j;
		if (bufs->buf[j].chunnel == chunnel)
		{
			if (bufs->buf[j].frame_index == frame_index - 1)
			{
				return 1;
			}
			if (bufs->buf[j].type == 0x02)	//I-Frame
			{
				return 0;
			}
		}
	}
	return 0;
}

int buffers_put_pframe_data (void *data, int length, buffers_t * bufs, int type, int chunnel, int frame_index)
{
	int res = 0;
	pthread_mutex_lock (&(bufs->mutex));
	if (buffers_has_previous_frame (bufs, chunnel, frame_index) == 0)
	{
		pthread_mutex_unlock (&(bufs->mutex));
		return 0;
	}
	if (((bufs->rear + 1) % bufs->bufnum) == bufs->front)
	{
		res = 0;
	}
	else
	{

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
			printf ("Out of memory, buffers_put_pframe_data:%d\r\n", __LINE__);fflush(NULL);
			exit (0);
		}
		memcpy (bufs->buf[bufs->rear].start, data, length);
		bufs->rear = (bufs->rear + 1) % bufs->bufnum;
		res = 1;
	}
	pthread_mutex_unlock (&(bufs->mutex));
	return res;
}

int buffers_put_iframe_data (void *data, int length, buffers_t * bufs, int type, int chunnel, int frame_index)
{
	int i, j;
	int res = 0;
	pthread_mutex_lock (&(bufs->mutex));
	if (((bufs->rear + 1) % bufs->bufnum) == bufs->front)
	{
		for (i = 0; i < bufs->bufnum - 1; i++)
		{
			j = bufs->rear - i - 1;
			if (j < 0)
				j = bufs->bufnum + j;
			if (bufs->buf[j].type != 0x02)	//P-Frame
			{
				bufs->buf[j].length = length;
				bufs->buf[j].type = type;
				bufs->buf[j].chunnel = chunnel;
				bufs->buf[j].frame_index = frame_index;
				if (NULL != bufs->buf[j].start)
				{
					free (bufs->buf[j].start);
					bufs->buf[j].start = NULL;
				}
				bufs->buf[j].start = malloc (length);
				if (NULL == bufs->buf[j].start)
				{
					printf ("Out of memory, buffers_put_iframe_data:%d\r\n", __LINE__);fflush(NULL);
					exit (0);
				}
				memcpy (bufs->buf[j].start, data, length);
				pthread_mutex_unlock (&(bufs->mutex));
				return 1;
			}
			if (bufs->buf[j].chunnel == chunnel)	//Old I-Frame
			{
				bufs->buf[j].length = length;
				bufs->buf[j].type = type;
				bufs->buf[j].chunnel = chunnel;
				bufs->buf[j].frame_index = frame_index;
				if (NULL != bufs->buf[j].start)
				{
					free (bufs->buf[j].start);
					bufs->buf[j].start = NULL;
				}
				bufs->buf[j].start = malloc (length);
				if (NULL == bufs->buf[j].start)
				{
					printf ("Out of memory, buffers_put_iframe_data:%d\r\n", __LINE__);fflush(NULL);
					exit (0);
				}
				memcpy (bufs->buf[j].start, data, length);
				pthread_mutex_unlock (&(bufs->mutex));
				return 1;
			}
		}
	}
	else
	{
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
			printf ("Out of memory, buffers_put_iframe_data:%d\r\n", __LINE__);fflush(NULL);
			exit (0);
		}
		memcpy (bufs->buf[bufs->rear].start, data, length);
		bufs->rear = (bufs->rear + 1) % bufs->bufnum;
		res = 1;
	}
	pthread_mutex_unlock (&(bufs->mutex));
	return res;
}

int buffers_get_data (void *data, int *length, buffers_t * bufs, int *type, int *chunnel, int *frame_index)
{
	int res = 0;
	pthread_mutex_lock (&(bufs->mutex));
	if (bufs->front != bufs->rear)
	{
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
	pthread_mutex_unlock (&(bufs->mutex));
	return res;
}

int buffers_put_data (void *data, int length, buffers_t * bufs, int type, int chunnel, int frame_index)
{
	int res = 0;
	pthread_mutex_lock (&(bufs->mutex));

	if (((bufs->rear + 1) % bufs->bufnum) == bufs->front)
	{
		res = 0;
	}

	else
	{
		if (length < bufs->bufsize)
		{
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
				printf ("Out of memory, buffers_put_data:%d\r\n", __LINE__);fflush(NULL);
				exit (0);
			}
			memcpy (bufs->buf[bufs->rear].start, data, length);
			bufs->rear = (bufs->rear + 1) % bufs->bufnum;
		}
		res = 1;
	}

	pthread_mutex_unlock (&(bufs->mutex));
	return res;
}

int buffers_get_data_with_pts (void *data, int *length, buffers_t * bufs, int *type,
								int *chunnel, int *frame_index, unsigned int *pts)
{
	int res = 0;
	pthread_mutex_lock (&(bufs->mutex));
	if (bufs->front != bufs->rear)
	{
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
	pthread_mutex_unlock (&(bufs->mutex));
	return res;
}

int buffers_put_data_with_pts (void *data, int length, buffers_t * bufs, int type,
								int chunnel, int frame_index,unsigned int pts)
{
	int res = 0;
	pthread_mutex_lock (&(bufs->mutex));

	if (((bufs->rear + 1) % bufs->bufnum) == bufs->front)
	{
		res = 0;
	}

	else
	{
		if (length < bufs->bufsize)
		{
			bufs->buf[bufs->rear].length = length;
			bufs->buf[bufs->rear].type = type;
			bufs->buf[bufs->rear].chunnel = chunnel;
			bufs->buf[bufs->rear].frame_index = frame_index;
			bufs->buf[bufs->rear].pts = pts;
			if (NULL != bufs->buf[bufs->rear].start)
			{
				free (bufs->buf[bufs->rear].start);
				bufs->buf[bufs->rear].start = NULL;
			}
			bufs->buf[bufs->rear].start = malloc (length);
			if (NULL == bufs->buf[bufs->rear].start)
			{
				printf ("Out of memory, buffers_put_data_with_pts:%d\r\n", __LINE__);fflush(NULL);
				exit (0);
			}
			memcpy (bufs->buf[bufs->rear].start, data, length);
			bufs->rear = (bufs->rear + 1) % bufs->bufnum;
		}
		res = 1;
	}

	pthread_mutex_unlock (&(bufs->mutex));
	return res;
}

int buffers_put_pframe_data_with_pts (void *data, int length, buffers_t * bufs, int type,
									int chunnel, int frame_index, unsigned int pts)
{
	int res = 0;
	pthread_mutex_lock (&(bufs->mutex));
	if (buffers_has_previous_frame (bufs, chunnel, frame_index) == 0)
	{
		pthread_mutex_unlock (&(bufs->mutex));
		return 0;
	}
	if (((bufs->rear + 1) % bufs->bufnum) == bufs->front)
	{
		res = 0;
	}
	else
	{

		bufs->buf[bufs->rear].length = length;
		bufs->buf[bufs->rear].type = type;
		bufs->buf[bufs->rear].chunnel = chunnel;
		bufs->buf[bufs->rear].frame_index = frame_index;
		bufs->buf[bufs->rear].pts = pts;
		if (NULL != bufs->buf[bufs->rear].start)
		{
			free (bufs->buf[bufs->rear].start);
			bufs->buf[bufs->rear].start = NULL;
		}
		bufs->buf[bufs->rear].start = malloc (length);
		if (NULL == bufs->buf[bufs->rear].start)
		{
			printf ("Out of memory, buffers_put_pframe_data_with_pts:%d\r\n", __LINE__);fflush(NULL);
			exit (0);
		}
		memcpy (bufs->buf[bufs->rear].start, data, length);
		bufs->rear = (bufs->rear + 1) % bufs->bufnum;
		res = 1;
	}
	pthread_mutex_unlock (&(bufs->mutex));
	return res;
}

int buffers_put_iframe_data_with_pts(void *data, int length, buffers_t * bufs, int type,
										int chunnel, int frame_index, unsigned int pts)
{
	int i, j;
	int res = 0;
	pthread_mutex_lock (&(bufs->mutex));
	if (((bufs->rear + 1) % bufs->bufnum) == bufs->front)
	{
		printf("BUF FULL!\r\n");
		for (i = 0; i < bufs->bufnum - 1; i++)
		{
			j = bufs->rear - i - 1;
			if (j < 0)
				j = bufs->bufnum + j;
			if (bufs->buf[j].type != 0x02)	//P-Frame
			{
				bufs->buf[j].length = length;
				bufs->buf[j].type = type;
				bufs->buf[j].chunnel = chunnel;
				bufs->buf[j].frame_index = frame_index;
				bufs->buf[j].pts = pts;
				if (NULL != bufs->buf[j].start)
				{
					free (bufs->buf[j].start);
					bufs->buf[j].start = NULL;
				}
				bufs->buf[j].start = malloc (length);
				if (NULL == bufs->buf[j].start)
				{
					printf ("Out of memory, buffers_put_iframe_data_with_pts:%d\r\n", __LINE__);fflush(NULL);
					exit (0);
				}
				memcpy (bufs->buf[j].start, data, length);
				pthread_mutex_unlock (&(bufs->mutex));
				return 1;
			}
			if (bufs->buf[j].chunnel == chunnel)	//Old I-Frame
			{
				bufs->buf[j].length = length;
				bufs->buf[j].type = type;
				bufs->buf[j].chunnel = chunnel;
				bufs->buf[j].frame_index = frame_index;
				bufs->buf[j].pts = pts;
				if (NULL != bufs->buf[j].start)
				{
					free (bufs->buf[j].start);
					bufs->buf[j].start = NULL;
				}
				bufs->buf[j].start = malloc (length);
				if (NULL == bufs->buf[j].start)
				{
					printf ("Out of memory, buffers_put_iframe_data_with_pts:%d\r\n", __LINE__);fflush(NULL);
					exit (0);
				}
				memcpy (bufs->buf[j].start, data, length);
				pthread_mutex_unlock (&(bufs->mutex));
				return 1;
			}
		}
	}
	else
	{
		bufs->buf[bufs->rear].length = length;
		bufs->buf[bufs->rear].type = type;
		bufs->buf[bufs->rear].chunnel = chunnel;
		bufs->buf[bufs->rear].frame_index = frame_index;
		bufs->buf[bufs->rear].pts = pts;
		if (NULL != bufs->buf[bufs->rear].start)
		{
			free (bufs->buf[bufs->rear].start);
			bufs->buf[bufs->rear].start = NULL;
		}
		bufs->buf[bufs->rear].start = malloc (length);
		if (NULL == bufs->buf[bufs->rear].start)
		{
			printf ("Out of memory, buffers_put_iframe_data_with_pts:%d\r\n", __LINE__);fflush(NULL);
			exit (0);
		}
		memcpy (bufs->buf[bufs->rear].start, data, length);
		bufs->rear = (bufs->rear + 1) % bufs->bufnum;
		res = 1;
	}
	pthread_mutex_unlock (&(bufs->mutex));
	return res;
}

void buffers_clear_data (buffers_t * bufs)
{
	int i;
	pthread_mutex_lock (&(bufs->mutex));
	bufs->rear = 0;
	bufs->front = 0;
	for (i = 0; i < bufs->bufnum - 1; i++)
	{
		bufs->buf[i].chunnel = -1;
		if (bufs->buf[i].start != NULL)
		{
			free (bufs->buf[i].start);
			bufs->buf[i].start = NULL;
		}
		bufs->buf[i].length = 0;
	}
	pthread_mutex_unlock (&(bufs->mutex));
}

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

int dvs_init_buffers (buffers_t * bufs, int bufsize, int bufnum)
{
	int i;
	bufs->rear = 0;
	bufs->front = 0;
	bufs->bufsize = bufsize;
	if (bufnum > MAX_BUF_NUM)
	{
		bufs->bufnum = MAX_BUF_NUM;
	}
	else
	{
		bufs->bufnum = bufnum;
	}
	for (i = 0; i < bufs->bufnum; i++)
	{
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
	if (pthread_mutex_init (&(bufs->mutex), NULL) != 0)
	{
		return -1;
	}
	return 0;
}

static int dvs_init_chunnel (chunnel_t * chunnel, int index)
{
	//int i, j;
	memset (chunnel->name, 0, MAX_NAME_LEN);
	sprintf (chunnel->name, "camera-%d", index + 1);

	//video_bufs
	if (dvs_init_buffers
			(&(chunnel->video_send_bufs), MAX_VIDEO_ENCODE_BUF_SIZE, 8) == -1)
		return -1;

	if (dvs_init_buffers
			(&(chunnel->video_send_bufs_small), MAX_VIDEO_ENCODE_BUF_SIZE / 4,
			 8) == -1)
		return -1;

	if (pthread_mutex_init (&(chunnel->mutex), NULL) != 0)
	{
		return -1;
	}
	if (pthread_mutex_init (&(chunnel->mutex_buf), NULL) != 0)
	{
		return -1;
	}

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

	chunnel->video_send_buf =
		(unsigned char *) malloc (MAX_VIDEO_ENCODE_BUF_SIZE);
	if (NULL == chunnel->video_send_buf)
		return -1;
	memset (chunnel->video_send_buf, 0, MAX_VIDEO_ENCODE_BUF_SIZE);

	chunnel->video_send_buf_small =
		(unsigned char *) malloc (MAX_VIDEO_ENCODE_BUF_SIZE / 4);
	if (NULL == chunnel->video_send_buf_small)
		return -1;
	memset (chunnel->video_send_buf_small, 0, MAX_VIDEO_ENCODE_BUF_SIZE / 4);

	return 0;
}

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
	if (pthread_mutex_init (&(client->mutex_buf), NULL) != 0)
	{
		return -1;
	}
	if (pthread_mutex_init (&(client->mutex_send), NULL) != 0)
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

	if (pthread_mutex_init (&(dvs->mutex_mark_date), NULL) != 0)
	{
		return -1;
	}
	if (pthread_mutex_init (&(dvs->mutex_serialport), NULL) != 0)
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

#if 0
static int point_in_rect (int x, int y, rect_t * rect)
{
	if ((x >= rect->left) && (x <= (rect->left + rect->width)) && (y >= rect->top) && (y <= (rect->top + rect->height)))
	{
		return 1;
	}
	return 0;
}
#endif

void * dvs_video_dispense (void *data)
{
	int index, i;
	unsigned char *videobuf;
	char headstr[10];
	char headstr_small[10];
	dvs_params_t *params;
	dvs_t *dvs;
	buffers_t *send_bufs;
	buffers_t *send_bufs_small;
	int length;
	int boundarylen = strlen ("$$boundary\r\n");
	int frame_type = 0;
	int chunnel = 0;
	int frame_index = 0;
	unsigned int pts;
	params = (dvs_params_t *) data;
	dvs = params->dvs;
	index = params->index;
	pthread_mutex_unlock (&(params->mutex));

	videobuf = dvs->chunnel[index].video_send_buf;
	send_bufs = &(dvs->chunnel[index].video_send_bufs);
	send_bufs_small = &(dvs->chunnel[index].video_send_bufs_small);
	memset (headstr, 0, 10);
	sprintf (headstr, "206^%d^", index);
	memset (headstr_small, 0, 10);
	sprintf (headstr_small, "244^%d^", index);
	int headlen = strlen (headstr);
	while (1)
	{
		length = 0;
		while (buffers_get_data_with_pts (videobuf + headlen + 4, &length, send_bufs, &frame_type,
				 							&chunnel, &frame_index, &pts) == 1)
		{
			if (length > 1)
			{
				memcpy (videobuf, headstr, headlen);
				//      memcpy (videobuf + headlen,
				//            &(dvs->chunnel[index].encode_fps), 4);
				memcpy (videobuf + headlen, &(length), 4);
				memcpy (videobuf + length + headlen + 4, "$$boundary\r\n", boundarylen);
				length += boundarylen + headlen + 4;
//rtsp
#if 1
				for (i = 0; i < MAX_RTSP_CLIENT_NUM; i++)
				{
					if ((dvs->rtsp_client[i].used == 1)
							&& (dvs->rtsp_client[i].chunnel == 0))   //大码流
					{
						if (frame_type == 0x02)	//I-Frame
						{
							buffers_put_iframe_data_with_pts (videobuf + headlen + 4,
									length - headlen - 4 - boundarylen,
									&(dvs->rtsp_client[i].video_send_bufs),
									frame_type, chunnel,
									frame_index, pts);
						}
						else
						{
							buffers_put_pframe_data_with_pts (videobuf + headlen + 4,
									length - headlen - 4 - boundarylen,
									&(dvs->rtsp_client[i].video_send_bufs),
									frame_type, chunnel,
									frame_index, pts);
						}
					}
				}
#endif
			}
			usleep (1000);
		}

		usleep (10000);
	}
	return (void *) NULL;
}

void * dvs_video_dispense_small (void *data)
{
	int index, i;
	unsigned char *videobuf;
	char headstr[10];
	char headstr_small[10];
	dvs_params_t *params;
	dvs_t *dvs;
	buffers_t *send_bufs;
	buffers_t *send_bufs_small;
	int length;
	int boundarylen = strlen ("$$boundary\r\n");
	int frame_type = 0;
	int chunnel = 0;
	int frame_index = 0;
	unsigned int pts;
	params = (dvs_params_t *) data;
	dvs = params->dvs;
	index = params->index;
	pthread_mutex_unlock (&(params->mutex));

	videobuf = dvs->chunnel[index].video_send_buf_small;
	send_bufs = &(dvs->chunnel[index].video_send_bufs);
	send_bufs_small = &(dvs->chunnel[index].video_send_bufs_small);
	memset (headstr, 0, 10);
	sprintf (headstr, "206^%d^", index);
	memset (headstr_small, 0, 10);
	sprintf (headstr_small, "244^%d^", index);
	int headlen = strlen (headstr);
	while (1)
	{
		length = 0;
		while (buffers_get_data_with_pts
				(videobuf + headlen + 4, &length, send_bufs_small, &frame_type,
				 &chunnel, &frame_index, &pts) == 1)
		{
			if (length > 1)
			{
				// printf("Getting,length=%d\r\n",length);
				memcpy (videobuf, headstr_small, headlen);
				memcpy (videobuf + length + headlen + 4, "$$boundary\r\n",
						boundarylen);
				memcpy (videobuf + headlen, &(length), 4);
				length += boundarylen + headlen + 4;

#if 1  //rtsp
				for (i = 0; i < MAX_RTSP_CLIENT_NUM; i++)
				{
					if ((dvs->rtsp_client[i].used == 1)
							&& (dvs->rtsp_client[i].chunnel == 1))   //小码流
					{
						if (frame_type == 0x02)	//I-Frame
						{
							buffers_put_iframe_data_with_pts (videobuf + headlen + 4,
									length - headlen - 4 - boundarylen,
									&(dvs->rtsp_client[i].video_send_bufs),
									frame_type, chunnel+ MAX_CHUNNEL_NUM,
									frame_index, pts);
						}
						else
						{
							buffers_put_pframe_data_with_pts (videobuf + headlen + 4,
									length - headlen - 4 - boundarylen,
									&(dvs->rtsp_client[i].video_send_bufs),
									frame_type, chunnel+ MAX_CHUNNEL_NUM,
									frame_index, pts);
						}
					}
				}
#endif
			}
			usleep (1000);
		}
		usleep (10000);
	}
	return (void *) NULL;
}

#if 0
int init_network (dvs_t * dvs)
{
	char cmdstr[TEMP_STR_LEN];
	memset (cmdstr, 0, TEMP_STR_LEN);
	sprintf (cmdstr, "ifconfig eth0 down");
	printf ("%s\r\n", cmdstr);
	send_cmd (dvs, cmdstr);

	memset (cmdstr, 0, TEMP_STR_LEN);
	sprintf (cmdstr, "ifconfig eth0 %s netmask %s", dvs->config.network.ip, dvs->config.network.netmask);
	printf ("%s\r\n", cmdstr);
	send_cmd (dvs, cmdstr);

	memset (cmdstr, 0, TEMP_STR_LEN);
	sprintf (cmdstr, "route add default gw %s", dvs->config.network.gateway);
	printf ("%s\r\n", cmdstr);
	send_cmd (dvs, cmdstr);

	memset (cmdstr, 0, TEMP_STR_LEN);
	sprintf (cmdstr, "echo " "nameserver %s" ">/etc/resolv.conf", dvs->config.network.dns1);
	printf ("%s\r\n", cmdstr);
	send_cmd (dvs, cmdstr);

	memset (cmdstr, 0, TEMP_STR_LEN);
	sprintf (cmdstr, "echo " "nameserver %s" ">>/etc/resolv.conf", dvs->config.network.dns2);
	printf ("%s\r\n", cmdstr);
	send_cmd (dvs, cmdstr);
	return 0;
}
#endif

int init_onvif(int width, int height)
{
	char xip[20];
	signal (SIGPIPE, SIG_IGN);
	g_dvs.sensor = HM1375;
	int fps;
	int bitrate;
	int quality;
	int flage;
	//int main_small;
	int w, h;
	struct  netinfo_t resp;


	g_dvs.time_zone = IniReadInteger ("sys", "timezone", 72000, "/etc/jffs2/system.ini") - 12*3600;

	GetOnvifData(g_dvs.onvif_data);

	GetVideoPara(&fps, &bitrate, &quality, &flage, &w, &h, 0);
	g_dvs.chunnel[0].camera.videoprofile.framerate = fps;// 15;
	g_dvs.chunnel[0].camera.videoprofile.bitrate = bitrate; //2048;
	g_dvs.chunnel[0].camera.videoprofile.quality = quality;
	g_dvs.chunnel[0].camera.videoprofile.resolution = 0;

	g_dvs.video_width = w;
	g_dvs.video_height = h;
	g_dvs.video_width_small = w / 2;
	g_dvs.video_height_small = h / 2;

	GetVideoPara(&fps, &bitrate, &quality, &flage, &w, &h, 1);
	g_dvs.chunnel[0].camera.videoprofile_small.framerate = fps;// 15;
	g_dvs.chunnel[0].camera.videoprofile_small.bitrate = bitrate; // 512;
	g_dvs.chunnel[0].camera.videoprofile_small.quality = quality;
	g_dvs.chunnel[0].camera.videoprofile_small.resolution = 1;

	if (GetNet(&resp))
		return -1;

	if (resp.IpType == 0)
	{
		g_dvs.config.network.type = resp.IpType; //0; //0:Fixed IP  1:dynamic ip
		g_dvs.config.network.auto_get_dns = resp.IpType; //0; // 0:no  1:yes;
		sprintf((g_dvs.config.network.ip), "%s", resp.cIp);
		sprintf((g_dvs.config.network.netmask), "%s", resp.cNetMask);
		sprintf((g_dvs.config.network.gateway), "%s", resp.cGateway);
		sprintf((g_dvs.config.network.dns1), "%s", resp.cMDns);
		sprintf((g_dvs.config.network.dns2), "%s", resp.cSDns);

		init_network(&g_dvs);

		send_cmd(&g_dvs, "ifconfig > /tmp/eth_msg");
		usleep(200);memset(xip, 0, 20);
		readmac(xip);
		sprintf((g_dvs.macaddr), "%s", xip);
	}
	else
	{
		// Start DHCP
		g_dvs.config.network.type = 1;
		g_dvs.config.network.auto_get_dns = 1;

#if 0
		send_cmd(&g_dvs, "ifconfig > /tmp/eth_msg");
		usleep(200);memset(xip, 0, 20);
		readmac(xip); printf("Mac = %s \r\n", xip);fflush(NULL);
		sprintf((g_dvs.macaddr), "%s", xip);

		usleep(200);memset(xip, 0, 20);
		readip(xip);printf("IP = %s \r\n", xip);fflush(NULL);
		sprintf((g_dvs.config.network.ip), "%s", xip);

		usleep(200);memset(xip, 0, 20);
		readmask(xip);printf("Mask = %s \r\n", xip);fflush(NULL);
		sprintf((g_dvs.config.network.netmask), "%s", xip);

		send_cmd(&g_dvs, "route | grep default > /tmp/getway");
		usleep(200);memset(xip, 0, 20);
		readgetway(xip);printf("Getway = %s \r\n", xip);fflush(NULL);
		sprintf((g_dvs.config.network.gateway), "%s", xip);

		send_cmd(&g_dvs, "cat /etc/resolv.conf > /tmp/dns");
		usleep(200);memset(xip, 0, 20);
		readdns1(xip);printf("Dns1 = %s \r\n", xip);fflush(NULL);
		sprintf((g_dvs.config.network.dns1), "%s", xip);

		usleep(200);memset(xip, 0, 20);
		readdns2(xip);printf("Dns2 = %s, Dns2Len=%d.\r\n", xip, strlen(xip));fflush(NULL);
		sprintf((g_dvs.config.network.dns2), "%s", xip);
#else
		readmac(xip); printf("Mac = %s \r\n", xip);fflush(NULL);
		sprintf((g_dvs.macaddr), "%s", xip);

		sprintf((g_dvs.config.network.ip), "%s", resp.cIp);
		sprintf((g_dvs.config.network.netmask), "%s", resp.cNetMask);
		sprintf((g_dvs.config.network.gateway), "%s", resp.cGateway);
		sprintf((g_dvs.config.network.dns1), "%s", resp.cMDns);
		sprintf((g_dvs.config.network.dns2), "%s", resp.cSDns);
#endif
	}

	config_load_videocolor(&(g_dvs.chunnel[0].camera.videocolor), 0);
	hal_set_brightness (&g_dvs, 0, g_dvs.chunnel[0].camera.videocolor.brightness);
	hal_set_contrast (&g_dvs, 0, g_dvs.chunnel[0].camera.videocolor.contrast);
	hal_set_saturation (&g_dvs, 0, g_dvs.chunnel[0].camera.videocolor.saturation);

	if (dvs_init_chunnel (&(g_dvs.chunnel[0]), 0) == -1)
	{
		return -1;
	}
	printf("\033[1;31m""[%s:%d] init_onvif ok\n""\033[m", __func__, __LINE__);
	fflush(stdout);

	return 0;
}

void* start_onvif()
{
#if 1

	int i;
	pthread_t thread;

	printf("\033[1;31m""[%s:%d] start onvif \n""\033[m", __func__, __LINE__);
	fflush(stdout);

	for (i = 0; i < 1; i++)
	{
		pthread_mutex_lock (&(g_param.mutex));
		g_param.index = i;
		g_param.dvs = &g_dvs;
		if (pthread_create (&(thread), NULL, dvs_video_dispense, &g_param) != 0)
		{
			printf ("create dvs_video_dispense thread %d error.\r\n", i);
			return NULL;
		}

		pthread_mutex_lock (&(g_param.mutex));
		g_param.index = i;
		g_param.dvs = &g_dvs;
		if (pthread_create
				(&(thread), NULL, dvs_video_dispense_small, &g_param) != 0)
		{
			printf ("create dvs_video_dispense_small thread %d error.\r\n", i);
			return NULL;
		}
	}
#endif
	//http
#if 1
	for (i = 0; i < MAX_HTTP_CLIENT_NUM; i++)
	{
		pthread_mutex_lock (&(g_param.mutex));
		g_param.index = i;
		g_param.dvs = &g_dvs;
		http_init_client (&(g_dvs.http_client[i]));
		if (pthread_create (&(g_dvs.http_client[i].recv_thread),
				 NULL, http_client_recv, &g_param) != 0) {
			printf ("create send thread %d error.\r\n", i);
			return NULL;
		}
	}
	if (pthread_create (&thread, NULL, http_client_listen, &g_dvs) != 0)
	{
		printf ("create http_client_listen thread error.\r\n");
		return NULL;
	}
#endif

	//rtsp
#if 1
	printf("\033[1;31m""start rtsp..., each buf size: %d.\r\n""\033[m", SEND_BUF_SIZE);
	fflush(stdout);

	//MAX_RTSP_CLIENT_NUM = 8
	for (i = 0; i < MAX_RTSP_CLIENT_NUM; i++)
	{
		if (rtsp_init_client (&(g_dvs.rtsp_client[i])) < 0)
		{
			return NULL;
		}
		g_dvs.rtsp_client[i].index = i;
		pthread_mutex_lock (&(g_param.mutex));
		g_param.index = i;
		g_param.dvs = &g_dvs;
		if (pthread_create (&thread, NULL, rtsp_client_recv, &g_param) != 0)
		{
			printf ("create  rtsp_client_recv %d error.\r\n", i);
			return NULL;
		}

		pthread_mutex_lock (&(g_param.mutex));
		if (pthread_create (&thread, NULL, rtsp_client_send, &g_param) != 0)
		{
			printf ("create rtsp_client_send %d error.\r\n", i);
			return NULL;
		}
#if 0
		//test, set thread priority
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
	if (pthread_create (&thread, NULL, rtsp_client_listen, &g_dvs) != 0)
	{
		printf ("create client_listen thread error.\r\n");
		return NULL;
	}
#endif
	return NULL;
}

