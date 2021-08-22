/**
* @FILENAME sysctl.h
*
* Copyright (C) 2006 Anyka (Guangzhou) Software Technology Co., LTD
* @DATE  2006-04-19
* @VERSION 1.0
* @REF 
*/

#ifndef __SYSCTL_H__
#define __SYSCTL_H__

/*以下定义的clock 模块必须独立，不能一个模块包含多个可控制clock的子模块*/
#define CLOCK_DEFAULT_ENABLE            (0)
#define CLOCK_MMCSD_ENABLE              (1<<1)
#define CLOCK_SDIO_ENABLE               (1<<2)
#define CLOCK_ADC2_ENABLE               (1<<3)
#define CLOCK_DAC_ENABLE                (1<<4)
#define CLOCK_SPI0_ENABLE               (1<<5)
#define CLOCK_SPI1_ENABLE               (1<<6)
#define CLOCK_UART0_ENABLE              (1<<7)
#define CLOCK_UART1_ENABLE              (1<<8)
#define CLOCK_L2_ENABLE                 (1<<9)
#define CLOCK_TWI_ENABLE                (1<<10)
#define CLOCK_GPIO_ENABLE               (1<<11)
#define CLOCK_MAC_ENABLE                (1<<12)
#define CLOCK_ENCRYPT_ENABLE            (1<<13)
#define CLOCK_USB_ENABLE                (1<<15)
#define CLOCK_CAMERA_ENABLE             (1<<15)
#define CLOCK_VIDEO_ENCODER_ENABLE      (1<<16)
#define CLOCK_DRAM_ENABLE               (1<<17)
#define CLOCK_NBITS                     (18)
#define CLOCK_ENABLE_MAX                (1<<CLOCK_NBITS)




/**clock ctrl1 0x0800000c bit map**/
#define CLOCK_CTRL_MCI1              (1<<1)
#define CLOCK_CTRL_MCI2               (1<<2)
#define CLOCK_CTRL_ADC2               (1<<3)
#define CLOCK_CTRL_DAC                (1<<4)
#define CLOCK_CTRL_SPI0               (1<<5)
#define CLOCK_CTRL_SPI1               (1<<6)
#define CLOCK_CTRL_UART0              (1<<7)
#define CLOCK_CTRL_UART1              (1<<8)
#define CLOCK_CTRL_L2                 (1<<9)
#define CLOCK_CTRL_TWI                (1<<10)
#define CLOCK_CTRL_GPIO               (1<<12)
#define CLOCK_CTRL_MAC                (1<<13)
#define CLOCK_CTRL_ENCRYPT            (1<<14)
#define CLOCK_CTRL_USBOTG             (1<<15)
#define CLOCK_CTRL_CAMERA             (1<<19)
#define CLOCK_CTRL_VIDEO_ENCODER      (1<<20)
#define CLOCK_CTRL_RAM               (1<<24)



#define RESET_SDMMC                     1
#define RESET_SDIO                      2
#define RESET_ADC2                      3
#define RESET_DAC                       4
#define RESET_SPI0                      5
#define RESET_SPI1                      6
#define RESET_UART0                     7
#define RESET_UART1                     8
#define RESET_L2                        9
#define RESET_TWI                       10
#define RESET_GPIO                      12
#define RESET_MAC                       13
#define RESET_ENCRYPT                   14
#define RESET_USB_OTG                   15
#define RESET_CAMERA                    19
#define RESET_ENCODER                   20
#define RESET_RAM                       24



/**
 * @BRIEF Set SleepMode
 * @AUTHOR guoshaofeng
 * @DATE 2007-04-23
 * @PARAM[in] unsigned long module
 * @RETURN void
 * @RETVAL
 * attention: if you close some parts such as LCD 
            you must init it again when you reopen 
            it 
            some settings may cause serious result
            better not to use it if not familar
 */
void sysctl_clock(unsigned long module);

/**
 * @brief get module clock states
 * @author LHS
 * @date 2011-10-26
 * @param module [in]: module to be get states
 * @return bool: return TURE mean clock is enable.
 */
bool sysctl_get_clock_state(unsigned long module);

/**
 * @brief reset module 
 * @author guoshaofeng
 * @date 2010-07-20
 * @param module [in]: module to be reset
 * @return void
 */
void sysctl_reset(unsigned long module);

#endif //__SYSCTL_H__

