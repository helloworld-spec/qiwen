/******************************************************************************

  Copyright (C), 2014-, GuangZhou JUAN Electronics Co., Ltd.

 ******************************************************************************
  File Name    : rtp.c
  Version       : Initial Draft
  Author        : kejiazhw@gmail.com(kaga)
  Created       : 2014/3/19
  Last Modified : 2014/3/19
  Description   : arp  utils , reference to rfc826
 				LINUX:SOCK_PACKET
  History       :
  1.Date        : 2014/3/19
    	Author      : kaga
 	Modification: Created file
******************************************************************************/
#define HAVE_ARP
#ifdef HAVE_ARP

#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdbool.h>
#include<stdint.h>
#include<errno.h>
#include<ctype.h>

#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <signal.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/ip_icmp.h>

#include <features.h> /* 需要里面的 glibc 版本号 */
#if __GLIBC__ >= 2 && __GLIBC_MINOR >= 1
#include <netpacket/packet.h>
#include <net/ethernet.h> /* 链路层（L2）协议 */
#else
#include <asm/types.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h> /* 链路层协议 */
#endif
#include <net/if_arp.h>

//#include "generic.h" //crc32
#include "arp.h"
#include "ak_thread.h"

#define ETH_HW_ADDR_LEN 	6
#define IPV4_ADDR_LEN 			4
#define ARP_FRAME_TYPE 		0x0806
#define ETHER_HW_TYPE 		1
#define IP_PROTO_TYPE 		0x0800
#define BROADCAST_HW_ADDR 	"FF:FF:FF:FF:FF:FF"
#define EMPTY_HW_ADDR		"00:00:00:00:00:00"
#define EMPTY_IP_ADDR		"0.0.0.0"

#define ARP_CHECK_RET(ret) \
	if ((ret) == (-1)) return -1
#define ARP_CHECK_RET_GOTO(ret) \
			if ((ret) == (-1)) goto _FAILED_EXIT

enum {
    OP_ARP_REQUEST = 1,
    OP_ARP_REPLY,
    OP_RARP_REQUEST,
    OP_RARP_REPLY,
    OP_DRARP_REQUEST,
    OP_DRARP_REPLY,
    OP_DRARP_ERROR,
    OP_INARP_REQUEST,
    OP_INARP_REPLY,
};

struct arp_packet {
    u_char targ_hw_addr[ETH_HW_ADDR_LEN];
    u_char src_hw_addr[ETH_HW_ADDR_LEN];
    u_short frame_type;
    u_short hw_type;
    u_short prot_type;
    u_char hw_addr_size;
    u_char prot_addr_size;
    u_short op;
    u_char sndr_hw_addr[ETH_HW_ADDR_LEN];
    u_char sndr_ip_addr[IPV4_ADDR_LEN];
    u_char rcpt_hw_addr[ETH_HW_ADDR_LEN];
    u_char rcpt_ip_addr[IPV4_ADDR_LEN];
    u_char padding[18];
    //u_int		crc32;
};

static char sArpDefEther[16] = {"eth0" };
#define DEFAULT_DEVICE sArpDefEther

static bool gArpTrigger = false;
//static ak_pthread_t gArpPid = 0;
//static fAddOnePeer arp_hook = NULL;

/*
 * get_ip_addr - 
 * in_addr[OUT]: in_addr struct
 * str[OUT]: ip addr
 * return: 0 success , -1 failed
 */
int get_ip_addr(struct in_addr* in_addr, char* str)
{
    struct hostent* hostp;
    in_addr->s_addr = inet_addr(str);
    if (in_addr->s_addr == -1) {
        if ((hostp = gethostbyname(str))) {
            bcopy(hostp->h_addr, in_addr, hostp->h_length);
        } else {
            fprintf(stderr, "send_arp: unknown host %s\n", str);
            return -1;
        }
    }
    return 0;
}

/*
 * get_hw_addr - 
 * buf[OUT]: hw_addr
 * str[OUT]: mac addr
 * return: 0 success , -1 failed
 */
static int get_hw_addr(char* buf, char* str)
{
    int i;
    char c, val;
    for (i = 0; i < ETH_HW_ADDR_LEN; i++) {
        if (!(c = tolower(*str++))) {
            printf("Invalid hardware address!\n");
            return -1;
        }
        if (isdigit(c)) {
            val = c - '0';
        } else if (c >= 'a' && c <= 'f') {
            val = c - 'a' + 10;
        } else if (c >= 'A' && c <= 'F') {
            val = c - 'A' + 10;
        } else {
            printf("Invalid hardware address!\n");
            return -1;
        }
        *buf = val << 4;
        if (!(c = tolower(*str++))) {
            printf("Invalid hardware address!\n");
            return -1;
        }
        if (isdigit(c)) {
            val = c - '0';
        } else if (c >= 'a' && c <= 'f') {
            val = c - 'a' + 10;
        } else if (c >= 'A' && c <= 'F') {
            val = c - 'A' + 10;
        } else {
            printf("Invalid hardware address!\n");
            return -1;
        }
        *buf++ |= val;
        if (*str == ':') {
            str++;
        } else {
            if (i < 5) {
                printf("Invalid hardware address 5 , %c!\n", *str);
                return -1;
            }
        }
    }
    if (i < 6) {
        printf("Invalid hardware address 6!\n");
        return -1;
    }

    return 0;
}

/*
 * _get_if_addr - 
 * eth[IN]: device, such as eth0
 * ip[OUT]: ip addr
 * mac[OUT]: mac addr
 * return: 0 success , -1 failed
 */
static int _get_if_addr(char *eth, char * const ip, char * const mac)
{
    int sock;
    //
    struct sockaddr_in sin;
    struct ifreq ifr;

    if(ip) 			ip[0] = 0;
    if(mac) 		mac[0]= 0;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == -1) {
        perror("socket");
        return -1;
    }

    strncpy(ifr.ifr_name, eth, IFNAMSIZ);
    ifr.ifr_name[IFNAMSIZ - 1] = 0;

    if(ip) {
        if (ioctl(sock, SIOCGIFADDR, &ifr) < 0) {
            perror("ioctl");
            close(sock);
            return -1;
        }
        memcpy(&sin, &ifr.ifr_addr, sizeof(sin));
        strcpy(ip, inet_ntoa(sin.sin_addr));
    }

    if(mac) {
        if (ioctl(sock, SIOCGIFHWADDR, &ifr) < 0) {
            perror("ioctl");
            close(sock);
            return -1;
        }
        sprintf(mac, "%02X:%02X:%02X:%02X:%02X:%02X", ifr.ifr_hwaddr.sa_data[0] & 0xff,ifr.ifr_hwaddr.sa_data[1] & 0xff,ifr.ifr_hwaddr.sa_data[2] & 0xff,
                ifr.ifr_hwaddr.sa_data[3] & 0xff,ifr.ifr_hwaddr.sa_data[4] & 0xff,ifr.ifr_hwaddr.sa_data[5] & 0xff);
    }

    close(sock);
    return 0;
}


/*
 * arp_send_packet2 - 
 * type[IN]: request type
 * dst_mac[IN]: dst mac addr
 * src_mac[IN]: src mac addr
 * ip[IN]: ip addr
 * mac[IN]: mac addr
 * query_ip[IN]: ip addr
 * query_mac[OUT]: mac addr
 * out_size[IN]: out size
 * timeout[IN]: timeout, unit is second
 * return: number of mac query, failed return -1
 */
static int arp_send_packet2(int type, char *dst_mac, char *src_mac, char *ip, char *mac, char *query_ip,
                     char (*query_mac)[18], int out_size, int timeout)
{
    struct in_addr src_in_addr, targ_in_addr;
    struct arp_packet pkt, *arp = NULL;
    char buf[10*1000];
    socklen_t addrlen;
    int query_index = 0;
    fd_set rset;
    struct timeval timeo;

    int ret, sock = 0;
    struct sockaddr_ll addr_ii, sa, da;
    struct ifreq ifr;
    int protocal = ETH_P_ARP;
    //int protocal = ETH_P_ALL;
    int eth_index = 0;

    if (type == OP_ARP_REQUEST) {
        protocal = ETH_P_ARP;
    } else if (type == OP_RARP_REQUEST) {
        protocal = ETH_P_RARP;
    } else {
        return -1;
    }

    sock = socket(PF_PACKET, SOCK_RAW, htons(protocal));
    if (sock < 0) {
        perror("socket");
        return -1;
    }

    memset(&ifr, 0, sizeof(ifr));
    strcpy(ifr.ifr_name, DEFAULT_DEVICE);
    if (ioctl(sock, SIOCGIFINDEX, &ifr) < 0) {
        close(sock);
        return -1;
    }
    eth_index = ifr.ifr_ifindex;
    /*
    	memset(&ifr, 0, sizeof(ifr));
    	strcpy(ifr.ifr_name, DEFAULT_DEVICE);
    	if (ioctl(sock, SIOCGIFHWADDR, &ifr) < 0) {
    		close(sock);
    		return -1;
    	}
    	memcpy (src_mac, ifr.ifr_hwaddr.sa_data, 6 * sizeof (uint8_t));
    */
    memset( &addr_ii, 0, sizeof(addr_ii) );
    addr_ii.sll_family = AF_PACKET;
    addr_ii.sll_ifindex = eth_index;
    addr_ii.sll_protocol = htons(protocal);
    addr_ii.sll_hatype = 0;//ARPHRD_ETHER;
    addr_ii.sll_pkttype = PACKET_HOST;
    ARP_CHECK_RET_GOTO(get_hw_addr((char *)addr_ii.sll_addr, src_mac));
    //memcpy (addr_ii.sll_addr, src_mac, ETH_HW_ADDR_LEN * sizeof (uint8_t));
    addr_ii.sll_halen =  ETH_HW_ADDR_LEN;
    if(bind(sock, (struct sockaddr *) &addr_ii, (socklen_t)sizeof(addr_ii)) == -1 ) {
        perror("bind()");
        close(sock);
        return -1;
    }

    //printf("mac local:%s  query mac:%s ip:%s\n", src_mac, query_mac[0], query_ip);
    pkt.frame_type = htons(ARP_FRAME_TYPE);
    pkt.hw_type = htons(ETHER_HW_TYPE);
    pkt.prot_type = htons(IP_PROTO_TYPE);
    pkt.hw_addr_size = ETH_HW_ADDR_LEN;
    pkt.prot_addr_size = IPV4_ADDR_LEN;
    pkt.op = htons(type);
    ARP_CHECK_RET_GOTO(get_hw_addr((char *)pkt.targ_hw_addr, dst_mac));
    ARP_CHECK_RET_GOTO(get_hw_addr((char *)pkt.rcpt_hw_addr, query_mac[0]));
    ARP_CHECK_RET_GOTO(get_hw_addr((char *)pkt.src_hw_addr, src_mac));
    ARP_CHECK_RET_GOTO(get_hw_addr((char *)pkt.sndr_hw_addr, mac));
    ARP_CHECK_RET_GOTO(get_ip_addr(&src_in_addr, ip));
    ARP_CHECK_RET_GOTO(get_ip_addr(&targ_in_addr, query_ip));
    memcpy(pkt.sndr_ip_addr, &src_in_addr, IPV4_ADDR_LEN);
    memcpy(pkt.rcpt_ip_addr, &targ_in_addr, IPV4_ADDR_LEN);
    bzero(pkt.padding, 18);
    //pkt.crc32 = calculate_crc32(0, &pkt, sizeof(struct arp_packet) - 4);

    //printf("sock:%d index:%d , type:%x , srcmac:%s dst_mac: %s\n", sock, eth_index, type, src_mac, dst_mac);

    memset( &sa, 0, sizeof(sa) );
    sa.sll_family = AF_PACKET;
    sa.sll_ifindex = eth_index;
    sa.sll_protocol = htons(protocal);
    sa.sll_hatype = 0;//ARPHRD_ETHER;
    sa.sll_pkttype = PACKET_BROADCAST;
    ARP_CHECK_RET_GOTO(get_hw_addr((char *)sa.sll_addr, dst_mac));
    sa.sll_halen =  ETH_HW_ADDR_LEN;
    if (sendto(sock, &pkt, sizeof(pkt), 0, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
        perror("sendto");
        close(sock);
        return -1;
    }

    gArpTrigger = true;

    timeo.tv_sec = ARP_RECV_TIMEOUT;
    timeo.tv_usec = 0;
    do {
        char peer_ip[20], peer_mac[48];

        FD_ZERO(&rset);
        FD_SET(sock, &rset);

        ret = select(sock + 1, &rset, NULL, NULL, &timeo);
        if (ret < 0) {
            if (errno == EINTR) {
                continue;
            }
            printf("arp recv failed, errno:%d!\n", errno);
            close(sock);
            return -1;
        } else if (ret == 0) {
            //printf("arp recv timeout!\n");
            break;
        } else {
            bzero(&da, sizeof(da));
            addrlen = sizeof(da);
            if (recvfrom(sock, buf, sizeof(buf), 0, (struct sockaddr *)&da, &addrlen) < 0) {
                printf("arp recv failed, errno:%d!\n", errno);
                close(sock);
                return -1;
            } else {
                arp = (struct arp_packet *)buf;
                //printf("got packet type: %d(%x)\n", arp->frame_type, arp->frame_type);
                if (arp->frame_type == ntohs(ARP_FRAME_TYPE) && arp->prot_type ==  ntohs(IP_PROTO_TYPE)) {
                    sprintf(peer_mac, "%02x:%02x:%02x:%02x:%02x:%02x", arp->sndr_hw_addr[0], arp->sndr_hw_addr[1],	 arp->sndr_hw_addr[2],
                            arp->sndr_hw_addr[3],	arp->sndr_hw_addr[4],  arp->sndr_hw_addr[5]);
                    sprintf(peer_ip,"%d.%d.%d.%d", arp->sndr_ip_addr[0], arp->sndr_ip_addr[1], arp->sndr_ip_addr[2], arp->sndr_ip_addr[3] );
                    //printf("got arp packet , frametype:0x%x,  type:%d !!!!!!\n",ntohs(arp->frame_type), ntohs(arp->op));
                    //printf("\tdst mac:%s, dst ip:%s\n", peer_mac, peer_ip);
                    //if (type == OP_ARP_REQUEST && arp->op == htons(OP_ARP_REPLY)) {
                    if (type == OP_ARP_REQUEST) {//FIX ME
                        if (memcmp(pkt.rcpt_ip_addr, arp->sndr_ip_addr, IPV4_ADDR_LEN) == 0) {
                            printf("got arp reply %d %s -> %s: !\n", query_index, query_ip, peer_mac);
                            strcpy(query_mac[query_index++], peer_mac);
                            if (query_index >= out_size)
                                break;
                        }
                    } else if (type == OP_RARP_REQUEST /* && arp->op == htons(OP_ARP_REPLY) */
                               /* && arp->op == htons(OP_RARP_REPLY)*/) {
                        if (memcmp(pkt.rcpt_hw_addr, arp->sndr_hw_addr, ETH_HW_ADDR_LEN) == 0) {
                            //printf("got rarp reply: !\n");
                            strcpy(query_ip, peer_ip);
                            query_index++;
                            break;
                        }
                    }
                }
            }
        }

    } while(gArpTrigger);

_FAILED_EXIT:
    if (sock > 0)
        close(sock);
    return query_index;
}

/*
 * ARP_send_request1 - 
 * query_ip[IN]: ip addr
 * query_mac[OUT]: mac addr
 * max_size[IN]: for example , query_mac[64][18], then max_size is 64
 * timeout[IN]: timeout, unit is second
 * return: number of mac query, failed return -1
 */
int ARP_send_request1(char *query_ip, char (*query_mac)[18], int max_size, int timeout)
{
    char ip[20];
    char mac[48];

    if (timeout > 30) {
        timeout = 30;
    } else if (timeout < 1) {
        timeout = ARP_RECV_TIMEOUT;
    }
	char def_eth[128];
    if(NULL == getenv("DEF_ETH")){
		snprintf(def_eth, sizeof(def_eth), "eth0");
	}else{
		snprintf(def_eth, sizeof(def_eth), "%s", getenv("DEF_ETH"));
	}
    strncpy(sArpDefEther, def_eth, sizeof(sArpDefEther));

    if (_get_if_addr(DEFAULT_DEVICE, ip, mac) < 0) return -1;

	strcpy(ip, "0.0.0.0");
    strncpy(query_mac[0], EMPTY_HW_ADDR, 18);
    return arp_send_packet2(OP_ARP_REQUEST, BROADCAST_HW_ADDR, mac, ip, mac, query_ip, query_mac, max_size, timeout);
}

#ifdef ARP_TEST
int main(int argc, char *argv[])
{
    setenv("DEF_ETH", "eth0", 1);

    if (argc != 3)
        return -1;
    if (strcmp(argv[1], "-a")  == 0) {
        char query_mac[128][18];
        ARP_send_request(argv[2], query_mac, 128, 3);
    } else if (strcmp(argv[1], "-a2")  == 0) {
        char _mac[18];
        ARP_query(argv[2], _mac);
    } else if (strcmp(argv[1], "-a3")  == 0) {
        ARP_send_request3(argv[2], NULL);
    } else {
        char szip[32];
        RARP_send_request(szip, argv[2]);
    }
    return 0;
}
#endif

#endif//HAVE_ARP
