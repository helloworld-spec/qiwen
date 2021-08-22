#ifndef __DHCPS_H__
#define __DHCPS_H__

typedef struct dhcps_state{
        short state;
} dhcps_state;

// ����dhcpclient�Զ����һ��DHCP msg�ṹ��
typedef struct dhcps_msg {
        unsigned char op, htype, hlen, hops;
        unsigned char xid[4];
        unsigned short secs, flags;
        unsigned char ciaddr[4];
        unsigned char yiaddr[4];
        unsigned char siaddr[4];
        unsigned char giaddr[4];
        unsigned char chaddr[16];
        unsigned char sname[64];
        unsigned char file[128];
        unsigned char options[312];
}dhcps_msg;

#ifndef LWIP_OPEN_SRC
struct dhcps_lease {
	unsigned int start_ip;
	unsigned int end_ip;
};
#endif

struct dhcps_pool{
	struct ip_addr ip;
	unsigned char mac[6];
	unsigned int lease_timer;
};

typedef struct _list_node{
	void *pnode;
	struct _list_node *pnext;
}list_node;

#define sfree mem_free
#define smalloc mem_malloc

#define DHCPS_LEASE_TIMER 0x05A0
#define DHCPS_MAX_LEASE 0x64
#define BOOTP_BROADCAST 0x8000

#define DHCP_REQUEST        1
#define DHCP_REPLY          2
#define DHCP_HTYPE_ETHERNET 1
#define DHCP_HLEN_ETHERNET  6
#define DHCP_MSG_LEN      236

#define DHCPS_SERVER_PORT  67
#define DHCPS_CLIENT_PORT  68

#define DHCPDISCOVER  1
#define DHCPOFFER     2
#define DHCPREQUEST   3
#define DHCPDECLINE   4
#define DHCPACK       5
#define DHCPNAK       6
#define DHCPRELEASE   7

#define DHCP_OPTION_SUBNET_MASK   1
#define DHCP_OPTION_ROUTER        3
#define DHCP_OPTION_DNS_SERVER    6
#define DHCP_OPTION_REQ_IPADDR   50
#define DHCP_OPTION_LEASE_TIME   51
#define DHCP_OPTION_MSG_TYPE     53
#define DHCP_OPTION_SERVER_ID    54
#define DHCP_OPTION_INTERFACE_MTU 26
#define DHCP_OPTION_PERFORM_ROUTER_DISCOVERY 31
#define DHCP_OPTION_BROADCAST_ADDRESS 28
#define DHCP_OPTION_REQ_LIST     55
#define DHCP_OPTION_END         255




#define DHCPS_STATE_OFFER 1
#define DHCPS_STATE_DECLINE 2
#define DHCPS_STATE_ACK 3
#define DHCPS_STATE_NAK 4
#define DHCPS_STATE_IDLE 5

void dhcps_start();
void dhcps_stop(void);

#endif

