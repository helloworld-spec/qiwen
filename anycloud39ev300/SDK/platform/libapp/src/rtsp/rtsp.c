/*************************************************

rtsp.c
used by rtsp

**************************************************/
#include <sys/time.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <arpa/inet.h>

#include "ak_common.h"
#include "ak_thread.h"
#include "ak_net.h"

#include "rtsp.h"

static char *session_str[] =
{
	"4DA587A7F0B1D7B2C417F1A94C2E20", 
	"4DA587A7F0B1D7B2C417F1A94C2E21", 
	"4DA587A7F0B1D7B2C417F1A94C2E22", 
	"4DA587A7F0B1D7B2C417F1A94C2E23", 
	"4DA587A7F0B1D7B2C417F1A94C2E24", 
	"4DA587A7F0B1D7B2C417F1A94C2E25", 
	"4DA587A7F0B1D7B2C417F1A94C2E26", 
	"4DA587A7F0B1D7B2C417F1A94C2E27", 
	"4DA587A7F0B1D7B2C417F1A94C2E28", 
	"4DA587A7F0B1D7B2C417F1A94C2E29", 
};


static int FindStrFromBuf (const char *buf, int buflen, const char *findingstr, int findinglen, int direction)
{
	int i, j;
	if (direction == 0)
	{
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
	}
	else
	{
		for (i = buflen - findinglen; i >= 0; i--)
		{
			for (j = 0; j < findinglen; j++)
			{
				if (buf[i + j] != findingstr[j])
					break;
				if (j == findinglen - 1)
					return i;
			}
		}
	}
	return -1;
}

static void RTPHeaderInit (RTP_HEADER_T * header)
{
	header->Version = 2;		//2 bit
	header->Paddig = 0;		//1 bit
	header->Extension = 0;	//1 bit
	header->CSRCCount = 0;	//4 bit
	header->Marker = 0;		//1 bit
	header->PayloadType = 0x60;	//7 bit, 0x 60 for all our application
	header->SequenceNumber = 0;	//16 bit
	header->TimeStamp = 0;	//32 bit
	header->SSRC = 1215552513;
	return;
}

static int MakeRTPHeader (unsigned char *Packet, RTP_HEADER_T * header)
{
	Packet[0] = (header->Version << 6) | (header->Paddig << 5) | (header->Extension << 4) | (header->CSRCCount);
	Packet[1] = (header->Marker << 7) | (header->PayloadType);
	Packet[2] = (header->SequenceNumber >> 8) & 0xff;
	Packet[3] = header->SequenceNumber & 0xff;
	Packet[4] = (header->TimeStamp >> 24) & 0xff;
	Packet[5] = (header->TimeStamp >> 16) & 0xff;
	Packet[6] = (header->TimeStamp >> 8) & 0xff;
	Packet[7] = header->TimeStamp & 0xff;
	Packet[8] = (header->SSRC >> 24) & 0xff;
	Packet[9] = (header->SSRC >> 16) & 0xff;
	Packet[10] = (header->SSRC >> 8) & 0xff;
	Packet[11] = header->SSRC & 0xff;

	return 12;
}

int MakeRTCP (rtsp_client_t * client, unsigned char *rtcp_buf, int timestamp) 
{
	int offset_msw = 0;
	struct ak_timeval tv;
	ak_get_ostime (&tv);
	memset (rtcp_buf, 0, 68);
	rtcp_buf[0] = 0x80;
	rtcp_buf[1] = 0xc8;
	rtcp_buf[2] = 0x00;
	rtcp_buf[3] = 0x06;
	rtcp_buf[4] = 0x48;		//SSRC = 1215552513
	rtcp_buf[5] = 0x73;
	rtcp_buf[6] = 0xdc;
	rtcp_buf[7] = 0x01;
	rtcp_buf[8] = ((offset_msw + tv.sec) & 0xff000000) >> 24;	//MSW
	rtcp_buf[9] = ((offset_msw + tv.sec) & 0x00ff0000) >> 16;
	rtcp_buf[10] = ((offset_msw + tv.sec) & 0x0000ff00) >> 8;
	rtcp_buf[11] = (offset_msw + tv.sec) & 0x000000ff;
	rtcp_buf[12] = 0x00;		//LSW                 
	rtcp_buf[13] = 0x00;
	rtcp_buf[14] = 0x00;
	rtcp_buf[15] = 0x00;
	rtcp_buf[16] = (timestamp & 0xff000000) >> 24;	//RTP timestamp
	rtcp_buf[17] = (timestamp & 0x00ff0000) >> 16;
	rtcp_buf[18] = (timestamp & 0x0000ff00) >> 8;
	rtcp_buf[19] = timestamp & 0x000000ff;
	rtcp_buf[20] = (client->pkg_sum & 0xff000000) >> 24;	//Sender's packet count
	rtcp_buf[21] = (client->pkg_sum & 0x00ff0000) >> 16;
	rtcp_buf[22] = (client->pkg_sum & 0x0000ff00) >> 8;
	rtcp_buf[23] = client->pkg_sum & 0x000000ff;
	rtcp_buf[24] = (client->payload_sum & 0xff000000) >> 24;	//Senders octect Count
	rtcp_buf[25] = (client->payload_sum & 0x00ff0000) >> 16;
	rtcp_buf[26] = (client->payload_sum & 0x0000ff00) >> 8;
	rtcp_buf[27] = client->payload_sum & 0x000000ff;
	rtcp_buf[28] = 0x81;
	rtcp_buf[29] = 0xca;
	rtcp_buf[30] = 0x00;
	rtcp_buf[31] = 0x04;
	rtcp_buf[32] = 0x48;		//SSRC = 1215552513
	rtcp_buf[33] = 0x73;
	rtcp_buf[34] = 0xdc;
	rtcp_buf[35] = 0x01;
	rtcp_buf[36] = 0x01;
	rtcp_buf[37] = 0x0e;
	rtcp_buf[38] = 0x55;
	rtcp_buf[39] = 0x6e;
	rtcp_buf[40] = 0x6e;
	rtcp_buf[41] = 0x61;
	rtcp_buf[42] = 0x6d;
	rtcp_buf[43] = 0x65;
	rtcp_buf[44] = 0x64;
	rtcp_buf[45] = 0x20;
	rtcp_buf[46] = 0x73;
	rtcp_buf[47] = 0x74;
	rtcp_buf[48] = 0x72;
	rtcp_buf[49] = 0x65;
	rtcp_buf[50] = 0x61;
	rtcp_buf[51] = 0x6d;

	return 0;
}

int rtsp_init_client(rtsp_client_t * client)
{
	client->used = 0;
	client->sock_fd = -1;
	client->recv_len = 0;
	client->rtp_port = 0;
	client->seq_no = 0;
	client->vops = 0;
	client->payload_sum = 0;
	client->pkg_sum = 0;
	client->timestamp = 0;
	client->ssrc = 1234567;
	client->client_changed = 0;
	client->chunnel = 0;
	memset (client->client_ip, 0, IP_ADDR_LEN);

	if (ak_thread_mutex_init (&(client->mutex_buf), NULL) != 0) {
		return -1;
	}
	if (ak_thread_mutex_init (&(client->mutex_send), NULL) != 0) {
		return -1;
	}

	client->rtp_buf = (unsigned char *)malloc(RTP_BUF_SIZE);
	if (NULL == client->rtp_buf)
		return -1;
	memset (client->rtp_buf, 0, RTP_BUF_SIZE);

	client->send_buf = (unsigned char *)malloc(SEND_BUF_SIZE);
	if (NULL == client->send_buf)
		return -1;
	memset (client->send_buf, 0, SEND_BUF_SIZE);

	client->sending_buf = (unsigned char *)malloc(SEND_BUF_SIZE);
	if (NULL == client->sending_buf)
		return -1;
	memset (client->sending_buf, 0, SEND_BUF_SIZE);

	client->recv_buf = (unsigned char *)malloc(RECV_BUF_SIZE);
	if (NULL == client->recv_buf)
		return -1;
	memset (client->recv_buf, 0, RECV_BUF_SIZE);

	if (dvs_init_buffers(&(client->video_send_bufs),
			   	MAX_VIDEO_ENCODE_BUF_SIZE, 20) == -1)
		return -1;

	// if (dvs_init_buffers(&(client->audio_send_bufs), MAX_AUDIO_BUF_SIZE, 20) == -1)
	// return -1;
	return 0;
}

#if 1
static int do_client_send (rtsp_client_t * client, char *buf, int buflen)
{
	int res;
	fd_set writefds;
	struct timeval tv;
	int ret;

	ak_thread_mutex_lock (&(client->mutex_send));
	if ((client->used == 1) && (client->sock_fd != -1))
	{
		//printf ("client_send_0\r\n");
		//printf("%d,%d,%d\r\n",client->sock_fd,buf,buflen);
		FD_ZERO (&writefds);
		FD_SET (client->sock_fd, &writefds);
		tv.tv_sec = 0;
		tv.tv_usec = 1000;
		ret = select (client->sock_fd + 1, NULL, &writefds, NULL, &tv);
		//printf ("ret %d ",ret);
        ret = 1;
		if (ret == 1)
		{
			// printf ("client_send_1\r\n");
			res = send (client->sock_fd, buf, buflen, 0);
            //printf ("sendlen %d buflen %d\n",res,buflen);
			if (res <= 0)
			{
				ak_thread_mutex_unlock (&(client->mutex_send));
				usleep(10000);
				printf ("client_send_1.1\r\n");
				return -1;
			}
		}
		else if (ret < 0)
		{
			printf ("client_send_2\r\n");
			client->used = -2;
			close (client->sock_fd);
			client->sock_fd = -1;
			ak_thread_mutex_unlock (&(client->mutex_send));
			return -1;
		}
	}
	else if (client->sock_fd != -1)
	{
		close (client->sock_fd);
		client->sock_fd = -1;
		printf ("client_send_4\r\n");
	}
	ak_thread_mutex_unlock (&(client->mutex_send));
	return 0;
}
#endif

static int get_message (char *buf, char *result, char *left_str, char *right_str)
{
	int pos, pos1;
	int left_len = strlen (left_str);
	int right_len = strlen (right_str);
	int buflen = strlen (buf);
	pos = FindStrFromBuf (buf, buflen, left_str, left_len,0);
	if (pos < 0)
	{
		return -1;
	}
	pos1 = FindStrFromBuf (buf + pos, buflen - pos, right_str, right_len,0);
	if (pos1 < 0)
	{
		return -1;
	}
	memcpy (result, buf + pos + left_len, pos1 - left_len);
	return 0;
}

void fill_rtp_head_info(unsigned char *rtp_buf, int rtp_len)
{
	rtp_buf[0] = 0x24;
	rtp_buf[1] = 0x00;
	rtp_buf[2] = rtp_len/256;
	rtp_buf[3] = rtp_len%256;
}

void set_rtp_header(RTP_HEADER_T *rtp_header, 
		rtsp_client_t *client, unsigned int pts, int mark)
{
	unsigned char rtp_head_buf[12];

	RTPHeaderInit(rtp_header);

	rtp_header->Marker = mark;
	rtp_header->SequenceNumber = client->seq_no;
	client->seq_no++;
	rtp_header->TimeStamp = pts;
	MakeRTPHeader (rtp_head_buf, rtp_header);
	memcpy (client->rtp_buf+4, rtp_head_buf, 12);
}

void set_hevc_nal_header(NALU_HEVC_HEADER *nalu_hevc_head, int type)
{
	/* h.265 nalu */
#if 0
	nalu_hevc_head->F = 0x0;
	nalu_hevc_head->TYPE = type;
	nalu_hevc_head->LayerId = 0x0;
	nalu_hevc_head->TID = 0x01;
#else
	nalu_hevc_head->TYPE = type << 1;
	nalu_hevc_head->TID = 0x01;
#endif
}

#if 1
int send_rtp_udp(rtsp_client_t * client, struct sockaddr *addr, struct sockaddr *addr1, 
						unsigned char *buf, int len, int frame_type, unsigned int pts)
{
	int frame_len, start, pkg_num;
	char nal_head[] = { 0x00, 0x00, 0x00, 0x01, };
	int addr_len = sizeof (struct sockaddr_in);
	int res, pos, pos1, pos2, rtp_len;
	unsigned char *ptr;
	unsigned char rtp_head_buf[12];
	unsigned char rtcp_buf[68];
	NALU_HEADER nalu_head;
	FU_INDICATOR fu_indictor;
	FU_HEADER fu_head;
	RTP_HEADER_T rtp_header;
	RTPHeaderInit (&rtp_header);

	if (frame_type == IFRAME) {
		pos = FindStrFromBuf((char *) buf, len, nal_head, 4,0);
		pos1 = FindStrFromBuf((char *) buf + pos + 4, len - pos - 4, nal_head, 4,0) + pos + 4;
		pos2 = FindStrFromBuf((char *) buf + pos1 + 4, len - pos1 - 4, nal_head, 4,0) + pos1 + 4;

		client->vops++;
		if (client->vops >= 20)
		{
			client->vops = 0;
			MakeRTCP (client, rtcp_buf, pts);
			res = sendto (client->fd_rtcp, rtcp_buf, 52, 0, addr1, addr_len);
			ak_print_info_ex("Send RTCP PKG,res=%d\r\n", res);
		}
		//sps
		RTPHeaderInit (&rtp_header);
		rtp_header.Marker = 0x01;
		rtp_header.SequenceNumber = client->seq_no;
		client->seq_no++;
		rtp_header.TimeStamp = pts;
		MakeRTPHeader (rtp_head_buf, &rtp_header);
		memcpy (client->rtp_buf, rtp_head_buf, 12);

		nalu_head.F = 0x00;
		nalu_head.NRI = 0x03;
		nalu_head.TYPE = 0x07;
		memcpy (&(client->rtp_buf[12]), &nalu_head, 1);
		memcpy (&(client->rtp_buf[13]), buf + pos + 5, pos1 - pos - 5);
		rtp_len = pos1 - pos - 5 + 13;
		sendto (client->fd_rtp, client->rtp_buf, rtp_len, 0, addr, addr_len);
		client->payload_sum += rtp_len - 13;
		client->pkg_sum++;
		// usleep (200);

		//pps
		RTPHeaderInit (&rtp_header);
		rtp_header.Marker = 0x01;
		rtp_header.SequenceNumber = client->seq_no;
		client->seq_no++;
		rtp_header.TimeStamp = pts;
		MakeRTPHeader (rtp_head_buf, &rtp_header);
		memcpy (client->rtp_buf, rtp_head_buf, 12);

		nalu_head.F = 0x00;
		nalu_head.NRI = 0x03;
		nalu_head.TYPE = 0x08;
		memcpy (&(client->rtp_buf[12]), &nalu_head, 1);
		memcpy (&(client->rtp_buf[13]), buf + pos1 + 5, pos2 - pos1 - 5);
		rtp_len = pos2 - pos1 - 5 + 13;
		sendto (client->fd_rtp, client->rtp_buf, rtp_len, 0, addr, addr_len);
		client->payload_sum += rtp_len - 13;
		client->pkg_sum++;
		// usleep (200);

		//IFrame Data
		frame_len = len - pos2 - 5;

		// printf("IFrame, len = %d\r\n", frame_len);
		if (frame_len > RTP_PKG_SIZE)	//?? 
		{
			start = 1;
			pkg_num = frame_len / RTP_PKG_SIZE;
			ak_print_info_ex("pkg_num: %d\n", pkg_num);
			ptr = buf + pos2 + 5;
			while (frame_len > 0)
			{
				RTPHeaderInit (&rtp_header);
				if (frame_len > RTP_PKG_SIZE)
				{
					rtp_header.Marker = 0x00;
				}
				else
				{
					rtp_header.Marker = 0x01;
				}
				rtp_header.SequenceNumber = client->seq_no;
				client->seq_no++;
				rtp_header.TimeStamp = pts;
				MakeRTPHeader (rtp_head_buf, &rtp_header);
				memcpy (client->rtp_buf, rtp_head_buf, 12);

				fu_indictor.F = 0x00;
				fu_indictor.NRI = 0x03;
				fu_indictor.TYPE = 28;
				memcpy (&(client->rtp_buf[12]), &fu_indictor, 1);

				fu_head.TYPE = 0x05;
				fu_head.S = 0x00;
				fu_head.E = 0x00;
				fu_head.R = 0x00;
				if (start == 1)
				{
					fu_head.S = 0x01;
					start = 0;
				}
				if (frame_len <= RTP_PKG_SIZE)
				{
					fu_head.E = 0x01;
				}

				memcpy (&(client->rtp_buf[13]), &fu_head, 1);

				memcpy (&(client->rtp_buf[14]), ptr, RTP_PKG_SIZE);
				if (frame_len > RTP_PKG_SIZE)
				{
					rtp_len = RTP_PKG_SIZE + 14;
				}
				else
				{
					rtp_len = frame_len + 14;
				}

				sendto (client->fd_rtp, client->rtp_buf, rtp_len, 0, addr,
						addr_len);
				client->payload_sum += rtp_len - 14;
				client->pkg_sum++;
				// usleep (200);

				ptr += RTP_PKG_SIZE;
				frame_len -= RTP_PKG_SIZE;
			}
		}
		else			//??
		{
			RTPHeaderInit (&rtp_header);
			rtp_header.Marker = 0x01;
			rtp_header.SequenceNumber = client->seq_no;
			client->seq_no++;
			rtp_header.TimeStamp = pts;
			MakeRTPHeader (rtp_head_buf, &rtp_header);
			memcpy (client->rtp_buf, rtp_head_buf, 12);

			nalu_head.F = 0x00;
			nalu_head.NRI = 0x03;
			nalu_head.TYPE = 0x05;
			memcpy (&(client->rtp_buf[12]), &nalu_head, 1);
			memcpy (&(client->rtp_buf[13]), buf + pos2 + 5, len - pos2 - 5);
			rtp_len = len - pos2 - 5 + 13;
			sendto (client->fd_rtp, client->rtp_buf, rtp_len, 0, addr,
					addr_len);
			client->payload_sum += rtp_len - 13;
			client->pkg_sum++;
			// usleep (200);
		}
		//printf("vops=%d\r\n",client->vops);

	}
	else	//PFrame
	{ 
		pos = FindStrFromBuf ((char *) buf, len, nal_head, 4,0);
		//  printf ("PFrame:pos=%d\r\n", pos);

		frame_len = len - pos - 5;
		pkg_num = frame_len / RTP_PKG_SIZE;
		ak_print_info_ex("PFrame:frame_len=%d, pkg_num: %d\r\n",
			   	frame_len, pkg_num);
		if (frame_len > RTP_PKG_SIZE)	//?? 
		{
			start = 1;
			ptr = buf + pos + 5;
			while (frame_len > 0)
			{
				RTPHeaderInit (&rtp_header);
				if (frame_len > RTP_PKG_SIZE)
				{
					rtp_header.Marker = 0x00;
				}
				else
				{
					rtp_header.Marker = 0x01;
				}
				rtp_header.SequenceNumber = client->seq_no;
				client->seq_no++;
				rtp_header.TimeStamp = pts;
				MakeRTPHeader (rtp_head_buf, &rtp_header);
				memcpy (client->rtp_buf, rtp_head_buf, 12);

				fu_indictor.F = 0x00;
				fu_indictor.NRI = 0x02;
				fu_indictor.TYPE = 28;
				memcpy (&(client->rtp_buf[12]), &fu_indictor, 1);

				fu_head.TYPE = 0x01;
				fu_head.S = 0x00;
				fu_head.E = 0x00;
				fu_head.R = 0x00;
				if (start == 1)
				{
					fu_head.S = 0x01;
					start = 0;
				}
				if (frame_len <= RTP_PKG_SIZE)
				{
					fu_head.E = 0x01;
				}

				memcpy (&(client->rtp_buf[13]), &fu_head, 1);

				memcpy (&(client->rtp_buf[14]), ptr, RTP_PKG_SIZE);
				if (frame_len > RTP_PKG_SIZE)
				{
					rtp_len = RTP_PKG_SIZE + 14;
				}
				else
				{
					rtp_len = frame_len + 14;
				}
				sendto (client->fd_rtp, client->rtp_buf, rtp_len, 0, addr, addr_len);
				client->payload_sum += rtp_len - 14;
				client->pkg_sum++;
				// usleep (200);
				ptr += RTP_PKG_SIZE;
				frame_len -= RTP_PKG_SIZE;
			}
		}
		else			//??
		{
			RTPHeaderInit (&rtp_header);
			rtp_header.Marker = 0x01;
			rtp_header.SequenceNumber = client->seq_no;
			client->seq_no++;
			rtp_header.TimeStamp = pts;
			MakeRTPHeader (rtp_head_buf, &rtp_header);
			memcpy (client->rtp_buf, rtp_head_buf, 12);
			nalu_head.F = 0x00;
			nalu_head.NRI = 0x02;
			nalu_head.TYPE = 0x01;
			memcpy (&(client->rtp_buf[12]), &nalu_head, 1);
			memcpy (&(client->rtp_buf[13]), buf + pos + 5, len - pos - 5);
			rtp_len = len - pos - 5 + 13;
			sendto (client->fd_rtp, client->rtp_buf, rtp_len, 0, addr, addr_len);
			client->payload_sum += rtp_len - 13;
			client->pkg_sum++;
			// usleep (200);
		} 
	}

	return 0;
}
#endif

static int send_hevc_rtp_tcp(rtsp_client_t * client, unsigned char *buf, 
		int len, int frame_type, unsigned int pts)
{
	char nal_head[] = {0x00, 0x00, 0x00, 0x01,};	//start code
	unsigned char *ptr;
	int frame_len, start, rtp_len = 0, marker = 0x00;
	/* to make it more readable */
	int start_code_len = sizeof(nal_head)/sizeof(nal_head[0]);
	int nal_head_byte = 2;	//on h.265 is 2B
	int rtp_len_byte = 4;
	int rtp_header_len = 12;

	NALU_HEVC_HEADER nalu_hevc_head;	//hevc
	FU_HEVC_HEADER fu_hevc_head;
	RTP_HEADER_T rtp_header;
	
	if (frame_type == IFRAME) {
		int vps_pos = 0, sps_pos = 0, pps_pos = 0, idr_pos = 0;
		/* 
		 * find start code 00 00 00 01, then get nalu position 
		 * bitstream structure(H265):
		 * |start-code(4B)|VPS|start-code(4B)|SPS|start-code(4B)|PPS|start-code(4B)|IDR|
		 * so, here, it record three posisions to mark the start posision of vps/sps/pps/idr
		 */
		vps_pos = FindStrFromBuf ((char *) buf, len, nal_head, start_code_len, 0); /* vps_pos should be 0 */
		sps_pos = FindStrFromBuf ((char *) buf + vps_pos + start_code_len, 
				len - vps_pos - start_code_len, 
				nal_head, start_code_len, 0) + vps_pos + start_code_len;	/* skip to next nalu */
		pps_pos = FindStrFromBuf ((char *) buf + sps_pos + start_code_len,
				len - sps_pos - start_code_len, 
				nal_head, start_code_len, 0) + sps_pos + start_code_len;	/* same as above */
		/* for h.265, because its has VPS before SPS */
		idr_pos = FindStrFromBuf ((char *) buf + pps_pos + start_code_len, 
				len - pps_pos - start_code_len, 
				nal_head, start_code_len, 0) + pps_pos + start_code_len;	/* same as above */

		client->vops++;
		if (client->vops >= 20) {
			unsigned char rtcp_buf[68];

			client->vops = 0;
			MakeRTCP (client, rtcp_buf + 4, pts);
			rtcp_buf[0] = 0x24;
			rtcp_buf[1] = 0x03;
			rtcp_buf[2] = 0x00;
			rtcp_buf[3] = 0x34;
			do_client_send (client, (char *)rtcp_buf, 56);
		}

		/* vps for hevc */
		marker = 0x01;
		set_rtp_header(&rtp_header, client, pts, marker);

		set_hevc_nal_header(&nalu_hevc_head, NAL_UNIT_VPS);
		/* on h.265, nalu is 2B, 12+4 -> 16B rtp-header,  */
		memcpy (&(client->rtp_buf[rtp_header_len + 4]), &nalu_hevc_head, nal_head_byte);	//copy nal head

		/* copy data */
		memcpy (&(client->rtp_buf[rtp_header_len + 4 + nal_head_byte]),
				buf + vps_pos + start_code_len + nal_head_byte,
			   	sps_pos - vps_pos - start_code_len - nal_head_byte);	//copy nalu data
		rtp_len = sps_pos - vps_pos - start_code_len + rtp_header_len;	// |rtp-header| + |NALU|

		fill_rtp_head_info(client->rtp_buf, rtp_len);
		do_client_send (client, (char *)client->rtp_buf, rtp_len + rtp_len_byte);
		client->payload_sum += rtp_len - rtp_header_len - nal_head_byte;
		client->pkg_sum++;

		//sps
		marker = 0x01;
		set_rtp_header(&rtp_header, client, pts, marker);

		set_hevc_nal_header(&nalu_hevc_head, NAL_UNIT_SPS);
		memcpy(&(client->rtp_buf[rtp_header_len + 4]), &nalu_hevc_head, nal_head_byte);	//copy nal head
		/* copy data */
		memcpy(&(client->rtp_buf[rtp_header_len + 4 + nal_head_byte]),
				buf + sps_pos + start_code_len + nal_head_byte,
				pps_pos - sps_pos - start_code_len - nal_head_byte);	//copy nalu data
		rtp_len = pps_pos - sps_pos - start_code_len + rtp_header_len;	// |rtp-header| + |NALU|

		fill_rtp_head_info(client->rtp_buf, rtp_len);
		do_client_send (client, (char *)client->rtp_buf, rtp_len + rtp_len_byte);
		client->payload_sum += rtp_len - rtp_header_len - nal_head_byte;
		client->pkg_sum++;

		//pps
		marker = 0x01;
		set_rtp_header(&rtp_header, client, pts, marker);

		set_hevc_nal_header(&nalu_hevc_head, NAL_UNIT_PPS);
		memcpy(&(client->rtp_buf[rtp_header_len + 4]), &nalu_hevc_head, nal_head_byte);	//copy nal head

		memcpy(&(client->rtp_buf[rtp_header_len + 4 + nal_head_byte]),
				buf + pps_pos + start_code_len + nal_head_byte,
				idr_pos - pps_pos - start_code_len - nal_head_byte);	//copy nalu data
		rtp_len = idr_pos - pps_pos - start_code_len + rtp_header_len;	// |rtp-header| + |NALU|

		fill_rtp_head_info(client->rtp_buf, rtp_len);
		do_client_send (client, (char *)client->rtp_buf, rtp_len + rtp_len_byte);
		client->payload_sum += rtp_len - rtp_header_len - nal_head_byte;
		client->pkg_sum++;

		/* send idr */
		frame_len = len - idr_pos - start_code_len - nal_head_byte; //on h.265
		if (frame_len > RTP_PKG_SIZE)	//fu packet
		{
			start = 1;
			ptr = buf + idr_pos + start_code_len + nal_head_byte;	//on h.265
			while (frame_len > 0) {
				/* init rpt head info */
				marker = 0x01;
				if (frame_len > RTP_PKG_SIZE) {
					marker = 0x00;
				} else {
					marker = 0x01;
				}
				set_rtp_header(&rtp_header, client, pts, marker);

				/* set fu payload */
				set_hevc_nal_header(&nalu_hevc_head, NAL_UNIT_UNSPECIFIED_49);
				memcpy (&(client->rtp_buf[12+4]), &nalu_hevc_head, 2);

				fu_hevc_head.S = 0x0;
				fu_hevc_head.E = 0x0;
				if (start == 1) {
					fu_hevc_head.S = 0x01;
					fu_hevc_head.TYPE = NAL_UNIT_CODED_SLICE_IDR;
					start = 0;
				} else
					fu_hevc_head.TYPE = NAL_UNIT_FILLER_DATA;

				if (frame_len <= RTP_PKG_SIZE) {
					fu_hevc_head.E = 0x01;
				}
				memcpy (&(client->rtp_buf[14+4]), &fu_hevc_head, 1);
				memcpy (&(client->rtp_buf[15+4]), ptr, RTP_PKG_SIZE);

				if (frame_len > RTP_PKG_SIZE) {
					rtp_len = RTP_PKG_SIZE + 15;	//15 = 12 + fu_hevc_indictor(2) + fu_hevc_head(1)
				} else {
					rtp_len = frame_len + 15;	//last packet
				}

				fill_rtp_head_info(client->rtp_buf, rtp_len);
				do_client_send (client, (char *)client->rtp_buf, rtp_len + rtp_len_byte);
				client->payload_sum += rtp_len - 15;
				client->pkg_sum++;
				// usleep (1);

				ptr += RTP_PKG_SIZE;
				frame_len -= RTP_PKG_SIZE;
			}
		}
		else			//single packet
		{
			marker = 0x01;
			set_rtp_header(&rtp_header, client, pts, marker);

			set_hevc_nal_header(&nalu_hevc_head, NAL_UNIT_CODED_SLICE_IDR);
			memcpy(&(client->rtp_buf[rtp_header_len + 4]), &nalu_hevc_head, nal_head_byte);	//copy nal head

			memcpy(&(client->rtp_buf[rtp_header_len + 4 + nal_head_byte]),
					buf + idr_pos + start_code_len + nal_head_byte,
					len - idr_pos - start_code_len - nal_head_byte);	//copy nalu payload data
			rtp_len = len - idr_pos - start_code_len - nal_head_byte + rtp_header_len;	// |i-data| + |NALU|

			fill_rtp_head_info(client->rtp_buf, rtp_len);
			do_client_send (client, (char *)client->rtp_buf, rtp_len+4);
			client->payload_sum += rtp_len - nal_head_byte - rtp_header_len;
			client->pkg_sum++;
		}

	}
	else				//PFrame
	{
		int pf_start_pos = FindStrFromBuf ((char *) buf, len, nal_head, 4, 0);
		frame_len = len - pf_start_pos - start_code_len - nal_head_byte;
		if (frame_len > RTP_PKG_SIZE)	//fu packet
		{
			start = 1;
			ptr = buf + pf_start_pos + start_code_len + nal_head_byte;
			while (frame_len > 0)
			{
				/* set rtp header */
				if (frame_len > RTP_PKG_SIZE)
					marker = 0x00;
				else
					marker = 0x01;
				set_rtp_header(&rtp_header, client, pts, marker);

				/* set fu payload */
				set_hevc_nal_header(&nalu_hevc_head, NAL_UNIT_UNSPECIFIED_49);
				memcpy (&(client->rtp_buf[12+4]), &nalu_hevc_head, 2);

				/* set fu header */
				fu_hevc_head.TYPE = NAL_UNIT_CODED_SLICE_TRAIL_R;	//1
				fu_hevc_head.S = 0x0;
				fu_hevc_head.E = 0x0;
				if (start == 1) {
					fu_hevc_head.S = 0x01;
					start = 0;
				}
				if (frame_len <= RTP_PKG_SIZE)
					fu_hevc_head.E = 0x01;
				memcpy (&(client->rtp_buf[14+4]), &fu_hevc_head, 1);
				memcpy (&(client->rtp_buf[15+4]), ptr, RTP_PKG_SIZE);

				if (frame_len > RTP_PKG_SIZE)
					rtp_len = RTP_PKG_SIZE + 15;	//15 = 12 + fu_hevc_indictor(2) + fu_hevc_head(1)
				else
					rtp_len = frame_len + 15;	//last packet

				fill_rtp_head_info(client->rtp_buf, rtp_len);
				do_client_send (client, (char *)client->rtp_buf, rtp_len+4);
				client->payload_sum += rtp_len - rtp_header_len - 1 - nal_head_byte;	//rtp_len - 15
				client->pkg_sum++;

				ptr += RTP_PKG_SIZE;
				frame_len -= RTP_PKG_SIZE;
			}
		}
		else			//single packet
		{
			/* init rtp header */
			marker = 0x01;
			set_rtp_header(&rtp_header, client, pts, marker);

			/* init nalu header, actually it is a payload */
			set_hevc_nal_header(&nalu_hevc_head, NAL_UNIT_CODED_SLICE_TRAIL_R);
			memcpy(&(client->rtp_buf[rtp_header_len + 4]), &nalu_hevc_head, nal_head_byte);	//copy nal head

			memcpy(&(client->rtp_buf[rtp_header_len + 4 + nal_head_byte]),
					buf + pf_start_pos + start_code_len + nal_head_byte,
					len - pf_start_pos - start_code_len - nal_head_byte);	//copy nalu payload data
			rtp_len = len - pf_start_pos - start_code_len - nal_head_byte + rtp_header_len;	// |i-data| + |NALU|

			fill_rtp_head_info(client->rtp_buf, rtp_len);
			do_client_send (client, (char *)client->rtp_buf, rtp_len+4);
			client->payload_sum += rtp_len - rtp_header_len - nal_head_byte;
			client->pkg_sum++;
		} 
	}

	return 0;
}

static int send_h264_rtp_tcp(rtsp_client_t * client, unsigned char *buf, 
		int len, int frame_type, unsigned int pts)
{
	char nal_head[] = {0x00, 0x00, 0x00, 0x01, };
	unsigned char *ptr;

	int frame_len, start, rtp_len = 0, marker = 0x00;
	/* to make it more readable */
	int start_code_len = sizeof(nal_head)/sizeof(nal_head[0]);
	int nal_head_byte = 1;	//on h.264 is 1B
	int rtp_len_byte = 4;
	int rtp_header_len = 12;

	NALU_HEADER nalu_head;
	FU_INDICATOR fu_indictor;
	FU_HEADER fu_head;
	RTP_HEADER_T rtp_header;
	
	if (frame_type == IFRAME)
	{
		int sps_pos = 0, pps_pos = 0, idr_pos = 0;
		/* 
		 * find start code 00 00 00 01, then get nalu position 
		 * bitstream structure(h.264):
		 * |start-code(4B)|SPS|start-code(4B)|PPS|start-code(4B)|IDR|
		 * so, here, it record three posisions to mark the start posision of sps/pps/idr
		 */
		sps_pos = FindStrFromBuf ((char *) buf, len, nal_head, start_code_len, 0); /* sps_pos should be 0 */
		pps_pos = FindStrFromBuf ((char *) buf + sps_pos + start_code_len, 
				len - sps_pos - start_code_len, 
				nal_head, start_code_len, 0) + sps_pos + start_code_len;	/* skip to next nalu */
		idr_pos = FindStrFromBuf ((char *) buf + pps_pos + start_code_len,
				len - pps_pos - start_code_len, 
				nal_head, start_code_len, 0) + pps_pos + start_code_len;	/* same as above */

		client->vops++;
		if (client->vops >= 20)
		{
			unsigned char rtcp_buf[68];

			client->vops = 0;
			MakeRTCP (client, rtcp_buf + 4, pts);
			rtcp_buf[0] = 0x24;
			rtcp_buf[1] = 0x03;
			rtcp_buf[2] = 0x00;
			rtcp_buf[3] = 0x34;
			do_client_send (client, (char *)rtcp_buf, 56);
		}

		//sps
		marker = 0x01;
		set_rtp_header(&rtp_header, client, pts, marker);

		/* h.264 nalu */
		nalu_head.F = 0x00;
		nalu_head.NRI = 0x03;
		nalu_head.TYPE = 0x07;

		/* on h.264, nalu is 1B */
		memcpy(&(client->rtp_buf[rtp_header_len + 4]), &nalu_head, nal_head_byte);	//copy nal head
		memcpy(&(client->rtp_buf[rtp_header_len + 4 + nal_head_byte]),
				buf + sps_pos + start_code_len + nal_head_byte,
				pps_pos - sps_pos - start_code_len - nal_head_byte);	//copy nalu data

		rtp_len = pps_pos - sps_pos - start_code_len + rtp_header_len;	// |rtp-header| + |NALU|

		fill_rtp_head_info(client->rtp_buf, rtp_len);
		do_client_send (client, (char *)client->rtp_buf, rtp_len + rtp_len_byte);
		client->payload_sum += rtp_len - rtp_header_len - nal_head_byte;
		client->pkg_sum++;

		//pps
		marker = 0x01;
		set_rtp_header(&rtp_header, client, pts, marker);

		/* h.264 nalu */
		nalu_head.F = 0x00;
		nalu_head.NRI = 0x03;
		nalu_head.TYPE = 0x08;
		memcpy(&(client->rtp_buf[rtp_header_len + 4]), &nalu_head, nal_head_byte);	//copy nal head

		/* copy data */
		memcpy(&(client->rtp_buf[rtp_header_len + 4 + nal_head_byte]),
				buf + pps_pos + start_code_len + nal_head_byte,
				idr_pos - pps_pos - start_code_len - nal_head_byte);	//copy nalu data
		rtp_len = idr_pos - pps_pos - start_code_len + rtp_header_len;	// |rtp-header| + |NALU|

		fill_rtp_head_info(client->rtp_buf, rtp_len);
		do_client_send (client, (char *)client->rtp_buf, rtp_len + rtp_len_byte);
		client->payload_sum += rtp_len - rtp_header_len - nal_head_byte;
		client->pkg_sum++;

		//IFrame Data
		frame_len = len - idr_pos - start_code_len - nal_head_byte;
		if (frame_len > RTP_PKG_SIZE)	//fu packet
		{
			start = 1;
			ptr = buf + idr_pos + start_code_len + nal_head_byte;
			while (frame_len > 0) {
				/* init rpt head info */
				if (frame_len > RTP_PKG_SIZE)
					marker = 0x00;
				else
					marker = 0x01;
				set_rtp_header(&rtp_header, client, pts, marker);

				/* set fu payload */
				fu_indictor.F = 0x00;
				fu_indictor.NRI = 0x03;
				fu_indictor.TYPE = 28;
				memcpy (&(client->rtp_buf[12+4]), &fu_indictor, 1);

				/* set fu header */
				fu_head.TYPE = 0x05;
				fu_head.S = 0x00;
				fu_head.E = 0x00;
				fu_head.R = 0x00;
				if (start == 1) {
					fu_head.S = 0x01;
					start = 0;
				}
				if (frame_len <= RTP_PKG_SIZE)
					fu_head.E = 0x01;
				memcpy (&(client->rtp_buf[13+4]), &fu_head, 1);
				memcpy (&(client->rtp_buf[14+4]), ptr, RTP_PKG_SIZE);

				if (frame_len > RTP_PKG_SIZE)
					rtp_len = RTP_PKG_SIZE + 14;	//14 = 12 + fu_indictor(1) + fu_head(1)
				else
					rtp_len = frame_len + 14;	//last packet

				fill_rtp_head_info(client->rtp_buf, rtp_len);
				do_client_send (client, (char *)client->rtp_buf, rtp_len + rtp_len_byte);
				client->payload_sum += rtp_len - 15;
				client->pkg_sum++;

				ptr += RTP_PKG_SIZE;
				frame_len -= RTP_PKG_SIZE;
			}
		}
		else			//single packet
		{
			marker = 0x01;
			set_rtp_header(&rtp_header, client, pts, marker);

			nalu_head.F = 0x00;
			nalu_head.NRI = 0x03;
			nalu_head.TYPE = 0x05;

			memcpy (&(client->rtp_buf[12+4]), &nalu_head, 1);
			memcpy (&(client->rtp_buf[13+4]), buf + idr_pos + 5, len - idr_pos - 5);
			rtp_len = len - idr_pos - 5 + 13;

			fill_rtp_head_info(client->rtp_buf, rtp_len);
			do_client_send (client, (char *)client->rtp_buf, rtp_len+4);
			client->payload_sum += rtp_len - nal_head_byte - rtp_header_len;
			client->pkg_sum++;
		}

	}
	else				//PFrame
	{
		int pf_start_pos = FindStrFromBuf ((char *) buf, len, nal_head, 4,0);
		frame_len = len - pf_start_pos - start_code_len - nal_head_byte;
		if (frame_len > RTP_PKG_SIZE)	//fu packet
		{
			start = 1;
			ptr = buf + pf_start_pos + start_code_len + nal_head_byte;
			while (frame_len > 0) {
				/* set rtp header */
				if (frame_len > RTP_PKG_SIZE)
					marker = 0x00;
				else
					marker = 0x01;
				set_rtp_header(&rtp_header, client, pts, marker);

				/* set fu payload */
				fu_indictor.F = 0x00;
				fu_indictor.NRI = 0x02;
				fu_indictor.TYPE = 28;
				memcpy (&(client->rtp_buf[12+4]), &fu_indictor, 1);

				/* set fu header */
				fu_head.TYPE = 0x01;
				fu_head.S = 0x00;
				fu_head.E = 0x00;
				fu_head.R = 0x00;
				if (start == 1) {
					fu_head.S = 0x01;
					start = 0;
				}
				if (frame_len <= RTP_PKG_SIZE)
					fu_head.E = 0x01;
				memcpy (&(client->rtp_buf[13+4]), &fu_head, 1);
				/* copy frame data */
				memcpy (&(client->rtp_buf[14+4]), ptr, RTP_PKG_SIZE);

				if (frame_len > RTP_PKG_SIZE)
					rtp_len = RTP_PKG_SIZE + 14;	//14 = 12 + fu_indictor(1) + fu_head(1)
				else
					rtp_len = frame_len + 14;	//last packet

				fill_rtp_head_info(client->rtp_buf, rtp_len);
				do_client_send (client, (char *)client->rtp_buf, rtp_len+4);
				client->payload_sum += rtp_len - rtp_header_len - 1 - nal_head_byte;	//rtp_len - 14
				client->pkg_sum++;
				// usleep (1);
				ptr += RTP_PKG_SIZE;
				frame_len -= RTP_PKG_SIZE;
			}
		}
		else			//single packet
		{
			marker = 0x01;
			set_rtp_header(&rtp_header, client, pts, marker);

			nalu_head.F = 0x00;
			nalu_head.NRI = 0x02;
			nalu_head.TYPE = 0x01;
			memcpy (&(client->rtp_buf[12+4]), &nalu_head, 1);
			memcpy (&(client->rtp_buf[13+4]), buf + pf_start_pos + 5, len - pf_start_pos - 5);
			rtp_len = len - pf_start_pos - 5 + 13;

			fill_rtp_head_info(client->rtp_buf, rtp_len);
			do_client_send (client, (char *)client->rtp_buf, rtp_len+4);
			client->payload_sum += rtp_len - rtp_header_len - nal_head_byte;
			client->pkg_sum++;
		} 
	}

	return 0;
}

int send_rtp_tcp (rtsp_client_t * client, unsigned char *buf, 
		int len, int frame_type, unsigned int pts)
{
	if (client->media_type == 0) {
		return send_h264_rtp_tcp(client, buf, len, frame_type, pts);
	} else if (client->media_type == 2) {
		return send_hevc_rtp_tcp(client, buf, len, frame_type, pts);
	} else {
		ak_print_error_ex("unknow media type:%d\n", client->media_type);	
		return -1;
	}
}

static void setnonblocking (int sock)
{
	int opts;
	opts = fcntl (sock, F_GETFL,0);
	if (opts < 0)
	{
		perror (" fcntl(sock,GETFL) ");
		return;
	}
	opts = opts | O_NONBLOCK;
	if (fcntl (sock, F_SETFL, opts) < 0)
	{
		perror (" fcntl(sock,SETFL,opts) ");
		return;
	}
}

void * rtsp_client_send (void *data)
{
	dvs_params_t *params;
	dvs_t *dvs;
	int length;
	int recvlen;
	unsigned char recvbuf[TEMP_STR_LEN];
	int index;
	int res;
	int frame_type;
	int chunnel;
	int frame_index;
	rtsp_client_t *client;
	params = (dvs_params_t *) data;
	dvs = params->dvs;
	index = params->index;
	ak_thread_mutex_unlock (&(params->mutex));
	client = (rtsp_client_t *) & (dvs->rtsp_client[index]);

	int reuse = 1;
	int len = sizeof (struct sockaddr_in);
	struct sockaddr_in my_addr, my_addr1;
	struct sockaddr_in their_addr, their_addr1;
	unsigned int pts;

    ak_thread_set_name("rtsp_client_send");

	/* rtp socket */
	if ((client->fd_rtp = socket (AF_INET, SOCK_DGRAM, 0)) == -1) {
		printf ("socket error\r\n");
		return (void *) NULL;
	}
	setsockopt (client->fd_rtp, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof (reuse));
	bzero (&my_addr, sizeof (my_addr));
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons (RTP_BASE_PORT + index * 2);
	my_addr.sin_addr.s_addr = htonl (INADDR_ANY);
	bzero (&(my_addr.sin_zero), 8);
	if (bind (client->fd_rtp, (struct sockaddr *) &my_addr, sizeof (my_addr)) < 0) {
		printf ("rtsp:bind error\r\n");
		return (void *) NULL;
	}

	/* rtcp socket */
	if ((client->fd_rtcp = socket (AF_INET, SOCK_DGRAM, 0)) == -1) {
		printf ("socket error\r\n");
		return (void *) NULL;
	}
	setsockopt (client->fd_rtcp, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof (reuse));
	setnonblocking (client->fd_rtcp);
	bzero (&my_addr1, sizeof (my_addr1));
	my_addr1.sin_family = AF_INET;
	my_addr1.sin_port = htons (RTP_BASE_PORT + index * 2 + 1);
	my_addr1.sin_addr.s_addr = htonl(INADDR_ANY);
	bzero (&(my_addr1.sin_zero), 8);
	if (bind (client->fd_rtcp, (struct sockaddr *) &my_addr1, sizeof (my_addr1)) < 0) {
		printf ("rtcp:bind error\r\n");
		return (void *) NULL;
	}

	while (1) {
next:
		if ((client->used == 1) && (client->rtp_send == 1)) {
			length = 0;
			/* get stream data from internal buffer, store to send_buf */
			res = buffers_get_data_with_pts(client->send_buf, 
					&length, &(client->video_send_bufs),
					&frame_type, &chunnel, &frame_index, &pts);
			client->media_type = dvs->chunnel[chunnel].media_type;
			if (res == 1) {
				/* this is tcp transmit */
				if (client->rtp_tcp == 1) {
					//send_rtp_tcp(client, client->send_buf, length, frame_type, pts);
					if (client->wait_iframe == 1) {
						if (frame_type != IFRAME) {
							goto next;
						} else {
							client->wait_iframe = 0;
							send_rtp_tcp(client, client->send_buf, length, frame_type, pts);
						}
					} else {
						send_rtp_tcp(client, client->send_buf, length, frame_type, pts);
					}
				} else {	/* this is udp transmit */
					if (client->client_changed == 1) {
						bzero(&their_addr, sizeof (their_addr));
						their_addr.sin_family 		= AF_INET;
						their_addr.sin_port		 	= htons(client->rtp_port);	/* short, NBO */
						their_addr.sin_addr.s_addr 	= inet_addr(client->client_ip);
						bzero(&(their_addr.sin_zero), 8);

						bzero(&their_addr1, sizeof (their_addr1));
						their_addr1.sin_family 		= AF_INET;
						their_addr1.sin_port 		= htons (client->rtp_port + 1);	/* short, NBO */
						their_addr1.sin_addr.s_addr = inet_addr(client->client_ip);
						bzero(&(their_addr1.sin_zero), 8);
						client->client_changed = 0;
					}
					// printf ("client->client_ip=%s, client->rtp_port=%d\r\n", client->client_ip, client->rtp_port);
					if (client->wait_iframe == 1) {
						if (frame_type != IFRAME) {
							goto next;
						} else {
							client->wait_iframe = 0;
							send_rtp_udp(client, (struct sockaddr *) &their_addr,
									(struct sockaddr *) &their_addr1,
									client->send_buf, length, frame_type, pts);
						}
					} else {
						send_rtp_udp(client, (struct sockaddr *) &their_addr,
								(struct sockaddr *) &their_addr1,
								client->send_buf, length, frame_type, pts);
					}
					recvlen = recvfrom(client->fd_rtcp, recvbuf, TEMP_STR_LEN, 
							0, (struct sockaddr *)&their_addr1, (socklen_t *)&len);
					if (recvlen < 0)
						ak_print_info_ex("recv error\n");

				}
				//   fwrite(client->send_buf,length,1,fp);
			} else {
				//printf ("res=%d\r\n", res);
				usleep(2000);
			}
		} else {
			usleep (100000);
		}
	}
	return (void *) NULL;
}

static void DoCommandNotFind (char *cmdstr, dvs_t *dvs, rtsp_client_t * client)
{
	char rtsp_str[TEMP_STR_LEN];
	char cseq_str[TEMP_STR_LEN];
	int res, cseq;

	memset (rtsp_str, 0, TEMP_STR_LEN);
	res = get_message (cmdstr, cseq_str, "CSeq: ", "\r\n");
	if (res == 0)
	{
		cseq = atoi (cseq_str);
	}
	else
	{
		cseq = 0;
	}
	memset (rtsp_str, 0, TEMP_STR_LEN);
	sprintf (rtsp_str, "RTSP/1.0 404 Not find\r\nCSeq: %d\r\n\r\n", cseq);
	do_client_send (client, rtsp_str, strlen (rtsp_str));
}

static int get_chunnel(char *cmdstr)
{
    int res = -1;
    int i,len;

    len = 0;
    for(i = 0; i < MAX_CHUNNEL_NUM; i++) {
        len = strlen(g_dvs.chunnel[i].name);
        if(len <= 0) {
            printf("error: chunne[%d] name is not set\n",i);
            return -1;
        }
    	if (FindStrFromBuf (cmdstr, strlen(cmdstr), g_dvs.chunnel[i].name, len,0) >= 0)
    	{
    		res = i;
            break;
    	}
    }
 
	return res;
}

static int getport (char *port_str)
{
	int i;
	char tmpstr[TEMP_STR_LEN];
	memset (tmpstr, 0, TEMP_STR_LEN);
	int len = strlen (port_str);
	for (i = 0; i < len; i++)
	{
		if (port_str[i] != '-')
		{
			tmpstr[i] = port_str[i];
		}
		else
		{
			break;
		}
	}
	return atoi (tmpstr);
}

static void DoOptions (char *cmdstr, dvs_t *dvs, rtsp_client_t * client)
{
	char head_str[TEMP_STR_LEN];
	char cseq_str[TEMP_STR_LEN];
	int res, cseq;

	memset (cseq_str, 0, TEMP_STR_LEN);
	res = get_message (cmdstr, cseq_str, "CSeq: ", "\r\n");
	if (res == 0)
	{
		cseq = atoi (cseq_str);
	}
	else
	{
		cseq = 0;
	}
	memset (head_str, 0, TEMP_STR_LEN);
	sprintf (head_str, "RTSP/1.0 200 OK\r\nCSeq: %d\r\nPublic: DESCRIBE, SETUP, TEARDOWN, PLAY, PAUSE\r\n\r\n", cseq);
	do_client_send (client, head_str, strlen (head_str));
}

static void DoDescribe (char *cmdstr, dvs_t *dvs, rtsp_client_t * client)
{
	char package[TEMP_STR_LEN];
	char sdp[TEMP_STR_LEN];
	char cseq_str[TEMP_STR_LEN];
	int res, cseq;
	int chunnel = get_chunnel(cmdstr);
	char ipaddr[16] = {0};
	char *enc_type = "NULL";
	int len = 0;

	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);
	int port = 0;


	getsockname(client->sock_fd, (struct sockaddr *)&addr, (socklen_t *)&(addrlen));
	inet_ntop(AF_INET, &addr.sin_addr, ipaddr, (socklen_t )addrlen);
	port = ntohs (addr.sin_port);

	if (-1 == chunnel) {
		ak_print_error_ex("name error!\n");
		return;
	}

	memset (cseq_str, 0, TEMP_STR_LEN);
	res = get_message (cmdstr, cseq_str, "CSeq: ", "\r\n");
	if (res == 0) {
		cseq = atoi (cseq_str);
	} else {
		cseq = 0;
	}


	/* on channel main is 0, channel sub is 1 */
	ak_print_notice_ex("current channel:%d\n", chunnel);
	client->media_type = dvs->chunnel[chunnel].media_type;

	if (client->media_type == 0) {
		enc_type = "H264";
		ak_print_normal_ex("* 0 channel:%d media-type:%s\n", client->index, enc_type);
	} else if (client->media_type == 2) {
		enc_type = "H265";
		ak_print_normal_ex("* 2 channel:%d media-type:%s\n", client->index, enc_type);
	}

	len = sprintf (sdp,
			"v=0\r\n"
			"o=- 1 1 IN IP4 %s\r\n"
			"t=0 0\r\n"
			"c=IN IP4 %s\r\n"
			"m=video 0 RTP/AVP 96\r\n"
			"a=rtpmap:96 %s/90000\r\n"
			"a=fmtp:96 packetization-mode=1;profile-level-id=420028\r\n"
			"a=control:track0\r\n",
			ipaddr, ipaddr, enc_type);

	len = sprintf (package,
			"RTSP/1.0 200 OK\r\n"
			"CSeq: %d\r\n"
			"Content-Base: rtsp://%s:%d/%s\r\n"
			"Content-Type: application/sdp\r\n"
			"Content-Length: %d\r\n"
			"\r\n"
			"%s",
			cseq, ipaddr, port, g_dvs.chunnel[chunnel].name, len, sdp);

	do_client_send (client, package, len);
}


static void DoSetup (char *cmdstr, dvs_t *dvs, rtsp_client_t * client)
{
	char head_str[TEMP_STR_LEN];
	char cseq_str[TEMP_STR_LEN];
	char client_port_str[TEMP_STR_LEN];
	char transport_str[TEMP_STR_LEN];
	int res, cseq; 
	int cmdlen = strlen (cmdstr);
	//int chunnel = get_chunnel(cmdstr);

	memset (cseq_str, 0, TEMP_STR_LEN);
	res = get_message (cmdstr, cseq_str, "CSeq: ", "\r\n");
	if (res == 0)
	{
		cseq = atoi (cseq_str);
	}
	else
	{
		cseq = 0;
	}

	memset (client_port_str, 0, TEMP_STR_LEN);
	res = get_message (cmdstr, client_port_str, "client_port=", "\r\n");
	if (res != 0)
	{
		sprintf (client_port_str, "56768-56769");
	}

	client->rtp_port = getport (client_port_str);
	printf ("rtp_port=%d\r\n", client->rtp_port);
	memset (transport_str, 0, TEMP_STR_LEN);
	res = get_message (cmdstr, transport_str, "Transport: ", ";");
	if (res != 0)
	{
		sprintf (transport_str, "RTP/AVP");
	}
	if (FindStrFromBuf (transport_str, strlen (transport_str), "TCP", 3,0) >= 0)
	{
		client->rtp_tcp = 1;
	}
	else
	{
		client->rtp_tcp = 0;
	}
	if (client->rtp_tcp == 0)
	{
		memset (head_str, 0, TEMP_STR_LEN);
		if (FindStrFromBuf (cmdstr, cmdlen, "/track0", strlen ("/track0"),0) >= 0)
		{
			sprintf (head_str,
					"RTSP/1.0 200 OK\r\nCSeq: %d\r\nTransport: %s;unicast;client_port=%s;server_port=%d-%d\r\nSession: %s\r\n\r\n",
					cseq, transport_str, client_port_str,
					RTP_BASE_PORT + client->index * 2,
					RTP_BASE_PORT + client->index * 2 + 1,
					session_str[client->index]);
		}
		else
		{
			sprintf (head_str,
					"RTSP/1.0 200 OK\r\nCSeq: %d\r\nTransport: %s;unicast;client_port=%s;server_port=%d-%d\r\nSession: %s\r\n\r\n",
					cseq, transport_str, client_port_str,
					RTP_BASE_PORT + client->index * 2,
					RTP_BASE_PORT + client->index * 2 + 1,
					session_str[client->index]);
		}
	}
	else
	{
		sprintf (head_str,
				"RTSP/1.0 200 OK\r\nCSeq: %d\r\nTransport: %s;unicast;interleaved=0-1\r\nSession: %s\r\n\r\n",
				cseq, transport_str, session_str[client->index]);
	}

	do_client_send (client, head_str, strlen (head_str));
}

static void DoPlay (char *cmdstr, dvs_t *dvs, rtsp_client_t * client)
{
	char head_str[TEMP_STR_LEN];
	char cseq_str[TEMP_STR_LEN];
	int res, cseq;
	//int cmdlen = strlen (cmdstr);
	int chunnel = get_chunnel(cmdstr);

	if (-1 == chunnel) {
		ak_print_error_ex("name error!\n");
		return;
	}
	
	memset (cseq_str, 0, TEMP_STR_LEN);
	res = get_message (cmdstr, cseq_str, "CSeq: ", "\r\n");
	if (res == 0)
	{
		cseq = atoi (cseq_str);
	}
	else
	{
		cseq = 0;
	}
    printf("play chunnel[%d] name %s\n",chunnel,g_dvs.chunnel[chunnel].name);
	client->chunnel = chunnel;
	client->client_changed = 1;
	client->rtp_send = 1;
	memset (head_str, 0, TEMP_STR_LEN);
	sprintf (head_str,
			"RTSP/1.0 200 OK\r\nCSeq: %d\r\nSession: %s\r\n\r\n",
			cseq, session_str[client->index]);

	do_client_send (client, head_str, strlen (head_str));
}

static void DoTeardown (char *cmdstr, dvs_t *dvs, rtsp_client_t * client)
{
	char head_str[TEMP_STR_LEN];
	char cseq_str[TEMP_STR_LEN];
	int res, cseq;
	//int cmdlen = strlen (cmdstr);

	memset (cseq_str, 0, TEMP_STR_LEN);
	res = get_message (cmdstr, cseq_str, "CSeq: ", "\r\n");
	if (res == 0)
	{
		cseq = atoi (cseq_str);
	}
	else
	{
		cseq = 0;
	}
	client->rtp_send = 0;
	memset (head_str, 0, TEMP_STR_LEN);
	sprintf (head_str,
			"RTSP/1.0 200 OK\r\nCSeq: %d\r\nSession: %s\r\n\r\n",
			cseq, session_str[client->index]);

	do_client_send (client, head_str, strlen (head_str));
}

static int do_command (char *cmdstr, int cmdlen, dvs_t *dvs, rtsp_client_t * client)
{
	//int content_len = 0;
	//int boundarylen = strlen ("\r\n\r\n");
	char tmpstr[TEMP_STR_LEN];
	memset (tmpstr, 0, TEMP_STR_LEN);
	//int tmplen;
	int chunnel = get_chunnel(cmdstr);
	if(chunnel == -1)
	{
		DoCommandNotFind (cmdstr, dvs, client);
	}
	printf ("[%s]\r\n", cmdstr);
	if (FindStrFromBuf (cmdstr, cmdlen, "OPTIONS", strlen ("OPTIONS"),0) >= 0)
	{
		DoOptions (cmdstr, dvs, client);
	}
	else if (FindStrFromBuf (cmdstr, cmdlen, "DESCRIBE", strlen ("DESCRIBE"),0) >= 0)
	{
		DoDescribe (cmdstr, dvs, client);
	}
	else if (FindStrFromBuf (cmdstr, cmdlen, "SETUP", strlen ("SETUP"),0) >= 0)
	{
		DoSetup (cmdstr, dvs, client);
	}
	else if (FindStrFromBuf (cmdstr, cmdlen, "PLAY", strlen ("PLAY"),0) >= 0)
	{
		DoPlay (cmdstr, dvs, client);
		usleep (10000);
	}
	else if (FindStrFromBuf (cmdstr, cmdlen, "TEARDOWN", strlen ("TEARDOWN"),0) >= 0)
	{
		DoTeardown (cmdstr, dvs, client);
	}
	else
	{
		DoCommandNotFind (cmdstr, dvs, client);
		printf ("%s", cmdstr);
	}
	return 0;
}

static void do_client_recv (dvs_t *dvs, rtsp_client_t * client)
{
	int pos;
	int length;
	// char tmpstr[TEMP_STR_LEN];
	int boundarylen = strlen ("\r\n\r\n");
	fd_set rdfds;
	struct timeval tv;
	int ret, res;

	char *tmpstr = (char *)malloc(RECV_BUF_SIZE);
	if(!tmpstr) {
		printf("[%s:%d] fails to malloc\n", __func__, __LINE__);
		return;
	}

	if ((client->used == 1) && (client->sock_fd != -1))
	{
		//printf("client_recv_1\r\n");
		FD_ZERO (&rdfds);
		FD_SET (client->sock_fd, &rdfds);
		tv.tv_sec = 2;
		tv.tv_usec = 0;

		ret = select (client->sock_fd + 1, &rdfds, NULL, NULL, &tv);
		if (ret == 1)
		{
			length = recv (client->sock_fd, client->recv_buf + client->recv_len, RECV_BUF_SIZE - client->recv_len, 0);
			if(client->recv_len > RECV_BUF_SIZE -256)
			{
				client->recv_len = 0;  
			}
			// printf("client->recv_len=%d\r\n", client->recv_len);
			if (length <= 0)
			{
				ak_thread_mutex_lock (&(client->mutex_send));
				client->used = -1;
				close (client->sock_fd);
				client->sock_fd = -1;
				printf ("client disconnected\r\n");
				ak_thread_mutex_unlock (&(client->mutex_send));
			}
			else
			{
				client->recv_len += length;
				while ((pos = FindStrFromBuf ((char *) (client->recv_buf), client->recv_len, "\r\n\r\n", boundarylen, 0)) > 0)
				{
					memset (tmpstr, 0, RECV_BUF_SIZE);
					memcpy (tmpstr, client->recv_buf, pos + 4);
					res = do_command (tmpstr, pos, dvs, client);
					if (res == 0)
					{
						client->recv_len = 0;
					}
					else
					{
						break;
					}
				}
			}
		}
		else if (ret < 0)
		{
			ak_thread_mutex_lock (&(client->mutex_send));
			client->used = -1;
			ak_thread_mutex_unlock (&(client->mutex_send));

		}
	}
	else
	{
		usleep(100000);
	}

	if(tmpstr)
		free(tmpstr);
}

void * rtsp_client_recv (void *data)
{
	dvs_params_t *params;
	dvs_t *dvs;
	int index;
	rtsp_client_t *client;

	params = (dvs_params_t *) data;
	dvs = params->dvs;
	index = params->index;
	ak_thread_mutex_unlock (&(params->mutex));

    ak_thread_set_name("rtsp_client_recv");
    
	client = (rtsp_client_t *) & (dvs->rtsp_client[index]);
	client->recv_len = 0;
	while (1)
	{
		do_client_recv (dvs, client); 
	}
	return (void *) NULL;
}

void * rtsp_client_listen (void *data)
{
	int i;
	int fd;
	int addrlen;
	int opt = 1;
	int clientfd;
	struct sockaddr_in addr;
	struct sockaddr_in client_addr;

	dvs_t *dvs = (dvs_t *) data;

	fd = socket (AF_INET, SOCK_STREAM, IPPROTO_IP);
	if (fd < 0) {
		printf ("client_listen:socket error.\r\n");
		return (void *) NULL;
	}
	setsockopt (fd, SOL_SOCKET, SO_REUSEADDR, (char *) &opt, sizeof (opt));

	addr.sin_family = AF_INET;
	addr.sin_port = htons (RTSP_LISTEN_PORT);
	addr.sin_addr.s_addr = htonl (INADDR_ANY);
	memset (&(addr.sin_zero), 0, sizeof (addr.sin_zero));
	if (bind (fd, (struct sockaddr *) &addr, sizeof (addr)) != 0) {
		printf ("client_listen:Binding error.\r\n");
		close (fd);
		return (void *) NULL;
	}
	if (listen (fd, 100) != 0) {
		printf ("client_listen:Listen Error.\r\n");
	}
	printf("client_listen: Listen to %d\r\n", RTSP_LISTEN_PORT);

	while (1) {
		addrlen = sizeof (client_addr);
		clientfd = accept(fd, (struct sockaddr *)&client_addr, (socklen_t *)&addrlen);
		if (clientfd != -1) {
			printf ("Client %s is connected.\r\n", inet_ntoa(client_addr.sin_addr));
			for (i = 0; i < MAX_RTSP_CLIENT_NUM; i++) {
				if (dvs->rtsp_client[i].used < 0) {
					dvs->rtsp_client[i].used++;
				}
			}
			for (i = 0; i < MAX_RTSP_CLIENT_NUM; i++) {
				if (dvs->rtsp_client[i].used == 0) {
					printf ("i=%d,clientfd=%d\r\n", i, clientfd);
					dvs->rtsp_client[i].used = 1;
					dvs->rtsp_client[i].sock_fd = clientfd;
					dvs->rtsp_client[i].recv_len = 0;
					dvs->rtsp_client[i].rtp_send = 0;
					dvs->rtsp_client[i].rtp_tcp = 0;
					dvs->rtsp_client[i].sq_no = 0;
					dvs->rtsp_client[i].payload_sum = 0;
					dvs->rtsp_client[i].pkg_sum = 0;
					dvs->rtsp_client[i].timestamp = 0;
					dvs->rtsp_client[i].wait_iframe = 1;
					dvs->rtsp_client[i].index = i;
					dvs->rtsp_client[i].vops = 20;
					memset (dvs->rtsp_client[i].client_ip, 0, IP_ADDR_LEN);
					sprintf (dvs->rtsp_client[i].client_ip, "%s", inet_ntoa (client_addr.sin_addr));
					break;
				}
			}
		}
	}
	return (void *) NULL;
}

