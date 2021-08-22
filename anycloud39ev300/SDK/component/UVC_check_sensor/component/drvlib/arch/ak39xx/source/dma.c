/**
 * @file dma.c
 * @brief provide functions of DMA operations
 *
 * Copyright (C) 2004 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author 
 * @date 2005-07-24
 * @version 1.0
 */
#include "anyka_cpu.h"
#include "anyka_types.h"
#include "drv_api.h"
#include <string.h>
#include "l2.h"
#include "freq.h"

void * DMA_Memcpy(void * dst, void * src, unsigned long count)
{
    unsigned long reg_value;
    unsigned long reg_id;
    unsigned long i, cnt = 0, loop = 0;
    unsigned char  buf_id;
#if 0
    buf_id = l2_alloc(ADDR_PCM_RX); //use PCM_RX
    loop = count>>9;    //512B
    
    for (i=0; i<loop; i++)
    {        
        //set source address
        reg_id = L2_DMA_ADDR+ (buf_id << 2);    
        outl(((unsigned long)src + (i<<9)), reg_id);

        //set DMA Operation Times  
        reg_id = L2_DMA_CNT + (buf_id<<2);    
        outl(8, reg_id);

        //set DMA dir, mem to buf
        REG32(L2_COMBUF_CFG) |= ( 1 << (8+buf_id));
        
        //start bufx DMA
        reg_value = inl(L2_DMA_REQ);
        reg_value &= ~((1 << 9) |(0xffffUL << 16));        //clear other buf req
        reg_value |= (1 << (24 + buf_id));
        outl(reg_value, L2_DMA_REQ);

        //wait dma finish
        while(REG32(L2_DMA_REQ) & (1<<(24 + buf_id)));


        //set dest address
        reg_id = L2_DMA_ADDR+ (buf_id << 2);    
        outl(((unsigned long)dst+ (i<<9)), reg_id);

        //set DMA Operation Times  
        reg_id = L2_DMA_CNT + (buf_id<<2);    
        outl(8, reg_id);

        //set DMA dir, mem to buf
        REG32(L2_COMBUF_CFG) &= ~( 1 << (8+buf_id));
        
        //start bufx DMA
        reg_value = inl(L2_DMA_REQ);
        reg_value &= ~((1 << 9) |(0xffffUL << 16));        //clear other buf req
        reg_value |= (1 << (24 + buf_id));
        outl(reg_value, L2_DMA_REQ);

        //wait dma finish
        while(REG32(L2_DMA_REQ) & (1<<(24 + buf_id)));        
    }

    //flush cache
    MMU_InvalidateDCache();

    l2_free(ADDR_PCM_RX);
    
    if (count&0x1ff)
        memcpy((unsigned char*)((unsigned long)dst+ (i<<9)), (unsigned char*)((unsigned long)src+ (i<<9)), count&0x1ff);
#endif
    return dst;
}

bool dma_init(void)
{
    return true;
}

/**
 * sdram max clk 162M
 * it doesn't work well now
 */
void sdram_on_change(unsigned long sys_clk)
{
    /* unit in ns
     * refer to sdram spec.
     * tWTR is 1 MCLK, tWR is 2 MCLK
     */
     //for -10 sdram timing
    #define tRAS_S10    50 //RAS active time, min is 45
    #define tRCD_S10    23 //RAS to CAS delay, min is 18
    #define tRP_S10     23 //RAS precharge time, min is 18
    #define tRFC_S10    80 //auto refresh, RAS cycle time, same as tRC

    //for -7 sdram timing
    #define tRAS_S7    50 //RAS active time, min is 45
    #define tRCD_S7    20 //RAS to CAS delay, min is 18
    #define tRP_S7     20 //RAS precharge time, min is 18
    #define tRFC_S7    80 //auto refresh, RAS cycle time, same as tRC

    unsigned long cycle, value, auto_refresh;
    unsigned char  t_ras=15, t_rcd=7, t_rp=7, t_rfc=15, t_wtr=3, t_wr=2;   //in clk cycle
    unsigned short tRAS_ture, tRCD_ture, tRP_ture, tRFC_ture;

    if(sys_clk > 1000000)
        cycle = 1000/(sys_clk/1000000);
    else
        cycle = 1000;

    if(sys_clk > 100 * 1000 * 1000)
    {
        tRAS_ture = tRAS_S7;
        tRCD_ture = tRCD_S7;
        tRP_ture = tRP_S7;
        tRFC_ture = tRFC_S7;
    }
    else
    {
        tRAS_ture = tRAS_S10;
        tRCD_ture = tRCD_S10;
        tRP_ture = tRP_S10;
        tRFC_ture = tRFC_S10;
    }
    //akprintf(C1, M_DRVSYS, "tRAS_%d,tRCD_%d,tRP_%d,tRFC_%d\n", tRAS_ture, tRCD_ture, tRP_ture, tRFC_ture);

#if 1    
    //t_ras = (tRAS_ture + cycle - 1) / cycle;     if (t_ras > 15) t_ras = 15;
    t_ras = 15;  //use max value
    t_rcd = tRCD_ture / cycle + 1; if (t_rcd > 7) t_rcd = 7;
    t_rp  = tRP_ture / cycle + 1; if (t_rp > 7) t_rp = 7;
    t_rfc = tRFC_ture / cycle + 1; if (t_rfc > 15) t_rfc = 15;
    t_wtr = 1;
    t_wr  = 2;
#endif

    //update sdram AC charateristics
    value = REG32(SDRAM_CFG_REG2);
    value &= ~(0x3<<24); //clear wtr
    value |= t_wtr<<24;   
    value &= ~(0xf<<20); //clear ras
    value |= t_ras<<20;
    value &= ~(0x7<<14); //clear wr
    value |= t_wr<<14;      
    value &= ~(0x7<<11); //clear rcd
    value |= t_rcd<<11;
    value &= ~(0x1f<<6); //clear rfc
    value |= t_rfc<<6;
    value &= ~(0x7<<3); //clear rp
    value |= t_rp<<3;    
    REG32(SDRAM_CFG_REG2) = value;

    if ((value & 0x7) <= 3)
    {
        //  <=16M SDRAM
        auto_refresh = (64*1000*1000 + 4095) / 4096; //ns
    }
    else if ((value & 0x7) >= 4)
    {
        // >=32M SDRAM
        auto_refresh = (64*1000*1000 + 8191) / 8192; //ns
    }

    //update auto refresh cycle
    auto_refresh = (auto_refresh + cycle - 1)/ cycle;
    if (auto_refresh > 0xffff) 
        auto_refresh = 0xffff;    

    REG32(SDRAM_CFG_REG3) = auto_refresh;

    us_delay(10);
    
}

static int sdram_reg(void)
{
    freq_register(E_MEMORY_CALLBACK, sdram_on_change);
    return 0;
}

#ifdef __CC_ARM
#pragma arm section rwdata = "__initcall_", zidata = "__initcall_"
#endif
module_init(sdram_reg)
#ifdef __CC_ARM
#pragma arm section
#endif
