/******************************************************************************

  Copyright (C), 2014-, GuangZhou JUAN Electronics Co., Ltd.

 ******************************************************************************
  File Name    : rtp.h
  Version       : Initial Draft
  Author        : kejiazhw@gmail.com(kaga)
  Created       : 2014/3/19
  Last Modified : 2014/3/19
  Description   : arp  utils , reference to rfc826 
 				LINUX:SOCK_PACKET
  History       : 
  1.Date        : 2014/3/19
    	Author      : kaga
 	Modification: Created file	
******************************************************************************/

#ifndef __ARP_H__
#define __ARP_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*fAddOnePeer)(int type, char *peer_ip, char *peer_mac);

#define ARP_RECV_TIMEOUT	(1) //UNIT : SECOND

int ARP_init(void);

/*
 * ARP_send_request1 - 
 * query_ip[IN]: ip addr
 * query_mac[OUT]: mac addr
 * max_size[IN]: for example , query_mac[64][18], then max_size is 64
 * timeout[IN]: timeout, unit is second
 * return: number of mac query, failed return -1
 */
extern int ARP_send_request1(char *query_ip, char (*query_mac)[18], int max_size, int timeout);
/*
	desc: return when receive one ack
	ret: return 1 if success, else return -1;
*/
//extern int ARP_query(char *query_ip, char *query_mac); 
//extern int RARP_send_request(char *query_ip, char *query_mac);// not work well

//extern int ARP_listener_start(fAddOnePeer hook);
//extern int ARP_listener_stop();


#ifdef __cplusplus
}
#endif


#endif
