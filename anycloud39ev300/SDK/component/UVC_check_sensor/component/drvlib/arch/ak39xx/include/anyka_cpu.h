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
#define USB_BASE_ADDR                                   0x20200000      // USB
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
#define CLOCK_CTRL_REG                                  (CHIP_CONF_BASE_ADDR + 0x0000001C)      // module clock control(switch)
#define RESET_CTRL_REG                                  (CHIP_CONF_BASE_ADDR + 0x00000020)      // module software reset control register
#define IRQINT_MASK_REG                                 (CHIP_CONF_BASE_ADDR + 0x00000024)      // module IRQ interrupt mask register, 1: unmask; 0:mask(default);
#define FRQINT_MASK_REG                                 (CHIP_CONF_BASE_ADDR + 0x00000028)      // module FRQ interrupt mask register, 1: ummask; 0:mask(default);
#define INT_SYS_MODULE_REG                              (CHIP_CONF_BASE_ADDR + 0x0000002C)      // system module interrupt control register
#define INT_STATUS_REG_2                                (CHIP_CONF_BASE_ADDR + 0x00000030)      // module interrupt status register
#define INT_STATUS_REG                                  (CHIP_CONF_BASE_ADDR + 0x0000004C)      // module interrupt status register

#define CPU_CLOCK_DIV_REG                               (CHIP_CONF_BASE_ADDR + 0x00000004)      // clock divider register 1
#define ASIC_CLOCK_DIV_REG                              (CHIP_CONF_BASE_ADDR + 0x00000008)      // clock divider register 1
#define PERI_CLOCK_DIV_REG_1                            (CHIP_CONF_BASE_ADDR + 0x00000014)      // clock divider register 1
#define PERI_CLOCK_DIV_REG_2                            (CHIP_CONF_BASE_ADDR + 0x00000018)      // clock divider register 1

#define CLOCK2X_CTRL_REG                                (CHIP_CONF_BASE_ADDR + 0x00000004)      // clock2x control register,1: 2*ASIC clock; 0: ASIC clock
#define CLOCK3X_CTRL_REG                                (CHIP_CONF_BASE_ADDR + 0x00000064)      //bit28 => 1: asic = pll1_clock /3, 0: refresh to bit[6..8] of 0x08000004 register 
#define CPU_TIMER_REG                                   (CHIP_CONF_BASE_ADDR + 0x000000d4)

/************************************************************************/
/** @{@name Interrupt bit map define
 */ 
/** interrupt status register bit map*/
#define INT_STATUS_MEM_BIT						(1 << 0)
#define INT_STATUS_CAMERA_BIT					(1 << 1)
#define INT_STATUS_VEDIO_ENCODE_BIT			    (1 << 2)
#define INT_STATUS_SYS_MODULE_BIT				(1 << 3)
#define INT_STATUS_MMCSD_BIT					(1 << 4)
#define INT_STATUS_SDIO_BIT					    (1 << 5)
#define INT_STATUS_ADC_BIT						(1 << 6)
#define INT_STATUS_DAC_BIT						(1 << 7)
#define INT_STATUS_SPI1_BIT					    (1 << 8)
#define INT_STATUS_SPI2_BIT					    (1 << 9)
#define INT_STATUS_UART1_BIT					(1 << 10)
#define INT_STATUS_UART2_BIT					(1 << 11)
#define INT_STATUS_L2_BIT						(1 << 12)
#define INT_STATUS_IIC_BIT						(1 << 13)
#define INT_STATUS_IRDA_BIT					    (1 << 14)
#define INT_STATUS_GPIO_BIT					    (1 << 15)
#define INT_STATUS_MAC_BIT						(1 << 16)
#define INT_STATUS_ENCRYPT_BIT				    (1 << 17)
#define INT_STATUS_USB_BIT						(1 << 18)
#define INT_STATUS_USBDMA_BIT					(1 << 19)
 
 //level 2 interrupt status bit map      
#define INT_STATUS_SARADC_BIT					(1 << 0)
#define INT_STATUS_TIMER5_BIT					(1 << 1)
#define INT_STATUS_TIMER4_BIT					(1 << 2)
#define INT_STATUS_TIMER3_BIT					(1 << 3)
#define INT_STATUS_TIMER2_BIT					(1 << 4)
#define INT_STATUS_TIMER1_BIT					(1 << 5)
#define INT_STATUS_TRIGGER_BIT					(1 << 6)
#define INT_STATUS_RTC_RDY_BIT				    (1 << 7)
#define INT_STATUS_RTC_BIT						(1 << 8)
#define INT_STATUS_RTC_TIMER_BIT				(1 << 9)
#define INT_STATUS_RTC_WATCHDOG_BIT			    (1 << 10)
  
/* define the level1 interrupt valid bits */
#define INT_STATUS_NBITS                                19
 
/** IRQ interrupt mask register bit map*/
#define IRQ_MASK_MEM_BIT                        (1 << 0)
#define IRQ_MASK_CAMERA_BIT						(1 << 1)
#define IRQ_MASK_VENC_BIT			    		(1 << 2)
#define IRQ_MASK_SYS_MODULE_BIT					(1 << 3)
#define IRQ_MASK_MMCSD_BIT					    (1 << 4)
#define IRQ_MASK_SDIO_BIT					    (1 << 5)
#define IRQ_MASK_ADC_BIT					    (1 << 6)
#define IRQ_MASK_DAC_BIT					    (1 << 7)
#define IRQ_MASK_SPI1_BIT					    (1 << 8)
#define IRQ_MASK_SPI2_BIT					    (1 << 9)
#define IRQ_MASK_UART1_BIT					    (1 << 10)
#define IRQ_MASK_UART2_BIT					    (1 << 11)
#define IRQ_MASK_L2_BIT						    (1 << 12)
#define IRQ_MASK_IIC_BIT						(1 << 13)
#define IRQ_MASK_IRDA_BIT					    (1 << 14)
#define IRQ_MASK_GPIO_BIT					    (1 << 15)
#define IRQ_MASK_MAC_BIT					    (1 << 16)
#define IRQ_MASK_ENCRYPT_BIT				    (1 << 17)
#define IRQ_MASK_USB_BIT						(1 << 18)
#define IRQ_MASK_USBDMA_BIT					    (1 << 19)
                                                         
 //level 2 interrupt mask bit map                        
#define IRQ_MASK_SARADC_BIT					    (1 << 0)
#define IRQ_MASK_TIMER5_BIT					    (1 << 1)
#define IRQ_MASK_TIMER4_BIT					    (1 << 2)
#define IRQ_MASK_TIMER3_BIT					    (1 << 3)
#define IRQ_MASK_TIMER2_BIT					    (1 << 4)
#define IRQ_MASK_TIMER1_BIT					    (1 << 5)
#define IRQ_MASK_TRIGGER_BIT				    (1 << 6)
#define IRQ_MASK_RTC_RDY_BIT				    (1 << 7)
#define IRQ_MASK_RTC_BIT						(1 << 8)
#define IRQ_MASK_RTC_TIMER_BIT				    (1 << 9)
#define IRQ_MASK_RTC_WATCHDOG_BIT			    (1 << 10)
/** @} */

/** @{@name SDRAM&DMA register and bit map define
 */
#define MEM_CTRL_CFG_BASE_ADDR                  (0x21000000)
#define DMA_CTRL_BASE_ADDR                      MEM_CTRL_CFG_BASE_ADDR
#define DMA_PRIORITY_CTRL_REG1                  (DMA_CTRL_BASE_ADDR + 0x0000000c)
#define DMA_PRIORITY_CTRL_REG2                  (DMA_CTRL_BASE_ADDR + 0x00000010)

#define AHB_PRIORITY_CTRL_REG1                  (MEM_CTRL_CFG_BASE_ADDR + 0x18)
#define AHB_PRIORITY_CTRL_REG2                  (MEM_CTRL_CFG_BASE_ADDR + 0x1c)

#define SDRAM_CFG_REG1                          (MEM_CTRL_CFG_BASE_ADDR + 0x0)
#define SDRAM_CFG_REG2                          (MEM_CTRL_CFG_BASE_ADDR + 0x4)
#define SDRAM_CFG_REG3                          (MEM_CTRL_CFG_BASE_ADDR + 0x8)
#define SDRAM_CFG_REG4                          (MEM_CTRL_CFG_BASE_ADDR + 0xc)

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
#define L2_CRC                                          (L2_BASE_ADDR+0xa4)
#define L2_STAT_REG2                                    (L2_BASE_ADDR+0xa8)
#define L2_COLLISION                                    (L2_BASE_ADDR+0xac) //

/** @} */

/** @{@name TWI register and bit map define
 */ 
#define TWI_REG_BASE									0x20150000
#define TWI_CTRL										(TWI_REG_BASE+0x00)
#define TWI_CMD1										(TWI_REG_BASE+0x10)
#define TWI_CMD2										(TWI_REG_BASE+0x14)
#define TWI_CMD3										(TWI_REG_BASE+0x18)
#define TWI_CMD4										(TWI_REG_BASE+0x1C)
#define TWI_DATA0										(TWI_REG_BASE+0x20)
#define TWI_DATA1										(TWI_REG_BASE+0x24)
#define TWI_DATA2										(TWI_REG_BASE+0x28)
#define TWI_DATA3										(TWI_REG_BASE+0x2C)

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
#define RTC_WAKEUP_OTHER_CON_REG						(RTC_MODULE_BASE_ADDR + 0x34)	//wakeup other config
#define RTC_WAKEUP_OTHER_S_REG							(RTC_MODULE_BASE_ADDR + 0x38)	// wakeup other source status
/** @} */


/** @} */



/** @{@name IMAGE sensor module register and bit map define
 */
#define IMAGE_MODULE_BASE_ADDR                          0x20030000      // image sensor
#define IMG_CMD_ADDR                                    (IMAGE_MODULE_BASE_ADDR | 0x0000)
#define IMG_SINFO_ADDR                                  (IMAGE_MODULE_BASE_ADDR | 0x0004)
#define IMG_DINFO_ADDR                                  (IMAGE_MODULE_BASE_ADDR | 0x0008)
#define IMG_PIXEL_NUM_ADDR                              (IMAGE_MODULE_BASE_ADDR | 0x0010)
#define IMG_YADDR_ODD                                   (IMAGE_MODULE_BASE_ADDR | 0x0018)
#define IMG_UADDR_ODD                                   (IMAGE_MODULE_BASE_ADDR | 0x001c)
#define IMG_VADDR_ODD                                   (IMAGE_MODULE_BASE_ADDR | 0x0020)
#define IMG_JPEGADDR_ODD                                (IMAGE_MODULE_BASE_ADDR | 0x0024)
#define IMG_YADDR_EVE                                   (IMAGE_MODULE_BASE_ADDR | 0x0028)
#define IMG_UADDR_EVE                                   (IMAGE_MODULE_BASE_ADDR | 0x002c)
#define IMG_VADDR_EVE                                   (IMAGE_MODULE_BASE_ADDR | 0x0030)
#define IMG_JPEGADDR_EVE                                (IMAGE_MODULE_BASE_ADDR | 0x0034)
#define IMG_CLIP_OFFSIZE_ADDR                           (IMAGE_MODULE_BASE_ADDR | 0x0038)
#define IMG_CLIP_SIZE_ADDR                              (IMAGE_MODULE_BASE_ADDR | 0x003c)
#define IMG_CONFIG_ADDR                                 (IMAGE_MODULE_BASE_ADDR | 0x0040)
#define IMG_STATUS_ADDR                                 (IMAGE_MODULE_BASE_ADDR | 0x0060)
#define IMG_NUM_ADDR                                    (IMAGE_MODULE_BASE_ADDR | 0x0080)

#define MUL_FUN_CTL_REG                                 (CHIP_CONF_BASE_ADDR | 0x0058)
#define MUL_FUN_CTL_REG2                                (CHIP_CONF_BASE_ADDR | 0x0014)

/** @} */


/** @{@name UART module register and bit map define
 */
#define UART_MODULE_BASE_ADDR                           0x20130000      // UART
#define UART0_BASE_ADDR                                 (UART_MODULE_BASE_ADDR + 0x00000000)
#define UART1_BASE_ADDR                                 (UART_MODULE_BASE_ADDR + 0x00008000)
#define	UARTx_REG_BASE_ADDR(uart)		                (UART_MODULE_BASE_ADDR + (unsigned long)(uart)*0x8000)


#define UART_CFG_REG1                                   (0x00)
#define UART_CFG_REG2                                   (0x04)
#define UART_DATA_CFG                                   (0x08)
#define UART_RX_THREINT                                 (0x0c)
#define UART_RX_BUFFER                                  (0x10)
#define UART_RX_EXTEND                                  (0x14)

/** @} */

/** @{@name SPI module register and bit map define
 */
#define SPI_MODULE_BASE_ADDR                            0x20120000      // SPI
#define SPI0_BASE_ADDR                                  (SPI_MODULE_BASE_ADDR + 0x00000000)
#define SPI1_BASE_ADDR                                  (SPI_MODULE_BASE_ADDR + 0x00008000)
                                                        
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
#define USB_I2S_CTRL_REG                                (0x08000058)
#define USB_CONTROL_REG                                 (0x20200002)
#define USB_CONTROL_REG2                                (0x20200004)
#define USB_DETECT_CTRL_REG                             (0x080000D8)
#define USB_DETECT_STATUS_REG                           (0x080000DC)
#define USB_FIFO_EP0                                    (USB_BASE_ADDR + 0x0020)
#define USB_FIFO_EP1                                    (USB_BASE_ADDR + 0x0024)
#define USB_FIFO_EP2                                    (USB_BASE_ADDR + 0x0028)
#define USB_FIFO_EP3                                    (USB_BASE_ADDR + 0x002C)
#define USB_FIFO_EP4                                    (USB_BASE_ADDR + 0x0030)
#define USB_FIFO_EP5                                    (USB_BASE_ADDR + 0x0034)
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
#define USB_REG_TXINTERVAL                              (USB_BASE_ADDR + 0x001B)
#define USB_REG_RXTYPE                                  (USB_BASE_ADDR + 0x001C)
#define USB_REG_RXINTERVAL                              (USB_BASE_ADDR + 0x001D)
#define USB_REG_NAKLIMIT0                               (USB_BASE_ADDR + 0x001B)
//
#define USB_REG_REQPKTCNT1                              (USB_BASE_ADDR + 0x0304)
#define USB_REG_REQPKTCNT2                              (USB_BASE_ADDR + 0x0308)
#define USB_REG_REQPKTCNT3                              (USB_BASE_ADDR + 0x030C)
#define USB_REG_REQPKTCNT4                              (USB_BASE_ADDR + 0x0310)
#define USB_REG_REQPKTCNT5                              (USB_BASE_ADDR + 0x0314)
#define USB_REG_REQPKTCNT6                              (USB_BASE_ADDR + 0x0318)

#define USB_EP0_TX_COUNT                                (USB_BASE_ADDR + 0x0330)
#define USB_EP2_TX_COUNT                                (USB_BASE_ADDR + 0x0334)
                                                        
#define USB_FORBID_WRITE_REG                            (USB_BASE_ADDR + 0x0338)
                                                        
#define USB_START_PRE_READ_REG                          (USB_BASE_ADDR + 0x033C)

#define USB_L2_CONTROL_FIFO                             (0x48001500)
                                                        
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
/** @} */


/** @{@name SDMMC/SDIO module register and bit map define
 */
#define SD_MMC_BASE_ADDR                                0x20100000       /*mmc_sd interface*/
#define SDIO_BASE_ADDR                                  0x20108000       /*sdio interface*/
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
#define ADC_CLK_DIV                                     (ADC_MODULE_BASE_ADDR + 0x0C)
#define ADC_HCLK_DIV                                    (ADC_MODULE_BASE_ADDR + 0x10)
#define ADC_SAMP_CTR_REG								(ADC_MODULE_BASE_ADDR + 0x005C)
#define ADC_SAMP_DATA_REG								(ADC_MODULE_BASE_ADDR + 0x0068)
#define ADC_SAMP_RATE_REG								(ADC_MODULE_BASE_ADDR + 0x0060)
#define ADC_INIT_REG									(ADC_MODULE_BASE_ADDR + 0x006C)

#define AUDIO_CODEC_CFG1_REG							(ADC_MODULE_BASE_ADDR + 0x009C)//
#define AUDIO_CODEC_CFG2_REG							(ADC_MODULE_BASE_ADDR + 0x00A0)
#define AUDIO_CODEC_CFG3_REG			                (ADC_MODULE_BASE_ADDR + 0x00A4)     // AUDIO CODEC CFG3

#define ANALOG_CTRL_REG1                                (ADC_MODULE_BASE_ADDR + 0x5c)
#define ANALOG_CTRL_REG2                                (ADC_MODULE_BASE_ADDR + 0x64)
#define ANALOG_CTRL_REG3                                (ADC_MODULE_BASE_ADDR + 0x11c)
#define ANALOG_CTRL_REG4                                (ADC_MODULE_BASE_ADDR + 0x120)
#define ANALOG_CTRL_REG5                                (ADC_MODULE_BASE_ADDR + 0x124)

#define ADC2_MODE_CFG                                   (0x20072000)        //ADC //
#define ADC_DATA_REG                                    (0x20072004)        //ADC//


#define DAC_CFG_REG										(ADC_MODULE_BASE_ADDR + 0x0070) // DAC  CONFIG

#define DAC_CONFIG_REG                                  (0x20110000)        //DAC
#define	DAC_I2S_CFG_REG									(0x20110004)
#define	DAC_DATA_CPU									(0x20110008)
#define ADC2_CONFIG_REG                                 (0x20118000)        //DAC
#define	ADC_DATA_CPU									(0x20118004)



#define I2S_CONFIG_REG                                  (0x2002E004)        //I2S
#define CPU_DATA_REG                                    (0x2002E008)



/** @} */


/** @{@name PWM/Timer module register define
 */
#define PWM_TIMER_BASE_ADDR                             0x08000000
#define PWM_TIMER1_CTRL_REG1                            (PWM_TIMER_BASE_ADDR + 0xB4)// PWM TIMER1 CFG0
#define PWM_TIMER1_CTRL_REG2                            (PWM_TIMER_BASE_ADDR + 0xB8)// PWM TIMER1 CFG1
#define PWM_TIMER2_CTRL_REG1                            (PWM_TIMER_BASE_ADDR + 0xBC)// PWM TIMER2 CFG0
#define PWM_TIMER2_CTRL_REG2                            (PWM_TIMER_BASE_ADDR + 0xC0)// PWM TIMER2 CFG1
#define PWM_TIMER3_CTRL_REG1                            (PWM_TIMER_BASE_ADDR + 0xC4)// PWM TIMER3 CFG0
#define PWM_TIMER3_CTRL_REG2                            (PWM_TIMER_BASE_ADDR + 0xC8)// PWM TIMER3 CFG1
#define PWM_TIMER4_CTRL_REG1                            (PWM_TIMER_BASE_ADDR + 0xCC)// PWM TIMER4 CFG0
#define PWM_TIMER4_CTRL_REG2                            (PWM_TIMER_BASE_ADDR + 0xD0)// PWM TIMER4 CFG1
#define PWM_TIMER5_CTRL_REG1                            (PWM_TIMER_BASE_ADDR + 0xD4)// PWM TIMER5 CFG0
#define PWM_TIMER5_CTRL_REG2                            (PWM_TIMER_BASE_ADDR + 0xD8)// PWM TIMER5 CFG1
     
#define WATCHDOG_TIME_REG					(CHIP_CONF_BASE_ADDR + 0x000000E4)     // WATCHDOG TIME REGISTER
#define WATCHDOG_CTRL_REG					(CHIP_CONF_BASE_ADDR + 0x000000E8)     // WATCHDOG CONTROL REGISTER

/** @{@name PMU register define
 */
 #define PMU_CTRL_REG                                   0x08000030

 /** @{@name PMU register define
  */
 #define TCM_CTRL_REG                                   0x08000014

 /** @{@name MAC module register define
  */
#define MAC_MODULE_BASE_ADDR                           0x20300000
/** @{@name GPIO module register and bit map define
 */
//gpio direction
#define GPIO_MODULE_BASE_ADDR                           0x20170000      // GPIO registers
#define GPIO_DIR_REG1                                   (GPIO_MODULE_BASE_ADDR + 0x00)
#define GPIO_DIR_REG2                                   (GPIO_MODULE_BASE_ADDR + 0x04)
#define GPIO_DIR_REG3                                   (GPIO_MODULE_BASE_ADDR + 0x08)
                                                        
//gpio output control                                   
#define GPIO_OUT_REG1                                   (GPIO_MODULE_BASE_ADDR + 0x0c)
#define GPIO_OUT_REG2                                   (GPIO_MODULE_BASE_ADDR + 0x10)
#define GPIO_OUT_REG3                                   (GPIO_MODULE_BASE_ADDR + 0x14)
                                                        
//gpio input control                                    
#define GPIO_IN_REG1                                    (GPIO_MODULE_BASE_ADDR + 0x18)
#define GPIO_IN_REG2                                    (GPIO_MODULE_BASE_ADDR + 0x1c)
#define GPIO_IN_REG3                                    (GPIO_MODULE_BASE_ADDR + 0x20)
                                                        
//gpio interrupt enable/disable                         
#define GPIO_INT_EN1                                    (GPIO_MODULE_BASE_ADDR + 0x24)
#define GPIO_INT_EN2                                    (GPIO_MODULE_BASE_ADDR + 0x28)
#define GPIO_INT_EN3                                    (GPIO_MODULE_BASE_ADDR + 0x2c)
                                                        
//gpio interrupt polarity level                      
#define GPIO_INT_LEVEL_REG1                             (GPIO_MODULE_BASE_ADDR + 0x3c)
#define GPIO_INT_LEVEL_REG2                             (GPIO_MODULE_BASE_ADDR + 0x40)
#define GPIO_INT_LEVEL_REG3                             (GPIO_MODULE_BASE_ADDR + 0x44)

//gpio interrupt mode select
#define GPIO_INT_MODE_REG1                             (GPIO_MODULE_BASE_ADDR + 0x30)
#define GPIO_INT_MODE_REG2                             (GPIO_MODULE_BASE_ADDR + 0x34)
#define GPIO_INT_MODE_REG3                             (GPIO_MODULE_BASE_ADDR + 0x38)

//gpio edge trigger interrupt status reg
#define GPIO_EDGE_INT_STATUS_REG1                      (GPIO_MODULE_BASE_ADDR + 0x48)
#define GPIO_EDGE_INT_STATUS_REG2                      (GPIO_MODULE_BASE_ADDR + 0x4c)
#define GPIO_EDGE_INT_STATUS_REG3                      (GPIO_MODULE_BASE_ADDR + 0x50)
                                                           
//gpio pull/pulldown reg                                
#define GPIO_PULLUPDOWN_REG1                            (CHIP_CONF_BASE_ADDR + 0x80)
#define GPIO_PULLUPDOWN_REG2                            (CHIP_CONF_BASE_ADDR + 0x84)
#define GPIO_PULLUPDOWN_REG3                            (CHIP_CONF_BASE_ADDR + 0x88)
#define GPIO_PULLUPDOWN_REG4                            (CHIP_CONF_BASE_ADDR + 0xe0)


//io control reg  //not                                     
#define GPIO_IO_CONTROL_REG1                            (GPIO_MODULE_BASE_ADDR + 0x9c)
                                                        
//share pin control reg                                 
#define GPIO_SHAREPIN_CONTROL1                          (CHIP_CONF_BASE_ADDR + 0x74)
#define GPIO_SHAREPIN_CONTROL2                          (CHIP_CONF_BASE_ADDR + 0x78)
#define GPIO_SHAREPIN_CONTROL3                          (CHIP_CONF_BASE_ADDR + 0x7C)
#define GPIO_SHAREPIN_CONTROL4                          (CHIP_CONF_BASE_ADDR + 0xDC)
/** @} */


/** @{@name Register Operation Define
 *      Define the macro for read/write register and memory
 */
#ifdef OS_ANYKA
/* ------ Macro definition for reading/writing data from/to register ------ */
#define HAL_READ_UINT32( _register_, _value_ )          ((_value_) = *((volatile unsigned long *)(_register_)))
#define HAL_WRITE_UINT32( _register_, _value_ )         (*((volatile unsigned long *)(_register_)) = (_value_))

#define REG32(_register_)                               (*(volatile unsigned long *)(_register_))
#define REG16(_register_)                               (*(volatile unsigned short *)(_register_))
#define REG8(_register_)                                (*(volatile unsigned char *)(_register_))

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

