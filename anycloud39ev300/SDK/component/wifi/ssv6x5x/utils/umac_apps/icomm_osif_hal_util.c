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

#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <pthread.h>
#include <time.h>

#include "ssv6006_reg.h"
#include "ssv6006_aux.h"

#include "list.h"
#include "icomm_osif_hal.h"
#include "ssv6200_common.h"
#include "pkt_m2data.h"
#include "hdr_80211.h"

unsigned char rate_tbl[] = {
    0x00,0x01,0x02,0x03,                        // B mode long preamble [0~3]
    0x11,0x12,0x13,                             // B mode short preamble  [4~6]
    0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,    // G mode [7~14]
    0xc0,0xc1,0xc2,0xc3,0xc4,0xc5,0xc6,0xc7,    // N mode HT20 long GI mixed format [15~22]
    0xd0,0xd1,0xd2,0xd3,0xd4,0xd5,0xd6,0xd7,    // N mode HT20 short GI mixed format  [23~30]
    0xe0,0xe1,0xe2,0xe3,0xe4,0xe5,0xe6,0xe7,    // N mode HT40 long GI mixed format [31~38]
    0xf0,0xf1,0xf2,0xf3,0xf4,0xf5,0xf6,0xf7,    // N mode HT40 short GI mixed format  [39~46]
};
#define SIZE_RATE_TBL   (sizeof(rate_tbl) / sizeof((rate_tbl)[0]))

static void hexdump(unsigned char *buf, int len)
{
    int i;

    printf("\n-----------------------------\n");
    printf("hexdump(len=%d):\n", len);
    for (i = 0; i < len; i++)
    {
        printf(" %02x", buf[i]);
        if ((i+1)%16 == 0)
            printf("\n");
    }
    printf("\n-----------------------------\n");
}

static int init_tx_sock(void)
{
    struct sockaddr_un addr;
    int fd;

    if ( (fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
    {
        printf("socket error");
        return -1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    sprintf(addr.sun_path, "%s", ICOMM_IPC_TX_SOCK);

    if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1)
    {
        printf("connect error");
        return -1;
    }

    return fd;
}

static int init_domain_socket(void)
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

static int read_register(unsigned int addr, unsigned int *value, struct icomm_osif_hal *hal_sc)
{
    int fd, rc, len;
    struct ssv_ipc_hdr hdr;
    struct ssv_wireless_register reg;
    unsigned char buf[sizeof(hdr)+sizeof(reg)];

    if ((fd = init_tx_sock()) < 0)
        return -1;

    memset(&hdr, 0, sizeof(hdr));
    memset(&reg, 0, sizeof(reg));
    hdr.type = ICOMM_OPS_READREG;
    hdr.len = sizeof(reg);
    reg.address = addr;
    memcpy(buf, &hdr, sizeof(hdr));
    memcpy(buf+sizeof(hdr), &reg, sizeof(reg));
    if ((rc = write(fd, buf, sizeof(buf))) < 0)
        printf("Fail to read register\n");

    close(fd);

    while (1) {

        if ((fd = accept(hal_sc->ipc_rx_fd, NULL, NULL)) == -1) {
            printf("fail to accept domain socket\n");
            continue;
        }

        bzero(buf, (sizeof(hdr)+sizeof(reg)));
        if ((len=read(fd, buf, (sizeof(hdr)+sizeof(reg)))) > 0) {
            if (len == -1) {
                printf("fail to read\n");
                close(fd);
                return -1;
            } 

            memcpy(&hdr, buf, sizeof(hdr));
            if(hdr.type != ICOMM_OPS_READREG){
                printf("register read error data !!\n");
                close(fd);
                return -1;
            }
            memcpy(&reg, (buf + sizeof(hdr)), sizeof(reg));
            close(fd);
        //    printf("register read address=0x%08x, value=0x%08x\n", reg.address, reg.value);
            *value = reg.value ;	
            break;
        }
    };

    return 0;
}

static int write_register(unsigned int addr, unsigned int value, struct icomm_osif_hal *hal_sc)
{
    int fd, rc;
    struct ssv_ipc_hdr hdr;
    struct ssv_wireless_register reg;
    char buf[sizeof(hdr)+sizeof(reg)];

    if ((fd = init_tx_sock()) < 0)
        return -1;

    memset(&hdr, 0, sizeof(hdr));
    memset(&reg, 0, sizeof(reg));
    hdr.type = ICOMM_OPS_WRITEREG;
    hdr.len = sizeof(reg);
    reg.address = addr;
    reg.value = value;
    memcpy(buf, &hdr, sizeof(hdr));
    memcpy(buf+sizeof(hdr), &reg, sizeof(reg));
    if ((rc = write(fd, buf, sizeof(buf))) < 0)
        printf("Fail to write register\n");

    close(fd);
    return 0;
}

static int set_register_bits(unsigned int addr, unsigned int value, unsigned int clr, struct icomm_osif_hal *hal_sc)
{
    unsigned int regval;
    
    read_register(addr, &regval, hal_sc);
    regval &= ~(clr);
    regval |= (value);

    return write_register(addr, regval, hal_sc);
}

static int sdio_dma_test(unsigned char *data, unsigned int len, struct icomm_osif_hal *hal_sc)
{
    int fd, rc;
    struct ssv_ipc_hdr hdr;
    unsigned char buf[MAX_PAYLOAD];

    if ((fd = init_tx_sock()) < 0)
        return -1;

    memset(&hdr, 0, sizeof(hdr));
    hdr.type = ICOMM_OPS_TEST;
    hdr.len = len;
    memcpy(buf, &hdr, sizeof(hdr));
    memcpy(buf+sizeof(hdr), data, len);

    if ((rc = write(fd, buf, (sizeof(hdr)+len))) < 0)
        printf("Fail sdio dma test \n");

    close(fd);
    return 0;

}

static void hci_agg_init(unsigned int on_off ,struct icomm_osif_hal *hal_sc)
{
    unsigned int i, regval;

    set_register_bits(ADR_HCI_TRX_MODE, 0x0, 0x2, hal_sc);
    do {
        read_register(ADR_RX_PACKET_LENGTH_STATUS, &regval, hal_sc);
        regval &= 0xffff;
        i++;
        if (i > 10000) {
            printf("CANNOT ENABLE HCI RX AGGREGATION!!!\n");
        }
    } while (regval != 0);
    
    if (on_off) {
        set_register_bits(ADR_HCI_TRX_MODE, 0x2, 0x2, hal_sc);
        set_register_bits(ADR_HCI_FORCE_PRE_BULK_IN, 0xa000, 0xffff, hal_sc);
        set_register_bits(ADR_HCI_TRX_MODE, 0x40000000, 0x40000000, hal_sc);
    } else {
        set_register_bits(ADR_HCI_TRX_MODE, 0x2, 0x2, hal_sc);
        set_register_bits(ADR_HCI_FORCE_PRE_BULK_IN, 0x0000, 0xffff, hal_sc);
        set_register_bits(ADR_HCI_TRX_MODE, 0x0, 0x40000000, hal_sc);
    }
}

static void tx_frame(unsigned char *data, unsigned int len,struct icomm_osif_hal *hal_sc)
{
    unsigned int fd, rc;
    struct ssv_ipc_hdr hdr;
    unsigned char buf[MAX_PAYLOAD];
    if ((fd = init_tx_sock()) < 0)
        return;
    
    memset(&hdr, 0, sizeof(hdr));
    hdr.type = ICOMM_OPS_TXFRAME;
    hdr.len = len;
    memcpy(buf, &hdr, sizeof(hdr));
    memcpy(buf+sizeof(hdr), data, len);
    if ((rc = write(fd, buf, (sizeof(hdr)+len))) < 0)
        printf("Fail to tx_frame\n");

    close(fd);
}


static unsigned char *rx_frame(struct icomm_osif_hal *hal_sc)
{
    unsigned int fd, len ;
    struct ssv_ipc_hdr hdr;
    unsigned char *buf, *buf_temp;

    buf = malloc(MAX_PAYLOAD);

    while(1){
        if ((fd = accept(hal_sc->ipc_rx_fd, NULL, NULL)) == -1)
        {
            printf("fail to accept domain socket\n");
            continue;
        }
        
        buf_temp = buf;
        while ((len=read(fd, buf_temp, MAX_PAYLOAD)) != 0) {
            if (len == -1) {
                printf("fail to read\n");
                close(fd);
                return 0;
            }
            buf_temp = buf_temp + len;
        }
            
        memcpy(&hdr, buf, sizeof(hdr));
        if (hdr.type != ICOMM_OPS_RXFRAME) {
            printf("register read error data!!\n");
            continue;
        }
        
        close(fd);
        return buf;
    };

}

void *sdio_rx_frame(struct icomm_osif_hal *hal_sc)
{
    int i;
    unsigned char *buf, *data;
    ssv6200_tx_desc *tx_desc;
    struct list_head *pos, *q;
    struct ssv_tx_frame_node *entry;

    while (1) {
        buf = rx_frame(hal_sc);
        tx_desc = (ssv6200_tx_desc *)(buf + sizeof(struct ssv_ipc_hdr));
        data = (unsigned char *)(tx_desc);
        
        //remove head
        list_for_each_safe(pos, q, &hal_sc->tx_frame) {
            entry = list_entry(pos, struct ssv_tx_frame_node, list);
            list_del(pos);
            break;
        }

        //compare data
        for (i = 0; i < tx_desc->len; i++) {
            if (data[i] != entry->data[i]) {
                hal_sc->fail_cnt++;
                printf("data compare Fail!!!!\n");
                printf("rx frame\n");
                hexdump(data, tx_desc->len);
                printf("tx frame\n");
                hexdump(entry->data, tx_desc->len);
                break;
            }
        }

        hal_sc->buffer_size -= (tx_desc->len/256) + ((tx_desc->len%256) ? 1 : 0);
        hal_sc->queue_size--;
       
        free(entry->data);
        free(entry);
        
        hal_sc->rx_pkt_cnt++;

        if (hal_sc->rx_pkt_cnt == hal_sc->tx_pkt_cnt) {
            hal_sc->finish_test = 1;
            break;
        }
    }

    return NULL;
}

void *sdio_rx_frame_rx_agg(struct icomm_osif_hal *hal_sc)
{
    struct ssv_ipc_hdr hdr;
    int i, offset;
    unsigned char *buf, *data, *aggr_data;
    struct ssv_hci_agg_hdr *agg_hdr;
    ssv6200_tx_desc *tx_desc;
    struct list_head *pos, *q;
    struct ssv_tx_frame_node *entry;
    int buffer_size = 0;
    int queue_size = 0;

    while (1) {
        buf = rx_frame(hal_sc);
        memcpy(&hdr, buf, sizeof(hdr));
       
        aggr_data = (unsigned char *)(buf + sizeof(struct ssv_ipc_hdr));
        offset = 0;
        buffer_size = 0;
        queue_size = 0;
       
        //printf("hdr.len = %d\n", hdr.len);
        //hexdump(aggr_data, hdr.len);

        while (offset < hdr.len) {
            agg_hdr = (struct ssv_hci_agg_hdr *)(aggr_data);
            //end of aggr frame
            if (((agg_hdr->jmp_mpdu_len == 0) && (agg_hdr->accu_rx_len == 0)) ||
                ((agg_hdr->jmp_mpdu_len + offset) > hdr.len))
                break;
            
            data = (unsigned char *)(aggr_data + SIZE_HCI_AGGR_HDR); 
            tx_desc = (ssv6200_tx_desc *)(data);
            
            //remove head
            list_for_each_safe(pos, q, &hal_sc->tx_frame) {
                entry = list_entry(pos, struct ssv_tx_frame_node, list);
                list_del(pos);
                break;
            }
        
            //compare data
            for (i = 0; i < tx_desc->len; i++) {
                if (data[i] != entry->data[i]) {
                    hal_sc->fail_cnt++;
                    printf("data compare Fail!!!!\n");
                    printf("rx frame\n");
                    hexdump(data, tx_desc->len);
                    printf("tx frame\n");
                    hexdump(entry->data, tx_desc->len);
                    printf("\n\n\n\n");
                    break;
                }
            }
       
            queue_size++;
            buffer_size += tx_desc->len; 
            
            aggr_data += agg_hdr->jmp_mpdu_len;
            offset += agg_hdr->jmp_mpdu_len;
            
            //free tx frame
            free(entry->data);
            free(entry);
            hal_sc->rx_pkt_cnt++;
            if (hal_sc->rx_pkt_cnt == hal_sc->tx_pkt_cnt)
                break;
        }

        hal_sc->buffer_size -= (buffer_size/256) + ((buffer_size%256) ? 1 : 0);
        hal_sc->queue_size -= queue_size;
       
        if (hal_sc->rx_pkt_cnt == hal_sc->tx_pkt_cnt) {
            hal_sc->finish_test = 1;
            break;
        }
    }

    return NULL;
}


void sdio_test(unsigned int count, unsigned int frame_size, struct icomm_osif_hal *hal_sc)
{
    unsigned int index = 0 ,i = 0;
    struct ssv_tx_frame_node *entry; 
    ssv6200_tx_desc *tx_desc;

    for (index = 0; index < count; index++) {
        
        entry = (struct ssv_tx_frame_node *)malloc(sizeof(struct ssv_tx_frame_node));
        if (!entry) {
            printf("%s(): alloc ssv_tx_frame_node fail\n", __FUNCTION__);
            break;
        }

        entry->data = (unsigned char *)malloc(sizeof(unsigned char) * frame_size);
        if (!entry) {
            free(entry);
            printf("%s(): alloc data fail\n", __FUNCTION__);
            break;
        }

        entry->index = index;
        entry->size = frame_size;
        entry->finish = 0;

        //fill data context
        for (i = 0; i < frame_size; i++)
            entry->data[i] = rand();
       
        //fill tx_desc
        tx_desc = (ssv6200_tx_desc *)entry->data;
        tx_desc->len = frame_size;
        tx_desc->c_type = M2_TXREQ;
        memcpy(entry->data + sizeof(ssv6200_tx_desc), &index, sizeof(unsigned int));
        list_add_tail(&(entry->list), &(hal_sc->tx_frame));

        /* HW resource */
        hal_sc->buffer_size += (frame_size/256) + ((frame_size%256) ? 1 : 0);
        hal_sc->queue_size++;
        while((hal_sc->buffer_size > 127) || (hal_sc->queue_size > 10));

        tx_frame((unsigned char *)entry->data, entry->size, hal_sc);
    }

    while (1) {
        if (hal_sc->finish_test == 0)
            sleep(1);
        else 
            break;
    }

}

static int sdio_dma_frame_send(unsigned int count, unsigned int frame_size, struct icomm_osif_hal *hal_sc)
{
    unsigned int index = 0 ,i = 0;
    unsigned int regval;
    unsigned char *data = NULL;
    unsigned char data_compare[4];
    int align_size = ((frame_size + 256) / 256) * 256;
    int fail_cnt = 0;

    //set slow bus 40M, CPU 120M
    write_register(0xccb0b004, 0xa51a8800, hal_sc);
    write_register(0xc0000054, 0x481100cc, hal_sc);

    data = (unsigned char *)malloc(sizeof(unsigned char) * align_size);
    if (!data) {
        printf("Cannot alloc test pattern\n");
        return -1;
    }

    for (index = 0; index < count; index++) {

        //generate test pattern
        for (i = 0; i < align_size; i++)
            data[i] = rand();
        
        //write dma
        sdio_dma_test((unsigned char *)data, align_size, hal_sc);

        //compare data
        for (i = 0; i < frame_size; i += 4) {
            sleep(0.05);
            read_register(i, &regval, hal_sc);
            
            data_compare[0] = (regval >> ( 0 )) & 0xff;
            data_compare[1] = (regval >> ( 8 )) & 0xff;
            data_compare[2] = (regval >> ( 16 )) & 0xff;
            data_compare[3] = (regval >> ( 24 )) & 0xff;
           
            if ((data[i] != data_compare[0]) ||
                (data[i+1] != data_compare[1]) ||
                (data[i+2] != data_compare[2]) ||
                (data[i+3] != data_compare[3])) {
                
                printf("data[%d]=%02x, compare[%d]=%02x\n", i, data[i], 0, data_compare[0]);
                printf("data[%d]=%02x, compare[%d]=%02x\n", i+1, data[i+1], 1, data_compare[1]);
                printf("data[%d]=%02x, compare[%d]=%02x\n", i+2, data[i+1], 2, data_compare[2]);
                printf("data[%d]=%02x, compare[%d]=%02x\n", i+3, data[i+1], 3, data_compare[3]);
                fail_cnt++;
            }
        }
    }

    if (data)
        free(data);

    return fail_cnt;
}

static int firmware_write(char *file_name,struct icomm_osif_hal *hal_sc)
{
    FILE *fw_file = NULL;
    unsigned int file_size ,i=0;
    unsigned int *file_write;
    
    fw_file = fopen(file_name, "r+");
    if (!fw_file) {
        printf("Fail to open file\n");
        return -1;
    }

    fseek(fw_file, 0, SEEK_END);
    file_size = ftell(fw_file);
    fseek(fw_file, 0, SEEK_SET);

    file_write = malloc(file_size);
    while(!feof(fw_file)){
        fread(file_write, file_size, 1, fw_file);
    }

    while(i < file_size){
        write_register(i, file_write[i/4], hal_sc);
        //printf("file_write=%x\n",file_write[i/4]);       
        i+=4;
    };

    free(file_write);
    fclose(fw_file);
    return 0 ;
}

int init_icomm_osif_hal(struct icomm_osif_hal *hal_sc)
{
    memset(hal_sc, 0, sizeof(struct icomm_osif_hal));
    hal_sc->ipc_rx_fd = init_domain_socket();
    
    if(hal_sc->ipc_rx_fd < 0)
        goto err;
    
    /* bind domain socket */
    unlink(ICOMM_IPC_RX_SOCK);
    hal_sc->domain_rx_addr.sun_family = AF_UNIX;
    sprintf(hal_sc->domain_rx_addr.sun_path, "%s", ICOMM_IPC_RX_SOCK);
    if (bind(hal_sc->ipc_rx_fd, (struct sockaddr*)&(hal_sc->domain_rx_addr), sizeof(hal_sc->domain_rx_addr)) == -1) {
        printf("fail to bind domain rx socket\n");
        goto err;
    }

    if (listen(hal_sc->ipc_rx_fd, 20) == -1) {
        printf("fail to listen domain rx socket\n");
        goto err;
    }

    INIT_LIST_HEAD(&hal_sc->tx_frame);
    return 0;

err:
    if (hal_sc->ipc_rx_fd > 0)
        close(hal_sc->ipc_rx_fd);
    
    return -1;
}

static void cmd_readreg(void *sc, int argc, char *argv[])
{
    struct icomm_osif_hal *hal_sc = (struct icomm_osif_hal *)sc;
    unsigned int addr, val;
    int ret = 0;

    if (argc != 3) {
        printf("command error\n");
        return;
    }
    
    sscanf(argv[2], "%x", &addr);
    ret = read_register(addr, &val, hal_sc);
    if (ret < 0)
        printf("Fail to register read[0x%8x]\n", addr);
    else
        printf("register read address=0x%08x, value=0x%08x\n", addr, val);

}

unsigned char *phy_lpbk_rx(struct icomm_osif_hal *hal_sc)
{
    int i, payload_len;
    unsigned char *buf, *data;
    ssv6200_tx_desc *tx_desc;
    struct list_head *pos, *q;
    struct ssv_tx_frame_node *entry;
    
    while (1) {
        buf = rx_frame(hal_sc);
        data = (unsigned char *)(buf + sizeof(struct ssv_ipc_hdr) + PB_OFFSET + COMMON_DATA_HDR_LEN);
        tx_desc = (ssv6200_tx_desc *)(buf + sizeof(struct ssv_ipc_hdr));
        payload_len = tx_desc->len - PB_OFFSET - COMMON_DATA_HDR_LEN;

        //remove head
        list_for_each_safe(pos, q, &hal_sc->tx_frame) {
            entry = list_entry(pos, struct ssv_tx_frame_node, list);
            list_del(pos);
            break;
        }

        //compare data
        for (i = 0; i < payload_len; i++) {
            if (data[i] != entry->data[i+PB_OFFSET+COMMON_DATA_HDR_LEN]) {
                hal_sc->fail_cnt++;
                printf("data compare Fail!!!!\n");
                printf("rx frame\n");
                hexdump(data, payload_len);
                printf("tx frame\n");
                hexdump(entry->data+PB_OFFSET+COMMON_DATA_HDR_LEN, payload_len);
                break;
            }
        }

        hal_sc->buffer_size -= (tx_desc->len/256) + ((tx_desc->len%256) ? 1 : 0);
        hal_sc->queue_size--;
       
        free(entry->data);
        free(entry);
        
        hal_sc->rx_pkt_cnt++;

        if (hal_sc->rx_pkt_cnt == hal_sc->tx_pkt_cnt) {
            hal_sc->finish_test = 1;
            break;
        }
    }

    return NULL;
}



static void cmd_writereg(void *sc, int argc, char *argv[])
{
    struct icomm_osif_hal *hal_sc = (struct icomm_osif_hal *)sc;
    unsigned int addr,value;
   
    if (argc != 4) {
        printf("command error\n");
        return;
    }

    sscanf(argv[2], "%x", &addr);
    sscanf(argv[3], "%x", &value);

    write_register(addr, value, hal_sc);
}

static void cmd_loadfw(void *sc, int argc,char *argv[])
{
    struct icomm_osif_hal *hal_sc = (struct icomm_osif_hal *)sc;
   
    if (argc != 3) {
        printf("command error\n");
        return;
    }
        
    firmware_write(argv[2], hal_sc);
}

static void cmd_pattern_test(void *sc, int argc, char *argv[])
{
    struct icomm_osif_hal *hal_sc = (struct icomm_osif_hal *)sc;
    unsigned char *buf;
    unsigned int regval = 0;
    int i;

    if (argc != 3) {
        printf("command error\n");
        return;
    }
        
    //MCU disable
    set_register_bits(ADR_PLATFORM_CLOCK_ENABLE, (0<<RESET_N_CPUN10_SFT), RESET_N_CPUN10_MSK, hal_sc);
    //PHY disable
    set_register_bits(ADR_WIFI_PHY_COMMON_ENABLE_REG, (0<<RG_PHY_MD_EN_SFT), RG_PHY_MD_EN_MSK, hal_sc);
    /* do MAC software reset first*/
    write_register(ADR_BRG_SW_RST, 1 << MAC_SW_RST_SFT, hal_sc);
    do {
        read_register(ADR_BRG_SW_RST, &regval, hal_sc);
        i ++;
        if (i >10000){
            printf("MAC reset fail !!!!\n");
            return;
        }
    } while (regval != 0);
   
    firmware_write(argv[2], hal_sc);
   
    write_register(ADR_WIFI_PHY_COMMON_SYS_REG, 0x80000000, hal_sc);
    //phy loopback
    set_register_bits(ADR_WIFI_PHY_COMMON_SYS_REG, (1<<RG_PMDLBK_SFT), RG_PMDLBK_MSK, hal_sc);
    //phy enable
    set_register_bits(ADR_WIFI_PHY_COMMON_ENABLE_REG, (1<<RG_PHY_MD_EN_SFT), RG_PHY_MD_EN_MSK, hal_sc);
    //for MP, ILM
    set_register_bits(0xc0000128, (1<<1), 0x00000020, hal_sc);
    //MCU enable
    set_register_bits(ADR_PLATFORM_CLOCK_ENABLE, (1<<RESET_N_CPUN10_SFT), RESET_N_CPUN10_MSK, hal_sc);

    buf = rx_frame(hal_sc); 
    
    //pattern resault print in 0x54
    printf("%s\n", buf+0x54);
    free(buf);
}

static void cmd_hci_lpbk(void *sc, int argc, char *argv[])
{
    struct icomm_osif_hal *hal_sc = (struct icomm_osif_hal *)sc;
    pthread_t tid;
    unsigned int data_len, data_num;
    unsigned int regval;

    if (argc != 5) {
        printf("command error\n");
        return; 
    }
    
    if (!strcmp(argv[2], "form1"))
        hci_agg_init(1, hal_sc);

    sscanf(argv[3], "%d", &data_len);
    sscanf(argv[4], "%d", &data_num);
    
    /* hci loopback */ 
    write_register(ADR_TX_PACKET_SEND_TO_RX_DIRECTLY, 0x1, hal_sc);
    write_register(ADR_HCI_TX_RX_INFO_SIZE, 0x45050, hal_sc);
    write_register(ADR_ID_LEN_THREADSHOLD1, 0x100801, hal_sc);
   
    hal_sc->finish_test = 0;
    hal_sc->rx_pkt_cnt = 0;
    hal_sc->fail_cnt = 0;
    hal_sc->tx_pkt_cnt = data_num;

    /* form 1 enable or disable */
    read_register(ADR_HCI_TRX_MODE, &regval, hal_sc);
    if (regval & 0x40000000) {
        pthread_create(&tid, NULL, (void*)sdio_rx_frame_rx_agg, (void *)hal_sc);
    } else {
        pthread_create(&tid, NULL, (void*)sdio_rx_frame, (void *)hal_sc);
    }
    
    sdio_test(data_num, data_len, hal_sc);
    if (hal_sc->fail_cnt != 0) {
        printf("UNIT TEST HCI LOOP BACK TEST FAIL!\n");
        hal_sc->fail_cnt = 0;
    } else {
        printf("UNIT TEST HCI LOOP BACK TEST PASS!\n");
    }
}

static void cmd_sdio_dma(void *sc, int argc, char *argv[])
{
    struct icomm_osif_hal *hal_sc = (struct icomm_osif_hal *)sc;
    unsigned int data_len, data_num;
    int ret = 0;

    if (argc != 4) {
        printf("command error\n");
        return;
    }
        
    sscanf(argv[2], "%d", &data_len);
    sscanf(argv[3], "%d", &data_num);
        
    ret = sdio_dma_frame_send(data_num, data_len, hal_sc);
    if (ret != 0) {
        printf("UNIT TEST SDIO DMA TEST FAIL!\n");
    } else {
        printf("UNIT TEST SDIO DMA TEST PASS!\n");
    }
}

static void cmd_txframe_set_mac_env(struct icomm_osif_hal *hal_sc)
{
    unsigned int regval = 0;
    int i;
    /* setting mac test env.*/
    /* do MAC software reset first*/
    write_register(ADR_BRG_SW_RST, 1 << MAC_SW_RST_SFT, hal_sc);
    do {
        read_register(ADR_BRG_SW_RST, &regval, hal_sc);
        i ++;
        if (i >10000){
            printf("MAC reset fail !!!!\n");
            return;
        }
    } while (regval != 0);
    /* hci tx and rx copy len */
    write_register(ADR_HCI_TX_RX_INFO_SIZE, 0x505050, hal_sc); 
    /* PHY disable */
    set_register_bits(ADR_WIFI_PHY_COMMON_ENABLE_REG, (0<<RG_PHY_MD_EN_SFT), RG_PHY_MD_EN_MSK, hal_sc);
    /* rx to host enable */
    read_register(ADR_CONTROL, &regval, hal_sc); 
    regval |= 0x00000004;
    regval &= 0xFFCFFFFF;
    write_register(ADR_CONTROL, regval, hal_sc);

    write_register(ADR_WSID0, 0x1, hal_sc);      // wsid1 en able
    write_register(ADR_BSSID_0, 0xc7b5a500, hal_sc);
    write_register(ADR_BSSID_1, 0xe8d7, hal_sc);
    write_register(ADR_STA_MAC_0, 0xc7b5a500, hal_sc);
    write_register(ADR_STA_MAC_1, 0xe8d7, hal_sc);
    write_register(ADR_PEER_MAC0_0, 0xc7b5a500, hal_sc);
    write_register(ADR_PEER_MAC0_1, 0xe8d7, hal_sc);
    write_register(ADR_REASON_TRAP0, 0xffffffff, hal_sc);
    write_register(ADR_REASON_TRAP1, 0xffffffff, hal_sc);
    write_register(ADR_TX_FLOW_1, 0x61, hal_sc);
    write_register(ADR_RX_FLOW_DATA, 0x14, hal_sc);
    write_register(ADR_MRX_FLT_TB11, 0x0, hal_sc);  // disable ack
    write_register(ADR_WIFI_PHY_COMMON_TX_CONTROL, 0x0, hal_sc);

    /* mac mode => phy loopback */
    set_register_bits(ADR_MAC_MODE, (1<<RG_PHY_LPBK_SFT), RG_PHY_LPBK_MSK, hal_sc);
    write_register(ADR_WIFI_PHY_COMMON_SYS_REG, 0x80008001, hal_sc);  // 40M phy loop back
    /* PHY disable */
    set_register_bits(ADR_WIFI_PHY_COMMON_ENABLE_REG, (1<<RG_PHY_MD_EN_SFT), RG_PHY_MD_EN_MSK, hal_sc);

    //set tx power
    //write_register(0xccb0e194, 0x0, hal_sc);
    //write_register(0xccb0e198, 0x0, hal_sc);
    //write_register(0xccb0e19c, 0x0, hal_sc);
}

static void cmd_txframe_fill_tx_desc(ssv6200_tx_desc *tx_desc, int payload_len, int index)
{
    /* setting tx desc  */
    memset(tx_desc, 0, PB_OFFSET);
    tx_desc->len = payload_len + PB_OFFSET + COMMON_DATA_HDR_LEN;
    tx_desc->hdr_offset = PB_OFFSET;
    tx_desc->unicast        = 1;
    tx_desc->hdr_len        = MAC_ADDR_LEN;
    tx_desc->payload_offset = PB_OFFSET + MAC_ADDR_LEN;
    tx_desc->fcmdidx        = 0;
    tx_desc->fcmd           = 0x61;
    
    tx_desc->c_type = 0x2;
    tx_desc->f80211 = 1;
    tx_desc->rate_rpt_mode = 2;
    tx_desc->rateidx0_main_rate_idx = rate_tbl[index % SIZE_RATE_TBL];
    tx_desc->rateidx0_ctrl_rate_idx = 0xd7;
    tx_desc->rateidx0_ctrl_duration = 0xa5a5;
    tx_desc->rateidx0_phy_11n_htmix_l_length = 0x75;
    tx_desc->rateidx0_trycnt = 1;
    tx_desc->rateidx0_is_last_rate = 1;
    tx_desc->rateidx0_ack_policy = 0;
    tx_desc->rateidx0_do_rts_cts = 0;    
}

static void cmd_txframe(void *sc, int argc, char *argv[])
{
    struct icomm_osif_hal *hal_sc = (struct icomm_osif_hal *)sc;
    unsigned char sample_addr[6] = {0x00,0xa5,0xb5,0xc7,0xd7,0xe8};
    unsigned int payload_len , i, frame_size, index, count;
    common_data_mac_header *mac_hdr;
    pthread_t tid;
    struct ssv_tx_frame_node *entry; 
    ssv6200_tx_desc *tx_desc;
    unsigned char *payload;


    if (argc != 3) {
        printf("command error\n");
        return;
    }
    
    /* for all rates */
    sscanf(argv[2], "%d", &count);
    count = count * SIZE_RATE_TBL;
    
    hal_sc->finish_test = 0;
    hal_sc->fail_cnt = 0;
    hal_sc->rx_pkt_cnt = 0;
    hal_sc->tx_pkt_cnt = count;

    cmd_txframe_set_mac_env(hal_sc);
    /* rx thread */
    pthread_create(&tid, NULL, (void*)phy_lpbk_rx, (void *)hal_sc);

    for (index = 0; index < count; index++) {
        
        entry = (struct ssv_tx_frame_node *)malloc(sizeof(struct ssv_tx_frame_node));
        if (!entry) {
            printf("%s(): alloc ssv_tx_frame_node fail\n", __FUNCTION__);
            break;
        }

        /* generate random payload len */
        payload_len = rand() % 2304;
        frame_size = payload_len + PB_OFFSET + COMMON_DATA_HDR_LEN;
        
        entry->data = (unsigned char *)malloc(sizeof(unsigned char) * frame_size);
        if (!entry) {
            free(entry);
            printf("%s(): alloc data fail\n", __FUNCTION__);
            break;
        }
        
        entry->index = index;
        entry->size = frame_size;
        entry->finish = 0;

        //fill tx_desc
        tx_desc = (ssv6200_tx_desc *)entry->data;
        mac_hdr = (common_data_mac_header *)(entry->data + PB_OFFSET);
        payload = (entry->data + PB_OFFSET + COMMON_DATA_HDR_LEN);

        /* setting data hdr info & payload */
        mac_hdr->fc = 0x0008;
        mac_hdr->durid = 0x0;
        mac_hdr->seqnum++;
        memcpy(mac_hdr->addr1, sample_addr, MAC_ADDR_LEN);
        memcpy(mac_hdr->addr2, sample_addr, MAC_ADDR_LEN);
        memcpy(mac_hdr->addr3, sample_addr, MAC_ADDR_LEN);
        for (i = 0 ; i < payload_len; i++) 
            payload[i] = (unsigned char)rand();
        
        cmd_txframe_fill_tx_desc(tx_desc, payload_len, index);
        list_add_tail(&(entry->list), &(hal_sc->tx_frame));

        /* HW resource */
        hal_sc->buffer_size += (frame_size/256) + ((frame_size%256) ? 1 : 0);
        hal_sc->queue_size++;
        while((hal_sc->buffer_size > 127) || (hal_sc->queue_size > 10));

        tx_frame((unsigned char *)entry->data, entry->size, hal_sc);
    }

    while (1) {
        if (hal_sc->finish_test == 0)
            sleep(1);
        else 
            break;
    }

    if (hal_sc->fail_cnt != 0) {
        printf("UNIT TEST HCI LOOP BACK TEST FAIL!\n");
        hal_sc->fail_cnt = 0;
    } else {
        printf("UNIT TEST HCI LOOP BACK TEST PASS!\n");
    }
}

static void cmd_rxframe(void *sc, int argc, char *argv[])
{
    struct icomm_osif_hal *hal_sc = (struct icomm_osif_hal *)sc;
    unsigned char *buf;
    
    buf = rx_frame(hal_sc);
    hexdump(buf, ((unsigned short *)buf)[0]);
    free(buf);
}

const ssv_cli_cmd_st g_cli_cmd_tbl[] =
{
    { "readreg",           cmd_readreg,                  "[address]"                            },
    { "writereg",          cmd_writereg,                 "[address] [value]"                    },
    { "loadfw",            cmd_loadfw,                   "[pattern_name]"                       },
    { "pattern_test",      cmd_pattern_test,             "[pattern_name]"                       },
    { "hci_lpbk",          cmd_hci_lpbk,                 "[form1|normal] [pkt_len] [pkt_num]"   },
    { "sdio_dma",          cmd_sdio_dma,                 "[data_len] [data_num]"                },
    { "txframe",           cmd_txframe,                  "[pkt_num]"                            },
    { "rxframe",           cmd_rxframe,                  ""                                     },
    { NULL, NULL, NULL },
};

static void cli_cmd_show(void) {

    const ssv_cli_cmd_st *cmd_ptr;

    printf("util:\n");
    for(cmd_ptr=g_cli_cmd_tbl; cmd_ptr->cmd; cmd_ptr++) {
        printf("\t%-15s %s\n", cmd_ptr->cmd, cmd_ptr->cmd_usage);
    }
}



int main(int argc, char *argv[])
{
    const ssv_cli_cmd_st *cmd_ptr;
    struct icomm_osif_hal *icomm_hal_sc = NULL;

    srand((unsigned)time(NULL));

    icomm_hal_sc = (struct icomm_osif_hal *)malloc(sizeof(struct icomm_osif_hal));
    init_icomm_osif_hal(icomm_hal_sc);

    if (argc < 2) {
        cli_cmd_show();
        return 0;
    }
    
    for (cmd_ptr=g_cli_cmd_tbl; cmd_ptr->cmd; cmd_ptr++) {
        if(!strcmp(argv[1], cmd_ptr->cmd)) {
            fflush(stdout);
            cmd_ptr->cmd_handle(icomm_hal_sc, argc, argv);
            break;
        }
    }
    
    if (cmd_ptr->cmd == NULL) {
        printf("\ncmd not found\n");
        cli_cmd_show();
    }

    return 0;
}
