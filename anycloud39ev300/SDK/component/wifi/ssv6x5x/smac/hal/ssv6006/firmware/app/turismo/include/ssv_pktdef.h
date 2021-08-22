#ifndef _SSV_PKTDEF_H_
#define _SSV_PKTDEF_H_

#include <types.h>
#include <ssv6006_common.h>
#include <turismo_common.h>
#include <ssv6006_mac.h>
#include <ieee80211/ieee80211_util.h>

#define M0_HDR_LEN                          4
#define M1_HDR_LEN                          8
#define M2_HDR_LEN                          16

#define RX_M0_HDR_LEN                       24


/**
 *
 *  Offset Table (register):
 *
 *    c_type            hdr_len     
 *  ----------     ----------
 *  M0_TXREQ         8-bytes
 *  M1_TXREQ         12-bytes   
 *  M2_TXREQ         sizeof(PKT_TXInfo)
 *  M0_RXEVENT      
 *  M1_RXEVENT
 *  HOST_CMD
 *  HOSt_EVENT
 *
 *
 */

#define IS_RX_PKT(_p)       ((_p)->c_type==M0_RXEVENT)
#define IS_TX_PKT(_p)       (/*((_p)->c_type>=M0_TXREQ)&&*/((_p)->c_type<=M2_TXREQ))

#define PBUF_HDR80211(p, i)             (*((u8 *)(p)+(p)->hdr_offset + (i)))


#define cfg_host_rxpkt ssv6006_rx_desc  

typedef struct ssv6006_tx_desc PKT_TxInfo;
typedef struct ssv6006_rx_desc PKT_RxInfo;
typedef struct ssv6006_rx_desc PKT_Info;

/**
 * Define constants for do_rts_cts field of PKT_TxInfo structure 
 * 
 * @ TX_NO_RTS_CTS
 * @ TX_RTS_CTS
 * @ TX_CTS
 */
#define TX_NO_RTS_CTS                   0
#define TX_RTS_CTS                      1
#define TX_CTS                          2

enum fcmd_seek_type {
    FCMD_SEEK_PREV  = 0,
    FCMD_SEEK_CUR,
    FCMD_SEEK_NEXT,
};

#endif /* _SSV_PKTDEF_H_ */

