#ifndef _JA_NET_H_
#define _JA_NET_H_

/**
 * ja_net_get_dhcp_status - get dhcp status
 * return: dhcp status
 */
int ja_net_get_dhcp_status(void);

/**
 * ja_net_found_ip - callback function for ip adapt
 * @origin_ip[IN]: no use
 * @origin_netmask[IN]: no use
 * @szip[IN]: ipaddr to set
 * @szmac[IN]: no use
 * return: 1 - success, fail return 0.
 */
void ja_net_found_ip(char *origin_ip, char *origin_netmask, char *szip, char *szmac);

/**
 * ja_net_init_ip_adapt - init ip adapt
 * return: void
 */
void ja_net_init_ip_adapt(void);

/**
 * ja_net_destroy_ip_adapt - destroy ip adapt
 * return: void
 */
void ja_net_destroy_ip_adapt(void);

/**
 * ja_net_set_ip_adapt_tmp_enable - set ip adapt tmp enable flag
 * @enable[IN]: tmp enable flag
 * return: void
 */
void ja_net_set_ip_adapt_tmp_enable(int enable);

/**
 * ja_net_get_ip_adapt_tmp_enable - get ip adapt tmp enable flag
 * return: tmp enable flag
 */
int ja_net_get_ip_adapt_tmp_enable(void);

/**
 * ja_net_update_ip_adapt_time - update ip adapt time
 * return: void
 */
void ja_net_update_ip_adapt_time(void);

/**
 * ja_net_get_ip_adapt_time - get ip adapt time
 * return: time
 */
int ja_net_get_ip_adapt_time(void);

/**
 * ja_net_set_ip_adapt_peroid - set ip adapt peroid
 * @peroid[IN]: peroid
 * return: peroid
 */
void ja_net_set_ip_adapt_peroid(int peroid);

/**
 * ja_net_get_ip_adapt_peroid - get ip adapt peroid
 * return: peroid
 */
int ja_net_get_ip_adapt_peroid(void);

/**
 * ja_net_check_net_parm - check ipaddr, neetmask, gateway
 * @ipaddr[IN]: ipaddr string
 * @netmask[IN]: netmask string
 * @gateway[IN]: gateway string
 * return: 1 - success, fail return 0.
 */
int ja_net_check_net_parm(char* ipaddr, char* netmask, char* gateway);

/**
 * ja_net_set_wired_net_parm - set wired ipaddr, neetmask, gateway
 * @ipaddr[IN]: ipaddr string
 * @netmask[IN]: netmask string
 * @gateway[IN]: gateway string
 * return: 0 - success, fail return -1.
 */
int ja_net_set_wired_net_parm(char* ipaddr, char* netmask, char* gateway);

#endif
