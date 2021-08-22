#ifndef __USR_CFG_H
#define	__USR_CFG_H

#include <sys/time.h>
#include "xrf_type.h"
#include "defs.h"
#include "wifi.h"

#define DEBUG_LEVEL1
//#define DEBUG_LEVEL2

#define STA_SUPPORT		
#define STA_CFG80211

//#define NOT_BYPASS_5GHZ
//#define NOT_REDUCE_UNUSE_FUNC
//#define FW_OUTSIDE

#define UAP_SUPPORT			
#define UAP_CFG80211

//#define REASSOCIATION		// 1.3版本引入，还没严格测试
//#define WIFI_DIRECT_SUPPORT	//尚不支持

#define UAP_CFG11N //wmj+
/*wmj+ 20160612 for tx pending control in moal, max pending number can set to 1 ~ 255*/
#define MAX_TX_PENDING_CTRL 1 
//wmj<<

//wmj+
//#define REASSOCIATION


#ifndef STA_SUPPORT
#undef REASSOCIATION
#endif


//内存优化选项，将一部分数组改为动态分配
#define CMD_BUFFER_ALLOC_DYNAMIC		1
#define EVENT_BODY_ALLOC_IN_USE			1
#define ASSOC_RSP_BUFF_ALLOC_IN_USE		1
#define GEN_IE_BUF_ALLOC_IN_USE			1
#define WAPI_IE_ALLOC_IN_USE			1
#define WPA_IE_ALLOC_IN_USE				1
#define WPS_IE_ALLOC_IN_USE				1


#define MARVELL_VENDOR_ID 	0x02df

#define SD_DEVICE_ID_8797   (0x9129)
#define SD_DEVICE_ID_8782   (0x9121)
#define SD_DEVICE_ID_8801   (0x9139)

#define MAX_LINKS				4

#define LINUX_VERSION_CODE		1
#define KERNEL_VERSION(a,b,c)	(0)

#define KERN_DEBUG
#define KERN_ERR
#define KERN_ALERT
#define KERN_INFO
#define KERN_WARNING

#define GFP_NOIO 0
#define __GFP_NOWARN	0x200

#define uninitialized_var(x) x

#define NSEC_PER_USEC	1000


#define printk		  printf
#define panic		  printf

#define netif_carrier_ok(x)	1	//mark 需要实现
#define netif_carrier_off(x)
#define netif_stop_queue(x)
#define netif_start_queue(x)
#define netif_carrier_on(x)
#define netif_wake_queue(x)

#define sdio_claim_host(x)
#define sdio_release_host(x)

#define kfree 	mem_free
#define kmalloc(x,y) mem_malloc(x)
#define kzalloc(x,y) mem_calloc(x, 1)

#define random32	os_time_get

#define  is_broadcast_ether_addr(addr)  (((addr)[0] & (addr)[1] & (addr)[2] & (addr)[3] & (addr)[4] & (addr)[5]) == 0xff)

#define  is_zero_ether_addr(addr)  (!((addr)[0] | (addr)[1] | (addr)[2] | (addr)[3] | (addr)[4] | (addr)[5]))

#define  is_multicast_ether_addr(addr)  (0x01 & (addr)[0])

#define  is_local_ether_addr(addr)  (0x02 & (addr)[0])

#define  is_unicast_ether_addr(addr)	!(is_multicast_ether_addr)

#define  is_valid_ether_addr(addr)	 (!is_multicast_ether_addr(addr) && !is_zero_ether_addr(addr))


struct scan_result_data{
	uint32_t   	num;
	uint8_t		mode;   			//工作模式	
	uint8_t		proto;   			//安全认证类型
	uint8_t		cipher;   			//加密模式
	uint8_t		channel;   			//频道
	uint32_t	freq;				//频率
	int32		rssi;				//信号强度
	uint8_t 	essid_len;
	uint8_t 	essid[32];  		// essid
	uint8_t 	bssid[6];  			//mac地址
}__packed;


#define PACKET_HOST		0		/* To us		*/
#define PACKET_BROADCAST	1		/* To all		*/
#define PACKET_MULTICAST	2		/* To group		*/
#define PACKET_OTHERHOST	3		/* To someone else 	*/
#define PACKET_OUTGOING		4		/* Outgoing of any type */
/* These ones are invisible by user level */
#define PACKET_LOOPBACK		5		/* MC/BRD frame looped back */
#define PACKET_FASTROUTE	6		/* Fastrouted frame	*/


/** Maximum length of lines in configuration file */
#define MAX_CONFIG_LINE                 1024
/** Ethernet address length */
#define ETH_ALEN                        6
/** MAC BROADCAST */
#define MAC_BROADCAST   0x1FF
/** MAC MULTICAST */
#define MAC_MULTICAST   0x1FE

/** Character, 1 byte */
typedef char t_s8;
/** Unsigned character, 1 byte */
typedef unsigned char t_u8;

/** Short integer */
typedef signed short t_s16;
/** Unsigned short integer */
typedef unsigned short t_u16;

/** Integer */
typedef signed int t_s32;
/** Unsigned integer */
//wmj -
//typedef unsigned int t_u32;


/** Long long integer */
//typedef  long long t_s64;
/** Unsigned long long integer */
//typedef unsigned long long t_u64;

#define inline __inline

#define GFP_ATOMIC	1
#define GFP_KERNEL	2
#define GFP_DMA		4

#define sdio_set_drvdata(x,y) 	x->dev_data = y
#define sdio_get_drvdata(x) 	x->dev_data


#define SIOCDEVPRIVATE	0x89F0	/* to 89FF */

/*
 *	TCP option
 */
 
#define TCPOPT_NOP		1	/* Padding */
#define TCPOPT_EOL		0	/* End of options */
#define TCPOPT_MSS		2	/* Segment size negotiating */
#define TCPOPT_WINDOW		3	/* Window scaling */
#define TCPOPT_SACK_PERM        4       /* SACK Permitted */
#define TCPOPT_SACK             5       /* SACK Block */
#define TCPOPT_TIMESTAMP	8	/* Better RTT estimations/PAWS */
#define TCPOPT_MD5SIG		19	/* MD5 Signature (RFC2385) */
#define TCPOPT_COOKIE		253	/* Cookie extension (experimental) */


/*
 *     TCP option lengths
 */

#define TCPOLEN_MSS            4
#define TCPOLEN_WINDOW         3
#define TCPOLEN_SACK_PERM      2
#define TCPOLEN_TIMESTAMP      10
#define TCPOLEN_MD5SIG         18
#define TCPOLEN_COOKIE_BASE    2	/* Cookie-less header extension */
#define TCPOLEN_COOKIE_PAIR    3	/* Cookie pair header extension */
//#define TCPOLEN_COOKIE_MIN     (TCPOLEN_COOKIE_BASE+TCP_COOKIE_MIN)
//#define TCPOLEN_COOKIE_MAX     (TCPOLEN_COOKIE_BASE+TCP_COOKIE_MAX)


/* Flags for encoding (along with the token) */
#define IW_ENCODE_INDEX		0x00FF	/* Token index (if needed) */
#define IW_ENCODE_FLAGS		0xFF00	/* Flags defined below */
#define IW_ENCODE_MODE		0xF000	/* Modes defined below */
#define IW_ENCODE_DISABLED	0x8000	/* Encoding disabled */
#define IW_ENCODE_ENABLED	0x0000	/* Encoding enabled */
#define IW_ENCODE_RESTRICTED	0x4000	/* Refuse non-encoded packets */
#define IW_ENCODE_OPEN		0x2000	/* Accept non-encoded packets */
#define IW_ENCODE_NOKEY		0x0800  /* Key is write only, so not present */
#define IW_ENCODE_TEMP		0x0400  /* Temporary key */

/* Power management flags available (along with the value, if any) */
#define IW_POWER_ON		0x0000	/* No details... */
#define IW_POWER_TYPE		0xF000	/* Type of parameter */
#define IW_POWER_PERIOD		0x1000	/* Value is a period/duration of  */
#define IW_POWER_TIMEOUT	0x2000	/* Value is a timeout (to go asleep) */
#define IW_POWER_MODE		0x0F00	/* Power Management mode */
#define IW_POWER_UNICAST_R	0x0100	/* Receive only unicast messages */
#define IW_POWER_MULTICAST_R	0x0200	/* Receive only multicast messages */
#define IW_POWER_ALL_R		0x0300	/* Receive all messages though PM */
#define IW_POWER_FORCE_S	0x0400	/* Force PM procedure for sending unicast */
#define IW_POWER_REPEATER	0x0800	/* Repeat broadcast messages in PM period */
#define IW_POWER_MODIFIER	0x000F	/* Modify a parameter */
#define IW_POWER_MIN		0x0001	/* Value is a minimum  */
#define IW_POWER_MAX		0x0002	/* Value is a maximum */
#define IW_POWER_RELATIVE	0x0004	/* Value is not in seconds/ms/us */

/* Transmit Power flags available */
#define IW_TXPOW_TYPE		0x00FF	/* Type of value */
#define IW_TXPOW_DBM		0x0000	/* Value is in dBm */
#define IW_TXPOW_MWATT		0x0001	/* Value is in mW */
#define IW_TXPOW_RELATIVE	0x0002	/* Value is in arbitrary units */
#define IW_TXPOW_RANGE		0x1000	/* Range of value between min/max */

/* Retry limits and lifetime flags available */
#define IW_RETRY_ON		0x0000	/* No details... */
#define IW_RETRY_TYPE		0xF000	/* Type of parameter */
#define IW_RETRY_LIMIT		0x1000	/* Maximum number of retries*/
#define IW_RETRY_LIFETIME	0x2000	/* Maximum duration of retries in us */
#define IW_RETRY_MODIFIER	0x00FF	/* Modify a parameter */
#define IW_RETRY_MIN		0x0001	/* Value is a minimum  */
#define IW_RETRY_MAX		0x0002	/* Value is a maximum */
#define IW_RETRY_RELATIVE	0x0004	/* Value is not in seconds/ms/us */
#define IW_RETRY_SHORT		0x0010	/* Value is for short packets  */
#define IW_RETRY_LONG		0x0020	/* Value is for long packets */

/* Scanning request flags */
#define IW_SCAN_DEFAULT		0x0000	/* Default scan of the driver */
#define IW_SCAN_ALL_ESSID	0x0001	/* Scan all ESSIDs */
#define IW_SCAN_THIS_ESSID	0x0002	/* Scan only this ESSID */
#define IW_SCAN_ALL_FREQ	0x0004	/* Scan all Frequencies */
#define IW_SCAN_THIS_FREQ	0x0008	/* Scan only this Frequency */
#define IW_SCAN_ALL_MODE	0x0010	/* Scan all Modes */
#define IW_SCAN_THIS_MODE	0x0020	/* Scan only this Mode */
#define IW_SCAN_ALL_RATE	0x0040	/* Scan all Bit-Rates */
#define IW_SCAN_THIS_RATE	0x0080	/* Scan only this Bit-Rate */
/* struct iw_scan_req scan_type */
#define IW_SCAN_TYPE_ACTIVE 0
#define IW_SCAN_TYPE_PASSIVE 1
/* Maximum size of returned data */
#define IW_SCAN_MAX_DATA	4096	/* In bytes */

/* Scan capability flags - in (struct iw_range *)->scan_capa */
#define IW_SCAN_CAPA_NONE		0x00
#define IW_SCAN_CAPA_ESSID		0x01
#define IW_SCAN_CAPA_BSSID		0x02
#define IW_SCAN_CAPA_CHANNEL	0x04
#define IW_SCAN_CAPA_MODE		0x08
#define IW_SCAN_CAPA_RATE		0x10
#define IW_SCAN_CAPA_TYPE		0x20
#define IW_SCAN_CAPA_TIME		0x40


/* ARP protocol HARDWARE identifiers. */
#define ARPHRD_NETROM	0		/* from KA9Q: NET/ROM pseudo	*/
#define ARPHRD_ETHER 	1		/* Ethernet 10Mbps		*/
#define	ARPHRD_EETHER	2		/* Experimental Ethernet	*/
#define	ARPHRD_AX25	3		/* AX.25 Level 2		*/
#define	ARPHRD_PRONET	4		/* PROnet token ring		*/
#define	ARPHRD_CHAOS	5		/* Chaosnet			*/
#define	ARPHRD_IEEE802	6		/* IEEE 802.2 Ethernet/TR/TB	*/
#define	ARPHRD_ARCNET	7		/* ARCnet			*/
#define	ARPHRD_APPLETLK	8		/* APPLEtalk			*/
#define ARPHRD_DLCI	15		/* Frame Relay DLCI		*/
#define ARPHRD_ATM	19		/* ATM 				*/
#define ARPHRD_METRICOM	23		/* Metricom STRIP (new IANA id)	*/
#define	ARPHRD_IEEE1394	24		/* IEEE 1394 IPv4 - RFC 2734	*/
#define ARPHRD_EUI64	27		/* EUI-64                       */
#define ARPHRD_INFINIBAND 32		/* InfiniBand			*/

/* Dummy types for non ARP hardware */
#define ARPHRD_SLIP	256
#define ARPHRD_CSLIP	257
#define ARPHRD_SLIP6	258
#define ARPHRD_CSLIP6	259
#define ARPHRD_RSRVD	260		/* Notional KISS type 		*/
#define ARPHRD_ADAPT	264
#define ARPHRD_ROSE	270
#define ARPHRD_X25	271		/* CCITT X.25			*/
#define ARPHRD_HWX25	272		/* Boards with X.25 in firmware	*/
#define ARPHRD_CAN	280		/* Controller Area Network      */
#define ARPHRD_PPP	512
#define ARPHRD_CISCO	513		/* Cisco HDLC	 		*/
#define ARPHRD_HDLC	ARPHRD_CISCO
#define ARPHRD_LAPB	516		/* LAPB				*/
#define ARPHRD_DDCMP    517		/* Digital's DDCMP protocol     */
#define ARPHRD_RAWHDLC	518		/* Raw HDLC			*/

#define ARPHRD_TUNNEL	768		/* IPIP tunnel			*/
#define ARPHRD_TUNNEL6	769		/* IP6IP6 tunnel       		*/
#define ARPHRD_FRAD	770             /* Frame Relay Access Device    */
#define ARPHRD_SKIP	771		/* SKIP vif			*/
#define ARPHRD_LOOPBACK	772		/* Loopback device		*/
#define ARPHRD_LOCALTLK 773		/* Localtalk device		*/
#define ARPHRD_FDDI	774		/* Fiber Distributed Data Interface */
#define ARPHRD_BIF      775             /* AP1000 BIF                   */
#define ARPHRD_SIT	776		/* sit0 device - IPv6-in-IPv4	*/
#define ARPHRD_IPDDP	777		/* IP over DDP tunneller	*/
#define ARPHRD_IPGRE	778		/* GRE over IP			*/
#define ARPHRD_PIMREG	779		/* PIMSM register interface	*/
#define ARPHRD_HIPPI	780		/* High Performance Parallel Interface */
#define ARPHRD_ASH	781		/* Nexus 64Mbps Ash		*/
#define ARPHRD_ECONET	782		/* Acorn Econet			*/
#define ARPHRD_IRDA 	783		/* Linux-IrDA			*/
/* ARP works differently on different FC media .. so  */
#define ARPHRD_FCPP	784		/* Point to point fibrechannel	*/
#define ARPHRD_FCAL	785		/* Fibrechannel arbitrated loop */
#define ARPHRD_FCPL	786		/* Fibrechannel public loop	*/
#define ARPHRD_FCFABRIC	787		/* Fibrechannel fabric		*/
	/* 787->799 reserved for fibrechannel media types */
#define ARPHRD_IEEE802_TR 800		/* Magic type ident for TR	*/
#define ARPHRD_IEEE80211 801		/* IEEE 802.11			*/
#define ARPHRD_IEEE80211_PRISM 802	/* IEEE 802.11 + Prism2 header  */
#define ARPHRD_IEEE80211_RADIOTAP 803	/* IEEE 802.11 + radiotap header */
#define ARPHRD_IEEE802154	  804

#define ARPHRD_PHONET	820		/* PhoNet media type		*/
#define ARPHRD_PHONET_PIPE 821		/* PhoNet pipe header		*/
#define ARPHRD_CAIF	822		/* CAIF media type		*/

#define ARPHRD_VOID	  0xFFFF	/* Void type, nothing is known */
#define ARPHRD_NONE	  0xFFFE	/* zero header length */

/* ARP protocol opcodes. */
#define	ARPOP_REQUEST	1		/* ARP request			*/
#define	ARPOP_REPLY	2		/* ARP reply			*/
#define	ARPOP_RREQUEST	3		/* RARP request			*/
#define	ARPOP_RREPLY	4		/* RARP reply			*/
#define	ARPOP_InREQUEST	8		/* InARP request		*/
#define	ARPOP_InREPLY	9		/* InARP reply			*/
#define	ARPOP_NAK	10		/* (ATM)ARP NAK			*/


#define mdelay	sleep

#define netif_device_detach(x)
#define netif_device_attach(x)

struct list_head {
	struct list_head *next, *prev;
};

struct list {
	struct list *next, *prev;
};

#define xrf_htons(x)	((u16_t)((((x) & 0xff) << 8) | (((x)& 0xff00) >> 8)))

#define xrf_ntohs(x)	xrf_htons(x)

#define	xrf_htonl(x)		\
	((u32_t)((((x) & 0xff) << 24) |	\
    (((x) & 0xff00) << 8) |	\
    (((x) & 0xff0000UL) >> 8) |	\
    (((x) & 0xff000000UL) >> 24)))


#define	xrf_ntohl(x) xrf_htonl(x)

#define __constant_htons	xrf_htons
/*
#define htons(x) xrf_htons(x)
#define ntohs(x) xrf_ntohs(x)
#define htonl(x) xrf_htonl(x)
#define ntohl(x) xrf_ntohl(x)
*/
//#ifndef offsetof
//#define offsetof(type, member) ((long) &((type *) 0)->member)
//#endif

#define container_of(ptr, type, member)  ((type *)( (char *)ptr - offsetof(type,member)))

#define list_entry(ptr, type, member) \
	container_of(ptr, type, member)

#define list_for_each_entry(type, pos, head, member)				\
	for (pos = list_entry((head)->next, type, member);	\
	     &pos->member != (head); 	\
	     pos = list_entry(pos->member.next, type, member))
	     
#define list_for_each(pos, head) \
	for (pos = (head)->next; pos != (head); pos = pos->next)

#define list_for_each_safe(pos, n, head) \
	for (pos = (head)->next, n = pos->next; pos != (head); \
		pos = n, n = pos->next)	

#define list_for_each_entry_safe(type, pos, n, head, member)			\
	for (pos = list_entry((head)->next, type, member),	\
		n = list_entry(pos->member.next, type, member);	\
	     &pos->member != (head); 					\
	     pos = n, n = list_entry(n->member.next, type, member))
	     
#define list_first_entry(ptr, type, member) \
			list_entry((ptr)->next, type, member)

static inline int list_empty(const struct list_head *head)
{
	return head->next == head;
}

void INIT_LIST_HEAD(struct list_head *list);
void list_add_tail(struct list_head *new, struct list_head *head);
void list_add(struct list_head *new, struct list_head *head);
void list_del(struct list_head *entry);


void list_del(struct list_head *entry);

int copy_from_user(void *dst, void *src, int size);
#define copy_to_user	copy_from_user


#define __ATTRIB_ALIGN__

#define wait_queue_head_t	wait_event_t

#define spinlock_t   int


#define spin_lock_irqsave(__lock, __irqflag)	\
do{												\
	*(__lock) = local_irq_save();			\
}while(0)	

#define spin_unlock_irqrestore(__lock , __irqflag)		\
do{												\
	local_irq_restore(*(__lock));					\
}while(0)

#define unlikely
#define likely

#define spin_lock_init(x)	*(x) = 0


typedef u16  __sum16;
typedef u32  __wsum;

/* Modes of operation */
#define IW_MODE_AUTO	0	/* Let the driver decides */
#define IW_MODE_ADHOC	1	/* Single cell network */
#define IW_MODE_INFRA	2	/* Multi cell network, roaming, ... */
#define IW_MODE_MASTER	3	/* Synchronisation master or Access Point */
#define IW_MODE_REPEAT	4	/* Wireless Repeater (forwarder) */
#define IW_MODE_SECOND	5	/* Secondary master/repeater (backup) */
#define IW_MODE_MONITOR	6	/* Passive monitor (listen only) */
#define IW_MODE_MESH	7	/* Mesh (IEEE 802.11s) network */


#define ZEROSIZE 1

struct iphdr {
	__u8	ihl:4,
		version:4;
	__u8	tos;
	__be16	tot_len;
	__be16	id;
	__be16	frag_off;
	__u8	ttl;
	__u8	protocol;
	__sum16	check;
	__be32	saddr;
	__be32	daddr;
	/*The options start here. */
};

struct tcphdr {
	__be16	source;
	__be16	dest;
	__be32	seq;
	__be32	ack_seq;
	__u16	res1:4,
		doff:4,
		fin:1,
		syn:1,
		rst:1,
		psh:1,
		ack:1,
		urg:1,
		ece:1,
		cwr:1;
	__be16	window;
	__sum16	check;
	__be16	urg_ptr;
};

#define udelay us_delay

#define EXPORT_SYMBOL(x)

#define free_netdev mem_free

#define netdev_priv(x)	(x)->ml_priv

void do_gettimeofday(struct timeval *t);
union ktime timeval_to_ktime(struct timeval *t);

#define THIS_MODULE	0

#define ARRAY_SIZE(x,y)	(sizeof(x)/sizeof(y)) 

int	try_module_get(int);
int	module_put(int);
int ieee80211_frequency_to_channel(int freq);

#endif
