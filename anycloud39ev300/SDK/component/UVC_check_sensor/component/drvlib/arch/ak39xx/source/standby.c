/**
 * @file
 * @brief ANYKA software
 * this file will constraint the function of enter & exit standby model
 *
 * @author Zou tianxiang
 * @date 2008-08-30
 * @version 1.0
 */
 
#include "anyka_types.h"
#include "anyka_cpu.h"
#include "standby.h"


/**
    该函数仅在编译standby代码时用到
 */
#ifdef __CC_ARM
#pragma arm section code = "__inner_"
#endif
void  enter_standby(unsigned long param) 
{
    unsigned long i, value;

    for(i=0; i<20000; i++);

    value = *(volatile unsigned int*)(0x21000010);
    //value &= (1UL<<31);   //keep cas_latency
    
    /*
                            CKE     CS      RAS     CAS     WE 
        bit                 25      19      20        21      	22
        auto-refresh        H       L       L       L       H
        enter self-refresh  L       L       L       L       H
        exit self-refresh   H       L       H       H       H
     */
     
    //send precharge command
    *(volatile unsigned int*)(0x21000010) = 0x2A00400; // very important!

	//delay
	for(i = 0; i < 10; i++);
    
    //send auto refresh command
    *(volatile unsigned int*)(0x21000010) = 0x2C00000;
   //close auto refresh
	*(volatile unsigned int*)(0x2100000C) &= (~1UL);
        
    //enter self refresh     
    *(volatile unsigned int*)(0x21000010) = 0x4C00000; //CKE low

    //after enter self-refreh, wait  serveral clock-cycle before stop dram external clock
    for (i=0; i<0x100; i++);

    //enter standby
    *(volatile unsigned int*)(0x21800000) = 1;

    for(i=0; i<20000; i++);    //ensure CLK stable, at least 100us

    //exit self refresh and send NOP command
    *(volatile unsigned int*)(0x21000010) = 0x2F00000; //CKE high
    
    //send two NOP command
#ifdef __GNUC__
    __asm("nop");
    __asm("nop");
#endif
#ifdef __CC_ARM
    __asm
    {
        nop
        nop
    }
#endif   
        
    //send auto-refresh command
	*(volatile unsigned int*)(0x2100000C) |= 1;

    for(i=0; i<5000; i++);

}
#ifdef __CC_ARM
#pragma arm section
#endif

/* end of file */
