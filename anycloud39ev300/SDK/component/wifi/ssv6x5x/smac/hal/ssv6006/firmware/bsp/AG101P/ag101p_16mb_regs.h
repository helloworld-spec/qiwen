/*****************************************************************************
 *
 *            Copyright Andes Technology Corporation 2007-2008
 *                         All Rights Reserved.
 *
 *  Revision History:
 *
 *    Aug.21.2007     Created.
 ****************************************************************************/

#ifndef __AG101P_16MB_REGS_H__ 
#define __AG101P_16MB_REGS_H__ 

#ifndef __ASSEMBLER__
#include <inttypes.h>
#include <nds32_intrinsic.h>
#endif
/*****************************************************************************
 * IRQ Vector
 ****************************************************************************/
#if( NO_EXTERNAL_INT_CTL == 1 ) 
	#define IRQ_FRAMEWX_VECTOR		0	// unused
	#define IRQ_CFC_VECTOR			1	// unused
	#define IRQ_SSP_VECTOR			2
	#define IRQ_I2C_VECTOR			3
	#define IRQ_KBC_VECTOR			4	// local memory dma interrupt for EVIC
	#define IRQ_SDC_VECTOR			5
	#define IRQ_I2SAC97_VECTOR		6
	#define IRQ_UART4_VECTOR		7
	#define IRQ_PMU_VECTOR			8	// unused
	#define IRQ_SWI_VECTOR			9	
	#define IRQ_UART1_VECTOR		10	// unused
	#define IRQ_UART2_VECTOR		11
	#define IRQ_PM_VECTOR			12	// unused
							// performance monitor
	#define IRQ_GPIO_VECTOR			13
	#define IRQ_TIMER2_VECTOR		14	// unused
	#define IRQ_TIMER3_VECTOR		15	// unused
	#define IRQ_WDT_VECTOR			16
	#define IRQ_RTCALARM_VECTOR		17
	#define IRQ_RTCSECOND_VECTOR		18
	#define IRQ_TIMER1_VECTOR		19	// Timer1/2/3 
	#define IRQ_CLCD_VECTOR			20	// unused
							// CLCD(bus error accessing SDRAM)
	#define IRQ_DMA_VECTOR			21
	#define IRQ_IRDA1_VECTOR		22	// unused
	#define IRQ_IRDA2_VECTOR		23	// unused
	#define IRQ_APBBRIDGE_VECTOR		24
	#define IRQ_ENETMAC_VECTOR		25
	#define IRQ_USB_VECTOR			26	// unused
	#define IRQ_EXTINT4_VECTOR		27	// unused
	#define IRQ_EXTINT0_VECTOR		28	// unused
	#define IRQ_EXTINT1_VECTOR		29	// unused
	#define IRQ_EXTINT2_VECTOR		30	// unused
	#define IRQ_EXTINT3_VECTOR		31	// unused



	/* IRQ priority */
	#define LOWEST				3
	#define HIGHEST				0

	#define IRQ_FRAMEWX_PRI			LOWEST			// HW0
	#define IRQ_CFC_PRI			LOWEST			// HW1
	#define IRQ_SSP_PRI			LOWEST			// HW2
	#define IRQ_I2C_PRI			LOWEST			// HW3
	#define IRQ_KBC_PRI			LOWEST			// HW4
	#define IRQ_SDC_PRI			LOWEST			// HW5
	#define IRQ_I2SAC97_PRI			LOWEST			// HW6
	#define IRQ_UART4_PRI			LOWEST			// HW7
	#define IRQ_PMU_PRI			LOWEST			// HW8
	#define IRQ_SWI_PRI			LOWEST			// HW9
	#define IRQ_UART1_PRI			LOWEST			// HW10
	#define IRQ_UART2_PRI			LOWEST			// HW11
	#define IRQ_PM_PRI			LOWEST			// HW12
	#define IRQ_GPIO_PRI			2			// HW13
	#define IRQ_TIMER2_PRI			LOWEST			// HW14
	#define IRQ_TIMER3_PRI			LOWEST			// HW15

	#define IRQ_WDT_RPI			LOWEST			// HW16
	#define IRQ_RTCALARM_PRI		LOWEST			// HW17
	#define IRQ_RTCSECOND_RPI		LOWEST			// HW18
	#define IRQ_TIMER1_PRI			HIGHEST			// HW19
	#define IRQ_CLCD_PRI			LOWEST			// HW20
	#define IRQ_DMA_PRI			1			// HW21
	#define IRQ_IRDA1_PRI			LOWEST			// HW22
	#define IRQ_IRDA2_PRI			LOWEST			// HW23
	#define IRQ_APBBRIDGE_PRI		1			// HW24
	#define IRQ_ENETMAC_PRI			2			// HW25
	#define IRQ_USB_PRI			LOWEST			// HW26
	#define IRQ_EXTINT4_RPI			LOWEST			// HW27
	#define IRQ_EXTINT0_PRI			LOWEST			// HW28
	#define IRQ_EXTINT1_PRI			LOWEST			// HW29
	#define IRQ_EXTINT2_PRI			LOWEST			// HW30
	#define IRQ_EXTINT3_PRI			LOWEST			// HW31 
	
	#define PRI1_DEFAULT			(IRQ_FRAMEWX_PRI)			\
						| (IRQ_CFC_PRI 		<< 2)		\
						| (IRQ_SSP_PRI 		<< 4)		\
						| (IRQ_I2C_PRI 		<< 6)		\
						| (IRQ_KBC_PRI 		<< 8)		\
						| (IRQ_SDC_PRI 		<< 10)		\
						| (IRQ_I2SAC97_PRI 	<< 12)		\
						| (IRQ_UART4_PRI	<< 14)		\
						| (IRQ_PMU_PRI		<< 16)		\
						| (IRQ_SWI_PRI		<< 18)		\
						| (IRQ_UART1_PRI	<< 20)		\
						| (IRQ_UART2_PRI	<< 22)		\
						| (IRQ_PM_PRI		<< 24)		\
						| (IRQ_GPIO_PRI		<< 26)		\
						| (IRQ_TIMER2_PRI	<< 28)		\
						| (IRQ_TIMER3_PRI	<< 30)		
	
	#define PRI2_DEFAULT			(IRQ_WDT_RPI)				\
						| (IRQ_RTCALARM_PRI	<< 2)		\
						| (IRQ_RTCSECOND_RPI	<< 4)		\
						| (IRQ_TIMER1_PRI	<< 6)		\
						| (IRQ_CLCD_PRI		<< 8)		\
						| (IRQ_DMA_PRI		<< 10)		\
						| (IRQ_IRDA1_PRI	<< 12)		\
						| (IRQ_IRDA2_PRI	<< 14)		\
						| (IRQ_APBBRIDGE_PRI	<< 16)		\
						| (IRQ_ENETMAC_PRI	<< 18)		\
						| (IRQ_USB_PRI		<< 20)		\
						| (IRQ_EXTINT4_RPI	<< 22)		\
						| (IRQ_EXTINT0_PRI	<< 24)		\
						| (IRQ_EXTINT1_PRI	<< 26)		\
						| (IRQ_EXTINT2_PRI	<< 28)		\
						| (IRQ_EXTINT3_PRI	<< 30)	



#else
	#define IRQ_CFCCD_VECTOR		0
	#define IRQ_CFCDMA_VECTOR		1
	#define IRQ_SSP_VECTOR			2
	#define IRQ_I2C_VECTOR			3
	#define IRQ_RESERVE4_VECTOR		4
	#define IRQ_SDC_VECTOR			5
	#define IRQ_I2SAC97_VECTOR		6
	#define IRQ_STUART_VECTOR		7
	#define IRQ_PMU_VECTOR			8
	#define IRQ_RESERVE9_VECTOR		9
	#define IRQ_FTUART_VECTOR		10
	#define IRQ_BTUART_VECTOR		11
	#define IRQ_EXTINT5_VECTOR		12
	#define IRQ_GPIO_VECTOR			13
	#define IRQ_TIMER2_VECTOR		14
	#define IRQ_TIMER3_VECTOR		15
	#define IRQ_WDT_VECTOR			16
	#define IRQ_RTCALARM_VECTOR		17
	#define IRQ_RTCSECOND_VECTOR		18
	#define IRQ_TIMER1_VECTOR		19
	#define IRQ_RESERVE20_VECTOR		20
	#define IRQ_DMA_VECTOR			21
	#define IRQ_RESERVE22_VECTOR		22
	#define IRQ_RESERVE23_VECTOR		23
	#define IRQ_APBBRIDGE_VECTOR		24
	#define IRQ_ENETMAC_VECTOR		25
	#define IRQ_USB_VECTOR			26
	#define IRQ_EXTINT4_VECTOR		27
	#define IRQ_EXTINT0_VECTOR		28
	#define IRQ_EXTINT1_VECTOR		29
	#define IRQ_EXTINT2_VECTOR		30
	#define IRQ_EXTINT3_VECTOR		31

	#define PRI1_DEFAULT		0xffffffff
	#define PRI2_DEFAULT		0xffffffff
#endif

/*****************************************************************************
 * OSC - AG101P-16MB OSC registers
 ****************************************************************************/
#ifdef CONFIG_OSC_SUPPORT

#define OSC_CTRL_BASE                   0x00F17000

#define OSC_CTRL_OFFSET                 0x0
#define OSC_FIXED_REGION_SIZE_OFFSET    0x4
#define OSC_OVL_REGION_BASE_OFFSET      0x8
#define OSC_OVL_REGION_END_OFFSET       0xc
#define OSC_OVL_REGION_DMA_OFFSET       0x10

#define OSC_CTRL                        (OSC_CTRL_BASE + OSC_CTRL_OFFSET)
#define OSC_FIXED_REGION_SIZE           (OSC_CTRL_BASE + OSC_FIXED_REGION_SIZE_OFFSET)
#define OSC_OVL_REGION_BASE             (OSC_CTRL_BASE + OSC_OVL_REGION_BASE_OFFSET)
#define OSC_OVL_REGION_END              (OSC_CTRL_BASE + OSC_OVL_REGION_END_OFFSET)
#define OSC_OVL_REGION_DMA              (OSC_CTRL_BASE + OSC_OVL_REGION_DMA_OFFSET)

#define OSC_OVLY_PAGE_SIZE_MASK         0x07000000

#define OSC_OVLY_PAGE_SIZE              ((  *( (volatile uint32_t *) (OSC_CTRL) ) & (uint32_t)(OSC_OVLY_PAGE_SIZE_MASK) ) >> 12)

#define OSC_EILM_SIZE                   (0x8000*2)      // 32K ILM

#endif  // #ifdef CONFIG_OSC_SUPPORT

/*****************************************************************************
 * AHBC - AG101P-16MB AHB
 ****************************************************************************/
#define AHBC_BASE			0x00E00000 /* Device base address */

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


/*****************************************************************************
 * SMC - AG101P-16MB AHB
 ****************************************************************************/
#define SMC_BASE			0x00E01000 /* Device base address */


/*****************************************************************************
 * SDRAMC - AG101P-16MB AHB
 ****************************************************************************/
#define SDRAMC_BASE			0x00E02000 /* Device base address */


/*****************************************************************************
 * DMAC - AG101P-16MB AHB
 ****************************************************************************/
#define DMAC_BASE			0x00E03000 /* Device base address */

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


/*****************************************************************************
 * APBBR - AG101P-16MB AHB to APB Bridge
 ****************************************************************************/
#define APBBR_BASE			0x00E04000 /* Device base address */

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

/*****************************************************************************
 * LCDC - AG101P-16MB AHB
 ****************************************************************************/
#define LCDC_BASE			0x00E10000 /* Device base address */

/*LCDC registers (32-bit width) */
#define LCDC_LCD_TIMING0		(LCDC_BASE + 0x00) /* LCD horizontal timing control */
#define LCDC_LCD_TIMING1		(LCDC_BASE + 0x04) /* LCD vertical timing control */
#define LCDC_LCD_TIMING2		(LCDC_BASE + 0x08) /* LCD clock and signal polarity control */
#define LCDC_LCD_FRAMEBASE		(LCDC_BASE + 0x10) /* LCD panel frame base address */
#define LCDC_LCD_INTENABLE		(LCDC_BASE + 0x18) /* LCD interrupt enable mask */
#define LCDC_LCD_CONTROL		(LCDC_BASE + 0x1c) /* LCD panel pixel parameters */
#define LCDC_LCD_INTCLR			(LCDC_BASE + 0x20) /* LCD interrupt clear */
#define LCDC_LCD_INTMASK		(LCDC_BASE + 0x24) /* LCD masked interrupt */

#define LCDC_OSD_CONTROL0		(LCDC_BASE + 0x34) /* OSD scaling and dimension control */
#define LCDC_OSD_CONTROL1		(LCDC_BASE + 0x38) /* OSD position control */
#define LCDC_OSD_CONTROL2		(LCDC_BASE + 0x3c) /* OSD foreground color control */
#define LCDC_OSD_CONTROL3		(LCDC_BASE + 0x40) /* OSD background color control */
#define LCDC_GPIO_CONTROL		(LCDC_BASE + 0x44) /* GPIO control */

#define LCDC_LCD_PALETTE_WPORT		(LCDC_BASE + 0x200) /* LCD palette RAM write access port */
#define LCDC_LCD_PALETTE_WPORT_END	(LCDC_BASE + 0x400) /* LCD palette RAM write access port end address */
#define LCDC_OSD_FONT_WPORT		(LCDC_BASE + 0x8000) /* OSD font database write access port */
#define LCDC_OSD_FONT_WPORT_END		(LCDC_BASE + 0xc000) /* OSD font database write access port end address */
#define LCDC_OSD_ATTR_WPORT		(LCDC_BASE + 0xc000) /* OSD window attribute write access port */
#define LCDC_OSD_ATTR_WPORT_END		(LCDC_BASE + 0xc800) /* OSD window attribute write access port end address */

/*****************************************************************************
 * MAC - AG101P-16MB AHB
 ****************************************************************************/
#define MAC_BASE			0x00E05000 /* Device base address */

/* MAC registers (32-bit width) */
#define MAC_ISR				(MAC_BASE + 0x00) /* Interrupt Status Segister */
#define MAC_IME				(MAC_BASE + 0x04) /* Interrupt Enable Register */
#define MAC_MADR			(MAC_BASE + 0x08) /* MAC address most significant 2 bytes */
#define MAC_LADR			(MAC_BASE + 0x0c) /* MAC address least significant 4 bytes */
#define MAC_MAHT0			(MAC_BASE + 0x10) /* Multicast address hash table [31:0] */
#define MAC_MAHT1			(MAC_BASE + 0x14) /* Multicast address hash table [31:0] */
#define MAC_TXPD			(MAC_BASE + 0x18) /* Transmit poll demand register */
#define MAC_RXPD			(MAC_BASE + 0x1c) /* Receive poll demand register */
#define MAC_TXR_BADR			(MAC_BASE + 0x20) /* Transmit ring base address register */
#define MAC_RXR_BADR			(MAC_BASE + 0x24) /* Receive ring base address register */
#define MAC_ITC				(MAC_BASE + 0x28) /* Interrupt timer control register */
#define MAC_APTC			(MAC_BASE + 0x2c) /* Automatic polling timer control register */
#define MAC_DBLAC			(MAC_BASE + 0x30) /* DMA burst length and arbitration control register */
#define MAC_MACCR			(MAC_BASE + 0x88) /* MAC control register */
#define MAC_MACSR			(MAC_BASE + 0x8c) /* MAC status register */
#define MAC_PHYCR			(MAC_BASE + 0x90) /* PHY control register */
#define MAC_PHYWDATA			(MAC_BASE + 0x94) /* PHY write data register */
#define MAC_FCR				(MAC_BASE + 0x98) /* Flow control register */
#define MAC_BPR				(MAC_BASE + 0x9c) /* Back pressure register */
#define MAC_WOLCR			(MAC_BASE + 0xa0) /* Wake-On-Lan control register */
#define MAC_WOLSR			(MAC_BASE + 0xa4) /* Wake-On-Lan status register */
#define MAC_WFCRC			(MAC_BASE + 0xa8) /* Wakeup frame CRC register */
#define MAC_WFBM1			(MAC_BASE + 0xb0) /* Wakeup frame byte mask 1'st double word register */
#define MAC_WFBM2			(MAC_BASE + 0xb4) /* Wakeup frame byte mask 2'nd double word register */
#define MAC_WFBM3			(MAC_BASE + 0xb8) /* Wakeup frame byte mask 3'rd double word register */
#define MAC_WFBM4			(MAC_BASE + 0xbc) /* Wakeup frame byte mask 4'th double word register */
#define MAC_TS				(MAC_BASE + 0xc4) /* Test seed register */
#define MAC_DMAFIFOS			(MAC_BASE + 0xc8) /* DMA FIFO state register */
#define MAC_TM				(MAC_BASE + 0xcc) /* Test mode register */
#define MAC_TX_MCOL			(MAC_BASE + 0xd4) /* TX_MCOL & TX_SCOL counter register */
#define MAC_RPF				(MAC_BASE + 0xd8) /* RPF & AEP counter register */
#define MAC_XM				(MAC_BASE + 0xdc) /* XM(tx fail, late col, col>=16) & PG(tx fail, col>=16) counter register */
#define MAC_RUNT_CNT			(MAC_BASE + 0xe0) /* RUNT_CNT(rx runt) & TLCC(late collision) counter register */
#define MAC_CRCER_CNT			(MAC_BASE + 0xe4) /* CRCER_CNT(crc err) & FTL_CNT(ftl, >1518) counter register */
#define MAC_RLC				(MAC_BASE + 0xe8) /* RLC(rx lost) & RCC(rx collision) counter register */
#define MAC_BROC			(MAC_BASE + 0xec) /* BROC(broadcast rx) counter register */
#define MAC_MULCA			(MAC_BASE + 0xf0) /* MULCA(multicast rx) counter register */
#define MAC_RP				(MAC_BASE + 0xf4) /* RP(rx ok) counter register */
#define MAC_XP				(MAC_BASE + 0xf8) /* XP(tx ok) counter register */

/*****************************************************************************
 * USB - AG101P-16MB AHB
 ****************************************************************************/
#define USBC_BASE			0x00E06000 /* Device base address */

/*****************************************************************************
 * PMU - AG101P-16MB Core APB
 ****************************************************************************/
#define PMU_BASE			0x00F01000 /* Device base address */

/* PMU registers (32-bit width) */
#define PMU_IDNMBR0			(PMU_BASE + 0x00) /* ID number 0 register */
#define PMU_FSSR			(PMU_BASE + 0x04) /* Frequency scaling status register */
#define PMU_OSCCR			(PMU_BASE + 0x08) /* OSC control register */
#define PMU_PMODE			(PMU_BASE + 0x0c) /* Power mode register */
#define PMU_PMCR			(PMU_BASE + 0x10) /* Power manager control register */
#define PMU_PED				(PMU_BASE + 0x14) /* Power manager edge detect register */
#define PMU_PEDSR			(PMU_BASE + 0x18) /* Power manager edge detect status register */
#define PMU_PMSR			(PMU_BASE + 0x20) /* Power manager status register */
#define PMU_PGSR			(PMU_BASE + 0x24) /* Power manager GPIO sleep state register */
#define PMU_MFPSR			(PMU_BASE + 0x28) /* Multi-function port setting register */
#define PMU_MISC			(PMU_BASE + 0x2c) /* Misc register */

#define PMU_PDLLCR0			(PMU_BASE + 0x30) /* PLL/DLL control register 0 */
#define PMU_PDLLCR1			(PMU_BASE + 0x34) /* PLL/DLL control register 1 */
#define PMU_AHBMCLKOFF			(PMU_BASE + 0x38) /* AHB module clock off control register */
#define PMU_APBMCLKOFF			(PMU_BASE + 0x3c) /* APB module clock off control register */

#define PMU_DCSRCR0			(PMU_BASE + 0x40) /* Driving capability and slew rate control register 0 */
#define PMU_DCSRCR1			(PMU_BASE + 0x44) /* Driving capability and slew rate control register 1 */
#define PMU_DCSRCR2			(PMU_BASE + 0x48) /* Driving capability and slew rate control register 2 */

#define PMU_SDRAMHTC			(PMU_BASE + 0x4c) /* SDRAM signal hold time control */

#define PMU_SCRATCHPAD_BASE		(PMU_BASE + 0x50) /* Scratch pad registers to hold data in sleep mode */
#define PMU_SCRATCHPAD_END		(PMU_BASE + 0x90)

#define PMU_SCRATCHPAD(n)		(PMU_SCRATCHPAD_BASE + ((uint32_t)(n) << 2)) /* n = 0 ~ 15 */

#define PMU_SCRATCHPAD0			(PMU_BASE + 0x50)
#define PMU_SCRATCHPAD1			(PMU_BASE + 0x54)
#define PMU_SCRATCHPAD2			(PMU_BASE + 0x58)
#define PMU_SCRATCHPAD3			(PMU_BASE + 0x5c)
#define PMU_SCRATCHPAD4			(PMU_BASE + 0x60)
#define PMU_SCRATCHPAD5			(PMU_BASE + 0x64)
#define PMU_SCRATCHPAD6			(PMU_BASE + 0x68)
#define PMU_SCRATCHPAD7			(PMU_BASE + 0x6c)
#define PMU_SCRATCHPAD8			(PMU_BASE + 0x70)
#define PMU_SCRATCHPAD9			(PMU_BASE + 0x74)
#define PMU_SCRATCHPAD10		(PMU_BASE + 0x78)
#define PMU_SCRATCHPAD11		(PMU_BASE + 0x7c)
#define PMU_SCRATCHPAD12		(PMU_BASE + 0x80)
#define PMU_SCRATCHPAD13		(PMU_BASE + 0x84)
#define PMU_SCRATCHPAD14		(PMU_BASE + 0x88)
#define PMU_SCRATCHPAD15		(PMU_BASE + 0x8c)
#define PMU_AHBDMA_REQACK		(PMU_BASE + 0x90) /* AHB DMA REQ/ACK connection configuration status register */
#define PMU_JMP_STATUS			(PMU_BASE + 0x9c) /* External jummper setting status register */

#define PMU_CFC_REQACK_CFG		(PMU_BASE + 0xa0) /* CFC REQ/ACK connection configuration register */
#define PMU_SSP1_REQACK_CFG		(PMU_BASE + 0xa4) /* SSP1 REQ/ACK connection configuration register */
#define PMU_UART1TX_REQACK_CFG		(PMU_BASE + 0xa8) /* UART1 TX REQ/ACK connection configuration register */
#define PMU_UART1RX_REQACK_CFG		(PMU_BASE + 0xac) /* UART1 RX REQ/ACK connection configuration register */
#define PMU_UART2TX_REQACK_CFG		(PMU_BASE + 0xb0) /* UART2 TX REQ/ACK connection configuration register */
#define PMU_UART2RX_REQACK_CFG		(PMU_BASE + 0xb4) /* UART2 RX REQ/ACK connection configuration register */
#define PMU_SDC_REQACK_CFG		(PMU_BASE + 0xb8) /* SDC REQ/ACK connection configuration register */
#define PMU_I2SAC97_REQACK_CFG		(PMU_BASE + 0xbc) /* I2S/AC97 REQ/ACK connection configuration register */
#define PMU_USB_REQACK_CFG		(PMU_BASE + 0xc8) /* USB 2.0 REQ/ACK connection configuration register */
#define PMU_EXT0_REQACK_CFG		(PMU_BASE + 0xd4) /* External device0 REQ/ACK connection configuration register */
#define PMU_EXT1_REQACK_CFG		(PMU_BASE + 0xd8) /* External device1 REQ/ACK connection configuration register */


/*****************************************************************************
 * Timer - AG101P-16MB Core APB
 ****************************************************************************/
#define TMRC_BASE			0x00F04000 /* Device base address */

/* TMRC registers (32-bit width) */
#define TMX_OFFSET(n)			(0x10*(n-1))
#define TMRC_TM_COUNTER(n)		(TMRC_BASE + 0x00 + TMX_OFFSET(n)) /* Timer n counter */
#define TMRC_TM_LOAD(n)			(TMRC_BASE + 0x04 + TMX_OFFSET(n)) /* Timer n auto reload value */
#define TMRC_TM_MATCH1(n)		(TMRC_BASE + 0x08 + TMX_OFFSET(n)) /* Timer n match value */
#define TMRC_TM_MATCH2(n)		(TMRC_BASE + 0x0c + TMX_OFFSET(n)) /* Timer n match value */


#define TMRC_TMCR			(TMRC_BASE + 0x30) /* Timer control register */
#define TMRC_INTSTATE			(TMRC_BASE + 0x34) /* Timer interrupt state */
#define TMRC_INTMASK			(TMRC_BASE + 0x38) /* Timer interrupt mask */

/*****************************************************************************
 * WDT - AG101P-16MB Core APB
 ****************************************************************************/
#define WDTC_BASE			0x00F05000 /* Device base address */


/*****************************************************************************
 * RTC - AG101P-16MB Core APB
 ****************************************************************************/
#define RTC_BASE			0x00F06000 /* Device base address */

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


/*****************************************************************************
 * GPIO - AG101P-16MB Core APB
 ****************************************************************************/
#define GPIOC_BASE			0x00F07000 /* Device base address */

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

/*****************************************************************************
 * INTC - AG101P-16MB Core APB
 ****************************************************************************/
#define INTC_BASE			0x00F08000 /* Device base address */

/* INTC registers (32-bit width) */
#define INTC_HW1_SRC			(INTC_BASE + 0x00) /* (ro) HW1 interrupt status register brefore masking */
#define INTC_HW1_ER			(INTC_BASE + 0x04) /* (rw) HW1 interrupt enable register */
#define INTC_HW1_CLR			(INTC_BASE + 0x08) /* (wo) HW1 interrupt clear register */
#define INTC_HW1_TMR			(INTC_BASE + 0x0c) /* (rw) HW1 interrupt trigger mode register (0: level, 1: edge) */
#define INTC_HW1_TLR			(INTC_BASE + 0x10) /* (rw) HW1 interrupt trigger level register (0: assert high, 1: assert low) */
#define INTC_HW1_STATUS			(INTC_BASE + 0x14) /* (ro) HW1 interrupt status register after masking */

#define INTC_HW0_SRC			(INTC_BASE + 0x20) /* (ro) HW0 interrupt status register brefore masking */
#define INTC_HW0_ER			(INTC_BASE + 0x24) /* (rw) HW0 interrupt enable register */
#define INTC_HW0_CLR			(INTC_BASE + 0x28) /* (wo) HW0 interrupt clear register */
#define INTC_HW0_TMR			(INTC_BASE + 0x2c) /* (rw) HW0 interrupt trigger mode register */
#define INTC_HW0_TLR			(INTC_BASE + 0x30) /* (rw) HW0 interrupt trigger level register */
#define INTC_HW0_STATUS			(INTC_BASE + 0x34) /* (ro) HW0 interrupt status register after masking */

/*****************************************************************************
 * I2C - AG101P-16MB Core APB
 ****************************************************************************/
#define I2C_BASE			0x00F0A000 /* Device base address */


/*****************************************************************************
 * SSP - AG101P-16MB DMA APB (SSP1)
 ****************************************************************************/
#define SSPC_BASE			0x00F0B000 /* (SSP1) Device base address */

/* SSP1 registers */
#define SSPC_CR0			(SSPC_BASE + 0x00) /* SSP1 control register 0 */
#define SSPC_CR1			(SSPC_BASE + 0x04) /* SSP1 control register 1 */
#define SSPC_CR2			(SSPC_BASE + 0x08) /* SSP1 control register 2 */
#define SSPC_SR				(SSPC_BASE + 0x0c) /* SSP1 status register */
#define SSPC_INTCR			(SSPC_BASE + 0x10) /* SSP1 interrupt control register */
#define SSPC_INTSR			(SSPC_BASE + 0x14) /* SSP1 interrupt status register */
#define SSPC_DR				(SSPC_BASE + 0x18) /* SSP1 data register */
#define SSPC_ACLINK			(SSPC_BASE + 0x20) /* SSP1 AC-Link slot valid register */

/* SSP register offset for both SSP1 & SSP2 */
#define SSPC_CR0_OFFSET			0x00 /* SSP1/SSP2 control register 0 */
#define SSPC_CR1_OFFSET			0x04 /* SSP1/SSP2 control register 1 */
#define SSPC_CR2_OFFSET			0x08 /* SSP1/SSP2 control register 2 */
#define SSPC_SR_OFFSET			0x0c /* SSP1/SSP2 status register */
#define SSPC_INTCR_OFFSET		0x10 /* SSP1/SSP2 interrupt control register */
#define SSPC_INTSR_OFFSET		0x14 /* SSP1/SSP2 interrupt status register */
#define SSPC_DR_OFFSET			0x18 /* SSP1/SSP2 data register */
#define SSPC_ACLINK_OFFSET		0x20 /* SSP1/SSP2 AC-Link slot valid register */


/*****************************************************************************
 * CFC - AG101P-16MB DMA APB
 ****************************************************************************/
#define CFC_BASE			0x00F0D000 /* Device base address */

/* CFC registers */
#define CFC_HSR				(CFC_BASE + 0x00) /* CF host status register */
#define CFC_HCR				(CFC_BASE + 0x04) /* CF host control register */
#define CFC_ATCR			(CFC_BASE + 0x08) /* Access timing configuration register */
#define CFC_ABCR			(CFC_BASE + 0x0c) /* Active buffer controller register */
#define CFC_ABDR			(CFC_BASE + 0x10) /* Active buffer data register */
#define CFC_MSR				(CFC_BASE + 0x14) /* Multi-sector register */
#define CFC_MER				(CFC_BASE + 0x18) /* Transfer size mode2 enable register */
#define CFC_MCR				(CFC_BASE + 0x1c) /* Transfer size mode2 counter register */


/*****************************************************************************
 * MMC/SDC - AG101P-16MB DMA APB
 ****************************************************************************/
#define SDC_BASE			0x00F0E000 /* Device base address */

/* SDC registers */
#define SDC_CMD				(SDC_BASE + 0x00) /* Command register */
#define SDC_CMD_ARG			(SDC_BASE + 0x04) /* Argument register */
#define SDC_R0				(SDC_BASE + 0x08) /* Response register 0 */
#define SDC_R1				(SDC_BASE + 0x0c) /* Response register 1 */
#define SDC_R2				(SDC_BASE + 0x10) /* Response register 2 */
#define SDC_R3				(SDC_BASE + 0x14) /* Response register 3 */
#define SDC_RSP_CMD			(SDC_BASE + 0x18) /* Responded command register */
#define SDC_DATA_CR			(SDC_BASE + 0x1c) /* Data control register */
#define SDC_DATA_TIMER			(SDC_BASE + 0x20) /* Data timer (timeout) register */
#define SDC_DATA_LEN			(SDC_BASE + 0x24) /* Data length register */
#define SDC_STATUS			(SDC_BASE + 0x28) /* Status register */
#define SDC_CLEAR			(SDC_BASE + 0x2c) /* Clear register */

/* (note: spec "mask" means "enable", spec uses wrong term to describe this register) */
#define SDC_MASK			(SDC_BASE + 0x30) /* Interrupt mask(enable) register */
#define SDC_PWR_CTL			(SDC_BASE + 0x34) /* Power control register */
#define SDC_CLK_CTL			(SDC_BASE + 0x38) /* Clock control register */
#define SDC_BUS_WIDTH			(SDC_BASE + 0x3c) /* Bus width register */
#define SDC_DATA_WIN			(SDC_BASE + 0x40) /* Data window register */


/*****************************************************************************
 * PWM - AG101P-16MB Core APB
 ****************************************************************************/
#define PWM_BASE			0x00F11000 /* Device base address */


/*****************************************************************************
 * I2S/AC97 - AG101P-16MB DMA APB (SSP2)
 ****************************************************************************/
#define I2SAC97_BASE			0x00F14000 /* (SSP2) Device base address */

/* I2S/AC97 registers */
#define I2SAC97_CR0			(I2SAC97_BASE + 0x00) /* SSP2 control register 0 */
#define I2SAC97_CR1			(I2SAC97_BASE + 0x04) /* SSP2 control register 1 */
#define I2SAC97_CR2			(I2SAC97_BASE + 0x08) /* SSP2 control register 2 */
#define I2SAC97_SR			(I2SAC97_BASE + 0x0c) /* SSP2 status register */
#define I2SAC97_INTCR			(I2SAC97_BASE + 0x10) /* SSP2 interrupt control register */
#define I2SAC97_INTSR			(I2SAC97_BASE + 0x14) /* SSP2 interrupt status register */
#define I2SAC97_DR			(I2SAC97_BASE + 0x18) /* SSP2 data register */
#define I2SAC97_ACLINK			(I2SAC97_BASE + 0x20) /* SSP2 AC-Link slot valid register */

/*****************************************************************************
 * UARTC - AG101P-16MB DMA APB
 ****************************************************************************/
/* Device base address */
#define STUARTC_BASE			0x00F16000 /* standard/IR UART */
#define BTUARTC_BASE			0x00F03000 /* blue tooth UART */

/* UART register offsets (4~8-bit width) */
/* SD_LCR_DLAB == 0 */
#define UARTC_RBR_OFFSET		0x00 /* receiver biffer register */
#define UARTC_THR_OFFSET		0x00 /* transmitter holding register */
#define UARTC_IER_OFFSET		0x04 /* interrupt enable register */
#define UARTC_IIR_OFFSET		0x08 /* interrupt identification register */
#define UARTC_FCR_OFFSET		0x08 /* FIFO control register */
#define UARTC_LCR_OFFSET		0x0c /* line control regitser */
#define UARTC_MCR_OFFSET		0x10 /* modem control register */
#define UARTC_LSR_OFFSET		0x14 /* line status register */
#define UARTC_TST_OFFSET		0x14 /* testing register */
#define UARTC_MSR_OFFSET		0x18 /* modem status register */
#define UARTC_SPR_OFFSET		0x1c /* scratch pad register */

/* SD_LCR_DLAB == 0 */
#define UARTC_DLL_OFFSET		0x00 /* baudrate divisor latch LSB */
#define UARTC_DLM_OFFSET		0x04 /* baudrate divisor latch MSB */
#define UARTC_PSR_OFFSET		0x08 /* prescaler register */

#define UARTC_MDR_OFFSET		0x20 /* mode definition register */
#define UARTC_ACR_OFFSET		0x24 /* auxiliary control register */


/*****************************************************************************
 * Macros for Register Access
 ****************************************************************************/

#define REG32(reg)		(  *( (volatile uint32_t *) (reg) ) )

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


#endif /* __AG101P_16MB_REGS_INC__ */
