/** @file
 * @brief Define the register of ANYKA CPU
 *
 * Define the register address and bit map for system.
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author xuchang
 * @date 2008-01-05
 * @version 1.0
 * @note
 * 该文件定义所有驱动模块的寄存器，不得在其他地方做寄存器定义!!
 * 寄存器的位定义原则上放在各个驱动模块的头文件中定义，如果对应的驱动模块太小没有头文件，
 * 则可放在此处定义
 */
#ifndef _ANYKA_CPU_H_
#define _ANYKA_CPU_H_

/** @defgroup ANYKA_CPU  
 *      @ingroup Drv_Lib
 */
/*@{*/


/** @{@name Base Address Define
 *      The base address of system memory space is define here. 
 *      Include memory assignment and module base address define.
 */
 /**Memory assignment*/
#define CHIP_CONF_BASE_ADDR                             0x08000000      // chip configurations
#define USB_BASE_ADDR                                   0x70000000      // USB
#define L2_BUF_MEM_BASE_ADDR                            0x48000000      //L2 Buffer start address
/** @} */


/** @{@name CPU working mode
 */
#define ANYKA_CPU_Mode_USR                              0x10
#define ANYKA_CPU_Mode_FIQ                              0x11
#define ANYKA_CPU_Mode_IRQ                              0x12
#define ANYKA_CPU_Mode_SVC                              0x13
#define ANYKA_CPU_Mode_ABT                              0x17
#define ANYKA_CPU_Mode_UNDEF                            0x1B
#define ANYKA_CPU_Mode_SYS                              0x1F            
#define ANYKA_CPU_I_Bit                                 0x80
#define ANYKA_CPU_F_Bit                                 0x40
/** @} */

/** @{@name System Control Register
 *      Define system control register here, include CLOCK/INT/RESET
 */
#define CLK_ASIC_PLL_CTRL				(CHIP_CONF_BASE_ADDR + 0x00000008)

#define CLOCK_CTRL_REG                                  (CHIP_CONF_BASE_ADDR + 0x0000001C)      // module clock control(switch)
#define CLOCK_CTRL2_REG                                 (CHIP_CONF_BASE_ADDR + 0x00000010)      // module clock control(switch)
#define RESET_CTRL_REG                                  (CHIP_CONF_BASE_ADDR + 0x0000000C)      // module software reset control register
#define RESET_CTRL2_REG                                 (CHIP_CONF_BASE_ADDR + 0x00000010)      // module clock control(switch)
#define IRQINT_MASK_REG                                 (CHIP_CONF_BASE_ADDR + 0x00000034)      // module IRQ interrupt mask register, 1: mask; 0:unmask(default);
#define FRQINT_MASK_REG                                 (CHIP_CONF_BASE_ADDR + 0x00000038)      // module FRQ interrupt mask register, 1: mask; 0:unmask(default);
#define INT_STATUS_REG                                  (CHIP_CONF_BASE_ADDR + 0x000000CC)      // module interrupt status register
#define INT_SYS_MODULE_REG                              (CHIP_CONF_BASE_ADDR + 0x0000004C)      // system module interrupt control register
#define CLOCK_DIV_REG                                   (CHIP_CONF_BASE_ADDR + 0x00000004)      // clock divider register 1
#define CLOCK2X_CTRL_REG                                (CHIP_CONF_BASE_ADDR + 0x00000004)      // clock2x control register,1: 2*ASIC clock; 0: ASIC clock
#define CLOCK3X_CTRL_REG                                (CHIP_CONF_BASE_ADDR + 0x00000064)      //bit28 => 1: asic = pll1_clock /3, 0: refresh to bit[6..8] of 0x08000004 register 
#define PLL_NPARAM_REG                                  (CHIP_CONF_BASE_ADDR + 0x000000dc)      // n configuration register,PLL_Clk = 4*M/N
/** @} */

#define SHARE_PIN_CFG_REG1								(CHIP_CONF_BASE_ADDR + 0x00000074) 
#define SHARE_PIN_CFG_REG2								(CHIP_CONF_BASE_ADDR + 0x00000078) 
#define SHARE_PIN_CFG_REG3								(CHIP_CONF_BASE_ADDR + 0x0000007C) 
#define SHARE_PIN_CFG_REG4								(CHIP_CONF_BASE_ADDR + 0x000000DC) 


/** @{@name Interrupt bit map define
 */ 
/** interrupt status register bit map*/
#define INT_STATUS_H264_BIT                             0x1
#define INT_STATUS_LCD_BIT                              (1 << 1)
#define INT_STATUS_CAMERA_BIT                           (1 << 2)
#define INT_STATUS_MOTION_BIT                           (1 << 3)
#define INT_STATUS_JPEG                                 (1 << 4)
#define INT_STATUS_MAC_BIT                              (1 << 5)
#define INT_STATUS_ROTATION_BIT                         (1 << 7)
#define INT_STATUS_DAC_BIT                              (1 << 8)
#define INT_STATUS_SIGDELTA_ADC_BIT                     (1 << 9)
#define INT_STATUS_L2_BIT                               (1 << 10)
#define INT_STATUS_ECC_BIT                              (1 << 11)
#define INT_STATUS_NANDFLASH_BIT                        (1 << 12)
#define INT_STATUS_UART4_BIT                            (1 << 13)
#define INT_STATUS_UART3_BIT                            (1 << 14)
#define INT_STATUS_UART2_BIT                            (1 << 15)
#define INT_STATUS_UART1_BIT                            (1 << 16)
#define INT_STATUS_SPI2_BIT                             (1 << 17)
#define INT_STATUS_SPI1_BIT                             (1 << 18)
#define INT_STATUS_I2C_BIT                              (1 << 19)
#define INT_STATUS_2D_ACCELE_BIT                        (1 << 20)
#define INT_STATUS_SDIO_BIT                             (1 << 21)
#define INT_STATUS_MMCSD_BIT                            (1 << 22)
#define INT_STATUS_USBFSMCU_BIT                         (1 << 23)
#define INT_STATUS_USBFSDMA_BIT                         (1 << 24)
#define INT_STATUS_USB_BIT                              (1 << 25)
#define INT_STATUS_USBDMA_BIT                           (1 << 26)
#define INT_STATUS_SYS_MODULE_BIT                       (1 << 27)
#define INT_STATUS_RMVB_BIT                             (1 << 28)
#define INT_STATUS_MPEG2_BIT                            (1 << 29)
#define INT_STATUS_HUFMAN_BIT                           (1 << 30)
#define INT_STATUS_PCM_BIT                              (1 << 31)
                                        
//level 2 interrupt status bit map      
#define INT_STATUS_TS_BIT                               (1 << 16)
#define INT_STATUS_TIMER5_BIT                           (1 << 17)
#define INT_STATUS_TIMER4_BIT                           (1 << 18)
#define INT_STATUS_TIMER3_BIT                           (1 << 19)
#define INT_STATUS_TIMER2_BIT                           (1 << 20)
#define INT_STATUS_TIMER1_BIT                           (1 << 21)
#define INT_STATUS_ASICCLK_BIT                          (1 << 22)
#define INT_STATUS_WGPIO_BIT                            (1 << 23)
#define INT_STATUS_RTC_READY_BIT                        (1 << 24)
#define INT_STATUS_RTC_ALARM_BIT                        (1 << 25)
#define INT_STATUS_GPIO_BIT                             (1 << 26)
#define INT_STATUS_RTC_BIT                              (1 << 27)
#define INT_STATUS_PENDOWN_BIT                          (1 << 28)


/* define the level1 interrupt valid bits */
#define INT_STATUS_NBITS                                27

/** IRQ interrupt mask register bit map*/
#define IRQ_MASK_H264_BIT                               (1 << 0)
#define IRQ_MASK_LCD_BIT                                (1 << 1)
#define IRQ_MASK_CAMERA_BIT                             (1 << 2)
#define IRQ_MASK_MOTION_BIT                             (1 << 3)
#define IRQ_MASK_JPEG                                   (1 << 4)
#define IRQ_MASK_MAC_BIT                                (1 << 5)
#define IRQ_MASK_ROTATION_BIT                           (1 << 7)
#define IRQ_MASK_DAC_BIT                                (1 << 8)
#define IRQ_MASK_SIGDELTA_ADC_BIT                       (1 << 9)
#define IRQ_MASK_L2_BIT                                 (1 << 10)
#define IRQ_MASK_ECC_BIT                                (1 << 11)
#define IRQ_MASK_NANDFLASH_BIT                          (1 << 12)
#define IRQ_MASK_UART4_BIT                              (1 << 13)
#define IRQ_MASK_UART3_BIT                              (1 << 14)
#define IRQ_MASK_UART2_BIT                              (1 << 15)
#define IRQ_MASK_UART1_BIT                              (1 << 16)
#define IRQ_MASK_SPI2_BIT                               (1 << 17)
#define IRQ_MASK_SPI1_BIT                               (1 << 18)
#define IRQ_MASK_I2C_BIT                                (1 << 19)
#define IRQ_MASK_2D_ACCELE_BIT                          (1 << 20)
#define IRQ_MASK_SDIO_BIT                               (1 << 21)
#define IRQ_MASK_MMCSD_BIT                              (1 << 22)
#define IRQ_MASK_USBFSMCU_BIT                           (1 << 23)
#define IRQ_MASK_USBFSDMA_BIT                           (1 << 24)
#define IRQ_MASK_USB_BIT                                (1 << 25)
#define IRQ_MASK_USBDMA_BIT                             (1 << 26)
#define IRQ_MASK_SYS_MODULE_BIT                         (1 << 27)
#define IRQ_MASK_RMVB_BIT                               (1 << 28)
#define IRQ_MASK_MPEG2_BIT                              (1 << 29)
#define IRQ_MASK_HUFMAN_BIT                             (1 << 30)
#define IRQ_MASK_PCM_BIT                                (1 << 31)
                                                        
//level 2 interrupt mask bit map                        
#define IRQ_MASK_TS_BIT                                 (1 << 0)
#define IRQ_MASK_TIMER5_BIT                             (1 << 1)
#define IRQ_MASK_TIMER4_BIT                             (1 << 2)
#define IRQ_MASK_TIMER3_BIT                             (1 << 3)
#define IRQ_MASK_TIMER2_BIT                             (1 << 4)
#define IRQ_MASK_TIMER1_BIT                             (1 << 5)
#define IRQ_MASK_ASICCLK_BIT                            (1 << 6)
#define IRQ_MASK_WGPIO_BIT                              (1 << 7)
#define IRQ_MASK_RTC_READY_BIT                          (1 << 8)
#define IRQ_MASK_RTC_ALARM_BIT                          (1 << 9)
#define IRQ_MASK_GPIO_BIT                               (1 << 10)
#define IRQ_MASK_RTC_BIT                                (1 << 11)
#define IRQ_MASK_PENDOWN_BIT                            (1 << 12) 
/** @} */



/** @{@name SDRAM&DMA register and bit map define
 */ 
#define DMA_CTRL_BASE_ADDR                              (0x2002d000)
#define DMA_PRIORITY_CTRL_REG1                          (DMA_CTRL_BASE_ADDR + 0x0000000c)
#define DMA_PRIORITY_CTRL_REG2                          (DMA_CTRL_BASE_ADDR + 0x00000010)
#define AHB_PRIORITY_CTRL_REG                           (DMA_CTRL_BASE_ADDR + 0x00000014)
#define SDRAM_CFG_REG1                                  (DMA_CTRL_BASE_ADDR + 0x0)
#define SDRAM_CFG_REG2                                  (DMA_CTRL_BASE_ADDR + 0x4)
#define SDRAM_CFG_REG3                                  (DMA_CTRL_BASE_ADDR + 0x8)
/** @} */


/** @} */

/** @{@name L2 memory register and bit map define
 */ 
#define L2_BASE_ADDR                                    0x20140000
#define L2_DMA_ADDR                                     (L2_BASE_ADDR+0x00)
#define L2_DMA_CNT                                      (L2_BASE_ADDR+0x40)
#define L2_DMA_REQ                                      (L2_BASE_ADDR+0x80)
#define L2_FRAC_ADDR                                    (L2_BASE_ADDR+0x84)
#define L2_COMBUF_CFG                                   (L2_BASE_ADDR+0x88)
#define L2_UARTBUF_CFG                                  (L2_BASE_ADDR+0x8c)
#define L2_ASSIGN_REG1                                  (L2_BASE_ADDR+0x90)
#define L2_ASSIGN_REG2                                  (L2_BASE_ADDR+0x94)
#define L2_LDMA_CFG                                     (L2_BASE_ADDR+0x98)
#define L2_INT_ENA                                      (L2_BASE_ADDR+0x9c)
#define L2_STAT_REG1                                    (L2_BASE_ADDR+0xa0)
#define L2_STAT_REG2                                    (L2_BASE_ADDR+0xa8)


/** @} */

/** @{@name L2 cache register and bit map define
 */ 
#define L2_CACHE_BASE_ADDR                              0x2000f000
#define L2_CACHE_CONFIG_REG                             (L2_CACHE_BASE_ADDR+0x0000)
#define L2_SEC0_START_ADDR_REG                          (L2_CACHE_BASE_ADDR+0x0010)
#define L2_SEC0_END_ADDR_REG                            (L2_CACHE_BASE_ADDR+0x0014)
#define L2_SEC1_START_ADDR_REG                          (L2_CACHE_BASE_ADDR+0x0020)
#define L2_SEC1_END_ADDR_REG                            (L2_CACHE_BASE_ADDR+0x0024)
#define L2_SEC2_START_ADDR_REG                          (L2_CACHE_BASE_ADDR+0x0030)
#define L2_SEC2_END_ADDR_REG                            (L2_CACHE_BASE_ADDR+0x0034)
#define L2_SEC3_START_ADDR_REG                          (L2_CACHE_BASE_ADDR+0x0040)
#define L2_SEC3_END_ADDR_REG                            (L2_CACHE_BASE_ADDR+0x0044)
#define L2_SEC4_START_ADDR_REG                          (L2_CACHE_BASE_ADDR+0x0050)
#define L2_SEC4_END_ADDR_REG                            (L2_CACHE_BASE_ADDR+0x0054)
#define L2_SEC5_START_ADDR_REG                          (L2_CACHE_BASE_ADDR+0x0060)
#define L2_SEC5_END_ADDR_REG                            (L2_CACHE_BASE_ADDR+0x0064)
#define L2_SEC6_START_ADDR_REG                          (L2_CACHE_BASE_ADDR+0x0070)
#define L2_SEC6_END_ADDR_REG                            (L2_CACHE_BASE_ADDR+0x0074)
#define L2_SEC7_START_ADDR_REG                          (L2_CACHE_BASE_ADDR+0x0080)
#define L2_SEC7_END_ADDR_REG                            (L2_CACHE_BASE_ADDR+0x0084)
/** @} */

/** @{@name Nandflash ECC Controller Define 
 *      Define the register and bit map of nandflash controller
 */
// ECC
#define FLASH_ECC_REG0                                  0x2002b000
#define FLASH_ECC_REPAIR_REG0                           (FLASH_ECC_REG0+0x4)
// NF controller
#define FLASH_CTRL_REG0                                 (0x2002a100)
#define FLASH_CTRL_REG20                                (FLASH_CTRL_REG0+0x50)
#define FLASH_CTRL_REG21                                (FLASH_CTRL_REG0+0x54)
#define FLASH_CTRL_REG22                                (FLASH_CTRL_REG0+0x58)
#define FLASH_CTRL_REG23                                (FLASH_CTRL_REG0+0x5c)
#define FLASH_CTRL_REG24                                (FLASH_CTRL_REG0+0x60)
/** @} */


/** @{@name RTC module register and bit map define
 */
#define RTC_MODULE_BASE_ADDR                            0x08000000      // RTC
#define RTC_CONFIG_REG                                  (RTC_MODULE_BASE_ADDR + 0x50)   // rtc confgiuration
#define RTC_BACK_DAT_REG                                (RTC_MODULE_BASE_ADDR + 0x54)   // rtc read back data
#define RTC_CLOCK_DIV_REG                               (RTC_MODULE_BASE_ADDR + 0x04)   // enable/disable wakeup function
#define RTC_WAKEUP_GPIO_P_REG                           (RTC_MODULE_BASE_ADDR + 0x3C)   // wakeup GPIO polarity
#define RTC_WAKEUP_GPIO_C_REG                           (RTC_MODULE_BASE_ADDR + 0x40)   // clear wakeup GPIO status
#define RTC_WAKEUP_GPIO_E_REG                           (RTC_MODULE_BASE_ADDR + 0x44)   // enable wake-up GPIO wakeup function
#define RTC_WAKEUP_GPIO_S_REG                           (RTC_MODULE_BASE_ADDR + 0x48)   // wakeup GPIO status
/** @} */


/** @{@name LCD module register and bit map define
 */ 
#define LCD_MODULE_BASE_ADDR                            0x2000D000
#define LCD_TOP_CONFIGURE_REG                           (LCD_MODULE_BASE_ADDR | 0x0000)
#define LCD_MPU_CTL_REG                                 (LCD_MODULE_BASE_ADDR | 0x0004)
#define LCD_RST_PIN_REG                                 (LCD_MODULE_BASE_ADDR | 0x0008)
#define LCD_MPU_READ_REG                                (LCD_MODULE_BASE_ADDR | 0x000C)
#define LCD_RGB_CTL_REG1                                (LCD_MODULE_BASE_ADDR | 0x0010)
#define LCD_RGB_CTL_REG2                                (LCD_MODULE_BASE_ADDR | 0x0014)
#define LCD_RGB_VIRTUAL_SIZE_REG                        (LCD_MODULE_BASE_ADDR | 0x0018)
#define LCD_RGB_VIRTUAL_OFFSET_REG                      (LCD_MODULE_BASE_ADDR | 0x001C)

#define LCD_OSD_ADDR_REG                                (LCD_MODULE_BASE_ADDR | 0x20)
#define LCD_OSD_OFFSET_REG                              (LCD_MODULE_BASE_ADDR | 0x24)
#define LCD_OSD_F_COLOR1_REG                            (LCD_MODULE_BASE_ADDR | 0x28)
#define LCD_OSD_F_COLOR2_REG                            (LCD_MODULE_BASE_ADDR | 0x2C)
#define LCD_OSD_F_COLOR3_REG                            (LCD_MODULE_BASE_ADDR | 0x30)
#define LCD_OSD_F_COLOR4_REG                            (LCD_MODULE_BASE_ADDR | 0x34)
#define LCD_OSD_F_COLOR5_REG                            (LCD_MODULE_BASE_ADDR | 0xd0)
#define LCD_OSD_F_COLOR6_REG                            (LCD_MODULE_BASE_ADDR | 0xd4)
#define LCD_OSD_F_COLOR7_REG                            (LCD_MODULE_BASE_ADDR | 0xd8)
#define LCD_OSD_F_COLOR8_REG                            (LCD_MODULE_BASE_ADDR | 0xdc)
#define LCD_OSD_SIZE_ALPHA_REG                          (LCD_MODULE_BASE_ADDR | 0x38)

#define LCD_GRB_BACKGROUND_REG                          (LCD_MODULE_BASE_ADDR | 0x003c)
#define LCD_RGB_CTL_REG3                                (LCD_MODULE_BASE_ADDR | 0x0040)
#define LCD_RGB_CTL_REG4                                (LCD_MODULE_BASE_ADDR | 0x0044)
#define LCD_RGB_CTL_REG5                                (LCD_MODULE_BASE_ADDR | 0x0048)
#define LCD_RGB_CTL_REG6                                (LCD_MODULE_BASE_ADDR | 0x004C)
#define LCD_RGB_CTL_REG7                                (LCD_MODULE_BASE_ADDR | 0x0050)
#define LCD_RGB_CTL_REG8                                (LCD_MODULE_BASE_ADDR | 0x0054)
#define LCD_RGB_CTL_REG9                                (LCD_MODULE_BASE_ADDR | 0x0058)

#define LCD_Y1_ADDR_REG                                 (LCD_MODULE_BASE_ADDR | 0x005c)
#define LCD_U1_ADDR_REG                                 (LCD_MODULE_BASE_ADDR | 0x0060) 
#define LCD_V1_ADDR_REG                                 (LCD_MODULE_BASE_ADDR | 0x0064) 
#define LCD_YUV1_H_INFO_REG                             (LCD_MODULE_BASE_ADDR | 0x0068) 
#define LCD_YUV1_V_INFO_REG                             (LCD_MODULE_BASE_ADDR | 0x006c) 
#define LCD_YUV1_VIR_SIZE_REG                           (LCD_MODULE_BASE_ADDR | 0x0078) 
#define LCD_YUV1_VIR_OFFSET_REG                         (LCD_MODULE_BASE_ADDR | 0x007c) 
#define LCD_YUV1_SCALER_INFO_REG                        (LCD_MODULE_BASE_ADDR | 0x0070) 
#define LCD_YUV1_DISPLAY_INFO_REG                       (LCD_MODULE_BASE_ADDR | 0x0074)

#define LCD_Y2_ADDR_REG                                 (LCD_MODULE_BASE_ADDR | 0x0080)
#define LCD_U2_ADDR_REG                                 (LCD_MODULE_BASE_ADDR | 0x0084) 
#define LCD_V2_ADDR_REG                                 (LCD_MODULE_BASE_ADDR | 0x0088) 
#define LCD_YUV2_H_INFO_REG                             (LCD_MODULE_BASE_ADDR | 0x008c) 
#define LCD_YUV2_V_INFO_REG                             (LCD_MODULE_BASE_ADDR | 0x0090) 
#define LCD_YUV2_SCALER_INFO_REG                        (LCD_MODULE_BASE_ADDR | 0x0094) 
#define LCD_YUV2_DISPLAY_INFO_REG                       (LCD_MODULE_BASE_ADDR | 0x0098)

#define LCD_RGB_OFFSET_REG                              (LCD_MODULE_BASE_ADDR | 0x00A8)
#define LCD_RGB_SIZE_REG                                (LCD_MODULE_BASE_ADDR | 0x00AC)
#define LCD_PANEL_SIZE_REG                              (LCD_MODULE_BASE_ADDR | 0x00B0)
#define LCD_REG_CONFIG_REG                              (LCD_MODULE_BASE_ADDR | 0x00B4)
#define LCD_LCD_GO_REG                                  (LCD_MODULE_BASE_ADDR | 0x00B8)
#define LCD_LCD_STATUS                                  (LCD_MODULE_BASE_ADDR | 0x00BC)
#define LCD_LCD_INTERRUPT_MASK                          (LCD_MODULE_BASE_ADDR | 0x00C0)
#define LCD_SOFTWARE_CTL_REG                            (LCD_MODULE_BASE_ADDR | 0x00C8)
#define LCD_TVOUT_CTL_REG                               (LCD_MODULE_BASE_ADDR | 0x00CC)
#define LCD_CLK_CTL_REG                                 (LCD_MODULE_BASE_ADDR | 0x00E8)

#define LCD_CHROMA_FRQ_CTR_REG                          (LCD_MODULE_BASE_ADDR | 0x0100)
/*TVOUT new register added for aspen3s*/                
#define TVOUT_CTRL_REG1                                 (LCD_MODULE_BASE_ADDR | 0x0104)
#define TVOUT_PARA_CONFIG_REG1                          (LCD_MODULE_BASE_ADDR | 0x0108)
#define TVOUT_PARA_CONFIG_REG2                          (LCD_MODULE_BASE_ADDR | 0x010c)
#define TVOUT_PARA_CONFIG_REG3                          (LCD_MODULE_BASE_ADDR | 0x0110)
#define TVOUT_PARA_CONFIG_REG4                          (LCD_MODULE_BASE_ADDR | 0x0114)
#define TVOUT_PARA_CONFIG_REG5                          (LCD_MODULE_BASE_ADDR | 0x0118)
#define TVOUT_PARA_CONFIG_REG6                          (LCD_MODULE_BASE_ADDR | 0x011c)

/** @} */


/** @{@name GUI module register and bit map define
 */
#define GUI_BASE_ADDR                                   0x20022000
#define GUI_SCALSRCADDR1_ADDR                           (GUI_BASE_ADDR+0x108)   // Input image start address 1
#define GUI_SCALSRCADDR2_ADDR                           (GUI_BASE_ADDR+0x10c) // Input image start address 2
#define GUI_SCALSRCADDR3_ADDR                           (GUI_BASE_ADDR+0x110) // Input image start address 3
#define GUI_SCALSRCSTRD_ADDR                            (GUI_BASE_ADDR+0x118)   // Input image line stride
#define GUI_SCALDSTADDR_ADDR                            (GUI_BASE_ADDR+0x11c)   // Output image start address
#define GUI_SCALSRCRECT_ADDR                            (GUI_BASE_ADDR+0x114)   // Input image rectangle dimensions
#define GUI_SCALDSTRECT_ADDR                            (GUI_BASE_ADDR+0x120)   // Output image rectangle dimensions
#define GUI_SCALDSTSTRD_ADDR                            (GUI_BASE_ADDR+0x124)   // Output image line stride
#define GUI_SCALRATIO_ADDR                              (GUI_BASE_ADDR+0x104)   // Scaling parameters, scale=8192/ILX[8:0]
#define GUI_POINT1_ADDR                                 (GUI_BASE_ADDR+0x0c)    // Destination Offset
#define GUI_CMD_ADDR                                    (GUI_BASE_ADDR+0x04)    // Command
#define GUI_SCALCTRL_ADDR                               (GUI_BASE_ADDR+0x100)   // Color Space conversion and scaling control
#define GUI_SCALOFFSET_ADDR                             (GUI_BASE_ADDR+0x128)   // Output image line stride
/** @} */

/** @{@name IMAGE sensor module register and bit map define
 */
#define IMAGE_MODULE_BASE_ADDR                          0x20030000      // image sensor
#define IMG_CMD_ADDR                                    (IMAGE_MODULE_BASE_ADDR | 0x0000)
#define IMG_HINFO1_ADDR                                 (IMAGE_MODULE_BASE_ADDR | 0x0004)
#define IMG_HINFO2_ADDR                                 (IMAGE_MODULE_BASE_ADDR | 0x0008)
#define IMG_VINFO1_ADDR                                 (IMAGE_MODULE_BASE_ADDR | 0x000C)
#define IMG_VINFO2_ADDR                                 (IMAGE_MODULE_BASE_ADDR | 0x0010)
                                                        
#define IMG_YADDR_ODD                                   (IMAGE_MODULE_BASE_ADDR | 0x0018)
#define IMG_UADDR_ODD                                   (IMAGE_MODULE_BASE_ADDR | 0x001c)
#define IMG_VADDR_ODD                                   (IMAGE_MODULE_BASE_ADDR | 0x0020)
#define IMG_RGBADDR_ODD                                 (IMAGE_MODULE_BASE_ADDR | 0x0024)
#define IMG_YADDR_EVE                                   (IMAGE_MODULE_BASE_ADDR | 0x0028)
#define IMG_UADDR_EVE                                   (IMAGE_MODULE_BASE_ADDR | 0x002c)
#define IMG_VADDR_EVE                                   (IMAGE_MODULE_BASE_ADDR | 0x0030)
#define IMG_RGBADDR_EVE                                 (IMAGE_MODULE_BASE_ADDR | 0x0034)
#define IMG_CONFIG_ADDR                                 (IMAGE_MODULE_BASE_ADDR | 0x0040)
#define IMG_STATUS_ADDR                                 (IMAGE_MODULE_BASE_ADDR | 0x0060)
#define IMG_NUM_ADDR                                    (IMAGE_MODULE_BASE_ADDR | 0x0080)

#define MUL_FUN_CTL_REG                                 (CHIP_CONF_BASE_ADDR | 0x0058)
/** @} */


/** @{@name UART module register and bit map define
 */
#define UART_MODULE_BASE_ADDR                           0x20026000      // UART
#define UART0_BASE_ADDR                                 (UART_MODULE_BASE_ADDR + 0x00000000)
#define UART1_BASE_ADDR                                 (UART_MODULE_BASE_ADDR + 0x00001000)
#define UART2_BASE_ADDR                                 (UART_MODULE_BASE_ADDR + 0x00002000)
#define UART3_BASE_ADDR                                 (UART_MODULE_BASE_ADDR + 0x00003000)
                                                        
#define UART_CFG_REG1                                   (0x00)
#define UART_CFG_REG2                                   (0x04)
#define UART_DATA_CFG                                   (0x08)
#define UART_RX_THREINT                                 (0x0c)
/** @} */

/** @{@name SPI module register and bit map define
 */
#define SPI_MODULE_BASE_ADDR                            0x20120000      // SPI
#define SPI0_BASE_ADDR                                  (SPI_MODULE_BASE_ADDR + 0x00000000)
#define SPI1_BASE_ADDR                                  (SPI_MODULE_BASE_ADDR + 0x00005000)
                                                        
#define ASPEN_SPI_CTRL                                  (0x00)
#define ASPEN_SPI_STA                                   (0x04)
#define ASPEN_SPI_INTENA                                (0x08)
#define ASPEN_SPI_NBR                                   (0x0c)
#define ASPEN_SPI_TX_EXBUF                              (0x10)
#define ASPEN_SPI_RX_EXBUF                              (0x14)
#define ASPEN_SPI_TX_INBUF                              (0x18)
#define ASPEN_SPI_RX_INBUF                              (0x1c)
#define ASPEN_SPI_RTIM                                  (0x20)
/** @} */

/** @{@name USB register and bit map define
 *  Define register to control USB port and the bit map of the register
 */
#define USB_CONTROL_REG                                 (0x08000058)
#define USB_FIFO_EP0                                    (USB_BASE_ADDR + 0x0020)
#define USB_FIFO_EP1                                    (USB_BASE_ADDR + 0x0024)
#define USB_FIFO_EP2                                    (USB_BASE_ADDR + 0x0028)
#define USB_FIFO_EP3                                    (USB_BASE_ADDR + 0x002C)
#define USB_REG_FADDR                                   (USB_BASE_ADDR + 0x0000)
#define USB_REG_POWER                                   (USB_BASE_ADDR + 0x0001)
#define USB_REG_INTRTX1                                 (USB_BASE_ADDR + 0x0002)
#define USB_REG_INTRTX2                                 (USB_BASE_ADDR + 0x0003)
#define USB_REG_INTRRX1                                 (USB_BASE_ADDR + 0x0004)
#define USB_REG_INTRRX2                                 (USB_BASE_ADDR + 0x0005)
#define USB_REG_INTRTX1E                                (USB_BASE_ADDR + 0x0006)
#define USB_REG_INTRTX2E                                (USB_BASE_ADDR + 0x0007)
#define USB_REG_INTRRX1E                                (USB_BASE_ADDR + 0x0008)
#define USB_REG_INTRRX2E                                (USB_BASE_ADDR + 0x0009)
#define USB_REG_INTRUSB                                 (USB_BASE_ADDR + 0x000A)
#define USB_REG_INTRUSBE                                (USB_BASE_ADDR + 0x000B)
#define USB_REG_FRAME1                                  (USB_BASE_ADDR + 0x000C)
#define USB_REG_FRAME2                                  (USB_BASE_ADDR + 0x000D)
#define USB_REG_INDEX                                   (USB_BASE_ADDR + 0x000E)
#define USB_REG_TESEMODE                                (USB_BASE_ADDR + 0x000F)
#define USB_REG_DEVCTL                                  (USB_BASE_ADDR + 0x0060)
#define USB_REG_TXMAXP0                                 (USB_BASE_ADDR + 0x0010)
#define USB_REG_TXMAXP1                                 (USB_BASE_ADDR + 0x0010)
#define USB_REG_CSR0                                    (USB_BASE_ADDR + 0x0012)
#define USB_REG_TXCSR1                                  (USB_BASE_ADDR + 0x0012)
#define USB_REG_CSR02                                   (USB_BASE_ADDR + 0x0013)
#define USB_REG_TXCSR2                                  (USB_BASE_ADDR + 0x0013)
#define USB_REG_RXMAXP1                                 (USB_BASE_ADDR + 0x0014)
#define USB_REG_RXMAXP2                                 (USB_BASE_ADDR + 0x0015)
#define USB_REG_RXCSR1                                  (USB_BASE_ADDR + 0x0016)
#define USB_REG_RXCSR2                                  (USB_BASE_ADDR + 0x0017)
#define USB_REG_COUNT0                                  (USB_BASE_ADDR + 0x0018)
#define USB_REG_RXCOUNT1                                (USB_BASE_ADDR + 0x0018)
#define USB_REG_RXCOUNT2                                (USB_BASE_ADDR + 0x0019)
#define USB_REG_TXTYPE                                  (USB_BASE_ADDR + 0x001A)
#define USB_REG_RXTYPE                                  (USB_BASE_ADDR + 0x001C)
#define USB_REG_RXINTERVAL                              (USB_BASE_ADDR + 0x001D)
#define USB_REG_NAKLIMIT0                               (USB_BASE_ADDR + 0x001B)
                                                        
#define USB_EP0_TX_COUNT                                (USB_BASE_ADDR + 0x0330)
#define USB_EP2_TX_COUNT                                (USB_BASE_ADDR + 0x0334)
                                                        
#define USB_FORBID_WRITE_REG                            (USB_BASE_ADDR + 0x0338)
                                                        
#define USB_START_PRE_READ_REG                          (USB_BASE_ADDR + 0x033C)
#define USB_FS_SPEED_REG                                (USB_BASE_ADDR + 0x0344)

#define USB_FS_HOST_BASE_ADDR                           0x70000800
#define USB_FS_HOST_REG_FADDR                           (USB_FS_HOST_BASE_ADDR + 0x0000)
#define USB_FS_HOST_REG_POWER                           (USB_FS_HOST_BASE_ADDR + 0x0001)
#define USB_FS_HOST_REG_INTRTX1                         (USB_FS_HOST_BASE_ADDR + 0x0002)
#define USB_FS_HOST_REG_INTRTX2                         (USB_FS_HOST_BASE_ADDR + 0x0003)
#define USB_FS_HOST_REG_INTRRX1                         (USB_FS_HOST_BASE_ADDR + 0x0004)
#define USB_FS_HOST_REG_INTRRX2                         (USB_FS_HOST_BASE_ADDR + 0x0005)
#define USB_FS_HOST_REG_INTRTX1E                        (USB_FS_HOST_BASE_ADDR + 0x0006)
#define USB_FS_HOST_REG_INTRTX2E                        (USB_FS_HOST_BASE_ADDR + 0x0007)
#define USB_FS_HOST_REG_INTRRX1E                        (USB_FS_HOST_BASE_ADDR + 0x0008)
#define USB_FS_HOST_REG_INTRRX2E                        (USB_FS_HOST_BASE_ADDR + 0x0009)
#define USB_FS_HOST_REG_INTRUSB                         (USB_FS_HOST_BASE_ADDR + 0x000A)
#define USB_FS_HOST_REG_INTRUSBE                        (USB_FS_HOST_BASE_ADDR + 0x000B)
#define USB_FS_HOST_REG_FRAME1                          (USB_FS_HOST_BASE_ADDR + 0x000C)
#define USB_FS_HOST_REG_FRAME2                          (USB_FS_HOST_BASE_ADDR + 0x000D)
#define USB_FS_HOST_REG_INDEX                           (USB_FS_HOST_BASE_ADDR + 0x000E)
#define USB_FS_HOST_REG_TESEMODE                        (USB_FS_HOST_BASE_ADDR + 0x000F)
#define USB_FS_HOST_REG_DEVCTL                          (USB_FS_HOST_BASE_ADDR + 0x0060)
#define USB_FS_HOST_REG_TXMAXP0                         (USB_FS_HOST_BASE_ADDR + 0x0010)
#define USB_FS_HOST_REG_TXMAXP1                         (USB_FS_HOST_BASE_ADDR + 0x0010)
#define USB_FS_HOST_REG_CSR0                            (USB_FS_HOST_BASE_ADDR + 0x0012)
#define USB_FS_HOST_REG_TXCSR1                          (USB_FS_HOST_BASE_ADDR + 0x0012)
#define USB_FS_HOST_REG_CSR02                           (USB_FS_HOST_BASE_ADDR + 0x0013)
#define USB_FS_HOST_REG_TXCSR2                          (USB_FS_HOST_BASE_ADDR + 0x0013)
#define USB_FS_HOST_REG_RXMAXP1                         (USB_FS_HOST_BASE_ADDR + 0x0014)
#define USB_FS_HOST_REG_RXMAXP2                         (USB_FS_HOST_BASE_ADDR + 0x0015)
#define USB_FS_HOST_REG_RXCSR1                          (USB_FS_HOST_BASE_ADDR + 0x0016)
#define USB_FS_HOST_REG_RXCSR2                          (USB_FS_HOST_BASE_ADDR + 0x0017)
#define USB_FS_HOST_REG_COUNT0                          (USB_FS_HOST_BASE_ADDR + 0x0018)
#define USB_FS_HOST_REG_RXCOUNT1                        (USB_FS_HOST_BASE_ADDR + 0x0018)
#define USB_FS_HOST_REG_RXCOUNT2                        (USB_FS_HOST_BASE_ADDR + 0x0019)
#define USB_FS_HOST_REG_TXTYPE                          (USB_FS_HOST_BASE_ADDR + 0x001A)
#define USB_FS_HOST_REG_RXTYPE                          (USB_FS_HOST_BASE_ADDR + 0x001C)
#define USB_FS_HOST_REG_RXINTERVAL                      (USB_FS_HOST_BASE_ADDR + 0x001D)
#define USB_FS_HOST_REG_NAKLIMIT0                       (USB_FS_HOST_BASE_ADDR + 0x001B)
#define USB_FS_HOST_REG_FIFOSIZE                        (USB_FS_HOST_BASE_ADDR + 0x001F)

#define USB_L2_CONTROL_FIFO                             (0x48001500)
                                                        
#define USB_FS_FIFO_EP0                                 0x70000820
#define USB_FS_FIFO_EP1                                 0x70000824
#define USB_FS_FIFO_EP2                                 0x70000828
#define USB_FS_FIFO_EP3                                 0x7000082c 
                                                        
/**  USB DMA */                         
#define USB_FS_HOST_DMA_INTR                            (USB_FS_HOST_BASE_ADDR + 0x0200)
#define USB_FS_HOST_DMA_CNTL_1                          (USB_FS_HOST_BASE_ADDR + 0x0204)
#define USB_FS_HOST_DMA_ADDR_1                          (USB_FS_HOST_BASE_ADDR + 0x0208)
#define USB_FS_HOST_DMA_COUNT_1                         (USB_FS_HOST_BASE_ADDR + 0x020c)
#define USB_FS_HOST_DMA_CNTL_2                          (USB_FS_HOST_BASE_ADDR + 0x0214)
#define USB_FS_HOST_DMA_ADDR_2                          (USB_FS_HOST_BASE_ADDR + 0x0218)
#define USB_FS_HOST_DMA_COUNT_2                         (USB_FS_HOST_BASE_ADDR + 0x021c)

/**Dynamic FIFO sizing   JUST FOR HOST  */
#define USB_REG_TXFIFO1                                 (USB_BASE_ADDR + 0x001C)
#define USB_REG_TXFIFO2                                 (USB_BASE_ADDR + 0x001D)
#define USB_REG_RXFIFO1                                 (USB_BASE_ADDR + 0x001E)
#define USB_REG_RXFIFO2                                 (USB_BASE_ADDR + 0x001F)
                                                        
#define USB_REG_FIFOSIZE                                (USB_BASE_ADDR + 0x001F)
                                                        
/**  USB DMA */                                         
#define USB_DMA_INTR                                    (USB_BASE_ADDR + 0x0200)
#define USB_DMA_CNTL_1                                  (USB_BASE_ADDR + 0x0204)
#define USB_DMA_ADDR_1                                  (USB_BASE_ADDR + 0x0208)
#define USB_DMA_COUNT_1                                 (USB_BASE_ADDR + 0x020c)
#define USB_DMA_CNTL_2                                  (USB_BASE_ADDR + 0x0214)
#define USB_DMA_ADDR_2                                  (USB_BASE_ADDR + 0x0218)
#define USB_DMA_COUNT_2                                 (USB_BASE_ADDR + 0x021c)                                 
#define USB_FS_SPEED_REG                                (USB_BASE_ADDR + 0x0344)
/** @} */


/** @{@name SDMMC/SDIO module register and bit map define
 */
#define SD_MMC_BASE_ADDR                                0x20020000       /*mmc_sd interface*/
#define SDIO_BASE_ADDR                                  0x20021000       /*sdio interface*/
#define SD_CLK_CTRL_REG                                 ( SD_MMC_BASE_ADDR + 0x04  )
#define SD_ARGUMENT_REG                                 ( SD_MMC_BASE_ADDR + 0x08  )
#define SD_CMD_REG                                      ( SD_MMC_BASE_ADDR + 0x0C  )
#define SD_RESP_CMD_REG                                 ( SD_MMC_BASE_ADDR + 0x10  )
#define SD_RESP_REG0                                    ( SD_MMC_BASE_ADDR + 0x14  )
#define SD_RESP_REG1                                    ( SD_MMC_BASE_ADDR + 0x18  )
#define SD_RESP_REG2                                    ( SD_MMC_BASE_ADDR + 0x1c  )
#define SD_RESP_REG3                                    ( SD_MMC_BASE_ADDR + 0x20  )
#define SD_DATA_TIM_REG                                 ( SD_MMC_BASE_ADDR + 0x24  )
#define SD_DATA_LEN_REG                                 ( SD_MMC_BASE_ADDR + 0x28  )
#define SD_DATA_CTRL_REG                                ( SD_MMC_BASE_ADDR + 0x2C  )
#define SD_DATA_COUT_REG                                ( SD_MMC_BASE_ADDR + 0x30  )
#define SD_INT_STAT_REG                                 ( SD_MMC_BASE_ADDR + 0x34  )
#define SD_INT_ENABLE                                   ( SD_MMC_BASE_ADDR + 0x38  )
#define SD_DMA_MODE_REG                                 ( SD_MMC_BASE_ADDR + 0x3C  )
#define SD_CPU_MODE_REG                                 ( SD_MMC_BASE_ADDR + 0x40  )
                                                        
#define SDIO_CLK_CTRL_REG                               ( SDIO_BASE_ADDR + 0x04  )
#define SDIO_ARGUMENT_REG                               ( SDIO_BASE_ADDR + 0x08  )
#define SDIO_CMD_REG                                    ( SDIO_BASE_ADDR + 0x0C  )
#define SDIO_RESP_CMD_REG                               ( SDIO_BASE_ADDR + 0x10  )
#define SDIO_RESP_REG0                                  ( SDIO_BASE_ADDR + 0x14  )
#define SDIO_RESP_REG1                                  ( SDIO_BASE_ADDR + 0x18  )
#define SDIO_RESP_REG2                                  ( SDIO_BASE_ADDR + 0x1c  )
#define SDIO_RESP_REG3                                  ( SDIO_BASE_ADDR + 0x20  )
#define SDIO_DATA_TIM_REG                               ( SDIO_BASE_ADDR + 0x24  )
#define SDIO_DATA_LEN_REG                               ( SDIO_BASE_ADDR + 0x28  )
#define SDIO_DATA_CTRL_REG                              ( SDIO_BASE_ADDR + 0x2C  )
#define SDIO_DATA_COUT_REG                              ( SDIO_BASE_ADDR + 0x30  )
#define SDIO_INT_STAT_REG                               ( SDIO_BASE_ADDR + 0x34  )
#define SDIO_INT_ENABLE                                 ( SDIO_BASE_ADDR + 0x38  )
#define SDIO_DMA_MODE_REG                               ( SDIO_BASE_ADDR + 0x3C  )
#define SDIO_CPU_MODE_REG                               ( SDIO_BASE_ADDR + 0x40  )
/** @} */


/** @{@name ANALOG module register and bit map define
 */
#define ADC_MODULE_BASE_ADDR                            0x08000000      // Analog 
/*** for aspen 3s */
#define ANALOG_CTRL_REG1                                (ADC_MODULE_BASE_ADDR + 0x130)
#define ANALOG_CTRL_REG2                                (ADC_MODULE_BASE_ADDR + 0x134)
#define ANALOG_CTRL_REG3                                (ADC_MODULE_BASE_ADDR + 0x5c)
#define ANALOG_CTRL_REG4                                (ADC_MODULE_BASE_ADDR + 0x64)

#define ADC_CONTROL1                                    (ADC_MODULE_BASE_ADDR + 0x60)
#define ADC_CLK_DIV                                     (ADC_MODULE_BASE_ADDR + 0x08)
#define ADC_CLK_DIV1                                    (ADC_MODULE_BASE_ADDR + 0x04)
#define ANLSOFT_CONTROL                                 (ADC_MODULE_BASE_ADDR + 0x34)
#define USB_CONTROL                                     (ADC_MODULE_BASE_ADDR + 0x44)
#define X_COORDINATE_REG                                (ADC_MODULE_BASE_ADDR + 0x68)
#define Y_COORDINATE_REG                                (ADC_MODULE_BASE_ADDR + 0x6c)
#define TS_CONTROL_REG2                                 (ADC_MODULE_BASE_ADDR + 0x64)
#define ADC1_STAT_REG                                   (ADC_MODULE_BASE_ADDR + 0x70)
#define TS_CONTROL_REG1                                 (ADC_MODULE_BASE_ADDR + 0x5c)
#define ANALOG_CONTROL1                                 (ADC_MODULE_BASE_ADDR + 0x5c)
#define ANALOG_CONTROL2                                 (ADC_MODULE_BASE_ADDR + 0x64)
#define WTPF_CONTROL                                 	(ADC_MODULE_BASE_ADDR + 0x128)
#define ADC2_MODE_CFG                                   (0x20072000)
#define DAC_CONFIG_REG                                  (0x2002E000)        //DAC
#define I2S_CONFIG_REG                                  (0x2002E004)        //I2S
/** @} */


/** @{@name PWM module register and bit map define
 */
#define PWM_MODULE_BASE_ADDR                            0x08000000
#define PWM_CTRL_REG1                                   (PWM_MODULE_BASE_ADDR + 0x2c)
#define PWM_CTRL_REG2                                   (PWM_MODULE_BASE_ADDR + 0x30)
#define PWM_CTRL_REG3                                   (PWM_MODULE_BASE_ADDR + 0xb4)
#define PWM_CTRL_REG4                                   (PWM_MODULE_BASE_ADDR + 0xb8)
/** @} */

/** @{@name I2C module register and bit map define
 */
#define I2C_BASE_ADDR                                   0x20023000
#define I2C_CTRL_REG                                    (I2C_BASE_ADDR + 0x0)
#define I2C_COMMAND_CONF_REG                            (I2C_BASE_ADDR + 0x10)
#define I2C_TXRX_DATA_REG1                              (I2C_BASE_ADDR + 0x20)
#define I2C_TXRX_DATA_REG2                              (I2C_BASE_ADDR + 0x24)
#define I2C_TXRX_DATA_REG3                              (I2C_BASE_ADDR + 0x28)
#define I2C_TXRX_DATA_REG4                              (I2C_BASE_ADDR + 0x2C)
/** @} */

/** @{@name TIMER module register and bit map define
 */
#define TIMER_MODULE_BASE_ADDR                          0x08000000      // timer registers
#define TIMER1_CTRL_REG                                 (TIMER_MODULE_BASE_ADDR + 0x18)
#define TIMER2_CTRL_REG                                 (TIMER_MODULE_BASE_ADDR + 0x1c)
#define TIMER3_CTRL_REG                                 (TIMER_MODULE_BASE_ADDR + 0x20)
#define TIMER4_CTRL_REG                                 (TIMER_MODULE_BASE_ADDR + 0x24)
#define TIMER5_CTRL_REG                                 (TIMER_MODULE_BASE_ADDR + 0x28) 
#define TIMER1_READ_BACK_REG                            (TIMER_MODULE_BASE_ADDR + 0x100)
#define TIMER2_READ_BACK_REG                            (TIMER_MODULE_BASE_ADDR + 0x104)
#define TIMER3_READ_BACK_REG                            (TIMER_MODULE_BASE_ADDR + 0x108)
#define TIMER4_READ_BACK_REG                            (TIMER_MODULE_BASE_ADDR + 0x10c)
#define TIMER5_READ_BACK_REG                            (TIMER_MODULE_BASE_ADDR + 0x110)
/** @} */


/** @{@name GPIO module register and bit map define
 */
//gpio direction
#define GPIO_MODULE_BASE_ADDR                           0x08000000      // GPIO registers
#define GPIO_DIR_REG1                                   (GPIO_MODULE_BASE_ADDR + 0x7c)
#define GPIO_DIR_REG2                                   (GPIO_MODULE_BASE_ADDR + 0x84)
#define GPIO_DIR_REG3                                   (GPIO_MODULE_BASE_ADDR + 0x8c)
#define GPIO_DIR_REG4                                   (GPIO_MODULE_BASE_ADDR + 0x94)
#define GPIO_DIR_REG5                                   (GPIO_MODULE_BASE_ADDR + 0x11c)
                                                        
//gpio output control                                   
#define GPIO_OUT_REG1                                   (GPIO_MODULE_BASE_ADDR + 0x80)
#define GPIO_OUT_REG2                                   (GPIO_MODULE_BASE_ADDR + 0x88)
#define GPIO_OUT_REG3                                   (GPIO_MODULE_BASE_ADDR + 0x90)
#define GPIO_OUT_REG4                                   (GPIO_MODULE_BASE_ADDR + 0x98)
#define GPIO_OUT_REG5                                   (GPIO_MODULE_BASE_ADDR + 0x118)
                                                        
//gpio input control                                    
#define GPIO_IN_REG1                                    (GPIO_MODULE_BASE_ADDR + 0xbc)
#define GPIO_IN_REG2                                    (GPIO_MODULE_BASE_ADDR + 0xc0)
#define GPIO_IN_REG3                                    (GPIO_MODULE_BASE_ADDR + 0xc4)
#define GPIO_IN_REG4                                    (GPIO_MODULE_BASE_ADDR + 0xc8)
#define GPIO_IN_REG5                                    (GPIO_MODULE_BASE_ADDR + 0x124)
                                                        
//gpio interrupt enable/disable                         
#define GPIO_INT_EN1                                    (GPIO_MODULE_BASE_ADDR + 0xe0)
#define GPIO_INT_EN2                                    (GPIO_MODULE_BASE_ADDR + 0xe4)
#define GPIO_INT_EN3                                    (GPIO_MODULE_BASE_ADDR + 0xe8)
#define GPIO_INT_EN4                                    (GPIO_MODULE_BASE_ADDR + 0xec)
                                                        
//gpio interrupt sensitivity level                      
#define GPIO_INT_LEVEL_REG1                             (GPIO_MODULE_BASE_ADDR + 0xf0)
#define GPIO_INT_LEVEL_REG2                             (GPIO_MODULE_BASE_ADDR + 0xf4)
#define GPIO_INT_LEVEL_REG3                             (GPIO_MODULE_BASE_ADDR + 0xf8)
#define GPIO_INT_LEVEL_REG4                             (GPIO_MODULE_BASE_ADDR + 0xfc)
                                                        
//gpio pull/pulldown reg                                
#define GPIO_PULLUPDOWN_REG1                            (GPIO_MODULE_BASE_ADDR + 0x9c)
#define GPIO_PULLUPDOWN_REG2                            (GPIO_MODULE_BASE_ADDR + 0xa0)
#define GPIO_PULLUPDOWN_REG3                            (GPIO_MODULE_BASE_ADDR + 0xa4)
#define GPIO_PULLUPDOWN_REG4                            (GPIO_MODULE_BASE_ADDR + 0xa8)
                                                        
//io control reg                                        
#define GPIO_IO_CONTROL_REG1                            (GPIO_MODULE_BASE_ADDR + 0xD4)
#define GPIO_IO_CONTROL_REG2                            (GPIO_MODULE_BASE_ADDR + 0xD8)
                                                        
//share pin control reg                                 
#define GPIO_SHAREPIN_CONTROL2                          (GPIO_MODULE_BASE_ADDR + 0x74)
#define GPIO_SHAREPIN_CONTROL1                          (GPIO_MODULE_BASE_ADDR + 0x78)
/** @} */


/** @{@name Register Operation Define
 *      Define the macro for read/write register and memory
 */
#ifdef OS_ANYKA
/* ------ Macro definition for reading/writing data from/to register ------ */
#define HAL_READ_UINT32( _register_, _value_ )          ((_value_) = *((volatile T_U32 *)(_register_)))
#define HAL_WRITE_UINT32( _register_, _value_ )         (*((volatile T_U32 *)(_register_)) = (_value_))

#define REG32(_register_)                               (*(volatile T_U32 *)(_register_))
#define REG16(_register_)                               (*(volatile T_U16 *)(_register_))
#define REG8(_register_)                                (*(volatile T_U8 *)(_register_))

//read and write register
#define outb(v,p)                                       (*(volatile unsigned char *)(p) = (v))
#define outw(v,p)                                       (*(volatile unsigned short *)(p) = (v))
#define outl(v,p)                                       (*(volatile unsigned long  *)(p) = (v))
                                                        
#define inb(p)                                          (*(volatile unsigned char *)(p))
#define inw(p)                                          (*(volatile unsigned short *)(p))
#define inl(p)                                          (*(volatile unsigned long *)(p))
#define WriteBuf(v,p)                                   (*(volatile unsigned long  *)(p) = (v))
#define ReadBuf(p)                                      (*(volatile unsigned long *)(p))
                                                        
#define WriteRamb(v,p)                                  (*(volatile unsigned char *)(p) = (v))
#define WriteRamw(v,p)                                  (*(volatile unsigned short *)(p) = (v))
#define WriteRaml(v,p)                                  (*(volatile unsigned long  *)(p) = (v))
                                                        
#define ReadRamb(p)                                     (*(volatile unsigned char *)(p))
#define ReadRamw(p)                                     (*(volatile unsigned short *)(p))
#define ReadRaml(p)                                     (*(volatile unsigned long *)(p))
#else
/* ------ Macro definition for reading/writing data from/to register ------ */
#define HAL_READ_UINT8( _register_, _value_ )        
#define HAL_WRITE_UINT8( _register_, _value_ )       
#define HAL_READ_UINT16( _register_, _value_ )      
#define HAL_WRITE_UINT16( _register_, _value_ )     
#define HAL_READ_UINT32( _register_, _value_ )      
#define HAL_WRITE_UINT32( _register_, _value_ )     
#define REG32(_register_)  
#define REG16(_register_)  
#define REG8(_register_)  
#endif

//error type define
#define UNDEF_ERROR             1
#define ABT_ERROR               2
#define PREF_ERROR              3
/** @} */

/*@}*/

#endif  // _ANYKA_CPU_H_

