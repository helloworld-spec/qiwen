/**
 * @file sysctl.c
 * @brief provide functions to control system clock setting
 *
 * Copyright (C) 2004 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author 
 * @date 2005-07-24
 * @version 1.0
 * @ref AK322x technical manual.
 */
#include "anyka_cpu.h"
#include "anyka_types.h"
#include "drv_api.h"
#include "sysctl.h"
#include "drv_module.h"

/* Only GPIO/Timer,CRC,RTC,CPU,SRAM enable */
#define DEFAULT_CLOCK_ON    (CLOCK_CTRL_RAM|CLOCK_CTRL_L2)    

static const unsigned long  clock_value_tbl[CLOCK_NBITS]={
	CLOCK_DEFAULT_ENABLE,
	CLOCK_CTRL_MCI1,           /*mmcsd*/         
	CLOCK_CTRL_MCI2,           /*sdio*/          
	CLOCK_CTRL_ADC2 ,          /*ad*/         
	CLOCK_CTRL_DAC ,           /*da*/           
	CLOCK_CTRL_SPI0 ,          /*spi0*/        
	CLOCK_CTRL_SPI1 ,          /*spi1*/        
	CLOCK_CTRL_UART0,          /*uart0*/         
	CLOCK_CTRL_UART1 ,         /*uart1*/        
	CLOCK_CTRL_L2 ,            /*l2*/           
	CLOCK_CTRL_TWI,            /*twi*/           
	CLOCK_CTRL_GPIO,           /*gpio*/         
	CLOCK_CTRL_MAC,            /*mac*/           
	CLOCK_CTRL_ENCRYPT,        /*encrypt*/     
	CLOCK_CTRL_USBOTG,         /*usb*/       
	CLOCK_CTRL_CAMERA,         /*isp*/      
	CLOCK_CTRL_VIDEO_ENCODER,  /*video encoder*/ 
	CLOCK_CTRL_RAM,            /*DRAM*/   
};

static void config_default_clock()
{
    unsigned long reg;
    
    /*disable all, enable default clock*/
    reg = REG32(CLOCK_CTRL_REG);
    reg |= 0xFFFFFFFF;
    reg &= ~DEFAULT_CLOCK_ON;
    REG32(CLOCK_CTRL_REG) = reg;
}

static void config_clock(unsigned long reg, unsigned long clock, bool enable)
{
    if (enable)
    {
        REG32(reg) &= ~clock;
    }
    else
    {
        REG32(reg) |= clock;
    }

}

/**
 * @BRIEF SetSleepMode
 * @AUTHOR guoshaofeng
 * @DATE 2007-04-23
 * @PARAM[in] vT_SleepModeConfig SleepModeConfig
 * @RETURN void
 * @RETVAL
 * attention: if you close some parts such as LCD 
            you must init it again when you reopen 
            it 
            some settings may cause serious result
            better not to use it if not familar
 */
void sysctl_clock(unsigned long module)
{

    unsigned long clock, i;

    irq_mask();

    if (module == CLOCK_DEFAULT_ENABLE)
    {
        /* config default clock */
        config_default_clock();     
    }
    else if (module < CLOCK_ENABLE_MAX)    //enable clock
    {
        clock = 0;

        for (i=0; i<CLOCK_NBITS; i++)
        {
            if (module & (1<<i))
            {
                clock |= clock_value_tbl[i];

            }
        }
        
        //enable clock
        if (clock)
        {
            config_clock(CLOCK_CTRL_REG, clock, true);
        }
    }
    else                //disable clock
    {
        //get module
        module = (~module) & (CLOCK_ENABLE_MAX-1);

        clock = 0;

        for (i=0; i<CLOCK_NBITS; i++)
        {
            if (module & (1<<i))
            {
                clock |= clock_value_tbl[i];
            }
        }

        //diable clock
        if (clock)
            config_clock(CLOCK_CTRL_REG, clock, false);
    }

    irq_unmask();
    return;
}

/**
 * @brief get module clock states
 * @author LHS
 * @date 2011-10-26
 * @param module [in]: module to be get states
 * @return bool: return TURE mean clock is enable.
 */
bool sysctl_get_clock_state(unsigned long module)
{
	unsigned long  i,clock_reg,clock_bit = 0;
	bool ret = false;
	
	if (module > (1 << (CLOCK_NBITS-1)))
	{
		return ret;
	}

	//get module clock control register bit
	for (i=0; i<CLOCK_NBITS; i++)
    {
        if (module & (1<<i))
        {
           clock_bit = clock_value_tbl[i];
		   break;
        }
    }

	// no find suited
	if (CLOCK_NBITS == i)
	{
		return ret;
	}

	irq_mask();
	clock_reg = REG32(CLOCK_CTRL_REG);
	if (!(clock_reg & clock_bit)) // when bit is 0, module clock is enable
	{
		ret = true;
	}
	irq_unmask();
	
	return ret;
}

/**
 * @brief reset module 
 * @author liao_zhijun
 * @date 2010-07-20
 * @param module [in]: module to be reset
 * @return void
 */
void sysctl_reset(unsigned long module)
{
    irq_mask();

    REG32(RESET_CTRL_REG) |= (1<<module);
    REG32(RESET_CTRL_REG) &= ~(1<<module);

    irq_unmask();

}

/**
 * @brief get a new module that is shared with the giving module, we'll use this for protection
 * @author liao_zhijun
 * @date 2010-07-20
 * @param module [in]: giving module
 * @return unsigned long new module
 */
unsigned long sysctl_get_share_module(E_DRV_MODULE module)
{
    signed long new_module = 0xFFFFFFFF;

    //check which sem need for the module
    switch(module)
    {
    case DRV_MODULE_I2S_TX:
    case DRV_MODULE_I2S_RX:
        //we use the same sem for I2S
        new_module = DRV_MODULE_I2S_TX;
        break;
        
    case DRV_MODULE_UVC:
    case DRV_MODULE_USB_DISK:
    case DRV_MODULE_USB_CDC:
    case DRV_MODULE_USB_CAMERA:
    case DRV_MODULE_USB_ANYKA:
    case DRV_MODULE_USB_CMMB:
    case DRV_MODULE_UDISK_HOST:
    case DRV_MODULE_UVC_HOST:
        //use the same sem for all usb module
        new_module = DRV_MODULE_USB_DISK;
        break;

    case DRV_MODULE_SDMMC:

    case DRV_MODULE_SPI:
        //sdmmc and nand and lcd share the same data  pin
        //check lcd refresh is finish or not
        //lcd_check_dma_finish();
        if(0)//(gpio_share_check_conflict(ePIN_AS_SDMMC2, ePIN_AS_LCD_MPU))
        {
            //new_module = DRV_MODULE_LCD;
        }
        else
        {            
            new_module = DRV_MODULE_SPI;
        }
        break;

    default:
        new_module = module;
    }

    return new_module;
}


