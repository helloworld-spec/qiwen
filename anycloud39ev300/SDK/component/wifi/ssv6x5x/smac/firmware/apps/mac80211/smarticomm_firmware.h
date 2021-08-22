
#ifndef _SMART_CONFIG_H_
#define _SMART_CONFIG_H_

#include <soc_config.h>
#include <hdr80211.h>
#include <ssv_pktdef.h>
#include <driver/phy/drv_phy.h>
#include <rtos.h>
#include <msgevt.h>
#include <cmd_def.h>
#include <log.h>
#include <pbuf.h>

#ifdef CONFIG_SSV_SMARTLINK
#define DEFAULT_SCAN_TIME_MS 200
#define DEFAULT_CHANNEL 1
#define SEQ_BASE_CMD 256 //0 ~ 255 SSID&PASS_DATA_CMD, 256 ~ 512 SEQ_CMD
#define SSID_DATA 1 // 1BYTE
#define PASS_DATA 1 // 1BYTE
#define CRC8_DATA 1 // 1BYTE
#define IP_DATA 4 // 4BYTE
#define MAC_DATA 6 // 6BYTE
#define CMD_DATA_MAX_LEN 256
#define MAX_BASE_LEN_BUF 5

#define TIM_NOTE2_80211_ADDR {0x38, 0xAA, 0x3C, 0xE1, 0xDC, 0xC9}

typedef struct smart_control {
    u8 stopScan;
    u16 backData[2];//0xffff data is clear data
    u8 crcData;
    u8 decrypValue;
    //ssid & pass len
    u8 ssidLen[2];
    u8 passLen[2];
    //seq
    u8 seqData[2];
    u8 NumSeq[2];
    u8 backSeq[2];
    //cmd
    u8 cmdNum[2]; // strat cmd number 1//Matt01, not the same
    u8 CmdNumLen[2]; //every cmd have diff len
    //get current packet len
    u8 baseLenBufCount[2];
    u8 checkBaseLenBuf[2][5];
    u8 turnBaseLenBufCount[2];
    u8 turnCheckBaseLenBuf[2][5];
    //rc4 & cmd  buf
    u8 sonkey[CMD_DATA_MAX_LEN];//rc4 buf
    u8 stable[CMD_DATA_MAX_LEN];//rc4 buf
    u8 packet[2][CMD_DATA_MAX_LEN];//cmd buf
    u8 phoneIP[2][IP_DATA];//
    u8 phoneMAC[2][MAC_DATA];//
    u8 buf[SEND_MAX_BUF];
    u8 stage1Timer;
    u8 stage2Timer;
} smart_control;

void getPhoneMac(u8 *src);
void initSmartLink();
void clearSmartLinkBuf();
void stopSmartLink();
void rx_process_smartConf(PKT_Info *pPktInfo, int unicast_ptk);
int startscanchannel(u32 timeperiod);
void stopscanchannel();


#endif //CONFIG_SSV_SMARTLINK

#endif //_SMART_CONFIG_H_

