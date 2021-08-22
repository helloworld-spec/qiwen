/*
 * Anyka MAC Fast Ethernet driver for Linux.
 * Features
 * Copyright (C) 2010 ANYKA
 * AUTHOR Tang Anyang
 * AUTHOR Zhang Jingyuan
 * 10-11-01 09:08:08
 */
#include <common.h>
#include <command.h>
#include <config.h>
#include <malloc.h>
#include <net.h>
#include "eth_ops.h"
#include "Ethernethw.h"
#include <asm/arch/anyka_cpu.h>
#include "phyhw.h"
#include <asm/io.h>
#include <asm/arch/module_reset.h>
#include <linux/netdevice.h>
#include <asm/arch-ak39/anyka_cpu.h>
#include <asm/cache.h>
#include <asm/dma-mapping.h>
#include <errno.h>  // cdh:add

/* Use power-down feature of the chip */
#define POWER_DOWN	0
#define NO_AUTOPROBE
#define SMC_DEBUG 0
#define AK_VA_L2CTRL (0x20140000)
#define psysbase (0x08000000)
#define SYSCONTROL_BASE 0x08000000
#define pMacBase	(0x20300000)
#define TPD_RING_SIZE 0x50
#define RFD_RING_SIZE 0x50
#define RRD_RING_SIZE 0x50

#if SMC_DEBUG > 1
static const char version[] =
	"smc91111.c:v1.0 04/25/01 by Daris A Nevil (dnevil@snmc.com)\n";
#endif

/* Autonegotiation timeout in seconds */
#ifndef CONFIG_SMC_AUTONEG_TIMEOUT
#define CONFIG_SMC_AUTONEG_TIMEOUT 10
#endif


/*
* Wait time for memory to be free.  This probably shouldn't be
* tuned that much, as waiting for this means nothing else happens
* in the system
*/
#define MEMORY_WAIT_TIME 16


#if (SMC_DEBUG > 2 )
#define PRINTK3(args...) printf(args)
#else
#define PRINTK3(args...)
#endif

#if SMC_DEBUG > 1
#define PRINTK2(args...) printf(args)
#else
#define PRINTK2(args...)
#endif

#ifdef SMC_DEBUG
#define PRINTK(args...) printf(args)
#else
#define PRINTK(args...)
#endif


#define SMC_DEV_NAME "AKEthernet"
#define SMC_PHY_ADDR 0x0000
#define SMC_ALLOC_MAX_TRY 5
#define SMC_TX_TIMEOUT 30

#define SMC_PHY_CLOCK_DELAY 1000

#define ETH_ZLEN 60

#ifdef	CONFIG_SMC_USE_32_BIT
#define USE_32_BIT  1
#else
#undef USE_32_BIT
#endif

#define AK_PA_SYSCTRL		(0x08000000)
#define AK_PA_GPIOCTRL		(0x20170000)

/* rrd format */
typedef struct _RrdDescr_s {

	unsigned short  xsum;           /*  */

	unsigned short  nor     :4  ;   /* number of RFD */
	unsigned short  si      :12 ;   /* start index of rfd-ring */

	unsigned short  hash;           /* rss(MSFT) hash value */

	unsigned short  hash1;  

	unsigned short  vidh    :4  ;   /* vlan-id high part */
	unsigned short  cfi     :1  ;   /* vlan-cfi */ 
	unsigned short  pri     :3  ;   /* vlan-priority */
	unsigned short  vidl    :8  ;   /* vlan-id low part */
	unsigned char   hdr_len;        /* Header Length of Header-Data Split. unsigned short unit */
	unsigned char   hds_typ :2  ;   /* Header-Data Split Type, 
									00:no split, 
									01:split at upper layer protocol header
									10:split at upper layer payload */
	unsigned char   rss_cpu :2  ;   /* CPU number used by RSS */
	unsigned char   hash_t6 :1  ;   /* TCP(IPv6) flag for RSS hash algrithm */
	unsigned char   hash_i6 :1  ;   /* IPv6 flag for RSS hash algrithm */
	unsigned char   hash_t4 :1  ;   /* TCP(IPv4)  flag for RSS hash algrithm */
	unsigned char   hash_i4 :1  ;   /* IPv4 flag for RSS hash algrithm */

	unsigned short  frm_len :14 ;   /* frame length of the packet */        
	unsigned short  l4f     :1  ;   /* L4(TCP/UDP) checksum failed */
	unsigned short  ipf     :1  ;   /* IP checksum failed */
	unsigned short  vtag    :1  ;   /* vlan tag */
	unsigned short  pid     :3  ;   /* protocol id,
										  000: non-ip packet
										  001: ipv4(only)
										  011: tcp/ipv4
										  101: udp/ipv4
										  010: tcp/ipv6
										  100: udp/ipv6
										  110: ipv6(only) */
	unsigned short  res     :1  ;   /* received error summary */
	unsigned short  crc     :1  ;   /* crc error */
	unsigned short  fae     :1  ;   /* frame alignment error */
	unsigned short  trunc   :1  ;   /* truncated packet, larger than MTU */
	unsigned short  runt    :1  ;   /* runt packet */
	unsigned short  icmp    :1  ;   /* incomplete packet, due to insufficient rx-descriptor */
	unsigned short  bar     :1  ;   /* broadcast address received */
	unsigned short  mar     :1  ;   /* multicast address received */
	unsigned short  typ     :1  ;   /* type of packet (ethernet_ii(1) or snap(0)) */
	unsigned short  resv1   :2  ;   /* reserved, must be 0 */
	unsigned short  updt    :1  ;   /* update by hardware. after hw fulfill the buffer, this bit 
									  should be 1 */
} RrdDescr_t, *PRrdDescr_t;

unsigned long g_tpdconsumerindex = 0;
unsigned long g_rfdconsumerindex = 0;
unsigned long g_rrdconsumerindex = 0;
bool g_update = false;
unsigned long tpdaddress = 0;  /* physical address for tpd */
unsigned long tpdaddressVa = 0; /* virtual address for tpd */
unsigned long tpdbufaddressVa = 0; /* virtual address for tpd buffer */
dma_addr_t tpdbufaddressPa = 0; /* physical address for tpd buffer */
unsigned long rrdaddressVa = 0; /* virtual address for rrd */
void *rfd_sequenceva = NULL; /* virtual address for rfd */
dma_addr_t rfd_sequence; /* physical address for rfd */
void *RingbufVa = NULL; /* virtual address for ring buf */
dma_addr_t RingbufPa; /* physical address for ring buf */
void *rfdbaseva = NULL;
dma_addr_t rfdbasepa;

/* if it is set to 1, interrupt is available for all the descriptors or else interrupt is available only for  */ 
#define MODULO_INTERRUPT   1 
#define DEFAULT_DELAY_VARIABLE  1000
#define DEFAULT_LOOP_VARIABLE   10000

/*SynopGMAC can support up to 32 phys*/
enum GMACPhyBase
{
    PHY0  = 0, //The device can support 32 phys, but we use first phy only
    PHY1  = 1,
    PHY31 = 31,
};

#define DEFAULT_PHY_BASE PHY1 //We use First Phy 
#define MACBASE 0x0000 // The Mac Base address offset is 0x0000
#define DMABASE 0x1000 // Dma base address starts with an offset 0x1000

#ifdef AVB_SUPPORT
#define DMABASE_CH0 DMABASE // DMA base address for Channel 0
#define DMABASE_CH1 0x1100 // DMA base address for Channel 1
#define DMABASE_CH2 0x1200 // DMA base address for Channel 2

#define ETHERNET_HEADER_AVB     18	//6 byte Dest addr, 6 byte Src addr, 2 byte length/type

#endif

#define NET_IF_TIMEOUT (10*HZ)
#define CHECK_TIME (HZ)
#define SA_SHIRQ IRQF_SHARED
#define SA_INTERRUPT IRQF_DISABLED
#define CHECKSUM_HW CHECKSUM_PARTIAL

#define TRANSMIT_DESC_SIZE  512 //64 //256 //1500 //1400 // 256	// 32 Tx Descriptors needed in the Descriptor pool/queue
#define RECEIVE_DESC_SIZE   512 //64 //256 // 1500 // 1400 // 256	// 32 Rx Descriptors needed in the Descriptor pool/queue

#define ETHERNET_HEADER         14 //6 byte Dest addr, 6 byte Src addr, 2 byte length/type
#define ETHERNET_CRC            4  //Ethernet CRC
#define ETHERNET_EXTRA          2  //Only God knows about this?????
#define ETHERNET_PACKET_COPY    250 // Maximum length when received data is copied on to a new skb  
#define ETHERNET_PACKET_EXTRA   18  // Preallocated length for the rx packets is MTU + ETHERNET_PACKET_EXTRA  
#define VLAN_TAG                4   //optional 802.1q VLAN Tag
#define MIN_ETHERNET_PAYLOAD    46  //Minimum Ethernet payload size
#define MAX_ETHERNET_PAYLOAD    1500 //Maximum Ethernet payload size
#define JUMBO_FRAME_PAYLOAD     9000 //Jumbo frame payload size

#define TX_BUF_SIZE      ETHERNET_HEADER + ETHERNET_CRC + MAX_ETHERNET_PAYLOAD + VLAN_TAG

#if 0
#define IOCTL_READ_REGISTER  SIOCDEVPRIVATE+1
#define IOCTL_WRITE_REGISTER SIOCDEVPRIVATE+2
#define IOCTL_READ_IPSTRUCT  SIOCDEVPRIVATE+3
#define IOCTL_READ_RXDESC    SIOCDEVPRIVATE+4
#define IOCTL_READ_TXDESC    SIOCDEVPRIVATE+5
#define IOCTL_POWER_DOWN     SIOCDEVPRIVATE+6

#define IOCTL_AVB_TEST     SIOCDEVPRIVATE+7
#endif

#define AVB_SET_CONFIG     0x00000001
#define AVB_CONFIG_HW      0x00000002
#define AVB_RUN_TEST       0x00000003
#define AVB_GET_RESULT     0x00000004
#define AVB_DEBUG_ENABLE   0x00000005
#define AVB_DEBUG_DISABLE  0x00000006
#define AVB_TX_FRAMES      0x00000007

/* Error Codes */
#define ESYNOPGMACNOERR   0
#define ESYNOPGMACNOMEM   1
#define ESYNOPGMACPHYERR  2
#define ESYNOPGMACBUSY    3

// This is the IP's phy address. This is unique address for every MAC in the universe
#define DEFAULT_MAC_ADDRESS {0x00, 0x55, 0x7B, 0xB5, 0x7D, 0xF7}


// cdh:add, check ok, ignore type
#ifdef ENH_DESC_8W
typedef struct DmaDescStruct
{
    u32   status;         /* Status */
    u32   length;         /* Buffer 1  and Buffer 2 length */
    u32   buffer1;        /* Network Buffer 1 pointer (Dma-able) */
    u32   buffer2;        /* Network Buffer 2 pointer or next descriptor pointer (Dma-able)in chain structure */
    /* This data below is used only by driver */
    u32   extstatus;      /* Extended status of a Rx Descriptor */
    u32   reserved1;      /* Reserved word */
    u32   timestamplow;   /* Lower 32 bits of the 64 bit timestamp value */
    u32   timestamphigh;  /* Higher 32 bits of the 64 bit timestamp value */
    u32   data1;          /* This holds virtual address of buffer1, not used by DMA */
    u32   data2;          /* This holds virtual address of buffer2, not used by DMA */
} DmaDesc;
#else
typedef struct DmaDescStruct
{
    u32   status;         /* Status */
    u32   length;         /* Buffer 1  and Buffer 2 length */
    u32   buffer1;        /* Network Buffer 1 pointer (Dma-able) */
    u32   buffer2;        /* Network Buffer 2 pointer or next descriptor pointer (Dma-able)in chain structure 	*/
    /* This data below is used only by driver */
    u32   data1;          /* This holds virtual address of buffer1, not used by DMA */
    u32   data2;          /* This holds virtual address of buffer2, not used by DMA */
} DmaDesc;
#endif

enum DescMode
{
    RINGMODE	= 0x00000001,
    CHAINMODE	= 0x00000002,
};

enum BufferMode
{
    SINGLEBUF	= 0x00000001,
    DUALBUF		= 0x00000002,
};

/* synopGMAC device data */
// cdh:add, check ok, ignore type
typedef struct synopGMACDeviceStruct
{
    u32 MacBase;			/* base address of MAC registers           */
    u32 DmaBase;			/* base address of DMA registers           */
    u32 PhyBase;			/* PHY device address on MII interface     */
    u32 Version;			/* Gmac Revision version	               */

    dma_addr_t TxDescDma;	/* Dma-able address of first tx descriptor either in ring or chain mode, this is used by the GMAC device*/
    dma_addr_t RxDescDma;	/* Dma-albe address of first rx descriptor either in ring or chain mode, this is used by the GMAC device*/
    DmaDesc *TxDesc;		/* start address of TX descriptors ring or chain, this is used by the driver  */
    DmaDesc *RxDesc;		/* start address of RX descriptors ring or chain, this is used by the driver  */
	DmaDesc *rx_ring[RECEIVE_DESC_SIZE]; // cdh:add
	DmaDesc *tx_ring[TRANSMIT_DESC_SIZE]; // cdh:add
	
    u32 BusyTxDesc;			/* Number of Tx Descriptors owned by DMA at any given time*/
    u32 BusyRxDesc;			/* Number of Rx Descriptors owned by DMA at any given time*/

    u32  RxDescCount;              /* number of rx descriptors in the tx descriptor queue/pool */
    u32  TxDescCount;              /* number of tx descriptors in the rx descriptor queue/pool */

    u32  TxBusy;                   /* index of the tx descriptor owned by DMA, is obtained by gmac_get_tx_qptr()                */
    u32  TxNext;                   /* index of the tx descriptor next available with driver, given to DMA by gmac_set_tx_qptr() */
    u32  RxBusy;                   /* index of the rx descriptor owned by DMA, obtained by gmac_get_rx_qptr()                   */
    u32  RxNext;                   /* index of the rx descriptor next available with driver, given to DMA by gmac_set_rx_qptr() */

    DmaDesc *TxBusyDesc;           /* Tx Descriptor address corresponding to the index TxBusy */
    DmaDesc *TxNextDesc;           /* Tx Descriptor address corresponding to the index TxNext */
    DmaDesc *RxBusyDesc;           /* Rx Descriptor address corresponding to the index TxBusy */
    DmaDesc *RxNextDesc;           /* Rx Descriptor address corresponding to the index RxNext */


    /*Phy related stuff*/
    u32 ClockDivMdc;	/* Clock divider value programmed in the hardware	*/
    /* The status of the link */
    u32 LinkState;		/* Link status as reported by the Marvel Phy		*/
    u32 DuplexMode;		/* Duplex mode of the Phy		*/
    u32 Speed;			/* Speed of the Phy				*/
    u32 LoopBackMode;	/* Loopback status of the Phy	*/
	u32	rx_tmp_len;			/* Gmac ctrl interrupt bit , cdh add */
} gmac_device;

// cdh:add, check ok, ignore type
#ifdef AVB_SUPPORT
typedef struct AVBStruct
{
    u8 ChSelMask;             /* This gives which DMA channel is enabled and which is disabled
                                Bit0 for Ch0
                                Bit1 for Ch1
                                Bit2 for Ch2
                              */
    u8 DurationOfExp;         /* Duration for which experiment should be conducted in minutes - Default 2 Minutes */

    u8 AvControlCh;           /* channel on which AV control channel must be received (Not used)*/
    u8 PTPCh;                 /* Channel on which PTP packets must be received (Not Used)*/
    u8 PrioTagForAV;          /* Used when more than One channel enabled in Rx path (Not Used)
                                for only CH1 Enabled:
                                Frames sith Priority > Value programmed, frames sent to CH1
                                Frames with priority < Value programmed are sent to CH0
                                For both CH1 and CH2 Enabled:
                                Frames sith Priority > Value programmed, frames sent to CH2
                                Frames with priority < Value programmed are sent to CH1
                              */

    u16 AvType;                /* AV Ethernet Type to be programmed for Core to identify AV type */

    u8 Ch1PrioWts;
    u8 Ch1Bw;
    u32 Ch1_frame_size;
    u8 Ch1_EnableSlotCheck;    /* Enable checking of slot numbers programmed in the Tx Desc*/
    u8 Ch1_AdvSlotInterval;    /* When Set Data fetched for current slot and for next 2 slots in advance
                                 When reset data fetched for current slot and in advance for next slot*/

    u8  Ch1CrSh;        /* When set Enables the credit based traffic shaping. Now works with Strict priority style*/
    u8  Ch1SlotCount;          /* Over which transmiteed bits per slot needs to be computed (Only for Credit based shaping) */
    u32 Ch1AvgBitsPerSlot;     /* Average bits per slot reported by core once in Ch1SlotCount * 125 micro seconds */
    u32 Ch1AvgBitsPerSlotAccL;  /* No of Avg Bits per slot on Channel1*/
    u32 Ch1AvgBitsPerSlotAccH;  /* No of Avg Bits per slot on Channel1*/
    u32 Ch1AvgBitsNoOfInterrupts;  /* Total Number of interrupts over which AvbBits are accumulated*/

    u8  Ch1CreditControl;      /* Will be zero (Not used) */

    u8 Ch1_tx_rx_prio_policy;   // Should Ch1 use Strict or RR policy
    u8 Ch1_use_tx_high_prio;    // Should Ch1 Tx have High priority over Rx
    u8 Ch1_tx_rx_prio_ratio;    // For Round Robin what is the ratio between tx-rx or rx-tx

    u8 Ch1_tx_desc_slot_no_start;
    u8 Ch1_tx_desc_slot_no_skip;

    u32 Ch1SendSlope;
    u32 Ch1IdleSlope;
    u32 Ch1HiCredit;
    u32 Ch1LoCredit;

    u32 Ch1FramecountTx;         /* No of Frames Transmitted on Channel 1 */
    u32 Ch1FramecountRx;         /* No of Frames Received on Channel 1 */

    u8 Ch2PrioWts;
    u8 Ch2Bw;
    u32 Ch2_frame_size;
    u8 Ch2_EnableSlotCheck;    /* Enable checking of slot numbers programmed in the Tx Desc*/
    u8 Ch2_AdvSlotInterval;    /* When Set Data fetched for current slot and for next 2 slots in advance
                                 When reset data fetched for current slot and in advance for next slot*/
    u8  Ch2CrSh;        /* When set Enables the credit based traffic shaping. Now works with Strict priority style*/
    u8  Ch2SlotCount;          /* Over which transmiteed bits per slot needs to be computed (Only for Credit based shaping) */
    u32 Ch2AvgBitsPerSlot;     /* Average bits per slot reported by core once in Ch2SlotCount * 125 micro seconds */
    u32 Ch2AvgBitsPerSlotAccL;  /* No of Avg Bits per slot on Channel2*/
    u32 Ch2AvgBitsPerSlotAccH;  /* No of Avg Bits per slot on Channel2*/
    u32 Ch2AvgBitsNoOfInterrupts; /* Total Number of interrupts over which AvbBits are accumulated*/

    u8  Ch2CreditControl;      /* Will be zero at present*/

    u8 Ch2_tx_rx_prio_policy;   // Should Ch1 use Strict or RR policy
    u8 Ch2_use_tx_high_prio;    // Should Ch1 Tx have High priority over Rx
    u8 Ch2_tx_rx_prio_ratio;    // For Round Robin what is the ratio between tx-rx or rx-tx


    u8 Ch2_tx_desc_slot_no_start;
    u8 Ch2_tx_desc_slot_no_skip;

    u32 Ch2SendSlope;
    u32 Ch2IdleSlope;
    u32 Ch2HiCredit;
    u32 Ch2LoCredit;

    u32 Ch2FramecountTx;         /* No of Frames Transmitted on Channel 2 */
    u32 Ch2FramecountRx;         /* No of Frames Received on Channel 2 */

    u8 Ch0PrioWts;
    u8 Ch0_tx_rx_prio_policy;   // Should Ch1 use Strict or RR policy
    u8 Ch0_use_tx_high_prio;    // Should Ch1 Tx have High priority over Rx
    u8 Ch0_tx_rx_prio_ratio;    // For Round Robin what is the ratio between tx-rx or rx-tx

    u32 Ch0_frame_size;
    u32 Ch0FramecountTx;         /* No of Frames Transmitted on Channel 0 */
    u32 Ch0FramecountRx;         /* No of Frames Received on Channel 0 */

} gmac_avb;
#endif


/* Structure/enum declaration ------------------------------- */
typedef struct mac_info {
	void __iomem	*io_addr;	/* Register I/O base address */
	u16		 irq;		/* IRQ */

	u16		tx_pkt_cnt;
	u16		queue_pkt_len;
	u16		queue_start_addr;
	u16		queue_ip_summed;
	u16		dbug_cnt;
	u8		io_mode;		/* 0:word, 2:byte */
	u8		phy_addr;
	u8		imr_all;

	unsigned int	flags;
	unsigned int	in_suspend :1;

	void (*inblk)(void __iomem *port, void *data, int length);
	void (*outblk)(void __iomem *port, void *data, int length);
	void (*dumpblk)(void __iomem *port, int length);

	struct device	*dev;	     /* parent device */

	struct resource	*addr_res;   /* resources found */
	struct resource	*addr_req;   /* resources requested */
	struct resource *irq_res;

	u32		msg_enable;

	int		rx_csum;
	int		can_csum;
	int		ip_summed;
	int		phy_id;
	struct clk	*clk;
} mac_info_t;

#define MAX_ADDR_LEN	32		/* Largest hardware address length */
#define MAC_ADDR_LEN 6


// cdh:add, check ok, ignore type
typedef struct synopGMACAdapterStruct
{
    /*Device Dependent Data structur*/
    gmac_device *gmacdev_pt;

    u8 dev_num;
    unsigned char	dev_addr[MAC_ADDR_LEN];
    unsigned char	broadcast[MAX_ADDR_LEN];	
    struct eth_device *netdev_pt;
    struct net_device_stats net_dev_stats;
    u32 pcistate[16];

} nt_adapter;

/* Below is "88E1011/88E1011S Integrated 10/100/1000 Gigabit Ethernet Transceiver"
 * Register and their layouts. This Phy has been used in the Dot Aster GMAC Phy daughter.
 * Since the Phy register map is standard, this map hardly changes to a different Ppy
 */
enum MiiRegisters
{
    PHY_CONTROL_REG           = 0x0000,		/*Control Register*/
    PHY_STATUS_REG            = 0x0001,		/*Status Register */
    PHY_ID_HI_REG             = 0x0002,		/*PHY Identifier High Register*/
    PHY_ID_LOW_REG            = 0x0003,		/*PHY Identifier High Register*/
    PHY_AN_ADV_REG            = 0x0004,		/*Auto-Negotiation Advertisement Register*/
    PHY_LNK_PART_ABl_REG      = 0x0005,		/*Link Partner Ability Register (Base Page)*/
    PHY_AN_EXP_REG            = 0x0006,		/*Auto-Negotiation Expansion Register*/
    PHY_AN_NXT_PAGE_TX_REG    = 0x0007,		/*Next Page Transmit Register*/
    PHY_LNK_PART_NXT_PAGE_REG = 0x0008,		/*Link Partner Next Page Register*/
    PHY_1000BT_CTRL_REG       = 0x0009,		/*1000BASE-T Control Register*/
    PHY_1000BT_STATUS_REG     = 0x000a,		/*1000BASE-T Status Register*/
    PHY_SPECIFIC_CTRL_REG     = 0x0010,		/*Phy specific control register*/
    PHY_SPECIFIC_STATUS_REG   = 0x0011,		/*Phy specific status register*/
    PHY_INTERRUPT_ENABLE_REG  = 0x0012,		/*Phy interrupt enable register*/
    PHY_INTERRUPT_STATUS_REG  = 0x0013,		/*Phy interrupt status register*/
    PHY_EXT_PHY_SPC_CTRL	  = 0x0014,		/*Extended Phy specific control*/
    PHY_RX_ERR_COUNTER	    = 0x0015,		/*Receive Error Counter*/
    PHY_EXT_ADDR_CBL_DIAG     = 0x0016,		/*Extended address for cable diagnostic register*/
    PHY_LED_CONTROL	          = 0x0018,		/*LED Control*/
    PHY_MAN_LED_OVERIDE       = 0x0019,		/*Manual LED override register*/
    PHY_EXT_PHY_SPC_CTRL2     = 0x001a,		/*Extended Phy specific control 2*/
    PHY_EXT_PHY_SPC_STATUS    = 0x001b,		/*Extended Phy specific status*/
    PHY_CBL_DIAG_REG	      = 0x001c,		/*Cable diagnostic registers*/
    RTL8201_PAGE_SELECT       = 31,
    RTL8201_P7_R16            = 16,
};


/* This is Control register layout. Control register is of 16 bit wide.
*/
enum Mii_GEN_CTRL
{
    /*	Description	               bits	       R/W	default value  */
    Mii_reset		 = 0x8000,
    Mii_Speed_10  	 = 0x0000,   /* 10   Mbps                    6:13   RW */
    Mii_Speed_100  	 = 0x2000,   /* 100  Mbps                    6:13   RW */
    Mii_Speed_1000	 = 0x0040,   /* 1000 Mbit/s                  6:13   RW */

    Mii_Duplex    	 = 0x0100,   /* Full Duplex mode             8      RW */
    Mii_AN_restart   = 0x0200,   /* Autonegotiation restart      9      RW */

    Mii_Manual_Master_Config = 0x0800,/* Manual Master Config    11     RW */

    Mii_AN_En        = 0x1000,   /* Autonegotiation enable       12     RW */

    Mii_Loopback   	 = 0x4000,   /* Enable Loop back             14     RW */
    Mii_NoLoopback 	 = 0x0000,   /* Enable Loop back             14     RW */
};

/* This is Status register layout. Status register is of 16 bit wide.
*/
enum Mii_GEN_STATUS
{
    Mii_AutoNegCmplt     = 0x0020,   /* Autonegotiation completed      5    RW */
    Mii_RemoteFault      = 0x0010,   /* Remote fault                   4    RW */
    Mii_AN_Ability       = 0x0008,   /* Autonegotiation ability        3    RW */
    Mii_Link             = 0x0004,   /* Link status                    2    RW */
};

enum Mii_Phy_Status
{
    Mii_phy_status_speed_10	 	= 0x0000,
    Mii_phy_status_speed_100  	= 0x4000,
    Mii_phy_status_speed_1000	= 0x8000,

    Mii_phy_status_full_duplex	= 0x0100, // cdh:bit 8, old,13
    Mii_phy_status_half_duplex	= 0x0000,

    Mii_phy_status_link_up		= 0x0400,
};

enum Mii_Link_Status
{
    LINKDOWN	= 0,
    LINKUP		= 1,
};

enum Mii_Duplex_Mode
{
    HALFDUPLEX = 1,
    FULLDUPLEX = 2,
};
enum Mii_Link_Speed
{
    SPEED10     = 1,
    SPEED100    = 2,
    SPEED1000   = 3,
};

enum Mii_Loop_Back
{
    NOLOOPBACK  = 0,
    LOOPBACK    = 1,
};



/**********************************************************
 * GMAC registers Map
 * For Pci based system address is BARx + GmacRegisterBase
 * For any other system translation is done accordingly
 **********************************************************/
enum GmacRegisters
{
    GmacConfig     	      = 0x0000,    /* Mac config Register                       */
    GmacFrameFilter  	  = 0x0004,    /* Mac frame filtering controls              */
    GmacHashHigh     	  = 0x0008,    /* Multi-cast hash table high                */
    GmacHashLow      	  = 0x000C,    /* Multi-cast hash table low                 */
    GmacGmiiAddr     	  = 0x0010,    /* GMII address Register(ext. Phy)           */
    GmacGmiiData     	  = 0x0014,    /* GMII data Register(ext. Phy)              */
    GmacFlowControl  	  = 0x0018,    /* Flow control Register                     */
    GmacVlan         	  = 0x001C,    /* VLAN tag Register (IEEE 802.1Q)           */

    GmacVersion     	  = 0x0020,    /* GMAC Core Version Register                */
    GmacWakeupAddr  	  = 0x0028,    /* GMAC wake-up frame filter adrress reg     */
    GmacPmtCtrlStatus  	  = 0x002C,    /* PMT control and status register           */

#ifdef LPI_SUPPORT
    GmacLPICtrlSts      = 0x0030,    /* LPI (low power idle) Control and Status Register */
    GmacLPITimerCtrl    = 0x0034,    /* LPI timer control register               */
#endif

    GmacInterruptStatus	  = 0x0038,    /* Mac Interrupt ststus register	       */
    GmacInterruptMask     = 0x003C,    /* Mac Interrupt Mask register	       */

    GmacAddr0High    	  = 0x0040,    /* Mac address0 high Register                */
    GmacAddr0Low    	  = 0x0044,    /* Mac address0 low Register                 */
    GmacAddr1High    	  = 0x0048,    /* Mac address1 high Register                */
    GmacAddr1Low     	  = 0x004C,    /* Mac address1 low Register                 */
    GmacAddr2High   	  = 0x0050,    /* Mac address2 high Register                */
    GmacAddr2Low     	  = 0x0054,    /* Mac address2 low Register                 */
    GmacAddr3High    	  = 0x0058,    /* Mac address3 high Register                */
    GmacAddr3Low     	  = 0x005C,    /* Mac address3 low Register                 */
    GmacAddr4High    	  = 0x0060,    /* Mac address4 high Register                */
    GmacAddr4Low     	  = 0x0064,    /* Mac address4 low Register                 */
    GmacAddr5High    	  = 0x0068,    /* Mac address5 high Register                */
    GmacAddr5Low     	  = 0x006C,    /* Mac address5 low Register                 */
    GmacAddr6High    	  = 0x0070,    /* Mac address6 high Register                */
    GmacAddr6Low     	  = 0x0074,    /* Mac address6 low Register                 */
    GmacAddr7High    	  = 0x0078,    /* Mac address7 high Register                */
    GmacAddr7Low     	  = 0x007C,    /* Mac address7 low Register                 */
    GmacAddr8High    	  = 0x0080,    /* Mac address8 high Register                */
    GmacAddr8Low     	  = 0x0084,    /* Mac address8 low Register                 */
    GmacAddr9High    	  = 0x0088,    /* Mac address9 high Register                */
    GmacAddr9Low      	  = 0x008C,    /* Mac address9 low Register                 */
    GmacAddr10High        = 0x0090,    /* Mac address10 high Register               */
    GmacAddr10Low    	  = 0x0094,    /* Mac address10 low Register                */
    GmacAddr11High   	  = 0x0098,    /* Mac address11 high Register               */
    GmacAddr11Low    	  = 0x009C,    /* Mac address11 low Register                */
    GmacAddr12High   	  = 0x00A0,    /* Mac address12 high Register               */
    GmacAddr12Low     	  = 0x00A4,    /* Mac address12 low Register                */
    GmacAddr13High   	  = 0x00A8,    /* Mac address13 high Register               */
    GmacAddr13Low   	  = 0x00AC,    /* Mac address13 low Register                */
    GmacAddr14High   	  = 0x00B0,    /* Mac address14 high Register               */
    GmacAddr14Low      	  = 0x00B4,    /* Mac address14 low Register                */
    GmacAddr15High     	  = 0x00B8,    /* Mac address15 high Register               */
    GmacAddr15Low  	      = 0x00BC,    /* Mac address15 low Register                */

    /*Time Stamp Register Map*/
    GmacTSControl	      = 0x0700,  /* Controls the Timestamp update logic                         : only when IEEE 1588 time stamping is enabled in corekit            */

    GmacTSSubSecIncr      = 0x0704,  /* 8 bit value by which sub second register is incremented     : only when IEEE 1588 time stamping without external timestamp input */

    GmacTSHigh  	      = 0x0708,  /* 32 bit seconds(MS)                                          : only when IEEE 1588 time stamping without external timestamp input */
    GmacTSLow   	      = 0x070C,  /* 32 bit nano seconds(MS)                                     : only when IEEE 1588 time stamping without external timestamp input */

    GmacTSHighUpdate      = 0x0710,  /* 32 bit seconds(MS) to be written/added/subtracted           : only when IEEE 1588 time stamping without external timestamp input */
    GmacTSLowUpdate       = 0x0714,  /* 32 bit nano seconds(MS) to be writeen/added/subtracted      : only when IEEE 1588 time stamping without external timestamp input */

    GmacTSAddend          = 0x0718,  /* Used by Software to readjust the clock frequency linearly   : only when IEEE 1588 time stamping without external timestamp input */

    GmacTSTargetTimeHigh  = 0x071C,  /* 32 bit seconds(MS) to be compared with system time          : only when IEEE 1588 time stamping without external timestamp input */
    GmacTSTargetTimeLow   = 0x0720,  /* 32 bit nano seconds(MS) to be compared with system time     : only when IEEE 1588 time stamping without external timestamp input */

    GmacTSHighWord        = 0x0724,  /* Time Stamp Higher Word Register (Version 2 only); only lower 16 bits are valid                                                   */
    //GmacTSHighWordUpdate    = 0x072C,  /* Time Stamp Higher Word Update Register (Version 2 only); only lower 16 bits are valid                                            */

    GmacTSStatus          = 0x0728,  /* Time Stamp Status Register                                                                                                       */
#ifdef AVB_SUPPORT
    GmacAvMacCtrl         = 0x0738,  /* AV mac control Register  */
#endif

};

/**********************************************************
 * GMAC Network interface registers
 * This explains the Register's Layout

 * FES is Read only by default and is enabled only when Tx
 * Config Parameter is enabled for RGMII/SGMII interface
 * during CoreKit Config.

 * DM is Read only with value 1'b1 in Full duplex only Config
 **********************************************************/

/* GmacConfig              = 0x0000,    Mac config Register  Layout */
enum GmacConfigReg
{
    /* Bit description                      Bits         R/W   Reset value  */
    GmacWatchdog		   = 0x00800000,
    GmacWatchdogDisable      = 0x00800000,     /* (WD)Disable watchdog timer on Rx      23           RW                */
    GmacWatchdogEnable       = 0x00000000,     /* Enable watchdog timer                                        0       */

    GmacJabber		   = 0x00400000,
    GmacJabberDisable        = 0x00400000,     /* (JD)Disable jabber timer on Tx        22           RW                */
    GmacJabberEnable         = 0x00000000,     /* Enable jabber timer                                          0       */

    GmacFrameBurst           = 0x00200000,
    GmacFrameBurstEnable     = 0x00200000,     /* (BE)Enable frame bursting during Tx   21           RW                */
    GmacFrameBurstDisable    = 0x00000000,     /* Disable frame bursting                                       0       */

    GmacJumboFrame           = 0x00100000,
    GmacJumboFrameEnable     = 0x00100000,     /* (JE)Enable jumbo frame for Tx         20           RW                */
    GmacJumboFrameDisable    = 0x00000000,     /* Disable jumbo frame                                          0       */

    GmacInterFrameGap7       = 0x000E0000,     /* (IFG) Config7 - 40 bit times          19:17        RW                */
    GmacInterFrameGap6       = 0x000C0000,     /* (IFG) Config6 - 48 bit times                                         */
    GmacInterFrameGap5       = 0x000A0000,     /* (IFG) Config5 - 56 bit times                                         */
    GmacInterFrameGap4       = 0x00080000,     /* (IFG) Config4 - 64 bit times                                         */
    GmacInterFrameGap3       = 0x00040000,     /* (IFG) Config3 - 72 bit times                                         */
    GmacInterFrameGap2       = 0x00020000,     /* (IFG) Config2 - 80 bit times                                         */
    GmacInterFrameGap1       = 0x00010000,     /* (IFG) Config1 - 88 bit times                                         */
    GmacInterFrameGap0       = 0x00000000,     /* (IFG) Config0 - 96 bit times                                 000     */

    GmacDisableCrs	   = 0x00010000,
    GmacMiiGmii		   = 0x00008000,
    GmacSelectMii            = 0x00008000,     /* (PS)Port Select-MII mode              15           RW                */
    GmacSelectGmii           = 0x00000000,     /* GMII mode                                                    0       */

    GmacFESpeed100           = 0x00004000,     /*(FES)Fast Ethernet speed 100Mbps       14           RW                */
    GmacFESpeed10            = 0x00000000,     /* 10Mbps                                                       0       */

    GmacRxOwn		   = 0x00002000,
    GmacDisableRxOwn         = 0x00002000,     /* (DO)Disable receive own packets       13           RW                */
    GmacEnableRxOwn          = 0x00000000,     /* Enable receive own packets                                   0       */

    GmacLoopback		   = 0x00001000,
    GmacLoopbackOn           = 0x00001000,     /* (LM)Loopback mode for GMII/MII        12           RW                */
    GmacLoopbackOff          = 0x00000000,     /* Normal mode                                                  0       */

    GmacDuplex		   = 0x00000800,
    GmacFullDuplex           = 0x00000800,     /* (DM)Full duplex mode                  11           RW                */
    GmacHalfDuplex           = 0x00000000,     /* Half duplex mode                                             0       */

    GmacRxIpcOffload	   = 0x00000400,     /*IPC checksum offload		      10           RW        0       */

    GmacRetry		   = 0x00000200,
    GmacRetryDisable         = 0x00000200,     /* (DR)Disable Retry                      9           RW                */
    GmacRetryEnable          = 0x00000000,     /* Enable retransmission as per BL                              0       */

    GmacLinkUp               = 0x00000100,     /* (LUD)Link UP                           8           RW                */
    GmacLinkDown             = 0x00000100,     /* Link Down                                                    0       */

    GmacPadCrcStrip	   = 0x00000080,
    GmacPadCrcStripEnable    = 0x00000080,     /* (ACS) Automatic Pad/Crc strip enable   7           RW                */
    GmacPadCrcStripDisable   = 0x00000000,     /* Automatic Pad/Crc stripping disable                          0       */

    GmacBackoffLimit	   = 0x00000060,
    GmacBackoffLimit3        = 0x00000060,     /* (BL)Back-off limit in HD mode          6:5         RW                */
    GmacBackoffLimit2        = 0x00000040,     /*                                                                      */
    GmacBackoffLimit1        = 0x00000020,     /*                                                                      */
    GmacBackoffLimit0        = 0x00000000,     /*                                                              00      */

    GmacDeferralCheck	   = 0x00000010,
    GmacDeferralCheckEnable  = 0x00000010,     /* (DC)Deferral check enable in HD mode   4           RW                */
    GmacDeferralCheckDisable = 0x00000000,     /* Deferral check disable                                       0       */

    GmacTx		   = 0x00000008,
    GmacTxEnable             = 0x00000008,     /* (TE)Transmitter enable                 3           RW                */
    GmacTxDisable            = 0x00000000,     /* Transmitter disable                                          0       */

    GmacRx		   = 0x00000004,
    GmacRxEnable             = 0x00000004,     /* (RE)Receiver enable                    2           RW                */
    GmacRxDisable            = 0x00000000,     /* Receiver disable                                             0       */
};

/* GmacFrameFilter    = 0x0004,     Mac frame filtering controls Register Layout*/
enum GmacFrameFilterReg
{
    GmacFilter		   = 0x80000000,
    GmacFilterOff            = 0x80000000,     /* (RA)Receive all incoming packets       31         RW                 */
    GmacFilterOn             = 0x00000000,     /* Receive filtered packets only                                0       */

    GmacHashPerfectFilter	   = 0x00000400,     /*Hash or Perfect Filter enable           10         RW         0       */

    GmacSrcAddrFilter	   = 0x00000200,
    GmacSrcAddrFilterEnable  = 0x00000200,     /* (SAF)Source Address Filter enable       9         RW                 */
    GmacSrcAddrFilterDisable = 0x00000000,     /*                                                              0       */

    GmacSrcInvaAddrFilter    = 0x00000100,
    GmacSrcInvAddrFilterEn   = 0x00000100,     /* (SAIF)Inv Src Addr Filter enable        8         RW                 */
    GmacSrcInvAddrFilterDis  = 0x00000000,     /*                                                              0       */

    GmacPassControl	   = 0x000000C0,
    GmacPassControl3         = 0x000000C0,     /* (PCS)Forwards ctrl frms that pass AF    7:6       RW                 */
    GmacPassControl2         = 0x00000080,     /* Forwards all control frames                                          */
    GmacPassControl1         = 0x00000040,     /* Does not pass control frames                                         */
    GmacPassControl0         = 0x00000000,     /* Does not pass control frames                                 00      */

    GmacBroadcast		   = 0x00000020,
    GmacBroadcastDisable     = 0x00000020,     /* (DBF)Disable Rx of broadcast frames     5         RW                 */
    GmacBroadcastEnable      = 0x00000000,     /* Enable broadcast frames                                      0       */

    GmacMulticastFilter      = 0x00000010,
    GmacMulticastFilterOff   = 0x00000010,     /* (PM) Pass all multicast packets         4         RW                 */
    GmacMulticastFilterOn    = 0x00000000,     /* Pass filtered multicast packets                              0       */

    GmacDestAddrFilter       = 0x00000008,
    GmacDestAddrFilterInv    = 0x00000008,     /* (DAIF)Inverse filtering for DA          3         RW                 */
    GmacDestAddrFilterNor    = 0x00000000,     /* Normal filtering for DA                                      0       */

    GmacMcastHashFilter      = 0x00000004,
    GmacMcastHashFilterOn    = 0x00000004,     /* (HMC)perfom multicast hash filtering    2         RW                 */
    GmacMcastHashFilterOff   = 0x00000000,     /* perfect filtering only                                       0       */

    GmacUcastHashFilter      = 0x00000002,
    GmacUcastHashFilterOn    = 0x00000002,     /* (HUC)Unicast Hash filtering only        1         RW                 */
    GmacUcastHashFilterOff   = 0x00000000,     /* perfect filtering only                                       0       */

    GmacPromiscuousMode      = 0x00000001,
    GmacPromiscuousModeOn    = 0x00000001,     /* Receive all frames                      0         RW                 */
    GmacPromiscuousModeOff   = 0x00000000,     /* Receive filtered packets only                                0       */
};


/*GmacGmiiAddr             = 0x0010,    GMII address Register(ext. Phy) Layout          */
enum GmacGmiiAddrReg
{
    GmiiDevMask              = 0x0000F800,     /* (PA)GMII device address                 15:11     RW         0x00    */
    GmiiDevShift             = 11,

    GmiiRegMask              = 0x000007C0,     /* (GR)GMII register in selected Phy       10:6      RW         0x00    */
    GmiiRegShift             = 6,

    GmiiCsrClkMask	   = 0x0000003C,     /* cdh:CSR Clock bit Mask			 4:2	, must :0x0000001C		     */
    GmiiCsrClk5              = 0x00000014,     /* (CR)CSR Clock Range     250-300 MHz      4:2      RW         000     */
    GmiiCsrClk4              = 0x00000010,     /*                         150-250 MHz                                  */
    GmiiCsrClk3              = 0x0000000C,     /*                         35-60 MHz                                    */
    GmiiCsrClk2              = 0x00000008,     /*                         20-35 MHz                                    */
    GmiiCsrClk1              = 0x00000004,     /*                         100-150 MHz                                  */
    GmiiCsrClk0              = 0x00000000,     /*                         60-100 MHz                                   */

    GmiiWrite                = 0x00000002,     /* (GW)Write to register                      1      RW                 */
    GmiiRead                 = 0x00000000,     /* Read from register                                            0      */

    GmiiBusy                 = 0x00000001,     /* (GB)GMII interface is busy                 0      RW          0      */
};

/* GmacGmiiData            = 0x0014,    GMII data Register(ext. Phy) Layout             */
enum GmacGmiiDataReg
{
    GmiiDataMask             = 0x0000FFFF,     /* (GD)GMII Data                             15:0    RW         0x0000  */
};


/*GmacFlowControl    = 0x0018,    Flow control Register   Layout                  */
enum GmacFlowControlReg
{
    GmacPauseTimeMask        = 0xFFFF0000,     /* (PT) PAUSE TIME field in the control frame  31:16   RW       0x0000  */
    GmacPauseTimeShift       = 16,

    GmacPauseLowThresh	   = 0x00000030,
    GmacPauseLowThresh3      = 0x00000030,     /* (PLT)thresh for pause tmr 256 slot time      5:4    RW               */
    GmacPauseLowThresh2      = 0x00000020,     /*                           144 slot time                              */
    GmacPauseLowThresh1      = 0x00000010,     /*                            28 slot time                              */
    GmacPauseLowThresh0      = 0x00000000,     /*                             4 slot time                       000    */

    GmacUnicastPauseFrame    = 0x00000008,
    GmacUnicastPauseFrameOn  = 0x00000008,     /* (UP)Detect pause frame with unicast addr.     3    RW                */
    GmacUnicastPauseFrameOff = 0x00000000,     /* Detect only pause frame with multicast addr.                   0     */

    GmacRxFlowControl	   = 0x00000004,
    GmacRxFlowControlEnable  = 0x00000004,     /* (RFE)Enable Rx flow control                   2    RW                */
    GmacRxFlowControlDisable = 0x00000000,     /* Disable Rx flow control                                        0     */

    GmacTxFlowControl   	   = 0x00000002,
    GmacTxFlowControlEnable  = 0x00000002,     /* (TFE)Enable Tx flow control                   1    RW                */
    GmacTxFlowControlDisable = 0x00000000,     /* Disable flow control                                           0     */

    GmacFlowControlBackPressure = 0x00000001,
    GmacSendPauseFrame       = 0x00000001,     /* (FCB/PBA)send pause frm/Apply back pressure   0    RW          0     */
};

/*  GmacInterruptStatus	  = 0x0038,     Mac Interrupt ststus register	       */
enum GmacInterruptStatusBitDefinition
{
    GmacTSIntSts		   = 0x00000200,    /* set if int generated due to TS (Read Time Stamp Status Register to know details)*/
    GmacMmcRxChksumOffload   = 0x00000080,    /* set if int generated in MMC RX CHECKSUM OFFLOAD int register	                  */
    GmacMmcTxIntSts	   = 0x00000040,    /* set if int generated in MMC TX Int register			   */
    GmacMmcRxIntSts	   = 0x00000020,    /* set if int generated in MMC RX Int register 			   */
    GmacMmcIntSts		   = 0x00000010,    /* set if any of the above bit [7:5] is set			   */
    GmacPmtIntSts		   = 0x00000008,    /* set whenver magic pkt/wake-on-lan frame is received		   */
    GmacPcsAnComplete	   = 0x00000004,    /* set when AN is complete in TBI/RTBI/SGMIII phy interface        */
    GmacPcsLnkStsChange	   = 0x00000002,    /* set if any lnk status change in TBI/RTBI/SGMII interface        */
    GmacRgmiiIntSts	   = 0x00000001,    /* set if any change in lnk status of RGMII interface		   */

};

/*  GmacInterruptMask       = 0x003C,     Mac Interrupt Mask register	       */
enum GmacInterruptMaskBitDefinition
{
    GmacTSIntMask		   = 0x00000200,    /* when set disables the time stamp interrupt generation            */
    GmacPmtIntMask	   = 0x00000008,    /* when set Disables the assertion of PMT interrupt     	    	*/
    GmacPcsAnIntMask	   = 0x00000004,    /* When set disables the assertion of PCS AN complete interrupt	      	*/
    GmacPcsLnkStsIntMask	   = 0x00000002,    /* when set disables the assertion of PCS lnk status change interrupt	*/
    GmacRgmiiIntMask	   = 0x00000001,    /* when set disables the assertion of RGMII int 			*/
};

/**********************************************************
 * GMAC DMA registers
 * For Pci based system address is BARx + GmaDmaBase
 * For any other system translation is done accordingly
 **********************************************************/

enum DmaRegisters
{
    DmaBusMode        = 0x0000,    /* CSR0 - Bus Mode Register                          */
    DmaTxPollDemand   = 0x0004,    /* CSR1 - Transmit Poll Demand Register              */
    DmaRxPollDemand   = 0x0008,    /* CSR2 - Receive Poll Demand Register               */
    DmaRxBaseAddr     = 0x000C,    /* CSR3 - Receive Descriptor list base address       */
    DmaTxBaseAddr     = 0x0010,    /* CSR4 - Transmit Descriptor list base address      */
    DmaStatus         = 0x0014,    /* CSR5 - Dma status Register                        */
    DmaControl        = 0x0018,    /* CSR6 - Dma Operation Mode Register                */
    DmaInterrupt      = 0x001C,    /* CSR7 - Interrupt enable                           */
    DmaMissedFr       = 0x0020,    /* CSR8 - Missed Frame & Buffer overflow Counter     */
    DmaTxCurrDesc     = 0x0048,    /*      - Current host Tx Desc Register              */
    DmaRxCurrDesc     = 0x004C,    /*      - Current host Rx Desc Register              */
    DmaTxCurrAddr     = 0x0050,    /* CSR20 - Current host transmit buffer address      */
    DmaRxCurrAddr     = 0x0054,    /* CSR21 - Current host receive buffer address       */

#ifdef AVB_SUPPORT
    HwFeature         = 0x0058,    /* Hardware Feature Register                         */

    DmaSlotFnCtrlSts  = 0x0030,    /* Slot function control and status register         */

    DmaChannelCtrl    = 0x0060,    /* Channel Control register only for Channel1 and Channel2 */
    DmaChannelAvSts   = 0x0064,    /* Channel Status register only for Channel1 and  Channel2 */
    IdleSlopeCredit   = 0x0068,    /* Idle slope credit register                              */
    SendSlopeCredit   = 0x006C,    /* Send slope credit register                              */
    HighCredit        = 0x0070,    /* High Credit register                                    */
    LoCredit          = 0x0074,    /* Lo Credit Register                                      */
#endif

};

/**********************************************************
 * DMA Engine registers Layout
 **********************************************************/

/*DmaBusMode               = 0x0000,    CSR0 - Bus Mode */
enum DmaBusModeReg
{
    /* Bit description                                Bits     R/W   Reset value */
#ifdef AVB_SUPPORT
    DmaChannelPrioWt       = 0x30000000,    /* Channel priority weight mask                     29:28    RW       0       */
    DmaChannelPrio1        = 0x00000000,    /* Channel priority weight 1                        29:28    RW       0       */
    DmaChannelPrio2        = 0x10000000,    /* Channel priority weight 2                        29:28    RW       0       */
    DmaChannelPrio3        = 0x20000000,    /* Channel priority weight 3                        29:28    RW       0       */
    DmaChannelPrio4        = 0x30000000,    /* Channel priority weight 4                        29:28    RW       0       */

    DmaTxRxPrio            = 0x08000000,    /* When set indicates Tx Dma has more priority       27      RW        0       */

    DmaPriorityRatio11     = 0x00000000,   /* (PR)TX:RX DMA priority ratio 1:1                15:14   RW        00      */
    DmaPriorityRatio21     = 0x00004000,   /* (PR)TX:RX DMA priority ratio 2:1                                          */
    DmaPriorityRatio31     = 0x00008000,   /* (PR)TX:RX DMA priority ratio 3:1                                          */
    DmaPriorityRatio41     = 0x0000C000,   /* (PR)TX:RX DMA priority ratio 4:1                                          */

    DmaArbitration         = 0x00000002,    /* Dma Arbitration decides whether strict prio or RR  1      RW       0       */
    DmaArbitrationStrict   = 0x00000002,    /* Dma Arbitration decides whether strict prio or RR  1      RW       0       */
    DmaArbitrationRR       = 0x00000000,    /* Dma Arbitration decides whether strict prio or RR  0      RW       0       */
#endif


    DmaFixedBurstEnable     = 0x00010000,   /* (FB)Fixed Burst SINGLE, INCR4, INCR8 or INCR16   16     RW                */
    DmaFixedBurstDisable    = 0x00000000,   /*             SINGLE, INCR                                          0       */

    DmaTxPriorityRatio11    = 0x00000000,   /* (PR)TX:RX DMA priority ratio 1:1                15:14   RW        00      */
    DmaTxPriorityRatio21    = 0x00004000,   /* (PR)TX:RX DMA priority ratio 2:1                                          */
    DmaTxPriorityRatio31    = 0x00008000,   /* (PR)TX:RX DMA priority ratio 3:1                                          */
    DmaTxPriorityRatio41    = 0x0000C000,   /* (PR)TX:RX DMA priority ratio 4:1                                          */

    DmaBurstLengthx8        = 0x01000000,   /* When set mutiplies the PBL by 8                  24      RW        0      */

    DmaBurstLength256       = 0x01002000,   /*(DmaBurstLengthx8 | DmaBurstLength32) = 256      [24]:13:8                 */
    DmaBurstLength128       = 0x01001000,   /*(DmaBurstLengthx8 | DmaBurstLength16) = 128      [24]:13:8                 */
    DmaBurstLength64        = 0x01000800,   /*(DmaBurstLengthx8 | DmaBurstLength8) = 64        [24]:13:8                 */
    DmaBurstLength32        = 0x00002000,   /* (PBL) programmable Dma burst length = 32        13:8    RW                */
    DmaBurstLength16        = 0x00001000,   /* Dma burst length = 16                                                     */
    DmaBurstLength8         = 0x00000800,   /* Dma burst length = 8                                                      */
    DmaBurstLength4         = 0x00000400,   /* Dma burst length = 4                                                      */
    DmaBurstLength2         = 0x00000200,   /* Dma burst length = 2                                                      */
    DmaBurstLength1         = 0x00000100,   /* Dma burst length = 1                                                      */
    DmaBurstLength0         = 0x00000000,   /* Dma burst length = 0                                               0x00   */

    DmaDescriptor8Words     = 0x00000080,   /* Enh Descriptor works  1=> 8 word descriptor      7                  0    */
    DmaDescriptor4Words     = 0x00000000,   /* Enh Descriptor works  0=> 4 word descriptor      7                  0    */

    DmaDescriptorSkip16     = 0x00000040,   /* (DSL)Descriptor skip length (no.of dwords)       6:2     RW               */
    DmaDescriptorSkip8      = 0x00000020,   /* between two unchained descriptors                                         */
    DmaDescriptorSkip4      = 0x00000010,   /*                                                                           */
    DmaDescriptorSkip2      = 0x00000008,   /*                                                                           */
    DmaDescriptorSkip1      = 0x00000004,   /*                                                                           */
    DmaDescriptorSkip0      = 0x00000000,   /*                                                                    0x00   */

    DmaArbitRr              = 0x00000000,   /* (DA) DMA RR arbitration                            1     RW         0     */
    DmaArbitPr              = 0x00000002,   /* Rx has priority over Tx                                                   */

    DmaResetOn              = 0x00000001,   /* (SWR)Software Reset DMA engine                     0     RW               */
    DmaResetOff             = 0x00000000,   /*                                                                      0    */
};


/*DmaStatus         = 0x0014,    CSR5 - Dma status Register                        */
enum DmaStatusReg
{
    /*Bit 28 27 and 26 indicate whether the interrupt due to PMT GMACMMC or GMAC LINE Remaining bits are DMA interrupts*/

#ifdef AVB_SUPPORT
    DmaSlotCounterIntr      = 0x40000000,   /* For Ch1 and Ch2 AVB slot interrupt status          31     RW       0       */
#endif
#ifdef LPI_SUPPORT
    GmacLPIIntr             = 0x40000000,   /* GMC LPI interrupt                                  31     RO       0       */
#endif

    GmacPmtIntr             = 0x10000000,   /* (GPI)Gmac subsystem interrupt                      28     RO       0       */
    GmacMmcIntr             = 0x08000000,   /* (GMI)Gmac MMC subsystem interrupt                  27     RO       0       */
    GmacLineIntfIntr        = 0x04000000,   /* Line interface interrupt                           26     RO       0       */

    DmaErrorBit2            = 0x02000000,   /* (EB)Error bits 0-data buffer, 1-desc. access       25     RO       0       */
    DmaErrorBit1            = 0x01000000,   /* (EB)Error bits 0-write trnsf, 1-read transfr       24     RO       0       */
    DmaErrorBit0            = 0x00800000,   /* (EB)Error bits 0-Rx DMA, 1-Tx DMA                  23     RO       0       */

    DmaTxState              = 0x00700000,   /* (TS)Transmit process state                         22:20  RO               */
    DmaTxStopped            = 0x00000000,   /* Stopped - Reset or Stop Tx Command issued                         000      */
    DmaTxFetching           = 0x00100000,   /* Running - fetching the Tx descriptor                                       */
    DmaTxWaiting            = 0x00200000,   /* Running - waiting for status                                               */
    DmaTxReading            = 0x00300000,   /* Running - reading the data from host memory                                */
    DmaTxSuspended          = 0x00600000,   /* Suspended - Tx Descriptor unavailabe                                       */
    DmaTxClosing            = 0x00700000,   /* Running - closing Rx descriptor                                            */

    DmaRxState              = 0x000E0000,   /* (RS)Receive process state                         19:17  RO                */
    DmaRxStopped            = 0x00000000,   /* Stopped - Reset or Stop Rx Command issued                         000      */
    DmaRxFetching           = 0x00020000,   /* Running - fetching the Rx descriptor                                       */
    DmaRxWaiting            = 0x00060000,   /* Running - waiting for packet                                               */
    DmaRxSuspended          = 0x00080000,   /* Suspended - Rx Descriptor unavailable                                      */
    DmaRxClosing            = 0x000A0000,   /* Running - closing descriptor                                               */
    DmaRxQueuing            = 0x000E0000,   /* Running - queuing the recieve frame into host memory                       */

    DmaIntNormal            = 0x00010000,   /* (NIS)Normal interrupt summary                     16     RW        0       */
    DmaIntAbnormal          = 0x00008000,   /* (AIS)Abnormal interrupt summary                   15     RW        0       */

    DmaIntEarlyRx           = 0x00004000,   /* Early receive interrupt (Normal)       RW        0       */
    DmaIntBusError          = 0x00002000,   /* Fatal bus error (Abnormal)             RW        0       */
    DmaIntEarlyTx           = 0x00000400,   /* Early transmit interrupt (Abnormal)    RW        0       */
    DmaIntRxWdogTO          = 0x00000200,   /* Receive Watchdog Timeout (Abnormal)    RW        0       */
    DmaIntRxStopped         = 0x00000100,   /* Receive process stopped (Abnormal)     RW        0       */
    DmaIntRxNoBuffer        = 0x00000080,   /* Receive buffer unavailable (Abnormal)  RW        0       */
    DmaIntRxCompleted       = 0x00000040,   /* Completion of frame reception (Normal) RW        0       */
    DmaIntTxUnderflow       = 0x00000020,   /* Transmit underflow (Abnormal)          RW        0       */
    DmaIntRcvOverflow       = 0x00000010,   /* Receive Buffer overflow interrupt      RW        0       */
    DmaIntTxJabberTO        = 0x00000008,   /* Transmit Jabber Timeout (Abnormal)     RW        0       */
    DmaIntTxNoBuffer        = 0x00000004,   /* Transmit buffer unavailable (Normal)   RW        0       */
    DmaIntTxStopped         = 0x00000002,   /* Transmit process stopped (Abnormal)    RW        0       */
    DmaIntTxCompleted       = 0x00000001,   /* Transmit completed (Normal)            RW        0       */
};

/*DmaControl        = 0x0018,     CSR6 - Dma Operation Mode Register                */
enum DmaControlReg
{
    DmaDisableDropTcpCs	  = 0x04000000,   /* (DT) Dis. drop. of tcp/ip CS error frames        26      RW        0       */
    DmaDisableFlush       = 0x01000000,

    DmaStoreAndForward      = 0x00200000,   /* (SF)Store and forward                            21      RW        0       */
    DmaFlushTxFifo          = 0x00100000,   /* (FTF)Tx FIFO controller is reset to default      20      RW        0       */

    DmaTxThreshCtrl         = 0x0001C000,   /* (TTC)Controls thre Threh of MTL tx Fifo          16:14   RW                */
    DmaTxThreshCtrl16       = 0x0001C000,   /* (TTC)Controls thre Threh of MTL tx Fifo 16       16:14   RW                */
    DmaTxThreshCtrl24       = 0x00018000,   /* (TTC)Controls thre Threh of MTL tx Fifo 24       16:14   RW                */
    DmaTxThreshCtrl32       = 0x00014000,   /* (TTC)Controls thre Threh of MTL tx Fifo 32       16:14   RW                */
    DmaTxThreshCtrl40       = 0x00010000,   /* (TTC)Controls thre Threh of MTL tx Fifo 40       16:14   RW                */
    DmaTxThreshCtrl256      = 0x0000c000,   /* (TTC)Controls thre Threh of MTL tx Fifo 256      16:14   RW                */
    DmaTxThreshCtrl192      = 0x00008000,   /* (TTC)Controls thre Threh of MTL tx Fifo 192      16:14   RW                */
    DmaTxThreshCtrl128      = 0x00004000,   /* (TTC)Controls thre Threh of MTL tx Fifo 128      16:14   RW                */
    DmaTxThreshCtrl64       = 0x00000000,   /* (TTC)Controls thre Threh of MTL tx Fifo 64       16:14   RW        000     */

    DmaTxStart              = 0x00002000,   /* (ST)Start/Stop transmission                      13      RW        0       */

    DmaRxFlowCtrlDeact      = 0x00401800,   /* (RFD)Rx flow control deact. threhold             [22]:12:11   RW                 */
    DmaRxFlowCtrlDeact1K    = 0x00000000,   /* (RFD)Rx flow control deact. threhold (1kbytes)   [22]:12:11   RW        00       */
    DmaRxFlowCtrlDeact2K    = 0x00000800,   /* (RFD)Rx flow control deact. threhold (2kbytes)   [22]:12:11   RW                 */
    DmaRxFlowCtrlDeact3K    = 0x00001000,   /* (RFD)Rx flow control deact. threhold (3kbytes)   [22]:12:11   RW                 */
    DmaRxFlowCtrlDeact4K    = 0x00001800,   /* (RFD)Rx flow control deact. threhold (4kbytes)   [22]:12:11   RW                 */
    DmaRxFlowCtrlDeact5K    = 0x00400000,   /* (RFD)Rx flow control deact. threhold (4kbytes)   [22]:12:11   RW                 */
    DmaRxFlowCtrlDeact6K    = 0x00400800,   /* (RFD)Rx flow control deact. threhold (4kbytes)   [22]:12:11   RW                 */
    DmaRxFlowCtrlDeact7K    = 0x00401000,   /* (RFD)Rx flow control deact. threhold (4kbytes)   [22]:12:11   RW                 */

    DmaRxFlowCtrlAct        = 0x00800600,   /* (RFA)Rx flow control Act. threhold              [23]:10:09   RW                 */
    DmaRxFlowCtrlAct1K      = 0x00000000,   /* (RFA)Rx flow control Act. threhold (1kbytes)    [23]:10:09   RW        00       */
    DmaRxFlowCtrlAct2K      = 0x00000200,   /* (RFA)Rx flow control Act. threhold (2kbytes)    [23]:10:09   RW                 */
    DmaRxFlowCtrlAct3K      = 0x00000400,   /* (RFA)Rx flow control Act. threhold (3kbytes)    [23]:10:09   RW                 */
    DmaRxFlowCtrlAct4K      = 0x00000300,   /* (RFA)Rx flow control Act. threhold (4kbytes)    [23]:10:09   RW                 */
    DmaRxFlowCtrlAct5K      = 0x00800000,   /* (RFA)Rx flow control Act. threhold (5kbytes)    [23]:10:09   RW                 */
    DmaRxFlowCtrlAct6K      = 0x00800200,   /* (RFA)Rx flow control Act. threhold (6kbytes)    [23]:10:09   RW                 */
    DmaRxFlowCtrlAct7K      = 0x00800400,   /* (RFA)Rx flow control Act. threhold (7kbytes)    [23]:10:09   RW                 */

    DmaRxThreshCtrl         = 0x00000018,   /* (RTC)Controls thre Threh of MTL rx Fifo          4:3   RW                */
    DmaRxThreshCtrl64       = 0x00000000,   /* (RTC)Controls thre Threh of MTL tx Fifo 64       4:3   RW                */
    DmaRxThreshCtrl32       = 0x00000008,   /* (RTC)Controls thre Threh of MTL tx Fifo 32       4:3   RW                */
    DmaRxThreshCtrl96       = 0x00000010,   /* (RTC)Controls thre Threh of MTL tx Fifo 96       4:3   RW                */
    DmaRxThreshCtrl128      = 0x00000018,   /* (RTC)Controls thre Threh of MTL tx Fifo 128      4:3   RW                */

    DmaEnHwFlowCtrl         = 0x00000100,   /* (EFC)Enable HW flow control                      8       RW                 */
    DmaDisHwFlowCtrl        = 0x00000000,   /* Disable HW flow control                                            0        */

    DmaFwdErrorFrames       = 0x00000080,   /* (FEF)Forward error frames                        7       RW        0       */
    DmaFwdUnderSzFrames     = 0x00000040,   /* (FUF)Forward undersize frames                    6       RW        0       */
    DmaTxSecondFrame        = 0x00000004,   /* (OSF)Operate on second frame                     4       RW        0       */
    DmaRxStart              = 0x00000002,   /* (SR)Start/Stop reception                         1       RW        0       */
};


/*DmaInterrupt      = 0x001C,    CSR7 - Interrupt enable Register Layout     */
enum  DmaInterruptReg
{
    DmaIeNormal            = DmaIntNormal     ,   /* Normal interrupt enable                 RW        0       */
    DmaIeAbnormal          = DmaIntAbnormal   ,   /* Abnormal interrupt enable               RW        0       */

    DmaIeEarlyRx           = DmaIntEarlyRx    ,   /* Early receive interrupt enable          RW        0       */
    DmaIeBusError          = DmaIntBusError   ,   /* Fatal bus error enable                  RW        0       */
    DmaIeEarlyTx           = DmaIntEarlyTx    ,   /* Early transmit interrupt enable         RW        0       */
    DmaIeRxWdogTO          = DmaIntRxWdogTO   ,   /* Receive Watchdog Timeout enable         RW        0       */
    DmaIeRxStopped         = DmaIntRxStopped  ,   /* Receive process stopped enable          RW        0       */
    DmaIeRxNoBuffer        = DmaIntRxNoBuffer ,   /* Receive buffer unavailable enable       RW        0       */
    DmaIeRxCompleted       = DmaIntRxCompleted,   /* Completion of frame reception enable    RW        0       */
    DmaIeTxUnderflow       = DmaIntTxUnderflow,   /* Transmit underflow enable               RW        0       */

    DmaIeRxOverflow        = DmaIntRcvOverflow,   /* Receive Buffer overflow interrupt       RW        0       */
    DmaIeTxJabberTO        = DmaIntTxJabberTO ,   /* Transmit Jabber Timeout enable          RW        0       */
    DmaIeTxNoBuffer        = DmaIntTxNoBuffer ,   /* Transmit buffer unavailable enable      RW        0       */
    DmaIeTxStopped         = DmaIntTxStopped  ,   /* Transmit process stopped enable         RW        0       */
    DmaIeTxCompleted       = DmaIntTxCompleted,   /* Transmit completed enable               RW        0       */
};

#ifdef AVB_SUPPORT
/*DmaSlotFnCtrlSts  = 0x0030,     Slot function control and status register         */
enum DmaSlotFnCtrlStsReg
{
    SlotNum                = 0x000F0000,   /* Current Slot Number                            19:16     R0         0      */
    AdvSlotInt             = 0x00000002,   /* Advance the slot interval for data fetch       1         RW         0      */
    EnaSlot                = 0x00000001,   /* Enable checking of Slot number                 0         RW         0      */
};

/*  DmaChannelCtrl    = 0x0060,     Channel Control register only for Channel1 and Channel2 */
enum DmaChannelCtrlReg
{
    ChannelSlotIntEn       = 0x00020000,   /* Channel Slot Interrupt Enable                  16         RW         0      */
    ChannelSlotCount       = 0x00000070,   /* Channel Slot Count                             6:4        RW         0      */
    ChannelCreditCtrl      = 0x00000002,   /* Channel Credit Control                         1          RW         0      */
    ChannelCreditShDis     = 0x00000001,   /* Channel Credit based shaping disable           0          RW         0      */
};

/*  DmaChannelSts     = 0x0064,     Channel Status register only for Channel1 and  Channel2 */
enum DmaChannelStsReg
{
    ChannelAvBitsPerSlot   = 0x0000FFFF,   /* Channel Average Bits per slot                  16:0       RO         0      */
};

/*  IdleSlopeCredit   = 0x0068,     Idle slope credit register                              */
enum IdleSlopeCreditReg
{
    ChannelIdleSlCr       = 0x00003FFF,   /*Channel Idle Slope Credit                       13:0       RW         0     */
};

/*SendSlopeCredit   = 0x006C,     Send slope credit register                              */
enum SendSlopeCreditReg
{
    ChannelSendSlCr       = 0x00003FFF,   /*Channel Send Slope Credit                       13:0       RW         0     */
};

/*  HighCredit        = 0x0070,     High Credit register                                    */
enum HighCreditReg
{
    ChannelHiCr           = 0x1FFFFFFF,   /*Channel Hi Credit                               28:0       RW         0     */
};

/*  LoCredit          = 0x0074,     Lo Credit Register                                      */
enum LoCreditReg
{
    ChannelLoCr           = 0x1FFFFFFF,   /* Channel Lo Credit                               28:0      RW         0    */
};
/*DmaChannelAvSts   */
enum DmaChannelAvStsReg
{
    ChannelAvgBitsPerSlotMsk = 0x0001FFFF,
};
#endif

/**********************************************************
 * DMA Engine descriptors
 **********************************************************/
#ifdef ENH_DESC
/*
**********Enhanced Descritpor structure to support 8K buffer per buffer ****************************

DmaRxBaseAddr     = 0x000C,   CSR3 - Receive Descriptor list base address
DmaRxBaseAddr is the pointer to the first Rx Descriptors. the Descriptor format in Little endian with a
32 bit Data bus is as shown below

Similarly
DmaTxBaseAddr     = 0x0010,  CSR4 - Transmit Descriptor list base address
DmaTxBaseAddr is the pointer to the first Rx Descriptors. the Descriptor format in Little endian with a
32 bit Data bus is as shown below
          --------------------------------------------------------------------------
    RDES0	  |OWN (31)| Status                                                        |
 		  --------------------------------------------------------------------------
    RDES1	  | Ctrl | Res | Byte Count Buffer 2 | Ctrl | Res | Byte Count Buffer 1    |
		  --------------------------------------------------------------------------
    RDES2	  |  Buffer 1 Address                                                      |
		  --------------------------------------------------------------------------
    RDES3	  |  Buffer 2 Address / Next Descriptor Address                            |
		  --------------------------------------------------------------------------

          --------------------------------------------------------------------------
    TDES0	  |OWN (31)| Ctrl | Res | Ctrl | Res | Status                              |
 		  --------------------------------------------------------------------------
    TDES1	  | Res | Byte Count Buffer 2 | Res |         Byte Count Buffer 1          |
		  --------------------------------------------------------------------------
    TDES2	  |  Buffer 1 Address                                                      |
		  --------------------------------------------------------------------------
    TDES3     |  Buffer 2 Address / Next Descriptor Address                        |
		  --------------------------------------------------------------------------

*/

enum DmaDescriptorStatus    /* status word of DMA descriptor */
{

    DescOwnByDma          = 0x80000000,   /* (OWN)Descriptor is owned by DMA engine              31      RW                  */

    DescDAFilterFail      = 0x40000000,   /* (AFM)Rx - DA Filter Fail for the rx frame           30                          */

    DescFrameLengthMask   = 0x3FFF0000,   /* (FL)Receive descriptor frame length                 29:16                       */
    DescFrameLengthShift  = 16,

    DescError             = 0x00008000,   /* (ES)Error summary bit  - OR of the follo. bits:     15                          */
    /*  DE || OE || IPC || LC || RWT || RE || CE */
    DescRxTruncated       = 0x00004000,   /* (DE)Rx - no more descriptors for receive frame      14                          */
    DescSAFilterFail      = 0x00002000,   /* (SAF)Rx - SA Filter Fail for the received frame     13                          */
    DescRxLengthError	    = 0x00001000,   /* (LE)Rx - frm size not matching with len field     12                          */
    DescRxDamaged         = 0x00000800,   /* (OE)Rx - frm was damaged due to buffer overflow     11                          */
    DescRxVLANTag         = 0x00000400,   /* (VLAN)Rx - received frame is a VLAN frame           10                          */
    DescRxFirst           = 0x00000200,   /* (FS)Rx - first descriptor of the frame              9                          */
    DescRxLast            = 0x00000100,   /* (LS)Rx - last descriptor of the frame               8                          */
    DescRxLongFrame       = 0x00000080,   /* (Giant Frame)Rx - frame is longer than 1518/1522    7                          */
    DescRxCollision       = 0x00000040,   /* (LC)Rx - late collision occurred during reception   6                          */
    DescRxFrameEther      = 0x00000020,   /* (FT)Rx - Frame type - Ethernet, otherwise 802.3     5                          */
    DescRxWatchdog        = 0x00000010,   /* (RWT)Rx - watchdog timer expired during reception   4                          */
    DescRxMiiError        = 0x00000008,   /* (RE)Rx - error reported by MII interface            3                          */
    DescRxDribbling       = 0x00000004,   /* (DE)Rx - frame contains non int multiple of 8 bits  2                          */
    DescRxCrc             = 0x00000002,   /* (CE)Rx - CRC error                                  1                          */
    //DescRxMacMatch        = 0x00000001,   /* (RX MAC Address) Rx mac address reg(1 to 15)match   0                          */

    DescRxEXTsts          = 0x00000001,   /* Extended Status Available (RDES4)                   0                          */

    DescTxIntEnable       = 0x40000000,   /* (IC)Tx - interrupt on completion                    30                       */
    DescTxLast            = 0x20000000,   /* (LS)Tx - Last segment of the frame                  29                       */
    DescTxFirst           = 0x10000000,   /* (FS)Tx - First segment of the frame                 28                       */
    DescTxDisableCrc      = 0x08000000,   /* (DC)Tx - Add CRC disabled (first segment only)      27                       */
    DescTxDisablePadd     = 0x04000000,   /* (DP)disable padding, added by - reyaz               26                       */

    DescTxCisMask     	= 0x00c00000,   /* Tx checksum offloading control mask		       23:22			*/
    DescTxCisBypass   	= 0x00000000,   /* Checksum bypass								                    */
    DescTxCisIpv4HdrCs	= 0x00400000,	/* IPv4 header checksum								                 */
    DescTxCisTcpOnlyCs    = 0x00800000,	/* TCP/UDP/ICMP checksum. Pseudo header checksum is assumed to be present */
    DescTxCisTcpPseudoCs  = 0x00c00000,	/* TCP/UDP/ICMP checksum fully in hardware including pseudo header        */

    TxDescEndOfRing       = 0x00200000,   /* (TER)End of descriptors ring                        21                       */
    TxDescChain           = 0x00100000,   /* (TCH)Second buffer address is chain address         20                       */

    DescRxChkBit0		    = 0x00000001,   /*()  Rx - Rx Payload Checksum Error                   0                          */
    DescRxChkBit7	    	= 0x00000080,   /* (IPC CS ERROR)Rx - Ipv4 header checksum error       7                          */
    DescRxChkBit5 		= 0x00000020,   /* (FT)Rx - Frame type - Ethernet, otherwise 802.3         5                          */

    DescRxTSavail         = 0x00000080,   /* Time stamp available                                7                          */
    DescRxFrameType   	= 0x00000020,   /* (FT)Rx - Frame type - Ethernet, otherwise 802.3       5                          */

    DescTxIpv4ChkError    = 0x00010000,   /* (IHE) Tx Ip header error                            16                         */
    DescTxTimeout         = 0x00004000,   /* (JT)Tx - Transmit jabber timeout                    14                         */
    DescTxFrameFlushed    = 0x00002000,   /* (FF)Tx - DMA/MTL flushed the frame due to SW flush  13                         */
    DescTxPayChkError     = 0x00001000,   /* (PCE) Tx Payload checksum Error                     12                         */
    DescTxLostCarrier     = 0x00000800,   /* (LC)Tx - carrier lost during tramsmission           11                         */
    DescTxNoCarrier       = 0x00000400,   /* (NC)Tx - no carrier signal from the tranceiver      10                         */
    DescTxLateCollision   = 0x00000200,   /* (LC)Tx - transmission aborted due to collision      9                         */
    DescTxExcCollisions   = 0x00000100,   /* (EC)Tx - transmission aborted after 16 collisions   8                         */
    DescTxVLANFrame       = 0x00000080,   /* (VF)Tx - VLAN-type frame                            7                         */

    DescTxCollMask        = 0x00000078,   /* (CC)Tx - Collision count                            6:3                        */
    DescTxCollShift       = 3,

    DescTxExcDeferral     = 0x00000004,   /* (ED)Tx - excessive deferral                         2                        */
    DescTxUnderflow       = 0x00000002,   /* (UF)Tx - late data arrival from the memory          1                        */
    DescTxDeferred        = 0x00000001,   /* (DB)Tx - frame transmision deferred                 0                        */

    /*
    This explains the RDES1/TDES1 bits layout
    	--------------------------------------------------------------------
        RDES1/TDES1  | Control Bits | Byte Count Buffer 2 | Byte Count Buffer 1   |
    	 --------------------------------------------------------------------
    */
    // DmaDescriptorLength     length word of DMA descriptor


    RxDisIntCompl   = 0x80000000, /* (Disable Rx int on completion) 			31 */
    RxDescEndOfRing = 0x00008000, /* (TER)End of descriptors ring               15 */
    RxDescChain     = 0x00004000, /* (TCH)Second buffer address is chain address 14 */


    DescSize2Mask   = 0x1FFF0000,   /* (TBS2) Buffer 2 size                  28:16 */
    DescSize2Shift  = 16,
    DescSize1Mask   = 0x00001FFF,   /* (TBS1) Buffer 1 size                  12:0                     */
    DescSize1Shift  = 0,


    /*
    This explains the RDES4 Extended Status bits layout
    		   --------------------------------------------------------------------
      RDES4   |                             Extended Status                        |
    		   --------------------------------------------------------------------
    */

#ifdef AVB_SUPPORT
    DescRxVlanPrioVal     = 0x001C0000,    /* Gives the VLAN Priority Value                      20:18                     */
    DescRxVlanPrioShVal   = 18,            /* Gives the shift value to get priority value in LS bits                       */

    DescRxAvTagPktRx      = 0x00020000,    /* Indicates AV tagged Packet is received              17                       */
    DescRxAvPktRx         = 0x00010000,    /* Indicates AV Packet received                        16                       */
#endif

    DescRxPtpAvail        = 0x00004000,    /* PTP snapshot available                              14                        */
    DescRxPtpVer          = 0x00002000,    /* When set indicates IEEE1584 Version 2 (else Ver1)   13                        */
    DescRxPtpFrameType    = 0x00001000,    /* PTP frame type Indicates PTP sent over ethernet     12                        */
    DescRxPtpMessageType  = 0x00000F00,    /* Message Type                                        11:8                      */
    DescRxPtpNo           = 0x00000000,    /* 0000 => No PTP message received                                               */
    DescRxPtpSync         = 0x00000100,    /* 0001 => Sync (all clock types) received                                       */
    DescRxPtpFollowUp     = 0x00000200,    /* 0010 => Follow_Up (all clock types) received                                  */
    DescRxPtpDelayReq     = 0x00000300,    /* 0011 => Delay_Req (all clock types) received                                  */
    DescRxPtpDelayResp    = 0x00000400,    /* 0100 => Delay_Resp (all clock types) received                                 */
    DescRxPtpPdelayReq    = 0x00000500,    /* 0101 => Pdelay_Req (in P to P tras clk)  or Announce in Ord and Bound clk     */
    DescRxPtpPdelayResp   = 0x00000600,    /* 0110 => Pdealy_Resp(in P to P trans clk) or Management in Ord and Bound clk   */
    DescRxPtpPdelayRespFP = 0x00000700,    /* 0111 => Pdealy_Resp_Follow_Up (in P to P trans clk) or Signaling in Ord and Bound clk   */
    DescRxPtpIPV6         = 0x00000080,    /* Received Packet is  in IPV6 Packet                  7                         */
    DescRxPtpIPV4         = 0x00000040,    /* Received Packet is  in IPV4 Packet                  6                         */

    DescRxChkSumBypass    = 0x00000020,    /* When set indicates checksum offload engine          5
                                            is bypassed                                                                   */
    DescRxIpPayloadError  = 0x00000010,    /* When set indicates 16bit IP payload CS is in error  4                         */
    DescRxIpHeaderError   = 0x00000008,    /* When set indicates 16bit IPV4 header CS is in       3
                                            error or IP datagram version is not consistent
                                            with Ethernet type value                                                      */
    DescRxIpPayloadType   = 0x00000007,     /* Indicate the type of payload encapsulated          2:0
                                             in IPdatagram processed by COE (Rx)                                          */
    DescRxIpPayloadUnknown = 0x00000000,    /* Unknown or didnot process IP payload                                         */
    DescRxIpPayloadUDP    = 0x00000001,     /* UDP                                                                          */
    DescRxIpPayloadTCP    = 0x00000002,     /* TCP                                                                          */
    DescRxIpPayloadICMP   = 0x00000003,     /* ICMP                                                                         */

};

#else
/*

********** Default Descritpor structure  ****************************
DmaRxBaseAddr     = 0x000C,   CSR3 - Receive Descriptor list base address
DmaRxBaseAddr is the pointer to the first Rx Descriptors. the Descriptor format in Little endian with a
32 bit Data bus is as shown below

Similarly
DmaTxBaseAddr     = 0x0010,  CSR4 - Transmit Descriptor list base address
DmaTxBaseAddr is the pointer to the first Rx Descriptors. the Descriptor format in Little endian with a
32 bit Data bus is as shown below
          --------------------------------------------------------------------
    RDES0/TDES0  |OWN (31)| Status                                                   |
 		  --------------------------------------------------------------------
    RDES1/TDES1  | Control Bits | Byte Count Buffer 2 | Byte Count Buffer 1          |
		  --------------------------------------------------------------------
    RDES2/TDES2  |  Buffer 1 Address                                                 |
		  --------------------------------------------------------------------
    RDES3/TDES3  |  Buffer 2 Address / Next Descriptor Address                       |
		  --------------------------------------------------------------------
*/
enum DmaDescriptorStatus    /* status word of DMA descriptor */
{
    DescOwnByDma          = 0x80000000,   /* (OWN)Descriptor is owned by DMA engine            31      RW */

    DescDAFilterFail      = 0x40000000,   /* (AFM)Rx - DA Filter Fail for the rx frame         30 */

    DescFrameLengthMask   = 0x3FFF0000,   /* (FL)Receive descriptor frame length               29:16 */
    DescFrameLengthShift  = 16,

    DescError             = 0x00008000,   /* (ES)Error summary bit  - OR of the follo. bits:   15 */
    /*  DE || OE || IPC || LC || RWT || RE || CE */
    DescRxTruncated       = 0x00004000,   /* (DE)Rx - no more descriptors for receive frame    14 */
    DescSAFilterFail      = 0x00002000,   /* (SAF)Rx - SA Filter Fail for the received frame   13 */
    DescRxLengthError	= 0x00001000,   /* (LE)Rx - frm size not matching with len field       12 */
    DescRxDamaged         = 0x00000800,   /* (OE)Rx - frm was damaged due to buffer overflow   11 */
    DescRxVLANTag         = 0x00000400,   /* (VLAN)Rx - received frame is a VLAN frame         10 */
    DescRxFirst           = 0x00000200,   /* (FS)Rx - first descriptor of the frame             9 */
    DescRxLast            = 0x00000100,   /* (LS)Rx - last descriptor of the frame              8 */
    DescRxLongFrame       = 0x00000080,   /* (Giant Frame)Rx - frame is longer than 1518/1522   7 */
    DescRxCollision       = 0x00000040,   /* (LC)Rx - late collision occurred during reception  6 */
    DescRxFrameEther      = 0x00000020,   /* (FT)Rx - Frame type - Ethernet, otherwise 802.3    5 */
    DescRxWatchdog        = 0x00000010,   /* (RWT)Rx - watchdog timer expired during reception  4 */
    DescRxMiiError        = 0x00000008,   /* (RE)Rx - error reported by MII interface           3 */
    DescRxDribbling       = 0x00000004,   /* (DE)Rx - frame contains non int multiple of 8 bits 2 */
    DescRxCrc             = 0x00000002,   /* (CE)Rx - CRC error                                 1 */
    DescRxMacMatch        = 0x00000001,   /* (RX MAC Address) Rx mac address reg(1 to 15)match  0 */

    //Rx Descriptor Checksum Offload engine (type 2) encoding
    //DescRxPayChkError     = 0x00000001,   /* ()  Rx - Rx Payload Checksum Error                 0 */
    //DescRxIpv4ChkError    = 0x00000080,   /* (IPC CS ERROR)Rx - Ipv4 header checksum error      7 */

    DescRxChkBit0		= 0x00000001,   /* ()  Rx - Rx Payload Checksum Error                 0 */
    DescRxChkBit7		= 0x00000080,   /* (IPC CS ERROR)Rx - Ipv4 header checksum error      7 */
    DescRxChkBit5		= 0x00000020,   /* (FT)Rx - Frame type - Ethernet, otherwise 802.3    5 */

    DescTxIpv4ChkError    = 0x00010000,   /* (IHE) Tx Ip header error                           16 */
    DescTxTimeout         = 0x00004000,   /* (JT)Tx - Transmit jabber timeout                   14 */
    DescTxFrameFlushed    = 0x00002000,   /* (FF)Tx - DMA/MTL flushed the frame due to SW flush 13 */
    DescTxPayChkError     = 0x00001000,   /* (PCE) Tx Payload checksum Error                    12 */
    DescTxLostCarrier     = 0x00000800,   /* (LC)Tx - carrier lost during tramsmission          11 */
    DescTxNoCarrier       = 0x00000400,   /* (NC)Tx - no carrier signal from the tranceiver     10 */
    DescTxLateCollision   = 0x00000200,   /* (LC)Tx - transmission aborted due to collision      9 */
    DescTxExcCollisions   = 0x00000100,   /* (EC)Tx - transmission aborted after 16 collisions   8 */
    DescTxVLANFrame       = 0x00000080,   /* (VF)Tx - VLAN-type frame                            7 */

    DescTxCollMask        = 0x00000078,   /* (CC)Tx - Collision count                           6:3 */
    DescTxCollShift       = 3,

    DescTxExcDeferral     = 0x00000004,   /* (ED)Tx - excessive deferral                          2 */
    DescTxUnderflow       = 0x00000002,   /* (UF)Tx - late data arrival from the memory           1 */
    DescTxDeferred        = 0x00000001,   /* (DB)Tx - frame transmision deferred                  0 */

    /*
    This explains the RDES1/TDES1 bits layout
    	--------------------------------------------------------------------
        RDES1/TDES1  | Control Bits | Byte Count Buffer 2 | Byte Count Buffer 1   |
    	--------------------------------------------------------------------
    */
    //DmaDescriptorLength     length word of DMA descriptor

    DescTxIntEnable       = 0x80000000,   /* (IC)Tx - interrupt on completion					31		*/
    DescTxLast            = 0x40000000,   /* (LS)Tx - Last segment of the frame					30		*/
    DescTxFirst           = 0x20000000,   /* (FS)Tx - First segment of the frame				29		*/
    DescTxDisableCrc      = 0x04000000,   /* (DC)Tx - Add CRC disabled (first segment only)		26		*/

    RxDisIntCompl		= 0x80000000,	/* (Disable Rx int on completion)						31		*/
    RxDescEndOfRing       = 0x02000000,   /* (TER)End of descriptors ring								*/
    RxDescChain           = 0x01000000,   /* (TCH)Second buffer address is chain address		24		*/

    DescTxDisablePadd	= 0x00800000,   /* (DP)disable padding, added by - reyaz				23		*/

    TxDescEndOfRing       = 0x02000000,   /* (TER)End of descriptors ring								*/
    TxDescChain           = 0x01000000,   /* (TCH)Second buffer address is chain address		24		*/

    DescSize2Mask         = 0x003FF800,   /* (TBS2) Buffer 2 size								21:11	*/
    DescSize2Shift        = 11,
    DescSize1Mask         = 0x000007FF,   /* (TBS1) Buffer 1 size								10:0	*/
    DescSize1Shift        = 0,


    DescTxCisMask  	= 0x18000000,   /* Tx checksum offloading control mask			28:27	*/
    DescTxCisBypass	= 0x00000000,   /* Checksum bypass */
    DescTxCisIpv4HdrCs	= 0x08000000,/* IPv4 header checksum */
    DescTxCisTcpOnlyCs    = 0x10000000,	/* TCP/UDP/ICMP checksum. Pseudo header checksum is assumed to be present */
    DescTxCisTcpPseudoCs  = 0x18000000,	/* TCP/UDP/ICMP checksum fully in hardware including pseudo header */
};
#endif

// Rx Descriptor COE type2 encoding
enum RxDescCOEEncode
{
    RxLenLT600			= 0,	/* Bit(5:7:0)=>0 IEEE 802.3 type frame Length field is Lessthan 0x0600 */
    RxIpHdrPayLoadChkBypass	= 1,	/* Bit(5:7:0)=>1 Payload & Ip header checksum bypassed (unsuppported payload) */
    RxIpHdrPayLoadRes		= 2,	/* Bit(5:7:0)=>2 Reserved */
    RxChkBypass			= 3,	/* Bit(5:7:0)=>3 Neither IPv4 nor IPV6. So checksum bypassed */
    RxNoChkError			= 4,	/* Bit(5:7:0)=>4 No IPv4/IPv6 Checksum error detected */
    RxPayLoadChkError		= 5,	/* Bit(5:7:0)=>5 Payload checksum error detected for Ipv4/Ipv6 frames */
    RxIpHdrChkError		= 6,	/* Bit(5:7:0)=>6 Ip header checksum error detected for Ipv4 frames */
    RxIpHdrPayLoadChkError	= 7,	/* Bit(5:7:0)=>7 Payload & Ip header checksum error detected for Ipv4/Ipv6 frames */
};

/**********************************************************
 * DMA engine interrupt handling functions
 **********************************************************/

enum synopGMACDmaIntEnum  /* Intrerrupt types */
{
    synopGMACDmaRxNormal   = 0x01,   /* normal receiver interrupt */
    synopGMACDmaRxAbnormal = 0x02,   /* abnormal receiver interrupt */
    synopGMACDmaRxStopped  = 0x04,   /* receiver stopped */
    synopGMACDmaTxNormal   = 0x08,   /* normal transmitter interrupt */
    synopGMACDmaTxAbnormal = 0x10,   /* abnormal transmitter interrupt */
    synopGMACDmaTxStopped  = 0x20,   /* transmitter stopped */
    synopGMACDmaError      = 0x80,   /* Dma engine error */

#ifdef AVB_SUPPORT
    synopGMADmaSlotCounter  = 0x40,  /* Dma SlotCounter interrupt mask for Channel1 and Channel2*/
#endif
};


/**********************************************************
 * Initial register values
 **********************************************************/
enum InitialRegisters
{
    /* Full-duplex mode with perfect filter on */
    GmacConfigInitFdx1000   = GmacWatchdogEnable | GmacJabberEnable         | GmacFrameBurstEnable | GmacJumboFrameDisable
    | GmacSelectGmii     | GmacEnableRxOwn          | GmacLoopbackOff
    | GmacFullDuplex     | GmacRetryEnable          | GmacPadCrcStripDisable
    | GmacBackoffLimit0  | GmacDeferralCheckDisable | GmacTxEnable          | GmacRxEnable,

    /* Full-duplex mode with perfect filter on */
    GmacConfigInitFdx110    = GmacWatchdogEnable | GmacJabberEnable         | GmacFrameBurstEnable  | GmacJumboFrameDisable
    | GmacSelectMii      | GmacEnableRxOwn          | GmacLoopbackOff
    | GmacFullDuplex     | GmacRetryEnable          | GmacPadCrcStripDisable
    | GmacBackoffLimit0  | GmacDeferralCheckDisable | GmacTxEnable          | GmacRxEnable,

    /* Full-duplex mode */
    // CHANGED: Pass control config, dest addr filter normal, added source address filter, multicast & unicast
    // Hash filter.
    /*                        = GmacFilterOff         | GmacPassControlOff | GmacBroadcastEnable */
    GmacFrameFilterInitFdx = GmacFilterOn          | GmacPassControl0   | GmacBroadcastEnable |  GmacSrcAddrFilterDisable
    | GmacMulticastFilterOn | GmacDestAddrFilterNor | GmacMcastHashFilterOff
    | GmacPromiscuousModeOff | GmacUcastHashFilterOff,

    /* Full-duplex mode */
    GmacFlowControlInitFdx = GmacUnicastPauseFrameOff | GmacRxFlowControlEnable | GmacTxFlowControlEnable,

    /* Full-duplex mode */
    GmacGmiiAddrInitFdx    = GmiiCsrClk2,


    /* Half-duplex mode with perfect filter on */
    // CHANGED: Removed Endian configuration, added single bit config for PAD/CRC strip,
    /*| GmacSelectMii      | GmacLittleEndian         | GmacDisableRxOwn      | GmacLoopbackOff*/
    GmacConfigInitHdx1000  = GmacWatchdogEnable | GmacJabberEnable         | GmacFrameBurstEnable  | GmacJumboFrameDisable
    | GmacSelectGmii     | GmacDisableRxOwn         | GmacLoopbackOff
    | GmacHalfDuplex     | GmacRetryEnable          | GmacPadCrcStripDisable
    | GmacBackoffLimit0  | GmacDeferralCheckDisable | GmacTxEnable          | GmacRxEnable,

    /* Half-duplex mode with perfect filter on */
    GmacConfigInitHdx110   = GmacWatchdogEnable | GmacJabberEnable         | GmacFrameBurstEnable  | GmacJumboFrameDisable
    | GmacSelectMii      | GmacDisableRxOwn         | GmacLoopbackOff
    | GmacHalfDuplex     | GmacRetryEnable          | GmacPadCrcStripDisable
    | GmacBackoffLimit0  | GmacDeferralCheckDisable | GmacTxEnable          | GmacRxEnable,

    /* Half-duplex mode */
    GmacFrameFilterInitHdx = GmacFilterOn          | GmacPassControl0        | GmacBroadcastEnable | GmacSrcAddrFilterDisable
    | GmacMulticastFilterOn | GmacDestAddrFilterNor   | GmacMcastHashFilterOff
    | GmacUcastHashFilterOff | GmacPromiscuousModeOff,

    /* Half-duplex mode */
    GmacFlowControlInitHdx = GmacUnicastPauseFrameOff | GmacRxFlowControlDisable | GmacTxFlowControlDisable,

    /* Half-duplex mode */
    GmacGmiiAddrInitHdx    = GmiiCsrClk2,



    /**********************************************
    *DMA configurations
    **********************************************/

    DmaBusModeInit         = DmaFixedBurstEnable |   DmaBurstLength8   | DmaDescriptorSkip2       | DmaResetOff,
    //   DmaBusModeInit         = DmaFixedBurstEnable |   DmaBurstLength8   | DmaDescriptorSkip4       | DmaResetOff,

    /* 1000 Mb/s mode */
    DmaControlInit1000     = DmaStoreAndForward,//       | DmaTxSecondFrame ,

    /* 100 Mb/s mode */
    DmaControlInit100      = DmaStoreAndForward,

    /* 10 Mb/s mode */
    DmaControlInit10       = DmaStoreAndForward,

    /* Interrupt groups */
    DmaIntErrorMask         = DmaIntBusError,           /* Error */
    DmaIntRxAbnMask         = DmaIntRxNoBuffer,         /* receiver abnormal interrupt */
    DmaIntRxNormMask        = DmaIntRxCompleted,        /* receiver normal interrupt   */
    DmaIntRxStoppedMask     = DmaIntRxStopped,          /* receiver stopped */
    DmaIntTxAbnMask         = DmaIntTxUnderflow,        /* transmitter abnormal interrupt */
    DmaIntTxNormMask        = DmaIntTxCompleted,        /* transmitter normal interrupt */
    DmaIntTxStoppedMask     = DmaIntTxStopped,          /* transmitter stopped */

    DmaIntEnable            = DmaIeNormal     | DmaIeAbnormal    | DmaIntErrorMask
    | DmaIntRxAbnMask | DmaIntRxNormMask | DmaIntRxStoppedMask
    | DmaIntTxAbnMask | DmaIntTxNormMask | DmaIntTxStoppedMask 
    | DmaIeTxNoBuffer ,
    DmaIntDisable           = 0,
};


/**********************************************************
 * Mac Management Counters (MMC)
 **********************************************************/

enum MMC_ENABLE
{
    GmacMmcCntrl			= 0x0100,	/* mmc control for operating mode of MMC						*/
    GmacMmcIntrRx			= 0x0104,	/* maintains interrupts generated by rx counters					*/
    GmacMmcIntrTx			= 0x0108,	/* maintains interrupts generated by tx counters					*/
    GmacMmcIntrMaskRx		= 0x010C,	/* mask for interrupts generated from rx counters					*/
    GmacMmcIntrMaskTx		= 0x0110,	/* mask for interrupts generated from tx counters					*/
};

enum MMC_TX
{
    GmacMmcTxOctetCountGb		= 0x0114,	/*Bytes Tx excl. of preamble and retried bytes     (Good or Bad)			*/
    GmacMmcTxFrameCountGb		= 0x0118,	/*Frames Tx excl. of retried frames	           (Good or Bad)			*/
    GmacMmcTxBcFramesG		= 0x011C,	/*Broadcast Frames Tx 				   (Good)				*/
    GmacMmcTxMcFramesG		= 0x0120,	/*Multicast Frames Tx				   (Good)				*/

    GmacMmcTx64OctetsGb		= 0x0124,	/*Tx with len 64 bytes excl. of pre and retried    (Good or Bad)			*/
    GmacMmcTx65To127OctetsGb	= 0x0128,	/*Tx with len >64 bytes <=127 excl. of pre and retried    (Good or Bad)			*/
    GmacMmcTx128To255OctetsGb	= 0x012C,	/*Tx with len >128 bytes <=255 excl. of pre and retried   (Good or Bad)			*/
    GmacMmcTx256To511OctetsGb	= 0x0130,	/*Tx with len >256 bytes <=511 excl. of pre and retried   (Good or Bad)			*/
    GmacMmcTx512To1023OctetsGb	= 0x0134,	/*Tx with len >512 bytes <=1023 excl. of pre and retried  (Good or Bad)			*/
    GmacMmcTx1024ToMaxOctetsGb	= 0x0138,	/*Tx with len >1024 bytes <=MaxSize excl. of pre and retried (Good or Bad)		*/

    GmacMmcTxUcFramesGb		= 0x013C,	/*Unicast Frames Tx 					 (Good or Bad)			*/
    GmacMmcTxMcFramesGb		= 0x0140,	/*Multicast Frames Tx				   (Good and Bad)			*/
    GmacMmcTxBcFramesGb		= 0x0144,	/*Broadcast Frames Tx 				   (Good and Bad)			*/
    GmacMmcTxUnderFlowError		= 0x0148,	/*Frames aborted due to Underflow error							*/
    GmacMmcTxSingleColG		= 0x014C,	/*Successfully Tx Frames after singel collision in Half duplex mode			*/
    GmacMmcTxMultiColG		= 0x0150,	/*Successfully Tx Frames after more than singel collision in Half duplex mode		*/
    GmacMmcTxDeferred		= 0x0154,	/*Successfully Tx Frames after a deferral in Half duplex mode				*/
    GmacMmcTxLateCol		= 0x0158,	/*Frames aborted due to late collision error						*/
    GmacMmcTxExessCol		= 0x015C,	/*Frames aborted due to excessive (16) collision errors					*/
    GmacMmcTxCarrierError		= 0x0160,	/*Frames aborted due to carrier sense error (No carrier or Loss of carrier)		*/
    GmacMmcTxOctetCountG		= 0x0164,	/*Bytes Tx excl. of preamble and retried bytes     (Good) 				*/
    GmacMmcTxFrameCountG		= 0x0168,	/*Frames Tx 				           (Good)				*/
    GmacMmcTxExessDef		= 0x016C,	/*Frames aborted due to excessive deferral errors (deferred for more than 2 max-sized frame times)*/

    GmacMmcTxPauseFrames		= 0x0170,	/*Number of good pause frames Tx.							*/
    GmacMmcTxVlanFramesG		= 0x0174,	/*Number of good Vlan frames Tx excl. retried frames					*/
};
enum MMC_RX
{
    GmacMmcRxFrameCountGb		= 0x0180,	/*Frames Rx 				           (Good or Bad)			*/
    GmacMmcRxOctetCountGb		= 0x0184,	/*Bytes Rx excl. of preamble and retried bytes     (Good or Bad)			*/
    GmacMmcRxOctetCountG		= 0x0188,	/*Bytes Rx excl. of preamble and retried bytes     (Good) 				*/
    GmacMmcRxBcFramesG		= 0x018C,	/*Broadcast Frames Rx 				   (Good)				*/
    GmacMmcRxMcFramesG		= 0x0190,	/*Multicast Frames Rx				   (Good)				*/

    GmacMmcRxCrcError		= 0x0194,	/*Number of frames received with CRC error						*/
    GmacMmcRxAlignError		= 0x0198,	/*Number of frames received with alignment (dribble) error. Only in 10/100mode		*/
    GmacMmcRxRuntError		= 0x019C,	/*Number of frames received with runt (<64 bytes and CRC error) error			*/
    GmacMmcRxJabberError		= 0x01A0,	/*Number of frames rx with jabber (>1518/1522 or >9018/9022 and CRC) 			*/
    GmacMmcRxUnderSizeG		= 0x01A4,	/*Number of frames received with <64 bytes without any error				*/
    GmacMmcRxOverSizeG		= 0x01A8,	/*Number of frames received with >1518/1522 bytes without any error			*/

    GmacMmcRx64OctetsGb		= 0x01AC,	/*Rx with len 64 bytes excl. of pre and retried    (Good or Bad)			*/
    GmacMmcRx65To127OctetsGb	= 0x01B0,	/*Rx with len >64 bytes <=127 excl. of pre and retried    (Good or Bad)			*/
    GmacMmcRx128To255OctetsGb	= 0x01B4,	/*Rx with len >128 bytes <=255 excl. of pre and retried   (Good or Bad)			*/
    GmacMmcRx256To511OctetsGb	= 0x01B8,	/*Rx with len >256 bytes <=511 excl. of pre and retried   (Good or Bad)			*/
    GmacMmcRx512To1023OctetsGb	= 0x01BC,	/*Rx with len >512 bytes <=1023 excl. of pre and retried  (Good or Bad)			*/
    GmacMmcRx1024ToMaxOctetsGb	= 0x01C0,	/*Rx with len >1024 bytes <=MaxSize excl. of pre and retried (Good or Bad)		*/

    GmacMmcRxUcFramesG		= 0x01C4,	/*Unicast Frames Rx 					 (Good)				*/
    GmacMmcRxLengthError		= 0x01C8,	/*Number of frames received with Length type field != frame size			*/
    GmacMmcRxOutOfRangeType		= 0x01CC,	/*Number of frames received with length field != valid frame size			*/

    GmacMmcRxPauseFrames		= 0x01D0,	/*Number of good pause frames Rx.							*/
    GmacMmcRxFifoOverFlow		= 0x01D4,	/*Number of missed rx frames due to FIFO overflow					*/
    GmacMmcRxVlanFramesGb		= 0x01D8,	/*Number of good Vlan frames Rx 							*/

    GmacMmcRxWatchdobError		= 0x01DC,	/*Number of frames rx with error due to watchdog timeout error				*/
};

enum MMC_IP_RELATED
{
    GmacMmcRxIpcIntrMask		= 0x0200,	/*Maintains the mask for interrupt generated from rx IPC statistic counters 		*/
    GmacMmcRxIpcIntr		= 0x0208,	/*Maintains the interrupt that rx IPC statistic counters generate			*/

    GmacMmcRxIpV4FramesG		= 0x0210,	/*Good IPV4 datagrams received								*/
    GmacMmcRxIpV4HdrErrFrames	= 0x0214,	/*Number of IPV4 datagrams received with header errors					*/
    GmacMmcRxIpV4NoPayFrames	= 0x0218,	/*Number of IPV4 datagrams received which didnot have TCP/UDP/ICMP payload		*/
    GmacMmcRxIpV4FragFrames		= 0x021C,	/*Number of IPV4 datagrams received with fragmentation					*/
    GmacMmcRxIpV4UdpChkDsblFrames	= 0x0220,	/*Number of IPV4 datagrams received that had a UDP payload checksum disabled		*/

    GmacMmcRxIpV6FramesG		= 0x0224,	/*Good IPV6 datagrams received								*/
    GmacMmcRxIpV6HdrErrFrames	= 0x0228,	/*Number of IPV6 datagrams received with header errors					*/
    GmacMmcRxIpV6NoPayFrames	= 0x022C,	/*Number of IPV6 datagrams received which didnot have TCP/UDP/ICMP payload		*/

    GmacMmcRxUdpFramesG		= 0x0230,	/*Number of good IP datagrams with good UDP payload					*/
    GmacMmcRxUdpErrorFrames		= 0x0234,	/*Number of good IP datagrams with UDP payload having checksum error			*/

    GmacMmcRxTcpFramesG		= 0x0238,	/*Number of good IP datagrams with good TDP payload					*/
    GmacMmcRxTcpErrorFrames		= 0x023C,	/*Number of good IP datagrams with TCP payload having checksum error			*/

    GmacMmcRxIcmpFramesG		= 0x0240,	/*Number of good IP datagrams with good Icmp payload					*/
    GmacMmcRxIcmpErrorFrames	= 0x0244,	/*Number of good IP datagrams with Icmp payload having checksum error			*/

    GmacMmcRxIpV4OctetsG		= 0x0250,	/*Good IPV4 datagrams received excl. Ethernet hdr,FCS,Pad,Ip Pad bytes			*/
    GmacMmcRxIpV4HdrErrorOctets	= 0x0254,	/*Number of bytes in IPV4 datagram with header errors					*/
    GmacMmcRxIpV4NoPayOctets	= 0x0258,	/*Number of bytes in IPV4 datagram with no TCP/UDP/ICMP payload				*/
    GmacMmcRxIpV4FragOctets		= 0x025C,	/*Number of bytes received in fragmented IPV4 datagrams 				*/
    GmacMmcRxIpV4UdpChkDsblOctets	= 0x0260,	/*Number of bytes received in UDP segment that had UDP checksum disabled		*/

    GmacMmcRxIpV6OctetsG		= 0x0264,	/*Good IPV6 datagrams received excl. Ethernet hdr,FCS,Pad,Ip Pad bytes			*/
    GmacMmcRxIpV6HdrErrorOctets	= 0x0268,	/*Number of bytes in IPV6 datagram with header errors					*/
    GmacMmcRxIpV6NoPayOctets	= 0x026C,	/*Number of bytes in IPV6 datagram with no TCP/UDP/ICMP payload				*/

    GmacMmcRxUdpOctetsG		= 0x0270,	/*Number of bytes in IP datagrams with good UDP payload					*/
    GmacMmcRxUdpErrorOctets		= 0x0274,	/*Number of bytes in IP datagrams with UDP payload having checksum error		*/

    GmacMmcRxTcpOctetsG		= 0x0278,	/*Number of bytes in IP datagrams with good TDP payload					*/
    GmacMmcRxTcpErrorOctets		= 0x027C,	/*Number of bytes in IP datagrams with TCP payload having checksum error		*/

    GmacMmcRxIcmpOctetsG		= 0x0280,	/*Number of bytes in IP datagrams with good Icmp payload				*/
    GmacMmcRxIcmpErrorOctets	= 0x0284,	/*Number of bytes in IP datagrams with Icmp payload having checksum error		*/
};


enum MMC_CNTRL_REG_BIT_DESCRIPTIONS
{
    GmacMmcCounterFreeze		= 0x00000008,		/* when set MMC counters freeze to current value				*/
    GmacMmcCounterResetOnRead	= 0x00000004,		/* when set MMC counters will be reset to 0 after read				*/
    GmacMmcCounterStopRollover	= 0x00000002,		/* when set counters will not rollover after max value				*/
    GmacMmcCounterReset		= 0x00000001,		/* when set all counters wil be reset (automatically cleared after 1 clk)	*/

};

enum MMC_RX_INTR_MASK_AND_STATUS_BIT_DESCRIPTIONS
{
    GmacMmcRxWDInt			= 0x00800000,		/* set when rxwatchdog error reaches half of max value				*/
    GmacMmcRxVlanInt		= 0x00400000,		/* set when GmacMmcRxVlanFramesGb counter reaches half of max value		*/
    GmacMmcRxFifoOverFlowInt	= 0x00200000,		/* set when GmacMmcRxFifoOverFlow counter reaches half of max value		*/
    GmacMmcRxPauseFrameInt		= 0x00100000,		/* set when GmacMmcRxPauseFrames counter reaches half of max value		*/
    GmacMmcRxOutOfRangeInt		= 0x00080000,		/* set when GmacMmcRxOutOfRangeType counter reaches half of max value		*/
    GmacMmcRxLengthErrorInt		= 0x00040000,		/* set when GmacMmcRxLengthError counter reaches half of max value		*/
    GmacMmcRxUcFramesInt		= 0x00020000,		/* set when GmacMmcRxUcFramesG counter reaches half of max value		*/
    GmacMmcRx1024OctInt		= 0x00010000,		/* set when GmacMmcRx1024ToMaxOctetsGb counter reaches half of max value	*/
    GmacMmcRx512OctInt		= 0x00008000,		/* set when GmacMmcRx512To1023OctetsGb counter reaches half of max value	*/
    GmacMmcRx256OctInt		= 0x00004000,		/* set when GmacMmcRx256To511OctetsGb counter reaches half of max value		*/
    GmacMmcRx128OctInt		= 0x00002000,		/* set when GmacMmcRx128To255OctetsGb counter reaches half of max value		*/
    GmacMmcRx65OctInt		= 0x00001000,		/* set when GmacMmcRx65To127OctetsG counter reaches half of max value		*/
    GmacMmcRx64OctInt		= 0x00000800,		/* set when GmacMmcRx64OctetsGb counter reaches half of max value		*/
    GmacMmcRxOverSizeInt		= 0x00000400,		/* set when GmacMmcRxOverSizeG counter reaches half of max value		*/
    GmacMmcRxUnderSizeInt		= 0x00000200,		/* set when GmacMmcRxUnderSizeG counter reaches half of max value		*/
    GmacMmcRxJabberErrorInt		= 0x00000100,		/* set when GmacMmcRxJabberError counter reaches half of max value		*/
    GmacMmcRxRuntErrorInt		= 0x00000080,		/* set when GmacMmcRxRuntError counter reaches half of max value		*/
    GmacMmcRxAlignErrorInt		= 0x00000040,		/* set when GmacMmcRxAlignError counter reaches half of max value		*/
    GmacMmcRxCrcErrorInt		= 0x00000020,		/* set when GmacMmcRxCrcError counter reaches half of max value			*/
    GmacMmcRxMcFramesInt		= 0x00000010,		/* set when GmacMmcRxMcFramesG counter reaches half of max value		*/
    GmacMmcRxBcFramesInt		= 0x00000008,		/* set when GmacMmcRxBcFramesG counter reaches half of max value		*/
    GmacMmcRxOctetGInt		= 0x00000004,		/* set when GmacMmcRxOctetCountG counter reaches half of max value		*/
    GmacMmcRxOctetGbInt		= 0x00000002,		/* set when GmacMmcRxOctetCountGb counter reaches half of max value		*/
    GmacMmcRxFrameInt		= 0x00000001,		/* set when GmacMmcRxFrameCountGb counter reaches half of max value		*/
};

enum MMC_TX_INTR_MASK_AND_STATUS_BIT_DESCRIPTIONS
{

    GmacMmcTxVlanInt		= 0x01000000,		/* set when GmacMmcTxVlanFramesG counter reaches half of max value		*/
    GmacMmcTxPauseFrameInt		= 0x00800000,		/* set when GmacMmcTxPauseFrames counter reaches half of max value		*/
    GmacMmcTxExessDefInt		= 0x00400000,		/* set when GmacMmcTxExessDef counter reaches half of max value			*/
    GmacMmcTxFrameInt		= 0x00200000,		/* set when GmacMmcTxFrameCount counter reaches half of max value		*/
    GmacMmcTxOctetInt		= 0x00100000,		/* set when GmacMmcTxOctetCountG counter reaches half of max value		*/
    GmacMmcTxCarrierErrorInt	= 0x00080000,		/* set when GmacMmcTxCarrierError counter reaches half of max value		*/
    GmacMmcTxExessColInt		= 0x00040000,		/* set when GmacMmcTxExessCol counter reaches half of max value			*/
    GmacMmcTxLateColInt		= 0x00020000,		/* set when GmacMmcTxLateCol counter reaches half of max value			*/
    GmacMmcTxDeferredInt		= 0x00010000,		/* set when GmacMmcTxDeferred counter reaches half of max value			*/
    GmacMmcTxMultiColInt		= 0x00008000,		/* set when GmacMmcTxMultiColG counter reaches half of max value		*/
    GmacMmcTxSingleCol		= 0x00004000,		/* set when GmacMmcTxSingleColG	counter reaches half of max value		*/
    GmacMmcTxUnderFlowErrorInt	= 0x00002000,		/* set when GmacMmcTxUnderFlowError counter reaches half of max value		*/
    GmacMmcTxBcFramesGbInt 		= 0x00001000,		/* set when GmacMmcTxBcFramesGb	counter reaches half of max value		*/
    GmacMmcTxMcFramesGbInt 		= 0x00000800,		/* set when GmacMmcTxMcFramesGb	counter reaches half of max value		*/
    GmacMmcTxUcFramesInt 		= 0x00000400,		/* set when GmacMmcTxUcFramesGb counter reaches half of max value		*/
    GmacMmcTx1024OctInt 		= 0x00000200,		/* set when GmacMmcTx1024ToMaxOctetsGb counter reaches half of max value	*/
    GmacMmcTx512OctInt 		= 0x00000100,		/* set when GmacMmcTx512To1023OctetsGb counter reaches half of max value	*/
    GmacMmcTx256OctInt 		= 0x00000080,		/* set when GmacMmcTx256To511OctetsGb counter reaches half of max value		*/
    GmacMmcTx128OctInt 		= 0x00000040,		/* set when GmacMmcTx128To255OctetsGb counter reaches half of max value		*/
    GmacMmcTx65OctInt 		= 0x00000020,		/* set when GmacMmcTx65To127OctetsGb counter reaches half of max value		*/
    GmacMmcTx64OctInt 		= 0x00000010,		/* set when GmacMmcTx64OctetsGb	counter reaches half of max value		*/
    GmacMmcTxMcFramesInt 		= 0x00000008,		/* set when GmacMmcTxMcFramesG counter reaches half of max value		*/
    GmacMmcTxBcFramesInt 		= 0x00000004,		/* set when GmacMmcTxBcFramesG counter reaches half of max value		*/
    GmacMmcTxFrameGbInt 		= 0x00000002,		/* set when GmacMmcTxFrameCountGb counter reaches half of max value		*/
    GmacMmcTxOctetGbInt 		= 0x00000001,		/* set when GmacMmcTxOctetCountGb counter reaches half of max value		*/

};


/**********************************************************
 * Power Management (PMT) Block
 **********************************************************/

/**
  * PMT supports the reception of network (remote) wake-up frames and Magic packet frames.
  * It generates interrupts for wake-up frames and Magic packets received by GMAC.
  * PMT sits in Rx path and is enabled with remote wake-up frame enable and Magic packet enable.
  * These enable are in PMT control and Status register and are programmed by apllication.
  *
  * When power down mode is enabled in PMT, all rx frames are dropped by the core. Core comes
  * out of power down mode only when either Magic packe tor a Remote wake-up frame is received
  * and the corresponding detection is enabled.
  *
  * Driver need not be modified to support this feature. Only Api to put the device in to power
  * down mode is sufficient
  */

#define WAKEUP_REG_LENGTH	8 				/*This is the reg length for wake up register configuration*/

enum GmacPmtCtrlStatusBitDefinition
{
    GmacPmtFrmFilterPtrReset	= 0x80000000,		/* when set remote wake-up frame filter register pointer to 3'b000 */
    GmacPmtGlobalUnicast		= 0x00000200,		/* When set enables any unicast packet to be a wake-up frame       */
    GmacPmtWakeupFrameReceived	= 0x00000040,		/* Wake up frame received					   */
    GmacPmtMagicPktReceived		= 0x00000020,		/* Magic Packet received					   */
    GmacPmtWakeupFrameEnable	= 0x00000004,		/* Wake-up frame enable						   */
    GmacPmtMagicPktEnable		= 0x00000002,		/* Magic packet enable						   */
    GmacPmtPowerDown		= 0x00000001,		/* Power Down							   */
};




/**********************************************************
 * IEEE 1588-2008 Precision Time Protocol (PTP) Support
 **********************************************************/
enum PTPMessageType
{
    SYNC        	   = 0x0,
    Delay_Req    	   = 0x1,
    Pdelay_Req             = 0x2,
    Pdelay_Resp            = 0x3,
    Follow_up              = 0x8,
    Delay_Resp             = 0x9,
    Pdelay_Resp_Follow_Up  = 0xA,
    Announce               = 0xB,
    Signaling              = 0xC,
    Management             = 0xD,
};



typedef struct TimeStampStruct
{
    u32   TSversion;      /* PTP Version 1 or PTP version2                                                                          */
    u32   TSmessagetype;  /* Message type associated with this time stamp                                                           */

    u16   TShighest16;    /* Highest 16 bit time stamp value, Valid onley when ADV_TIME_HIGH_WORD configured in corekit		  */
    u32   TSupper32;      /* Most significant 32 bit time stamp value								  */
    u32   TSlower32;      /* Least Significat 32 bit time stamp value								  */

} TimeStamp;


/**
* IEEE 1588-2008 is the optional module to support Ethernet frame time stamping.
* Sixty four (+16) bit time stamps are given in each frames transmit and receive status.
* The driver assumes the following
*  1. "IEEE 1588 Time Stamping" "TIME_STAMPING"is ENABLED in corekit
*  2. "IEEE 1588 External Time Stamp Input Enable" "EXT_TIME_STAMPING" is DISABLED in corekit
*  3. "IEEE 1588 Advanced Time Stamp support" "ADV_TIME_STAMPING" is ENABLED in corekit
*  4. "IEEE 1588 Higher Word Register Enable" "ADV_TIME_HIGH_WORD" is ENABLED in corekit
*/

/* GmacTSControl  = 0x0700,   Controls the Timestamp update logic  : only when IEEE 1588 time stamping is enabled in corekit         */
enum GmacTSControlReg
{
    GmacTSENMACADDR	  = 0x00040000,     /* Enable Mac Addr for PTP filtering     18            RW         0     */

    GmacTSCLKTYPE		  = 0x00030000,     /* Select the type of clock node         17:16         RW         00    */
    /*
        TSCLKTYPE        TSMSTRENA      TSEVNTENA         Messages for wihich TS snapshot is taken
         00/01                X             0              SYNC, FOLLOW_UP, DELAY_REQ, DELAY_RESP
         00/01                1             0              DELAY_REQ
         00/01                0             1              SYNC
          10                  NA            0              SYNC, FOLLOW_UP, DELAY_REQ, DELAY_RESP
          10                  NA            1              SYNC, FOLLOW_UP
          11                  NA            0              SYNC, FOLLOW_UP, DELAY_REQ, DELAY_RESP, PDELAY_REQ, PDELAY_RESP
          11                  NA            1              SYNC, PDELAY_REQ, PDELAY_RESP
    */
    GmacTSOrdClk		  = 0x00000000,	    /* 00=> Ordinary clock*/
    GmacTSBouClk		  = 0x00010000,	    /* 01=> Boundary clock*/
    GmacTSEtoEClk		  = 0x00020000,	    /* 10=> End-to-End transparent clock*/
    GmacTSPtoPClk		  = 0x00030000,	    /* 11=> P-to-P transparent clock*/

    GmacTSMSTRENA		  = 0x00008000,	    /* Ena TS Snapshot for Master Messages   15            RW         0     */
    GmacTSEVNTENA		  = 0x00004000,	    /* Ena TS Snapshot for Event Messages    14            RW         0     */
    GmacTSIPV4ENA		  = 0x00002000,	    /* Ena TS snapshot for IPv4              13            RW         1     */
    GmacTSIPV6ENA		  = 0x00001000,	    /* Ena TS snapshot for IPv6              12            RW         0     */
    GmacTSIPENA		  = 0x00000800,	    /* Ena TS snapshot for PTP over E'net    11            RW         0     */
    GmacTSVER2ENA		  = 0x00000400,	    /* Ena PTP snooping for version 2        10            RW         0     */

    GmacTSCTRLSSR           = 0x00000200,      /* Digital or Binary Rollover           9             RW         0     */

    GmacTSENALL             = 0x00000100,      /* Enable TS fro all frames (Ver2 only) 8             RW         0     */

    GmacTSADDREG		  = 0x00000020,	     /* Addend Register Update		     5             RW_SC      0     */
    GmacTSUPDT		  = 0x00000008,	     /* Time Stamp Update		     3             RW_SC      0     */
    GmacTSINT		  = 0x00000004,	     /* Time Atamp Initialize		     2             RW_SC      0     */

    GmacTSTRIG		  = 0x00000010,	     /* Time stamp interrupt Trigger Enable  4             RW_SC      0     */

    GmacTSCFUPDT		  = 0x00000002,	     /* Time Stamp Fine/Coarse		     1             RW         0     */
    GmacTSCUPDTCoarse	  = 0x00000000,	     /* 0=> Time Stamp update method is coarse			            */
    GmacTSCUPDTFine	  = 0x00000002,	     /* 1=> Time Stamp update method is fine				    */

    GmacTSENA		  = 0x00000001,      /* Time Stamp Enable                    0             RW         0     */
};


/*  GmacTSSubSecIncr     	  = 0x0704,   8 bit value by which sub second register is incremented     : only when IEEE 1588 time stamping without external timestamp input */
enum GmacTSSubSecIncrReg
{
    GmacSSINCMsk            = 0x000000FF,       /* Only Lower 8 bits are valid bits     7:0           RW         00    */
};

/*  GmacTSLow   	  = 0x070C,   Indicates whether the timestamp low count is positive or negative; for Adv timestamp it is always zero */
enum GmacTSSign
{
    GmacTSSign              = 0x80000000,      /* PSNT                                  31            RW          0    */
    GmacTSPositive          = 0x00000000,
    GmacTSNegative          = 0x80000000,
};

/*GmacTargetTimeLow   	  = 0x0718,   32 bit nano seconds(MS) to be compared with system time     : only when IEEE 1588 time stamping without external timestamp input */
enum GmacTSLowReg
{
    GmacTSDecThr            = 0x3B9AC9FF,      /*when TSCTRLSSR is set the max value for GmacTargetTimeLowReg and GmacTimeStampLow register is 0x3B9AC9FF at 1ns precision       */
};

/* GmacTSHighWord          = 0x0724,   Time Stamp Higher Word Register (Version 2 only); only lower 16 bits are valid                                                   */
enum GmacTSHighWordReg
{
    GmacTSHighWordMask      = 0x0000FFFF,     /* Time Stamp Higher work register has only lower 16 bits valid			*/
};

/*GmacTSStatus            = 0x0728,   Time Stamp Status Register                                                                                                       */
enum GmacTSStatusReg
{
    GmacTSTargTimeReached   = 0x00000002,     /* Time Stamp Target Time Reached          1             RO          0    */
    GmacTSSecondsOverflow   = 0x00000001,     /* Time Stamp Seconds Overflow             0             RO          0    */
};


static u32 GMAC_Power_down = 0;

/**
* The synopGMAC_wakeup_filter_config3[] is a sample configuration for wake up filter.
* Filter1 is used here
* Filter1 offset is programmed to 50 (0x32)
* Filter1 mask is set to 0x000000FF, indicating First 8 bytes are used by the filter
* Filter1 CRC= 0x7EED this is the CRC computed on data 0x55 0x55 0x55 0x55 0x55 0x55 0x55 0x55
* Refer accompanied software DWC_gmac_crc_example.c for CRC16 generation and how to use the same.
*/
u32 synopGMAC_wakeup_filter_config3[] =
{
    0x00000000,	// For Filter0 CRC is not computed may be it is 0x0000
    0x000000FF,	// For Filter1 CRC is computed on 0,1,2,3,4,5,6,7 bytes from offset
    0x00000000,	// For Filter2 CRC is not computed may be it is 0x0000
    0x00000000, // For Filter3 CRC is not computed may be it is 0x0000
    0x00000100, // Filter 0,2,3 are disabled, Filter 1 is enabled and filtering applies to only unicast packets
    0x00003200, // Filter 0,2,3 (no significance), filter 1 offset is 50 bytes from start of Destination MAC address
    0x7eED0000, // No significance of CRC for Filter0, Filter1 CRC is 0x7EED,
    0x00000000  // No significance of CRC for Filter2 and Filter3
};

typedef enum {
	AK_PULLDOWN_ENABLE = 0,
	AK_PULLDOWN_DISABLE
} T_GPIO_PD_CONFG;

struct ak39_ethernet_priv{
	u8 dev_num;
};


static void gmac_disable_dma_tx(gmac_device *gmacdev);
static void gmac_disable_dma_rx(gmac_device *gmacdev);
static void gmac_mii_sharepin_cfg(void);
static void gmac_rmii_sharepin_cfg(void);
static void smc_desc_set_rx_owner(DmaDesc *desc);


/**
* @brief init share pin , mac controller clock and 25M_phy clock
*
* @author CaoDonghua
* @date 2016-09-26
* @param[in] void
* @return	void
* @retval none
*/
static void gmac_ctrl_init(void)
{
	unsigned long val = 0;
	
	debug("Reset MAC controller!\n");
	ak_soft_reset(AK39_SRESET_MAC);

	debug("DGPIO62 Reset MAC phy!\n");

	// cdh:set gpio0 share as gpio ,and reset phy
	val = REG32(SHARE_PIN_CFG_REG1);
    val &= ~(0x3<<0); 				
    REG32(SHARE_PIN_CFG_REG1) = val;
	
    /* gpio0 for phy reset, POWER ON */
    REG32(0x20170000 + 0x0000) |= (1<<0); /* DGPIO0 output */	
    REG32(0x20170000 + 0x000C) |= (1<<0); /* RST = 1 */	
    debug("..PHY reset delay H ..\n");
	mdelay(5);
	REG32(0x20170000 + 0x000C) &= ~(1<<0); /* RST = 0 */	
	debug("..PHY reset delay L ..\n");
	mdelay(25);
	REG32(AK_PA_GPIOCTRL + 0x000C) |= (1<<0); /* RST = 1 */	
	debug("..PHY reset delay K ..\n");
	mdelay(5);
}

/**
 * @brief The Low level function to clear bits of a register in Hardware.
 *
 * @author CaoDonghua
 * @date 2016-09-26
 * @param[in] pointer to the base of register map
 * @param[in] Offset from the base
 * @param[in] Bit mask to clear bits to logical 0
 * @return  void
 * @retval none
 */
static void gmac_clr_bits(u32 *RegBase, u32 RegOffset, u32 BitPos)
{
    u32 addr = (u32)RegBase + RegOffset;
    u32 data = readl((void *)addr);
    
    data &= (~BitPos);
    writel(data, (void *)addr);

    return;
}

/**
 * @brief The Low level function to set bits of a register in Hardware.
 *
 * @author CaoDonghua
 * @date 2016-09-26
 * @param[in] pointer to the base of register map
 * @param[in] Offset from the base
 * @param[in] Bit mask to set bits to logical 1
 * @return  void
 * @retval none
 */
static void gmac_set_bits(u32 *RegBase, u32 RegOffset, u32 BitPos)
{
    u32 addr = (u32)RegBase + RegOffset;
    u32 data = readl((void *)addr);
    
    data |= BitPos;
    writel(data, (void *)addr);

    return;
}


/**
  * @brief Configures the MMC to stop rollover.
  *
  * Programs MMC interface so that counters will not rollover after reaching maximum value.
  * @author CaoDonghua
  * @date 2016-09-26
  * @param[in] pointer to gmac_device.
  * @return returns void.
  * @retval none
  */
static void gmac_mmc_counters_reset(gmac_device *gmacdev)
{
    gmac_clr_bits((u32 *)gmacdev->MacBase, GmacMmcCntrl, GmacMmcCounterReset);
    return;
}

/**
  * @brief Configures the MMC to stop rollover.
  *
  * Programs MMC interface so that counters will not rollover after reaching maximum value.
  * @author CaoDonghua
  * @date 2016-09-26
  * @param[in] pointer to gmac_device.
  * @return returns void.
  * @retval none
  */
static void gmac_mmc_counters_disable_rollover(gmac_device *gmacdev)
{
    gmac_set_bits((u32 *)gmacdev->MacBase, GmacMmcCntrl, GmacMmcCounterStopRollover);
    return;
}

/**
 * @brief  The Low level function to read register contents from Hardware.
 *
 * @author CaoDonghua
 * @date 2016-09-26
 * @param[in] pointer to the base of register map
 * @param[in] Offset from the base
 * @return  u32 Returns the register contents
 * @retval register contents
 */
static u32 gmac_read_reg(u32 *RegBase, u32 RegOffset)
{
    u32 addr = (u32)RegBase + RegOffset;
    u32 data = readl((void *)addr);

    return data;

}


/**
 * @brief  The Low level function to write to a register in Hardware.
 *
 * @author CaoDonghua
 * @date 2016-09-26
 * @param[in] pointer to the base of register map
 * @param[in] Offset from the base
 * @param[in] Data to be written
 * @return void.
 * @retval none
 */
static void  gmac_write_reg(u32 *RegBase, u32 RegOffset, u32 RegData)
{

    u32 addr = (u32)RegBase + RegOffset;

    writel(RegData, (void *)addr);
    return;
}

/**
  * @brief  This is a wrapper function for platform dependent delay
  *
  * Take care while passing the argument to this function
  * @author CaoDonghua
  * @date 2016-09-26
  * @param[in] delay delay cnt.
  * @return void.
  * @retval none
  */
static void plat_delay(u32 delay)
{
    while (delay--);
    return;
}

/**
  * @brief Function to reset the GMAC core.
  *
  * This reests the DMA and GMAC core. After reset all the registers holds their respective reset value
  * @author CaoDonghua
  * @date 2016-09-26
  * @param[in] pointer to gmac_device.
  * @return 0 on success else return failed.
  * @retval 0 on success
  * @retval non-zero on failed
  */
static s32 gmac_reset(gmac_device *gmacdev)
{
    u32 data = 0;
	s32 reset_cnt = 0xFFFF;
	
    /* software reset , the resets all of the GMAC internal registers and logic */ 
    gmac_write_reg((u32 *)gmacdev->DmaBase, DmaBusMode , DmaResetOn); // cdh:reset dma engine
    plat_delay(DEFAULT_LOOP_VARIABLE);
    
	while(reset_cnt>0){
		data = gmac_read_reg((u32 *)gmacdev->DmaBase, DmaBusMode);
		
		/* after finish become 0 */ 
		if((data & DmaResetOn) != DmaResetOn) {
			break;
		}
		reset_cnt--;
	}

	if (reset_cnt <= 0) {
		printf("No find phy small board!\n");
		return -1;
	}

    return 0;
}

/**
  * @brief Sets the Mac address in to GMAC register.
  *
  * This function sets the MAC address to the MAC register in question.
  * @author CaoDonghua
  * @date 2016-09-26
  * @param[in] pointer to gmac_device to populate mac dma and phy addresses.
  * @param[in] Register offset for Mac address high
  * @param[in] Register offset for Mac address low
  * @param[in] buffer containing mac address to be programmed.
  * @return return 0 on success
  * @retval 0 on success
  */
static s32 gmac_set_mac_addr(gmac_device *gmacdev, u32 MacHigh, u32 MacLow, u8 *MacAddr)
{
    u32 data;

    data = (MacAddr[5] << 8) | MacAddr[4];
    gmac_write_reg((u32 *)gmacdev->MacBase, MacHigh, data);
    data = (MacAddr[3] << 24) | (MacAddr[2] << 16) | (MacAddr[1] << 8) | MacAddr[0] ;
    gmac_write_reg((u32 *)gmacdev->MacBase, MacLow, data);
    return 0;
}

/**
  * @brief Get the Mac address in to the address specified.
  *
  * The mac register contents are read and written to buffer passed.
  * @author CaoDonghua
  * @date 2016-09-26
  * @param[in] pointer to gmac_device to populate mac dma and phy addresses.
  * @param[in] Register offset for Mac address high
  * @param[in] Register offset for Mac address low
  * @param[out] buffer containing the device mac address.
  * @return return 0 on success
  * @retval 0 on success
  */
static s32 gmac_get_mac_addr(gmac_device *gmacdev, u32 MacHigh, u32 MacLow, u8 *MacAddr)
{
    u32 data;

    data = gmac_read_reg((u32 *)gmacdev->MacBase, MacHigh);
    MacAddr[5] = (data >> 8) & 0xff;
    MacAddr[4] = (data)        & 0xff;
    data = gmac_read_reg((u32 *)gmacdev->MacBase, MacLow);
    MacAddr[3] = (data >> 24) & 0xff;
    MacAddr[2] = (data >> 16) & 0xff;
    MacAddr[1] = (data >> 8 ) & 0xff;
    MacAddr[0] = (data )      & 0xff;

    return 0;
}

/**
  * @brief Function to read the GMAC IP Version and populates the same in device data structure.
  *
  * @author CaoDonghua
  * @date 2016-09-26
  * @param[in] pointer to gmac_device.
  * @return return 0 on success
  * @retval 0 on success
  */
static s32 gmac_read_version(gmac_device *gmacdev)
{
    u32 data = 0;

    debug("gmac version register %x\n", (unsigned int)(gmacdev->MacBase + GmacVersion));

    /* available the mac ip version */ 
    data = gmac_read_reg((u32 *)gmacdev->MacBase, GmacVersion );
    gmacdev->Version = data;
    debug("The data read from %08x is %08x\n", (unsigned int)(gmacdev->MacBase + GmacVersion), (unsigned int)data);

    return 0;
}

/**
  * @brief  Function to set the MDC clock for mdio transactiona
  *
  * @author CaoDonghua
  * @date 2016-09-26
  * @param[in] pointer to device structure.
  * @param[in] clk divider value.
  * @return Reuturns 0 on success else return the error value.
  * @retval 0 on success
  */
static s32 gmac_set_mdc_clk_div(gmac_device *gmacdev, u32 clk_div_val)
{
    u32 orig_data;

	//debug("cdh:mod clk=0x%x\n",  REG32(0x0800001c));
	//debug("cdh:pll clk=0x%x\n",  REG32(0x08000004));
	//debug("cdh:asic clk=0x%x\n", REG32(0x08000008));
	
    /* set MDO_CLK for MDIO transmit, note GmacGmiiAddr bit5, and 802.3 limit 2.5MHz */ 
    orig_data = gmac_read_reg((u32 *)gmacdev->MacBase, GmacGmiiAddr); //set the mdc clock to the user defined value
    orig_data &= (~ GmiiCsrClkMask); // cdh:csr clk bit[5:2], must 0x3C
    orig_data |= clk_div_val;
    gmac_write_reg((u32 *)gmacdev->MacBase, GmacGmiiAddr , orig_data);
    return 0;
}

/**
  * @brief Returns the current MDC divider value programmed in the ip.
  *
  * @author CaoDonghua
  * @date 2016-09-26
  * @param[in] pointer to device structure.
  * @param[in] clk divider value.
  * @return u32 Returns the MDC divider value read.
  * @retval MDC divider value
  */
static u32 synopGMAC_get_mdc_clk_div(gmac_device *gmacdev)
{
    u32 data;
    
    data = gmac_read_reg((u32 *)gmacdev->MacBase, GmacGmiiAddr);
    // debug("cdh:mdc_clk_div1:0x%x\n", data);
    data &= GmiiCsrClkMask;
    // debug("cdh:mdc_clk_div2:0x%x\n", data);
    return data;
}


/**
  * @brief Function gpio to reset the Phy register. 
  *
  * gpio reset phy from MDI/MDO interface
  * @author CaoDonghua
  * @date 2016-09-26
  * @param[in] void.
  * @return void.
  * @retval none
  */
static void gmac_phy_gpio_reset(void)
{
	debug("DGPIO62 Reset MAC phy!\n");
	
    /* gpio62 for phy reset, POWER ON */
    REG32(0x20170000 + 0x0004) |= (1<<30); /* DGPIO62 1:output */	
    REG32(0x20170000 + 0x0010) |= (1<<30); /* RST = 1 */	
    debug("..PHY reset delay H..\n");
	mdelay(5);
	REG32(0x20170000 + 0x0010) &= ~(1<<30); /* RST = 0 */	
	debug("..PHY reset delay L..\n");
	mdelay(25);
	REG32(0x20170000 + 0x0010) |= (1<<30); /* RST = 1 */	
	debug("..PHY reset delay K..\n");
	mdelay(5);
}

/**
  * @brief Function to read the Phy register. The access to phy register
  *
  * is a slow process as the data is moved accross MDI/MDO interface
  * @author CaoDonghua
  * @date 2016-09-26
  * @param[in] pointer to Register Base (It is the mac base in our case) .
  * @param[in] PhyBase register is the index of one of supported 32 PHY devices.
  * @param[in] Register offset is the index of one of the 32 phy register.
  * @param[out] u16 data read from the respective phy register (only valid iff return value is 0).
  * @return Returns 0 on success else return the error status.
  * @retval 0 on success
  * @retval error number on failed
  */
static s32 gmac_read_phy_reg(u32 *RegBase, u32 PhyBase, u32 RegOffset, u16 *data)
{
    u32 addr;
    u32 loop_variable;
    
    addr = ((PhyBase << GmiiDevShift) & GmiiDevMask) | ((RegOffset << GmiiRegShift) & GmiiRegMask);
    addr = addr | GmiiBusy ; //Gmii busy bit
    gmac_write_reg(RegBase, GmacGmiiAddr, addr); //write the address from where the data to be read in GmiiGmiiAddr register of synopGMAC ip

	/* Wait till the busy bit gets cleared with in a certain amount of time */
    for (loop_variable=0; loop_variable<DEFAULT_LOOP_VARIABLE; loop_variable++) {
        if (!(gmac_read_reg(RegBase, GmacGmiiAddr) & GmiiBusy)){
            break;
        } else {
	        ;//CDH:TR("mdio busy\n");
        }
        plat_delay(DEFAULT_DELAY_VARIABLE);
    }
    
    if(loop_variable < DEFAULT_LOOP_VARIABLE){
        *data = (u16)(gmac_read_reg(RegBase, GmacGmiiData) & 0xFFFF);
    }else{
        return -ESYNOPGMACPHYERR;
    }
    
    return -ESYNOPGMACNOERR;
}

/**
  * @brief Function to write to the Phy register. The access to phy register
  *
  * is a slow process as the data is moved accross MDI/MDO interface
  * @author CaoDonghua
  * @date 2016-09-26
  * @param[in] pointer to Register Base (It is the mac base in our case) .
  * @param[in] PhyBase register is the index of one of supported 32 PHY devices.
  * @param[in] Register offset is the index of one of the 32 phy register.
  * @param[in] data to be written to the respective phy register.
  * @return Returns 0 on success else return the error status.
  * @retval 0 on success
  * @retval error number on failed
  */
static s32 gmac_write_phy_reg(u32 *RegBase, u32 PhyBase, u32 RegOffset, u16 data)
{
    u32 addr;
    u32 loop_variable;

    gmac_write_reg(RegBase, GmacGmiiData, data);

    addr = ((PhyBase << GmiiDevShift) & GmiiDevMask) | ((RegOffset << GmiiRegShift) & GmiiRegMask) | GmiiWrite;
    addr = addr | GmiiBusy ; //set Gmii clk to 20-35 Mhz and Gmii busy bit
    gmac_write_reg(RegBase, GmacGmiiAddr, addr);
    
    for (loop_variable=0; loop_variable<DEFAULT_LOOP_VARIABLE; loop_variable++) {
        if (!(gmac_read_reg(RegBase, GmacGmiiAddr) & GmiiBusy)){
            break;
        }else {
	       ; // TR("mdio busy\n");
        }
        plat_delay(DEFAULT_DELAY_VARIABLE);
    }

    if(loop_variable < DEFAULT_LOOP_VARIABLE){
        return -ESYNOPGMACNOERR;
    }else {
        return -ESYNOPGMACPHYERR;
    }
}

/**
  * @brief Checks and initialze phy reset.
  *
  * Checks and initialze phy reset enter idle.
  * @author CaoDonghua
  * @date 2016-09-26
  * @param[in] pointer to gmac_device.
  * @return 0 if success else returns the error number.
  * @retval 0 on success
  * @retval error number on failed
  */
static s32 phy_wait_idle(gmac_device *gmacdev)
{
	u32 i = 0;
	s32 status;
	u16 data;
	
	/* wait phy idle, reset cycle < 0.5s  */ 
	while(1) {
		status = gmac_read_phy_reg((u32 *)gmacdev->MacBase, gmacdev->PhyBase, PHY_CONTROL_REG, &data);
		if(status) { 
			continue; 
		}

		// cdh:Mii_reset=1(soft_reset), or=0(normal), self-clearing
		if((data & Mii_reset) == 0){ 
			debug("phy reset enter idle OK\n"); 
			break;
		}

		if(i>0xFFFF){
			printf("timeout in waiting phy to idle!\n");
			return 1;
		}

		i++;
	}
	
	return status;
}

/**
  * @brief Checks and initialze phy.
  *
  * This function checks whether the phy initialization is complete.
  * @author CaoDonghua
  * @date 2016-09-26
  * @param[in] pointer to gmac_device.
  * @return 0 if success else returns the error number.
  * @retval 0 on success
  * @retval error number on failed
  */
static s32 gmac_phy_init(gmac_device *gmacdev)
{
	s32 status;
	u16 data_ctl;
	u16 data_id1;
	u16 data_id2;
	
	/* reset */
	gmac_phy_gpio_reset();
	
	/* wait phy idle */ 
	if(0x1 == phy_wait_idle(gmacdev)) {
		printf("cdh:%s,phy_wait_idle err!\n", __func__);
	}else {
		debug("cdh:%s,phy_wait_idle ok!\n", __func__);
	}

	mdelay(200);
	status = gmac_read_phy_reg((u32 *)gmacdev->MacBase, gmacdev->PhyBase, PHY_CONTROL_REG, &data_ctl);
	if(status) { 
		printf("phy read phy ctrl waiting Error\n");  
	}
	debug("BMCR:0x%x\n", data_ctl);

	status = gmac_read_phy_reg((u32 *)gmacdev->MacBase, gmacdev->PhyBase, PHY_ID_HI_REG, &data_id1);
	if(status) { 
		printf("phy read phy id1 waiting Error,data=0x%x\n", data_id1);  
	}else {
		debug("phy read phy id1 waiting ok,data=0x%x\n", data_id1);  
	}

	status = gmac_read_phy_reg((u32 *)gmacdev->MacBase, gmacdev->PhyBase, PHY_ID_LOW_REG, &data_id2);
	if(status) { 
		printf("phy read phy id2 waiting Error,data=0x%x\n", data_id2);  
	}else {
		debug("phy read phy id2 waiting ok,data=0x%x\n", data_id2);  
	}
	
	if(data_id1 == 0x22) {
		debug("cdh:id 0x22 set!\n");
		gmac_read_phy_reg((u32 *)gmacdev->MacBase, gmacdev->PhyBase, PHY_CONTROL_REG, &data_ctl);
		data_ctl |= 0x1000; // cdh:enable auto-negotiation process
		gmac_write_phy_reg((u32 *)gmacdev->MacBase, gmacdev->PhyBase, PHY_CONTROL_REG, data_ctl);
	}else {
		debug("cdh:id other set!\n");
		gmac_read_phy_reg((u32 *)gmacdev->MacBase, gmacdev->PhyBase, PHY_CONTROL_REG, &data_ctl);
		data_ctl |= 0x1100; // cdh:bit12 enable auto-negotiation process, set full-duplex mode
		gmac_write_phy_reg((u32 *)gmacdev->MacBase, gmacdev->PhyBase, PHY_CONTROL_REG, data_ctl);

	}

	return 0;

}


/**
  * @brief Checks and initialze phy.
  *
  * This function checks whether the phy initialization is complete.
  * @author CaoDonghua
  * @date 2016-09-26
  * @param[in] pointer to gmac_device.
  * @return 0 if success else returns the error number.
  * @retval 0 on success
  * @retval error number on failed
  */
static s32 gmac_check_phy_init(gmac_device *gmacdev)
{
    u16 data;
    s32 status = -ESYNOPGMACNOERR;
    s32 loop_count;

	/** 
	*Auto-Negotiation
	*The KSZ8041NL conforms to the auto-negotiation protocol, defined in Clause 28 of the IEEE 802.3u specification. Autonegotiation
	*is enabled by either hardware pin strapping (pin 30) or software (register 0h bit 12).
	*/
    loop_count = 10; 
    while (1)
    {
        status = gmac_read_phy_reg((u32 *)gmacdev->MacBase, gmacdev->PhyBase, PHY_STATUS_REG, &data);
        if(status)
            return status;

        if((data & Mii_AutoNegCmplt) != 0){
            debug("Autonegotiation Complete\n");  // cdh:check ok
            break;
        }
    }

	/* note may be for same special phy define register, not same 8201F phy,cdh: PHY_SPECIFIC_STATUS_REG */ 
    status = gmac_read_phy_reg((u32 *)gmacdev->MacBase, gmacdev->PhyBase, PHY_STATUS_REG, &data);
    if(status) {
        return status;
    }

    if((data & Mii_Link) == 0) { // cdh:Mii_phy_status_link_up
        printf("No Link\n");
        gmacdev->LinkState = LINKDOWN;
        return -ESYNOPGMACPHYERR;
    }else {
        gmacdev->LinkState = LINKUP;
        debug("Link UP\n");
    }

    status = gmac_read_phy_reg((u32 *)gmacdev->MacBase, gmacdev->PhyBase, PHY_STATUS_REG, &data);
    if(status) {
        return status;
    }
    
    if(data & Mii_phy_status_speed_100) {
        gmacdev->Speed = SPEED100;
        gmacdev->DuplexMode = FULLDUPLEX;
        printf("Link is with 100M Speed, FULLDUPLEX mode\n");
    }
    else {
        gmacdev->Speed = SPEED10;
        gmacdev->DuplexMode = HALFDUPLEX;
        printf("Link is with 10M Speed, HALFDUPLEX mode \n");
	}
	
    return -ESYNOPGMACNOERR;
}



/**
  * @brief Initialize the tx descriptors for ring or chain mode operation.
  * 	- Status field is initialized to 0.
  *	- EndOfRing set for the last descriptor.
  *	- buffer1 and buffer2 set to 0 for ring mode of operation. (note)
  *	- data1 and data2 set to 0. (note)
  * @author CaoDonghua
  * @date 2016-09-26
  * @param[in] pointer to DmaDesc structure.
  * @param[in] whether end of ring
  * @return void.
  * @retval none
  * @note Initialization of the buffer1, buffer2, data1,data2 and status are not done here. This only initializes whether one wants to use this descriptor
  * in chain mode or ring mode. For chain mode of operation the buffer2 and data2 are programmed before calling this function.
  */
static void gmac_tx_desc_init_ring(DmaDesc *desc, bool last_ring_desc)
{
	desc->status = 0; 
    desc->length = last_ring_desc ? TxDescEndOfRing : 0; // cdh:TxDescEndOfRing bit25(TER)
    desc->buffer1 = 0;
    desc->buffer2 = 0;
    desc->data1 = 0;
    desc->data2 = 0;
    return;
}



/**
  * @brief This sets up the transmit Descriptor queue in ring or chain mode.
  *
  * This function is tightly coupled to the platform and operating system
  * Device is interested only after the descriptors are setup. Therefore this function
  * is not included in the device driver API. This function should be treated as an
  * example code to design the descriptor structures for ring mode or chain mode.
  * This function depends on the pcidev structure for allocation consistent dma-able memory in case of linux.
  * This limitation is due to the fact that linux uses pci structure to allocate a dmable memory
  *	- Allocates the memory for the descriptors.
  *	- Initialize the Busy and Next descriptors indices to 0(Indicating first descriptor).
  *	- Initialize the Busy and Next descriptors to first descriptor address.
  * 	- Initialize the last descriptor with the endof ring in case of ring mode.
  *	- Initialize the descriptors in chain mode.
  * @author CaoDonghua
  * @date 2016-09-26
  * @param[in] pointer to gmac_device.
  * @param[in] pointer to pci_device structure.
  * @param[in] number of descriptor expected in tx descriptor queue.
  * @param[in] whether descriptors to be created in RING mode or CHAIN mode.
  * @return s32 0 upon success. Error code upon failure.
  * @retval 0 upon success
  * @retval Error code upon failure
  * @note This function fails if allocation fails for required number of descriptors in Ring mode, but in chain mode
  *  function returns -ESYNOPGMACNOMEM in the process of descriptor chain creation. once returned from this function
  *  user should for gmacdev->TxDescCount to see how many descriptors are there in the chain. Should continue further
  *  only if the number of descriptors in the chain meets the requirements
  */
static s32 setup_tx_desc_queue(gmac_device *gmacdev, struct eth_device *pnetdev, u32 no_of_desc, u32 desc_mode)
{
    s32 i;

    DmaDesc *first_desc = NULL;
    dma_addr_t dma_addr;
    gmacdev->TxDescCount = 0;

    if(desc_mode == RINGMODE) {
        first_desc = (DmaDesc *)dma_alloc_coherent(sizeof(DmaDesc) * no_of_desc, (unsigned long *)&dma_addr);
        if(first_desc == NULL) {
            printf("Error in Tx Descriptors memory allocation\n");
            return -ESYNOPGMACNOMEM;
        }
        
        gmacdev->TxDescCount = no_of_desc;	// cdh:ring descriptor count
        gmacdev->tx_ring[0]  = first_desc;	// cdh:ring descriptor base virtual address
        gmacdev->TxDescDma   = dma_addr;	// cdh:ring descriptor base physic address
		
        for(i=0; i<gmacdev->TxDescCount; i++) {
        	/* tx ring descriptor every one give base virtual address */ 
        	gmacdev->tx_ring[i]  = first_desc + i;	
        	
        	/* why not init dma fifo start addresss??? */ 
            gmac_tx_desc_init_ring(gmacdev->tx_ring[i], i == gmacdev->TxDescCount - 1);
        }

    }

    gmacdev->TxNext = 0;
    gmacdev->TxBusy = 0;
    gmacdev->TxNextDesc = gmacdev->TxDesc;
    gmacdev->TxBusyDesc = gmacdev->TxDesc;
    gmacdev->BusyTxDesc  = 0;

    return -ESYNOPGMACNOERR;
}


/**
  * @brief Programs the DmaTxBaseAddress with the Tx descriptor base address.
  *
  * Tx Descriptor's base address is available in the gmacdev structure. This function progrms the
  * Dma Tx Base address with the starting address of the descriptor ring or chain.
  * @author CaoDonghua
  * @date 2016-09-26
  * @param[in] pointer to gmac_device.
  * @return returns void.
  * @retval none
  */
static void gmac_init_tx_desc_base(gmac_device *gmacdev)
{
	/* write TxDescDma consistent  physic address to mac dma ctrl reg */ 
    gmac_write_reg((u32 *)gmacdev->DmaBase, DmaTxBaseAddr, (u32)gmacdev->TxDescDma);
    return;
}

/**
  * @brief Initialize the rx descriptors for ring or chain mode operation.
  *
  * 	- Status field is initialized to 0.
  *	- EndOfRing set for the last descriptor.
  *	- buffer1 and buffer2 set to 0 for ring mode of operation. (note)
  *	- data1 and data2 set to 0. (note)
  * @author CaoDonghua
  * @date 2016-09-26
  * @param[in] pointer to DmaDesc structure.
  * @param[in] whether end of ring
  * @return void.
  * @retval none
  * @note Initialization of the buffer1, buffer2, data1,data2 and status are not done here. This only initializes whether one wants to use this descriptor
  * in chain mode or ring mode. For chain mode of operation the buffer2 and data2 are programmed before calling this function.
  */
static void gmac_rx_desc_init_ring(DmaDesc *desc, bool last_ring_desc)
{
    desc->status = 0;
    desc->length = last_ring_desc ? RxDescEndOfRing : 0;
    desc->buffer1 = 0;
    desc->buffer2 = 0;
    desc->data1 = 0;
    desc->data2 = 0;
    return;
}

/**
  * @brief init gmac rx desc ring last flags
  *
  * init gmac rx desc ring last flags start before run 
  * @author CaoDonghua
  * @date 2016-09-26
  * @param[in] index buffer index
  * @param[in] desc dma descriptor.
  * @param[in] last_ring_desc last ring desc flag.
  * @return returns void.
  * @retval none
  */
static void gmac_rx_desc_init_ring2(u32 index, DmaDesc *desc, bool last_ring_desc)
{
    desc->status = 0;
    desc->length = last_ring_desc ? RxDescEndOfRing : 0;
    desc->length |= (((PKTSIZE_ALIGN << DescSize1Shift) & DescSize1Mask) | ((0 << DescSize2Shift) & DescSize2Mask));
    desc->buffer1 = (unsigned int)NetRxPackets[index];
    desc->buffer2 = 0;
    desc->data1 = 0;
    desc->data2 = 0;
    return;
}


/**
  * @brief init gmac rx desc ring last flags
  *
  * init gmac rx desc ring last flags after loop
  * @author CaoDonghua
  * @date 2016-09-26
  * @param[in] index buffer index
  * @param[in] desc dma descriptor.
  * @param[in] last_ring_desc last ring desc flag.
  * @return returns void.
  * @retval none
  */
static void gmac_rx_desc_init_ring3(u32 index, DmaDesc *desc, bool last_ring_desc)
{
    desc->length |= last_ring_desc ? RxDescEndOfRing : 0;

    return;
}



/**
  * @brief  This sets up the receive Descriptor queue in ring or chain mode.
  *
  * This function is tightly coupled to the platform and operating system
  * Device is interested only after the descriptors are setup. Therefore this function
  * is not included in the device driver API. This function should be treated as an
  * example code to design the descriptor structures in ring mode or chain mode.
  * This function depends on the pcidev structure for allocation of consistent dma-able memory in case of linux.
  * This limitation is due to the fact that linux uses pci structure to allocate a dmable memory
  *	- Allocates the memory for the descriptors.
  *	- Initialize the Busy and Next descriptors indices to 0(Indicating first descriptor).
  *	- Initialize the Busy and Next descriptors to first descriptor address.
  * 	- Initialize the last descriptor with the endof ring in case of ring mode.
  *	- Initialize the descriptors in chain mode.
  * @author CaoDonghua
  * @date 2016-09-26
  * @param[in] pointer to gmac_device.
  * @param[in] pointer to pci_device structure.
  * @param[in] number of descriptor expected in rx descriptor queue.
  * @param[in] whether descriptors to be created in RING mode or CHAIN mode.
  * @return s32 0 upon success. Error code upon failure.
  * @retval 0 upon success.
  * @retval Error code upon failure.
  * @note This function fails if allocation fails for required number of descriptors in Ring mode, but in chain mode
  *  function returns -ESYNOPGMACNOMEM in the process of descriptor chain creation. once returned from this function
  *  user should for gmacdev->RxDescCount to see how many descriptors are there in the chain. Should continue further
  *  only if the number of descriptors in the chain meets the requirements
  */
static s32 setup_rx_desc_queue(gmac_device *gmacdev, struct eth_device *pnetdev, u32 no_of_desc, u32 desc_mode)
{
    s32 i;

    DmaDesc *first_desc = NULL;
    dma_addr_t dma_addr;
    gmacdev->RxDescCount = 0;

    if(desc_mode == RINGMODE) {
        first_desc = (DmaDesc *)dma_alloc_coherent(sizeof(DmaDesc) * no_of_desc, &dma_addr);
        if(first_desc == NULL) {
            printf("Error in Rx Descriptor Memory allocation in Ring mode\n");
            return -ESYNOPGMACNOMEM;
        }
        
        gmacdev->RxDescCount = no_of_desc;
        gmacdev->rx_ring[0]  = first_desc; // cdh:the first desc virtual addr
        gmacdev->RxDescDma   = dma_addr;   // cdh:the first desc physic addr
		
        for(i=0; i<gmacdev->RxDescCount; i++) {
        	/* tx ring descriptor every one give base virtual address */ 
        	gmacdev->rx_ring[i]  = first_desc + i;
        	
            gmac_rx_desc_init_ring2(i, gmacdev->rx_ring[i], i == gmacdev->RxDescCount - 1);
			smc_desc_set_rx_owner(gmacdev->rx_ring[i]);
        }
    }

    gmacdev->RxNext = 0;
    gmacdev->RxBusy = 0;
    gmacdev->RxNextDesc = gmacdev->RxDesc;
    gmacdev->RxBusyDesc = gmacdev->RxDesc;
    gmacdev->BusyRxDesc   = 0;

    return -ESYNOPGMACNOERR;
}



/**
  * @brief Programs the DmaRxBaseAddress with the Rx descriptor base address.
  *
  * Rx Descriptor's base address is available in the gmacdev structure. This function progrms the
  * Dma Rx Base address with the starting address of the descriptor ring or chain.
  * @author CaoDonghua
  * @date 2016-09-26
  * @param[in] pointer to gmac_device.
  * @return returns void.
  * @retval none
  */
static void gmac_init_rx_desc_base(gmac_device *gmacdev)
{
	u32 rx_desc_addr;

	/* write RxDescDma consistent  physic address to mac dma ctrl reg */ 
    gmac_write_reg((u32 *)gmacdev->DmaBase, DmaRxBaseAddr, (u32)gmacdev->RxDescDma);
    rx_desc_addr = gmac_read_reg((u32 *)gmacdev->DmaBase, DmaRxBaseAddr);

    return;
}


/**
  * @brief Function to program DMA bus mode register.
  *
  * The Bus Mode register is programmed with the value given. The bits to be set are
  * bit wise or'ed and sent as the second argument to this function.
  * @author CaoDonghua
  * @date 2016-09-26
  * @param[in] pointer to gmac_device.
  * @param[in] the data to be programmed.
  * @return s32 0 on success else return the error status.
  * @retval 0 on success
  */
static s32 gmac_set_dma_bus_mode(gmac_device *gmacdev, u32 init_value )
{
    gmac_write_reg((u32 *)gmacdev->DmaBase, DmaBusMode , init_value);
    return 0;
}

/**
  * @brief Function to program DMA Control register.
  *
  * The Dma Control register is programmed with the value given. The bits to be set are
  * bit wise or'ed and sent as the second argument to this function.
  * @author CaoDonghua
  * @date 2016-09-26
  * @param[in] pointer to gmac_device.
  * @param[in] the data to be programmed.
  * @return s32 0 on success else return the error status.
  * @retval 0 on success
  */
static s32 gmac_set_dma_control(gmac_device *gmacdev, u32 init_value)
{
    gmac_write_reg((u32 *)gmacdev->DmaBase, DmaControl, init_value);
    return 0;
}


/**
  * @brief Enable the watchdog timer on the receiver.
  *
  * When enabled, Gmac enables Watchdog timer, and GMAC allows no more than
  * 2048 bytes of data (10,240 if Jumbo frame enabled).
  * @author CaoDonghua
  * @date 2016-09-26
  * @param[in] pointer to gmac_device.
  * @return returns void.
  * @retval none
  */
static void gmac_wd_enable(gmac_device *gmacdev)
{
    gmac_clr_bits((u32 *)gmacdev->MacBase, GmacConfig, GmacWatchdog);
    return;
}


/**
  * @brief Enables the Jabber frame support.
  *
  * When enabled, GMAC disabled the jabber timer, and can transfer 16,384 byte frames.
  * @author CaoDonghua
  * @date 2016-09-26
  * @param[in] pointer to gmac_device.
  * @return returns void.
  * @retval none
  */
static void gmac_jab_enable(gmac_device *gmacdev)
{
    gmac_set_bits((u32 *)gmacdev->MacBase, GmacConfig, GmacJabber);
    return;
}

/**
  * @brief Enables Frame bursting (Only in Half Duplex Mode).
  *
  * When enabled, GMAC allows frame bursting in GMII Half Duplex mode.
  * Reserved in 10/100 and Full-Duplex configurations.
  * @author CaoDonghua
  * @date 2016-09-26
  * @param[in] pointer to gmac_device.
  * @return returns void.
  * @retval none
  */
static void gmac_frame_burst_enable(gmac_device *gmacdev)
{
    gmac_set_bits((u32 *)gmacdev->MacBase, GmacConfig, GmacFrameBurst);
    return;
}


/**
  * @brief Disable Jumbo frame support.
  *
  * When Disabled GMAC does not supports jumbo frames.
  * Giant frame error is reported in receive frame status.
  * @author CaoDonghua
  * @date 2016-09-26
  * @param[in] pointer to gmac_device.
  * @return returns void.
  * @retval none
  */
static void gmac_jumbo_frame_disable(gmac_device *gmacdev)
{
    gmac_clr_bits((u32 *)gmacdev->MacBase, GmacConfig, GmacJumboFrame);
    return;
}

/**
  * @brief Enables Receive Own bit (Only in Half Duplex Mode).
  *
  * When enaled GMAC receives all the packets given by phy while transmitting.
  * @author CaoDonghua
  * @date 2016-09-26
  * @param[in] pointer to gmac_device.
  * @return returns void.
  * @retval none
  */
static void gmac_rx_own_enable(gmac_device *gmacdev)
{
    gmac_clr_bits((u32 *)gmacdev->MacBase, GmacConfig, GmacRxOwn);
    return;
}


/**
  * @brief Sets the GMAC in Normal mode.
  *
  * @author CaoDonghua
  * @date 2016-09-26
  * @param[in] pointer to gmac_device.
  * @return returns void.
  * @retval none
  */
static void gmac_loopback_off(gmac_device *gmacdev)
{
    gmac_clr_bits((u32 *)gmacdev->MacBase, GmacConfig, GmacLoopback);
    return;
}

/**
  * @brief Sets the GMAC core in Full-Duplex mode.
  *
  * @author CaoDonghua
  * @date 2016-09-26
  * @param[in] pointer to gmac_device.
  * @return returns void.
  * @retval none
  */
static void gmac_set_full_duplex(gmac_device *gmacdev)
{
    gmac_set_bits((u32 *)gmacdev->MacBase, GmacConfig, GmacDuplex);
    return;
}

/**
  * @brief Sets the GMAC core in Half-Duplex mode.
  *
  * @author CaoDonghua
  * @date 2016-09-26
  * @param[in] pointer to gmac_device.
  * @return returns void.
  * @retval none
  */
static void gmac_set_half_duplex(gmac_device *gmacdev)
{
    gmac_clr_bits((u32 *)gmacdev->MacBase, GmacConfig, GmacDuplex);
    return;
}

/**
  * @brief  GMAC tries retransmission (Only in Half Duplex mode).
  *
  * If collision occurs on the GMII/MII, GMAC attempt retries based on the
  * back off limit configured.
  * @author CaoDonghua
  * @date 2016-09-26
  * @param[in] pointer to gmac_device.
  * @return returns void.
  * @retval none
  * @note This function is tightly coupled with gmac_back_off_limit(gmacdev_pt *, u32).
  */
static void gmac_retry_enable(gmac_device *gmacdev)
{
    gmac_clr_bits((u32 *)gmacdev->MacBase, GmacConfig, GmacRetry);
    return;
}

/**
  * @brief GMAC doesnot strips the Pad/FCS field of incoming frames.
  *
  * GMAC will pass all the incoming frames to Host unmodified.
  * @author CaoDonghua
  * @date 2016-09-26
  * @param[in] pointer to gmac_device.
  * @return returns void.
  * @retval none
  */
static void gmac_pad_crc_strip_disable(gmac_device *gmacdev)
{
    gmac_clr_bits((u32 *)gmacdev->MacBase, GmacConfig, GmacPadCrcStrip);
    return;
}

/**
  * @brief GMAC programmed with the back off limit value.
  *
  * @author CaoDonghua
  * @date 2016-09-26
  * @param[in] pointer to gmac_device.
  * @return returns void.
  * @retval none
  * @note This function is tightly coupled with gmac_retry_enable(gmac_device * gmacdev)
  */
static void gmac_back_off_limit(gmac_device *gmacdev, u32 value)
{
    u32 data;
    
    data = gmac_read_reg((u32 *)gmacdev->MacBase, GmacConfig);
    data &= (~GmacBackoffLimit);
    data |= value;
    gmac_write_reg((u32 *)gmacdev->MacBase, GmacConfig, data);
    return;
}


/**
  * @brief  Disables the Deferral check in GMAC (Only in Half Duplex mode).
  *
  * GMAC defers until the CRS signal goes inactive.
  * @author CaoDonghua
  * @date 2016-09-26
  * @param[in] pointer to gmac_device.
  * @return returns void.
  * @retval none
  */
static void gmac_deferral_check_disable(gmac_device *gmacdev)
{
    gmac_clr_bits((u32 *)gmacdev->MacBase, GmacConfig, GmacDeferralCheck);
    return;
}

/**
  * @brief Selects the GMII port.
  *
  * When called GMII (1000Mbps) port is selected (programmable only in 10/100/1000 Mbps configuration).
  * @author CaoDonghua
  * @date 2016-09-26
  * @param[in] pointer to gmac_device.
  * @return returns void.
  * @retval none
  */
static void gmac_select_gmii(gmac_device *gmacdev)
{
    gmac_clr_bits((u32 *)gmacdev->MacBase, GmacConfig, GmacMiiGmii);
    return;
}


/**
  * @brief Selects the MII port.
  *
  * When called MII (10/100Mbps) port is selected (programmable only in 10/100/1000 Mbps configuration).
  * @author CaoDonghua
  * @date 2016-09-26
  * @param[in] pointer to gmac_device.
  * @return returns void.
  * @retval none
  */
static void gmac_select_mii(gmac_device *gmacdev)
{
    gmac_set_bits((u32 *)gmacdev->MacBase, GmacConfig, GmacMiiGmii);
    return;
}


/**
  *@brief  Enables reception of all the frames to application.
  *
  * GMAC passes all the frames received to application irrespective of whether they
  * pass SA/DA address filtering or not.
  * @author CaoDonghua
  * @date 2016-09-26
  * @param[in] pointer to gmac_device.
  * @return returns void.
  * @retval none
  */
static void gmac_frame_filter_enable(gmac_device *gmacdev)
{
    gmac_clr_bits((u32 *)gmacdev->MacBase, GmacFrameFilter, GmacFilter);
    return;
}

/**
  * @brief Enables forwarding of control frames.
  *
  * When set forwards all the control frames (incl. unicast and multicast PAUSE frames).
  * @author CaoDonghua
  * @date 2016-09-26
  * @param[in] pointer to gmac_device.
  * @return void.
  * @retval none
  * @note Depends on RFE of FlowControlRegister[2]
  */
static void gmac_set_pass_control(gmac_device *gmacdev, u32 passcontrol)
{
    u32 data;
    
    data = gmac_read_reg((u32 *)gmacdev->MacBase, GmacFrameFilter);
    data &= (~GmacPassControl);
    data |= passcontrol;
    gmac_write_reg((u32 *)gmacdev->MacBase, GmacFrameFilter, data);
    return;
}

/**
  * @brief  Enables Broadcast frames.
  *
  * When enabled Address filtering module passes all incoming broadcast frames.
  * @author CaoDonghua
  * @date 2016-09-26
  * @param[in] pointer to gmac_device.
  * @return void.
  * @retval none
  */
static void gmac_broadcast_enable(gmac_device *gmacdev)
{
    gmac_clr_bits((u32 *)gmacdev->MacBase, GmacFrameFilter, GmacBroadcast);
    return;
}


/**
  * @brief  Disables Source address filtering.
  *
  * When disabled GMAC forwards the received frames with updated SAMatch bit in RxStatus.
  * @author CaoDonghua
  * @date 2016-09-26
  * @param[in] pointer to gmac_device.
  * @return void.
  * @retval none
  */
static void gmac_src_addr_filter_disable(gmac_device *gmacdev)
{
    gmac_clr_bits((u32 *)gmacdev->MacBase, GmacFrameFilter, GmacSrcAddrFilter);
    return;
}

/**
  * @brief Enables Multicast frames.
  *
  * When enabled all multicast frames are passed.
  * @author CaoDonghua
  * @date 2016-09-26
  * @param[in] pointer to gmac_device.
  * @return void.
  * @retval none
  */
static void gmac_multicast_enable(gmac_device *gmacdev)
{
    gmac_set_bits((u32 *)gmacdev->MacBase, GmacFrameFilter, GmacMulticastFilter);
    return;
}


/**
  * @brief Enables the normal Destination address filtering.
  *
  * @author CaoDonghua
  * @date 2016-09-26
  * @param[in] pointer to gmac_device.
  * @return void.
  * @retval none
  */
static void gmac_dst_addr_filter_normal(gmac_device *gmacdev)
{
    gmac_clr_bits((u32 *)gmacdev->MacBase, GmacFrameFilter, GmacDestAddrFilterNor);
    return;
}

/**
  * @brief Enables multicast hash filtering.
  *
  * When enabled GMAC performs teh destination address filtering according to the hash table.
  * @author CaoDonghua
  * @date 2016-09-26
  * @param[in] pointer to gmac_device.
  * @return void.
  * @retval none
  */
static void gmac_multicast_hash_filter_enable(gmac_device *gmacdev)
{
    gmac_set_bits((u32 *)gmacdev->MacBase, GmacFrameFilter, GmacMcastHashFilter);
    return;
}



/**
  * @brief Clears promiscous mode.
  *
  * When called the GMAC falls back to normal operation from promiscous mode.
  * @author CaoDonghua
  * @date 2016-09-26
  * @param[in] pointer to gmac_device.
  * @return void.
  * @retval none
  */
static void gmac_promisc_disable(gmac_device *gmacdev)
{
    gmac_clr_bits((u32 *)gmacdev->MacBase, GmacFrameFilter, GmacPromiscuousMode);
    return;
}


/**
  * @brief Disables multicast hash filtering.
  *
  * When disabled GMAC performs perfect destination address filtering for unicast frames, it compares
  * DA field with the value programmed in DA register.
  * @author CaoDonghua
  * @date 2016-09-26
  * @param[in] pointer to gmac_device.
  * @return void.
  * @retval none
  */
static void gmac_unicast_hash_filter_disable(gmac_device *gmacdev)
{
    gmac_clr_bits((u32 *)gmacdev->MacBase, GmacFrameFilter, GmacUcastHashFilter);
    return;
}


/**
  * @brief Disables detection of pause frames with stations unicast address.
  *
  * When disabled GMAC only detects with the unique multicast address (802.3x).
  * @author CaoDonghua
  * @date 2016-09-26
  * @param[in] pointer to gmac_device.
  * @return void.
  * @retval none
  */
static void gmac_unicast_pause_frame_detect_disable(gmac_device *gmacdev)
{
    gmac_clr_bits((u32 *)gmacdev->MacBase, GmacFlowControl, GmacUnicastPauseFrame);
    return;
}

/**
  * @brief  Rx flow control enable.
  *
  * When Enabled GMAC will decode the rx pause frame and disable the tx for a specified time.
  * @author CaoDonghua
  * @date 2016-09-26
  * @param[in] pointer to gmac_device.
  * @return void.
  * @retval none
  */
static void gmac_rx_flow_control_enable(gmac_device *gmacdev)
{
    gmac_set_bits((u32 *)gmacdev->MacBase, GmacFlowControl, GmacRxFlowControl);
    return;
}

/**
  * @brief  Tx flow control enable.
  *
  * When Enabled
  * 	- In full duplex GMAC enables flow control operation to transmit pause frames.
  *	- In Half duplex GMAC enables the back pressure operation
  * @author CaoDonghua
  * @date 2016-09-26
  * @param[in] pointer to gmac_device.
  * @return void.
  * @retval none
  */
static void gmac_tx_flow_control_enable(gmac_device *gmacdev)
{
    gmac_set_bits((u32 *)gmacdev->MacBase, GmacFlowControl, GmacTxFlowControl);
    return;
}


/**
  * @brief Enable the transmission of frames on GMII/MII.
  *
  * @author CaoDonghua
  * @date 2016-09-26
  * @param[in] pointer to gmac_device.
  * @return returns void.
  * @retval none
  */
static void gmac_tx_enable(gmac_device *gmacdev)
{
    gmac_set_bits((u32 *)gmacdev->MacBase, GmacConfig, GmacTx);
    return;
}

/**
  * @brief Disable the transmission of frames on GMII/MII.
  *
  * GMAC transmit state machine is disabled after completion of transmission of current frame.
  * @author CaoDonghua
  * @date 2016-09-26
  * @param[in] pointer to gmac_device.
  * @return returns void.
  * @retval none
  */
static void gmac_tx_disable(gmac_device *gmacdev)
{
    gmac_clr_bits((u32 *)gmacdev->MacBase, GmacConfig, GmacTx);
    return;
}

/**
  * @brief Enable the reception of frames on GMII/MII.
  *
  * @author CaoDonghua
  * @date 2016-09-26
  * @param[in] pointer to gmac_device.
  * @return returns void.
  * @retval none
  */
static void gmac_rx_enable(gmac_device *gmacdev)
{
    gmac_set_bits((u32 *)gmacdev->MacBase, GmacConfig, GmacRx);
    return;
}

/**
  * @brief Disable the reception of frames on GMII/MII.
  *
  * GMAC receive state machine is disabled after completion of reception of current frame.
  * @author CaoDonghua
  * @date 2016-09-26
  * @param[in] pointer to gmac_device.
  * @return returns void.
  * @retval none
  */
static void gmac_rx_disable(gmac_device *gmacdev)
{
    gmac_clr_bits((u32 *)gmacdev->MacBase, GmacConfig, GmacRx);
    
    return;
}


/**
  * @brief  mac initialization sequence.
  *
  * This function calls the initialization routines to initialize the GMAC register.
  * One can change the functions invoked here to have different configuration as per the requirement
  * @author CaoDonghua
  * @date 2016-09-26
  * @param[in] pointer to gmac_device.
  * @return s32 Returns 0 on success.
  * @retval 0 on success
  */
static s32 gmac_mac_init(gmac_device *gmacdev)
{
    u32 PHYreg;

    if(gmacdev->DuplexMode == FULLDUPLEX)
    {
        gmac_wd_enable(gmacdev);
        gmac_jab_enable(gmacdev);
        gmac_frame_burst_enable(gmacdev);
        gmac_jumbo_frame_disable(gmacdev);
        gmac_rx_own_enable(gmacdev);
        gmac_loopback_off(gmacdev);
        gmac_set_full_duplex(gmacdev);
        gmac_retry_enable(gmacdev);
        gmac_pad_crc_strip_disable(gmacdev);
        gmac_back_off_limit(gmacdev, GmacBackoffLimit0);
        gmac_deferral_check_disable(gmacdev);
        gmac_tx_enable(gmacdev); // CDH:TX Enable
        gmac_rx_enable(gmacdev); // CDH:RX Enable

        if(gmacdev->Speed == SPEED1000)
            gmac_select_gmii(gmacdev);
        else
            gmac_select_mii(gmacdev); // cdh:we use mii

        /*Frame Filter Configuration*/
        gmac_frame_filter_enable(gmacdev);
        gmac_set_pass_control(gmacdev, GmacPassControl0);
        gmac_broadcast_enable(gmacdev);
        gmac_src_addr_filter_disable(gmacdev);
        gmac_multicast_enable(gmacdev);
        gmac_dst_addr_filter_normal(gmacdev);
        gmac_multicast_hash_filter_enable(gmacdev);
        gmac_promisc_disable(gmacdev);
        gmac_unicast_hash_filter_disable(gmacdev); //cdH: not can enable

        /*Flow Control Configuration*/
        gmac_unicast_pause_frame_detect_disable(gmacdev);
        gmac_rx_flow_control_enable(gmacdev);
        gmac_tx_flow_control_enable(gmacdev);
    }
    else //Half Duplex
    {
        gmac_wd_enable(gmacdev);
        gmac_jab_enable(gmacdev);
        gmac_frame_burst_enable(gmacdev);
        gmac_jumbo_frame_disable(gmacdev);
        gmac_rx_own_enable(gmacdev);
        gmac_loopback_off(gmacdev);
        gmac_set_half_duplex(gmacdev);
        gmac_retry_enable(gmacdev);
        gmac_pad_crc_strip_disable(gmacdev);
        gmac_back_off_limit(gmacdev, GmacBackoffLimit0);
        gmac_deferral_check_disable(gmacdev);
        gmac_tx_enable(gmacdev);
        gmac_rx_enable(gmacdev);

        if(gmacdev->Speed == SPEED1000)
            gmac_select_gmii(gmacdev);
        else
            gmac_select_mii(gmacdev);

        /*Frame Filter Configuration*/
        gmac_frame_filter_enable(gmacdev);
        gmac_set_pass_control(gmacdev, GmacPassControl0);
        gmac_broadcast_enable(gmacdev);
        gmac_src_addr_filter_disable(gmacdev);
        gmac_multicast_enable(gmacdev);
        gmac_dst_addr_filter_normal(gmacdev);
        gmac_multicast_hash_filter_enable(gmacdev);
        gmac_promisc_disable(gmacdev);
        gmac_unicast_hash_filter_disable(gmacdev);

        /*Flow Control Configuration*/
        gmac_unicast_pause_frame_detect_disable(gmacdev);
        gmac_rx_flow_control_enable(gmacdev);
        gmac_tx_flow_control_enable(gmacdev);

        /*To set PHY register to enable CRS on Transmit*/
        gmac_write_reg((u32 *)gmacdev->MacBase, GmacGmiiAddr, GmiiBusy | 0x00000408);
        PHYreg = gmac_read_reg((u32 *)gmacdev->MacBase, GmacGmiiData);
        gmac_write_reg((u32 *)gmacdev->MacBase, GmacGmiiData, PHYreg   | 0x00000800);
        gmac_write_reg((u32 *)gmacdev->MacBase, GmacGmiiAddr, GmiiBusy | 0x0000040a);
    }
    
    return 0;
}

/**
  * @brief This enables the pause frame generation after programming the appropriate registers.
  *
  * presently activation is set at 3k and deactivation set at 4k. These may have to tweaked
  * if found any issues
  * @author CaoDonghua
  * @date 2016-09-26
  * @param[in] pointer to gmac_device.
  * @return void.
  * @retval none
  */
static void gmac_pause_control(gmac_device *gmacdev)
{
    u32 omr_reg;
    u32 mac_flow_control_reg;
    omr_reg = gmac_read_reg((u32 *)gmacdev->DmaBase, DmaControl);
    omr_reg |= DmaRxFlowCtrlAct4K | DmaRxFlowCtrlDeact5K | DmaEnHwFlowCtrl;
    gmac_write_reg((u32 *)gmacdev->DmaBase, DmaControl, omr_reg);

    mac_flow_control_reg = gmac_read_reg((u32 *)gmacdev->MacBase, GmacFlowControl);
    mac_flow_control_reg |= GmacRxFlowControl | GmacTxFlowControl | 0xFFFF0000;
    gmac_write_reg((u32 *)gmacdev->MacBase, GmacFlowControl, mac_flow_control_reg);

    return;

}

/**
  * @brief Checks whether the descriptor is empty.
  *
  * If the buffer1 and buffer2 lengths are zero in ring mode descriptor is empty.
  * In chain mode buffer2 length is 0 but buffer2 itself contains the next descriptor address.
  * @author CaoDonghua
  * @date 2016-09-26
  * @param[in] pointer to DmaDesc structure.
  * @return returns true if descriptor is empty, false if not empty.
  * @retval true descriptor is empty
  * @retval false not empty
  */
static bool gmac_is_desc_empty(DmaDesc *desc)
{
    return(((desc->length  & DescSize1Mask) == 0) && ((desc->length  & DescSize2Mask) == 0) );
}


/**
  * @brief  Checks whether this rx descriptor is last rx descriptor.
  *
  * This returns true if it is last descriptor either in ring mode or in chain mode.
  * @author CaoDonghua
  * @date 2016-09-26
  * @param[in] pointer to devic structure.
  * @param[in] pointer to DmaDesc structure.
  * @return returns true if it is last descriptor, false if not.
  * @retval none
  * @note This function should not be called before initializing the descriptor using synopGMAC_desc_init().
  */
static bool gmac_is_last_rx_desc(gmac_device *gmacdev, DmaDesc *desc)
{
    return ((desc->length & RxDescEndOfRing) == RxDescEndOfRing);
}

/**
  * @brief clears all the pending interrupts.
  *
  * If the Dma status register is read then all the interrupts gets cleared
  * @author CaoDonghua
  * @date 2016-09-26
  * @param[in] pointer to gmac_device.
  * @return returns void.
  * @retval none
  */
static void gmac_clear_interrupt(gmac_device *gmacdev)
{
    u32 data;
    
    data = gmac_read_reg((u32 *)gmacdev->DmaBase, DmaStatus);
    gmac_write_reg((u32 *)gmacdev->DmaBase, DmaStatus , data);
}


/**
  * @brief Disable the MMC Tx interrupt.
  *
  * The MMC tx interrupts are masked out as per the mask specified.
  * @author CaoDonghua
  * @date 2016-09-26
  * @param[in] pointer to gmac_device.
  * @param[in] tx interrupt bit mask for which interrupts needs to be disabled.
  * @return returns void.
  * @retval none
  */
static void gmac_disable_mmc_tx_interrupt(gmac_device *gmacdev, u32 mask)
{
    gmac_set_bits((u32 *)gmacdev->MacBase, GmacMmcIntrMaskTx, mask);
    
    return;
}


/**
  * @brief disable the MMC Rx interrupt.
  *
  * The MMC rx interrupts are masked out as per the mask specified.
  * @author CaoDonghua
  * @date 2016-09-26
  * @param[in] pointer to gmac_device.
  * @param[in] rx interrupt bit mask for which interrupts needs to be disabled.
  * @return returns void.
  * @retval none
  */
static void gmac_disable_mmc_rx_interrupt(gmac_device *gmacdev, u32 mask)
{
    gmac_set_bits((u32 *)gmacdev->MacBase, GmacMmcIntrMaskRx, mask);
    return;
}


/**
  * @brief disable the MMC ipc rx checksum offload interrupt.
  *
  * The MMC ipc rx checksum offload interrupts are masked out as per the mask specified.
  * @author CaoDonghua
  * @date 2016-09-26
  * @param[in] pointer to gmac_device.
  * @param[in] rx interrupt bit mask for which interrupts needs to be disabled.
  * @return returns void.
  * @retval none
  */
static void gmac_disable_mmc_ipc_rx_interrupt(gmac_device *gmacdev, u32 mask)
{
    gmac_set_bits((u32 *)gmacdev->MacBase, GmacMmcRxIpcIntrMask, mask);
    return;
}



/**
  * @brief enable the DMA reception.
  *
  * enable the DMA controller reception..
  * @author CaoDonghua
  * @date 2016-09-26
  * @param[in] gmacdev gmac device
  * @return void.
  * @retval none
  */   
static void gmac_enable_dma_rx(gmac_device *gmacdev)
{
    u32 data;
    
    data = gmac_read_reg((u32 *)gmacdev->DmaBase, DmaControl);
    data |= DmaRxStart;
    gmac_write_reg((u32 *)gmacdev->DmaBase, DmaControl , data);
}


/**
  * @brief enable the DMA transmission.
  *
  * enable the DMA controller transmission..
  * @author CaoDonghua
  * @date 2016-09-26
  * @param[in] gmacdev gmac device
  * @return void.
  * @retval none
  */  
static void gmac_enable_dma_tx(gmac_device *gmacdev)
{
    u32 data;
    
    data = gmac_read_reg((u32 *)gmacdev->DmaBase, DmaControl);
    data |= DmaTxStart;
    gmac_write_reg((u32 *)gmacdev->DmaBase, DmaControl , data);

}


/**
  * @brief Checks whether this tx descriptor is last tx descriptor.
  *
  * This returns true if it is last descriptor either in ring mode or in chain mode.
  * @author CaoDonghua
  * @date 2016-09-26
  * @param[in] pointer to devic structure.
  * @param[in] pointer to DmaDesc structure.
  * @return bool returns true if it is last descriptor, false if not.
  * @retval true last descriptor
  * @retval false not the last descriptor
  * @note This function should not be called before initializing the descriptor using synopGMAC_desc_init().
  */
static bool gmac_is_last_tx_desc(gmac_device *gmacdev, DmaDesc *desc)
{
    return ((desc->length & TxDescEndOfRing) == TxDescEndOfRing);
}


/**
  * @brief Populate the tx desc structure with the buffer address.
  *
  * Populate the tx desc structure with the buffer address.
  * Once the driver has a packet ready to be transmitted, this function is called with the
  * valid dma-able buffer addresses and their lengths. This function populates the descriptor
  * and make the DMA the owner for the descriptor. This function also controls whetther Checksum
  * offloading to be done in hardware or not.
  * This api is same for both ring mode and chain mode.
  * @author CaoDonghua
  * @date 2016-09-26
  * @param[in] pointer to gmac_device.
  * @param[in] Dma-able buffer1 pointer.
  * @param[in] length of buffer1 (Max is 2048).
  * @param[in] virtual pointer for buffer1.
  * @param[in] Dma-able buffer2 pointer.
  * @param[in] length of buffer2 (Max is 2048).
  * @param[in] virtual pointer for buffer2.
  * @param[in] u32 data indicating whether the descriptor is in ring mode or chain mode.
  * @param[in] u32 indicating whether the checksum offloading in HW/SW.
  * @return returns present tx descriptor index on success. Negative value if error.
  * @retval tx next descriptor index
  */
static s32 gmac_set_tx_qptr(gmac_device *gmacdev, u32 Buffer1, u32 Length1, u32 Data1, u32 Buffer2, u32 Length2, u32 Data2, u32 offload_needed)
{
    u32 txnext = gmacdev->TxNext;
    DmaDesc *txdesc = gmacdev->tx_ring[txnext];
    
    if(!gmac_is_desc_empty(txdesc)) {
        return -1;
	}
	
    txdesc->length |= (((Length1 << DescSize1Shift) & DescSize1Mask) | ((Length2 << DescSize2Shift) & DescSize2Mask));
    txdesc->length |=  (DescTxFirst | DescTxLast | DescTxIntEnable); //Its always assumed that complete data will fit in to one descriptor
    txdesc->buffer1 = Buffer1;
    txdesc->data1   = Data1;
    txdesc->buffer2 = Buffer2;
    txdesc->data2  = Data2;
    txdesc->status = DescOwnByDma;

    return txnext;
}


/**
  * @brief resumes the DMA transmission.
  *
  * the DmaTxPollDemand is written. (the data writeen could be anything).
  * This forces the DMA to resume transmission.
  * @author CaoDonghua
  * @date 2016-09-26
  * @param[in] gmacdev gmac device.
  * @return void.
  * @retval none
  */  
static void gmac_resume_dma_tx(gmac_device *gmacdev)
{
    gmac_write_reg((u32 *)gmacdev->DmaBase, DmaTxPollDemand, 0);
}


/**
  * @brief resumes the DMA reception.
  *
  * resumes the DMA reception.
  * @author CaoDonghua
  * @date 2016-09-26
  * @param[in] gmacdev gmac device.
  * @return void.
  * @retval none
  */
static void gmac_resume_dma_rx(gmac_device *gmacdev)
{
    gmac_write_reg((u32 *)gmacdev->DmaBase, DmaRxPollDemand, 0);
}


/**
  * @brief disable the DMA for transmission.
  *
  * disable the DMA for transmission.
  * @author CaoDonghua
  * @date 2016-09-26
  * @param[in] gmacdev gmac device.
  * @return void.
  * @retval none
  */
static void gmac_disable_dma_tx(gmac_device *gmacdev)
{
    u32 data;
    
    data = gmac_read_reg((u32 *)gmacdev->DmaBase, DmaControl);
    data &= (~DmaTxStart);
    gmac_write_reg((u32 *)gmacdev->DmaBase, DmaControl , data);
}


/**
  * @brief disable the DMA for reception.
  *
  * disable the DMA controller for reception.
  * @author CaoDonghua
  * @date 2016-09-26
  * @param[in] gmacdev gmac device.
  * @return void.
  * @retval none
  */  
static void gmac_disable_dma_rx(gmac_device *gmacdev)
{
    u32 data;
    
    data = gmac_read_reg((u32 *)gmacdev->DmaBase, DmaControl);
    data &= (~DmaRxStart);
    gmac_write_reg((u32 *)gmacdev->DmaBase, DmaControl , data);
}


/**
  * @brief get descriptor status owner
  *
  * get descriptor status owner,indicate descriptor status
  * @author CaoDonghua
  * @date 2016-09-26
  * @param[in] desc rx descriptor.
  * @return int dma owner status.
  * @retval dma onwer value
  */
static int smc_desc_get_owner(DmaDesc *desc)
{
	return  (desc->status & DescOwnByDma);
}


/**
  * @brief get rx frame len from rx descriptor 
  *
  * get rx frame len from rx descriptor 
  * @author CaoDonghua
  * @date 2016-09-26
  * @param[in] desc rx descriptor.
  * @return int rx frame len.
  * @retval rx one frame len
  */
static int smc_desc_get_rx_frame_len(DmaDesc *desc)
{
	u32 data = desc->status;
	u32 len = (data & DescFrameLengthMask) >> DescFrameLengthShift;
	
	return len;
}

/**
  * @brief get rx buffer address from rx descriptor 
  *
  * get rx buffer address from rx descriptor 
  * @author CaoDonghua
  * @date 2016-09-26
  * @param[in] desc rx descriptor.
  * @return void.
  * @retval none
  */
static void *smc_desc_get_buf_addr(DmaDesc *desc)
{
	return (void *)desc->buffer1;
}

/**
  * @brief set rx descriptor owner
  *
  * set rx descriptor owner for telling dma controller desc status
  * @author CaoDonghua
  * @date 2016-09-26
  * @param[in] desc rx descriptor.
  * @return void.
  * @retval none
  */
static void smc_desc_set_rx_owner(DmaDesc *desc)
{
	desc->status = DescOwnByDma;
}



/**
  * @brief gmac open and initialize board
  *
  * gmac open and initialize board, set up everything, reset the card, etc ..
  * @author CaoDonghua
  * @date 2016-09-26
  * @param[in] dev ethernet device.
  * @param[in] bd board config infor.
  * @return int 0 success ok, else  failed.
  * @retval return 0 on success
  * @retval return -1  on failed
  */
static int smc_init(struct eth_device *dev, bd_t *bd)
{
	signed int 		ijk;
	gmac_device 	*gmacdev;
	nt_adapter 		*adapter;
	
  	debug("================smc_init========\r\n");

	adapter = (nt_adapter *)dev->priv;
	gmacdev = (gmac_device *)adapter->gmacdev_pt;
	
  	/* initial all mii or rmii share pin and clock,and complete phy reset from gpio62 */
#if defined(CONFIG_ETHERNET_MAC_MII)  	
  	gmac_mii_sharepin_cfg();
#else
	gmac_rmii_sharepin_cfg();
#endif

  	gmac_ctrl_init();

  	/* now platform dependent initialization */
	gmac_mmc_counters_reset(gmacdev); // Mac Management Counters (MMC)
	gmac_mmc_counters_disable_rollover(gmacdev); 

	/* software reset , the resets all of the GMAC internal registers and logic, cdh:check ok */ 
	if (gmac_reset(gmacdev)) {
		printf("%s, no find phy device!\n", __func__);
		return -1;
	}

	/* program flash in the station IP's Mac address */
	gmac_set_mac_addr(gmacdev, GmacAddr0High, GmacAddr0Low, dev->enetaddr);
	
	/* Lets read the version of ip */
	gmac_read_version(gmacdev);

	/* Lets set ipaddress in to device structure */
	gmac_get_mac_addr(gmacdev, GmacAddr0High, GmacAddr0Low, dev->enetaddr);

	/* Now set the broadcast address */
	for(ijk = 0; ijk < 6; ijk++) {
		adapter->broadcast[ijk] = 0xff;
	}

#if 0
	for(ijk = 0; ijk < 6; ijk++) {
		debug("adapter->dev_addr[%d] = %02x and adapter->broadcast[%d] = %02x\n", ijk, dev->enetaddr[ijk], ijk, adapter->broadcast[ijk]);
	}
#endif

	/* set MDO_CLK for MDIO transmit, note GmacGmiiAddr bit5, and 802.3 limit 2.5MHz div=42 */ 
	gmac_set_mdc_clk_div(gmacdev, GmiiCsrClk0);

	/* get MDO_CLK div */ 
	gmacdev->ClockDivMdc = synopGMAC_get_mdc_clk_div(gmacdev);

	/* initial phy and check phy link status */ 
	gmac_phy_init(gmacdev);

	/* 申请并创建512个tx 环行的描述符链表，并做初始化 */ 
	setup_tx_desc_queue(gmacdev, dev, TRANSMIT_DESC_SIZE, RINGMODE);
	
	/* 将首个tx描述符物理地址写入mac的dma寄存器。 */ 
	gmac_init_tx_desc_base(gmacdev); // cdh:Program the transmit descriptor base address in to DmaTxBase addr

	/* 申请并创建512个rx 环行的描述符链表，并做初始化 */  
	setup_rx_desc_queue(gmacdev, dev, RECEIVE_DESC_SIZE, RINGMODE);
	
	/* 将首个rx描述符物理地址写入mac的dma寄存器。 */ 
	gmac_init_rx_desc_base(gmacdev); // cdh:Program the transmit descriptor base address in to DmaTxBase addr
	
	/* dma busrt=32words=128B, two dma_descriptor interval = 2Bytes  */ 
	gmac_set_dma_bus_mode(gmacdev, DmaBurstLength32 | DmaDescriptorSkip2); //pbl32 incr with rxthreshold 128

	/* set dma transmit method */ 
	gmac_set_dma_control(gmacdev, DmaDisableFlush | DmaStoreAndForward | DmaTxSecondFrame | DmaRxThreshCtrl128);

	/* initial phy register and mac part ip */ 
	gmac_check_phy_init(gmacdev);
	gmac_mac_init(gmacdev);

	/**
	 *inital dma and mac flow control
	*/
	gmac_pause_control(gmacdev); // This enables the pause control in Full duplex mode of operation

	/**
	 *clear all the interrupts
	*/
	gmac_clear_interrupt(gmacdev);

	/**
	*Disable the interrupts generated by MMC and IPC counters.
	*If these are not disabled ISR should be modified accordingly to handle these interrupts.
	 */
	gmac_disable_mmc_tx_interrupt(gmacdev, 0xFFFFFFFF); // cdh:set 1for mask interrupt
	gmac_disable_mmc_rx_interrupt(gmacdev, 0xFFFFFFFF);
	gmac_disable_mmc_ipc_rx_interrupt(gmacdev, 0xFFFFFFFF);

	/**
	 *Enable Tx and Rx DMA
	*/
	gmac_enable_dma_rx(gmacdev);
	gmac_enable_dma_tx(gmacdev);

	return 0;
}

/**
  * @brief gmac controller halt transmit data
  *
  * gmac controller halt transmit data
  * @author CaoDonghua
  * @date 2016-09-26
  * @param[in] dev ethernet device.
  * @return void none.
  * @retval return none
  */
static void smc_halt(struct eth_device *dev)
{
	gmac_device 	*gmacdev;
	nt_adapter 		*adapter;
	
	adapter = (nt_adapter *)dev->priv;
	gmacdev = (gmac_device *)adapter->gmacdev_pt;

	/* disable gmac controller tx/rx */
	gmac_tx_disable(gmacdev);
    gmac_rx_disable(gmacdev);

    /* disable gmac dma tx/rx */
	gmac_disable_dma_tx(gmacdev);
	gmac_disable_dma_rx(gmacdev);

	/* must set to 0, or when started up will cause issues */
	gmacdev->TxNext = 0;
	gmacdev->RxNext = 0;

}



/**
  * @brief gmac controller transmit data
  *
  * gmac controller transmit data from phy device
  * @author CaoDonghua
  * @date 2016-09-26
  * @param[in] dev ethernet device.
  * @param[in] packet transmit data buffer pointer.
  * @param[in] packet_length transmit data length.
  * @return int length transmit ok, else  negative data set failed.
  * @retval returns transmit le ngth on success
  * @retval returns negative one on failed
  */
static int smc_send(struct eth_device *dev, void *packet, int packet_length)
{
   	gmac_device 	*gmacdev;
	nt_adapter 		*adapter;
	int timeout;
	u32 txnext;
	DmaDesc *txdesc;
	
	adapter = (nt_adapter *)dev->priv;
	gmacdev = (gmac_device *)adapter->gmacdev_pt;

    /* set tx descriptor  for ready transmit*/
    gmac_set_tx_qptr(gmacdev, (u32)packet, (u32)packet_length, (u32)packet, 0, 0, 0, 0);

	/* write poll demand start transmit */
    gmac_resume_dma_tx(gmacdev);

    txnext = gmacdev->TxNext;
	txdesc = gmacdev->tx_ring[txnext];

	/* wait for transmit finish */
	timeout = 1000000;
	while (smc_desc_get_owner(txdesc)) {
		if (timeout-- < 0) {
			printf("gmac: tx timeout\n");
			return -ETIMEDOUT;
		}
		udelay(1);
	}

	/* update mac controller transmit descriptor ring */
	gmacdev->TxNext = gmac_is_last_tx_desc(gmacdev, txdesc) ? 0 : txnext + 1;
	gmac_tx_desc_init_ring(txdesc, gmac_is_last_tx_desc(gmacdev, txdesc));
	
	return packet_length;
	
}

/**
  * @brief invalidate and clean cpu dcache
  *
  * invalidate and clean cpu dcache when dma received data and changed sdram data
  * but cpu unknown, dcache also unknown
  * @author CaoDonghua
  * @date 2016-09-26
  * @param[in] void none.
  * @return void none.
  * @retval void none
  */
static void l2cache_clean_invalidate(void)
{
	__asm__ __volatile__(
		"MMU_Clean_Invalidate_Dcache:\n\t" 
		"mrc  p15,0,r15,c7,c14,3\n\t" 
		"bne MMU_Clean_Invalidate_Dcache\t"); 

}


/**
  * @brief gmac controller receive data
  *
  * gmac controller receive data from phy device
  * @author CaoDonghua
  * @date 2016-09-26
  * @param[in] dev ethernet device.
  * @return int 0 set mac address ok, else set failed.
  * @retval returns received length on success
  * @retval returns negative one on failed
  */
static int smc_rcv(struct eth_device *dev)
{
	gmac_device 	*gmacdev;
	nt_adapter 		*adapter;
	u32 rxnext;	
    DmaDesc *rxdesc;
	int length = 0;

	adapter = (nt_adapter *)dev->priv;
	gmacdev = (gmac_device *)adapter->gmacdev_pt;
	rxnext  = gmacdev->RxNext;	
	rxdesc  = gmacdev->rx_ring[rxnext];
				
	/* 
	* make sure we see the changes made by the DMA engine 
	* check if the host(cpu)  has the desc,  no data receive, must wait for 
	*/
	if (smc_desc_get_owner(rxdesc)) {
		/* something bad happened */
		return -1; 
	}
	
	/* cdh:crc len 4B, so must decrease 4Byte */
	length = smc_desc_get_rx_frame_len(rxdesc) - 4;

#if 0
	l2cache_clean_invalidate();
#endif

	/* procced received data */
	NetReceive(smc_desc_get_buf_addr(rxdesc), length);

	/* re-init gmac ring descriptor  */
	gmacdev->RxNext = gmac_is_last_rx_desc(gmacdev, rxdesc) ? 0 : rxnext + 1;
	gmac_rx_desc_init_ring3(gmacdev->RxNext, rxdesc, gmac_is_last_rx_desc(gmacdev, rxdesc));

	/* set descriptor back to owned by XGMAC */
	smc_desc_set_rx_owner(rxdesc);

	/* poll cmd start  */
	gmac_resume_dma_rx(gmacdev);

	return length;
	
}


/**
  * @brief re-set gmac mac address
  *
  * re-set gmac mac address for mac controller
  * @author CaoDonghua
  * @date 2016-09-26
  * @param[in] dev ethernet device.
  * @return int 0 set mac address ok, else set failed.
  * @retval returns zero on success
  * @retval returns no-zero on failed
  */
static int smc_write_hwaddr(struct eth_device *dev)
{
	gmac_device 	*gmacdev;
	nt_adapter 		*adapter;
    
	debug("================smc_write_hwaddr========\r\n");
	debug("name:%s, addr:%x:%x:%x:%x:%x:%x\r\n", dev->name, 
		dev->enetaddr[0], dev->enetaddr[1], dev->enetaddr[2], dev->enetaddr[3], dev->enetaddr[4], dev->enetaddr[5]);

	adapter = (nt_adapter *)dev->priv;
	gmacdev = (gmac_device *)adapter->gmacdev_pt;
    gmac_set_mac_addr(gmacdev, GmacAddr0High, GmacAddr0Low, dev->enetaddr);

	return 0;
}



/**
  * @brief init gmac mii interface sharepin config
  *
  * config gmac mii interface sharepin
  * @author CaoDonghua
  * @date 2016-09-26
  * @param[in] void.
  * @return void.
  */
#if defined(CONFIG_ETHERNET_MAC_MII)    
static void gmac_mii_sharepin_cfg(void)
{
	unsigned long val = 0;
	
	/* TODO: 
	* Need to be changed to new clock, to enable the 25MHz oscillator 
	* h2 bit [10:9]=01, set sharepin gpio47 share as opclk
	*/
	val = REG32(SHARE_PIN_CFG_REG1);
    val &= ~(0x3<<10);
    val |= (0x1<<10);  				// CDH:H3 CHECK modify OK
    REG32(SHARE_PIN_CFG_REG1) = val;
    
    val = REG32(SHARE_PIN_CFG_REG3);
    val &= ~(0xfffffff<<0);
    val |= (0x575f5a5 <<0);
    REG32(SHARE_PIN_CFG_REG3) = val;

    REG32(SHARE_PIN_CFG_REG3) &= ~(0x1 << 4);  // cdh:gpio12,
	REG32(GPIO_DIR_REG1) &= ~(0x1 << 12);  // cdh:gpio12, receive input PHY interrupt 

}
#endif

/**
* @brief init gmac rmii interface sharepin config
*
* config gmac rmii interface sharepin
* @author CaoDonghua
* @date 2016-09-26
* @param[in] void.
* @return void.
*/
#if defined(CONFIG_ETHERNET_MAC_RMII)  
static void gmac_rmii_sharepin_cfg(void)
{
	unsigned long val = 0;

	/* TODO: 
	* Need to be changed to new clock, to enable the 25MHz oscillator 
	* h2 bit [10:9]=01, set sharepin gpio47 share as opclk
	*/
	val = REG32(SHARE_PIN_CFG_REG1);
    val &= ~(0x3<<10);
    val |= (0x1<<10);
    REG32(SHARE_PIN_CFG_REG1) = val;
    
    val = REG32(SHARE_PIN_CFG_REG3);
    val &= ~(0xfffffff<<0);
    val |= (0x1450525<<0);
    REG32(SHARE_PIN_CFG_REG3) = val;

    REG32(SHARE_PIN_CFG_REG3) &= ~(0x1 << 4);  // cdh:gpio12,
	REG32(GPIO_DIR_REG1) &= ~(0x1 << 12);  // cdh:gpio12, input
}	
#endif


/**
  * @brief enable or disable gpio47 pulldown
  *
  * enable or disable gpio47 pulldown and it share as 25M/50M mclk pin
  * @author CaoDonghua
  * @date 2016-09-26
  * @param[in] gpio_pddis_en enable or disable pull down.
  * @return void.
  */
static void ak39e_gpio47_pulldown(T_GPIO_PD_CONFG gpio_pddis_en)
{
	/* set chip system ctrl reg base address, 1:disable, 0:enable */ 
	if (gpio_pddis_en){
		REG32(AK_PA_SYSCTRL + 0x80) |= (0x1<<8); // cdh:disable gpio47 pull down
	}else {
		REG32(AK_PA_SYSCTRL + 0x80) &= ~(0x1<<8); // cdh:enable gpio47 pull down
	}
}

/**
* @brief init gmac mii interface 25m clk
*
* init gmac controller mii interface 25m clk.
* @author CaoDonghua
* @date 2016-09-26
* @param[in] void.
* @return void.
*/
#if defined(CONFIG_ETHERNET_MAC_MII)  
static void gmac_mii_interface_25mclk(void)
{
	int i = 0;

	/* mii, select top mii interface and 100mbps speed, and make 25M clock to phy  */ 
	REG32(AK_PA_SYSCTRL + 0x14) |=  ((0x1 << 22)|(0x1 << 23)); // cdh:first mac interface select mii, mac_speed_cfg=1(100m)
	REG32(AK_PA_SYSCTRL + 0x14) &= ~(0x1 << 15); // cdh:clear bit[15], prohibit 25m crystal
	REG32(AK_PA_SYSCTRL + 0x14) |=  ((0x1 << 16)|(0x1 << 18)); // cdh:set   bit[16],enable div24, generate 25m,	bit[18], select 25m clock of mac from pll div
	REG32(AK_PA_SYSCTRL + 0x1c) &= ~(1 << 13);		 // cdh:mac clk ctrl

	/* bit[20]:select mac 25M clock from 25M crystal or pll div,here do what's mean? ,pg said at least repeat twice */ 
	for(i=0; i<6; i++) {
		REG32(AK_PA_SYSCTRL + 0x14) |= (1 << 20);	// cdh:select 25m crystal pad, what's mean?
		REG32(AK_PA_SYSCTRL + 0x14) &= ~(1 << 20);	// cdh:select 25m clock input, what's mean?
	}

}
#endif

#if defined(CONFIG_ETHERNET_MAC_RMII)
static void gmac_rmii_interface_50mclk(void)
{
	int i = 0;

	/* Rmii, select top rmii interface and 100mbps speed, and make 50M clock to phy */ 
	REG32(AK_PA_SYSCTRL + 0x14) &=  ~(0x1 << 22); // cdh:first mac interface select Rmii
	REG32(AK_PA_SYSCTRL + 0x14) |=  (0x1 << 23); // cdh:first  mac_speed_cfg=1(100m)
	REG32(AK_PA_SYSCTRL + 0x14) |=  (0x1 << 21); // cdh:bit[21],enable generate 50m
	REG32(AK_PA_SYSCTRL + 0x14) |=  ((0x1 << 16)|(0x1 << 18)); // cdh:set   bit[21],enable generate 50m,	bit[18], select 25m clock of mac from pll div
	REG32(AK_PA_SYSCTRL + 0x1c) &= ~(1 << 13);		 // cdh:mac ctronller clk 

	/* bit[20]:select mac 50M clock from 50M crystal or pll div,here do what's mean? ,pg said at least repeat twice */ 
	for(i=0; i<6; i++) {
		REG32(AK_PA_SYSCTRL + 0x14) |= (1 << 20);	// cdh:select 50m crystal pad, what's mean?
		REG32(AK_PA_SYSCTRL + 0x14) &= ~(1 << 20);	// cdh:select 50m clock input, what's mean?
	}

}
#endif


/**
  * @brief init gmac module clk and sharepin for mii interface 
  *
  * init gmac module clk and sharepin for mii interface 
  * @author CaoDonghua
  * @date 2016-09-26
  * @param[in] void none
  * @return void none
  */
static void gmac_clk_sharepin_init(void)
{
#if defined(CONFIG_ETHERNET_MAC_RMII)
	debug("Configed MAC RMII interface!\n");
	
	/* set rmii interface and 50m phy clock sharepin */ 
	gmac_rmii_sharepin_cfg();

	/* disable gpio47 pull down */ 
	ak39e_gpio47_pulldown(AK_PULLDOWN_DISABLE);

	/* open 50M phy clock */ 
	gmac_rmii_interface_50mclk();

#elif defined(CONFIG_ETHERNET_MAC_MII)	
	debug("Configed MAC MII interface!\n");
	
	/* set mii interface and 25m phy clock sharepin  */ 
	gmac_mii_sharepin_cfg();

	/* disable gpio47 pull down */ 
	ak39e_gpio47_pulldown(AK_PULLDOWN_DISABLE);

	/* open 25M phy clock */ 
	gmac_mii_interface_25mclk();

#else
	debug("Please config MAC interface Failed!\n");
#endif

}


/**
  * @brief attach ak39e mac hardware address
  *
  * attaches the gmac device structure to the hardware.
  * device structure is populated with mac/dma and phy base addresses.
  * @author CaoDonghua
  * @date 2016-09-26
  * @param[in] pointer to gmac_device to populate mac dma and phy addresses.
  * @param[in] gmac ip mac base address.
  * @param[in] gmac ip dma base address.
  * @param[in] gmac ip phy base address.
  * @return s32 0 upon success. Error code upon failure.
  * @retval returns zero on success
  * @retval return	non-zero if failed
  */
static s32 gmac_attach(gmac_device *gmacdev, u32 macBase, u32 dmaBase, u32 phyBase)
{
	
    /* make sure the device data strucure is cleared before we proceed further */
    memset((void *) gmacdev, 0, sizeof(gmac_device));
    
    /* populate the mac and dma base addresses*/
    gmacdev->MacBase = macBase;
    gmacdev->DmaBase = dmaBase;
    gmacdev->PhyBase = phyBase;
	
    return 0;
}

/**
* @brief  ak39e ethernet device driver initialize
*
*  initialize and register ak39e ethernet device driver 
* @author CaoDonghua
* @date 2016-09-26
* @param[in] dev_num  ethernet device num.
* @param[in] base_addr   ethernet mac controller register base address.
* @return int return initial and register ethernet success or failed
* @retval returns one on success
* @retval return  zero if failed
*/
int ak39_ethernet_initialize(u8 dev_num, int base_addr)
{
	struct eth_device *dev;
	nt_adapter        *adapter_pt;
	unsigned char	  dev_addr[MAC_ADDR_LEN] = DEFAULT_MAC_ADDRESS;
	
	debug("=========akethernet_initialize=============\r\n");

	/*
	* malloc a eth_device struct space
	*/ 
	dev = malloc(sizeof(*dev));
	if (!dev) {
		printf("malloc eth_device space Err!\n");
		return 0;
	}

	/* initial eth_device all element default value */
	memset(dev, 0, sizeof(*dev));

	/*
	* malloc a eth_device adapter struct space
	*/
	adapter_pt = malloc(sizeof(*adapter_pt));
	if (!adapter_pt) {
		printf("malloc eth_device adapter space Err!\n");
		free(dev);
		return 0;
	}
	
	/* setup board info structure */
	dev->priv = adapter_pt;
	adapter_pt->netdev_pt 	= dev;
    adapter_pt->dev_num 	= dev_num;

	/* Allocate Memory for the GMACip structure */
	RingbufVa = malloc(sizeof (gmac_device));
	adapter_pt->gmacdev_pt = (gmac_device *)RingbufVa;
	if(!adapter_pt->gmacdev_pt) {
        printf("error in gmac_device memory allocataion \n");
        free(dev);
        free(adapter_pt);
		return 0;
    }else {
	    debug("allocataion gmacdev OK\n");
    }

	dev->iobase = base_addr; // cdh:0x20300000
	dev->init = smc_init;
	dev->halt = smc_halt;
	dev->send = smc_send;
	dev->recv = smc_rcv;
	dev->write_hwaddr = smc_write_hwaddr;
	sprintf(dev->name, "%s-%hu", SMC_DEV_NAME, dev_num);

	/* cfg mii share pin and open mac controller clock */ 
	gmac_clk_sharepin_init();

	/* set mac address */ 
	memcpy(adapter_pt->dev_addr, dev_addr, 6);
	memcpy(dev->enetaddr, adapter_pt->dev_addr, 6);

	/* set mac ctrl address */ 
	gmac_attach(adapter_pt->gmacdev_pt, (u32) dev->iobase + MACBASE, (u32) dev->iobase + DMABASE, DEFAULT_PHY_BASE);

	/* register  ethernet device */ 
	eth_register(dev);
	
	return 1;
}

