#ifndef _LINUX_NETDEVICE_H
#define _LINUX_NETDEVICE_H

/* source back-compat hooks */
#define SET_ETHTOOL_OPS(netdev,ops) \
	( (netdev)->ethtool_ops = (ops) )

#define HAVE_ALLOC_NETDEV		/* feature macro: alloc_xxxdev
					   functions are available. */
#define HAVE_FREE_NETDEV		/* free_netdev() */
#define HAVE_NETDEV_PRIV		/* netdev_priv() */

/* hardware address assignment types */
#define NET_ADDR_PERM		0	/* address is permanent (default) */
#define NET_ADDR_RANDOM		1	/* address is generated randomly */
#define NET_ADDR_STOLEN		2	/* address is stolen from other device */

/* Backlog congestion levels */
#define NET_RX_SUCCESS		0	/* keep 'em coming, baby */
#define NET_RX_DROP		1	/* packet dropped */

/* qdisc ->enqueue() return codes. */
#define NET_XMIT_SUCCESS	0x00
#define NET_XMIT_DROP		0x01	/* skb dropped			*/
#define NET_XMIT_CN		0x02	/* congestion notification	*/
#define NET_XMIT_POLICED	0x03	/* skb is shot by police	*/
#define NET_XMIT_MASK		0x0f	/* qdisc flags in net/sch_generic.h */

/* NET_XMIT_CN is special. It does not guarantee that this packet is lost. It
 * indicates that the device will soon be dropping packets, or already drops
 * some packets of the same priority; prompting us to send less aggressively. */
#define net_xmit_eval(e)	((e) == NET_XMIT_CN ? 0 : (e))
#define net_xmit_errno(e)	((e) != NET_XMIT_CN ? -ENOBUFS : 0)

/* Driver transmit return codes */
#define NETDEV_TX_MASK		0xf0

enum netdev_tx {
	__NETDEV_TX_MIN	 = -1,	/* make sure enum is signed */
	NETDEV_TX_OK	 = 0x00,	/* driver took care of packet */
	NETDEV_TX_BUSY	 = 0x10,	/* driver tx path was busy*/
	NETDEV_TX_LOCKED = 0x20,	/* driver tx lock was already taken */
};
typedef enum netdev_tx netdev_tx_t;

/*
 * Current order: NETDEV_TX_MASK > NET_XMIT_MASK >= 0 is significant;
 * hard_start_xmit() return < NET_XMIT_MASK means skb was consumed.
 */


#define MAX_ADDR_LEN	32		/* Largest hardware address length */

/* Initial net device group. All devices belong to group 0 by default. */
#define INIT_NETDEV_GROUP	0


/* Media selection options. */
enum {
        IF_PORT_UNKNOWN = 0,
        IF_PORT_10BASE2,
        IF_PORT_10BASET,
        IF_PORT_AUI,
        IF_PORT_100BASET,
        IF_PORT_100BASETX,
        IF_PORT_100BASEFX
};  


#define TC_MAX_QUEUE	16
#define TC_BITMASK	15
/* HW offloaded queuing disciplines txq count and offset maps */
struct netdev_tc_txq {
	u16 count;
	u16 offset;
};


struct net_device_stats {
	unsigned long	rx_packets;
	unsigned long	tx_packets;
	unsigned long	rx_bytes;
	unsigned long	tx_bytes;
	unsigned long	rx_errors;
	unsigned long	tx_errors;
	unsigned long	rx_dropped;
	unsigned long	tx_dropped;
	unsigned long	multicast;
	unsigned long	collisions;
	unsigned long	rx_length_errors;
	unsigned long	rx_over_errors;
	unsigned long	rx_crc_errors;
	unsigned long	rx_frame_errors;
	unsigned long	rx_fifo_errors;
	unsigned long	rx_missed_errors;
	unsigned long	tx_aborted_errors;
	unsigned long	tx_carrier_errors;
	unsigned long	tx_fifo_errors;
	unsigned long	tx_heartbeat_errors;
	unsigned long	tx_window_errors;
	unsigned long	rx_compressed;
	unsigned long	tx_compressed;
};




#ifndef IFNAMSIZ
#define IFNAMSIZ 16
#endif

#ifndef ETH_ALEN
#define ETH_ALEN            6
#endif
struct net_device {

	char			name[IFNAMSIZ];

	struct net_device_stats	stats;

	/* Management operations */
	const struct net_device_ops *netdev_ops;
	const struct ethtool_ops *ethtool_ops;
	const struct cfg80211_ops *cfg80211_ops;
		
	/* Hardware header description */
	const struct header_ops *header_ops;

	unsigned int		flags;	/* interface flags (a la BSD)	*/
	

	unsigned int		mtu;	/* interface MTU value		*/
	unsigned short		type;	/* interface hardware type	*/
	unsigned short		hard_header_len;	/* hardware hdr length	*/

	
	unsigned char		addr_len;	/* hardware address length	*/
	
	unsigned char	mc_addr[ETH_ALEN];	/* Multicast mac addresses */
	
	struct wireless_dev	*ieee80211_ptr;	/* IEEE 802.11 specific data,
						   assign before registering */


	/* Interface address info used in eth_type_trans() */
	unsigned char		dev_addr[ETH_ALEN];	/* hw address, (before bcast
						   because most packets are
						   unicast) */

	unsigned long		tx_queue_len;	/* Max frames per queue allowed */

	void				*ml_priv;  //= lbs_privata
 

};
#define to_net_dev(d) container_of(d, struct net_device, dev)

#define	NETDEV_ALIGN		32

struct net_device_ops {
	int			(*ndo_open)(struct net_device *dev);
	int			(*ndo_stop)(struct net_device *dev);
	int			(*ndo_start_xmit) (struct sk_buff *skb,
						   struct net_device *dev);
	int			(*ndo_set_mac_address)(struct net_device *dev,
						       void *addr);
	void			(*ndo_set_multicast_list)(struct net_device *dev);
	int			(*ndo_do_ioctl)(struct net_device *dev,
					        struct ifreq *ifr, int cmd);
	void			(*ndo_tx_timeout) (struct net_device *dev);
	struct net_device_stats* (*ndo_get_stats)(struct net_device *dev);
	void			(*ndo_set_rx_mode)(struct net_device *dev);
	u16			(*ndo_select_queue)(struct net_device *dev,
						    struct sk_buff *skb);
	
};


#endif /* __KERNEL__ */
