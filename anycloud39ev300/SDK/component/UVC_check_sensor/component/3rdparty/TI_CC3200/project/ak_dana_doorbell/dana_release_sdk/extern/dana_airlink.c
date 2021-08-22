#include <stdio.h>
#include <stdlib.h>


#include <string.h>
#include "dana_base.h"
#include "dana_airlink.h"
#include "dana_mem.h"

#include "simplelink.h"
#include "uart_if.h"
#include "osi.h"
#include "dana_debug.h"
#include "dana_task.h"

struct dana_airlink_handler {
    int32_t fd;
    char if_name[32];
    int32_t work_channel;
};

static int CC_Iwconfig_Set_Mode(char * _if_name, uint8_t _mode)
{
       
	if(sl_WlanSetMode(_mode) != 0)
				return (-1);

	if(sl_Stop(0)<0)
				return (-1);

	if(sl_Start(0,0,0)<0)
				return (-1);
	return 0;
}

static int CC_Iwconfig_Scan_Chanlist(char * _if_name, dana_airlink_channel_info_t *list)
{
	unsigned char ucpolicyOpt=0,val=0;
	int  lRetVal = 0;
	Sl_WlanNetworkEntry_t   netEntries[10];   
    //关闭所有连接策略 
    ucpolicyOpt = SL_CONNECTION_POLICY(0, 0, 0, 0,0);
    lRetVal = sl_WlanPolicySet(SL_POLICY_CONNECTION , ucpolicyOpt, NULL, 0);
    if(lRetVal != 0)
    {
        Report("Unable to clear the Connection Policy\r\n");
        return lRetVal;
    }
    
    //switch to station mode
    //切换到station模式
    lRetVal = sl_WlanSetMode(ROLE_STA);
    
     
    lRetVal = sl_Stop(0);
    if(lRetVal<0)
    {
      Report("sl_stop error\r\n");
      return  -1;
    }
    
    lRetVal = sl_Start(0,0,0);
    if(lRetVal<0)
    {
      Report("sl_start error\r\n");
      return  -1;
    }

    ucpolicyOpt = SL_SCAN_POLICY(1);
    val = 2;                                                //扫描时间2S
    
     // set scan policy - this starts the scan 
    lRetVal = sl_WlanPolicySet(SL_POLICY_SCAN , ucpolicyOpt, &val, sizeof(val));
    if(lRetVal!=0)
    {
        Report("Unable to set the Scan Policy\n\r");
        return -1;
    }   
    osi_Sleep(2000);   		//休眠2s
    lRetVal = sl_WlanGetNetworkList(0,10,&netEntries[0]);
    if(lRetVal==0)
    {
        Report("Unable to retreive the network list\n\r");
        return lRetVal;
    }  
    
    Report("sl_WlanGetNetworkList number : %d \r\n",lRetVal);
    for(val=0; val< lRetVal ; val++)
    {
        Report("%s;     ktype:%d\r\n",netEntries[val].ssid,netEntries[val].sec_type);
        strncpy(list->ssids[val].ssid,netEntries[val].ssid,netEntries[val].ssid_len);
        strncpy(list->ssids[val].bssid,netEntries[val].bssid,6);
    }     
        list->ssids_count = val;	
        list->channel[0] = 3;
        list->channel[1] = 6;
        list->channel[2] = 9;
        list->size = 3;
       
        return lRetVal;
}


dana_airlink_handler_t *dana_airlink_create(const char *if_name)
{
    if (NULL == if_name) {
        return NULL; 
    }

    dana_airlink_handler_t *dana_airlink = (dana_airlink_handler_t *)dana_calloc(1, sizeof(dana_airlink_handler_t));
    if (NULL == dana_airlink) {
        return NULL; 
    }

    strncpy(dana_airlink->if_name, if_name, sizeof(dana_airlink->if_name) -1);
    dana_airlink->work_channel = 6;
    
    return dana_airlink;
}

int32_t dana_airlink_scanninglist(dana_airlink_handler_t *dana_airlink, dana_airlink_channel_info_t *list)
{
    if (CC_Iwconfig_Scan_Chanlist(dana_airlink->if_name,list) < 0) {
        return -1;
    }

    return 0;
}

int32_t dana_airlink_setmode(dana_airlink_handler_t *dana_airlink, dana_airlink_mode_t mode)
{
    if (mode == DANA_AIRLINK_MODE_MONITOR) {
        if (CC_Iwconfig_Set_Mode(dana_airlink->if_name, ROLE_STA) < 0) {
            return -1;
        }
       
    } else if (mode == DANA_AIRLINK_MODE_STATION) {
        if (CC_Iwconfig_Set_Mode(dana_airlink->if_name, ROLE_STA) < 0) {
            return -1;
        }
    } else {
        return -1; 
    }

    return 0;
}

int32_t dana_airlink_createrawsock(dana_airlink_handler_t *dana_airlink)
{  
    int      ret=0;
 
    dana_airlink->fd = sl_Socket(SL_AF_RF, SL_SOCK_RAW, dana_airlink->work_channel);
    if (0 > dana_airlink->fd) {
        return -1; 
    }
#if 1
    struct SlTimeval_t timeval;
    dana_memset(&timeval, 0, sizeof(timeval));
    timeval.tv_sec =  0;             // Seconds
    timeval.tv_usec = 2000;         // Microseconds.
    ret = sl_SetSockOpt(dana_airlink->fd ,SL_SOL_SOCKET,SL_SO_RCVTIMEO, &timeval, sizeof(timeval));
    if(ret < 0){
      dbg("sl_SetSockOpt failed\r\n");
      return -1;
    } else {
      dbg("sl_SetSockOpt set success\r\n");
    }
#endif
    
    return 0;
}

int32_t dana_airlink_recv(dana_airlink_handler_t *dana_airlink, char *recv_buf, int32_t size)
{
  int32_t ret = sl_Recv(dana_airlink->fd, recv_buf, size, 0);
  if (-8 == ret || -9 == ret)
      ret = 0;
  return ret;
}

int32_t dana_airlink_closerawsock(dana_airlink_handler_t *dana_airlink)
{ 
    sl_Close(dana_airlink->fd);
    return 0;
}

bool dana_airlink_getchannel(dana_airlink_handler_t *dana_airlink, int32_t *channel)
{
    *channel = dana_airlink->work_channel;
    return true;
}


bool dana_airlink_setchannel(dana_airlink_handler_t *dana_airlink, int32_t channel)
{
    if (channel<0 || channel>13) {
         return -1;
    }
#if 1 
    sl_Close(dana_airlink->fd);
    int fd = -1;
    while (fd < 0) {
        fd = sl_Socket(SL_AF_RF, SL_SOCK_RAW, channel);
        if (fd < 0) {
            dbg("sl_Socket failed：%d\r\n", fd);
            sl_Close(dana_airlink->fd);
            osi_Sleep(20);
        } else {
            break;  
        }
    }
    
    struct SlTimeval_t timeval;
    dana_memset(&timeval, 0, sizeof(timeval));
    timeval.tv_sec =  0;             // Seconds
    timeval.tv_usec = 20000;         // Microseconds.
    if (sl_SetSockOpt(fd ,SL_SOL_SOCKET,SL_SO_RCVTIMEO, &timeval, sizeof(timeval)) < 0) {
        dbg("setchannel sl_setsockopt failed\n");
        return -1;
    }
    dana_airlink->fd = fd;
#else
    int ch = channel;
    if (sl_SetSockOpt(dana_airlink->fd, SL_SOL_SOCKET, SL_SO_CHANGE_CHANNEL, &ch, sizeof(ch))< 0) {
        dbg("setchannel sl_setsockopt failed\n");
        return -1;
    }
#endif
    
    dana_airlink->work_channel = channel;
    return true;  
     //sl_SetSockOpt(fd, SL_SOL_SOCKET, SL_SO_CHANGE_CHANNEL, &channel, sizeof(channel));
}

int32_t dana_airlink_destory(dana_airlink_handler_t *dana_airlink)
{
    //关闭socket
    dana_free(dana_airlink);
    return 0;
}


static char *ether_ntoa_r(uint8_t *hwaddr, char *buf, size_t size)
{
    if (NULL == hwaddr || NULL == buf) {
        return NULL;
    }
    snprintf(buf, size, "%02x:%02x:%02x:%02x:%02x:%02x", hwaddr[0],hwaddr[1],hwaddr[2],hwaddr[3],hwaddr[4],hwaddr[5]);
    return buf;
}

bool  dana_airlink_getframefomat(char *buffer, int32_t size, frame_format_t *frame_format)
{
    int32_t pos = 0;

#if 0
    uint8_t radiotap_version = buffer[pos++];
    uint8_t radiotap_pad = buffer[pos++];
    uint16_t radiotap_length = buffer[pos];

    if (radiotap_version != 0 || radiotap_pad != 0 || radiotap_length > 1000) {
        return false;
    }

    // Skip Radiotap
    pos += radiotap_length - 4 + 2;
#endif

    pos +=  8;

    memcpy(&(frame_format->frame_control), buffer + pos, sizeof(frame_format->frame_control));

#if 0
    dbg("version: %d\n", frame_format->frame_control.version);
    dbg("type: %d\n", frame_format->frame_control.type);
    dbg("subtype: %d\n", frame_format->frame_control.subtype);
    dbg("toDS: %d\n", frame_format->frame_control.toDS);
    dbg("fromDS: %d\n", frame_format->frame_control.fromDS);
    dbg("morefrag: %d\n", frame_format->frame_control.morefrag);
    dbg("retry: %d\n", frame_format->frame_control.retry);
    dbg("pwr: %d\n", frame_format->frame_control.pwr);
    dbg("moredata: %d\n", frame_format->frame_control.moredata);
    dbg("wep: %d\n", frame_format->frame_control.wep);
    dbg("rsvd: %d\n", frame_format->frame_control.rsvd);
    dbg("\n");
#endif


    pos += 2;

    memcpy(&(frame_format->duration_id), buffer + pos, sizeof(frame_format->duration_id));

    pos += 2;

    memcpy(frame_format->destination_mac_address.ether_addr_octet, buffer + pos, 6);
    pos += 6;
    memcpy(frame_format->bssid_mac_address.ether_addr_octet, buffer + pos, 6);
    pos += 6;
    memcpy(frame_format->source_mac_address.ether_addr_octet, buffer + pos, 6);

#if 0
    char addr1_str[80];
    char addr2_str[80];
    char addr3_str[80];
    ether_ntoa_r(frame_format->destination_mac_address.ether_addr_octet, addr1_str, sizeof(addr1_str));
    ether_ntoa_r(frame_format->bssid_mac_address.ether_addr_octet, addr2_str, sizeof(addr2_str));
    ether_ntoa_r(frame_format->source_mac_address.ether_addr_octet, addr3_str, sizeof(addr3_str));
    dbg("addr1: %s\n", addr1_str);
    dbg("addr2: %s\n", addr2_str);
    dbg("addr3: %s\n\n", addr3_str);
#endif

    return true;
}



