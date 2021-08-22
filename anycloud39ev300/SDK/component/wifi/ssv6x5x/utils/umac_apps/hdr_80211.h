/*
 * Copyright (c) 2014 South Silicon Valley Microelectronics Inc.
 * Copyright (c) 2015 iComm Semiconductor Ltd.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef _HDR80211_H_
#define _HDR80211_H_

#define MAC_ADDR_LEN            6

struct _ether_addr {
    unsigned char      addr[MAC_ADDR_LEN];
};

typedef struct _ether_addr ether_addr;

#define MAX_PACKET_SIZE		4096

#define ETHER_ADDR_LEN	6

#define IEEE80211_FRAME_CTL_LEN	4
#define FCS_LEN                 4
#define SECURITY_LEN            16
#define HDR80211_MGMT_LEN       24
#define IEEE80211_QOS_CTL_LEN   2
#define IEEE80211_HT_CTL_LEN    4
#define IEEE80211_4ADDR_LEN    6
#define SNAP_HDR    6


/**
 * IEEE 802.11 Address offset position. This defines the offset of Address1,
 * Address2, Address3 and Address4 if present.
 */
#define OFFSET_ADDR1            4
#define OFFSET_ADDR2            10
#define OFFSET_ADDR3            16
#define OFFSET_ADDR4            24

/* IEEE 802.11 Frame Control: */
#define M_FC_VER		0x0003
#define M_FC_FTYPE		0x000c
#define M_FC_STYPE		0x00f0
#define M_FC_TODS		0x0100
#define M_FC_FROMDS		0x0200
#define M_FC_WDS                0x0300
#define M_FC_MOREFRAGS	        0x0400
#define M_FC_RETRY		0x0800
#define M_FC_PWRMGMT	        0x1000
#define M_FC_MOREDATA		0x2000
#define M_FC_PROTECTED	        0x4000
#define M_FC_ORDER		0x8000

#define M_SC_FRAG		0x000F
#define M_SC_SEQ		0xFFF0

#define M_QOS_TID		0x000F
#define M_QOS_TYPE		0x0010
#define M_QOS_ACK_POLICY	0x0060
#define M_QOS_TXOP		0xFF00

/* Frame Type: */
#define FT_MGMT		        0x0000
#define FT_CTRL		        0x0004
#define FT_DATA		        0x0008

/* Frame SubType: Management Frames */
#define FST_ASSOC_REQ	        0x0000
#define FST_ASSOC_RESP	        0x0010
#define FST_REASSOC_REQ	        0x0020
#define FST_REASSOC_RESP	0x0030
#define FST_PROBE_REQ	        0x0040
#define FST_PROBE_RESP	        0x0050
#define FST_BEACON		0x0080
#define FST_ATIM		0x0090
#define FST_DISASSOC	        0x00A0
#define FST_AUTH		0x00B0
#define FST_DEAUTH		0x00C0
#define FST_ACTION		0x00D0

/* Frame SubType: Control Frames */
#define FST_CONTROL_WRAPPER     0x0070
#define FST_BA_REQ	        0x0080
#define FST_BA	         	0x0090
#define FST_PSPOLL		0x00A0
#define FST_RTS		        0x00B0
#define FST_CTS		        0x00C0
#define FST_ACK		        0x00D0
#define FST_CFEND		0x00E0
#define FST_CFENDACK	        0x00F0

/* Frame SubType: Data Frames */
#define FST_DATA		0x0000
#define FST_DATA_CFACK		0x0010
#define FST_DATA_CFPOLL		0x0020
#define FST_DATA_CFACKPOLL	0x0030
#define FST_NULLFUNC		0x0040
#define FST_CFACK		0x0050
#define FST_CFPOLL		0x0060
#define FST_CFACKPOLL		0x0070
#define FST_QOS_DATA		0x0080
#define FST_QOS_DATA_CFACK	0x0090
#define FST_QOS_DATA_CFPOLL	0x00A0
#define FST_QOS_DATA_CFACKPOLL	0x00B0
#define FST_QOS_NULLFUNC	0x00C0
#define FST_QOS_CFACK		0x00D0
#define FST_QOS_CFPOLL		0x00E0
#define FST_QOS_CFACKPOLL	0x00F0


#define IS_EQUAL(a, b)                  ( (a) == (b) )

#define FC_TYPE(fc)           ((fc) & (M_FC_FTYPE|M_FC_STYPE))
#define FC_FTYPE(fc)          ((fc) & M_FC_FTYPE)
#define FC_STYPE(fc)          ((fc) & M_FC_STYPE)

#define IS_TODS_SET(fc)       ((fc) & M_FC_TODS)
#define IS_FROMDS_SET(fc)     ((fc) & M_FC_FROMDS)
#define IS_MOREFLAG_SET(fc)   ((fc) & M_FC_MOREFRAGS)
#define IS_RETRY_SET(fc)      ((fc) & M_FC_RETRY)
#define IS_PM_SET(fc)         ((fc) & M_FC_PWRMGMT)
#define IS_MOREDATA_SET(fc)   ((fc) & M_FC_MOREDATA)
#define IS_PROTECT_SET(fc)    ((fc) & M_FC_PROTECTED)
#define IS_ORDER_SET(fc)      ((fc) & M_FC_ORDER)
#define IS_4ADDR_FORMAT(fc)   IS_EQUAL(((fc)&(M_FC_TODS|M_FC_FROMDS)), (M_FC_TODS|M_FC_FROMDS))
#define IS_MGMT_FRAME(fc)     IS_EQUAL(FC_FTYPE(fc), FT_MGMT)
#define IS_CTRL_FRAME(fc)     IS_EQUAL(FC_FTYPE(fc), FT_CTRL)
#define IS_DATA_FRAME(fc)     IS_EQUAL(FC_FTYPE(fc), FT_DATA)
#define IS_QOS_DATA(fc)       IS_EQUAL(((fc)&(M_FC_FTYPE|FST_QOS_DATA)), (FT_DATA|FST_QOS_DATA))
#define IS_NULL_DATA(fc)      IS_EQUAL((fc)&(M_FC_FTYPE|FST_NULLFUNC), (FT_DATA|FST_NULLFUNC))
#define IS_QOS_NULL_DATA(fc)  IS_EQUAL((fc)&(M_FC_FTYPE|FST_NULLFUNC|FST_QOS_DATA), (FT_DATA|FST_NULLFUNC|FST_QOS_DATA)) 

#define IS_ASSOC_REQ(fc)      IS_EQUAL(FC_TYPE(fc), (FT_MGMT|FST_ASSOC_REQ))
#define IS_ASSOC_RESP(fc)     IS_EQUAL(FC_TYPE(fc), (FT_MGMT|FST_ASSOC_RESP))
#define IS_REASSOC_REQ(fc)    IS_EQUAL(FC_TYPE(fc), (FT_MGMT|FST_REASSOC_REQ))
#define IS_REASSOC_RESP(fc)   IS_EQUAL(FC_TYPE(fc), (FT_MGMT|FST_REASSOC_RESP))
#define IS_PROBE_REQ(fc)      IS_EQUAL(FC_TYPE(fc), (FT_MGMT|FST_PROBE_REQ))
#define IS_PROBE_RESP(fc)     IS_EQUAL(FC_TYPE(fc), (FT_MGMT|FST_PROBE_RESP))
#define IS_BEACON(fc)         IS_EQUAL(FC_TYPE(fc), (FT_MGMT|FST_BEACON))
#define IS_ATIM(fc)           IS_EQUAL(FC_TYPE(fc), (FT_MGMT|FST_ATIM))
#define IS_DISASSOC(fc)       IS_EQUAL(FC_TYPE(fc), (FT_MGMT|FST_DISASSOC))
#define IS_AUTH(fc)           IS_EQUAL(FC_TYPE(fc), (FT_MGMT|FST_AUTH))
#define IS_DEAUTH(fc)         IS_EQUAL(FC_TYPE(fc), (FT_MGMT|FST_DEAUTH))
#define IS_ACTION(fc)         IS_EQUAL(FC_TYPE(fc), (FT_MGMT|FST_ACTION))
#define IS_PSPOLL(fc)         IS_EQUAL(FC_TYPE(fc), (FT_CTRL|FST_PSPOLL))
#define IS_RTS(fc)            IS_EQUAL(FC_TYPE(fc), (FT_MGMT|FST_RTS))
#define IS_CTS(fc)            IS_EQUAL(FC_TYPE(fc), (FT_MGMT|FST_CTS))
#define IS_ACK(fc)            IS_EQUAL(FC_TYPE(fc), (FT_CTRL|FST_ACK))
#define IS_BAR(fc)            IS_EQUAL(FC_TYPE(fc), (FT_CTRL|FST_BA_REQ))
#define IS_BA(fc)             IS_EQUAL(FC_TYPE(fc), (FT_CTRL|FST_BA))

#define GET_FC_MOREFRAG(fc)	  (((fc) & 0x0400) >> 10)

//Frame Control - type
#define FC_TYPE_MGMT		        0
#define FC_TYPE_CTRL		        1
#define FC_TYPE_DATA		        2

//Frame Control - sub type
//Management
#define FC_SUBTYPE_ASSOC_REQ		0	
#define FC_SUBTYPE_ASSOC_RESP		1	
#define FC_SUBTYPE_REASSOC_REQ		2	
#define FC_SUBTYPE_REASSOC_RESP		3	
#define FC_SUBTYPE_PROBE_REQ		4	
#define FC_SUBTYPE_PROBE_RESP		5	
#define FC_SUBTYPE_BEACON		8	
#define FC_SUBTYPE_ATIM			9	
#define FC_SUBTYPE_DISASSOC		10	
#define FC_SUBTYPE_AUTH			11	
#define FC_SUBTYPE_DEAUTH		12	
#define FC_SUBTYPE_ACTION		13	
#define FC_SUBTYPE_ACTION_NOACK		14	
//Control
#define FC_SUBTYPE_BLOCKACK_REQ		8	
#define FC_SUBTYPE_BLOCKACK		9	
#define FC_SUBTYPE_PS_POLL		10	
#define FC_SUBTYPE_RTS			11	
#define FC_SUBTYPE_CTS			12	
#define FC_SUBTYPE_ACK			13	
#define FC_SUBTYPE_CF_END		14	
#define FC_SUBTYPE_CF_END_ACK		15	
//Data
#define FC_SUBTYPE_DATA			0	
#define FC_SUBTYPE_DATA_CF_ACK		1	
#define FC_SUBTYPE_DATA_CF_POLL		2	
#define FC_SUBTYPE_DATA_CF_ACK_POLL	3	
#define FC_SUBTYPE_NULL			4	
#define FC_SUBTYPE_CF_ACK		5	
#define FC_SUBTYPE_CF_POLL		6	
#define FC_SUBTYPE_CF_ACK_POLL		7	
#define FC_SUBTYPE_QOS_DATA		8	
#define FC_SUBTYPE_QOS_DATA_CF_ACK	9	
#define FC_SUBTYPE_QOS_DATA_CF_POLL	10	
#define FC_SUBTYPE_QOS_DATA_CF_ACK_POLL	11	
#define FC_SUBTYPE_QOS_NULL		12	
#define FC_SUBTYPE_QOS_CF_POLL		14	
#define FC_SUBTYPE_QOS_CF_ACK_POLL	15
//To DS and From DS fields
#define FC_DS_IBSS			0
#define FC_DS_TOAP			1
#define FC_DS_FROMAP			2
#define FC_DS_WDS			3

/* modify 80211 frame macro */
#define SET_FC_TYPE_SUBTYPE(hdr, _fc)	(hdr->fc = ((_fc<<2)  | ((hdr->fc) & (~(M_FC_FTYPE|M_FC_STYPE)))))

#define SET_FC_TYPE(hdr, _fc)		(hdr->fc = ((_fc<<2)  | ((hdr->fc) & (~M_FC_FTYPE))))
#define SET_FC_SUBTYPE(hdr, _fc)	(hdr->fc = ((_fc<<4)  | ((hdr->fc) & (~M_FC_STYPE))))
#define SET_FC_TODS(hdr, _fc)		(hdr->fc = ((_fc<<8)  | ((hdr->fc) & (~M_FC_TODS))))
#define SET_FC_FROMDS(hdr, _fc)		(hdr->fc = ((_fc<<9)  | ((hdr->fc) & (~M_FC_FROMDS))))
#define SET_FC_MOREFLAG(hdr, _fc)	(hdr->fc = ((_fc<<10) | ((hdr->fc) & (~M_FC_MOREFRAGS))))
#define SET_FC_RETRY(hdr, _fc)		(hdr->fc = ((_fc<<11) | ((hdr->fc) & (~M_FC_RETRY))))
#define SET_FC_PM(hdr, _fc)		(hdr->fc = ((_fc<<12) | ((hdr->fc) & (~M_FC_PWRMGMT))))
#define SET_FC_MOREDATA(hdr, _fc)	(hdr->fc = ((_fc<<13) | ((hdr->fc) & (~M_FC_MOREDATA))))
#define SET_FC_PROTECT(hdr, _fc)	(hdr->fc = ((_fc<<14) | ((hdr->fc) & (~M_FC_PROTECTED))))
#define SET_FC_ORDER(hdr, _fc)		(hdr->fc = ((_fc<<15) | ((hdr->fc) & (~M_FC_ORDER))))

#define SET_DURID(hdr, _durid)		(hdr->durid = (unsigned short) _durid)

#define SET_SEQCTRL_FRAG(hdr, _seq)	(hdr = ((_seq<<0) | ((hdr) & (~M_SC_FRAG))))
#define SET_SEQCTRL_SEQ(hdr, _seq)	(hdr = ((_seq<<4) | ((hdr) & (~M_SC_SEQ))))

#define SET_QOS_TID(hdr, _qos)		(hdr = ((_qos<<0) | ((hdr) & (~M_QOS_TID))))
#define SET_QOS_TYPE(hdr, _qos)		(hdr = ((_qos<<4) | ((hdr) & (~M_QOS_TYOE))))
#define SET_QOS_ACK_POLICY(hdr, _qos)	(hdr = ((_qos<<5) | ((hdr) & (~M_QOS_ACK_POLICY))))
#define SET_QOS_TXOP(hdr, _qos)		(hdr = ((_qos<<8) | ((hdr) & (~M_QOS_TXOP))))

#define GET_FC_TYPE_SUBTYPE(hdr)	(((hdr) & (M_FC_FTYPE|M_FC_STYPE)) >>2)
#define GET_FC_TYPE(hdr)		(((hdr) & (M_FC_FTYPE)) >>2)
#define GET_FC_SUBTYPE(hdr)		(((hdr) & (M_FC_STYPE)) >>4)

#define GET_FC_RETRY(hdr)		(((hdr) & (M_FC_RETRY)) >>11)

#define GET_SEQCTRL_FRAG(hdr)		(((hdr) & (M_SC_FRAG)) >>0)
#define GET_SEQCTRL_SEQ(hdr)		(((hdr) & (M_SC_SEQ)) >>4)

#define GET_QOS_TID(hdr)		(((hdr) & (M_QOS_TID))>>0)
#define GET_QOS_TYPE(hdr)		(((hdr) & (M_QOS_TYOE))>>4)
#define GET_QOS_ACK_POLICY(hdr)		(((hdr) & (M_QOS_ACK_POLICY))>>5)
#define GET_QOS_TXOP(hdr)		(((hdr) & (M_QOS_TXOP))>>8)

#define GET_QOS_BIT(hdr)		(((hdr) & (FST_QOS_DATA)) >>7)

#define ST_SEQ_MASK	0xfff0
#define GET_BAR_ST_SEQ(hdr)		(((hdr) & (ST_SEQ_MASK)) >> 4)


typedef struct _frame_control {
    unsigned char version		:2;
    unsigned char type		:2;
    unsigned char qos	        :1;
    unsigned char sub_type	        :3;
    unsigned char ds		:2;
    unsigned char more_frag 	:1;
    unsigned char retry		:1;
    unsigned char pwr_mgt		:1;
    unsigned char more_date 	:1;
    unsigned char protect		:1;
    unsigned char order		:1;
}__attribute__((packed)) frame_control;

//Sequence
typedef struct _sequence_control {
    unsigned short fragment	:4;
    unsigned short sequence	:12;
}__attribute__((packed)) sequence_control;

//QOS
typedef struct _qos_header {
    unsigned short tid		:4;
    unsigned short type		:1;
    unsigned short ack_policy	:2;
    unsigned short reserved	:1;
    unsigned short txop		:8;			
}__attribute__((packed)) qos_header;
#define QOS_HDR_LEN	2

//Ether header
typedef struct _ether_header {
    unsigned char da[MAC_ADDR_LEN];
    unsigned char sa[MAC_ADDR_LEN];
    unsigned short ether_type     ;
}__attribute__((packed)) ether_header;

//802.1h & RFC1042 logical link control
typedef struct _dot11_llc_header {
    unsigned char dsap;
    unsigned char ssap;
    unsigned char control;
    unsigned char encap[3];			
}__attribute__((packed)) dot11_llc_header;
#define DOT11_LLC_HDR_LEN	6

//MAC header
typedef struct _mac_header{
    unsigned short	fc;		
    unsigned short	durid;		
    unsigned char	addr1[MAC_ADDR_LEN];
    unsigned char	addr2[MAC_ADDR_LEN];
    unsigned char	addr3[MAC_ADDR_LEN];
    unsigned short	seq;	
    unsigned char	addr4[MAC_ADDR_LEN];
}__attribute__((packed)) mac_header;

//Control frames
//RTS
typedef struct _rts_frame {
    unsigned short fc;
    unsigned short durid;
    unsigned char  addr1[MAC_ADDR_LEN];
    unsigned char  addr2[MAC_ADDR_LEN];
}__attribute__((packed)) rts_frame;
#define RTS_LEN         16

//CTS
typedef struct _cts_frame {
    unsigned short fc;
    unsigned short durid;
    unsigned char  addr1[MAC_ADDR_LEN];
}__attribute__((packed)) cts_frame;
#define CTS_LEN         10

//ACK
typedef struct _ack_frame {
    unsigned short fc;
    unsigned short durid;
    unsigned char  addr1[MAC_ADDR_LEN];
}__attribute__((packed)) ack_frame;
#define ACK_LEN         10      

//PS-Poll
typedef struct _ps_poll_frame {
    unsigned short fc;
    unsigned short durid;
    unsigned char  addr1[MAC_ADDR_LEN];
    unsigned char  addr2[MAC_ADDR_LEN];
}__attribute__((packed)) ps_poll_frame;
#define PS_POLL_LEN     16

//CF-END
typedef struct _cf_end_frame {
    unsigned short fc;
    unsigned short durid;
    unsigned char  addr1[MAC_ADDR_LEN];
    unsigned char  addr2[MAC_ADDR_LEN];
}__attribute__((packed)) cf_end_frame;
#define CF_END_LEN      16

//CF-End + CF-Ack
typedef struct _cf_end_ack_frame {
    unsigned short fc;
    unsigned short durid;
    unsigned char  addr1[MAC_ADDR_LEN];
    unsigned char  addr2[MAC_ADDR_LEN];
}__attribute__((packed)) cf_end_ack_frame;
#define CF_END_ACK_LEN  16              

//Block Ack Request
typedef struct _block_ack_req {
    unsigned short fc;
    unsigned short durid;
    unsigned char  addr1[MAC_ADDR_LEN];
    unsigned char  addr2[MAC_ADDR_LEN];
    unsigned short bar_control;
    unsigned short seqnum;
}__attribute__((packed)) block_ack_req;
#define BAR_LEN         20              

//Block Ack
#define BA_BITMAP_LEN   128     
typedef struct _block_ack {
    unsigned short fc;
    unsigned short durid;
    unsigned char  addr1[MAC_ADDR_LEN];
    unsigned char  addr2[MAC_ADDR_LEN];
    unsigned short ba_control;
    unsigned short seqnum;
    unsigned char  bitmap[BA_BITMAP_LEN];
}__attribute__((packed)) block_ack;
#define BA_LEN  148
//Control header
typedef struct _control_mac_header {
    unsigned short fc;
    unsigned short durid;
    unsigned char  addr1[MAC_ADDR_LEN];
    unsigned char  addr2[MAC_ADDR_LEN];
}__attribute__((packed)) control_mac_header;
#define CTL_HDR_LEN     16

//Data frames
//Common data header
typedef struct _common_data_mac_header {
    unsigned short    fc;
    unsigned short    durid;
    unsigned char     addr1[MAC_ADDR_LEN];
    unsigned char     addr2[MAC_ADDR_LEN];
    unsigned char     addr3[MAC_ADDR_LEN];
    unsigned short    seqnum;
}__attribute__((packed)) common_data_mac_header;
#define COMMON_DATA_HDR_LEN     24

//Qos data header
typedef struct _qos_data_mac_header {
    unsigned short    fc;
    unsigned short    durid;
    unsigned char     addr1[MAC_ADDR_LEN];
    unsigned char     addr2[MAC_ADDR_LEN];
    unsigned char     addr3[MAC_ADDR_LEN];
    unsigned short    seqnum;
    unsigned short    qos;
}__attribute__((packed)) qos_data_mac_header;
#define QOS_DATA_HDR_LEN        26

//Full data header
typedef struct _full_data_mac_header {
    unsigned short    fc;
    unsigned short    durid;
    unsigned char     addr1[MAC_ADDR_LEN];
    unsigned char     addr2[MAC_ADDR_LEN];
    unsigned char     addr3[MAC_ADDR_LEN];
    unsigned short    seqnum;
    unsigned char     addr4[MAC_ADDR_LEN];
    unsigned short    qos;
    //unsigned char     llc[6]; /* DOT11_LLC_HEADER llc ??*/
}__attribute__((packed)) full_data_mac_header;
#define FULL_DATA_HDR_LEN       38

//Management frames
//Management header
typedef struct _management_mac_header {
        unsigned short     fc;
        unsigned short     durid;
        unsigned char      addr1[MAC_ADDR_LEN];
        unsigned char      addr2[MAC_ADDR_LEN];
        unsigned char      addr3[MAC_ADDR_LEN];
        unsigned short     seqnum;
}__attribute__((packed)) management_mac_header;
#define MGMT_HDR_LEN    24

#define AMPDU_SIGNATURE         0x4E
#define DELIMITER_LEN           0x4
#define ALIGNMENT_OFFSET        0x4

typedef struct _delimiter
{
    unsigned short  reserved:4;     //0-3
    unsigned short  length:12;      //4-15
    unsigned char   crc;
    unsigned char   signature;
}__attribute__((packed)) delimiter;//, *pDELIMITER;

typedef struct _bip_mme
{
    unsigned char  element_id;
    unsigned char  length;
    unsigned short   key_id;
    unsigned char   ipn[6];
    unsigned char   mic[8];
}__attribute__((packed)) bip_mme;

typedef struct _ccmp_hdr
{
    unsigned char  pn_0;
    unsigned char  pn_1;
    unsigned char  rsvd_0;
    unsigned char  rsvd_1:5;
    unsigned char  ext_iv:1;
    unsigned char  key_id:2;
    unsigned char  pn_2;
    unsigned char  pn_3;
    unsigned char  pn_4;
    unsigned char  pn_5;
}__attribute__((packed)) ccmp_hdr;



/**
 * WLAN Operation Mode Definition (for wlan_mode field):
 *
 * @ WLAN_STA: operate as STA (infrastructure) mode
 * @ WLAN_AP: operate as AP (infrastructure) mode
 * @ WLAN_IBSS: operate as IBSS (ad-hoc) mode
 * @ WLAN_WDS: Wireless Distribution System mode (Wireless Bridge)
 */
#define WLAN_STA                            0
#define WLAN_AP                             1
#define WLAN_IBSS                           2
#define WLAN_WDS                            3

#endif /* _HDR80211_H_ */
