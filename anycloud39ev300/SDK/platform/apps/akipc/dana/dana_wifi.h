#ifndef _DANA_WIFI_H_
#define _DANA_WIFI_H_

struct ak_ap_info {
    char essid[33];
    unsigned int enc_type;
    unsigned int quality;
} ;

struct ak_ap_list{
    int ap_num;
    struct ak_ap_info ap_info[32];
} ;

/**
 * dana_wifi_get_ap_list - get  wifi ap list   
 * return:   wifi ap list or null
 */
struct ak_ap_list* dana_wifi_get_ap_list(void);

/**
 * dana_wifi_net_chk - check net configure if it is ok.  
 * @if_name[IN]: 	 net point name   
 * return:   1, net is ok;  0 , net is  failed;
 */
int dana_wifi_net_chk(const char *if_name);

/**
 * dana_wifi_driver_chk - check wifi driver if it install successfully  
 * return:     0 , success ;  -1 , failed;
 */
int dana_wifi_driver_chk(void);

/**
 * dana_wifi_conn_new_ssid - use new ssid to wifi.  
 * @ssid[IN]: 	 ssid string  
 * @pswd[IN]: passwd string 
 * return:   1, connected;  0 , not;
 */
int dana_wifi_conn_new_ssid(const char *ssid, const char* pswd);

#endif
