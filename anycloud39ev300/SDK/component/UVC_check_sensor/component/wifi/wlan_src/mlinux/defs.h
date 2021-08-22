/*
 * This header file contains global constant/enum definitions,
 * global variable declaration.
 */
#ifndef _LBS_DEFS_H_
#define _LBS_DEFS_H_

//#ifdef CONFIG_LIBERTAS_DEBUG

#ifndef DRV_NAME
#define DRV_NAME "libertas"
#endif


#define ZEROSIZE		1

#define MAX_BSS_LIST_SIZE   	2   //实际只会保存一个

#define	IFNAMSIZ	16
#define	IFALIASZ	256
//#include <linux/hdlc/ioctl.h>

/* Standard interface flags (netdevice->flags). */
#define	IFF_UP		0x1		/* interface is up		*/
#define	IFF_BROADCAST	0x2		/* broadcast address valid	*/
#define	IFF_DEBUG	0x4		/* turn on debugging		*/
#define	IFF_LOOPBACK	0x8		/* is a loopback net		*/
#define	IFF_POINTOPOINT	0x10		/* interface is has p-p link	*/
#define	IFF_NOTRAILERS	0x20		/* avoid use of trailers	*/
#define	IFF_RUNNING	0x40		/* interface RFC2863 OPER_UP	*/
#define	IFF_NOARP	0x80		/* no ARP protocol		*/
#define	IFF_PROMISC	0x100		/* receive all packets		*/
#define	IFF_ALLMULTI	0x200		/* receive all multicast packets*/

#define IFF_MASTER	0x400		/* master of a load balancer 	*/
#define IFF_SLAVE	0x800		/* slave of a load balancer	*/

#define IFF_MULTICAST	0x1000		/* Supports multicast		*/

#define IFF_PORTSEL	0x2000          /* can set media type		*/
#define IFF_AUTOMEDIA	0x4000		/* auto media select active	*/
#define IFF_DYNAMIC	0x8000		/* dialup device with changing addresses*/

#define IFF_LOWER_UP	0x10000		/* driver signals L1 up		*/
#define IFF_DORMANT	0x20000		/* driver signals dormant	*/

#define IFF_ECHO	0x40000		/* echo sent packets		*/

#define IFF_VOLATILE	(IFF_LOOPBACK|IFF_POINTOPOINT|IFF_BROADCAST|IFF_ECHO|\
		IFF_MASTER|IFF_SLAVE|IFF_RUNNING|IFF_LOWER_UP|IFF_DORMANT)

/* Private (from user) interface flags (netdevice->priv_flags). */
#define IFF_802_1Q_VLAN 0x1             /* 802.1Q VLAN device.          */
#define IFF_EBRIDGE	0x2		/* Ethernet bridging device.	*/
#define IFF_SLAVE_INACTIVE	0x4	/* bonding slave not the curr. active */
#define IFF_MASTER_8023AD	0x8	/* bonding master, 802.3ad. 	*/
#define IFF_MASTER_ALB	0x10		/* bonding master, balance-alb.	*/
#define IFF_BONDING	0x20		/* bonding master or slave	*/
#define IFF_SLAVE_NEEDARP 0x40		/* need ARPs for validation	*/
#define IFF_ISATAP	0x80		/* ISATAP interface (RFC4214)	*/
#define IFF_MASTER_ARPMON 0x100		/* bonding master, ARP mon in use */
#define IFF_WAN_HDLC	0x200		/* WAN HDLC device		*/
#define IFF_XMIT_DST_RELEASE 0x400	/* dev_hard_start_xmit() is allowed to
					 * release skb->dst
					 */
#define IFF_DONT_BRIDGE 0x800		/* disallow bridging this ether dev */
#define IFF_DISABLE_NETPOLL	0x1000	/* disable netpoll at run-time */
#define IFF_MACVLAN_PORT	0x2000	/* device used as macvlan port */
#define IFF_BRIDGE_PORT	0x4000		/* device used as bridge port */
#define IFF_OVS_DATAPATH	0x8000	/* device used as Open vSwitch
					 * datapath port */

#define IF_GET_IFACE	0x0001		/* for querying only */
#define IF_GET_PROTO	0x0002

/* For definitions see hdlc.h */
#define IF_IFACE_V35	0x1000		/* V.35 serial interface	*/
#define IF_IFACE_V24	0x1001		/* V.24 serial interface	*/
#define IF_IFACE_X21	0x1002		/* X.21 serial interface	*/
#define IF_IFACE_T1	0x1003		/* T1 telco serial interface	*/
#define IF_IFACE_E1	0x1004		/* E1 telco serial interface	*/
#define IF_IFACE_SYNC_SERIAL 0x1005	/* can't be set by software	*/
#define IF_IFACE_X21D   0x1006          /* X.21 Dual Clocking (FarSite) */

/* For definitions see hdlc.h */
#define IF_PROTO_HDLC	0x2000		/* raw HDLC protocol		*/
#define IF_PROTO_PPP	0x2001		/* PPP protocol			*/
#define IF_PROTO_CISCO	0x2002		/* Cisco HDLC protocol		*/
#define IF_PROTO_FR	0x2003		/* Frame Relay protocol		*/
#define IF_PROTO_FR_ADD_PVC 0x2004	/*    Create FR PVC		*/
#define IF_PROTO_FR_DEL_PVC 0x2005	/*    Delete FR PVC		*/
#define IF_PROTO_X25	0x2006		/* X.25				*/
#define IF_PROTO_HDLC_ETH 0x2007	/* raw HDLC, Ethernet emulation	*/
#define IF_PROTO_FR_ADD_ETH_PVC 0x2008	/*  Create FR Ethernet-bridged PVC */
#define IF_PROTO_FR_DEL_ETH_PVC 0x2009	/*  Delete FR Ethernet-bridged PVC */
#define IF_PROTO_FR_PVC	0x200A		/* for reading PVC status	*/
#define IF_PROTO_FR_ETH_PVC 0x200B
#define IF_PROTO_RAW    0x200C          /* RAW Socket                   */

#define ETH_ALEN	6		/* Octets in one ethernet addr	 */
#define ETH_HLEN	14		/* Total octets in header.	 */
#define ETH_ZLEN	60		/* Min. octets in frame sans FCS */
#define ETH_DATA_LEN	1500		/* Max. octets in payload	 */
#define ETH_FRAME_LEN	1514		/* Max. octets in frame sans FCS */
#define ETH_FCS_LEN	4		/* Octets in the FCS		 */

#define LBS_DEB_ENTER	0x00000001
#define LBS_DEB_LEAVE	0x00000002
#define LBS_DEB_MAIN	0x00000004
#define LBS_DEB_NET	0x00000008
#define LBS_DEB_MESH	0x00000010
#define LBS_DEB_WEXT	0x00000020
#define LBS_DEB_IOCTL	0x00000040
#define LBS_DEB_SCAN	0x00000080
#define LBS_DEB_ASSOC	0x00000100
#define LBS_DEB_JOIN	0x00000200
#define LBS_DEB_11D	0x00000400
#define LBS_DEB_DEBUGFS	0x00000800
#define LBS_DEB_ETHTOOL	0x00001000
#define LBS_DEB_HOST	0x00002000
#define LBS_DEB_CMD	0x00004000
#define LBS_DEB_RX	0x00008000
#define LBS_DEB_TX	0x00010000
#define LBS_DEB_USB	0x00020000
#define LBS_DEB_CS	0x00040000
#define LBS_DEB_FW	0x00080000
#define LBS_DEB_THREAD	0x00100000
#define LBS_DEB_HEX	0x00200000
#define LBS_DEB_SDIO	0x00400000
#define LBS_DEB_SYSFS	0x00800000
#define LBS_DEB_SPI	0x01000000
#define LBS_DEB_CFG80211 0x02000000

extern unsigned int lbs_debug;

/*
 *	These are the defined Ethernet Protocol ID's.
 */

#define ETH_P_LOOP	0x0060		/* Ethernet Loopback packet	*/
#define ETH_P_PUP	0x0200		/* Xerox PUP packet		*/
#define ETH_P_PUPAT	0x0201		/* Xerox PUP Addr Trans packet	*/
#define ETH_P_IP	0x0800		/* Internet Protocol packet	*/
#define ETH_P_X25	0x0805		/* CCITT X.25			*/
#define ETH_P_ARP	0x0806		/* Address Resolution packet	*/
#define	ETH_P_BPQ	0x08FF		/* G8BPQ AX.25 Ethernet Packet	[ NOT AN OFFICIALLY REGISTERED ID ] */
#define ETH_P_IEEEPUP	0x0a00		/* Xerox IEEE802.3 PUP packet */
#define ETH_P_IEEEPUPAT	0x0a01		/* Xerox IEEE802.3 PUP Addr Trans packet */
#define ETH_P_DEC       0x6000          /* DEC Assigned proto           */
#define ETH_P_DNA_DL    0x6001          /* DEC DNA Dump/Load            */
#define ETH_P_DNA_RC    0x6002          /* DEC DNA Remote Console       */
#define ETH_P_DNA_RT    0x6003          /* DEC DNA Routing              */
#define ETH_P_LAT       0x6004          /* DEC LAT                      */
#define ETH_P_DIAG      0x6005          /* DEC Diagnostics              */
#define ETH_P_CUST      0x6006          /* DEC Customer use             */
#define ETH_P_SCA       0x6007          /* DEC Systems Comms Arch       */
#define ETH_P_TEB	0x6558		/* Trans Ether Bridging		*/
#define ETH_P_RARP      0x8035		/* Reverse Addr Res packet	*/
#define ETH_P_ATALK	0x809B		/* Appletalk DDP		*/
#define ETH_P_AARP	0x80F3		/* Appletalk AARP		*/
#define ETH_P_8021Q	0x8100          /* 802.1Q VLAN Extended Header  */
#define ETH_P_IPX	0x8137		/* IPX over DIX			*/
#define ETH_P_IPV6	0x86DD		/* IPv6 over bluebook		*/
#define ETH_P_PAUSE	0x8808		/* IEEE Pause frames. See 802.3 31B */
#define ETH_P_SLOW	0x8809		/* Slow Protocol. See 802.3ad 43B */
#define ETH_P_WCCP	0x883E		/* Web-cache coordination protocol
					 * defined in draft-wilson-wrec-wccp-v2-00.txt */
#define ETH_P_PPP_DISC	0x8863		/* PPPoE discovery messages     */
#define ETH_P_PPP_SES	0x8864		/* PPPoE session messages	*/
#define ETH_P_MPLS_UC	0x8847		/* MPLS Unicast traffic		*/
#define ETH_P_MPLS_MC	0x8848		/* MPLS Multicast traffic	*/
#define ETH_P_ATMMPOA	0x884c		/* MultiProtocol Over ATM	*/
#define ETH_P_LINK_CTL	0x886c		/* HPNA, wlan link local tunnel */
#define ETH_P_ATMFATE	0x8884		/* Frame-based ATM Transport
					 * over Ethernet
					 */
#define ETH_P_PAE	0x888E		/* Port Access Entity (IEEE 802.1X) */
#define ETH_P_AOE	0x88A2		/* ATA over Ethernet		*/
#define ETH_P_TIPC	0x88CA		/* TIPC 			*/
#define ETH_P_1588	0x88F7		/* IEEE 1588 Timesync */
#define ETH_P_FCOE	0x8906		/* Fibre Channel over Ethernet  */
#define ETH_P_FIP	0x8914		/* FCoE Initialization Protocol */
#define ETH_P_EDSA	0xDADA		/* Ethertype DSA [ NOT AN OFFICIALLY REGISTERED ID ] */

/*
 *	Non DIX types. Won't clash for 1500 types.
 */

#define ETH_P_802_3	0x0001		/* Dummy type for 802.3 frames  */
#define ETH_P_AX25	0x0002		/* Dummy protocol id for AX.25  */
#define ETH_P_ALL	0x0003		/* Every packet (be careful!!!) */
#define ETH_P_802_2	0x0004		/* 802.2 frames 		*/
#define ETH_P_SNAP	0x0005		/* Internal only		*/
#define ETH_P_DDCMP     0x0006          /* DEC DDCMP: Internal only     */
#define ETH_P_WAN_PPP   0x0007          /* Dummy type for WAN PPP frames*/
#define ETH_P_PPP_MP    0x0008          /* Dummy type for PPP MP frames */
#define ETH_P_LOCALTALK 0x0009		/* Localtalk pseudo type 	*/
#define ETH_P_CAN	0x000C		/* Controller Area Network      */
#define ETH_P_PPPTALK	0x0010		/* Dummy type for Atalk over PPP*/
#define ETH_P_TR_802_2	0x0011		/* 802.2 frames 		*/
#define ETH_P_MOBITEX	0x0015		/* Mobitex (kaz@cafe.net)	*/
#define ETH_P_CONTROL	0x0016		/* Card specific control frames */
#define ETH_P_IRDA	0x0017		/* Linux-IrDA			*/
#define ETH_P_ECONET	0x0018		/* Acorn Econet			*/
#define ETH_P_HDLC	0x0019		/* HDLC frames			*/
#define ETH_P_ARCNET	0x001A		/* 1A for ArcNet :-)            */
#define ETH_P_DSA	0x001B		/* Distributed Switch Arch.	*/
#define ETH_P_TRAILER	0x001C		/* Trailer switch tagging	*/
#define ETH_P_PHONET	0x00F5		/* Nokia Phonet frames          */
#define ETH_P_IEEE802154 0x00F6		/* IEEE802.15.4 frame		*/
#define ETH_P_CAIF	0x00F7		/* ST-Ericsson CAIF protocol	*/


//wmj+ ??
#ifdef __GNUC__
#define __packed		__attribute__((packed))
#endif

/*
 *	This is an Ethernet frame header.
 */

struct ethhdr {
	unsigned char	h_dest[ETH_ALEN];	/* destination eth addr	*/
	unsigned char	h_source[ETH_ALEN];	/* source ether addr	*/
	__be16		h_proto;		/* packet type ID field	*/
} __packed;


//#ifdef DEBUG
#if 0
#define LBS_DEB_LL(grp, grpnam, fmt, args...) \
do { if ((lbs_debug & (grp)) == (grp)) \
  printk(KERN_DEBUG DRV_NAME grpnam "%s: " fmt, \
         in_interrupt() ? " (INT)" : "", ## args); } while (0)
#else
#define LBS_DEB_LL(grp, grpnam, fmt, args...) do {} while (0)
#endif

#define lbs_deb_enter(grp) \
  LBS_DEB_LL(grp | LBS_DEB_ENTER, " enter", "%s()\n", __func__);
#define lbs_deb_enter_args(grp, fmt, args...) \
  LBS_DEB_LL(grp | LBS_DEB_ENTER, " enter", "%s(" fmt ")\n", __func__, ## args);
#define lbs_deb_leave(grp) \
  LBS_DEB_LL(grp | LBS_DEB_LEAVE, " leave", "%s()\n", __func__);
#define lbs_deb_leave_args(grp, fmt, args...) \
  LBS_DEB_LL(grp | LBS_DEB_LEAVE, " leave", "%s(), " fmt "\n", \
  __func__, ##args);
#define lbs_deb_main(fmt, args...)      LBS_DEB_LL(LBS_DEB_MAIN, " main", fmt, ##args)
#define lbs_deb_net(fmt, args...)       LBS_DEB_LL(LBS_DEB_NET, " net", fmt, ##args)
#define lbs_deb_mesh(fmt, args...)      LBS_DEB_LL(LBS_DEB_MESH, " mesh", fmt, ##args)
#define lbs_deb_wext(fmt, args...)      LBS_DEB_LL(LBS_DEB_WEXT, " wext", fmt, ##args)
#define lbs_deb_ioctl(fmt, args...)     LBS_DEB_LL(LBS_DEB_IOCTL, " ioctl", fmt, ##args)
#define lbs_deb_scan(fmt, args...)      LBS_DEB_LL(LBS_DEB_SCAN, " scan", fmt, ##args)
#define lbs_deb_assoc(fmt, args...)     LBS_DEB_LL(LBS_DEB_ASSOC, " assoc", fmt, ##args)
#define lbs_deb_join(fmt, args...)      LBS_DEB_LL(LBS_DEB_JOIN, " join", fmt, ##args)
#define lbs_deb_11d(fmt, args...)       LBS_DEB_LL(LBS_DEB_11D, " 11d", fmt, ##args)
#define lbs_deb_debugfs(fmt, args...)   LBS_DEB_LL(LBS_DEB_DEBUGFS, " debugfs", fmt, ##args)
#define lbs_deb_ethtool(fmt, args...)   LBS_DEB_LL(LBS_DEB_ETHTOOL, " ethtool", fmt, ##args)
#define lbs_deb_host(fmt, args...)      LBS_DEB_LL(LBS_DEB_HOST, " host", fmt, ##args)
#define lbs_deb_cmd(fmt, args...)       LBS_DEB_LL(LBS_DEB_CMD, " cmd", fmt, ##args)
#define lbs_deb_rx(fmt, args...)        LBS_DEB_LL(LBS_DEB_RX, " rx", fmt, ##args)
#define lbs_deb_tx(fmt, args...)        LBS_DEB_LL(LBS_DEB_TX, " tx", fmt, ##args)
#define lbs_deb_fw(fmt, args...)        LBS_DEB_LL(LBS_DEB_FW, " fw", fmt, ##args)
#define lbs_deb_usb(fmt, args...)       LBS_DEB_LL(LBS_DEB_USB, " usb", fmt, ##args)
#define lbs_deb_usbd(dev, fmt, args...) LBS_DEB_LL(LBS_DEB_USB, " usbd", "%s:" fmt, dev_name(dev), ##args)
#define lbs_deb_cs(fmt, args...)        LBS_DEB_LL(LBS_DEB_CS, " cs", fmt, ##args)
#define lbs_deb_thread(fmt, args...)    LBS_DEB_LL(LBS_DEB_THREAD, " thread", fmt, ##args)
#define lbs_deb_sdio(fmt, args...)      LBS_DEB_LL(LBS_DEB_SDIO, " sdio", fmt, ##args)
#define lbs_deb_sysfs(fmt, args...)     LBS_DEB_LL(LBS_DEB_SYSFS, " sysfs", fmt, ##args)
#define lbs_deb_spi(fmt, args...)       LBS_DEB_LL(LBS_DEB_SPI, " spi", fmt, ##args)
#define lbs_deb_cfg80211(fmt, args...)  LBS_DEB_LL(LBS_DEB_CFG80211, " cfg80211", fmt, ##args)


#define cpu_to_le16(v16) (v16)
#define cpu_to_le32(v32) (v32)
#define cpu_to_le64(v64) (v64)
#define le16_to_cpu(v16) (v16)
#define le32_to_cpu(v32) (v32)
#define le64_to_cpu(v64) (v64)


#define os_memset memset
#define os_memcpy memcpy
#define os_strlen strlen
#define os_memcmp memcmp
#define os_malloc  mem_malloc
#define os_free  mem_free


/* Buffer Constants */

/*	The size of SQ memory PPA, DPA are 8 DWORDs, that keep the physical
 *	addresses of TxPD buffers. Station has only 8 TxPD available, Whereas
 *	driver has more local TxPDs. Each TxPD on the host memory is associated
 *	with a Tx control node. The driver maintains 8 RxPD descriptors for
 *	station firmware to store Rx packet information.
 *
 *	Current version of MAC has a 32x6 multicast address buffer.
 *
 *	802.11b can have up to  14 channels, the driver keeps the
 *	BSSID(MAC address) of each APs or Ad hoc stations it has sensed.
 */

//#define MRVDRV_MAX_MULTICAST_LIST_SIZE	4
#define LBS_NUM_CMD_BUFFERS             3
#define LBS_CMD_BUFFER_SIZE             (1 * 512)
#define MRVDRV_MAX_CHANNEL_SIZE		14
#define MRVDRV_ASSOCIATION_TIME_OUT	255
#define MRVDRV_SNAP_HEADER_LEN          8


//lbs_hard_start_xmit  一次只会发送一个PBUF
#define	LBS_UPLD_SIZE			(PBUF_POOL_BUFSIZE + 64/*sizeof(struct txpd)*/)//2312
#define DEV_NAME_LEN			32

/* Wake criteria for HOST_SLEEP_CFG command */
#define EHS_WAKE_ON_BROADCAST_DATA	0x0001
#define EHS_WAKE_ON_UNICAST_DATA	0x0002
#define EHS_WAKE_ON_MAC_EVENT		0x0004
#define EHS_WAKE_ON_MULTICAST_DATA	0x0008
#define EHS_REMOVE_WAKEUP		0xFFFFFFFF
/* Wake rules for Host_Sleep_CFG command */
#define WOL_RULE_NET_TYPE_INFRA_OR_IBSS	0x00
#define WOL_RULE_NET_TYPE_MESH		0x10
#define WOL_RULE_ADDR_TYPE_BCAST	0x01
#define WOL_RULE_ADDR_TYPE_MCAST	0x08
#define WOL_RULE_ADDR_TYPE_UCAST	0x02
#define WOL_RULE_OP_AND			0x01
#define WOL_RULE_OP_OR			0x02
#define WOL_RULE_OP_INVALID		0xFF
#define WOL_RESULT_VALID_CMD		0
#define WOL_RESULT_NOSPC_ERR		1
#define WOL_RESULT_EEXIST_ERR		2

/* Misc constants */
/* This section defines 802.11 specific contants */

#define MRVDRV_MAX_BSS_DESCRIPTS		16
//#define MRVDRV_MAX_REGION_CODE			6

#define MRVDRV_DEFAULT_LISTEN_INTERVAL		10

#define	MRVDRV_CHANNELS_PER_SCAN		4
#define	MRVDRV_MAX_CHANNELS_PER_SCAN		14

#define MRVDRV_MIN_BEACON_INTERVAL		20
#define MRVDRV_MAX_BEACON_INTERVAL		1000
#define MRVDRV_BEACON_INTERVAL			100

#define MARVELL_MESH_IE_LENGTH		9

/*
 * Values used to populate the struct mrvl_mesh_ie.  The only time you need this
 * is when enabling the mesh using CMD_MESH_CONFIG.
 */
#define MARVELL_MESH_IE_TYPE		4
#define MARVELL_MESH_IE_SUBTYPE		0
#define MARVELL_MESH_IE_VERSION		0
#define MARVELL_MESH_PROTO_ID_HWMP	0
#define MARVELL_MESH_METRIC_ID		0
#define MARVELL_MESH_CAPABILITY		0

/* INT status Bit Definition */
#define MRVDRV_TX_DNLD_RDY		0x0001
#define MRVDRV_RX_UPLD_RDY		0x0002
#define MRVDRV_CMD_DNLD_RDY		0x0004
#define MRVDRV_CMD_UPLD_RDY		0x0008
#define MRVDRV_CARDEVENT		0x0010

/* Automatic TX control default levels */
#define POW_ADAPT_DEFAULT_P0 13
#define POW_ADAPT_DEFAULT_P1 15
#define POW_ADAPT_DEFAULT_P2 18
#define TPC_DEFAULT_P0 5
#define TPC_DEFAULT_P1 10
#define TPC_DEFAULT_P2 13

/* TxPD status */

/*
 *	Station firmware use TxPD status field to report final Tx transmit
 *	result, Bit masks are used to present combined situations.
 */

#define MRVDRV_TxPD_POWER_MGMT_NULL_PACKET 0x01
#define MRVDRV_TxPD_POWER_MGMT_LAST_PACKET 0x08

/* Tx mesh flag */
/*
 * Currently we are using normal WDS flag as mesh flag.
 * TODO: change to proper mesh flag when MAC understands it.
 */
#define TxPD_CONTROL_WDS_FRAME (1<<17)
#define TxPD_MESH_FRAME TxPD_CONTROL_WDS_FRAME

/* Mesh interface ID */
#define MESH_IFACE_ID					0x0001
/* Mesh id should be in bits 14-13-12 */
#define MESH_IFACE_BIT_OFFSET				0x000c
/* Mesh enable bit in FW capability */
#define MESH_CAPINFO_ENABLE_MASK			(1<<16)

/* FW definition from Marvell v4 */
#define MRVL_FW_V4					(0x04)
/* FW definition from Marvell v5 */
#define MRVL_FW_V5					(0x05)
/* FW definition from Marvell v10 */
#define MRVL_FW_V10					(0x0a)
/* FW major revision definition */
#define MRVL_FW_MAJOR_REV(x)				((x)>>24)

/* RxPD status */

#define MRVDRV_RXPD_STATUS_OK                0x0001

/* RxPD status - Received packet types */
/* Rx mesh flag */
/*
 * Currently we are using normal WDS flag as mesh flag.
 * TODO: change to proper mesh flag when MAC understands it.
 */
#define RxPD_CONTROL_WDS_FRAME (0x40)
#define RxPD_MESH_FRAME RxPD_CONTROL_WDS_FRAME

/* RSSI-related defines */
/*
 *	RSSI constants are used to implement 802.11 RSSI threshold
 *	indication. if the Rx packet signal got too weak for 5 consecutive
 *	times, miniport driver (driver) will report this event to wrapper
 */

#define MRVDRV_NF_DEFAULT_SCAN_VALUE		(-96)

/* RTS/FRAG related defines */
#define MRVDRV_RTS_MIN_VALUE		0
#define MRVDRV_RTS_MAX_VALUE		2347
#define MRVDRV_FRAG_MIN_VALUE		256
#define MRVDRV_FRAG_MAX_VALUE		2346

/* This is for firmware specific length */
#define EXTRA_LEN	36

//#define MRVDRV_ETH_TX_PACKET_BUFFER_SIZE \
//	(ETH_FRAME_LEN + sizeof(struct txpd) + EXTRA_LEN)

//#define MRVDRV_ETH_RX_PACKET_BUFFER_SIZE \
//	(ETH_FRAME_LEN + sizeof(struct rxpd) \
//	 + MRVDRV_SNAP_HEADER_LEN + EXTRA_LEN)

#define	CMD_F_HOSTCMD		(1 << 0)
#define FW_CAPINFO_WPA  	(1 << 0)
#define FW_CAPINFO_PS  		(1 << 1)
#define FW_CAPINFO_FIRMWARE_UPGRADE	(1 << 13)
#define FW_CAPINFO_BOOT2_UPGRADE	(1<<14)
#define FW_CAPINFO_PERSISTENT_CONFIG	(1<<15)

#define KEY_LEN_WPA_AES			16
#define KEY_LEN_WPA_TKIP		32
#define KEY_LEN_WEP_104			13
#define KEY_LEN_WEP_40			5

#define RF_ANTENNA_1		0x1
#define RF_ANTENNA_2		0x2
#define RF_ANTENNA_AUTO		0xFFFF

//#define	BAND_B			(0x01)
//#define	BAND_G			(0x02)
//#define ALL_802_11_BANDS	(BAND_B | BAND_G)

#define MAX_RATES			14

#define	MAX_LEDS			8

/* Global Variable Declaration */
extern const char lbs_driver_version[];
//extern const unsigned short lbs_region_code_to_index[MRVDRV_MAX_REGION_CODE];


struct firmware {
		u32 size;
		u8 *data;
};

/* ENUM definition */
/* SNRNF_TYPE */
enum SNRNF_TYPE {
	TYPE_BEACON = 0,
	TYPE_RXPD,
	MAX_TYPE_B
};

/* SNRNF_DATA */
enum SNRNF_DATA {
	TYPE_NOAVG = 0,
	TYPE_AVG,
	MAX_TYPE_AVG
};

/* LBS_802_11_POWER_MODE */
enum LBS_802_11_POWER_MODE {
	LBS802_11POWERMODECAM,
	LBS802_11POWERMODEMAX_PSP,
	LBS802_11POWERMODEFAST_PSP,
	/* not a real mode, defined as an upper bound */
	LBS802_11POWEMODEMAX
};

/* PS_STATE */
/*
enum PS_STATE {
	PS_STATE_FULL_POWER,
	PS_STATE_AWAKE,
	PS_STATE_PRE_SLEEP,
	PS_STATE_SLEEP
};
*/
/* DNLD_STATE */
enum DNLD_STATE {
	DNLD_RES_RECEIVED,
	DNLD_DATA_SENT,
	DNLD_CMD_SENT,
	DNLD_BOOTCMD_SENT,
};

/* LBS_MEDIA_STATE */
enum LBS_MEDIA_STATE {
	LBS_CONNECTED,
	LBS_CONNECTING,
	LBS_DISCONNECTED
};

/* LBS_802_11_PRIVACY_FILTER */
enum LBS_802_11_PRIVACY_FILTER {
	LBS802_11PRIVFILTERACCEPTALL,
	LBS802_11PRIVFILTER8021XWEP
};

/* mv_ms_type */
enum mv_ms_type {
	MVMS_DAT = 0,
	MVMS_CMD = 1,
	MVMS_TXDONE = 2,
	MVMS_EVENT
};

/* KEY_TYPE_ID */
/*
enum KEY_TYPE_ID {
	KEY_TYPE_ID_WEP = 0,
	KEY_TYPE_ID_TKIP,
	KEY_TYPE_ID_AES
};
*/

/* KEY_INFO_WPA (applies to both TKIP and AES/CCMP) */
enum KEY_INFO_WPA {
	KEY_INFO_WPA_MCAST = 0x01,
	KEY_INFO_WPA_UNICAST = 0x02,
	KEY_INFO_WPA_ENABLED = 0x04
};

/* Default values for fwt commands. */
#define FWT_DEFAULT_METRIC 0
#define FWT_DEFAULT_DIR 1
/* Default Rate, 11Mbps */
#define FWT_DEFAULT_RATE 3
#define FWT_DEFAULT_SSN 0xffffffff
#define FWT_DEFAULT_DSN 0
#define FWT_DEFAULT_HOPCOUNT 0
#define FWT_DEFAULT_TTL 0
#define FWT_DEFAULT_EXPIRATION 0
#define FWT_DEFAULT_SLEEPMODE 0
#define FWT_DEFAULT_SNR 0

/*
#ifdef FALSE
#undef FALSE
#endif
#ifdef TRUE
#undef TRUE
#endif		 */
typedef unsigned char Boolean;


#define WPA_CIPHER_NONE BIT(0)
#define WPA_CIPHER_WEP40 BIT(1)
#define WPA_CIPHER_WEP104 BIT(2)
#define WPA_CIPHER_TKIP BIT(3)
#define WPA_CIPHER_CCMP BIT(4)
#ifdef CONFIG_IEEE80211W
#define WPA_CIPHER_AES_128_CMAC BIT(5)
#endif /* CONFIG_IEEE80211W */

#define WPA_KEY_MGMT_IEEE8021X BIT(0)
#define WPA_KEY_MGMT_PSK BIT(1)
#define WPA_KEY_MGMT_NONE BIT(2)
#define WPA_KEY_MGMT_IEEE8021X_NO_WPA BIT(3)
#define WPA_KEY_MGMT_WPA_NONE BIT(4)
#define WPA_KEY_MGMT_FT_IEEE8021X BIT(5)
#define WPA_KEY_MGMT_FT_PSK BIT(6)
#define WPA_KEY_MGMT_IEEE8021X_SHA256 BIT(7)
#define WPA_KEY_MGMT_PSK_SHA256 BIT(8)
#define WPA_KEY_MGMT_WPS BIT(9)

#define wpa_key_mgmt_wpa_ieee8021x(akm)			  \
(													   \
		akm == WPA_KEY_MGMT_IEEE8021X ||			   \
		akm == WPA_KEY_MGMT_FT_IEEE8021X ||			   \
		akm == WPA_KEY_MGMT_IEEE8021X_SHA256		   \
)

#define wpa_key_mgmt_wpa_psk(akm)						\
(														\
		akm == WPA_KEY_MGMT_PSK ||						 \
		akm == WPA_KEY_MGMT_FT_PSK ||					 \
		akm == WPA_KEY_MGMT_PSK_SHA256					 \
)

#define wpa_key_mgmt_ft(akm)							\
(														\
		akm == WPA_KEY_MGMT_FT_PSK ||					\
		akm == WPA_KEY_MGMT_FT_IEEE8021X				\
)

#define wpa_key_mgmt_sha256(akm)					   \
(													   \
		akm == WPA_KEY_MGMT_PSK_SHA256 ||			   \
		akm == WPA_KEY_MGMT_IEEE8021X_SHA256		   \
)

#define WPA_PROTO_NONE 	0			//add by lck
#define WPA_PROTO_WPA BIT(0)
#define WPA_PROTO_RSN BIT(1)
#define WPA_PROTO_OPEN_SHARE	3	//add by lck

#define WPA_AUTH_ALG_OPEN BIT(0)
#define WPA_AUTH_ALG_SHARED BIT(1)
#define WPA_AUTH_ALG_LEAP BIT(2)
#define WPA_AUTH_ALG_FT BIT(3)


enum wpa_alg {
	WPA_ALG_NONE,
	WPA_ALG_WEP,
	WPA_ALG_TKIP,
	WPA_ALG_CCMP,
	WPA_ALG_IGTK,
	WPA_ALG_PMK
};

/**
 * enum wpa_cipher - Cipher suites
 */
enum wpa_cipher {
	CIPHER_NONE,
	CIPHER_WEP40,
	CIPHER_TKIP,
	CIPHER_CCMP,
	CIPHER_WEP104
};

/**
 * enum wpa_key_mgmt - Key management suites
 */
enum wpa_key_mgmt {
	KEY_MGMT_802_1X,
	KEY_MGMT_PSK,
	KEY_MGMT_NONE,
	KEY_MGMT_802_1X_NO_WPA,
	KEY_MGMT_WPA_NONE,
	KEY_MGMT_FT_802_1X,
	KEY_MGMT_FT_PSK,
	KEY_MGMT_802_1X_SHA256,
	KEY_MGMT_PSK_SHA256,
	KEY_MGMT_WPS
};

/**
 * enum wpa_states - wpa_supplicant state
 *
 * These enumeration values are used to indicate the current wpa_supplicant
 * state (wpa_s->wpa_state). The current state can be retrieved with
 * wpa_supplicant_get_state() function and the state can be changed by calling
 * wpa_supplicant_set_state(). In WPA state machine (wpa.c and preauth.c), the
 * wrapper functions wpa_sm_get_state() and wpa_sm_set_state() should be used
 * to access the state variable.
 */
enum wpa_states {
	/**
	 * WPA_DISCONNECTED - Disconnected state
	 *
	 * This state indicates that client is not associated, but is likely to
	 * start looking for an access point. This state is entered when a
	 * connection is lost.
	 */
	WPA_DISCONNECTED,

	/**
	 * WPA_INACTIVE - Inactive state (wpa_supplicant disabled)
	 *
	 * This state is entered if there are no enabled networks in the
	 * configuration. wpa_supplicant is not trying to associate with a new
	 * network and external interaction (e.g., ctrl_iface call to add or
	 * enable a network) is needed to start association.
	 */
	WPA_INACTIVE,

	/**
	 * WPA_SCANNING - Scanning for a network
	 *
	 * This state is entered when wpa_supplicant starts scanning for a
	 * network.
	 */
	WPA_SCANNING,

	/**
	 * WPA_AUTHENTICATING - Trying to authenticate with a BSS/SSID
	 *
	 * This state is entered when wpa_supplicant has found a suitable BSS
	 * to authenticate with and the driver is configured to try to
	 * authenticate with this BSS. This state is used only with drivers
	 * that use wpa_supplicant as the SME.
	 */
	WPA_AUTHENTICATING,

	/**
	 * WPA_ASSOCIATING - Trying to associate with a BSS/SSID
	 *
	 * This state is entered when wpa_supplicant has found a suitable BSS
	 * to associate with and the driver is configured to try to associate
	 * with this BSS in ap_scan=1 mode. When using ap_scan=2 mode, this
	 * state is entered when the driver is configured to try to associate
	 * with a network using the configured SSID and security policy.
	 */
	WPA_ASSOCIATING,

	/**
	 * WPA_ASSOCIATED - Association completed
	 *
	 * This state is entered when the driver reports that association has
	 * been successfully completed with an AP. If IEEE 802.1X is used
	 * (with or without WPA/WPA2), wpa_supplicant remains in this state
	 * until the IEEE 802.1X/EAPOL authentication has been completed.
	 */
	WPA_ASSOCIATED,

	/**
	 * WPA_4WAY_HANDSHAKE - WPA 4-Way Key Handshake in progress
	 *
	 * This state is entered when WPA/WPA2 4-Way Handshake is started. In
	 * case of WPA-PSK, this happens when receiving the first EAPOL-Key
	 * frame after association. In case of WPA-EAP, this state is entered
	 * when the IEEE 802.1X/EAPOL authentication has been completed.
	 */
	WPA_4WAY_HANDSHAKE,

	/**
	 * WPA_GROUP_HANDSHAKE - WPA Group Key Handshake in progress
	 *
	 * This state is entered when 4-Way Key Handshake has been completed
	 * (i.e., when the supplicant sends out message 4/4) and when Group
	 * Key rekeying is started by the AP (i.e., when supplicant receives
	 * message 1/2).
	 */
	WPA_GROUP_HANDSHAKE,

	/**
	 * WPA_COMPLETED - All authentication completed
	 *
	 * This state is entered when the full authentication process is
	 * completed. In case of WPA2, this happens when the 4-Way Handshake is
	 * successfully completed. With WPA, this state is entered after the
	 * Group Key Handshake; with IEEE 802.1X (non-WPA) connection is
	 * completed after dynamic keys are received (or if not used, after
	 * the EAP authentication has been completed). With static WEP keys and
	 * plaintext connections, this state is entered when an association
	 * has been completed.
	 *
	 * This state indicates that the supplicant has completed its
	 * processing for the association phase and that data connection is
	 * fully configured.
	 */
	WPA_COMPLETED
};

#define MLME_SETPROTECTION_PROTECT_TYPE_NONE 0
#define MLME_SETPROTECTION_PROTECT_TYPE_RX 1
#define MLME_SETPROTECTION_PROTECT_TYPE_TX 2
#define MLME_SETPROTECTION_PROTECT_TYPE_RX_TX 3

#define MLME_SETPROTECTION_KEY_TYPE_GROUP 0
#define MLME_SETPROTECTION_KEY_TYPE_PAIRWISE 1


/**
 * enum mfp_options - Management frame protection (IEEE 802.11w) options
 */
enum mfp_options {
	NO_MGMT_FRAME_PROTECTION = 0,
	MGMT_FRAME_PROTECTION_OPTIONAL = 1,
	MGMT_FRAME_PROTECTION_REQUIRED = 2
};

/**
 * enum hostapd_hw_mode - Hardware mode
 */
enum hostapd_hw_mode {
	HOSTAPD_MODE_IEEE80211B,
	HOSTAPD_MODE_IEEE80211G,
	HOSTAPD_MODE_IEEE80211A,
	NUM_HOSTAPD_MODES
};

#endif
