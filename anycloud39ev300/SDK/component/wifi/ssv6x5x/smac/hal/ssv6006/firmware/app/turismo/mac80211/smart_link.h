
#ifndef _SMART_LINK_H_
#define _SMART_LINK_H_

#include <soc_config.h>
#include <ssv6006_common.h>

#ifdef CONFIG_SSV_SMARTLINK

enum t_SMARK_LINK_TYPE
{
    NOSMART	= 0,
    ICOMM,
    WECHAT,
    USER,
} SMARK_LINK_TYPE;

#define IP_LEN 4
#define MAC_LEN 6
#define PHONE_PORT 8209
#define DEVICE_PORT 8302
#define SEND_MAX_BUF 64

enum DATA_STATUS
{
    NOTYPE = -1,
    AP_MODE = 0,
    PHONE_MODE = 1,
    BOTH = 2,
} DATA_STATUS;

enum CMD_NUM
{
    START = 0,
    INFO,
    SSID_PASS_DATA, 
    AP_LIST_CHECK, 
    DONE, 
    CHECK = 255,
} CMD_NUM;

typedef struct _smartComm {
    u8 sendToPhoneEnable;
    u8 sendToPhoneDataEnable;
    u8 phoneIP[IP_LEN];
    u8 phoneMAC[MAC_LEN];
    u8 buf[SEND_MAX_BUF];
    u8 bufLen;
} smartComm;

typedef struct t_AP_DETAIL_INFO
{
	u8 		ssid[32];
	u8		ssid_len;
	u8		key[64];
	u8		key_len;
	u8 		mac[6];
	char	pmk[32];
	u8		channel;
} AP_DETAIL_INFO;


typedef struct t_IEEE80211STATUS
{
	AP_DETAIL_INFO		connAP;
	bool                ptk_in_checking;
    u32                 ch_cfg_size;
	struct ssv6xxx_ch_cfg *ch_cfg;
}IEEE80211STATUS;


//void enableSmartHwFilter();
//void disableSmartHwFilter();

void setSmartCmdStatus(u8 cmd, u8 type);
u8 getSmartCmdStatus(u8 type);

//void disableSendToPhone();
//u8 getSendToPhoneStatus();

//void enableSendToPhoneData();
//void disableSendToPhoneData();

//void smartSetIfconfigAndConnect();
//u8 checkAndSetAPMac();

//void sendMessageToPhone();
//void setPhoneData(u8 *data, u8 len);
//void setPhoneMAC(u8 *mac, u8 len);
//void setSmartPhoneIP(u8 *ip, u8 len);

#endif //CONFIG_SSV_SMARTLINK

#endif //_SMART_LINK_H_


