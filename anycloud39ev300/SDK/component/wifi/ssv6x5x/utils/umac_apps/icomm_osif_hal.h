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

#include "list.h"
#include <linux/genetlink.h>

#define REG32_R(_REG_) (read_register(_REG_))
#define REG32_W(_REG_, _VAL_) (write_register(_REG_, _VAL_))


/* IPC operation */
#define ICOMM_OPS_READREG		1
#define ICOMM_OPS_WRITEREG		2
#define ICOMM_OPS_TXFRAME		3
#define ICOMM_OPS_RXFRAME		4
#define ICOMM_OPS_CONFIG		5
#define ICOMM_OPS_HCI_START		6
#define ICOMM_OPS_TEST			7

#define     MAX_PAYLOAD     32767
#define     PB_OFFSET       80
#define     ICOMM_IPC_TX_SOCK  "/tmp/icomm_ipc_tx_sock"
#define     ICOMM_IPC_RX_SOCK  "/tmp/icomm_ipc_rx_sock"

typedef struct ssv_data_node ssv_data_node;


/* SSV netlink operation */
enum {
	SSV_WIRELESS_CMD_UNSPEC,
	SSV_WIRELESS_CMD_CONFIG,
	SSV_WIRELESS_CMD_READ_REG,
	SSV_WIRELESS_CMD_WRITE_REG,
	SSV_WIRELESS_CMD_TXFRAME,
	SSV_WIRELESS_CMD_RXFRAME,
	SSV_WIRELESS_CMD_START_HCI,
	SSV_WIRELESS_CMD_TEST,
	__SSV_WIRELESS_CMD_MAX,
};
#define SSV_WIRELESS_CMD_MAX (__SSV_WIRELESS_CMD_MAX - 1)

/* SSV netlink attribute */
enum {
	SSV_WIRELESS_ATTR_UNSPEC,
	SSV_WIRELESS_ATTR_CONFIG,
	SSV_WIRELESS_ATTR_REGISTER,
	SSV_WIRELESS_ATTR_TXFRAME,
	SSV_WIRELESS_ATTR_RXFRAME,
	SSV_WIRELESS_ATTR_START_HCI,
	SSV_WIRELESS_ATTR_TEST,
	__SSV_WIRELESS_ATTR_MAX,
};
#define SSV_WIRELESS_ATTR_MAX (__SSV_WIRELESS_ATTR_MAX - 1)

struct ssv_ipc_hdr {
	unsigned short	type;
	unsigned short	len;
};

struct ssv_wireless_register {
	unsigned int address;
	unsigned int value;
};

struct ssv_netlink_family {
	struct nlmsghdr n;
	struct genlmsghdr g;
	char buf[255];
};

struct ssv_netlink_msg {
	struct nlmsghdr n;
	struct genlmsghdr g;
	char buf[32767];
};

struct ssv_tx_frame_node {
    int index;
    int size;
    int finish;
    unsigned char *data;
    struct list_head list;
};

struct icomm_osif_hal {
    int nl_fd;
    int nl_id;
    int ipc_tx_fd;
    int ipc_rx_fd;
    struct sockaddr_nl nl_saddr, nl_daddr;
    struct sockaddr_un      domain_tx_addr;
    struct sockaddr_un      domain_rx_addr;
    /* HW resource */
    int buffer_size;
    int queue_size;
    // Fail counter
    int fail_cnt;
    // tx frame list
    struct list_head tx_frame;
    int rx_pkt_cnt;
    int tx_pkt_cnt;
    int finish_test;
};

typedef struct ssv_cli_cmd {
    const char        *cmd;
    void (*cmd_handle) (void *, int, char **);
    const char        *cmd_usage;
} ssv_cli_cmd_st;


/* Generic macros for dealing with netlink sockets. */
#define GENLMSG_DATA(glh) ((void *)(NLMSG_DATA(glh) + GENL_HDRLEN))
#define GENLMSG_PAYLOAD(glh) (NLMSG_PAYLOAD(glh, 0) - GENL_HDRLEN)
#define NLA_DATA(na) ((void *)((char*)(na) + NLA_HDRLEN))
//#define NLA_PAYLOAD(len) (len - NLA_HDRLEN)
