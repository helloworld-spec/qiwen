#ifndef _DANA_AIRLINK_H_
#define _DANA_AIRLINK_H_

#include "dana_base.h"
#include "dana_sock.h"

#ifdef __cplusplus
extern "C"
{
#endif

//大拿airlink所需相关接口函数， 厂商根据平台特性自行实现
//


typedef enum dana_airlink_mode {
    DANA_AIRLINK_MODE_MONITOR,
    DANA_AIRLINK_MODE_STATION,
}dana_airlink_mode_t;

typedef struct dana_airlink_ssid_info_s {
    char ssid[33];
    unsigned char bssid[6]; 
} dana_airlink_ssid_info_t;

typedef struct dana_airlink_channel_info_s{
#ifdef FREE_OS
    int channel[32];
#else
    int channel[256];	
#endif 
    int size;
#ifdef FREE_OS
    dana_airlink_ssid_info_t ssids[32];
#else
    dana_airlink_ssid_info_t ssids[256];
#endif 
    int ssids_count; 
}dana_airlink_channel_info_t;

typedef struct frame_control_s {
    uint8_t version:2;
    uint8_t type:2;
    uint8_t subtype:4;
    uint8_t toDS:1;
    uint8_t fromDS:1;
    uint8_t morefrag:1;
    uint8_t retry:1;
    uint8_t pwr:1;
    uint8_t moredata:1;
    uint8_t wep:1;
    uint8_t rsvd:1;
} frame_control_t;

typedef struct frame_format_s {
    frame_control_t frame_control;
    uint16_t duration_id;
    struct ether_addr destination_mac_address;
    struct ether_addr bssid_mac_address;
    struct ether_addr source_mac_address;
    //后面暂时不需要了
    //sequence control 
    //addr 4 
    //qos 
    //body
    //fcs
}frame_format_t;

typedef struct dana_airlink_handler dana_airlink_handler_t;

dana_airlink_handler_t *dana_airlink_create(const char *if_name);

int32_t dana_airlink_scanninglist(dana_airlink_handler_t *dana_airlink, dana_airlink_channel_info_t *list);

int32_t dana_airlink_setmode(dana_airlink_handler_t *dana_airlink, dana_airlink_mode_t mode);

int32_t dana_airlink_createrawsock(dana_airlink_handler_t *dana_airlink);

int32_t dana_airlink_recv(dana_airlink_handler_t *dana_airlink, char *recv_buf, int32_t size);

int32_t dana_airlink_closerawsock(dana_airlink_handler_t *dana_airlink);

bool dana_airlink_getchannel(dana_airlink_handler_t *dana_airlink, int32_t *channel);  

bool dana_airlink_setchannel(dana_airlink_handler_t *dana_airlink, int32_t channel);  

int32_t dana_airlink_destory(dana_airlink_handler_t *dana_airlink);

bool  dana_airlink_getframefomat(char *buffer, int32_t size, frame_format_t *frame_format);


//解析frame_control



//跳转channel


//解析header


//获取数据





#ifdef __cplusplus
extern "C"
{
#endif
#endif
