/************************************************************************
 * Copyright (c) 2016, Anyka Co., Ltd. 
 * All rights reserved.	
 *  
 * File Name：netdemo.c
 * Function：This file is demo for network.
 *
 * Author：lx
 * Date：
 * Version：
**************************************************************************/

#define ANYKA_DEBUG 

#include "lwip/sockets.h"
#include "lwip/ipv4/ip.h"
#include "lwip/ipv4/ip_addr.h"
#include "string.h"

#include "ak_thread.h"
#include "ak_common.h"
#include "kernel.h"

 

struct sock_cfg{
  unsigned long  addr;
  unsigned short port;
};

struct netrecv_arg
{
	struct sock_cfg  sock;
    unsigned long    runtime;   //second
};

#define NETSEND_STRING_LIMIT  (32) //Byte
struct netsend_arg
{
	struct sock_cfg  sock;
    unsigned long    count; 
	         char    string[NETSEND_STRING_LIMIT];
};

#define UDPRECV_LOCAL_PORT     (4900)
#define TCPRECV_LOCAL_PORT     (4900)

#define UDPSEND_REMOTE_PORT    (4900)
#define TCPSEND_REMOTE_PORT    (4900)

#define NETRECV_RUNTIME        (60) //second
#define NETSEND_REMOTE_ADDR    "192.168.0.2"
#define NETSEND_COUNT_DEFAULT  (10)
#define NETSEND_STRING_DEFAULT "netsend_hello."

#define PARAM_UDP_PROTOCOL     "udp"
#define PARAM_TCP_PROTOCOL     "tcp"

#define RECVBUF_TEMPSIZE       (1024)

//wmj- for gcc 4.4.1 not support TLS
#define errno 1


static char *help_recv[]={
	"udp/tcp recvdata demo.",
	"usage: netrecv [udp/tcp] [time_second] <local_port> \n"
};

static char *help_send[]={
	"udp/tcp senddata demo.",
	"usage: netsend [udp/tcp] [string] [send_count] <remote_ipaddr> <remote_port>\n"
};

static char *help_speed_test[]={
	"speedtest demo",
	"usage: speedtest [pkg_size] [time seconds] <remote_ipaddr> <remote_port> <udp/tcp> "
};


/////////////////////////////udp recv ////////////////////////////////////
static void udprecv(void *argv)
{
    struct netrecv_arg *nr_arg;
	int socket_s = -1;
	int err = 0;
	struct sockaddr_in net_cfg;
	struct sockaddr remote_addr;
	unsigned char rcv_buff[RECVBUF_TEMPSIZE];
	socklen_t retval;
	unsigned long starttick;
	unsigned long tick;
	int recvlen;
	unsigned long total_len, total_err;
	
	
	nr_arg = (struct netrecv_arg *)argv;
	if (NULL == nr_arg)
	{
	    ak_print_error("udprecv argv error.\n");
		return;
	}

	memset(&net_cfg, 0, sizeof(struct sockaddr_in));

	net_cfg.sin_family = AF_INET;
	net_cfg.sin_port = htons(nr_arg->sock.port);
	net_cfg.sin_addr.s_addr = htons(nr_arg->sock.addr);

	total_len = 0;
	total_err = 0;

	socket_s = socket(AF_INET, SOCK_DGRAM, 0);
	if (socket_s <0)
	{
		ak_print_error("create socket error.\n");
	}
	else
	{
		err = bind(socket_s, (struct sockaddr*) &net_cfg, sizeof(struct sockaddr_in));
		if (err !=0)
		{
			ak_print_error("bind socket error\n");
		}
		else
		{
		    tick = (nr_arg->runtime) * 1000;
		    starttick = get_tick_count();
			ak_print_normal("recv data:\n");
			
			while(get_tick_count() - starttick < tick)
			{
				recvlen = recvfrom(socket_s, rcv_buff, sizeof(rcv_buff)-1, MSG_WAITALL, &remote_addr, (socklen_t*) &retval);
				if (recvlen > 0)
				{
					rcv_buff[recvlen]=0;
					total_len += recvlen;
					ak_print_normal("recv len=%d,data=%s\n",(int)recvlen,rcv_buff);		
				}
				else
				{
					ak_print_error("recv error.\n");
					total_err++;
					break;
			    }
			}
			ak_print_normal("%ld bytes data received, %d times recv error\n", total_len, total_err);
		}
		if (-1 != socket_s)
		{
			closesocket(socket_s);
		}
	}
}

//////////////////////udp send ///////////////////////////////////////////////////////////////////
static void udpsend(void *argv)
{
    struct netsend_arg *ns_arg;
	struct sockaddr_in net_cfg;
	int socket_s = -1;
	unsigned long  sendcnt=0;
	int send_len = 0;
	unsigned long total_len, total_err;

	ns_arg = (struct netsend_arg *)argv;
	if (NULL == ns_arg)
	{
	    ak_print_error("udpsend argv error.\n");
		return;
	}

	net_cfg.sin_family = AF_INET;
	net_cfg.sin_len = sizeof(struct sockaddr_in);
	net_cfg.sin_port = htons(ns_arg->sock.port);
	net_cfg.sin_addr.s_addr = ns_arg->sock.addr;

	socket_s = socket(AF_INET, SOCK_DGRAM, 0);
	if (socket_s < 0)
	{
		ak_print_error("get socket err:%d\n", errno);
		return;
	} 
	
	total_len = 0;
	total_err = 0;
	sendcnt = ns_arg->count;
	
	while (sendcnt-- > 0)
	{
	    ak_print_normal("send data=[%s]\n",ns_arg->string);
		send_len = sendto(socket_s, ns_arg->string, strlen(ns_arg->string), 0, (struct sockaddr*)&net_cfg, sizeof(struct sockaddr));
		if (send_len <= 0)
		{
			ak_print_error("send error\n");
			total_err++;
			break;
		}
		total_len += send_len;
		ak_sleep_ms(50);
	}
	ak_print_normal("%ld bytes data sent success, %d times send error\n", total_len, total_err);
	shutdown(socket_s, SHUT_RDWR);
	if (-1 != socket_s)
	{
		closesocket(socket_s);
	}
}

/////////////////////////////tcp recv ////////////////////////////////////
static void tcprecv(void *argv)
{
    struct netrecv_arg *nr_arg;
	int socket_s = -1;
	int new_socket = -1;
	int err = 0;
	int sockaddr_len;
	int len;
	struct sockaddr_in net_cfg;
	struct sockaddr remote_addr;
	unsigned char rcv_buff[RECVBUF_TEMPSIZE];
	socklen_t retval;
	int size;
	unsigned long starttick;
	unsigned long tick;
	int opt;
	unsigned long total_len, total_err;

	nr_arg = (struct netrecv_arg *)argv;
	if (NULL == nr_arg)
	{
	    ak_print_error("tcprecv argv error.\n");
		return;
	}

	memset(&net_cfg, 0, sizeof(struct sockaddr_in));

	net_cfg.sin_family = AF_INET;
	net_cfg.sin_port = htons(nr_arg->sock.port);
	net_cfg.sin_addr.s_addr = htons(nr_arg->sock.addr);

	total_len = 0;
	total_err = 0;

	socket_s = socket(AF_INET, SOCK_STREAM, 0);//tcp socket
	if (socket_s <0)
	{
		ak_print_error("create socket error.\n");
	}
	else
	{
	    //使server 能立即重用
		opt = SOF_REUSEADDR;
		if (setsockopt(socket_s, SOL_SOCKET, SOF_REUSEADDR, &opt, sizeof(int)) != 0) 
		{
			printf("set opt error\n");
		}
        else
        {
			err = bind(socket_s, (struct sockaddr*) &net_cfg, sizeof(struct sockaddr_in));
			if (err !=0)
			{
				ak_print_error("bind socket error\n");
			}
			else
			{
				err = listen(socket_s, 4);
				if (err !=0 )
				{
					ak_print_error("listen socket error\n");
				}
	            else
	            {
	                sockaddr_len = sizeof(struct sockaddr);
					new_socket = accept((int)socket_s, (struct sockaddr*) &remote_addr, (socklen_t*) &sockaddr_len);
					if (new_socket <0)
					{
						ak_print_error("accept socket error\n");
					}
	                else
	                {
	                    ak_print_debug("remote addr:%s.\n", remote_addr.sa_data);
						
						ak_print_normal("recv data:\n");
					    tick = (nr_arg->runtime) * 1000;
					    starttick = get_tick_count();
						
						while(get_tick_count() - starttick < tick)
						{
							len = recv(new_socket, rcv_buff, sizeof(rcv_buff)-1,MSG_WAITALL);
							if (len > 0)
							{
								total_len += len;
								rcv_buff[len]=0;
								ak_print_normal("recv len=%d,data=%s\n",(int)len,rcv_buff);
							}
							else
							{
								ak_print_error("recv error.\n");
								total_err++;
								break;
							}
						}
						ak_print_normal("%ld bytes data received, %d times recv error\n", total_len, total_err);
	                }
				}
			}
        }
	}
	
	if (-1 != new_socket)
	{
		closesocket(new_socket);
	}
	if (-1 != socket_s)
	{
		closesocket(socket_s);
	}	
}


/////////////////////////////tcp send ////////////////////////////////////
static void tcpsend(void *argv)
{
    struct netsend_arg *ns_arg;
	struct sockaddr_in net_cfg;
	int socket_s = -1;
	unsigned long  sendcnt=0;
	int send_len = 0;
	int err = -1;
	unsigned long total_len, total_err;

	ns_arg = (struct netsend_arg *)argv;
	if (NULL == ns_arg)
	{
	    ak_print_error("tcpsend argv error.\n");
		return;
	}

	net_cfg.sin_family = AF_INET;
	net_cfg.sin_len = sizeof(struct sockaddr_in);
	net_cfg.sin_port = htons(ns_arg->sock.port);
	net_cfg.sin_addr.s_addr = ns_arg->sock.addr;

	total_len = 0;
	total_err = 0;

	socket_s = socket(AF_INET, SOCK_STREAM, 0);//tcp socket
	if (socket_s <0)
	{
		ak_print_error("create socket error.\n");
	}
	else
	{
		err = connect(socket_s, (struct sockaddr*)&net_cfg, sizeof(struct sockaddr));
		if (err !=0)
		{
			ak_print_error("connect socket error\n");
		}
		else
		{
			
			sendcnt = ns_arg->count;
			while (sendcnt-- > 0)
			{
				ak_print_normal("send data=[%s]\n",ns_arg->string);
				send_len = send(socket_s, ns_arg->string, strlen(ns_arg->string), MSG_WAITALL);
				if (send_len < 0)
				{
					ak_print_error("send error\n");
					total_err++;
					break;
				}
				total_len += send_len;
				ak_sleep_ms(50);
			}
			ak_print_normal("%ld bytes data sent success, %d times send error\n", total_len, total_err);
		}
	}
	
	if (-1 != socket_s)
	{
		closesocket(socket_s);
	}	
}


/*******************************************************
 * @brief udp test: udp send data
 * @author
 * @date 
 * @param [in]  ipaddr 
 * @param [in]  port
 * @param [in]  pkt_size
 * @param [in]  duriation
 * @return void
 *******************************************************/
#define SPEED_TEST_REMOTE_PORT  	(4801)
#define SPEED_TEST_REMOTE_ADDR  	"192.168.1.2"
#define WIFI_RESERVE_MEMORY    	(30<<10)  //reserve for wifi working.
#define UDP_MTU  				(1460) //wmj+
#define MAX_SEND_BUF 			(64*1024)
#define PROTOCOL_UDP            17
#define PROTOCOL_TCP            6


static void udp_speed(unsigned long  ipaddr, unsigned short port, unsigned int pkt_size, unsigned int duriation)
{
	int                 fd;
	struct sockaddr_in 	addr;
	unsigned int       	tick_start, tick_end, tick_stat;
	unsigned int       	send_time_count;
	int                	send_len;
	unsigned long      	send_count;;
	unsigned long  		send_count_total;
	unsigned int   		print_count;;
	unsigned char       *sendBuf;
	int 				tx_pending = 0, max_tx_pending;//wmj+
	
	if(pkt_size > MAX_SEND_BUF)
	{
		ak_print_error("pkt_size larger than the max len %d\n", MAX_SEND_BUF);
		return;
	}
	//wmj+ for queue limit
	max_tx_pending = wifi_get_max_tx_pending();
	ak_print_normal("----max tx pending %d-----\n", max_tx_pending);
	if( max_tx_pending > 0 && pkt_size > max_tx_pending * UDP_MTU)
	{
		ak_print_error("packet size %d is greater than the max queue %d * %d(MTU), some pkts may be droped by wifi driver\n", (int)pkt_size, max_tx_pending, UDP_MTU);
		ak_print_error("exit speedtest\n");
		return;
	}
	//wmj<<
	
	addr.sin_family = AF_INET;
	addr.sin_len = sizeof(struct sockaddr_in);
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = ipaddr;

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0)
	{
		ak_print_error("get socket error:%d");
		return;
	} 
	
	sendBuf = malloc(MAX_SEND_BUF);
	if(sendBuf == NULL)
	{
		ak_print_error("malloc buf for send error\n");
		goto sock_free_exit; 
	}
	/*fill buffer*/	
	for(int i = 0; i < MAX_SEND_BUF; i++)
	{
		sendBuf[i] = i % 26 + 65;  /*A - Z*/
	}
    ak_print_normal("memleft=%d\n", (int)Fwl_GetRemainRamSize());
	ak_print_normal("start send data to %s:%d in udp\n", inet_ntoa(ipaddr), port);
	ak_print_normal("data len=[%d], duriation=%d sec\n\n", pkt_size, duriation);
    
	send_count_total = 0;
	send_count = 0;
	tick_start = tick_end = tick_stat = get_tick_count();
	
	while((tick_end - tick_start) < duriation * 1000)
	{
		//wmj+ for tx pending test, the ramainning pkt queue should cover the sending pkg  
		//incase tx_pending greater than max_tx_pending , convert pkt_size to int to compare
		//reserve several pkts for some protocal packet such as arp to transmit is better. 
		//ie.  ((max_tx_pending - 5 - (tx_pending = wifi_get_tx_pending()))* (UDP_MTU) < (int)pkt_size)
		print_count = 0;
		while ((max_tx_pending > 0 && ((max_tx_pending - (tx_pending = wifi_get_tx_pending()))* (UDP_MTU) < (int)pkt_size))
			    || (Fwl_GetRemainRamSize() < WIFI_RESERVE_MEMORY))
		{
			print_count++;
			if (print_count % 10 == 0)
			{
				ak_print_normal("curr tx_pending %d, max_tx_pending is %d, memleft: %d waiting driver to process tx queue...\n",tx_pending, max_tx_pending, (int)Fwl_GetRemainRamSize());
			}
			if (print_count > 1000)
			{
			    ak_print_normal("long waiting memleft:%d..\n", (int)Fwl_GetRemainRamSize());
				break;
			}
			ak_sleep_ms(1);
		}
		
		//wmj<<
		send_len = sendto(fd, sendBuf, pkt_size, 0, (struct sockaddr*)&addr, sizeof(struct sockaddr));
		if(send_len <= 0)
		{
			ak_print_error("send error\n");
			break;
		}	
		
        send_count++;
		send_count_total++;
		tick_end = get_tick_count();
		
		/*print statistic every 5 seconds*/
		if (tick_end - tick_stat >= 5000)
		{
			tick_stat = tick_end;
			ak_print_normal("send_len %d Bytes, send speed = %dKB/s, memleft = %d \n", send_len
				, (int)((send_count * send_len / 5) >> 10), (int)Fwl_GetRemainRamSize() );
			send_count = 0;
		}
	}
	if (tick_end - tick_start > 1000) /* greate than 1 second, show statistics*/
	{
		ak_print_normal("speedtest end, duriation = %d(ms), packet count = %d, total data = %d(KB),speed=%d(KB/s)\n"
			, (int)(tick_end - tick_start)
			, (int)send_count_total
			, (int)((pkt_size * send_count_total) >> 10) 
			, (int)(((pkt_size * send_count_total) >> 10) /((tick_end - tick_start)/1000)) );
	}
	else
	{
		ak_print_normal("send end, duriation less than 1 second\n");
	}
	free(sendBuf);
sock_free_exit:
	shutdown(fd,SHUT_RDWR);
	closesocket(fd);
	ak_print_normal("close socke memleft:%d\n", (int)Fwl_GetRemainRamSize());
	
}

/////////////////////////////tcp send ////////////////////////////////////
static void tcp_speed(unsigned long  ipaddr, unsigned short port, unsigned int pkt_size, unsigned int duriation)
{
    int                 fd;
	struct sockaddr_in 	addr;
	int 				send_len = 0;
	int					err = -1;
	unsigned int       	tick_start, tick_end, tick_stat;
	unsigned long      	send_count;;
	unsigned long  		send_count_total;
	unsigned int   		print_count;
	unsigned char 		*sendBuf;
	int 				tx_pending = 0, max_tx_pending;//wmj+
	
	addr.sin_family = AF_INET;
	addr.sin_len = sizeof(struct sockaddr_in);
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = ipaddr;

	if(pkt_size > MAX_SEND_BUF)
	{
		ak_print_error("pkt_size larger than the max len %d\n", MAX_SEND_BUF);
		return;
	}
	//wmj+ for queue limit
	max_tx_pending = wifi_get_max_tx_pending();
	ak_print_normal("----max tx pending %d-----\n", max_tx_pending);
	if( max_tx_pending > 0 && pkt_size > max_tx_pending * UDP_MTU)
	{
		ak_print_error("packet size %d is greater than the max queue %d * %d(MTU), some pkts may be droped by wifi driver\n", (int)pkt_size, max_tx_pending, UDP_MTU);
		ak_print_error("exit speedtest\n");
		return;
	}
	
	fd = socket(AF_INET, SOCK_STREAM, 0);//tcp socket
	if (fd < 0)
	{
		ak_print_error("create socket error.\n");
	}
	else
	{
		err = connect(fd, (struct sockaddr*)&addr, sizeof(struct sockaddr));
		if (err !=0)
		{
			ak_print_error("connect socket error\n");
		}
		else
		{
			
			sendBuf = malloc(MAX_SEND_BUF);
			if(sendBuf == NULL)
			{
				ak_print_error("malloc buf for send error\n");
				goto sock_free_exit; 
			}
			/*fill buffer*/	
			for(int i = 0; i < MAX_SEND_BUF; i++)
			{
				sendBuf[i] = i % 26 + 65;  /*A - Z*/
			}
			
			ak_print_normal("memleft=%d\n", (int)Fwl_GetRemainRamSize());
			ak_print_normal("start send data to %s:%d in tcp\n", inet_ntoa(ipaddr), port);
			ak_print_normal("data len=[%d], duriation=%d sec\n\n", pkt_size, duriation);
    
			send_count_total = 0;
			send_count = 0;
			tick_start = tick_end = tick_stat = get_tick_count();
			
			while((tick_end - tick_start) < duriation * 1000)
			{
				//wmj+ for tx pending test, the ramainning pkt queue should cover the sending pkg  
				//incase tx_pending greater than max_tx_pending , convert pkt_size to int to compare
				//reserve several pkts for some protocal packet such as arp to transmit is better. 
				//ie.  ((max_tx_pending - 5 - (tx_pending = wifi_get_tx_pending()))* (UDP_MTU) < (int)pkt_size)
				print_count = 0;
				while ((max_tx_pending > 0 && ((max_tx_pending - (tx_pending = wifi_get_tx_pending()))* (UDP_MTU) < (int)pkt_size))
					    || (Fwl_GetRemainRamSize() < WIFI_RESERVE_MEMORY))
				{
					print_count++;
					if (print_count % 10 == 0)
					{
						ak_print_normal("curr tx_pending %d, max_tx_pending is %d, memleft: %d waiting driver to process tx queue...\n",tx_pending, max_tx_pending, (int)Fwl_GetRemainRamSize());
					}

					if (print_count > 1000)
					{
					    ak_print_normal("long waiting memleft:%d..\n", (int)Fwl_GetRemainRamSize());
						break;
					}
					ak_sleep_ms(1);
				}
				send_len = send(fd, sendBuf, pkt_size, MSG_WAITALL);
				if (send_len < 0)
				{
					ak_print_error("send error\n");
					break;
				}
				send_count++;
				send_count_total++;
				tick_end = get_tick_count();
		
				/*print statistic every 5 seconds*/
				if (tick_end - tick_stat >= 5000)
				{
					tick_stat = tick_end;
					ak_print_normal("send_len %d Bytes, send speed = %dKB/s, memleft = %d \n", send_len
						, (int)((send_count * send_len / 5)>>10), (int)Fwl_GetRemainRamSize() );
					send_count = 0;
				}
			}
			
			if (tick_end - tick_start > 1000) /* greate than 1 second, show statistics*/
			{
				ak_print_normal("speedtest end, duriation = %d(ms), packet count = %d, total data = %d(KB),speed=%d(KB/s)\n"
					, (int)(tick_end - tick_start)
					, (int)send_count_total
					, (int)((pkt_size * send_count_total) >> 10) 
					, (int)(((pkt_size * send_count_total) >> 10) /((tick_end - tick_start)/1000)) );
			}
			else
			{
				ak_print_normal("send end, duriation less than 1 second\n");
			}
			
		}
		free(sendBuf);
sock_free_exit:
		shutdown(fd,SHUT_RDWR);
		closesocket(fd);
		ak_print_normal("close socke memleft:%d\n", (int)Fwl_GetRemainRamSize());
				
	}
}

void cmd_speed_test (int argc, char **args)
{
	unsigned long  ipaddr = IPADDR_NONE;
    unsigned short port = 0;
	unsigned int pkt_size = 0;
	unsigned int duriation = 0;
	unsigned int protocol;
	
	if(argc < 2)
	{
		ak_print_error("%s\n",help_speed_test[1]);
		return;
	}
	
	/*pkt_size and duriation*/
	pkt_size  = atoi(args[0]);
	duriation = atoi(args[1]);    

	/*ip addr*/
	if(argc > 2)
	{
		ipaddr = inet_addr((char *)args[2]);
		if (IPADDR_NONE == ipaddr)
		{
		   ak_print_error("set remote_ipaddr wrong.\n");
		   return;
		}
	}
	else
	{
		ipaddr = inet_addr(SPEED_TEST_REMOTE_ADDR);
	}
	/*port*/
	if (argc > 3)
	{
		port = atoi(args[3]);
		if(port > 65535)
		{
			ak_print_error("port should less than 65535\n");
			return;
		}
	}
	else
	{
		port = SPEED_TEST_REMOTE_PORT;
	}

	/*protocol*/
	if (argc > 4)
	{
		if (strcmp(args[4], PARAM_UDP_PROTOCOL) == 0)
	    {
	       protocol = PROTOCOL_UDP;
		   
		}
		else if (strcmp(args[4], PARAM_TCP_PROTOCOL) == 0)
		{
	       protocol = PROTOCOL_TCP;
		}
		else
		{
			ak_print_error("%s\n",help_speed_test[1]);
			return;
		}
	}
	else
	{
		protocol = PROTOCOL_UDP;
	}
	
	
	if(protocol == PROTOCOL_UDP)
	{
		udp_speed(ipaddr, port, pkt_size, duriation);
	}
	else if(protocol == PROTOCOL_TCP)
	{
		tcp_speed(ipaddr, port, pkt_size, duriation);
	}
	else
	{
		ak_print_error("unknown protocol, do nothing.\n");
	}
	
}


void cmd_netrecv_demo(int argc, char **args)
{
	struct netrecv_arg *nr_arg;
	unsigned short port = 0;
    unsigned long  runtime = NETRECV_RUNTIME;
	bool is_udp;
    bool parse_err = true;

    //protocol
	if (argc>0 && (char *)args[0] != NULL)
	{
	    if (strcmp(args[0], PARAM_UDP_PROTOCOL) == 0)
	    {
           is_udp = true;
		   parse_err = false;
		   port = UDPRECV_LOCAL_PORT;
		}
		else if (strcmp(args[0], PARAM_TCP_PROTOCOL) == 0)
		{
           is_udp = false;
		   parse_err = false;
		   port = TCPRECV_LOCAL_PORT;
		}
	}
	
	if (parse_err || argc < 2)
	{
		ak_print_error("%s",help_recv[1]);
		return;
	}

    //udprecv timer
	if (argc>1 && (char *)args[1] != NULL)
	{
	    runtime = atoi(args[1]);
	}

	//socket port
	if (argc>2 && (int *)args[2] != NULL)
	{
		port = atoi(args[2]);
		if(port > 65535)
		{
			ak_print_error("port should less than 65535\n");
			return;
		}
	}
	
	nr_arg = (struct netrecv_arg *)malloc(sizeof(struct netrecv_arg));
    if (NULL == nr_arg)
    {
		ak_print_error("malloc netrecv_arg err.\n");
		return;
	}
	nr_arg->sock.addr = INADDR_ANY;
	nr_arg->sock.port = port;
    nr_arg->runtime = runtime;
	
    ak_print_normal("netrecv protocol=%s,local_port=%d,runtime:%d.\n", 
		           (is_udp?PARAM_UDP_PROTOCOL:PARAM_TCP_PROTOCOL), port, runtime);
	if (is_udp)
	{
		udprecv(nr_arg);
	}
	else
	{
		tcprecv(nr_arg);
	}

	free(nr_arg);
}

void cmd_netsend_demo(int argc, char **args)
{
	struct netsend_arg *ns_arg;
	unsigned long  ipaddr = IPADDR_NONE;
    unsigned long  count = NETSEND_COUNT_DEFAULT;
	unsigned short port = 0;
	char string[NETSEND_STRING_LIMIT] = NETSEND_STRING_DEFAULT;
	bool is_udp;
    bool parse_err = true;
		
    //protocol
	if (argc>0 && (char *)args[0] != NULL)
	{
	    if (strcmp(args[0], PARAM_UDP_PROTOCOL) == 0)
	    {
		    is_udp = true;
			parse_err = false;
		    port = UDPSEND_REMOTE_PORT;
		}
		else if (strcmp(args[0], PARAM_TCP_PROTOCOL) == 0)
		{
			is_udp = false;
			parse_err = false;
		    port = TCPSEND_REMOTE_PORT;
		}
	}
	
	if (parse_err || argc < 3)
    {
		ak_print_error("%s",help_send[1]);
		return;
	}

	 //string
	if (argc>1 && (char *)args[1] != NULL)
	{
		if(strlen(args[1]) > NETSEND_STRING_LIMIT - 1)
		{
			ak_print_error("string length should less than %d\n",NETSEND_STRING_LIMIT);
			return;
		}
	    strcpy(string, (char *)args[1]);
		
	}

	//count
	if (argc>2 && (int *)args[2] != NULL)
	{
		count = atoi(args[2]);
		
	}
	
	//ipaddr
	if (argc>3 && (char *)args[3] != NULL)
	{
	    ipaddr = inet_addr((char *)args[3]);
		if (IPADDR_NONE == ipaddr)
		{
		   ak_print_error("set remote_ipaddr wrong.\n");
		   return;
		}
	}

	//port
	if (argc>4 && (int *)args[4] != NULL)
	{
		port = atoi(args[4]);
		if(port > 65535)
		{
			ak_print_error("port should less than 65535\n");
			return;
		}
	}
	

	ns_arg = (struct netsend_arg *)malloc(sizeof(struct netsend_arg));
    if (NULL == ns_arg)
    {
		ak_print_error("malloc netsend_arg err.\n");
		return;
	}
	
    ak_print_normal("netsend protocol=%s,remote_ipaddr=%s,remote_port=%d,string=%s,count:%d.\n", 
		           (is_udp?PARAM_UDP_PROTOCOL:PARAM_TCP_PROTOCOL), 
		           ((ipaddr==IPADDR_NONE)?NETSEND_REMOTE_ADDR:(char *)args[3]),
		           port, string, count);
	
	if (ipaddr == IPADDR_NONE)
	{
		ipaddr = inet_addr(NETSEND_REMOTE_ADDR);
	}

	memcpy(ns_arg->string, string, NETSEND_STRING_LIMIT);
	ns_arg->count = count;
    ns_arg->sock.addr = ipaddr;
	ns_arg->sock.port = port;
	
	if (is_udp)
	{
		udpsend(ns_arg);
	}
	else
	{
		tcpsend(ns_arg);
	}
	
	free(ns_arg);
}


static int cmd_net_demo_reg(void)
{
    cmd_register("netrecv", cmd_netrecv_demo, help_recv);
	cmd_register("netsend", cmd_netsend_demo, help_send);
	cmd_register("speedtest", cmd_speed_test, help_speed_test);
    return 0;
}

cmd_module_init(cmd_net_demo_reg)


