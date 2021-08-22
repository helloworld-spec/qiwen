#include "drv_api.h"
#include "sys_common.h"

//#undef REG32 
#define REG32(_reg_)  (*(volatile unsigned long *)(_reg_))
#define CHIP_CONF_BASE_ADDR		0x08000000// chip configurations
#define RESET_CTRL_REG			(CHIP_CONF_BASE_ADDR + 0x00000020)// module software reset control register
#define INT_SYS_MODULE_REG		(CHIP_CONF_BASE_ADDR + 0x0000002C)

/**
* @brief software reset
* @author 
* @date 
* @param none
* @return void 
* @version 
*/
void sys_reset(void)
{
	void (*F)(void);
	
	set_asic_pll(120);
    mini_delay(100);
    store_all_int();
    gpio_int_disableall();
    vtimer_free();

	REG32(RESET_CTRL_REG)&=((~(1<<26))&(~(1<<27))&(~(1<<28))&(~(1<<29))&(~(1<<30))); //复位除了RAM和ROM的所有片上外设控制器
	REG32(RESET_CTRL_REG)|=((1<<1)|(1<<2)|(1<<3)|(1<<4)|(1<<5)|(1<<6)|(1<<7)|(1<<8)|(1<<9)|(1<<10)|(1<<12)|(1<<13)|(1<<14)|(1<<15)|(1<<19)|(1<<20));
	REG32(INT_SYS_MODULE_REG) = 0;//关闭二级中断。

    MMU_InvalidateIDCache();
    MMU_DisableDCache();
    MMU_DisableICache();
    MMU_DisableMMU();
    MMU_InvalidateTLB();

    F = (void*)0; 
    F();
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
    
}


