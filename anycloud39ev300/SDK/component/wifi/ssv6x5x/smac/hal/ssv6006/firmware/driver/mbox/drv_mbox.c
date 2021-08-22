#include <config.h>
#include <stdio.h>
#include <log.h>
#include <rtos.h>
#include <regs.h>
#include <pbuf.h>
#include <ssv_lib.h>
#include "drv_mbox.h"


//Mount mailbox FIFO for HW module(low priority)
/*
	default CPU(0)
	Command Flow => 87654321   drv_mailbox_cpu_ff(0x80000000,1)  => 1-2-3-4-5-6-7-8-9-0
	Command Flow => 87654321   drv_mailbox_cpu_ff(0x80000000,4)  => 4-5-6-7-8-9-0
	Command Flow => 87654321   drv_mailbox_cpu_ff(0x80000000,0)  => 0
	Command Flow => 87654321   drv_mailbox_cpu_ff(0x80000000,9)  => 0
*/

#define ADDRESS_OFFSET	16
s32 drv_mailbox_cpu_ff(u32 pktmsg, u32 hw_number)
{
	u8 failCount=0;
#ifdef MAILBOX_DEBUG
	LOG_PRINTF("mailbox 0x%08x\n\r",(u32)((pktmsg >> ADDRESS_OFFSET) | (hw_number << HW_ID_OFFSET))));
#endif

	while (GET_CH0_FULL)
	{
	    if (failCount++ < 1000) continue;
            printf("ERROR!!MAILBOX Block[%d]\n", failCount);
            return FALSE;
	} //Wait until input queue of cho is not full.

	{
		SET_HW_PKTID((u32)((pktmsg >> ADDRESS_OFFSET) | (hw_number << HW_ID_OFFSET)));
		return TRUE;
	}
}

//Mount mailbox FIFO for HW module(high priority) 
s32 drv_mailbox_cpu_pri_ff(u32 pktmsg, u32 hw_number)
{
	u8 failCount=0;
#ifdef MAILBOX_DEBUG
	LOG_PRINTF("mailbox 0x%08x\n\r",(u32)((pktmsg >> ADDRESS_OFFSET) | (hw_number << HW_ID_OFFSET)));
#endif

	while(GET_CH0_FULL)
	{
		printf("ERROR!!MAILBOX Block[%d]\n",failCount++);
	};//Wait until input queue of cho is not full.


	{
		SET_PRI_HW_PKTID((u32)((pktmsg >> ADDRESS_OFFSET) | (hw_number << HW_ID_OFFSET)));
		return TRUE;
	}
}


s32 drv_mailbox_cpu_next(u32 pktmsg)
{
    PKT_Info *pkt_info=(PKT_Info *)pktmsg;
    u32 shiftbit, eng_id;

    // shiftbit = pkt_info->fCmdIdx << 2;
    pkt_info->fCmdIdx ++;
    shiftbit = pkt_info->fCmdIdx << 2;
    eng_id = ((pkt_info->fCmd & (0x0F<<shiftbit))>>shiftbit);	   

    return drv_mailbox_cpu_ff(pktmsg, eng_id);
}


void drv_mailbox_arb_ff(void)
{
#ifdef MAILBOX_DEBUG
		LOG_PRINTF("ARB FIFO=(%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x)\n\r", 
			GET_FF0_CNT,GET_FF1_CNT,GET_FF2_CNT,GET_FF3_CNT,GET_FF4_CNT,
			GET_FF5_CNT,GET_FF6_CNT,GET_FF7_CNT,GET_FF8_CNT,GET_FF9_CNT);
#endif
}

inline s32 ENG_MBOX_NEXT(u32 pktmsg){
	return drv_mailbox_cpu_next(pktmsg);
}

inline s32 ENG_MBOX_SEND(u32 hw_number,u32 pktmsg){
	return drv_mailbox_cpu_ff(pktmsg,hw_number);
}

inline s32 TX_FRAME(u32 pktmsg){
	return drv_mailbox_cpu_ff(pktmsg,M_ENG_HWHCI);
}

extern bool g_ps;
inline s32 FRAME_TO_HCI(u32 pktmsg){
    if (g_ps==true)
        return drv_mailbox_cpu_ff(pktmsg,M_ENG_TRASH_CAN);
    else
        return drv_mailbox_cpu_ff(pktmsg,M_ENG_HWHCI);
}

#if (MAILBOX_DBG == 1)

#define DEBUG_SIZE 2048
char debug_buffer[DEBUG_SIZE];

void dump_mailbox_dbg (int num)
{
    int i;
    int total_num = (num > 0) ? (num * 4) : sizeof(debug_buffer);
    //u32 b = *(volatile u32 *)(0xc10000a8);
    //su32 c = *(volatile u32 *)(0xc10000b0);

    LOG_PRINTF("0xC10000A0: %08X\n", *(volatile u32 *)(0xc10000A0));
    LOG_PRINTF("0xC10000A4: %08X\n", *(volatile u32 *)(0xc10000A4));
    LOG_PRINTF("0xC10000A8: %08X\n", *(volatile u32 *)(0xc10000A8));
    LOG_PRINTF("0xC10000AC: %08X\n", *(volatile u32 *)(0xc10000AC));
    LOG_PRINTF("0xC10000B0: %08X\n", *(volatile u32 *)(0xc10000B0));

    for (i = 0; i < total_num; i += 4)
    {
#if 0
        u32    log_type = (u32)(debug_buffer[0] >> 5);
        u32    time_tag = (((u32)(debug_buffer[0] & 0x1F)) << 7) + (u32)(debug_buffer[1] >> 1);
#endif
        u32    log = *(u32 *)&debug_buffer[i];
        u32    log_type = log >> 29;
        u32    time_tag = (log >> 17) & 0x0FFF;

        LOG_PRINTF("%d: %d - ", time_tag, log_type);

        switch (log_type) {
        case 0: // Normal case
            {
            u32     r_w = (log >> 16) & 0x01;
            u32     port = (log >> 12) & 0x0F;
            u32     HID = (log >> 8) & 0x0F;
            u32     PID = log & 0x0FF;
            LOG_PRINTF("RW: %u, PRT: %u, HID: %u, PID: %u\n", r_w, port, HID, PID);
            }
            break;
            break;
        case 1: // Allocate ID failed
            {
            u32     ID = log & 0x03F;
            LOG_PRINTF("PID: %u\n", ID);
            }
            break;
        case 2: // Release ID failed
            {
            u32     ID = log & 0x03F;
            LOG_PRINTF("PID: %u\n", ID);
            }
            break;
        case 4: // Packet demand request
            {
            u32     port = (log >> 8) & 0x0F;
            u32     ID = log & 0x0FF;
            LOG_PRINTF("PRT: %u, HID: %u\n", port, ID);
            }
            break;
        case 5: // ID allocation request
            {
            u32     page_length = (log >> 7) & 0xFF;
            u32     chl = (log >> 15) & 0x03;
            u32     ID = log & 0x07F;
            LOG_PRINTF("LEN: %u, CHL: %u, ID: %u\n", page_length, chl, ID);
            }
            break;
        case 6: // ID allocation ack
            {
            u32     nack = 0;
            u32     page_length = (log >> 7) & 0xFF;
            u32     chl = (log >> 15) & 0x03;
            u32     ID = log & 0x07F;
            LOG_PRINTF("NCK: %u, LEN: %u, CHL: %u, ID: %u\n", nack, page_length, chl, ID);
            }
                        break;
        case 7: // ID allocation nack
            {
            u32     nack = 1;
            u32     page_length = (log >> 7) & 0xFF;
            u32     chl = (log >> 15) & 0x03;
            u32     ID = log & 0x07F;
            LOG_PRINTF("NCK: %u, LEN: %u, CHL: %u, ID: %u\n", nack, page_length, chl, ID);
            }
            break;
        }
#if 0
        LOG_PRINTF("%02X %02X %02X %02X\n",
                   debug_buffer[i + 0], debug_buffer[i + 1],
                   debug_buffer[i + 2], debug_buffer[i + 3]);
#endif
    }
}

void enable_mailbox_dbg (void)
{
    printf("MAILBOX debug buffer = %08x\n", (int)debug_buffer);

    SET_MB_DBG_CFG_ADDR((int)debug_buffer);
    SET_MB_DBG_LENGTH(DEBUG_SIZE);
    SET_MB_DBG_EN(1);
    SET_MB_ERR_AUTO_HALT_EN(1);
}

#endif // MAILBOX_DBG


