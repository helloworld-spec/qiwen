/*****************************************************************************
 *
 *            Copyright Andes Technology Corporation 2014
 *                         All Rights Reserved.
 *
 *  Revision History:
 *
 *    Jan.11.2014     Created.
 ****************************************************************************/

#ifndef __AE210P_REGS_H__ 
#define __AE210P_REGS_H__ 

#ifndef __ASSEMBLER__
#include <inttypes.h>
#include <nds32_intrinsic.h>
#endif

/*****************************************************************************
 * IRQ Vector
 ****************************************************************************/
#define IRQ_RTC_PERIOD_VECTOR		0	
#define IRQ_RTC_ALARM_VECTOR		1	
#define IRQ_PIT_VECTOR			2
#define IRQ_SPI1_VECTOR			3
#define IRQ_SPI2_VECTOR			4	
#define IRQ_I2C_VECTOR			5
#define IRQ_GPIO_VECTOR			6
#define IRQ_UART1_VECTOR		7
#define IRQ_UATR2_VECTOR		8	
#define IRQ_DMA_VECTOR			9	
#define IRQ_BMC_VECTOR			10	
#define IRQ_SWI_VECTOR			11

/* EXT_INT_0~19 are reserved for vendor IPs */
#define IRQ_EXTINT0_VECTOR		12	
#define IRQ_EXTINT1_VECTOR		13
#define IRQ_EXTINT2_VECTOR		14	
#define IRQ_EXTINT3_VECTOR		15	
#define IRQ_EXTINT4_VECTOR		16
#define IRQ_EXTINT5_VECTOR		17
#define IRQ_EXTINT6_VECTOR		18
#define IRQ_EXTINT7_VECTOR		19	
#define IRQ_EXTINT8_VECTOR		20	
#define IRQ_EXTINT9_VECTOR		21
#define IRQ_EXTINT10_VECTOR		22	
#define IRQ_EXTINT11_VECTOR		23	
#define IRQ_EXTINT12_VECTOR		24
#define IRQ_EXTINT13_VECTOR		25
#define IRQ_EXTINT14_VECTOR		26	
#define IRQ_EXTINT15_VECTOR		27	
#define IRQ_EXTINT16_VECTOR		28	
#define IRQ_EXTINT17_VECTOR		29	
#define IRQ_EXTINT18_VECTOR		30	
#define IRQ_EXTINT19_VECTOR		31	



#define REG32(reg)               (  *( (volatile uint32_t *) (reg) ) )


/*****************************************************************************
 * ExLM - AE210P AHB
 * **************************************************************************/
#define EILM_BASE			0x00000000
#define EDLM_BASE			0x00200000

/*****************************************************************************
 * AHBC - AE210P AHB
 ****************************************************************************/
#define AHBC_BASE_4_7			0x00400000 /* Vendor AHB Slave 4~7 */
#define AHBC_BASE_0_3			0x00E20000 /* Vendor AHB Slave 0~3 */
#if 0
/* AHB slave#n base/size registers */
#define AHBC_SLAVE(n)			(AHBC_BASE + 0x04 * (uint32_t)(n)) /* AHB slave#n base/size register */

#define AHBC_SLAVE0			(AHBC_BASE + 0x00) /* AHB slave0 base/size register  (0x90100000 AHBC) */
#define AHBC_SLAVE1			(AHBC_BASE + 0x04) /* AHB slave1 base/size register  (0x90500000 AHB/APB Bridge) */
#define AHBC_SLAVE2			(AHBC_BASE + 0x08) /* AHB slave2 base/size register  (0x98070000 ?? APB Devices) */
#define AHBC_SLAVE3			(AHBC_BASE + 0x0c) /* AHB slave3 base/size register  (0x90200000 SMC) */
#define AHBC_SLAVE4			(AHBC_BASE + 0x10) /* AHB slave4 base/size register  (0x00080000 ?? SRAM/ROM/FLASH) */
#define AHBC_SLAVE5			(AHBC_BASE + 0x14) /* AHB slave5 base/size register  (0x90300000 SDRAMC) */
#define AHBC_SLAVE6			(AHBC_BASE + 0x18) /* AHB slave6 base/size register  (0x100b0000 ?? SDRAM) */
#define AHBC_SLAVE7			(AHBC_BASE + 0x1c) /* AHB slave7 base/size register  (0x90400000 DMAC) */
#define AHBC_SLAVE9			(AHBC_BASE + 0x24) /* AHB slave9 base/size register  (0x90600000 LCDC) */
#define AHBC_SLAVE12			(AHBC_BASE + 0x30) /* AHB slave12 base/size register (0x90900000 MAC) */
#define AHBC_SLAVE13			(AHBC_BASE + 0x34) /* AHB slave13 base/size register (0x90a00000 External AHB Device, HS13) */
#define AHBC_SLAVE14			(AHBC_BASE + 0x38) /* AHB slave14 base/size register (0x90b00000 USB) */
#define AHBC_SLAVE15			(AHBC_BASE + 0x3c) /* AHB slave15 base/size register (0x90c00000 External AHB Device, HS15) */
#define AHBC_SLAVE17			(AHBC_BASE + 0x44) /* AHB slave17 base/size register (0x90e00000 External AHB Device, HS17) */
#define AHBC_SLAVE18			(AHBC_BASE + 0x48) /* AHB slave18 base/size register (0x90f00000 External AHB Device, HS18) */
#define AHBC_SLAVE19			(AHBC_BASE + 0x4c) /* AHB slave19 base/size register (0x92000000 External AHB Device, HS19) */
#define AHBC_SLAVE21			(AHBC_BASE + 0x54) /* AHB slave21 base/size register (0xa0080000 External AHB Device, HS21) */
#define AHBC_SLAVE22			(AHBC_BASE + 0x58) /* AHB slave22 base/size register (0xb0070000 External AHB Device, HS22) */

/* AHB control registers */
#define AHBC_PRICR			(AHBC_BASE + 0x80) /* AHB master#n (n = 1~9) priority level */
#define AHBC_TRANSCR			(AHBC_BASE + 0x84) /* AHB transfer control register */
#define AHBC_INTCR			(AHBC_BASE + 0x88) /* AHB interrupt control register */
#endif

/*****************************************************************************
 * BMC - AE210P AHB
 ****************************************************************************/
#define BMC_BASE			0x00E00000 /* APB Decoder */


/*****************************************************************************
 * DMAC - AE210P AHB
 ****************************************************************************/
#define DMAC_BASE			0x00E0E000 /* Device base address */
#if 0
/* DMA controller registers (8-bit width) */
#define DMAC_INT			(DMAC_BASE + 0x00) /* Interrupt status register */
#define DMAC_INT_TC			(DMAC_BASE + 0x04) /* Interrupt for terminal count status register */
#define DMAC_INT_TC_CLR			(DMAC_BASE + 0x08) /* Interrupt for terminal count clear register */
#define DMAC_INT_ERRABT			(DMAC_BASE + 0x0c) /* Interrupt for error/abort status register */
#define DMAC_INT_ERRABT_CLR		(DMAC_BASE + 0x10) /* Interrupt for error/abort clear register */
#define DMAC_TC				(DMAC_BASE + 0x14) /* Terminal count status register */
#define DMAC_ERRABT			(DMAC_BASE + 0x18) /* Error/abort status register */
#define DMAC_CH_EN			(DMAC_BASE + 0x1c) /* Channel enable status register */
#define DMAC_CH_BUSY			(DMAC_BASE + 0x20) /* Channel busy status register */
#define DMAC_CSR			(DMAC_BASE + 0x24) /* Main configuration status register */
#define DMAC_SYNC			(DMAC_BASE + 0x28) /* Sync register */

/* DMA channel registers base address */
#define DMAC_C0_OFFSET			0x100
#define DMAC_C1_OFFSET			0x120
#define DMAC_C2_OFFSET			0x140
#define DMAC_C3_OFFSET			0x160

#define DMAC_C0_BASE			(DMAC_BASE + DMAC_C0_OFFSET)
#define DMAC_C1_BASE			(DMAC_BASE + DMAC_C1_OFFSET)
#define DMAC_C2_BASE			(DMAC_BASE + DMAC_C2_OFFSET)
#define DMAC_C3_BASE			(DMAC_BASE + DMAC_C3_OFFSET)

#define DMAC_MAX_CHANNELS		8
#define DMAC_BASE_CH(n)			(DMAC_C0_BASE + (DMAC_C1_OFFSET - DMAC_C0_OFFSET) * (uint32_t)(n)) /* n = 0 ~ 3 */

#define DMAC_CSR_OFFSET			0x00
#define DMAC_CFG_OFFSET			0x04
#define DMAC_SRC_ADDR_OFFSET		0x08
#define DMAC_DST_ADDR_OFFSET		0x0c
#define DMAC_LLP_OFFSET			0x10
#define DMAC_SIZE_OFFSET		0x14

/* DMA channel 0 registers (32-bit width) */
#define DMAC_C0_CSR			(DMAC_C0_BASE + DMAC_CSR_OFFSET)	/* Channel 0 control register */
#define DMAC_C0_CFG			(DMAC_C0_BASE + DMAC_CFG_OFFSET)	/* Channel 0 configuration register */
#define DMAC_C0_SRC_ADDR		(DMAC_C0_BASE + DMAC_SRC_ADDR_OFFSET)	/* Channel 0 source register */
#define DMAC_C0_DST_ADDR		(DMAC_C0_BASE + DMAC_DST_ADDR_OFFSET)	/* Channel 0 destination register */
#define DMAC_C0_LLP			(DMAC_C0_BASE + DMAC_LLP_OFFSET)	/* Channel 0 linked list pointer register */
#define DMAC_C0_SIZE			(DMAC_C0_BASE + DMAC_SIZE_OFFSET)	/* Channel 0 transfer size register */

/* DMA channel 1 registers (32-bit width) */
#define DMAC_C1_CSR			(DMAC_C1_BASE + DMAC_CSR_OFFSET)	/* Channel 1 control register */
#define DMAC_C1_CFG			(DMAC_C1_BASE + DMAC_CFG_OFFSET)	/* Channel 1 configuration register */
#define DMAC_C1_SRC_ADDR		(DMAC_C1_BASE + DMAC_SRC_ADDR_OFFSET)	/* Channel 1 source register */
#define DMAC_C1_DST_ADDR		(DMAC_C1_BASE + DMAC_DST_ADDR_OFFSET)	/* Channel 1 destination register */
#define DMAC_C1_LLP			(DMAC_C1_BASE + DMAC_LLP_OFFSET)	/* Channel 1 linked list pointer register */
#define DMAC_C1_SIZE			(DMAC_C1_BASE + DMAC_SIZE_OFFSET)	/* Channel 1 transfer size register */

/* DMA channel 2 registers (32-bit width) */
#define DMAC_C2_CSR			(DMAC_C2_BASE + DMAC_CSR_OFFSET)	/* Channel 2 control register */
#define DMAC_C2_CFG			(DMAC_C2_BASE + DMAC_CFG_OFFSET)	/* Channel 2 configuration register */
#define DMAC_C2_SRC_ADDR		(DMAC_C2_BASE + DMAC_SRC_ADDR_OFFSET)	/* Channel 2 source register */
#define DMAC_C2_DST_ADDR		(DMAC_C2_BASE + DMAC_DST_ADDR_OFFSET)	/* Channel 2 destination register */
#define DMAC_C2_LLP			(DMAC_C2_BASE + DMAC_LLP_OFFSET)	/* Channel 2 linked list pointer register */
#define DMAC_C2_SIZE			(DMAC_C2_BASE + DMAC_SIZE_OFFSET)	/* Channel 2 transfer size register */

/* DMA channel 3 registers (32-bit width) */
#define DMAC_C3_CSR			(DMAC_C3_BASE + DMAC_CSR_OFFSET)	/* Channel 3 control register */
#define DMAC_C3_CFG			(DMAC_C3_BASE + DMAC_CFG_OFFSET)	/* Channel 3 configuration register */
#define DMAC_C3_SRC_ADDR		(DMAC_C3_BASE + DMAC_SRC_ADDR_OFFSET)	/* Channel 3 source register */
#define DMAC_C3_DST_ADDR		(DMAC_C3_BASE + DMAC_DST_ADDR_OFFSET)	/* Channel 3 destination register */
#define DMAC_C3_LLP			(DMAC_C3_BASE + DMAC_LLP_OFFSET)	/* Channel 3 linked list pointer register */
#define DMAC_C3_SIZE			(DMAC_C3_BASE + DMAC_SIZE_OFFSET)	/* Channel 3 transfer size register */
#endif

/*****************************************************************************
 * APBBRG - AE210P APB
 ****************************************************************************/

#define APBBR_BASE			0x00F00000 /*  base address */
#if 0
/* AHB to APB bridge registers (32-bit width) */
#define APBBR_SLAVE(n)			(APBBR_BASE + 0x04 * (uint32_t)(n)) /* APB slave#n (n = 1~6, 8, 11, 16~23) base/size register */

#define APBBR_SLAVE1			(APBBR_BASE + 0x04) /* APB slave1 base/size register  (0x18d00000) */
#define APBBR_SLAVE2			(APBBR_BASE + 0x08) /* APB slave2 base/size register  (0x18b00000) */
#define APBBR_SLAVE3			(APBBR_BASE + 0x0c) /* APB slave3 base/size register  (0x18200000) */
#define APBBR_SLAVE4			(APBBR_BASE + 0x10) /* APB slave4 base/size register  (0x18300000) */
#define APBBR_SLAVE5			(APBBR_BASE + 0x14) /* APB slave5 base/size register  (0x18e00000) */
#define APBBR_SLAVE6			(APBBR_BASE + 0x18) /* APB slave6 base/size register  (0x19400000) */
#define APBBR_SLAVE8			(APBBR_BASE + 0x20) /* APB slave8 base/size register  (0x19600000) */
#define APBBR_SLAVE11			(APBBR_BASE + 0x2c) /* APB slave11 base/size register (0x18900000) */
#define APBBR_SLAVE16			(APBBR_BASE + 0x40) /* APB slave16 base/size register (0x18100000) */
#define APBBR_SLAVE17			(APBBR_BASE + 0x44) /* APB slave17 base/size register (0x18400000) */
#define APBBR_SLAVE18			(APBBR_BASE + 0x48) /* APB slave18 base/size register (0x18500000) */
#define APBBR_SLAVE19			(APBBR_BASE + 0x4c) /* APB slave19 base/size register (0x18600000) */
#define APBBR_SLAVE20			(APBBR_BASE + 0x50) /* APB slave20 base/size register (0x18700000) */
#define APBBR_SLAVE21			(APBBR_BASE + 0x54) /* APB slave21 base/size register (0x18800000) */
#define APBBR_SLAVE22			(APBBR_BASE + 0x58) /* APB slave22 base/size register (0x18a00000) */
#define APBBR_SLAVE23			(APBBR_BASE + 0x5c) /* APB slave23 base/size register (0x19100000) */

/* DMA channel A registers (32-bit width) */
#define APBBR_DMAA_BASE			(APBBR_BASE + 0x80) /* APB Bridge DMA channel A base register */
#define APBBR_DMAB_BASE			(APBBR_BASE + 0x90) /* APB Bridge DMA channel B base register */
#define APBBR_DMAC_BASE			(APBBR_BASE + 0xa0) /* APB Bridge DMA channel C base register */
#define APBBR_DMAD_BASE			(APBBR_BASE + 0xb0) /* APB Bridge DMA channel D base register */

#define APBBR_DMA_MAX_CHANNELS		4
#define APBBR_DMA_BASE_CH(n)		(APBBR_DMAA_BASE + (APBBR_DMAB_BASE - APBBR_DMAA_BASE) * (uint32_t)(n)) /* n = 0 ~ 3 */

#define APBBR_DMA_SAD_OFFSET		0x00 /* DMA channel source address register */
#define APBBR_DMA_DAD_OFFSET		0x04 /* DMA channel destination address register */
#define APBBR_DMA_CYC_OFFSET		0x08 /* DMA channel transfer cycles register */
#define APBBR_DMA_CMD_OFFSET		0x0c /* DMA channel command register */
#endif

/*****************************************************************************
 * SMU - AE210P
 ****************************************************************************/

/*****************************************************************************
 * UARTx - AE210P
 ****************************************************************************/
/* Device base address */
#define UART1_BASE			0x00F02000
#define UART2_BASE			0x00F03000
#define STUARTC_BASE                    UART2_BASE /* standard/IR UART */
//#define BTUARTC_BASE                    0x00F03000 /* blue tooth UART */

/* UART register offsets (4~8-bit width) */
/* SD_LCR_DLAB == 0 */
#define UARTC_RBR_OFFSET                0x20 /* receiver biffer register */
#define UARTC_THR_OFFSET                0x20 /* transmitter holding register */
#define UARTC_IER_OFFSET                0x24 /* interrupt enable register */
#define UARTC_IIR_OFFSET                0x28 /* interrupt identification register */
#define UARTC_FCR_OFFSET                0x28 /* FIFO control register */
#define UARTC_LCR_OFFSET                0x2c /* line control regitser */
#define UARTC_MCR_OFFSET                0x30 /* modem control register */
#define UARTC_LSR_OFFSET                0x34 /* line status register */
#define UARTC_TST_OFFSET                0x34 /* testing register */
#define UARTC_MSR_OFFSET                0x38 /* modem status register */
#define UARTC_SPR_OFFSET                0x3c /* scratch pad register */

/* SD_LCR_DLAB == 0 */
#define UARTC_DLL_OFFSET                0x20 /* baudrate divisor latch LSB */
#define UARTC_DLM_OFFSET                0x24 /* baudrate divisor latch MSB */
#define UARTC_PSR_OFFSET                0x28 /* prescaler register */

#define UARTC_MDR_OFFSET                0x40 /* mode definition register */
#define UARTC_ACR_OFFSET                0x44 /* auxiliary control register */


/*****************************************************************************
 * PIT - AE210P
 ****************************************************************************/
#define PIT_BASE			0x00F04000 /* Device base address */

/* PIT register (32-bit width) */
#define PIT_ID_REV			(PIT_BASE + 0x00 ) /* (ro)  PIT ID and Revision Register */
#define PIT_CFG				(PIT_BASE + 0x10 ) /* (ro)  PIT Configuration Register	 */
#define PIT_INT_EN			(PIT_BASE + 0x14 ) /* (rw)  PIT Interrupt Enable Register*/
#define PIT_INT_ST			(PIT_BASE + 0x18 ) /* (w1c) PIT Interrupt Status Register*/
#define PIT_CH_EN			(PIT_BASE + 0x1C ) /* (rw)  PIT Channel Enable Register	 */

/* _chn_ from 0 to 3*/
/* (rw) PIT Channel x Control Register (32-bit width) */
#define PIT_CHNx_CTL(_chn_)		( PIT_BASE + 0x20 + ( (_chn_)* 0x10) )

/* (rw) PIT Channel x Reload Register (32-bit width)  */
#define PIT_CHNx_LOAD(_chn_)		( PIT_BASE + 0x24 + ( (_chn_)* 0x10) )

/* (ro) PIT Channel x Counter Register (32-bit width) */
#define PIT_CHNx_COUNT(_chn_)		( PIT_BASE + 0x28 + ( (_chn_)* 0x10) )


/*****************************************************************************
 * WDT - AE210P
 ****************************************************************************/
#define WDTC_BASE			0x00F05000 /* Device base address */


/*****************************************************************************
 * RTC - AE210P
 ****************************************************************************/

#define RTC_BASE			0x00F06000 /* Device base address */
#if 0
/* RTC registers (32-bit width) */
#define RTC_SEC				(RTC_BASE + 0x00) /* (ro) RTC second counter register [5:0] */
#define RTC_MIN				(RTC_BASE + 0x04) /* (ro) RTC minute counter register [5:0] */
#define RTC_HOUR			(RTC_BASE + 0x08) /* (ro) RTC hour counter register [4:0] */
#define RTC_DAY				(RTC_BASE + 0x0c) /* (ro) RTC day counter register [15:0] */

#define RTC_ALARM_SEC			(RTC_BASE + 0x10) /* (rw) RTC second alarm register [5:0] */
#define RTC_ALARM_MIN			(RTC_BASE + 0x14) /* (rw) RTC minute alarm register [5:0] */
#define RTC_ALARM_HOUR			(RTC_BASE + 0x18) /* (rw) RTC hour alarm register [4:0] */

#define RTC_RECORD			(RTC_BASE + 0x1c) /* (rw) RTC record register [31:0] */
#define RTC_CR				(RTC_BASE + 0x20) /* (rw) RTC control register */

#define RTC_WSEC			(RTC_BASE + 0x24) /* (rw) RTC second counter write port */
#define RTC_WMIN			(RTC_BASE + 0x28) /* (rw) RTC minute counter write port */
#define RTC_WHOUR			(RTC_BASE + 0x2c) /* (rw) RTC hour counter write port */
#define RTC_WDAY			(RTC_BASE + 0x30) /* (rw) RTC day counter write port */

#define RTC_ISR				(RTC_BASE + 0x34) /* (rw) RTC interrupt register */
#define RTC_DIVIDE			(RTC_BASE + 0x38) /* (rw) RTC frequency divider */
#define RTC_REVISION			(RTC_BASE + 0x3c) /* (rw) RTC revision register */
#endif

/*****************************************************************************
 * GPIO - AE210P
 ****************************************************************************/
#define GPIOC_BASE			0x00F07000 /* Device base address */
#if 0
/* GPIO registers (32-bit width) */
#define GPIOC_DATA_OUT			(GPIOC_BASE + 0x00) /* (rw) GPIO data output register */
#define GPIOC_DATA_IN			(GPIOC_BASE + 0x04) /* (ro) GPIO data input register */
#define GPIOC_PIN_DIR			(GPIOC_BASE + 0x08) /* (rw) GPIO direction register (0: in, 1: out) */
#define GPIOC_PIN_BYPASS		(GPIOC_BASE + 0x0c) /* (rw) GPIO pin bypass register */
#define GPIOC_DATA_SET			(GPIOC_BASE + 0x10) /* (wo) GPIO data bit set register */
#define GPIOC_DATA_CLEAR		(GPIOC_BASE + 0x14) /* (wo) GPIO data bit clear register */

#define GPIOC_PIN_PULL_ENABLE		(GPIOC_BASE + 0x18) /* (rw) GPIO pull enable register (0: no-pull, 1: pull) */
#define GPIOC_PIN_PULL_TYPE		(GPIOC_BASE + 0x1c) /* (rw) GPIO pull hi/lo register (0: pull-low, 1: pull-high) */

#define GPIOC_INT_ENABLE		(GPIOC_BASE + 0x20) /* (rw) GPIO interrupt enable register (0: disable, 1: enable) */
#define GPIOC_INT_RAW_STATE		(GPIOC_BASE + 0x24) /* (ro) GPIO interrupt raw status register (0: no, 1: detected) */
#define GPIOC_INT_MASKED_STATE		(GPIOC_BASE + 0x28) /* (ro) GPIO interrupt masked status register (0: masked/no, 1: detected) */
#define GPIOC_INT_MASK			(GPIOC_BASE + 0x2c) /* (rw) GPIO interrupt mask register (0: no-mask, 1: mask) */
#define GPIOC_INT_CLEAR			(GPIOC_BASE + 0x30) /* (wo) GPIO interrupt clear register (0: ignore, 1: clear) */
#define GPIOC_INT_TRIGGER		(GPIOC_BASE + 0x34) /* (rw) GPIO interrupt trigger method register (0: edge, 1: level) */
#define GPIOC_INT_BOTH			(GPIOC_BASE + 0x38) /* (rw) GPIO interrupt edge trigger type register (0: single, 1: both) */
#define GPIOC_INT_RISE_NEG		(GPIOC_BASE + 0x3c) /* (rw) GPIO interrupt trigger side/level register (0: rising/high, 1: falling/low) */
#define GPIOC_INT_BOUNCE_ENABLE		(GPIOC_BASE + 0x40) /* (rw) GPIO interrupt debounce-clock enable register (0: disable, 1: enable) */
#define GPIOC_INT_BOUNCE_PRESCALE	(GPIOC_BASE + 0x44) /* (rw) GPIO interrupt debounce-clock pclk-div value register (0: disable, valid: 0x0001 ~ 0xffff) */
#endif


/*****************************************************************************
 * I2C - AE210P
 ****************************************************************************/
#define I2C_BASE			0x00F0A000 /* Device base address */


/*****************************************************************************
 * SPI1 - AE210P
 ****************************************************************************/
#define SPI1_BASE			0x00F0B000 /* (SPI1) Device base address */



/*****************************************************************************
 * I2S/AC97 - AE210P (SSP2)
 ****************************************************************************/
#define SPI2_BASE			0x00F0F000

/*****************************************************************************
 * APB_SLAVE - AE210P Vender APB Slave 0~4
 ****************************************************************************/
#define APB_SLAVE_BASE 			0x00F19000



/*****************************************************************************
 * Macros for Register Access
 ****************************************************************************/
#ifdef REG_IO_HACK

/* 8 bit access */
//#define IN8(reg)			(  *( (volatile uint8_t *) (reg) ) )
#define OUT8(reg, data)			( (*( (volatile uint8_t *) (reg) ) ) = (uint8_t)(data) )

#define CLR8(reg)			(  *( (volatile uint8_t *) (reg) ) = (uint8_t)0 )
#define MASK8(reg, mask)		(  *( (volatile uint8_t *) (reg) ) & (uint8_t)(mask) )
#define UMSK8(reg, mask)		(  *( (volatile uint8_t *) (reg) ) & ~( (uint8_t)(mask) ) )

#define SETR8SHL(reg, mask, shift, v)	(  *( (volatile uint8_t *) (reg) ) = \
					( ( *( (volatile uint8_t *) (reg) ) & ~( (uint8_t)(mask) ) ) | \
					( ( (uint8_t)(v) << (shift) ) & (uint8_t)(mask)          ) ) )

#define SETR8(reg, mask)		(  *( (volatile uint8_t *) (reg) ) = \
					( ( *( (volatile uint8_t *) (reg) ) & ~( (uint8_t)(mask) ) ) | (uint8_t)(mask) ) )

#define CLRR8(reg, mask)		(  *( (volatile uint8_t *) (reg) ) &= ~( (uint8_t)(mask) ) )

#define SETB8(reg, bit)			(  *( (volatile uint8_t *) (reg) ) |= (uint8_t)( (uint8_t)1 << (bit) ) )
#define CLRB8(reg, bit)			(  *( (volatile uint8_t *) (reg) ) &= ( ~( (uint8_t) ( (uint8_t)1 << (bit) ) ) ) )
#define GETB8(reg, bit)			(  *( (volatile uint8_t *) (reg) ) & (uint8_t) ( (uint8_t)1 << (bit) ) )
#define GETB8SHR(reg, bit)		( (*( (volatile uint8_t *) (reg) ) & (uint8_t) ( (uint8_t)1 << (bit) )) >> (bit) )

/* 16 bit access */
#define IN16(reg)			(  *( (volatile uint16_t *) (reg) ) )
#define OUT16(reg, data)		( (*( (volatile uint16_t *) (reg) ) ) = (uint16_t)(data) )

#define CLR16(reg)			(  *( (volatile uint16_t *) (reg) ) = (uint16_t)0 )
#define MASK16(reg, mask)		(  *( (volatile uint16_t *) (reg) ) & (uint16_t)(mask) )
#define UMSK16(reg, mask)		(  *( (volatile uint16_t *) (reg) ) & ~( (uint16_t)(mask) ) )

#define SETR16SHL(reg, mask, shift, v)	(  *( (volatile uint16_t *) (reg) ) = \
					( ( *( (volatile uint16_t *) (reg) ) & ~( (uint16_t)(mask) ) ) | \
					( ( (uint16_t)(v) << (shift) ) & (uint16_t)(mask)          ) ) )

#define SETR16(reg, mask)		(  *( (volatile uint16_t *) (reg) ) = \
					( ( *( (volatile uint16_t *) (reg) ) & ~( (uint16_t)(mask) ) ) | (uint16_t)(mask) ) )

#define CLRR16(reg, mask)		(  *( (volatile uint16_t *) (reg) ) &= ~( (uint16_t)(mask) ) )

#define SETB16(reg, bit)		(  *( (volatile uint16_t *) (reg) ) |= (uint16_t)( (uint16_t)1 << (bit) ) )
#define CLRB16(reg, bit)		(  *( (volatile uint16_t *) (reg) ) &= ( ~( (uint16_t) ( (uint16_t)1 << (bit) ) ) ) )
#define GETB16(reg, bit)		(  *( (volatile uint16_t *) (reg) ) & (uint16_t) ( (uint16_t)1 << (bit) ) )
#define GETB16SHR(reg, bit)		( (*( (volatile uint16_t *) (reg) ) & (uint16_t) ( (uint16_t)1 << (bit) )) >> (bit) )

/* 32 bit access */
#define IN32(reg)			_IN32((uint32_t)(reg))
#define OUT32(reg, data)		_OUT32((uint32_t)(reg), (uint32_t)(data))

#define CLR32(reg)			_CLR32((uint32_t)(reg))
#define MASK32(reg, mask)		_MASK32((uint32_t)(reg), (uint32_t)(mask))
#define UMSK32(reg, mask)		_UMSK32((uint32_t)(reg), (uint32_t)(mask))

#define SETR32SHL(reg, mask, shift, v)	_SETR32SHL((uint32_t)(reg), (uint32_t)(mask), (uint32_t)(shift), (uint32_t)(v))
#define SETR32(reg, mask)		_SETR32((uint32_t)(reg), (uint32_t)(mask))
#define CLRR32(reg, mask)		_CLRR32((uint32_t)(reg), (uint32_t)(mask))

#define SETB32(reg, bit)		_SETB32((uint32_t)(reg), (uint32_t)(bit))
#define CLRB32(reg, bit)		_CLRB32((uint32_t)(reg), (uint32_t)(bit))
#define GETB32(reg, bit)		_GETB32((uint32_t)(reg), (uint32_t)(bit))
#define GETB32SHR(reg, bit)		_GETB32SHR((uint32_t)(reg), (uint32_t)(bit))

#else /* REG_IO_HACK */

/* 8 bit access */
//#define IN8(reg)			(  *( (volatile uint8_t *) (reg) ) )
#define OUT8(reg, data)			( (*( (volatile uint8_t *) (reg) ) ) = (uint8_t)(data) )

#define CLR8(reg)			(  *( (volatile uint8_t *) (reg) ) = (uint8_t)0 )
#define MASK8(reg, mask)		(  *( (volatile uint8_t *) (reg) ) & (uint8_t)(mask) )
#define UMSK8(reg, mask)		(  *( (volatile uint8_t *) (reg) ) & ~( (uint8_t)(mask) ) )

#define SETR8SHL(reg, mask, shift, v)	(  *( (volatile uint8_t *) (reg) ) = \
					( ( *( (volatile uint8_t *) (reg) ) & ~( (uint8_t)(mask) ) ) | \
					( ( (uint8_t)(v) << (shift) ) & (uint8_t)(mask)          ) ) )

#define SETR8(reg, mask)		(  *( (volatile uint8_t *) (reg) ) = \
					( ( *( (volatile uint8_t *) (reg) ) & ~( (uint8_t)(mask) ) ) | (uint8_t)(mask) ) )

#define CLRR8(reg, mask)		(  *( (volatile uint8_t *) (reg) ) &= ~( (uint8_t)(mask) ) )

#define SETB8(reg, bit)			(  *( (volatile uint8_t *) (reg) ) |= (uint8_t)( (uint8_t)1 << (bit) ) )
#define CLRB8(reg, bit)			(  *( (volatile uint8_t *) (reg) ) &= ( ~( (uint8_t) ( (uint8_t)1 << (bit) ) ) ) )
#define GETB8(reg, bit)			(  *( (volatile uint8_t *) (reg) ) & (uint8_t) ( (uint8_t)1 << (bit) ) )
#define GETB8SHR(reg, bit)		( (*( (volatile uint8_t *) (reg) ) & (uint8_t) ( (uint8_t)1 << (bit) )) >> (bit) )

/* 16 bit access */
#define IN16(reg)			(  *( (volatile uint16_t *) (reg) ) )
#define OUT16(reg, data)		( (*( (volatile uint16_t *) (reg) ) ) = (uint16_t)(data) )

#define CLR16(reg)			(  *( (volatile uint16_t *) (reg) ) = (uint16_t)0 )
#define MASK16(reg, mask)		(  *( (volatile uint16_t *) (reg) ) & (uint16_t)(mask) )
#define UMSK16(reg, mask)		(  *( (volatile uint16_t *) (reg) ) & ~( (uint16_t)(mask) ) )

#define SETR16SHL(reg, mask, shift, v)	(  *( (volatile uint16_t *) (reg) ) = \
					( ( *( (volatile uint16_t *) (reg) ) & ~( (uint16_t)(mask) ) ) | \
					( ( (uint16_t)(v) << (shift) ) & (uint16_t)(mask)          ) ) )

#define SETR16(reg, mask)		(  *( (volatile uint16_t *) (reg) ) = \
					( ( *( (volatile uint16_t *) (reg) ) & ~( (uint16_t)(mask) ) ) | (uint16_t)(mask) ) )
#define CLRR16(reg, mask)		(  *( (volatile uint16_t *) (reg) ) &= ~( (uint16_t)(mask) ) )

#define SETB16(reg, bit)		(  *( (volatile uint16_t *) (reg) ) |= (uint16_t)( (uint16_t)1 << (bit) ) )
#define CLRB16(reg, bit)		(  *( (volatile uint16_t *) (reg) ) &= ( ~( (uint16_t) ( (uint16_t)1 << (bit) ) ) ) )
#define GETB16(reg, bit)		(  *( (volatile uint16_t *) (reg) ) & (uint16_t) ( (uint16_t)1 << (bit) ) )
#define GETB16SHR(reg, bit)		( (*( (volatile uint16_t *) (reg) ) & (uint16_t) ( (uint16_t)1 << (bit) )) >> (bit) )

/* 32 bit access */
#define IN32(reg)			(  *( (volatile uint32_t *) (reg) ) )
#define OUT32(reg, data)		( (*( (volatile uint32_t *) (reg) ) ) = (uint32_t)(data) )

#define CLR32(reg)			(  *( (volatile uint32_t *) (reg) ) = (uint32_t)0 )
#define MASK32(reg, mask)		(  *( (volatile uint32_t *) (reg) ) & (uint32_t)(mask) )
#define UMSK32(reg, mask)		(  *( (volatile uint32_t *) (reg) ) & ~( (uint32_t)(mask) ) )

#define SETR32SHL(reg, mask, shift, v)	(  *( (volatile uint32_t *) (reg) ) = \
					( ( *( (volatile uint32_t *) (reg) ) & ~( (uint32_t)(mask) ) ) | \
					( ( (uint32_t)(v) << (shift) ) & (uint32_t)(mask)          ) ) )

#define SETR32(reg, mask)		(  *( (volatile uint32_t *) (reg) ) = \
					( ( *( (volatile uint32_t *) (reg) ) & ~( (uint32_t)(mask) ) ) | (uint32_t)(mask) ) )

#define CLRR32(reg, mask)		(  *( (volatile uint32_t *) (reg) ) &= ~( (uint32_t)(mask) ) )

#define SETB32(reg, bit)		(  *( (volatile uint32_t *) (reg) ) |= (uint32_t)( (uint32_t)1 << (bit) ) )
#define CLRB32(reg, bit)		(  *( (volatile uint32_t *) (reg) ) &= ( ~( (uint32_t) ( (uint32_t)1 << (bit) ) ) ) )
#define GETB32(reg, bit)		(  *( (volatile uint32_t *) (reg) ) & (uint32_t) ( (uint32_t)1 << (bit) ) )
#define GETB32SHR(reg, bit)		( (*( (volatile uint32_t *) (reg) ) & (uint32_t) ( (uint32_t)1 << (bit) )) >> (bit) )

#endif /* REG_IO_HACK */

#define SR_CLRB32(reg, bit)		\
{					\
	int mask = __nds32__mfsr(reg)& ~(1<<bit);\
        __nds32__mtsr(mask, reg);	 \
	__nds32__dsb();				 \
}

#define SR_SETB32(reg,bit)\
{\
	int mask = __nds32__mfsr(reg)|(1<<bit);\
	__nds32__mtsr(mask, reg);			\
	__nds32__dsb();						\
}


#endif /* __AE210P_REGS_INC__ */
