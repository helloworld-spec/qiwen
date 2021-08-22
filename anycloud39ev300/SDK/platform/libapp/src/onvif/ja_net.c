#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#include "ak_thread.h"
#include "ak_common.h"
#include "ak_net.h"

#include "net_dhcp.h"
#include "ak_onvif_config.h"


typedef struct
{
    ak_pthread_t	ipconflict_pth_id;
	ak_pthread_t	loop_pth_id;
	bool			bRunning;
	int				ip_adapt_tmp_en;
	int				cnt;
	time_t  		time;
	int 			peroid;
}IP_ADAPT_CTRL;

static IP_ADAPT_CTRL ip_adapt_ctrl = {0};

int IS_VALID_IPADDR(unsigned char *ip)
{
	if((NULL == ip) || (4 > ip[0]) || (127 == ip[0])
			|| (224 <= ip[0]) || (0 > ip[1]) || (255 < ip[1])
		|| (0 > ip[2]) || (255 < ip[2]) || (0 >= ip[3]) || (255 <= ip[3])) {
		ak_print_error_ex("invalid ip, %d.%d.%d.%d\n", ip[0], ip[1], ip[2], ip[3]);
		return AK_FALSE;
	}

	return AK_TRUE;
}

int IS_VALID_NETMASK(unsigned char *ipaddr)
{
	unsigned int b = 0, i;

	if (NULL != ipaddr) {
		unsigned char n[4] = {ipaddr[3], ipaddr[2], ipaddr[1], ipaddr[0]};

		for (i = 0; i < 4; ++i)
			b += n[i] << (i * 8);
		b = ~b + 1;
		if ((b & (b - 1)) == 0)
			return AK_TRUE;
	}

	return AK_FALSE;
}

#define IP_GET_VALUE_FROM_STRING(IP, IPSTR) \
{ \
	int tmp[4]; \
	IP[0] = 0; \
	IP[1] = 0; \
	IP[2] = 0; \
	IP[3] = 0; \
	if(sscanf(IPSTR, "%d.%d.%d.%d", &tmp[0], &tmp[1], &tmp[2], &tmp[3]) == 4) \
	{ \
		IP[0] = tmp[0]; \
		IP[1] = tmp[1]; \
		IP[2] = tmp[2]; \
		IP[3] = tmp[3]; \
	}else{\
		printf("ERR: parse ip address failed!\n");\
		IP[0] = 192; \
		IP[1] = 168; \
		IP[2] = 1; \
		IP[3] = 1; \
	}\
}

#define IP_VALUE_TO_STRING(__Prop, __text, __size) \
	snprintf(__text, (__size), "%d.%d.%d.%d",\
		(int)(__Prop[0]),\
		(int)(__Prop[1]),\
		(int)(__Prop[2]),\
		(int)(__Prop[3]))

#define UPTIME "/proc/uptime"

/**
 * get_uptime - get system uptime
 * return: uptime
 */
int get_uptime(void)
{
	FILE *filp = fopen(UPTIME, "r");
	if (!filp) {
		perror("fopen /proc/uptime");
		return -1;
	}

	char buf[100] = {0};
	float uptimes = 0.0;
	if (fgets(buf, 99, filp)) {
		if (sscanf(buf, "%f", &uptimes) == 1)
			ak_print_info_ex("get uptime : %d\n", (int)uptimes);
	}

	fclose(filp);

	return (int)uptimes;
}

/**
 * loop_proc - ip adapt stop working after 24 hours 
 * @param[IN]: 
 * return: 
 */
void* loop_proc(void *param)
{
	struct onvif_net_config *net_info = onvif_config_get_net();
	ak_print_normal_ex("thread id : %ld\n", ak_thread_get_tid());

	while (ip_adapt_ctrl.bRunning)
	{
		if (1 == net_info->ip_adjust_en)
		{
			ip_adapt_ctrl.cnt++;

			if (0 == (ip_adapt_ctrl.cnt % 60))
			{
				if (get_uptime() >= 24 * 60 * 60)
				{
					net_info->ip_adjust_en = 0;
					onvif_config_set_net(net_info);
					ak_print_normal_ex("close ip adjust!\n");
				}
			}
		}
		ak_sleep_ms(1000);
	}

	return NULL;
}

/**
 * get_ip_conflict_proc - get_ip_conflict thread function
 * @param[IN]: 
 * return: 
 */
void* get_ip_conflict_proc(void *param)
{
	char ipstr[32] = {0};

	ak_print_normal_ex("thread id : %ld\n", ak_thread_get_tid());

    while(ip_adapt_ctrl.bRunning)
    {
		ak_net_get_ip("eth0", ipstr);

	    g_ip_conflict_flag = NET_check_ip_conflict(ipstr);
	    if (g_ip_conflict_flag)
	        ak_print_normal_ex("IP %s conflict:%d\r\n", ipstr, g_ip_conflict_flag);

	    ak_sleep_ms(3000);
    }

	return NULL;
}

/**
 * ja_net_get_dhcp_status - get dhcp status
 * return: dhcp status
 */
unsigned int ja_net_get_dhcp_status(void)
{
#if 1
    return 0; //disable DHCP
#else
	char buf[10] = {0};

	int fd = open("/tmp/dhcp_status", O_RDONLY);
	if(fd < 0) {
		perror("open");
		return 0;
	}
	read(fd, buf, 1);

	close(fd);
	ak_print_normal_ex("dhcp: %d\n", atoi(buf));

	return atoi(buf);
#endif

}

/**
 * ja_net_found_ip - callback function for ip adapt
 * @origin_ip[IN]: no use
 * @origin_netmask[IN]: no use
 * @szip[IN]: ipaddr to set
 * @szmac[IN]: no use
 * return: 1 - success, fail return 0.
 */
void ja_net_found_ip(char *origin_ip, char *origin_netmask, char *szip, char *szmac)
{
    unsigned int my_ip[4];
	char gateway[64]={0};
	char def_eth[128];

	if(NULL == getenv("DEF_ETHER")){
		snprintf(def_eth, sizeof(def_eth), "eth0");
	}else{
		snprintf(def_eth, sizeof(def_eth), "%s", getenv("DEF_ETHER"));
	}

	struct onvif_net_config *net_info = onvif_config_get_net();
	if ( !strcmp("eth0", def_eth) ) {
		net_info = onvif_config_get_net();
	} else {
		//net_info = anyka_get_sys_wireless_net_setting();
	}

 	ak_print_normal_ex("[%s] set new ip:%s\n", __func__, szip);

 	if (4 != sscanf(szip, "%u.%u.%u.%u", &my_ip[0], &my_ip[1], &my_ip[2], &my_ip[3]))
		return;

	unsigned char ip[4] = {my_ip[0], my_ip[1], my_ip[2], my_ip[3]};
	if (!IS_VALID_IPADDR(ip))
		return;

    //config ip
    ak_net_set_ip(def_eth, szip);

	my_ip[3] = 1; //set last field as 1, for gateway

	bzero(gateway, sizeof(gateway));
	sprintf(gateway, "%u.%u.%u.%u", my_ip[0], my_ip[1], my_ip[2], my_ip[3]);

    //config gateway
	ak_net_set_default_gateway(gateway);

    //save ip & gateway
	strncpy(net_info->ipaddr, szip, 16);
	strncpy(net_info->gateway, gateway, 16);
	net_info->dhcp = 0;

	if ( !strcmp("eth0", def_eth) ) {
		onvif_config_set_net(net_info);
		onvif_config_flush_data();
	} else {
		//anyka_save_wireless_netinfo(net_info);
	}
}

/**
 * ja_net_init_ip_adapt - init ip adapt
 * return: void
 */
void ja_net_init_ip_adapt(void)
{
	ip_adapt_ctrl.ip_adapt_tmp_en = AK_FALSE;
	ip_adapt_ctrl.bRunning = AK_TRUE;
	ip_adapt_ctrl.peroid = 0;
	ip_adapt_ctrl.time = 0;
	
	ak_thread_create(&ip_adapt_ctrl.ipconflict_pth_id, get_ip_conflict_proc,
					(void *)NULL, ANYKA_THREAD_MIN_STACK_SIZE, -1);
	ak_thread_create(&ip_adapt_ctrl.loop_pth_id, loop_proc,
					(void *)NULL, ANYKA_THREAD_NORMAL_STACK_SIZE, -1);

}

/**
 * ja_net_destroy_ip_adapt - destroy ip adapt
 * return: void
 */
void ja_net_destroy_ip_adapt(void)
{
	ip_adapt_ctrl.bRunning = AK_FALSE;
	ak_thread_join(ip_adapt_ctrl.ipconflict_pth_id);
	ak_thread_join(ip_adapt_ctrl.loop_pth_id);
}

/**
 * ja_net_set_ip_adapt_tmp_enable - set ip adapt tmp enable flag
 * @enable[IN]: tmp enable flag
 * return: void
 */
void ja_net_set_ip_adapt_tmp_enable(int enable)
{
	ip_adapt_ctrl.ip_adapt_tmp_en = enable;
}

/**
 * ja_net_get_ip_adapt_tmp_enable - get ip adapt tmp enable flag
 * return: tmp enable flag
 */
int ja_net_get_ip_adapt_tmp_enable(void)
{
	return ip_adapt_ctrl.ip_adapt_tmp_en;
}

/**
 * ja_net_update_ip_adapt_time - update ip adapt time
 * return: void
 */
void ja_net_update_ip_adapt_time(void)
{
	ip_adapt_ctrl.time = time(NULL);
}

/**
 * ja_net_get_ip_adapt_time - get ip adapt time
 * return: time
 */
int ja_net_get_ip_adapt_time(void)
{
	return ip_adapt_ctrl.time;
}

/**
 * ja_net_set_ip_adapt_peroid - set ip adapt peroid
 * @peroid[IN]: peroid
 * return: peroid
 */
void ja_net_set_ip_adapt_peroid(int peroid)
{
	ip_adapt_ctrl.peroid = peroid;
}

/**
 * ja_net_get_ip_adapt_peroid - get ip adapt peroid
 * return: peroid
 */
int ja_net_get_ip_adapt_peroid(void)
{
	return ip_adapt_ctrl.peroid;
}

/**
 * ja_net_check_net_parm - check ipaddr, neetmask, gateway
 * @ipaddr[IN]: ipaddr string
 * @netmask[IN]: netmask string
 * @gateway[IN]: gateway string
 * return: 1 - success, fail return 0.
 */
int ja_net_check_net_parm(char* ipaddr, char* netmask, char* gateway)
{
	unsigned char ip[4] = {0};
	unsigned char mask[4] = {0};
	unsigned char gw[4] = {0};
	
    if (NULL == ipaddr || NULL == netmask || NULL == gateway)
    {
        ak_print_error_ex("parm null\n");
        return AK_FALSE;
    }

	IP_GET_VALUE_FROM_STRING(ip, ipaddr);
	IP_GET_VALUE_FROM_STRING(mask, netmask);
	IP_GET_VALUE_FROM_STRING(gw, gateway);

    if (!IS_VALID_IPADDR(ip))
    {
        return AK_FALSE;
    }

    if ((0 == mask[0]) && (0 == mask[1]) && (0 == mask[2]) && (0 == mask[3]))
    {
        ak_print_error_ex("mask is all 0\n");
        return AK_FALSE;
    }

    if ((255 == mask[0]) && (255 == mask[1]) && (255 == mask[2]) && (255 == mask[3]))
    {
        ak_print_error_ex("mask is all 255\n");
        return AK_FALSE;
    }

	if (!IS_VALID_NETMASK(mask)) {
        ak_print_error_ex("invalid netmask: %d.%d.%d.%d\r\n", mask[0], mask[1], mask[2], mask[3]);
        return AK_FALSE;
	}

	if (!IS_VALID_IPADDR(gw))
	{
        ak_print_error_ex("invalid gateway: %d.%d.%d.%d\r\n", gw[0], gw[1], gw[2], gw[3]);
		return AK_FALSE;
	}


    if (((ip[0] & mask[0]) != (gw[0] & mask[0]))
    || ((ip[1] & mask[1]) != (gw[1] & mask[1]))
    || ((ip[2] & mask[2]) != (gw[2] & mask[2]))
    || ((ip[3] & mask[3]) != (gw[3] & mask[3])))
    {
        ak_print_error_ex("ip and gw are not in the same network section\n");

        gw[0] = (mask[0] == 255) ? ip[0]: gw[0];
		gw[1] = (mask[1] == 255) ? ip[1]: gw[1];
		gw[2] = (mask[2] == 255) ? ip[2]: gw[2];

		ak_print_normal_ex("modify gw to %d.%d.%d.%d\n", gw[0], gw[1], gw[2], gw[3]);
		IP_VALUE_TO_STRING(gw, gateway, 32);
    }

    return AK_TRUE;
}

/**
 * ja_net_set_wired_net_parm - set wired ipaddr, neetmask, gateway
 * @ipaddr[IN]: ipaddr string
 * @netmask[IN]: netmask string
 * @gateway[IN]: gateway string
 * return: void
 */
int ja_net_set_wired_net_parm(char* ipaddr, char* netmask, char* gateway)
{
	char ip[32]={0};
	char gw[32]={0};
	char mask[32]={0};
	struct onvif_net_config *net_info = onvif_config_get_net();

	if (!ja_net_check_net_parm(ipaddr, netmask, gateway))
        return AK_FAILED;
	
	ak_net_set_ip("eth0", ipaddr);
	ak_net_set_netmask("eth0", netmask);
	ak_net_set_default_gateway(gateway);

	ak_net_get_ip("eth0", ip);
	ak_net_get_netmask("eth0", mask);
	ak_net_get_route("eth0", gw);

	if ((0 != strcmp(ip, ipaddr))
		|| (0 != strcmp(gw, gateway))
		|| (0 != strcmp(mask, netmask)))
	{
		ak_print_error_ex("Test: Set Wired Network failed.\n");

		ak_net_set_ip("eth0", net_info->ipaddr);
		ak_net_set_netmask("eth0", net_info->netmask);
		ak_net_set_default_gateway(net_info->gateway);

		return AK_FAILED;
	}

	//force diable DHCP
	net_info->dhcp = 0; //LanSetupWired->NetWired.EnableDHCP.val;
	strncpy(net_info->ipaddr, ipaddr, 16);
	strncpy(net_info->netmask, netmask, 16);
	strncpy(net_info->gateway, gateway, 16);

	ak_print_normal_ex("set ip:%s, netmask:%s, gateway:%s, dhcp:%d\n",
			ipaddr, netmask, gateway, net_info->dhcp);

	onvif_config_set_net(net_info);
	onvif_config_flush_data();

	return AK_SUCCESS;
}
