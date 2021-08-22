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

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "icomm_osif_hal.h"

extern unsigned int  icomm_send_receive_cmd(struct ssv_ipc_hdr hdr ,unsigned char *data, unsigned int len);


int status_wait = 0 ;
unsigned int *local_buff= 0 ;

int init_netlink_socket(void)
{
	int fd;
	struct sockaddr_nl local;

	fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_GENERIC);
	if (fd < 0)
	{
		printf("fail to create netlink socket\n");
		return -1;
	}
	
	memset(&local, 0, sizeof(local));
	local.nl_family = AF_NETLINK;
	local.nl_groups = 0;
	if (bind(fd, (struct sockaddr *) &local, sizeof(local)) < 0)
		goto error;
	
	return fd;

error:
	close(fd);
	return -1;
}

int init_rx_sock(void)
{
    struct sockaddr_un addr;
    int fd;

    if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        printf("socket error");
        return -1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    sprintf(addr.sun_path, "%s", ICOMM_IPC_RX_SOCK);

    if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        printf("connect error");
        return -1;
    }

    return fd;
}

/* send message to kernel */
int send_msg2kernel(int s, const char *buf, int bufLen)
{
	struct sockaddr_nl nladdr;
	int r;

	memset(&nladdr, 0, sizeof(nladdr));
	nladdr.nl_family = AF_NETLINK;

	while ((r = sendto(s, buf, bufLen, 0, (struct sockaddr *)&nladdr,
						sizeof(nladdr))) < bufLen)
	{
		if (r > 0)
		{
			buf += r;
			bufLen -= r;
		}
		else if (errno != EAGAIN)
			return -1;
	}

	return 0;
}

int get_netlink_family_id(int sock)
{
	struct ssv_netlink_family family_req, ans;
	int id;
	struct nlattr *na;
	int rep_len;

	/* Get family name */
	family_req.n.nlmsg_type = GENL_ID_CTRL;
	family_req.n.nlmsg_flags = NLM_F_REQUEST;
	family_req.n.nlmsg_seq = 0;
	family_req.n.nlmsg_pid = getpid();
	family_req.n.nlmsg_len = NLMSG_LENGTH(GENL_HDRLEN);
	family_req.g.cmd = CTRL_CMD_GETFAMILY;
	family_req.g.version = 0x1;

	na = (struct nlattr *) GENLMSG_DATA(&family_req);
	na->nla_type = CTRL_ATTR_FAMILY_NAME;
	na->nla_len = strlen("SSV_WIRELESS") + 1 + NLA_HDRLEN;
	strcpy(NLA_DATA(na), "SSV_WIRELESS");
	family_req.n.nlmsg_len += NLMSG_ALIGN(na->nla_len);

	if (send_msg2kernel(sock, (char *) &family_req, family_req.n.nlmsg_len) < 0)
		return -1;

	rep_len = recv(sock, &ans, sizeof(ans), 0);
	if (rep_len < 0)
	{
		printf("fail to receive family id\n");
		return -1;
	}

	/* Validate response message */
	if (!NLMSG_OK((&ans.n), rep_len))
	{
		printf("Receive invalid family id message\n");
		return -1;
	}

	if (ans.n.nlmsg_type == NLMSG_ERROR)
	{
		printf("Receive family id error\n");
		return -1;
	}

	na = (struct nlattr *) GENLMSG_DATA(&ans);
	na = (struct nlattr *) ((char *) na + NLA_ALIGN(na->nla_len));
	if (na->nla_type == CTRL_ATTR_FAMILY_ID)
	{
		id = *(__u16 *) NLA_DATA(na);
	}

	return id;
}

int init_domain_socket(void)
{
	int sock;

	sock = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sock < 0)
	{
		printf("fail to create domain socket\n");
		return -1;
	}

	return sock;
}

int init_icomm_osif_hal(struct icomm_osif_hal *hal_sc)
{
	memset(hal_sc, 0, sizeof(struct icomm_osif_hal));

	/* init netlink socket */
	hal_sc->nl_fd = init_netlink_socket();
	if (hal_sc->nl_fd < 0)
		goto err;

    hal_sc->nl_id = get_netlink_family_id(hal_sc->nl_fd);
	if (hal_sc->nl_id < 0)
		goto err;

	/* init domain socket */
	hal_sc->ipc_tx_fd = init_domain_socket();
	if (hal_sc->ipc_tx_fd < 0)
		goto err;

	/* bind domain socket */
	unlink(ICOMM_IPC_TX_SOCK);
	hal_sc->domain_tx_addr.sun_family = AF_UNIX;
	sprintf(hal_sc->domain_tx_addr.sun_path, "%s", ICOMM_IPC_TX_SOCK);
	if (bind(hal_sc->ipc_tx_fd, (struct sockaddr*)&(hal_sc->domain_tx_addr), sizeof(hal_sc->domain_tx_addr)) == -1)
	{
		printf("fail to bind domain tx socket\n");
		goto err;
	}

	if (listen(hal_sc->ipc_tx_fd, 20) == -1)
	{
		printf("fail to listen domain tx socket\n");
		goto err;
	}



	return 0;
err:
	if (hal_sc->nl_fd > 0)
		close(hal_sc->nl_fd);
	if (hal_sc->ipc_tx_fd > 0)
		close(hal_sc->ipc_tx_fd);

	return -1;
}

void icomm_send_netlink_cmd(struct icomm_osif_hal *hal_sc, void *buf, int len)
{
	struct ssv_netlink_msg msg;
	struct nlattr *na;
	struct ssv_ipc_hdr *hdr = (struct ssv_ipc_hdr *)buf;
	int mlength, retval, pid, hci_start;
	struct sockaddr_nl nladdr;

	/* netlink header */
	msg.n.nlmsg_len = NLMSG_LENGTH(GENL_HDRLEN);
	msg.n.nlmsg_type = hal_sc->nl_id;;
	msg.n.nlmsg_flags = NLM_F_REQUEST;
	msg.n.nlmsg_seq = 0;
	msg.n.nlmsg_pid = getpid();

	switch (hdr->type)
	{
		case ICOMM_OPS_CONFIG:
				//printf("CONFIG OPS\n");
				/* compose message */
				msg.g.cmd = SSV_WIRELESS_CMD_CONFIG;
				na = (struct nlattr *) GENLMSG_DATA(&msg);
				na->nla_type = SSV_WIRELESS_ATTR_CONFIG;
				mlength = sizeof(unsigned int);
				na->nla_len = mlength+NLA_HDRLEN;
				pid = (int)getpid();
				memcpy(NLA_DATA(na), &pid, mlength);
				msg.n.nlmsg_len += NLMSG_ALIGN(na->nla_len);
			break;
		case ICOMM_OPS_HCI_START:
				//printf("TRIGER IRQ OPS\n");
				/* compose message */
				msg.g.cmd = SSV_WIRELESS_CMD_START_HCI;
				na = (struct nlattr *) GENLMSG_DATA(&msg);
				na->nla_type = SSV_WIRELESS_ATTR_START_HCI;
				mlength = sizeof(unsigned int);
				na->nla_len = mlength+NLA_HDRLEN;
				hci_start = 1;
				memcpy(NLA_DATA(na), &hci_start, mlength);
				msg.n.nlmsg_len += NLMSG_ALIGN(na->nla_len);
			break;
		case ICOMM_OPS_READREG:
				//printf("READREG OPS\n");
				/* compose message */
				msg.g.cmd = SSV_WIRELESS_CMD_READ_REG;
				na = (struct nlattr *) GENLMSG_DATA(&msg);
				na->nla_type = SSV_WIRELESS_ATTR_REGISTER;
				mlength = hdr->len;
				na->nla_len = mlength+NLA_HDRLEN;
				memcpy(NLA_DATA(na), buf+sizeof(struct ssv_ipc_hdr), mlength);
				msg.n.nlmsg_len += NLMSG_ALIGN(na->nla_len);
			break;
		case ICOMM_OPS_WRITEREG:
				//printf("WRITEREG OPS\n");
				msg.g.cmd = SSV_WIRELESS_CMD_WRITE_REG;
				na = (struct nlattr *) GENLMSG_DATA(&msg);
				na->nla_type = SSV_WIRELESS_ATTR_REGISTER;
				mlength = hdr->len;
				na->nla_len = mlength+NLA_HDRLEN;
				memcpy(NLA_DATA(na), buf+sizeof(struct ssv_ipc_hdr), mlength);
				msg.n.nlmsg_len += NLMSG_ALIGN(na->nla_len);
			break;
		case ICOMM_OPS_TXFRAME:
				//printf("TXFRAME OPS\n");
				msg.g.cmd = SSV_WIRELESS_CMD_TXFRAME;
				na = (struct nlattr *) GENLMSG_DATA(&msg);
				na->nla_type = SSV_WIRELESS_ATTR_TXFRAME;
				mlength = hdr->len;
				na->nla_len = mlength+NLA_HDRLEN;
				memcpy(NLA_DATA(na), buf+sizeof(struct ssv_ipc_hdr), mlength);
				msg.n.nlmsg_len += NLMSG_ALIGN(na->nla_len);
			break;
        case ICOMM_OPS_TEST:
                //printf("TEST OPS\n");
                /* compose message */
                msg.g.cmd = SSV_WIRELESS_CMD_TEST;
                na = (struct nlattr *) GENLMSG_DATA(&msg);
                na->nla_type = SSV_WIRELESS_ATTR_TEST;
                mlength = hdr->len;
                na->nla_len = mlength+NLA_HDRLEN;
                memcpy(NLA_DATA(na), buf+sizeof(struct ssv_ipc_hdr), mlength);
                msg.n.nlmsg_len += NLMSG_ALIGN(na->nla_len);
            break;
		default:
			printf("Unknown operation\n");
			return;
	}

	/* send message to kernel */
	memset(&nladdr, 0, sizeof(nladdr));
	nladdr.nl_family = AF_NETLINK;

	retval = sendto(hal_sc->nl_fd, (char *)&msg, msg.n.nlmsg_len, 0, 
					(struct sockaddr *) &nladdr, sizeof(nladdr));
	if (retval < 0)
		printf("Fail to send message to kernel\n");
}

void icomm_send_wireless_cmd(struct icomm_osif_hal *hal_sc)
{
	int cl, len;
	unsigned char buf[MAX_PAYLOAD];
	struct ssv_ipc_hdr hdr;
    unsigned char *buf_temp;

	/* Send Config Message to kernel */
	hdr.type = ICOMM_OPS_CONFIG;
	hdr.len = 0;
	icomm_send_netlink_cmd(hal_sc, &hdr, sizeof(hdr));
	
    /* Send "HCI_START" Message to kernel */
	hdr.type = ICOMM_OPS_HCI_START;
	hdr.len = 0;
	icomm_send_netlink_cmd(hal_sc, &hdr, sizeof(hdr));
	
    while (1)
	{
		if ((cl = accept(hal_sc->ipc_tx_fd, NULL, NULL)) == -1)
		{
			printf("fail to accept domain socket\n");
			break;
		}

		bzero(buf, sizeof(buf));
		buf_temp = buf;
		while ((len=read(cl, buf_temp, sizeof(buf))) != 0){
            if (len == -1)
            {
                printf("fail to read\n");
                close(cl);
            }
            /* test loop back 
			hdr.type = ICOMM_OPS_RXFRAME;
			hdr.len = (len-4);
			icomm_send_receive_cmd(hdr, (buf+4), (len-4));
             */
            buf_temp = buf_temp + len;
        }
        icomm_send_netlink_cmd(hal_sc, buf, len);
        close(cl);

	}
}

unsigned int icomm_send_receive_cmd(struct ssv_ipc_hdr hdr ,unsigned char *data, unsigned int len)
{
        unsigned int fd, rc;
        unsigned char buf[MAX_PAYLOAD];
        if ((fd = init_rx_sock()) < 0)
                return -1;
        //printf("DATA SEND \n");

        memcpy(buf, &hdr, sizeof(hdr));
        memcpy(buf+sizeof(hdr), data, len);
       // hexdump(buf,(len+4));

        if ((rc = write(fd, buf, (sizeof(hdr)+len))) < 0)
                printf("Fail to rx_frame\n");

        close(fd);
        return 0;
}


void hexdump(unsigned char *buf, int len)
{
	int i;

	printf("\n-----------------------------\n");
	printf("hexdump(len=%d):\n", len);
	for (i = 0; i < len; i++)
	{
		printf(" %02x", buf[i]);
		if ((i+1)%40 == 0)
			printf("\n");
	}
	printf("\n-----------------------------\n");
}

void *icomm_receive_wireless_event(void *argv)
{
	struct icomm_osif_hal *hal_sc = (struct icomm_osif_hal *)argv;
	int len, rx_frame_len;
	struct ssv_netlink_msg msg;
	struct nlattr *na;
	struct ssv_wireless_register *reg,reg_size;
    struct ssv_ipc_hdr hdr;
	unsigned char *rx_frame;

	while (1)
	{
		len = recv(hal_sc->nl_fd, &msg, sizeof(msg), 0);
		if (len > 0)
		{
			/* Validate response message */
			if (msg.n.nlmsg_type == NLMSG_ERROR)
			{
				printf("Error, receive NACK\n");
				continue;
			}
			if (!NLMSG_OK((&msg.n), len))
			{
				 printf("Invalid reply message received via Netlink\n");
				 continue;
			}

			/* parse reply message */
			switch (msg.g.cmd)
			{
				case SSV_WIRELESS_CMD_READ_REG:
						//printf("Receive READ REGISTER\n");
						na = (struct nlattr *) GENLMSG_DATA(&msg);
						if (na->nla_type != SSV_WIRELESS_ATTR_REGISTER)
							break;
						reg = (struct ssv_wireless_register *)NLA_DATA(na);
						hdr.type = ICOMM_OPS_READREG;
						hdr.len = sizeof(reg_size);
						icomm_send_receive_cmd(hdr, (unsigned char *)reg, sizeof(reg_size));
						//printf("address=0x%08x, value=0x%08x\n", reg->address, reg->value);
						break;
				case SSV_WIRELESS_CMD_WRITE_REG:
						//printf("Receive WRITE REGISTER\n");
						na = (struct nlattr *) GENLMSG_DATA(&msg);
						if (na->nla_type != SSV_WIRELESS_ATTR_REGISTER)
							break;
						reg = (struct ssv_wireless_register *)NLA_DATA(na);
						//printf("address=0x%08x, value=0x%08x\n", reg->address, reg->value);
					break;
				case SSV_WIRELESS_CMD_RXFRAME:
						//printf("Receive RX FRAME\n");
						na = (struct nlattr *) GENLMSG_DATA(&msg);
						if (na->nla_type != SSV_WIRELESS_ATTR_RXFRAME)
							break;
						rx_frame = (unsigned char *)NLA_DATA(na);
						rx_frame_len = na->nla_len - NLA_HDRLEN;
						hdr.type = ICOMM_OPS_RXFRAME;
						hdr.len = rx_frame_len;
						icomm_send_receive_cmd(hdr, rx_frame, rx_frame_len);
					break;
				default:
					printf("Receive unknown command[%d]\n", msg.g.cmd);
					break;
			}
		}
	}
}

int main(int argc, char *argv[])
{
	struct icomm_osif_hal icomm_hal_sc;
        local_buff = malloc(4096);
	pthread_t tid;
	
	init_icomm_osif_hal(&icomm_hal_sc);
	
	pthread_create(&tid, NULL, (void*)icomm_receive_wireless_event, (void*)&icomm_hal_sc);
	icomm_send_wireless_cmd(&icomm_hal_sc);
	
        pthread_exit((void*)icomm_receive_wireless_event);
        free(local_buff);
	close(icomm_hal_sc.nl_fd);
	close(icomm_hal_sc.ipc_tx_fd);
	return 0;
}
