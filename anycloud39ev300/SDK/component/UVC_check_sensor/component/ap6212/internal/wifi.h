#ifndef _WIFI_H_
#define _WIFI_H_

/* SDIO CIS Tuple code */
#define CISTPL_NULL      0x00
#define CISTPL_CHECKSUM  0x10
#define CISTPL_VERS_1    0x15
#define CISTPL_ALTSTR    0x16
#define CISTPL_MANFID    0x20
#define CISTPL_FUNCID    0x21
#define CISTPL_FUNCE     0x22
#define CISTPL_SDIO_STD  0x91
#define CISTPL_SDIO_EXT  0x92
#define CISTPL_END       0xff

#define WIFI_POWERDOWN_PIN		GPIO_WIFI_POWERDOWN

extern char *Wifi_GetVersion(void);

//set wifi module
//mwj+
typedef enum
{
	WIFI_MODE_STA = 0, 
	WIFI_MODE_AP,
	WIFI_MODE_ADHOC,
	WIFI_MODE_UNKNOWN
	
}AK_WIFI_MODE;

#define MAC_ADDR_LEN 6
#define MAX_SSID_LEN 32
#define MIN_KEY_LEN  8
#define MAX_KEY_LEN  64



struct _apcfg
{
	unsigned char mac_addr[MAC_ADDR_LEN];
	unsigned char ssid[MAX_SSID_LEN];
	unsigned int  ssid_len;
	unsigned char mode;    // bg, bgn, an,...
	unsigned char channel;
	unsigned char txpower; //0 means max power
	unsigned int  enc_protocol; // encryption protocol  eg.wep, wpa, wpa2 
	unsigned char key[MAX_KEY_LEN];
	unsigned int  key_len;
	
};

//user interface to control wifi 

/**
 * @brief  wifi_get_mode, get wifi mode
 * @param  : 
 * @retval AK_WIFI_MODE
 */
int wifi_get_mode();  

/**
 * @brief  wifi_set_mode, set wifi mode.
 * ap_wifi_dev and sta_wifi_dev are two different device, set mode just set the global variable wifi_dev
 * @param  : 
 * @retval AK_WIFI_MODE
 */
int wifi_set_mode(int type);  


 /**
 * @brief  wifi_connect, wifi connect to ap 
 * @param  : ssid, key
 * @retval 0 - send connect msg ok , -1 - some error happen 
 */
int wifi_connect(char *ssid, char *password); 

 /**
 * @brief  wifi_disconnect, wifi disconnect from ap 
 * @param  : ssid, key
 * @retval 0 - send connect msg ok , -1 - some error happen 
 */
int wifi_disconnect(); 

/**
 * @brief  wifi_isconnected, return wifi connect status
 * @param  : 
 * @retval 1 - connect , 0 - not connected 
 */
int wifi_isconnected();  

/**
 * @brief  wifi_create_ap, destroy
 * @param  : 
 * @retval AK_WIFI_MODE
 */
int wifi_create_ap(struct _apcfg *ap_cfg);

/**
 * @brief  wifi_destroy_ap, destroy
 * @param  : 
 * @retval AK_WIFI_MODE
 */
int wifi_destroy_ap();

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
int wifi_power_cfg(uint8_t power_save_level);


//kernel interface for lwip interface
int wifi_get_mac(unsigned char *mac_addr);
int netif_tx(void *data);
int netif_rx(void *data);
int wifi_init();


//wmj<<

typedef enum
{
	KEY_NONE = 0, KEY_WEP, KEY_WPA, KEY_WPA2, KEY_MAX_VALUE
} SECURITY_TYPE;

typedef struct _WIFI_INITPARAM{
	unsigned int size;
	unsigned char *cache;
}T_WIFI_INITPARAM, *T_pWIFI_INITPARAM;

extern struct net_device *wifi_dev;
//wmj+ 20160913

//wmj+ for 11n
#define DOT11N_ENABLE 1
#define DOT11N_DISABLE 0
#define WIFI_MAX_CLIENT_DEFAULT 4
//wmj+ for tx pending control
#define DEFAULT_MAX_TX_PENDING 100

#define UDP_MTU  (1460) //wmj+


#ifdef MAX_TX_PENDING_CTRL
int wifi_get_tx_pending();
int wifi_set_max_tx_pending(u8 max_tx_pending);
int wifi_get_max_tx_pending();
#endif
//wmj<<

#endif
