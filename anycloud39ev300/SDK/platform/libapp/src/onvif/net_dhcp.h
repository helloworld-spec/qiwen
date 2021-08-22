/******************************************************************************



 ******************************************************************************
  File Name    : nvc_dhcp.h
  Version       : Initial Draft
  Author        : kejiazhw@gmail.com(kaga)
  Created       : 2014/8/6
  Last Modified : 2014/8/6
  Description   : setup network interface of IPCs automatically

  History       :
  1.Date        : 2014/8/6
    	Author      : kaga
 	Modification: Created file
  2, Date 	: 2014/8/13
	Author		: kaga
	Modification: Done, only support onvif now
******************************************************************************/

#ifndef __NET_DHCP_H__
#define __NET_DHCP_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <pthread.h>

/**
 * f_ip_found - callback function for ip adapt
 * @origin_ip[IN]: no use
 * @origin_netmask[IN]: no use
 * @szip[IN]: ipaddr to set
 * @szmac[IN]: no use
 * return: 1 - success, fail return 0.
 */
typedef void (*f_ip_found)(char *origin_ip, char *origin_netmask, char *szip, char *szmac);

/*
 * function :
 * NET_find_avai_ip - find available ip
 * 
 * params:
 * sz_l_ip[IN]: client ip addr
 * sz_l_netmask[IN]: netmask
 * ip_start[IN]: ip address to search start(null to use default: network + 15)
 * ip_end[IN]: ip address to search end (0xffffffff)
 * need_n[IN]: the number of available IPs needed
 * multi_tasks[IN]: the number of pallel tasks to use when searching available IPs
 * timeout_s[IN]: timeout in second
 * hook[IN]: found ip callback function
 *
 * return: int
 * retval: failed return -1, else return number of ip found
 */
extern int NET_find_avai_ip(char *sz_l_ip, char *sz_l_netmask, char *ip_start, char *ip_end,
	int need_n, int multi_tasks, int timeout_s, f_ip_found hook);

/*
 * NET_check_ip_conflict - check ip conflict
 * ip[IN]: ip addr
 * return: 
 */
extern int NET_check_ip_conflict(char *ip);

/*
 * NET_adapt_ip - ip adapt
 * shelf_ip[IN]: self ip addr
 * client_ip[IN]: client ip addr
 * hook[IN]: found ip callback function
 * return: 0 success, -1 failed
 */
extern int NET_adapt_ip(char *shelf_ip, char* client_ip, f_ip_found hook);

extern int g_ip_conflict_flag;

int ipstr2uint8(unsigned char *, char *);
int _ip_2string(unsigned char *, char *);

#ifdef __cplusplus
}
#endif

#endif


