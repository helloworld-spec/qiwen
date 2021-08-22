/*
 * 
 *
 */
#define DEBUG

#include "cfg80211.h"
#include "defs.h"
#include "xrf_type.h"
#include "types.h"

#include "moal_sdio.h"
#include "moal_main.h"
#include "moal_uap.h"
#include "moal_uap_priv.h"
#include "mlan_ioctl.h"

#include "wpa.h"
#include "wifi.h"
#include "gpio_config.h"
#include "arch_mmc_sd.h"

#include "lwip/netif/etharp.h"
#include "lwip/tcpip.h"


#define WIFI_LIB_VERSION	"Wifi_marvell_8801_LIB_V1.0.10"

/*
*分为station和ap两个设备,其接口函数是不一样的
*所以在不同的模式下我们选择相对应的设备进行操作
*wifi_dev指向当前设备
*/
struct net_device *sta_wifi_dev = NULL;
struct net_device *ap_wifi_dev = NULL;
struct net_device *direct_wifi_dev = NULL;

struct net_device *wifi_dev = NULL;

#ifdef FW_OUTSIDE
T_WIFI_INITPARAM wifi_initparam; 
#endif

#define SSID_MAX_LEN 32
#define KEY_MAX_LEN 64

//this interface is initialized by netif_add() in lwip interface
extern struct netif *p_netif; 


char *Wifi_GetVersion(void)
{
	return WIFI_LIB_VERSION;
}


struct net_device *get_wifi_dev()
{
	return wifi_dev;
}
#pragma arm section code ="_bootcode_"

int is_hw_ok()
{
	return wifi_dev?1:0;
}

#pragma arm section code 


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
	int ret = -1;
	moal_private *priv;

	priv = (moal_private *)wifi_dev->ml_priv;
	if(priv->bss_role == MLAN_BSS_ROLE_UAP)
	{
		if(wifi_dev->netdev_ops && wifi_dev->netdev_ops->ndo_do_ioctl)
		{
			struct iwreq *wrq = (struct iwreq *)mem_calloc(sizeof(struct iwreq), 1);
			if(wrq)
			{
				/*
				if(sleep)
				{
					deep_sleep_para param;
					memset(&param, 0, sizeof(deep_sleep_para));
					wrq->u.data.pointer = &param;
					param.subcmd = UAP_DEEP_SLEEP;
					param.action = MLAN_ACT_SET;
					param.deep_sleep = TRUE;
					param.idle_time = 1000;
					ret = wifi_dev->netdev_ops->ndo_do_ioctl(wifi_dev, (void*)wrq, UAP_IOCTL_CMD);
	
				}else
				*/
				{
					mlan_ds_ps_mgmt ps_mgmt;

					memset(&ps_mgmt, 0, sizeof(mlan_ds_ps_mgmt));
					wrq->u.data.pointer = &ps_mgmt;

					ps_mgmt.flags = PS_FLAG_PS_MODE;
					if(power_save_level == 0)
						ps_mgmt.ps_mode = PS_MODE_DISABLE;
					else if(power_save_level == 1)
						ps_mgmt.ps_mode = PS_MODE_PERIODIC_DTIM;
					else if(power_save_level == 2)
						ps_mgmt.ps_mode = PS_MODE_INACTIVITY;
					else
						ps_mgmt.ps_mode = PS_MODE_DISABLE;
					
					ret = wifi_dev->netdev_ops->ndo_do_ioctl(wifi_dev, (void*)wrq, UAP_POWER_MODE);
				}

				mem_free(wrq);
			}
		}
	}

	if(priv->bss_role == MLAN_BSS_ROLE_STA)
	{
		if(wifi_dev->cfg80211_ops && wifi_dev->cfg80211_ops->set_power_mgmt)
		{
			bool enable;
			if(power_save_level == 0)
				enable = FALSE;
			else
				enable = TRUE;
			ret = wifi_dev->cfg80211_ops->set_power_mgmt(priv->wdev->wiphy, wifi_dev,enable, 0); 
		}
	}
	
	return ret;
}

/**
 * @brief  wifi_get_mode, get wifi mode
 * @param  : 
 * @retval AK_WIFI_MODE
 */

int wifi_get_mode()
{
	
	moal_private *priv;
	priv = (moal_private *)wifi_dev->ml_priv;
	if(priv->bss_role == MLAN_BSS_ROLE_UAP)
	{
		return WIFI_MODE_AP;
	}
	else if(priv->bss_role == MLAN_BSS_ROLE_STA)
	{
		return WIFI_MODE_STA;
	}
	else
	{
		p_dbg("get 8801 wifi mode error\n");
		return WIFI_MODE_UNKNOWN;
	}
}

/**
 * @brief  wifi_set_mode, set wifi mode.
 * ap_wifi_dev and sta_wifi_dev are two different device, set mode just set the global variable wifi_dev
 * @param  : 
 * @retval AK_WIFI_MODE
 */

int wifi_set_mode(int type)
{
	int ret = 0;
	int set_type;
	moal_private *priv;
	
	struct net_device *saved_dev = wifi_dev;

	priv = (moal_private *)wifi_dev->ml_priv;
	//not support AP+ station, 
	if(priv->bss_role == MLAN_BSS_ROLE_UAP)
	{
		wifi_destroy_ap();
	}
	else
	{
		wifi_disconnect();
	}
	
	if(0)
		;
#ifdef UAP_SUPPORT		
	else if(type == WIFI_MODE_AP){
		set_type = NL80211_IFTYPE_AP;
		wifi_dev = ap_wifi_dev;
	}
#endif
#ifdef STA_SUPPORT	
	else if(type == WIFI_MODE_STA){
		wifi_dev = sta_wifi_dev;
		set_type = NL80211_IFTYPE_STATION;
	}
#endif
#ifdef ADHOC_SUPPORT

	else if(type == WIFI_MODE_ADHOC){
		wifi_dev = sta_wifi_dev;
		set_type = NL80211_IFTYPE_ADHOC;
	}
#endif
#ifdef WIFI_DIRECT_SUPPORT	
	else if(type == WIFI_MODE_P2P_GO){
		wifi_dev = direct_wifi_dev;
		set_type = NL80211_IFTYPE_P2P_GO;
	}
	else if(type == WIFI_MODE_P2P_GC){
		wifi_dev = direct_wifi_dev;
		set_type = NL80211_IFTYPE_P2P_CLIENT;
	}
#endif
	else{
		p_err("type %d not support", type);
		return -1;
	}

	if(wifi_dev == NULL){
		
		wifi_dev = saved_dev;
		p_err("%s not support", __func__ );
		return -1;
	}
#if 0  //mode already setted at wifi init
	if(wifi_dev->cfg80211_ops->change_virtual_intf){
		priv = (moal_private *)wifi_dev->ml_priv;
		p_dbg("change_virtual_intf");
		//ap和sta指向相同的change_virtual_intf
		ret = wifi_dev->cfg80211_ops->change_virtual_intf(priv->wdev->wiphy, wifi_dev, (enum nl80211_iftype)set_type, 0, 0);
		if(ret != 0)
		{
			wifi_dev = saved_dev;
		}
	}else
		p_err("%s not support", __func__ );
#endif
	
	return ret;
}


/**
 * @brief  wifi_isconnected, return wifi connect status
 * @param  : 
 * @retval 1 - connect , 0 - not connected 
 */

int wifi_isconnected()
{

	int ret = 0;
	moal_private *priv;
	priv = (moal_private *)wifi_dev->ml_priv;
	
	if(priv->media_connected)
		ret = 1;

	return ret;
}

/**
 * @brief  wifi_connect, wifi connect to ap 
 * @param  : ssid, key
 * @retval 0 - send connect msg ok , -1 - some error happen 
 */

int wifi_connect( char *essid, char *key)
{

	int result_ret = 0;
	moal_private *priv;
	int essid_len = strlen(essid);
	int key_len = strlen(key);
	struct cfg80211_connect_params *smee = 0;

	if(essid_len > 32 || key_len > 32)
	{
		result_ret = -1;
		goto ret;
	}
	smee = (struct cfg80211_connect_params *)mem_malloc(sizeof(struct cfg80211_connect_params));
	if(smee == 0)
	{
		result_ret = -1;
		goto ret;
	}

	priv = (moal_private *)wifi_dev->ml_priv;
	memset(smee,0,sizeof(struct cfg80211_connect_params));
	smee->bssid = 0; 
	
	memcpy(smee->ssid,essid,essid_len);
	smee->ssid[essid_len] = 0;
	
	smee->ssid_len = essid_len;
	
	memcpy(smee->key, key,key_len);
	smee->key[key_len] = 0;
	smee->key_len = key_len;
	/*wifi lib will fill these member
	smee->auth_type = NL80211_AUTHTYPE_AUTOMATIC;
	smee->crypto.wpa_versions = NL80211_WPA_VERSION_2;
	smee->crypto.cipher_group = MLAN_ENCRYPTION_MODE_CCMP;
	smee->crypto.n_ciphers_pairwise = 1;
	smee->crypto.ciphers_pairwise[0] = MLAN_ENCRYPTION_MODE_CCMP;
	*/

/*
 *smee结构只填充了部分成员，其他成员将在连接的时候自动填充(根据目标ap的信息)
*/
	
	result_ret = wifi_dev->cfg80211_ops->wifi_connect(priv->wdev->wiphy, wifi_dev,smee);
ret:
	if(smee)
		mem_free(smee);
	if(result_ret)
		p_err("user_link_app err:%d\n",result_ret);

	p_dbg("exit %s\n", __func__ );
	
	return result_ret;
}

/**
 * @brief  wifi_disconnect, wifi disconnect from ap 
 * @param  : ssid, key
 * @retval 0 - send connect msg ok , -1 - some error happen 
 */

int wifi_disconnect()
{
	int ret = -1;
	moal_private *priv;

	if(sta_wifi_dev)
	{
		priv = (moal_private *)sta_wifi_dev->ml_priv;
		if(sta_wifi_dev->cfg80211_ops && sta_wifi_dev->cfg80211_ops->disconnect)
		{
			ret = sta_wifi_dev->cfg80211_ops->disconnect(priv->wdev->wiphy, sta_wifi_dev, 0);

		}else
			p_err("%s not support", __func__ );
	}
	
	return ret;
}




/**
 * @brief  wifi_create_ap, destroy
 * @param  : 
 * @retval AK_WIFI_MODE
 */

int wifi_create_ap(struct _apcfg *ap_cfg)
{
	p_dbg_enter;
	wifi_ap_cfg(ap_cfg->ssid, ap_cfg->mode, ap_cfg->key, ap_cfg->enc_protocol, ap_cfg->channel, WIFI_MAX_CLIENT_DEFAULT);
	wifi_power_cfg(0);
	p_dbg_exit;
}

/**
 * @brief  wifi_destroy_ap, destroy
 * @param  : 
 * @retval AK_WIFI_MODE
 */

int wifi_destroy_ap()
{
	int ret = -1;
	p_dbg_enter;
	if(ap_wifi_dev)
	{
		if(ap_wifi_dev->netdev_ops && ap_wifi_dev->netdev_ops->ndo_do_ioctl)
		{
			ret = ap_wifi_dev->netdev_ops->ndo_do_ioctl(ap_wifi_dev, 0, WOAL_UAP_FROYO_AP_BSS_STOP);
		}
	}
	p_dbg_exit;
	return ret;
}


/**
*wifi_ap_cfg, set ap config with 11n param
*@param name: ssid
*@param 11n_mode:  1 - enable 11n , others - disable 11n
*@param key: ciper
*@param key_type: key mode KEY_NONE, KEY_WEP,KEY_WPA,KEY_WPA2
*@param channel: wifi channel
*@param max_client: max client allowed
*/
int wifi_ap_cfg(char *name, int dot11n_enable, char *key, int key_type, int channel, int max_client)
{
	int ret = -1;
	char str_tmp[64];
	char *opt_ptr, data_buf[256];
	struct iwreq wrq;
	
	
	//配置ap参数我们没有使用cfg80211_ops提供的, 而是使用更简单的netdev_ops接口
	//netdev_ops接口比较丰富,这里做一个参考
	if(wifi_dev->netdev_ops && wifi_dev->netdev_ops->ndo_do_ioctl)
	{
		memset(data_buf, 0, 256);
		opt_ptr = data_buf;
		
		if(strlen(name) > SSID_MAX_LEN)
		{
			p_err("SSID len should less than %d", SSID_MAX_LEN);
			return ret;
		}
		if(strlen(key) > KEY_MAX_LEN)
		{
			p_err("key len should less than %d", KEY_MAX_LEN);
			return ret;
		}
		
		/*use '\0' as delimiter to support special character as ,./"\=*/
		
		strcpy(opt_ptr, "ASCII_CMD=AP_CFG");

		opt_ptr += strlen("ASCII_CMD=AP_CFG");
		*opt_ptr++ = 0;

		/*SSID*/
		//snprintf(str_tmp, 64, "SSID=%s", name);
		strcpy(opt_ptr, "SSID=");
		opt_ptr += strlen("SSID=");
		strcpy(opt_ptr, name);
		opt_ptr += strlen(name);
		*opt_ptr++ = 0;

		/*dot11n */
		if(dot11n_enable == 1)
		{
			strcpy(str_tmp, "11N_MODE=1");
		}
		else
		{
			strcpy(str_tmp, "11N_MODE=0");
		}
		strcpy(opt_ptr, str_tmp);
		opt_ptr += strlen(str_tmp);
		*opt_ptr++ = 0;

		/*SECURYTY*/
		if (key_type == KEY_NONE)
			strcpy(str_tmp, "SEC=open");
		else if (key_type == KEY_WEP)
			strcpy(str_tmp, "SEC=wep128");
		else if (key_type == KEY_WPA)
			strcpy(str_tmp, "SEC=wpa-psk");
		else if (key_type == KEY_WPA2)
			strcpy(str_tmp, "SEC=wpa2-psk");
		else
			strcpy(str_tmp, "SEC=open");
		
		strcpy(opt_ptr, str_tmp);
		opt_ptr += strlen(str_tmp);
		*opt_ptr++ = 0;

		snprintf(str_tmp, 64, "KEY=%s", key);
		strcpy(opt_ptr, str_tmp);
		opt_ptr += strlen(str_tmp);
		*opt_ptr++ = 0;

		snprintf(str_tmp, 64, "CHANNEL=%d", channel);
		strcpy(opt_ptr, str_tmp);
		opt_ptr += strlen(str_tmp);
		*opt_ptr++ = 0;

		snprintf(str_tmp, 64, "MAX_SCB=%d", max_client);
		strcpy(opt_ptr, str_tmp);
		opt_ptr += strlen(str_tmp);
		*opt_ptr++ = 0;

		strcpy(opt_ptr, "AP_CFG_END");
		opt_ptr += strlen("AP_CFG_END");
		*opt_ptr++ = 0;
		
		wrq.u.data.length = opt_ptr - data_buf;
		wrq.u.data.pointer = data_buf;
			
		ret = wifi_dev->netdev_ops->ndo_do_ioctl(wifi_dev, (void*)&wrq, WOAL_UAP_FROYO_AP_SET_CFG);
		if(ret == 0)
			ret = wifi_dev->netdev_ops->ndo_do_ioctl(wifi_dev, 0, WOAL_UAP_FROYO_AP_BSS_START);

		
	}else
		p_err("%s not support", __func__ );
	
	return ret;
}
 

#ifdef MAX_TX_PENDING_CTRL
/**
*wmj+ 
*wifi_get_tx_pending get current tx_pending in driver.
*@return   tx_pending --success, -1 --fail
*/
int wifi_get_tx_pending()
{
	struct ifreq ifreq;
	int ret;
	u8 tx_pending;
		
	ifreq.ifr_ifru.ifru_data = &tx_pending;
	ret = wifi_dev->netdev_ops->ndo_do_ioctl(wifi_dev, &ifreq, UAP_GET_TX_PENDING);
	
	if(ret < 0)
	{
		p_err("get tx_pending error\n");
		return -1;
	}
	else
	{
		return tx_pending;
	}

}
/**
*wmj+ 
*wifi_set_max_tx_pending  set max tx_pending allowed in driver.
*@param tx_pending,  input value 
*@return   0 --success, others --fail

*/

int wifi_set_max_tx_pending(u8 max_tx_pending)
{
	struct ifreq ifreq;
	int ret = -1;
	u8 max_pending = max_tx_pending;

	ifreq.ifr_ifru.ifru_data = &max_pending;	
	ret = wifi_dev->netdev_ops->ndo_do_ioctl(wifi_dev, &ifreq, UAP_SET_MAX_TX_PENDING);
	if(ret < 0)
	{
		p_err("set max tx_pending error\n");
		return -1;
	}
	else
	{
		return 0;
	}
}

/**
*wmj+ 
*wifi_get_max_tx_pending  get max tx_pending allowed in driver.
*@return   max_tx_pending --success, -1 --fail

*/

int wifi_get_max_tx_pending()
{
	struct ifreq ifreq;
	int ret = -1;
	u8 max_tx_pending;

	ifreq.ifr_ifru.ifru_data = &max_tx_pending;	
	ret = wifi_dev->netdev_ops->ndo_do_ioctl(wifi_dev, &ifreq, UAP_GET_MAX_TX_PENDING);
	if(ret < 0)
	{
		p_err("get max tx_pending error\n");
		return -1;
	}
	else
	{
		return max_tx_pending;
	}
}

#endif
//wmj<<


#ifdef UAP_SUPPORT
int wifi_get_sta_list(mlan_ds_sta_list *p_list)
{
	int ret = -1;
	
	if(wifi_dev->netdev_ops && wifi_dev->netdev_ops->ndo_do_ioctl)
	{
		struct iwreq *wrq = (struct iwreq *)mem_calloc(sizeof(struct iwreq), 1);
		if(wrq)
		{
			wrq->u.data.pointer = p_list;
		
			ret = wifi_dev->netdev_ops->ndo_do_ioctl(wifi_dev, (void*)wrq, UAP_GET_STA_LIST);

			mem_free(wrq);
		}
		
	}else
		p_err("%s not support", __func__ );
	
	return ret;
	
}
#endif



/**
 * @brief  读取mac地址，mac地址从8782芯片读取
 * @param  mac_addr : mac地址 6BYTE
 */
int wifi_get_mac(unsigned char *mac_addr)
{
	
	memcpy(mac_addr, wifi_dev->dev_addr, ETH_ALEN);
	return 0;
}


/**
 * @brief  获取WIFI连接的统计信息(信号强度...)
 * @param  pStats : 指向station_info的指针
 * @param  mac : 指向需要获取的station的地址
 */
int wifi_get_stats(uint8_t *mac,struct station_info *sinfo)
{
	int ret = -1;
	moal_private *priv;
	
	priv = (moal_private *)wifi_dev->ml_priv;
	if(wifi_dev->cfg80211_ops && wifi_dev->cfg80211_ops->get_station)
	{
		ret = wifi_dev->cfg80211_ops->get_station(priv->wdev->wiphy, wifi_dev, mac,sinfo);

	}else
		p_err("%s not support", __func__ );
	
	return ret;
}



/**
 * @brief  这里不再提供WIFI事件的回调
 *
 * ap模式下请参考wlan_ops_uap_process_event函数
 * sta模式下请参考wlan_ops_sta_process_event函数
 */

int event_callback(int type)
{

	return 0;
}

//extern uint8_t g_mac_addr[6]; //wmj-


//extern struct netif if_wifi;//wmj-

/**
 * @brief register net device fuction
 * @author
 * @date 
 * @param [in]  dev  a pointer to struct net_device
 * @return int
 * @retval  0   register suces
 */
int register_netdev(struct net_device *dev)
{
	moal_private *priv;

	priv = (moal_private *)dev->ml_priv;
	if(0)
		;
#ifdef STA_SUPPORT	
	else if(priv->bss_type == MLAN_BSS_TYPE_STA)
		sta_wifi_dev = dev;
#endif
#ifdef UAP_SUPPORT	
	else if(priv->bss_type == MLAN_BSS_TYPE_UAP)
		ap_wifi_dev = dev;
#endif
#ifdef WIFI_DIRECT_SUPPORT	
	else if(priv->bss_type == MLAN_BSS_TYPE_WIFIDIRECT)
		direct_wifi_dev = dev;
#endif
	else
		p_err("unkown bss_role");
	
	wifi_dev = dev;
	return 0;
}



extern struct sdio_func func;
extern struct sdio_cccr cccr;

/**
 * @brief read card common control register(CCCR)
 * @author
 * @date 
 * @param none
 * @return int
 * @retval  0   read  ok
 * @retval  value>0 and value<0   read  err
 */
int sdio_read_cccr()
{
	int ret;
	int cccr_vsn;
	unsigned char data;

	memset(&cccr, 0, sizeof(struct sdio_cccr));

	ret = mmc_io_rw_direct(0, 0, SDIO_CCCR_CCCR, 0, &data);
	if (ret != 0)
		goto out;

	cccr_vsn = data & 0x0f;
	p_dbg("SDIO_CCCR_CCCR:%x\n", cccr_vsn);

	if (cccr_vsn > SDIO_CCCR_REV_1_20)
	{
		p_err("unrecognised CCCR structure version %d\n", cccr_vsn);
		return -1;
	}

	cccr.sdio_vsn = (data & 0xf0) >> 4;

	ret = mmc_io_rw_direct(0, 0, SDIO_CCCR_CAPS, 0, &data);
	if (ret != 0)
		goto out;

	p_dbg("SDIO_CCCR_CAPS:%x\n", data);
	if (data & SDIO_CCCR_CAP_SMB)
		cccr.multi_block = 1;
	if (data & SDIO_CCCR_CAP_LSC)
		cccr.low_speed = 1;
	if (data & SDIO_CCCR_CAP_4BLS)
		cccr.wide_bus = 1;

	if (cccr_vsn >= SDIO_CCCR_REV_1_10)
	{
		ret = mmc_io_rw_direct(0, 0, SDIO_CCCR_POWER, 0, &data);

		p_dbg("SDIO_CCCR_POWER:%x\n", data);
		if (ret != 0)
			goto out;

		if (data & SDIO_POWER_SMPC)
			cccr.high_power = 1;
	}

	if (cccr_vsn >= SDIO_CCCR_REV_1_20)
	{
		ret = mmc_io_rw_direct(0, 0, SDIO_CCCR_SPEED, 0, &data);

		p_dbg("SDIO_CCCR_SPEED:%x\n", data);
		if (ret != 0)
			goto out;

		if (data & SDIO_SPEED_SHS)
			cccr.high_speed = 1;
	}

	ret = mmc_io_rw_direct(0, 0, 7, 0, &data);
	if (ret != 0)
		goto out;

	data |= 0X20;
	ret = mmc_io_rw_direct(1, 0, 7, data, &data);
	if (ret != 0)
		goto out;
	
out:
	if (ret != 0)
		p_err("sdio_read_cccr err\n");
	return ret;
}



struct cis_tpl
{
	unsigned char code;
	unsigned char min_size;
	//	tpl_parse_t *parse;
};

struct sdio_func_tuple
{
	unsigned char code;
	unsigned char size;
	unsigned char data[128];
};

//struct sdio_func_tuple func_tuple[10];

/**
 * @brief read sdio fuction CIS's area
 * @author
 * @date 
 * @param [in]  f_n sdio fuction number
 * @return int
 * @retval  0   read  ok
 * @retval  value>0 and value<0   read  err
 */
static int sdio_read_cis(int f_n)
{
	int ret;
	struct sdio_func_tuple *this;
	unsigned i, ptr = 0, tuple_cnt = 0;
	unsigned char tpl_code, tpl_link;
	/*
	 * Note that this works for the common CIS (function number 0) as
	 * well as a function's CIS * since SDIO_CCCR_CIS and SDIO_FBR_CIS
	 * have the same offset.
	 */
	for (i = 0; i < 3; i++)
	{
		unsigned char x;

		ret = mmc_io_rw_direct(0, f_n,
				SDIO_FBR_BASE(f_n) + SDIO_FBR_CIS + i, 0, &x);
		if (ret)
			return ret;
		ptr |= x << (i * 8);
	}

	p_dbg("read_cis,fn:%d,addr:%d\n", f_n, ptr);
	
	do
	{
		ret = mmc_io_rw_direct(0, f_n, ptr++, 0, &tpl_code);
		if (ret)
			break;

		/* 0xff means we're done */
		if (tpl_code == 0xff)
			break;

		/* null entries have no link field or data */
		if (tpl_code == 0x00)
			continue;

		ret = mmc_io_rw_direct(0, f_n, ptr++, 0, &tpl_link);
		if (ret)
			break;

		/* a size of 0xff also means we're done */
		if (tpl_link == 0xff)
			break;

		p_info("tpl code:%x,size:%d\n", tpl_code, tpl_link);


		if (tuple_cnt > 9 || tpl_link > 128)
		{
			p_dbg("tuple_cnt over\n");
			break;
		}
//		func_tuple[tuple_cnt].size = tpl_link;
		
		this = (struct sdio_func_tuple *)mem_malloc(sizeof(*this) + tpl_link);
		if (!this)
			return -ENOMEM;

		for (i = 0; i < tpl_link; i++)
		{
			ret = mmc_io_rw_direct(0, f_n,
					ptr + i, 0, &this->data[i]);
			if (ret)
				break;
		}

		dump_hex("cis", this->data, i);

		if (ret)
		{
			mem_free(this);
			break;
		}
		if(tpl_code == CISTPL_VERS_1)
			p_info("%s\n", this->data + 2);
		
		mem_free(this);

		ptr += tpl_link;
		tuple_cnt += 1;
	}
	while (!ret);

	if (tpl_link == 0xff)
		ret = 0;

	return ret;
}

/**
 * @brief read common sdio CIS's area
 * @author
 * @date 
 * @param none
 * @return int
 * @retval  0   read  ok
 * @retval  value>0 and value<0   read  err
 */
int sdio_read_common_cis()
{
	return sdio_read_cis(0);
}

/**
 * @brief switching sdio to high speed mode
 * @author
 * @date 
 * @param [in]  enable enabling value :if enable !=0  switching high speed mode else do not switch high speed mode
 * @return int
 * @retval  0   mmc_sdio_switch_hs ok
 * @retval  -1 mmc_sdio_switch_hs err
 */
int mmc_sdio_switch_hs(int enable)
{
	int ret;
	u8 speed;

	ret = mmc_io_rw_direct(0, 0, SDIO_CCCR_SPEED, 0, &speed);
	if (ret != 0)
	{
		p_err("mmc_sdio_switch_hs err:%d\n", ret);
		return ret;
	}
	if (enable)
		speed |= SDIO_SPEED_EHS;
	else
		speed &= ~SDIO_SPEED_EHS;

	ret = mmc_io_rw_direct(1, 0, SDIO_CCCR_SPEED, speed, NULL);
	if (ret != 0)
		p_err("mmc_sdio_switch_hs err1:%d\n", ret);
	else
		p_dbg("mmc_sdio_switch_hs ok\n");

	return ret;
}

//extern struct lbs_private *if_sdio_probe(void);
extern int woal_init_module(struct sdio_func *func);
extern char *mac_addr;

/**
 * @brief powering down  wifi8801 module  
 * @author
 * @date 
 * @param none
 * @return void
 * @retval  
 */
void Marvell_8801_PowerDown()
{
	gpio_set_pin_as_gpio(WIFI_POWERDOWN_PIN);
	gpio_set_pull_down_r(WIFI_POWERDOWN_PIN, 0); //disable pull-down
    gpio_set_pin_level(WIFI_POWERDOWN_PIN, GPIO_LEVEL_LOW);
	gpio_set_pin_dir(WIFI_POWERDOWN_PIN, GPIO_DIR_OUTPUT);
}

/**
 * @brief powering on  wifi8801 module  
 * @author
 * @date 
 * @param none
 * @return void
 * @retval    
 */
void Marvell_8801_PowerOn()
{
	gpio_set_pin_as_gpio(WIFI_POWERDOWN_PIN);
	gpio_set_pull_down_r(WIFI_POWERDOWN_PIN, 0); //disable pull-down
	gpio_set_pin_level(WIFI_POWERDOWN_PIN, GPIO_LEVEL_HIGH);
	gpio_set_pin_dir(WIFI_POWERDOWN_PIN, GPIO_DIR_OUTPUT);	
}

/* Forward declarations. */
//void  ethernetif_input(struct netif *netif, void *p_buf,int size);

/**
 * In this function, the hardware should be initialized.
 * Called from ethernetif_init().
 *
 * @param netif the already initialized lwip network interface structure
 *        for this ethernetif
 */

//extern OS_EVENT  *pool_alloc_sem;

struct ethernetif {
  struct eth_addr *ethaddr;
  /* Add whatever per-interface state that is needed here. */
};

/* Define those to better describe your network interface. */
#define IFNAME0 'w'
#define IFNAME1 'i'


static void
low_level_init(struct netif *netif)
{
     
  /* set MAC hardware address length */
  netif->hwaddr_len = 6;

  /* set MAC hardware address */
  wifi_get_mac(netif->hwaddr); //wmj+
    
  /* maximum transfer unit */
  netif->mtu = 1500;
  
  /* device capabilities */
  /* don't set NETIF_FLAG_ETHARP if this device is not an ethernet one */
  netif->flags = NETIF_FLAG_BROADCAST | 
  				NETIF_FLAG_ETHARP | 
  				NETIF_FLAG_LINK_UP | 
  				NETIF_FLAG_IGMP;

  /* Do whatever else is needed to initialize interface. */  
}

#pragma arm section code ="_video_server_"

/**
 * This function should do the actual transmission of the packet. The packet is
 * contained in the pbuf that is passed to the function. This pbuf
 * might be chained.
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @param p the MAC packet to send (e.g. IP packet including MAC addresses and type)
 * @return ERR_OK if the packet could be sent
 *         an err_t value if the packet couldn't be sent
 *
 * @note Returning ERR_MEM here if a DMA queue of your MAC is full can lead to
 *       strange results. You might consider waiting for space in the DMA queue
 *       to become availale since the stack doesn't retry to send a packet
 *       dropped because of memory failure (except for the TCP timers).
 */

static err_t
low_level_output(struct netif *netif, struct pbuf *p)
{

#if ETH_PAD_SIZE
  pbuf_header(p, -ETH_PAD_SIZE); /* drop the padding word */
#endif

  //the  the actual xmit interface, implement in wifi lib
  netif_tx(p);

#if ETH_PAD_SIZE
  pbuf_header(p, ETH_PAD_SIZE); /* reclaim the padding word */
#endif
  
  //LINK_STATS_INC(link.xmit);

  return ERR_OK;
}

/**
 * Should allocate a pbuf and transfer the bytes of the incoming
 * packet from the interface into the pbuf.
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return a pbuf filled with the received packet (including MAC header)
 *         NULL on memory error
 */
static struct pbuf *
low_level_input(struct netif *netif,void *p_buf,int size)
{
//  struct ethernetif *ethernetif = netif->state;
  struct pbuf *p, *q;
  u16_t len;
  int rem_len;

  /* Obtain the size of the packet and put it into the "len"
     variable. */
  len = size;

#if ETH_PAD_SIZE
  len += ETH_PAD_SIZE; /* allow room for Ethernet padding */
#endif

  /* We allocate a pbuf chain of pbufs from the pool. */
  p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);
  
  if (p != NULL && (p->tot_len >= len)) {

#if ETH_PAD_SIZE
    pbuf_header(p, -ETH_PAD_SIZE); /* drop the padding word */
#endif

    /* We iterate over the pbuf chain until we have read the entire
     * packet into the pbuf. */
     rem_len = p->tot_len;
    for(q = p; q != NULL; q = q->next) {
      /* Read enough bytes to fill this pbuf in the chain. The
       * available data in the pbuf is given by the q->len
       * variable.
       * This does not necessarily have to be a memcpy, you can also preallocate
       * pbufs for a DMA-enabled MAC and after receiving truncate it to the
       * actually received size. In this case, ensure the tot_len member of the
       * pbuf is the sum of the chained pbuf len members.
       */
    if(rem_len > 0)
	{
		memcpy(q->payload, (char*)p_buf + (p->tot_len - rem_len), q->len);
		rem_len -= q->len;
	}
	else
		p_err("low_level_input memcpy err\n");
 //     read data into(q->payload, q->len);
    }
 //   acknowledge that packet has been read();

#if ETH_PAD_SIZE
    pbuf_header(p, ETH_PAD_SIZE); /* reclaim the padding word */
#endif

    //LINK_STATS_INC(link.recv);
  } else {

   // LINK_STATS_INC(link.memerr);
   // LINK_STATS_INC(link.drop);
  }

  return p;  
}


/**
 * This function should be called when a packet is ready to be read
 * from the interface. It uses the function low_level_input() that
 * should handle the actual reception of bytes from the network
 * interface. Then the type of the received packet is determined and
 * the appropriate input function is called.
 *
 * @param netif the lwip network interface structure for this ethernetif
 */
#define ETH_FRAME_LEN	1514

void wifi_input(struct netif *netif,void *p_buf,int size)
{
//  struct ethernetif *ethernetif;
 // struct ethhdr *hdr;
 struct eth_hdr *hdr; //wmj  not use type define in wifi lib
  struct pbuf *p;

  if(size > ETH_FRAME_LEN)
  {
  	p_err("ethernetif_input too long:%d\n",size);
	return;
  }
  p = low_level_input(netif, p_buf, size);
  if (p == NULL) {
	return;
  }

  /* points to packet payload, which starts with an Ethernet header */
  hdr = p->payload;

  //switch (htons(hdr->h_proto)) {
  switch (htons(hdr->type)) {  //wmj  not use type define in wifi lib
  /* IP or ARP packet? */
  case ETHTYPE_IP:
  case ETHTYPE_ARP:
 // case ETH_P_EAPOL:  //wmj- no need to support 802.1x
#if PPPOE_SUPPORT
  /* PPPoE packet? */
  case ETHTYPE_PPPOEDISC:
  case ETHTYPE_PPPOE:
#endif /* PPPOE_SUPPORT */
    /* full packet send to tcpip_thread to process */
    if (netif->input(p, netif)!=ERR_OK)
     { LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_input: IP input error\n"));
       pbuf_free(p);
       p = NULL;
     }
    break;

  default:
    pbuf_free(p);
    p = NULL;
    break;
  }
}



/**
 * Should be called at the beginning of the program to set up the
 * network interface. It calls the function low_level_init() to do the
 * actual setup of the hardware.
 *
 * This function should be passed as a parameter to netif_add().
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return ERR_OK if the loopif is initialized
 *         ERR_MEM if private data couldn't be allocated
 *         any other err_t on error
 */
err_t
wifi_if_init(struct netif *netif)
{
  struct ethernetif *ethernetif;

  LWIP_ASSERT("netif != NULL", (netif != NULL));
    
  ethernetif = malloc(sizeof(struct ethernetif));
  if (ethernetif == NULL) {
    LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_init: out of memory\n"));
    return ERR_MEM;
  }

#if LWIP_NETIF_HOSTNAME
  /* Initialize interface hostname */
  netif->hostname = "lwip";
#endif /* LWIP_NETIF_HOSTNAME */

  /*
   * Initialize the snmp variables and counters inside the struct netif.
   * The last argument should be replaced with your link speed, in units
   * of bits per second.
   */
  NETIF_INIT_SNMP(netif, snmp_ifType_ethernet_csmacd, LINK_SPEED_OF_YOUR_NETIF_IN_BPS);

  netif->state = ethernetif;
  netif->name[0] = IFNAME0;
  netif->name[1] = IFNAME1;
  /* We directly use etharp_output() here to save a function call.
   * You can instead declare your own function an call etharp_output()
   * from it if you have to do some checks before sending (e.g. if link
   * is available...) */
  netif->output = etharp_output;
  netif->linkoutput = low_level_output;
  
  ethernetif->ethaddr = (struct eth_addr *)&(netif->hwaddr[0]);
  
  /* initialize the hardware */
  low_level_init(netif);

  return ERR_OK;
}

__be16 eth_type_trans(struct sk_buff *skb, struct net_device *dev)
{
	//struct ethhdr *eth;

	//skb->dev = dev;
	skb_reset_mac_header(skb);
	skb_pull_inline(skb, ETH_HLEN);
	return 0;
}

int netif_tx(void *data)
{

	struct pbuf *skb = (struct pbuf *)data;
	
#ifdef DEBUG
	static int first_pkg = 1;
#endif
	struct sk_buff *tx_skb = 0;
	struct pbuf *q;
	char *p802x_hdr;
	
	if(!is_hw_ok())
		return;

	tx_skb = alloc_skb(skb->tot_len + MLAN_MIN_DATA_HEADER_LEN + sizeof(mlan_buffer), 0);
	if(tx_skb == 0)
		return;

	skb_reserve(tx_skb, MLAN_MIN_DATA_HEADER_LEN + sizeof(mlan_buffer));
	
	p802x_hdr = (char*)tx_skb->data;
	for(q = skb; q != NULL; q = q->next) {
		
		memcpy(p802x_hdr, q->payload, q->len);
		p802x_hdr += q->len;
		
	}
	tx_skb->len = skb->tot_len;
	p802x_hdr = (char*)tx_skb->data;

	if(wifi_dev && wifi_dev->netdev_ops)
		wifi_dev->netdev_ops->ndo_start_xmit((void*)tx_skb, wifi_dev);
	
	return 0;
}

//called by wifi lib to recv packet to upper layer
int netif_rx(void *data)
{

	struct sk_buff *skb = (struct sk_buff*)data;
	
	if(p_netif)
	{
		//dump_hex("", skb_mac_header(skb), 32);
		wifi_input(p_netif, skb_mac_header(skb), skb->len + sizeof(struct ethhdr));
	}
	kfree_skb(skb);
	return 0;
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
	int ret;
	moal_private *priv;

	p_info("\nWifilib version:%s", Wifi_GetVersion());

	func.vendor = MARVELL_VENDOR_ID;

	func.device = SD_DEVICE_ID_8801;
	/*  not use firemware outside wmj
    if (pParam != NULL && pParam->cache != NULL && pParam->size != 0)
    {
		wifi_initparam.cache = pParam->cache;
		wifi_initparam.size = pParam->size;
	}
	else
	{
		wifi_initparam.cache = NULL;
		wifi_initparam.size = 0;
	}
	*/
	Marvell_8801_PowerDown();
	sleep(50);
	Marvell_8801_PowerOn();


	ret = sdio_initial(INTERFACE_SDIO,USE_ONE_BUS, 256); //USE_ONE_BUS   USE_FOUR_BUS  3 SDIO
	
	if(ret == AK_FALSE){
		p_err("sdio initial faild");
		return -1;
	}

	ret = sdio_read_cccr();
	if (ret != 0)
	{
		p_err("sdio_read_cccr faild");
		return -1;
	}

	sdio_read_common_cis();

	mmc_sdio_switch_hs(TRUE);
	
	//mac_addr = "00:4c:35:12:34:56"; 如果你想用自己定义的mac地址

	ret = woal_init_module(&func);
	if (ret != 0)
	{
		p_err("woal_init_module faild");
		return -1;
	}
	
#if 0 //woal_init_module has do woal_init_priv
	if(sta_wifi_dev){
		priv = (moal_private *)sta_wifi_dev->ml_priv;
		priv->wdev = sta_wifi_dev->ieee80211_ptr;
		woal_init_priv(priv, MOAL_CMD_WAIT);
	}
	
	if(ap_wifi_dev){
		priv = (moal_private *)ap_wifi_dev->ml_priv;
		priv->wdev = ap_wifi_dev->ieee80211_ptr;
		woal_init_priv(priv, MOAL_CMD_WAIT);
	}

	wifi_power_cfg(0);
#endif
	
	p_dbg("wifi init ok");
	return ret;
}



