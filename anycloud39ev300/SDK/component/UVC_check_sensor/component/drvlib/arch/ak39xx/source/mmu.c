/**
 * @file mmu.c
 * @brief mmu function file, provide drivers of MMU module
 *
 * Copyright (C) 2004 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author 
 * @date 2005-07-24
 * @version 1.0
 * @ref AK3223 technical manual.
 */
#include "anyka_cpu.h"
#include "anyka_types.h"
#include "drv_api.h"


extern void  MMU_CleanInvalidateDCacheIndex(unsigned long index);
extern void  MMU_SetTTBase(unsigned long base);
extern void  MMU_SetDomain(unsigned long domain);
extern void  MMU_SetProcessId(unsigned long pid);

static unsigned long mmuTT_Start_Addr;


// 1) Only the section table is used. 
// 2) The cachable/non-cachable area can be changed by MMT_DEFAULT value.
//    The section size is 1MB.

void MMU_Init(unsigned long mmutt_start_addr)
{
	unsigned long i,j;
	//========================== IMPORTANT NOTE =========================
	//The current stack and code area can't be re-mapped in this routine.
	//If you want memory map mapped freely, your own sophiscated MMU
	//initialization code is needed.
	//===================================================================

	if((mmutt_start_addr & 0x3FFF) != 0)
	{
		printf("start address 0x%lx is not aligned to 16K\n", mmutt_start_addr);
		while(1);
	}

	mmuTT_Start_Addr = mmutt_start_addr;   //deliver para to MMU_SetMTT  

    //store_all_int();
	MMU_DisableDCache();
	MMU_DisableICache();    
	MMU_DisableMMU();

	MMU_InvalidateICache();
	MMU_InvalidateDCache();
	MMU_InvalidateTLB();

    /* first set all 4G space as non-access */
    MMU_SetMTT(0x00000000,0xf0000000-1,0x00000000,RW_FAULT);    //last few words used for some purpose
    /* only mapping avaiable space */
	MMU_SetMTT(0x00000000,0x00100000-1,0x00000000,RW_NCNB);  //on-chip ROM
	MMU_SetMTT(0x08000000,0x08100000-1,0x08000000,RW_NCNB);  //system control
	MMU_SetMTT(0x20000000,0x23000000-1,0x20000000,RW_NCNB);  //SFR !!
	MMU_SetMTT(0x48000000,0x48100000-1,0x48000000,RW_NCNB);  //L2
    MMU_SetMTT(0x80000000,0x80000000+0x4000000-1,0x80000000,RW_CNB);   //DRAM
	//MMU_SetMTT(0x80000000,0x80001000-1,0x80000000,RW_NCNB);  //TCM
    MMU_SetMTT(0x21000000,0x21FFFFFF-1,0x21000000,RW_NCNB);  //SFR !!


    MMU_CleanSR();
	MMU_SetTTBase(mmutt_start_addr);
	MMU_SetDomain(0x55555550|DOMAIN1_ATTR|DOMAIN0_ATTR); 
	MMU_SetProcessId(0x0);

	MMU_EnableMMU();
	MMU_EnableICache();
	MMU_EnableDCache(); //DCache should be turned on after MMU is turned on.
	//restore_all_int();
	//MMU_EnableWriteBuffer();
}     
    
void MMU_SetMTT(unsigned long vaddrStart,unsigned long vaddrEnd,unsigned long paddrStart,unsigned long attr)
{
    unsigned long *pTT;
    unsigned long i,nSec;

    pTT = (unsigned long *)mmuTT_Start_Addr + (vaddrStart >> 20);	/* pounsigned longer arith! */
    nSec = (vaddrEnd >> 20) - (vaddrStart >> 20);
    for(i = 0; i <= nSec; i++) 
    {
        *pTT++ = attr | (((paddrStart >> 20)  + i) << 20);
    }
}

/* ARM doesn't support*/
void MMU_DisableWriteBuffer()
{

}

void MMU_InvalidateDCache()
{
    irq_mask();
    MMU_Clean_Invalidate_Dcache();
    irq_unmask();
}

