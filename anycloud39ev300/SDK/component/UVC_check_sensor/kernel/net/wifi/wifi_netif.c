/**
 * @file
 * Ethernet Interface Skeleton
 *
 */

/*
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
 * All rights reserved. 
 * 
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 * 
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

/*
 * This file is a skeleton for developing Ethernet network interface
 * drivers for lwIP. Add code to the low_level functions and do a
 * search-and-replace for the word "ethernetif" to replace it with
 * something that better describes your network interface.
 */

#include "lwip/netif/etharp.h"
#include "lwip/tcpip.h"
#include "libc_mem.h"

#ifdef ANYKA_DEBUG
#define akp_info(...) do{printf(__VA_ARGS__) ;}while(0)
#define akp_err(...) do{printf("err:");printf(__VA_ARGS__);}while(0)
#define akp_debug(...) do{printf(__VA_ARGS__); }while(0)
#define akp_debug_enter do{printf("enter %s\n", __func__ ); printf("\r\n");}while(0)
#define akp_debug_exit do{printf("exit %s\n", __func__ ); printf("\r\n");}while(0)

#else
#define akp_info(...) do{printf(__VA_ARGS__) ;}while(0)
#define akp_err(...) do{printf("err:");printf(__VA_ARGS__);}while(0)

#define akp_debug(...)
#define akp_debug_exit
#define akp_debug_enter
#endif



struct sk_buff *tx_skb = 0;

struct netif if_wifi;
struct netif *p_netif = NULL; //一个netif对应一个网络设备

/* Define those to better describe your network interface. */
#define IFNAME0 'w'
#define IFNAME1 'i'

extern err_t wifi_if_init(struct netif *netif);


extern struct net_device *get_wifi_dev(void);
/**
 * Helper struct to hold private data used to operate your ethernet interface.
 * Keeping the ethernet address of the MAC in this struct is not necessary
 * as it is already kept in the struct netif.
 * But this is only an example, anyway...
 */
struct ethernetif {
  struct eth_addr *ethaddr;
  /* Add whatever per-interface state that is needed here. */
};



int wifi_netif_init()
{
	struct ip_addr ipaddr, netmask, gw;

	const char * IP_ADDR = 	"192.168.1.1";
	const char * GW_ADDR = 	"192.168.1.1";
	const char * MASK_ADDR = 	"255.255.255.0";
	
	gw.addr = inet_addr(GW_ADDR);
	ipaddr.addr = inet_addr(IP_ADDR);
	netmask.addr = inet_addr(MASK_ADDR);

	if (p_netif)
		netif_remove(p_netif);

	p_netif = &if_wifi;

	akp_info("wifi ip addr=%s, gw addr=%s, mask=%s\n",IP_ADDR,GW_ADDR,MASK_ADDR);
	if (netif_add(p_netif, &ipaddr, &netmask, &gw, NULL, wifi_if_init, tcpip_input) == 0)
	{
		akp_err("netif_add faild \r\n");
		return  - 1;
	}

	netif_set_default(p_netif);
	netif_set_up(p_netif);

	dhcps_stop();
	dhcps_start(ipaddr);
	
	return 0;
}


int wifistation_netif_init()
{
	struct ip_addr ipaddr, netmask, gw;
	unsigned int tick;
	
	gw.addr =  0;
	ipaddr.addr = 0;
	netmask.addr = 0;

	if (p_netif)
	{
		netif_remove(p_netif);
		netif_set_down(p_netif);
		dhcp_stop(p_netif);
	}

	p_netif = &if_wifi;
	if (netif_add(p_netif, &ipaddr, &netmask, &gw, NULL, wifi_if_init, tcpip_input) == 0)
	{
		akp_err("netif_add faild \r\n");
		return  - 1;
	}

	netif_set_default(p_netif);
	netif_set_up(p_netif);

	if (dhcp_start(p_netif) !=ERR_OK)
	{
		akp_err("dhcp_start failed");
		return -1;
	}
	tick = get_tick_count();
	while(1)
	{
		if (get_tick_count() - tick > 10000)
		{
			akp_err("Get ip by dhcp failed!\n");
			return -1;
		}
		if (p_netif->ip_addr.addr !=0)
		{
			akp_info("Get ip=%u.%u.%u.%u,gw=%u.%u.%u.%u,mask=%u.%u.%u.%u \n"
				, p_netif->ip_addr.addr & 0xff,(p_netif->ip_addr.addr & 0xff00)>>8,(p_netif->ip_addr.addr &0xff0000) >>16,p_netif->ip_addr.addr>>24
				, p_netif->gw.addr & 0xff,(p_netif->gw.addr & 0xff00)>>8,(p_netif->gw.addr &0xff0000) >>16,p_netif->gw.addr>>24
				, p_netif->netmask.addr & 0xff, (p_netif->netmask.addr & 0xff00)>>8,(p_netif->netmask.addr &0xff0000) >>16,p_netif->netmask.addr>>24 );
			break;
		}else
			sleep(10);
		
					
	}
	
	return 0;
}

//TODO: set dhcp

//int dhcp_setup()





