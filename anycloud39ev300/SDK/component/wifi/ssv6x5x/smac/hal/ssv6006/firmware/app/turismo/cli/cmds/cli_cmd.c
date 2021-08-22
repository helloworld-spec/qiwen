//#define __SFILE__ "cli_cmd.c"
#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <log.h>
#include <pbuf.h>
#include <mbox/drv_mbox.h>
#include <cli/cli.h>
#include <soc_global.h>
#include <regs.h>
#include <ssv_lib.h>
#include "mac_monitor.h"
#include <cli.h>
#include <FreeRTOS.h>
#include <task.h>
// #include "ssv6200_reg_sim.h"

#define _VERIFY_INIT_CALI_

#ifdef _VERIFY_INIT_CALI_
#include <phy/drv_phy.h>
#endif

#include <p2p/drv_p2p.h>

/*---------------------------------- CMDs -----------------------------------*/

void send_data_to_host(u32 size,u32 loopTimes)
{
    u32 i = 0, allocate_fail = 0;
    PKT_RxInfo *NewPkt = NULL;
    HDR_HostEvent *host_evt;
    u32 packet_len = size | (RX_BUF << 16);

    printf("SDIO testing RX Frame size[%d] loopTimes[%d]\n", size, loopTimes);

    for (i = 0; ;i++)
    {
        do {
    		if (gOsFromISR == false)
				taskENTER_CRITICAL();	
            SET_ALC_LENG(packet_len);
            NewPkt = (PKT_RxInfo *)GET_MCU_PKTID;
    		if (gOsFromISR == false)
				taskEXIT_CRITICAL();	
            //NewPkt = (PKT_RxInfo *)PBUF_MAlloc(size, NOTYPE_BUF);
            if (NewPkt == NULL)
            {
                //OS_MsDelay(10);
                allocate_fail++;
            }
        } while (NewPkt == NULL);

        host_evt = (HDR_HostEvent *)NewPkt;
        host_evt->c_type = HOST_EVENT;
        host_evt->h_event = SOC_EVT_SDIO_TEST_COMMAND;
        host_evt->len = size - sizeof(HDR_HostEvent);
        host_evt->evt_seq_no = i;

        TX_FRAME((u32)NewPkt);

        NewPkt = NULL;
        if (loopTimes != 0)
        {
            if (i == loopTimes)
            {
                printf( "[Allocate fail=%d \n",allocate_fail);
                return;
            } 
        }
        else
        {
            printf( "Error loopTimes=0\n");
            return;
        }
    }
}


static void Cmd_SDIO(s32 argc, s8 *argv[])
{
    if (argc==3) 
    {
        send_data_to_host((u32)ssv6xxx_atoi(argv[1]), (u32)ssv6xxx_atoi(argv[2]));
        return;
    }
    else
    {
        printf("sdio [size] [frames]\n");
    }
}

static void Cmd_ReadBufferInfo(s32 argc, s8 *argv[])
{
    u32 reg_buf_flush_ctrl = REG32(0x70004020); 

    printf("RX buffer reside: 0x%x, RX buffer empty: %x\n", 
        ((reg_buf_flush_ctrl & 0x1fff0000)>>16),
        ((reg_buf_flush_ctrl & 0x8000)>>15));
    printf("HCI_MONITOR_REG5: 0x%08x\n", GET_HCI_MONITOR_REG5); 
    printf("HCI_MONITOR_REG4: 0x%08x\n", GET_HCI_MONITOR_REG4); 
    printf("TX_ID_REMAIN: 0x%x\n", GET_TX_ID_REMAIN); 
    printf("TX_PAGE_REMAIN: 0x%x\n", GET_TX_PAGE_REMAIN); 
}

static void Cmd_Mib(s32 argc, s8 *argv[])
{
    volatile u32 * pointer;

    if (argc==2 && (strcmp(argv[1], "reset")==0 || strcmp(argv[1], "on")==0)) 
    {
        SET_TRX_DEBUG_CNT_ENA(1);

        //Reset MAC MIB
        pointer = (u32 *)MIB_REG_BASE;
        *pointer = 0x00;
        *pointer = 0xffffffff;
        printf("MIB reset success!!\n");
        //Reset PHY MIB
		SET_REG(0xCCB0E088, 0, 31, 0x7fffffff);
		SET_REG(0xCCB0E088, 1, 31, 0x7fffffff);
		SET_REG(0xCCB0EBF8, 0, 20, 0xffefffff);
		SET_REG(0xCCB0EBF8, 1, 20, 0xffefffff);
		SET_REG(0xCCB0F3F8, 0, 20, 0xffefffff);
		SET_REG(0xCCB0F3F8, 1, 20, 0xffefffff);

        return;
    }
    else if (argc==2 && strcmp(argv[1], "off")==0) 
    {
        SET_TRX_DEBUG_CNT_ENA(0);

        pointer = (u32 *)MIB_REG_BASE;
        *pointer = 0x00;

        //Disable PHY MIB
		SET_REG(0xCCB0E088, 0, 31, 0x7fffffff);
		SET_REG(0xCCB0EBF8, 0, 20, 0xffefffff);
		SET_REG(0xCCB0F3F8, 0, 20, 0xffefffff);

        return;
    }
    else if(argc==2)
    {
        if (strcmp(argv[1], "hci") == 0) 
        {
            printf("HW-HCI status:\n");
			printf("    Tx Alloc Count		:%d\n", GET_HCI_TX_ALLOC_CNT);
            printf("    Tx Pkt Count        :%d\n", GET_TX_PKT_COUNTER);
            printf("    Rx Pkt Count        :%d\n", GET_RX_PKT_COUNTER);
            printf("    Host CMD Count      :%d\n", GET_HOST_CMD_COUNTER);
            printf("    Host Event Count    :%d\n", GET_HOST_EVENT_COUNTER);
            printf("    Tx Drop Count       :%d\n", GET_TX_PKT_DROP_COUNTER);
            printf("    Rx Drop Count       :%d\n", GET_RX_PKT_DROP_COUNTER);
            printf("    Tx Pkt Trap Count   :%d\n", GET_TX_PKT_TRAP_COUNTER);
            printf("    Rx Pkt Trap Count   :%d\n", GET_RX_PKT_TRAP_COUNTER);
            printf("    Tx Fail Count       :%d\n", GET_HOST_TX_FAIL_COUNTER);
            printf("    Rx Fail Count       :%d\n", GET_HOST_RX_FAIL_COUNTER);
        } 
        else if (strcmp(argv[1], "tx") == 0)
        {
            printf("MAC TX status:\n");
            printf("    Tx Group                       :%08d\n", GET_MTX_GRP);
            printf("    Tx Fial                        :%08d\n", GET_MTX_FAIL);
            printf("    Tx Retry                       :%08d\n", GET_MTX_RETRY);
            printf("    Tx Multi Retry                 :%08d\n", GET_MTX_MULTI_RETRY);
            printf("    Tx RTS success                 :%08d\n", GET_MTX_RTS_SUCC);
            printf("    Tx RTS Fail                    :%08d\n", GET_MTX_RTS_FAIL);
            printf("    Tx ACK Fail                    :%08d\n", GET_MTX_ACK_FAIL);
            printf("    Tx total frame count           :%08d\n", GET_MTX_FRM);
            printf("    Tx ack frame count             :%08d\n", GET_MTX_ACK_TX);
            printf("    Tx WSID-0 success              :%08d\n", GET_MTX_WSID0_SUCC);
            printf("    Tx WSID-0 frame                :%08d\n", GET_MTX_WSID0_FRM);
            printf("    Tx WSID-0 retry                :%08d\n", GET_MTX_WSID0_RETRY);
            printf("    Tx WSID-0 Total                :%08d\n", GET_MTX_WSID0_TOTAL);
        }
        else if (strcmp(argv[1], "rx") == 0) 
        {
            printf("MAC RX status:\n");
            printf("    RX duplicated frame            :%08d\n", GET_MRX_DUP);
            printf("    RX fragment frame              :%08d\n", GET_MRX_FRG);
            printf("    RX group frame                 :%08d\n", GET_MRX_GRP);
            printf("    RX fcs error frame             :%08d\n", GET_MRX_FCS_ERR);
            printf("    RX fcs success frame           :%08d\n", GET_MRX_FCS_SUC);
            printf("    RX miss a pcaket from PHY frame:%08d\n", GET_MRX_MISS);
            printf("    Allocation failure             :%08d\n", GET_MRX_ALC_FAIL);
            printf("    RX ACK notify                  :%08d\n", GET_MRX_DAT_NTF);
            printf("    RX rts frame notify            :%08d\n", GET_MRX_RTS_NTF);
            printf("    RX cts frame notify            :%08d\n", GET_MRX_CTS_NTF);
            printf("    RX receive ACK frames          :%08d\n", GET_MRX_ACK_NTF);
            printf("    RX BA frame notify             :%08d\n", GET_MRX_BA_NTF);
            printf("    RX data frame notify           :%08d\n", GET_MRX_DATA_NTF);
            printf("    RX manage frame notify         :%08d\n", GET_MRX_MNG_NTF);
            printf("    RX ACK notify but crc error    :%08d\n", GET_MRX_DAT_CRC_NTF);
            printf("    RX BAR frame notify            :%08d\n", GET_MRX_BAR_NTF);
            printf("    RX MBOX miss                   :%08d\n", GET_MRX_MB_MISS);
            printf("    Not-in-IDLE                    :%08d\n", GET_MRX_NIDLE_MISS);
            printf("    CSR notify                     :%08d\n", GET_MRX_CSR_NTF);
        }
        else if (strcmp(argv[1], "phy") == 0) 
        {
            printf("\nTX & RX status:\n");
            //PHY B mode
            pointer = (u32 *)0xCCB0E088;
            printf("    PHY rx_en counter           :%08d\n",(*pointer)&0xffff);
            printf("PHY B mode:\n");
            pointer = (u32 *)0xCCB0EBE4;
            printf("    RX SFD match                :%08d\n",((*pointer)>>16)&0xffff);
            printf("    RX Header CRC match         :%08d\n",(*pointer)&0xffff);
            pointer = (u32 *)0xCCB0EBE8;
            printf("    RX packet CRC error         :%08d\n",(*pointer)&0xffff);
            pointer = (u32 *)0xCCB0EBEC;
            printf("    RX packet CCA               :%08d\n",((*pointer)>>16)&0xffff);
            printf("    RX packet counter           :%08d\n",(*pointer)&0xffff);
            //PHY G/N mode
            printf("PHY G/N mode:\n");
            pointer = (u32 *)0xCCB0F3E8;
            printf("    RX packet CRC error         :%08d\n",(*pointer)&0xffff);
            pointer = (u32 *)0xCCB0F3EC;
            printf("    RX packet CCA               :%08d\n",((*pointer)>>16)&0xffff);
            printf("    RX packet counter           :%08d\n",(*pointer)&0xffff);
        }
		else if (strcmp(argv[1], "debug") == 0) 
        {
            //MAC RX
            printf("MAC RX status:\n");
            printf("    RX duplicated frame                 :%08d\n", GET_MRX_DUP);
            printf("    RX fragment frame                   :%08d\n", GET_MRX_FRG);
            printf("    RX group frame                      :%08d\n", GET_MRX_GRP);
            printf("    RX fcs error frame                  :%08d\n", GET_MRX_FCS_ERR);
            printf("    RX fcs success frame                :%08d\n", GET_MRX_FCS_SUC);
            printf("    RX miss a pcaket from PHY frame     :%08d\n", GET_MRX_MISS);
            printf("    Allocation failure                  :%08d\n", GET_MRX_ALC_FAIL);
            printf("    RX ACK notify                       :%08d\n", GET_MRX_DAT_NTF);
            printf("    RX rts frame notify                 :%08d\n", GET_MRX_RTS_NTF);
            printf("    RX cts frame notify                 :%08d\n", GET_MRX_CTS_NTF);
            printf("    RX receive ACK frames               :%08d\n", GET_MRX_ACK_NTF);
            printf("    RX BA frame notify                  :%08d\n", GET_MRX_BA_NTF);
            printf("    RX data frame notify                :%08d\n", GET_MRX_DATA_NTF);
            printf("    RX manage frame notify              :%08d\n", GET_MRX_MNG_NTF);
            printf("    RX ACK notify but crc error         :%08d\n", GET_MRX_DAT_CRC_NTF);
            printf("    RX BAR frame notify                 :%08d\n", GET_MRX_BAR_NTF);
            printf("    RX MBOX miss                        :%08d\n", GET_MRX_MB_MISS);
            printf("    Not-in-IDLE                         :%08d\n", GET_MRX_NIDLE_MISS);
            printf("    CSR notify                          :%08d\n", GET_MRX_CSR_NTF);
            //MAC TX
            printf("MAC TX status:\n");
            printf("    Tx Group                            :%08d\n", GET_MTX_GRP);
            printf("    Tx Fial                             :%08d\n", GET_MTX_FAIL);
            printf("    Tx Retry                            :%08d\n", GET_MTX_RETRY);
            printf("    Tx Multi Retry                      :%08d\n", GET_MTX_MULTI_RETRY);
            printf("    Tx RTS success                      :%08d\n", GET_MTX_RTS_SUCC);
            printf("    Tx RTS Fail                         :%08d\n", GET_MTX_RTS_FAIL);
            printf("    Tx ACK Fail                         :%08d\n", GET_MTX_ACK_FAIL);
            printf("    Tx total frame count                :%08d\n", GET_MTX_FRM);
            printf("    Tx ack frame count                  :%08d\n", GET_MTX_ACK_TX);
            printf("    Tx WSID-0 success                   :%08d\n", GET_MTX_WSID0_SUCC);
            printf("    Tx WSID-0 frame                     :%08d\n", GET_MTX_WSID0_FRM);
            printf("    Tx WSID-0 retry                     :%08d\n", GET_MTX_WSID0_RETRY);
            printf("    Tx WSID-0 Total                     :%08d\n", GET_MTX_WSID0_TOTAL);
        }
        else
        {
            printf("mib hci/tx/rx/debug \n");
            printf("mib on/off/reset \n");
        }
        return;
    }
    else
    {
        printf("mib hci/tx/rx/debug \n");
        printf("mib on/off/reset \n");
    }
}


#ifdef ENABLE_MEAS_IRQ_TIME 
extern u32 g_int_max_latency;
extern u32 g_int_min_latency;
extern u32 g_int_total_latency;
extern u32 g_int_cnt;        
extern u32 g_enable;

static void Cmd_IRQ(s32 argc, s8 *argv[]){
    u8 mode = 0;
    
    if (argc != 2){
        printf("Usage:\nirq 0->show irq info\nirq 1->enable irq measure\nirq 2->reset irq measure\n");
        return;
    }

    mode = (u32)ssv6xxx_atoi(argv[1]);    
    if(mode==0){               
        printf("total measure time 500000\nmax_latency[%d] min_latency[%d] toal_latency[%d] g_int_cnt[%d]\n", 
                    g_int_max_latency,
                    g_int_min_latency,
                    g_int_total_latency,
                    g_int_cnt
                    );        
    }else if(mode==1){
       printf("int measure enable\n");
       g_enable = 1;
    }else if(mode==2){
       g_enable = 0;
       g_int_min_latency=0; 
       g_int_max_latency=0;
       g_int_total_latency = 0;
       g_int_cnt =0;
       
       printf("reset cnt\nmax_latency[%d] min_latency[%d] toal_latency[%d] g_int_cnt[%d]\n", 
                    g_int_max_latency,
                    g_int_min_latency,
                    g_int_total_latency,
                    g_int_cnt
                    );        
       
    }
    else
    {;}


}
#endif//#ifdef ENABLE_MEAS_IRQ_TIME

#ifdef CONFIG_P2P_NOA
extern void drv_p2p_dump_noa(int r);
static void Cmd_Noa(s32 argc, s8 *argv[])
{
    u8 mode = 0;
    if (argc==2) 
    {
        mode = (u32)ssv6xxx_atoi(argv[1]);
        drv_p2p_dump_noa(mode);
    }
} // end of - Cmd_Read -
#endif

// r addr [size]
static void Cmd_Read(s32 argc, s8 *argv[])
{
    static u32 read_size = 1;
    static u32 read_addr = 0xCD010000;

    if (argc < 2)
    {
        //printf("Usage: r addr [size]\n");
        hex_dump((const u32 *)read_addr, read_size);
        return;
    }

    read_addr = (u32)ssv6xxx_atoi_base(argv[1], 16);
    if (argc > 2)
        read_size = (u32)ssv6xxx_atoi(argv[2]);
    else
        read_size = 1;

    read_addr &= 0xFFFFFFE0;
    read_size = (read_size+7)&0xFFFFFFF8;
    hex_dump((const u32 *)read_addr, read_size);
} // end of - Cmd_Read -

// w addr value
static void Cmd_Write(s32 argc, s8 *argv[])
{
    volatile u32 *write_addr;
    u32           value;

    if (argc != 3)
    {
        printf("Usage: w addr value\n");
        return;
    }

    write_addr = (volatile u32 *)ssv6xxx_atoi_base(argv[1], 16);
    value = (u32)ssv6xxx_atoi_base(argv[2], 16);
    *write_addr = value;
    printf("\n    %08X => %08X : %08X\n", value, (u32)write_addr, *write_addr);
} // end of - Cmd_Write


static void Cmd_BUFFER(s32 argc, s8 *argv[])
{
    u32 tx_page,rx_page;
    if (argc==3) 
    {
        tx_page = ssv6xxx_atoi(argv[1]);
        rx_page = ssv6xxx_atoi(argv[2]);
        SET_ID_LEN_THOLD_INT_EN(1);
        SET_ID_TX_LEN_THOLD(tx_page);
        SET_ID_RX_LEN_THOLD(rx_page);
        printf("TX buffer[%d] RX buffer[%d]!\n",tx_page,rx_page);
        return;
    }
    else
    {
        printf("buffer [tx page number] [rx page number]\n");
    }
}

static void Cmd_MONITOR(s32 argc, s8 *argv[])
{
    u32 monitorTime;
    u8 mode = 0;
    if (argc==2) 
    {
        monitorTime = (u32)ssv6xxx_atoi(argv[1]);
        mac_monitor_start(monitorTime,mode);
        return;
    }
    else if (argc==3) 
    {
        monitorTime = (u32)ssv6xxx_atoi(argv[1]);
        mode = (u32)ssv6xxx_atoi(argv[2]);
        mac_monitor_start(monitorTime,mode);
        return;
    }
    else
    {
        printf("monitor [duration time]\n");
    }
}

static void Cmd_ReadQueue (s32 argc, s8 *argv[])
{
    // 0 - MCU0
    // 1 - HCI
    // 2 - 
    // 3 - Security
    // 4 - MRX
    // 5 - MIC
    // 6 - TX 0
    // 7 - TX 1
    // 8 - TX 2
    // 9 - TX 3
    // A - TX MNG
    // B - Security Sec
    // C - MIC Sec
    // D - 
    // E - 
    // F - Trash Can
    printf("\n[TAG]  MCU - HCI - SEC -  RX - MIC - TX0 - TX1 - TX2 - TX3 - TX4 - SEC - MIC - TSH\n");
    printf("OUTPUT %3d - %3d - %3d - %3d - %3d - %3d - %3d - %3d - %3d - %3d - %3d - %3d - %3d\n",
            GET_FFO0_CNT, GET_FFO1_CNT, GET_FFO3_CNT, GET_FFO4_CNT, GET_FFO5_CNT, GET_FFO6_CNT,
            GET_FFO7_CNT, GET_FFO8_CNT, GET_FFO9_CNT, GET_FFO10_CNT, GET_FFO11_CNT, GET_FFO12_CNT, GET_FFO15_CNT);
    printf("INPUT  %3d - %3d - %3d - %3d - %3d - %3d - %3d - %3d - %3d - %3d - %3d - %3d - %3d\n",
            GET_FF0_CNT, GET_FF1_CNT, GET_FF3_CNT, GET_FF4_CNT, GET_FF5_CNT, GET_FF6_CNT,
            GET_FF7_CNT, GET_FF8_CNT, GET_FF9_CNT, GET_FF10_CNT, GET_FF11_CNT, GET_FF12_CNT, GET_FF15_CNT);
    printf("TX[%d]RX[%d]AVA[%d]\n",GET_TX_ID_ALC_LEN,GET_RX_ID_ALC_LEN,GET_AVA_TAG);
}

void Cmd_Mailbox_Debug()
{
    #define DEBUG_SIZE 0x4000
    char debug_buffer[DEBUG_SIZE];

    {
        printf("MAILBOX debug buffer=%08x size=%08x\n", (int)debug_buffer,DEBUG_SIZE);
    
        SET_MB_DBG_CFG_ADDR((int)debug_buffer);
        SET_MB_DBG_LENGTH(DEBUG_SIZE);
        SET_MB_DBG_EN(1);
        SET_MB_ERR_AUTO_HALT_EN(1);
    }

    while(GET_CPU_INT == 0)
    {};

    SET_MB_DBG_EN(0);
    printf("MAILBOX debug Get result\n");
}

#ifdef _VERIFY_INIT_CALI_
static void Cmd_Phy(s32 argc, s8 *argv[]) {
#if 0  // ToDo Liam: not supported yet.
    u32 argv_api;
    u32 loop;
    u32 loop_i;
    argv_api = ssv6xxx_atoi(argv[2]);

    //
    if ((strcmp(argv[1], "init_cali") == 0)) {
        // ##
        printf("# init_cali running\n");
        printf("## cfg_pa as %d\n", g_rf_config.cfg_pa);
        if (argc == 3) {
            loop = 1;
        }
        else if (argc == 4) {
            loop = ssv6xxx_atoi(argv[3]);
        }
        else {
            return;
        }
        // ##
        for (loop_i=0; loop_i < loop; loop_i++) {
            drv_phy_init_cali(argv_api);
        }
    } else if ((strcmp(argv[1], "channel") == 0)) {
        printf("# channel change running\n");
        drv_phy_channel_change(argv_api);
    } else if ((strcmp(argv[1], "cali_rtbl_load") == 0)) {
        printf("# cali_rtbl_load running\n");
        drv_phy_cali_rtbl_load(argv_api);
    } else if ((strcmp(argv[1], "cali_rtbl_load_def") == 0)) {
        printf("# cali_rtbl_load_def running\n");
        drv_phy_cali_rtbl_load_def(argv_api);
    } else if ((strcmp(argv[1], "cali_rtbl_reset") == 0)) {
        printf("# cali_rtbl_reset running\n");
        drv_phy_cali_rtbl_reset(argv_api);
    } else if ((strcmp(argv[1], "cali_rtbl_set") == 0)) {
        printf("# cali_rtbl_set running\n");
        drv_phy_cali_rtbl_set(argv_api);
    } else if ((strcmp(argv[1], "cali_rtbl_export") == 0)) {
        printf("# cali_rtbl_export running\n");
        drv_phy_cali_rtbl_export(argv_api);
    } else if ((strcmp(argv[1], "evm_tk") == 0)) {
        printf("# evm_tk running\n");
        drv_phy_evm_tk(argv_api);
    } else if ((strcmp(argv[1], "tone_tk") == 0)) {
        printf("# tone_tk running\n");
        drv_phy_tone_tk(argv_api);
    } else if ((strcmp(argv[1], "cfg_pa") == 0)) {
        printf("# set cfg_pa as %d\n", argv_api);
        g_rf_config.cfg_pa = argv_api;
    } else if ((strcmp(argv[1], "dpd_ch") == 0)) {
        printf("# set phy_cali_config_papd.cfg_ch %d\n", argv_api);
        phy_cali_config_papd.cfg_ch = argv_api;
    }
#endif
}
#endif

#if 0
void reg_dump_bank()
{
    s32 i,n;

    for (n = 0; n < BANK_COUNT; n++)
    {
        printf("bank [%d] (%s) : size = %d \n", n, STR_BANK_SSV6200[n], SIZE_BANK_SSV6200[n]);
        if ((BASE_BANK_SSV6200[n]==HCI_REG_BASE) ||
            (BASE_BANK_SSV6200[n]==MRX_REG_BASE) ||
            (BASE_BANK_SSV6200[n]==HIF_INFO_BASE) ||
            (BASE_BANK_SSV6200[n]==PHY_RATE_INFO_BASE) ||
            (BASE_BANK_SSV6200[n]==AMPDU_REG_BASE) ||
            (BASE_BANK_SSV6200[n]==MAC_GLB_SET_BASE) ||
            (BASE_BANK_SSV6200[n]==TXQ0_MT_Q_REG_CSR_BASE) ||
            (BASE_BANK_SSV6200[n]==TXQ1_MT_Q_REG_CSR_BASE) ||
            (BASE_BANK_SSV6200[n]==TXQ2_MT_Q_REG_CSR_BASE) ||
            (BASE_BANK_SSV6200[n]==TXQ3_MT_Q_REG_CSR_BASE) ||
            (BASE_BANK_SSV6200[n]==TXQ4_MT_Q_REG_CSR_BASE) ||
            (BASE_BANK_SSV6200[n]==MT_REG_CSR_BASE))
        {
            for (i=0; i< (s32)SIZE_BANK_SSV6200[n]; i+=4)
            {
                printf("%08x %08x\n", BASE_BANK_SSV6200[n]+i, *(u32 *)(BASE_BANK_SSV6200[n]+i));
            }
        }
    }
}
#endif // 0

#ifdef CONFIG_RX_MGMT_CHECK
extern int enable_rx_mgmt_check;
static void Cmd_Rx_Mgmt(s32 argc, s8 *argv[])
{
    if (argc != 2) {
        printf("rx_mgmt [X]\n");
        printf("X:\n");
        printf("  0 = disable rx mgmt check\n");        
        printf("  1 = enable, beacon 500ms timeout, tx/rx page > 10 check\n");              
        printf("  2 = enable, beacon 500ms timeout, tx/rx page > 10,\n");
        printf("              beacon seq no.\n");
        return ;
    }

    enable_rx_mgmt_check = ssv6xxx_atoi(argv[1]);
}
#endif //CONFIG_RX_MGMT_CHECK

/* ---------------------- Registered CMDs to CMD Table ---------------------- */
extern void _notify_host_reset();
const CLICmds gCliCmdTable[] =
{
    { "mib",            Cmd_Mib,                        "SoC MIB Counter"},
    { "r",              Cmd_Read,                       "Read memory or register."},
    { "w",              Cmd_Write,                      "Write memory or register."},
    { "q",              Cmd_ReadQueue,                  "Show sizes of queues"},
    { "queue",          Cmd_ReadQueue,                  "Show sizes of queues"},
    #ifdef CONFIG_P2P_NOA
    { "n",              Cmd_Noa,                        "Show noa parameter and internal states"},
    #endif

    #ifdef ENABLE_MEAS_IRQ_TIME
    { "irq",            Cmd_IRQ,                        "Debug irq latency"},
    #endif
    { "buffer",         Cmd_BUFFER,                     "Setting TX RX buffer"},
    { "monitor",        Cmd_MONITOR,                    "Monitor time (second)"},
    { "mailbox-debug",  Cmd_Mailbox_Debug,              "Mailbox debug"},
    //{ "reg",            reg_dump_bank,                  "Register dump"},
    #ifdef _VERIFY_INIT_CALI_
    { "phy",            Cmd_Phy,                        "Phy API"},
    #endif
    { "sdio",           Cmd_SDIO,                       "SDIO rx throughput testing"},
    { "host-reset",     _notify_host_reset,             "Trigger host reset"},
    { "check-buffer",   Cmd_ReadBufferInfo,            "Show buffer information"},
    #ifdef CONFIG_RX_MGMT_CHECK
    { "rx_mgmt",        Cmd_Rx_Mgmt       ,             "enable rx management frame check"},
    #endif //CONFIG_RX_MGMT_CHECK

    /*lint -save -e611 */
    { (const char *)NULL, (CliCmdFunc)NULL, (const char *)NULL },
    /*lint -restore */
};


