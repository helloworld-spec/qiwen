#include <turismo_regs.h>
#include "smart_link.h"
#include "smarticomm_firmware.h"

#ifdef CONFIG_SSV_SMARTLINK

u8 magicNum[11] = "iot";//strt cmd will be check magic number
u8 rc4Key[4] = "Key";//rc4 key
u8 poly = 0x1D;//crc8 poly
int smart_icomm_state = 0;
static u8 timeInterval = 200;
static OsTimer channel_change_timer;

IEEE80211STATUS gwifistatus;

static smart_control recePhone;
static inline void swap(u8 *a, u8 *b);
static void KSA();
static void PRGA();
u8 getSmartCmdStatus(u8 type);
void setSmartCmdStatus(u8 cmd, u8 type);
static inline void checkBaseLen(u8 value, u8 type);
static inline void checkSeqSetPacket(u8 cmdValue, u8 type);
static inline u8 checkCrcWithSaveData(PKT_Info *pPktInfo, u8 cmdValue, u8 type);
static inline u8 decryption(u8 value, u8 num);
static inline u8 decryptionSeq(u8 value);

void channelSwitchInfo(u32 size, const u8 *cfg)
{
    int i;
    
    gwifistatus.ch_cfg = (struct ssv6xxx_ch_cfg *)PBUF_MAlloc(size*sizeof(struct ssv6xxx_ch_cfg), NOTYPE_BUF);
    gwifistatus.ch_cfg_size = size;
    memcpy(gwifistatus.ch_cfg, cfg, size*sizeof(struct ssv6xxx_ch_cfg));
    
    for (i = 0; i < size; i++)
        printf("%s, reg:0x%08x, ch1:0x%08x, ch13:0x%08x\n", __func__, gwifistatus.ch_cfg[i].reg_addr, gwifistatus.ch_cfg[i].ch1_12_value, gwifistatus.ch_cfg[i].ch13_14_value);
	
}

void channelSwitching(u32 ch)
{
    int i;
    //volatile u32 ch_addr, ch_value;
    
    for (i = 0; i < gwifistatus.ch_cfg_size; i++) {
        if (ch == 13) {
            REG32(gwifistatus.ch_cfg[i].reg_addr) = gwifistatus.ch_cfg[i].ch13_14_value;
        }
        else {
            REG32(gwifistatus.ch_cfg[i].reg_addr) = gwifistatus.ch_cfg[i].ch1_12_value;
        }
    }

}
void channelChangeHandler()
{
    static int channel = DEFAULT_CHANNEL;
    static int ipd_channel_touch = 0;
    while (gwifistatus.ptk_in_checking == true) {
        printf("%s, Oops, delay 10ms\n", __func__);
        OS_MsDelay(10);
    }
    if (recePhone.stopScan != TRUE) {		
        printf("%s: %d\n", __func__, channel);
        if (channel == 13) {
            if (ipd_channel_touch == 0) {
                channelSwitching(channel);
                ipd_channel_touch = 1;
            }
        }
        else {
            if (ipd_channel_touch == 1) {
                channelSwitching(channel);
                ipd_channel_touch = 0;
            }
        }
       // drv_phy_channel_change(channel); // ToDo Liam: not support yet
        gwifistatus.connAP.channel = channel;
        channel = (channel % 13) + 1;

        OS_TimerStart(channel_change_timer);
    }	
}

int initscanchannel()
{
    //Init 200ms timer 
    if( OS_TimerCreate(&channel_change_timer, timeInterval, (u8)FALSE, NULL, (OsTimerHandler)channelChangeHandler) == OS_FAILED)
        return OS_FAILED;

    OS_TimerStop(channel_change_timer);
    return OS_SUCCESS;
}

int startscanchannel(u32 timeperiod)
{
    return OS_TimerStart(channel_change_timer);
}

void stopscanchannel()
{
    recePhone.stopScan = TRUE;
    if (gwifistatus.ch_cfg) {
        PBUF_MFree(gwifistatus.ch_cfg);
        gwifistatus.ch_cfg = NULL;
    }
    //OS_TimerStop(channel_change_timer);
}

void setSmartCmdStatus(u8 cmd, u8 type)
{
    if (type == BOTH) {
        recePhone.cmdNum[AP_MODE] = recePhone.cmdNum[PHONE_MODE] = cmd;
    }
    else {
        recePhone.cmdNum[type] = cmd;
    }
    printf("cmd: %d, type: %d\n", cmd, type);
}

u8 getSmartCmdStatus(u8 type)
{
    return recePhone.cmdNum[type];
}

static u8
compData(u8 *a, u8 *b, u8 size)
{
    u8 i = 0;
    
    for(i = 0; i < size; i++)
        if(*(a + i) != *(b + i))
            return FALSE;
    
    return TRUE;
}


static u8
crc8_msb(u8 poly, u8* data, u8 size)
{
    u8 crc = 0;
    u8 bit = 0;
    
    while (size--) {
        crc ^= *data++;
        for (bit = 0; bit < 8; bit++) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ poly;
            } else {
                crc <<= 1;
            }
        }
    }
    
    return crc;
}


static inline void
swap(u8 *a, u8 *b)
{
    u8 temp;
    temp=*a;
    *a=*b;
    *b=temp;
}

static void
KSA()
{
    int i,j=0;
    for(i=0;i<CMD_DATA_MAX_LEN;i++)
        recePhone.stable[i]=i;
    for(i=0;i<CMD_DATA_MAX_LEN;i++) {
        j=(j+recePhone.stable[i]+ rc4Key[i % strlen((char *)rc4Key)])%CMD_DATA_MAX_LEN;
        swap(&recePhone.stable[i],&recePhone.stable[j]);
    }
}

static void
PRGA()
{
    int m=0,i=0,j=0,t,k = CMD_DATA_MAX_LEN;
    while(k--) {
        i=(i+1)%CMD_DATA_MAX_LEN;
        j=(j+recePhone.stable[i])%CMD_DATA_MAX_LEN;
        swap(&recePhone.stable[i],&recePhone.stable[j]);
        t=(recePhone.stable[j]+recePhone.stable[i])%CMD_DATA_MAX_LEN;
        recePhone.sonkey[m++]=recePhone.stable[t];
    }
}

static inline u8
decryption(u8 value, u8 num)
{
    return (value ^ recePhone.sonkey[num]);
}

static inline u8
decryptionSeq(u8 value)
{
    return (value ^ recePhone.sonkey[0]);
}

static inline u8
setCmdLen(u8 type)
{
    u8 cmdNum = getSmartCmdStatus(type);
    if(cmdNum == START) {
        recePhone.CmdNumLen[type] = strlen((char *)magicNum) + CRC8_DATA;		
    } else if(cmdNum == INFO) {
        recePhone.CmdNumLen[type] =  SSID_DATA + PASS_DATA + CRC8_DATA + IP_DATA;
    } else if(cmdNum == SSID_PASS_DATA){
        recePhone.CmdNumLen[type] = recePhone.ssidLen[type] + recePhone.passLen[type] + CRC8_DATA;
    } else {
        //printf("### setCmdLen error: %d type: %d ###\n", cmdNum, type);
        return FALSE;
    }

    //printf("### %s: cmd=%d, type=%s, CmdNumLen=%d ###\n", __func__, cmdNum, type==AP_MODE? "AP_MODE" : "PHONE_MODE", recePhone.CmdNumLen[type]);
    //printf("### recePhone.CmdNumLen: %d ###\n", recePhone.CmdNumLen[type]);
    return TRUE;
}

void clearAllSmartLinkBuf()
{
    memset(recePhone.checkBaseLenBuf[PHONE_MODE], 0, 5);
    recePhone.CmdNumLen[PHONE_MODE] = 0;
    recePhone.baseLenBufCount[PHONE_MODE]= 0;
    memset(recePhone.checkBaseLenBuf[AP_MODE], 0, 5);
    recePhone.CmdNumLen[AP_MODE] = 0;
    recePhone.baseLenBufCount[AP_MODE]= 0;
}

static void changeRxDataControlFlow(bool accept)
{
    volatile u32 *accept_unicast = (u32 *)(ADR_MRX_FLT_TB13);

    if (accept)
        *accept_unicast = 0x2;
    else
        *accept_unicast = 0x3;
}

void resetSmartLink()
{
    printf("%s\n", __func__);
    clearAllSmartLinkBuf();	
    smart_icomm_state = RESET_SMART_ICOMM;
    setSmartCmdStatus(CHECK, PHONE_MODE);
    setSmartCmdStatus(CHECK, AP_MODE);
    //gwifistatus.smart_tmerSec = 10;
    recePhone.stopScan = FALSE;
    recePhone.stage1Timer = FALSE;
    recePhone.stage2Timer = FALSE;
	
    memset(&gwifistatus.connAP, 0, sizeof(AP_DETAIL_INFO));
    changeRxDataControlFlow(true);
	
    initscanchannel();
    //if(gwifistatus.action == WIFI_CONNECTION)
    //    startscanchannel(200 * CLOCK_MINI_SECOND);
    //else
    //    startscanchannel(20 * CLOCK_MINI_SECOND);
}

void initSmartLink()
{
    printf("%s\n", __func__);
    smart_icomm_state = START_SMART_ICOMM;
    memset(&recePhone, 0, sizeof(smart_control));
    recePhone.backData[PHONE_MODE] = 0xffff;
    recePhone.backData[AP_MODE] = 0xffff;
    //recePhone.stopScan = FALSE;
    //recePhone.stage1Timer = FALSE;
    //recePhone.stage2Timer = FALSE;
    memset(&gwifistatus.connAP, 0, sizeof(AP_DETAIL_INFO));
    setSmartCmdStatus(CHECK, PHONE_MODE);
    setSmartCmdStatus(CHECK, AP_MODE);
    KSA();
    PRGA();
    changeRxDataControlFlow(true);	
    initscanchannel();
    startscanchannel(DEFAULT_SCAN_TIME_MS);
}

void stopSmartLink()
{
    printf("%s\n", __func__);
    stopscanchannel();
    smart_icomm_state = STOP_SMART_ICOMM;
    clearAllSmartLinkBuf();	
    recePhone.stopScan = FALSE;
    recePhone.stage1Timer = FALSE;
    recePhone.stage2Timer = FALSE;
    setSmartCmdStatus(DONE, PHONE_MODE);
    setSmartCmdStatus(DONE, AP_MODE);
    changeRxDataControlFlow(false);
    stopscanchannel();
}

static inline void 
checkSeqSetPacket(u8 cmdValue, u8 type)
{
    u8 cmdNum = getSmartCmdStatus(type);
    recePhone.seqData[type] = decryptionSeq(cmdValue);
    recePhone.NumSeq[type] = (recePhone.seqData[type] >> cmdNum);//seq shift cmd num bit
    //printf("recePhone.seqData:%d cmdValue:%d\n", recePhone.seqData[type], cmdValue);
    //printf("[%d]seqData: %d NumSeq: %d CmdNumLen: %d\n", type,recePhone.seqData[type], recePhone.NumSeq[type], recePhone.CmdNumLen[type]);
    if(((recePhone.seqData[type] & cmdNum) == cmdNum) && (recePhone.NumSeq[type] <= recePhone.CmdNumLen[type]) ) {
        //printf("[%d]NumSeq: %d backSeq: %d backData: %d\n", type,recePhone.NumSeq[type], recePhone.backSeq[type], recePhone.backData[type]);
        if((recePhone.NumSeq[type] - recePhone.backSeq[type]) == 1 && recePhone.backData[type] != 0xffff) {
            recePhone.decrypValue = decryption((u8)recePhone.backData[type], recePhone.backSeq[type]);
            recePhone.packet[type][recePhone.backSeq[type]] = recePhone.decrypValue;
            printf("[%d]packet: %s\n", type, recePhone.packet[type]);
        } else {
            //printf("may be lost smartConf data: %d\n", cmdValue);
        }
        recePhone.backSeq[type] = recePhone.NumSeq[type];
    } else {
        recePhone.backSeq[type]  = recePhone.CmdNumLen[type] + 1;
        //printf("not smartConf data: %d\n", cmdValue);
    }
    recePhone.backData[type] = 0xffff;

}


static inline u8 
checkCrcWithSaveData(PKT_Info *pPktInfo, u8 cmdValue, u8 type)
{
    u8 cmdNum = getSmartCmdStatus(type);    
    PHDR80211_Data ptr = (PHDR80211_Data)((u32)pPktInfo + pPktInfo->hdr_offset);

    if(recePhone.backData[type]  != 0xffff) {
        recePhone.backSeq[type]  = recePhone.CmdNumLen[type] + 1;
    } else {
        recePhone.backData[type] = cmdValue;
    }
    //printf("cmdNum: %d, type: %d recePhone.backData: %d\n", cmdNum, type, recePhone.backData[type]);
    if(recePhone.backSeq[type] == (recePhone.CmdNumLen[type] - CRC8_DATA)) {//seq num equal crc8 data
        recePhone.crcData = crc8_msb(poly,recePhone.packet[type], (recePhone.CmdNumLen[type] - CRC8_DATA));
        if(recePhone.crcData == decryption((u8)recePhone.backData[type], recePhone.backSeq[type]) && cmdNum == START) {
            if(TRUE == compData(recePhone.packet[type], magicNum, strlen((char *)magicNum))) {
                setSmartCmdStatus(INFO, type);//change start cmd to ssid_data cmd
                recePhone.backSeq[type] = 0;
                recePhone.backData[type] = 0xffff;
                memset(recePhone.packet[type], 0, CMD_DATA_MAX_LEN);
                //etimer_reset(&timer);
                //printf("[%d]have magic data\n", type);
                if(recePhone.stage1Timer == FALSE) {
                    //gwifistatus.smart_tmerSec = 20;
                    //process_post_synch(&smart_conf_process, PROCESS_EVENT_MSG, NULL);
                    recePhone.stage1Timer =TRUE;
                }
            }
        } else if(recePhone.crcData == decryption((u8)recePhone.backData[type], recePhone.backSeq[type]) && cmdNum == INFO) {
            setSmartCmdStatus(SSID_PASS_DATA, type);//change start cmd to ssid_data cmd
            recePhone.backSeq[type] = 0;
            recePhone.backData[type] = 0xffff;
            recePhone.ssidLen[type] = recePhone.packet[type][0];
            recePhone.passLen[type] = recePhone.packet[type][SSID_DATA];
            memcpy(recePhone.phoneIP[type], &recePhone.packet[type][SSID_DATA + 1], IP_DATA);
            printf("[%d] phoneIP: %d.%d.%d.%d\n",type, recePhone.phoneIP[type][0],  recePhone.phoneIP[type][1],  recePhone.phoneIP[type][2],  recePhone.phoneIP[type][3]);
            if(type == PHONE_MODE) {
                memcpy(recePhone.phoneMAC[PHONE_MODE], &ptr->addr2.addr[0], MAC_DATA);
            } else {
                memcpy(recePhone.phoneMAC[AP_MODE], &ptr->addr1.addr[0], MAC_DATA);
            }
            printf("[%d] phoneMAC: %02x:%02x:%02x:%02x:%02x:%02x\n", type, recePhone.phoneMAC[type][0],  recePhone.phoneMAC[type][1],  recePhone.phoneMAC[type][2],  recePhone.phoneMAC[type][3],  recePhone.phoneMAC[type][4],  recePhone.phoneMAC[type][5]);
            memset(recePhone.packet[type], 0, CMD_DATA_MAX_LEN);
            if(recePhone.stage2Timer == FALSE) {
                //gwifistatus.smart_tmerSec = 40;
                //process_post_synch(&smart_conf_process, PROCESS_EVENT_MSG, NULL);
                recePhone.stage2Timer =TRUE;
            }
			//setSmartCmdStatus(DONE, type);
        } else if(recePhone.crcData == decryption((u8)recePhone.backData[type], recePhone.backSeq[type]) && cmdNum == SSID_PASS_DATA){
            //printf("data cmd end packet: %s\n", recePhone.packet);
            setSmartCmdStatus(AP_LIST_CHECK, PHONE_MODE);//change ssid_data cmd to check base len
            setSmartCmdStatus(AP_LIST_CHECK, AP_MODE);//change ssid_data cmd to check base len
            recePhone.backData[type] = 0xffff;
            recePhone.baseLenBufCount[type] = 0;
            gwifistatus.connAP.ssid_len= recePhone.ssidLen[type];
            memcpy(gwifistatus.connAP.ssid, recePhone.packet[type], (unsigned int)recePhone.ssidLen[type]);
            //gwifistatus.connAP.ssid[recePhone.ssidLen[type]] = '\0';
            gwifistatus.connAP.key_len= recePhone.passLen[type];
            memcpy(gwifistatus.connAP.key, (recePhone.packet[type] + recePhone.ssidLen[type]), (unsigned int)recePhone.passLen[type]);
            //pbkdf2_sha1((unsigned char *)gwifistatus.connAP.key, gwifistatus.connAP.key_len, (unsigned char *)gwifistatus.connAP.ssid, gwifistatus.connAP.ssid_len, 4096, (unsigned char *)gwifistatus.connAP.pmk, 32);
            printf("###############SSID: %s LEN: %d###############\n", gwifistatus.connAP.ssid, gwifistatus.connAP.ssid_len);
            printf("###############PASS: %s LEN: %d###############\n", gwifistatus.connAP.key, gwifistatus.connAP.key_len);
            memset(recePhone.packet[PHONE_MODE], 0, CMD_DATA_MAX_LEN);
            memset(recePhone.packet[AP_MODE], 0, CMD_DATA_MAX_LEN);
            //setSmartPhoneIP(&(recePhone.phoneIP[type][0]), IP_DATA);
            //setPhoneMAC(&(recePhone.phoneMAC[type][0]), MAC_DATA);
            //memcpy(recePhone.buf, &(gCabrioConf).local_ip_addr, IP_DATA);
            //memcpy(recePhone.buf + IP_DATA, &(gwifistatus).local_mac, MAC_DATA);
            //setPhoneData(&(recePhone.buf[0]), IP_DATA + MAC_DATA);
            setSmartCmdStatus(DONE, type);
            return TRUE;
        } else {
            //printf("do nothing: %d\n", cmdValue);
        }
    }
    return FALSE;
}

static void sendSmartLinkInfo()
{
    u16				len;
    MsgEvent		*msg_evt;
    len = sizeof(struct ssv6xxx_si_cfg);
    HOST_EVENT_ALLOC(msg_evt, SOC_EVT_SMART_ICOMM, len);

    if(msg_evt){
#if (CONFIG_LOG_ENABLE)
        struct ssv6xxx_si_cfg *si_evt;

        si_evt = (struct ssv6xxx_si_cfg *)(LOG_EVENT_DATA_PTR(msg_evt));
        memcpy(si_evt->ssid, gwifistatus.connAP.ssid, gwifistatus.connAP.ssid_len);
        memcpy(si_evt->password, gwifistatus.connAP.key, gwifistatus.connAP.key_len);
        si_evt->ssid_len = gwifistatus.connAP.ssid_len;
        si_evt->password_len = gwifistatus.connAP.key_len;

        printf("===\n");
        printf("sendSmartLinkInfo:\nSSID=%s, LEN=%d\nPASS=%s, LEN=%d\n",
                si_evt->ssid, si_evt->ssid_len, si_evt->password, si_evt->password_len);
        printf("===\n");
#endif
        HOST_EVENT_SET_LEN(msg_evt, len);
        HOST_EVENT_SEND(msg_evt);

        stopSmartLink();
    }

    return;
}

static inline void 
setSmartConfData(PKT_Info *pPktInfo, u8 type)
{
    u32 size = 0;
    u8 status = 0;
    u32 value = pPktInfo->len;
    if(recePhone.baseLenBufCount[type] <= MAX_BASE_LEN_BUF) {
    	checkBaseLen(value, type);
    } else {
        if(value < recePhone.checkBaseLenBuf[type][0] )
            return;
        
        size  =  value - recePhone.checkBaseLenBuf[type][0] ;//send data offset 1 byte
        status = setCmdLen(type);
        //printf("value: %d size: %d\n", value, size);
        if(FALSE == status) {
            //printf("getCmdLen can not get normal cmd number\n");
        }

        if(size >= SEQ_BASE_CMD) {// seq data
            checkSeqSetPacket(size - SEQ_BASE_CMD, type);
        } else {// ssid & password data   	
            if(TRUE == checkCrcWithSaveData(pPktInfo, size, type)) {
                sendSmartLinkInfo();
                //SET_REASON_TRAP0(0x0000);
            } else {
                //printf("setSmart do nothing: %d\n", cmdValue);
            }
        }
    }
}


void rx_process_smartConf(PKT_Info *pPktInfo, int unicast_ptk)
{
    u8 cmdNum = 0;
    u8 type = NOTYPE;
    
    gwifistatus.ptk_in_checking = true;

    if (unicast_ptk) {
        type = PHONE_MODE;
    } else {
        type = AP_MODE;
    } 

    cmdNum = getSmartCmdStatus(type);
    if(cmdNum != DONE) {
        if(cmdNum != AP_LIST_CHECK) {
            setSmartConfData(pPktInfo, type);
        } else {
            //setSmartCmdStatus(AP_LIST_CHECK, type);
            //process_post_synch(&smart_conf_process, PROCESS_EVENT_CONTINUE, NULL);
        }
    } else {
        //dump_data ("other data", &rx_skb.data[0], rx_skb.len);
    }
    
    gwifistatus.ptk_in_checking = false;
}


static inline void 
checkBaseLen(u8 value, u8 type)
{
    u8 i = 0, count = 0, lastBufVal = 0;

    //printf("[%d]recePhone.baseLenBufCount: %d value: %d \n", type ,recePhone.baseLenBufCount[type], value);
    if(recePhone.checkBaseLenBuf[type][0] >= value) {
        memset(recePhone.checkBaseLenBuf[type], 0, 5);
        recePhone.baseLenBufCount[type] = 0;
    }
    count = recePhone.baseLenBufCount[type];
    lastBufVal = recePhone.checkBaseLenBuf[type][count -1];
    if(count > 0 && (lastBufVal == value)) {
        return;
    } else if(count > 0 && ((value - lastBufVal) != 1)) {
        memset(recePhone.checkBaseLenBuf[type], 0, 5);
        recePhone.baseLenBufCount[type] = 0;
        return;
    }
    recePhone.checkBaseLenBuf[type][count] = value;
    recePhone.baseLenBufCount[type] = (count + 1) % MAX_BASE_LEN_BUF;
#if 0
    printf("status[%d] - ", type);
    for( i = 0; i < MAX_BASE_LEN_BUF; i++ ) {
        printf("%d ", recePhone.checkBaseLenBuf[type][i]);
    }
    printf("\n");
#endif

    //TODO: give it one more chance for the incoming three ptks while we need change it the next channel.
    for(i = 0; i < 3; i++) {
        if(recePhone.checkBaseLenBuf[type][i+1] - recePhone.checkBaseLenBuf[type][i] != 1) {
            if(count >= 4)
                memset(recePhone.checkBaseLenBuf[type], 0, 5);
            return;
        }
    }

    recePhone.baseLenBufCount[type] = MAX_BASE_LEN_BUF + 1;
    setSmartCmdStatus(START, type);
    if(recePhone.stopScan == FALSE) {
        recePhone.stopScan = TRUE;
		
        stopscanchannel();
        //gwifistatus.checkbeacon = FALSE;
        //process_start(&smart_conf_process, NULL);
        //printf("set channel: %d\n", gwifistatus.connAP.channel);
    }
}

#endif //CONFIG_SSV_SMARTLINK
