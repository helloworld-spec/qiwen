/*@file
 * 
 *
 */
#include <stdint.h> 
#include <stddef.h>
#include "wifi.h"
#include "wiced_result.h"
#include "wwd_structures.h"
#include "wwd_debug.h"



#define WIFI_LIB_VERSION   "BCM-1.0.0"

//this interface is initialized by netif_add() in lwip interface
extern struct netif *p_netif; 


char *wifi_get_version(void)
{
	return WIFI_LIB_VERSION;
}

/**
 * @brief  读取mac地址，mac地址从8782芯片读取
 * @param  mac_addr : mac地址 6BYTE
 */
 //TODO:
int wifi_get_mac(unsigned char *mac_addr)
{
	char mac[6] = {0};
	memcpy(mac_addr,  mac, 6);
	return 0;
}


/**
 * @brief  配置省电模式,在保持连接的情况下可达到很可观省电效果
 *		省电模式下的耗电量由数据收发量决定
 *		此外用户可以配置更加细节的电源管理参数,比如DTIM间隔
 *		一般情况下使用此函数已经够用
 *
 * @param  power_save_level : 取值 0,1,2; 
 *	0代表关闭省电模式(CAM)
 *	1采用DTIM省电方式
 *    2采用INACTIVITY省电方式
 * @param  mac : 指向需要获取的station的地址
 */
int wifi_power_cfg(uint8_t power_save_level)
{
	return 0;
}

/**
 * @brief  wifi_get_mode, get wifi mode
 * @param  : 
 * @retval AK_WIFI_MODE
 */

int wifi_get_mode()
{
	
	return 0;
}

/**
 * @brief  wifi_set_mode, set wifi mode.
 * ap_wifi_dev and sta_wifi_dev are two different device, set mode just set the global variable wifi_dev
 * @param  : 
 * @retval AK_WIFI_MODE
 */

int wifi_set_mode(int type)
{
	return 0;
}


/**
 * @brief  wifi_isconnected, return wifi connect status
 * @param  : 
 * @retval 1 - connect , 0 - not connected 
 */
//TODO:
int wifi_isconnected()
{

	return 0;
}

/**
 * @brief  wifi_connect, wifi connect to ap 
 * @param  : ssid, key
 * @retval 0 - send connect msg ok , -1 - some error happen 
 */

int wifi_connect( char *essid, char *key)
{

	wiced_result_t		join_result = WICED_STA_JOIN_FAILED;
	wiced_scan_result_t temp_scan_result;

	WPRINT_WICED_INFO(("Joining : %s\n", essid));
	
	wiced_security_t security;

	//TODO: make security unnecessary 
	security = WICED_SECURITY_WPA2_MIXED_PSK;
	/* Try scan and join AP */
	join_result = (wiced_result_t) wwd_wifi_join( essid, security, (uint8_t*) key, strlen(key), NULL, WWD_STA_INTERFACE );
	
	if ( join_result == WICED_SUCCESS )
	{
		WPRINT_WICED_INFO( ( "Successfully joined : %s\n", essid) );

		//wwd_management_set_event_handler( link_events, wiced_link_events_handler, NULL, WWD_STA_INTERFACE );
		//return WICED_SUCCESS;
	}
	else
	{
		WPRINT_WICED_INFO(("Failed to join : %s\n", essid));
	}
	return join_result;
}


/**
 * @brief  wifi_disconnect, wifi disconnect from ap 
 * @param  : ssid, key
 * @retval 0 - send connect msg ok , -1 - some error happen 
 */

int wifi_disconnect()
{
	return wwd_wifi_leave( WWD_STA_INTERFACE );
}




/**
 * @brief  wifi_create_ap, destroy
 * @param  : 
 * @retval AK_WIFI_MODE
 */

int wifi_create_ap(struct _apcfg *ap_cfg)
{
	wiced_security_t security;
	/*seurity convert*/
	switch(ap_cfg->enc_protocol)
	{
		case KEY_NONE:
			security = WICED_SECURITY_OPEN;
			break;
		case KEY_WEP:
			security = WICED_SECURITY_WEP_PSK;
			break;
		case KEY_WPA:
			security = WICED_SECURITY_WPA_MIXED_PSK;
			break;
		case KEY_WPA2:
			security = WICED_SECURITY_WPA2_MIXED_PSK;
			break;
		default:
			security = WICED_SECURITY_OPEN;
			break;
	}
	
    return (wiced_result_t) wwd_wifi_start_ap(ap_cfg->ssid, security, (uint8_t*)ap_cfg->key, (uint8_t) strlen(ap_cfg->key),ap_cfg->channel);

}

/**
 * @brief  wifi_destroy_ap, destroy
 * @param  : 
 * @retval AK_WIFI_MODE
 */

int wifi_destroy_ap(void)
{
	return wwd_wifi_stop_ap();
}


/**
 * @brief initializing wifi 
 * @author
 * @date 
 * @param [in] pParam a pointer to T_WIFI_INITPARAM type
 * @return int
 * @retval   0  initializing sucessful
 * @retval  -1 initializing fail
 */
int wifi_init()
{
	wwd_result_t result;

	result = wwd_management_wifi_on( WICED_COUNTRY_CHINA );
    if ( result != WWD_SUCCESS )
    {
        WPRINT_LIB_INFO(("Error %d while init wifi!\n", result));
    }
	else
	{
		WPRINT_LIB_INFO(("init wifi OK!\n"));
	}
	
	return result;
}



