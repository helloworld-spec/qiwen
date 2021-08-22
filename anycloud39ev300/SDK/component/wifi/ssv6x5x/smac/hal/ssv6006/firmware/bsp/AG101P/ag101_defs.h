/*****************************************************************************
 *
 *            Copyright Andes Technology Corporation 2007-2008
 *                         All Rights Reserved.
 *
 *  Revision History:
 *
 *    Aug.21.2007     Created.
 ****************************************************************************/

#ifndef __AG101_DEFS_H__
#define __AG101_DEFS_H__

/*****************************************************************************
 * AHBC - AG101 AHB
 ****************************************************************************/

/* AHB slave#n (n = 0~7, 9, 12~15, 17~19, 21~22) base/size register */
#define AHBC_SLAVE_SIZE_SHIFT		16
#define AHBC_SLAVE_BASE_SHIFT		20
#define AHBC_SLAVE_SIZE_MASK		0x000f0000 /* Size of address space */
#define AHBC_SLAVE_BASE_MASK		0xfff00000 /* Base address (12 bits, should be (integer multiples of SIZE)) */

#define AHBC_SLAVE_SIZE_1M		0
#define AHBC_SLAVE_SIZE_2M		1
#define AHBC_SLAVE_SIZE_4M		2
#define AHBC_SLAVE_SIZE_8M		3
#define AHBC_SLAVE_SIZE_16M		4
#define AHBC_SLAVE_SIZE_32M		5
#define AHBC_SLAVE_SIZE_64M		6
#define AHBC_SLAVE_SIZE_128M		7
#define AHBC_SLAVE_SIZE_256M		8
#define AHBC_SLAVE_SIZE_512M		9
#define AHBC_SLAVE_SIZE_1G		10
#define AHBC_SLAVE_SIZE_2G		11

/* AHB master#n (n = 1~9) priority level (+0x80) */
#define AHBC_PRICR_PLEVEL_SHIFT		1
#define AHBC_PRICR_PLEVEL_MASK		0x000003fe /* Bit#n represent level of master#n */

/* AHB transfer control register (+0x84) */
#define AHBC_TRANSCR_CTL_BIT		1
#define AHBC_TRANSCR_CTL_MASK		0x00000002 /* Burst transfer interrupt */

/* AHB interrupt control register (+0x88) */
#define AHBC_INTCR_REMAP_BIT		0
#define AHBC_INTCR_INTS_BIT		16
#define AHBC_INTCR_RESPONSE_SHIFT	20
#define AHBC_INTCR_STS_BIT		24
#define AHBC_INTCR_REMAP_MASK		0x00000001 /* Remap (swap) base/size configuration of AHB slave 4/6 (SRAM_ROM_FLASH/SDRAM) */
#define AHBC_INTCR_INTS_MASK		0x00010000 /* Interrupt mask */
#define AHBC_INTCR_RESPONSE_MASK	0x00300000 /* Response status */
#define AHBC_INTCR_STS_MASK		0x01000000 /* Interrupt status */

#define AHBC_INTCR_RESPONSE_OK		0
#define AHBC_INTCR_RESPONSE_ERROR	1
#define AHBC_INTCR_RESPONSE_RETRY	2
#define AHBC_INTCR_RESPONSE_SPLIT	3


/*****************************************************************************
 * DMAC - AG101 AHB
 ****************************************************************************/

/* Interrupt status register (+00) */
#define DMAC_INT0_BIT			0
#define DMAC_INT1_BIT			1
#define DMAC_INT2_BIT			2
#define DMAC_INT3_BIT			3
#define DMAC_INT0_MASK			0x01 /* Status of the DMA channel 0 interrupt status */
#define DMAC_INT1_MASK			0x02 /* Status of the DMA channel l interrupt status */
#define DMAC_INT2_MASK			0x04 /* Status of the DMA channel 2 interrupt status */
#define DMAC_INT3_MASK			0x08 /* Status of the DMA channel 3 interrupt status */

/* Interrupt for terminal count status register (+0x04) */
#define DMAC_INT_TC_MASK		0x0f
#define DMAC_INT_TC_SHIFT		0

#define DMAC_INT_TC0_BIT		0
#define DMAC_INT_TC1_BIT		1
#define DMAC_INT_TC2_BIT		2
#define DMAC_INT_TC3_BIT		3
#define DMAC_INT_TC0_MASK		0x01 /* Status of the DMA channel 0 terminal count interrupt status */
#define DMAC_INT_TC1_MASK		0x02 /* Status of the DMA channel l terminal count interrupt status */
#define DMAC_INT_TC2_MASK		0x04 /* Status of the DMA channel 2 terminal count interrupt status */
#define DMAC_INT_TC3_MASK		0x08 /* Status of the DMA channel 3 terminal count interrupt status */


/* Interrupt for terminal count clear register (+0x08) */
#define DMAC_INT_TC_CLR_SHIFT		0
#define DMAC_INT_TC_CLR_MASK		0x0f

#define DMAC_INT_TC0_CLR_BIT		0
#define DMAC_INT_TC1_CLR_BIT		1
#define DMAC_INT_TC2_CLR_BIT		2
#define DMAC_INT_TC3_CLR_BIT		3
#define DMAC_INT_TC0_CLR_MASK		0x01 /* DMA channel 0 terminal count interrupt status clear */
#define DMAC_INT_TC1_CLR_MASK		0x02 /* DMA channel 1 terminal count interrupt status clear */
#define DMAC_INT_TC2_CLR_MASK		0x04 /* DMA channel 2 terminal count interrupt status clear */
#define DMAC_INT_TC3_CLR_MASK		0x08 /* DMA channel 3 terminal count interrupt status clear */

/* Interrupt for error/abort status register (+0x0c, 32-bits width) */
#define DMAC_INT_ERR_SHIFT		0
#define DMAC_INT_ABT_SHIFT		16
#define DMAC_INT_ERR_MASK		0x0000000f
#define DMAC_INT_ABT_MASK		0x000f0000

#define DMAC_INT_ERR0_BIT		0
#define DMAC_INT_ERR1_BIT		1
#define DMAC_INT_ERR2_BIT		2
#define DMAC_INT_ERR3_BIT		3
#define DMAC_INT_ERR0_MASK		0x00000001 /* DMA channel 0 error interrupt status */
#define DMAC_INT_ERR1_MASK		0x00000002 /* DMA channel 1 error interrupt status */
#define DMAC_INT_ERR2_MASK		0x00000004 /* DMA channel 2 error interrupt status */
#define DMAC_INT_ERR3_MASK		0x00000008 /* DMA channel 3 error interrupt status */

#define DMAC_INT_ABT0_BIT		16
#define DMAC_INT_ABT1_BIT		17
#define DMAC_INT_ABT2_BIT		18
#define DMAC_INT_ABT3_BIT		19
#define DMAC_INT_ABT0_MASK		0x00010000 /* DMA channel 0 abort interrupt status */
#define DMAC_INT_ABT1_MASK		0x00020000 /* DMA channel 1 abort interrupt status */
#define DMAC_INT_ABT2_MASK		0x00040000 /* DMA channel 2 abort interrupt status */
#define DMAC_INT_ABT3_MASK		0x00080000 /* DMA channel 3 abort interrupt status */

/* Interrupt for error/abort status clear register (+0x10, 32-bits width) */
#define DMAC_INT_ERR_CLR_SHIFT		0
#define DMAC_INT_ABT_CLR_SHIFT		16
#define DMAC_INT_ERR_CLR_MASK		0x0000000f
#define DMAC_INT_ABT_CLR_MASK		0x000f0000

#define DMAC_INT_ERR0_CLR_BIT		0
#define DMAC_INT_ERR1_CLR_BIT		1
#define DMAC_INT_ERR2_CLR_BIT		2
#define DMAC_INT_ERR3_CLR_BIT		3
#define DMAC_INT_ERR0_CLR_MASK		0x00000001 /* DMA channel 0 error interrupt status clear */
#define DMAC_INT_ERR1_CLR_MASK		0x00000002 /* DMA channel 1 error interrupt status clear */
#define DMAC_INT_ERR2_CLR_MASK		0x00000004 /* DMA channel 2 error interrupt status clear */
#define DMAC_INT_ERR3_CLR_MASK		0x00000008 /* DMA channel 3 error interrupt status clear */

#define DMAC_INT_ABT0_CLR_BIT		16
#define DMAC_INT_ABT1_CLR_BIT		17
#define DMAC_INT_ABT2_CLR_BIT		18
#define DMAC_INT_ABT3_CLR_BIT		19
#define DMAC_INT_ABT0_CLR_MASK		0x00010000 /* DMA channel 0 abort interrupt status clear */
#define DMAC_INT_ABT1_CLR_MASK		0x00020000 /* DMA channel 1 abort interrupt status clear */
#define DMAC_INT_ABT2_CLR_MASK		0x00040000 /* DMA channel 2 abort interrupt status clear */
#define DMAC_INT_ABT3_CLR_MASK		0x00080000 /* DMA channel 3 abort interrupt status clear */


/* Terminal count status register (+0x14) */
#define DMAC_TC0_BIT			0
#define DMAC_TC1_BIT			1
#define DMAC_TC2_BIT			2
#define DMAC_TC3_BIT			3

#define DMAC_TC0_MASK			0x01 /* DMA channel 0 terminal count status */
#define DMAC_TC1_MASK			0x02 /* DMA channel 1 terminal count status */
#define DMAC_TC2_MASK			0x04 /* DMA channel 2 terminal count status */
#define DMAC_TC3_MASK			0x08 /* DMA channel 3 terminal count status */
/* Error/abort status register (+0x18, 32-bits width) */
#define DMAC_ERR0_BIT			0
#define DMAC_ERR1_BIT			1
#define DMAC_ERR2_BIT			2
#define DMAC_ERR3_BIT			3
#define DMAC_ERR0_MASK			0x00000001 /* DMA channel 0 error status */
#define DMAC_ERR1_MASK			0x00000002 /* DMA channel 1 error status */
#define DMAC_ERR2_MASK			0x00000004 /* DMA channel 2 error status */
#define DMAC_ERR3_MASK			0x00000008 /* DMA channel 3 error status */

#define DMAC_ABT0_BIT			16
#define DMAC_ABT1_BIT			17
#define DMAC_ABT2_BIT			18
#define DMAC_ABT3_BIT			19
#define DMAC_ABT0_MASK			0x00010000 /* DMA channel 0 abort status */
#define DMAC_ABT1_MASK			0x00020000 /* DMA channel 1 abort status */
#define DMAC_ABT2_MASK			0x00040000 /* DMA channel 2 abort status */
#define DMAC_ABT3_MASK			0x00080000 /* DMA channel 3 abort status */

/* Channel enable status register (+0x1c) */
#define DMAC_CH0_EN_BIT			0
#define DMAC_CH1_EN_BIT			1
#define DMAC_CH2_EN_BIT			2
#define DMAC_CH3_EN_BIT			3
#define DMAC_CH0_EN_MASK		0x01 /* DMA channel 0 CSR CH_EN status */
#define DMAC_CH1_EN_MASK		0x02 /* DMA channel 1 CSR CH_EN status */
#define DMAC_CH2_EN_MASK		0x04 /* DMA channel 2 CSR CH_EN status */
#define DMAC_CH3_EN_MASK		0x08 /* DMA channel 3 CSR CH_EN status */

/* Channel busy status register (+0x20) */
#define DMAC_CH0_BUSY_BIT		0
#define DMAC_CH1_BUSY_BIT		1
#define DMAC_CH2_BUSY_BIT		2
#define DMAC_CH3_BUSY_BIT		3
#define DMAC_CH0_BUSY_MASK		0x01 /* DMA channel 0 CFG BUSY status */
#define DMAC_CH1_BUSY_MASK		0x02 /* DMA channel 1 CFG BUSY status */
#define DMAC_CH2_BUSY_MASK		0x04 /* DMA channel 2 CFG BUSY status */
#define DMAC_CH3_BUSY_MASK		0x08 /* DMA channel 3 CFG BUSY status */

/* Main configuration status register (+0x24) */
#define DMAC_DMACEN_BIT			0
#define DMAC_M0ENDIAN_BIT		1
#define DMAC_M1ENDIAN_BIT		2
#define DMAC_DMACEN_MASK		0x01 /* DMA controller enable */
#define DMAC_M0ENDIAN_MASK		0x02 /* Master 0 endianness configuration (0: LE, 1: BE) */
#define DMAC_M1ENDIAN_MASK		0x04 /* Master 1 endianness configuration (0: LE, 1: BE) */

#define DMAC_ENDIAN_LITTLE		0
#define DMAC_ENDIAN_BIG			1

/* Sync register (+0x28) */
#define DMAC_SYNC0_BIT			0
#define DMAC_SYNC1_BIT			1
#define DMAC_SYNC2_BIT			2
#define DMAC_SYNC3_BIT			3
#define DMAC_SYNC0_MASK			0x01 /* DMA channel 0 synchronization logic enable */
#define DMAC_SYNC1_MASK			0x02 /* DMA channel 1 synchronization logic enable */
#define DMAC_SYNC2_MASK			0x04 /* DMA channel 2 synchronization logic enable */
#define DMAC_SYNC3_MASK			0x08 /* DMA channel 3 synchronization logic enable */

/* DMA channel 0~3 Control Registers (CH[n]_BASE + 0x00) */
#define DMAC_CSR_CH_EN_MASK		0x00000001 /* Channel enable / kickoff */
#define DMAC_CSR_CH_EN_BIT		0

#define DMAC_CSR_DST_SEL_BIT		1
#define DMAC_CSR_SRC_SEL_BIT		2
#define DMAC_CSR_DSTAD_CTL_SHIFT	3
#define DMAC_CSR_SRCAD_CTL_SHIFT	5
#define DMAC_CSR_MODE_BIT		7
#define DMAC_CSR_DST_WIDTH_SHIFT	8
#define DMAC_CSR_SRC_WIDTH_SHIFT	11
#define DMAC_CSR_ABT_BIT		15
#define DMAC_CSR_SRC_SIZE_SHIFT		16
#define DMAC_CSR_PROT1_BIT		19
#define DMAC_CSR_PROT2_BIT		20
#define DMAC_CSR_PROT3_BIT		21
#define DMAC_CSR_CHPRI_SHIFT		22
#define DMAC_CSR_TC_MSK_BIT		31
#define DMAC_CSR_DST_SEL_MASK		0x00000002 /* AHB master 0/1 is the destination */
#define DMAC_CSR_SRC_SEL_MASK		0x00000004 /* AHB master 0/1 is the destination */
#define DMAC_CSR_DSTAD_CTL_MASK		0x00000018 /* Destination address stepping control */
#define DMAC_CSR_SRCAD_CTL_MASK		0x00000060 /* Source address stepping control */
#define DMAC_CSR_MODE_MASK		0x00000080 /* 0: Normal mode; 1: HW handshake mode */
#define DMAC_CSR_DST_WIDTH_MASK		0x00000700 /* Destination transfer width */
#define DMAC_CSR_SRC_WIDTH_MASK		0x00003800 /* Source transfer width */
#define DMAC_CSR_ABT			0x00008000 /* Transaction abort */
#define DMAC_CSR_SRC_SIZE_MASK		0x00070000 /* Source burst size */
#define DMAC_CSR_CHPRI_MASK		0x00c00000 /* Channel priority level */
#define DMAC_CSR_TC_MSK_MSK		0x80000000 /* Terminal count status mask for current LLP */

#define DMAC_CSR_SEL_MASTER0		0x00
#define DMAC_CSR_SEL_MASTER1		0x01
#define DMAC_CSR_AD_INC			0x00
#define DMAC_CSR_AD_DEC			0x01
#define DMAC_CSR_AD_FIX			0x02

#define DMAC_CSR_MODE_NORMAL		0x00
#define DMAC_CSR_MODE_HSHK		0x01 /* (required when need multiple bursts or in chain mode?) */

#define DMAC_CSR_WIDTH_8		0x00
#define DMAC_CSR_WIDTH_16		0x01
#define DMAC_CSR_WIDTH_32		0x02

#define DMAC_CSR_SIZE_1			0x00
#define DMAC_CSR_SIZE_4			0x01
#define DMAC_CSR_SIZE_8			0x02
#define DMAC_CSR_SIZE_16		0x03
#define DMAC_CSR_SIZE_32		0x04
#define DMAC_CSR_SIZE_64		0x05
#define DMAC_CSR_SIZE_128		0x06
#define DMAC_CSR_SIZE_256		0x07

#define DMAC_CSR_PROT1			0x00080000 /* Protection information for privilege mode */
#define DMAC_CSR_PROT2			0x00100000 /* Protection information for bufferability */
#define DMAC_CSR_PROT3			0x00200000 /* Protection information for cacheablity */
#define DMAC_CSR_CHPRI_0		0x00
#define DMAC_CSR_CHPRI_1		0x01
#define DMAC_CSR_CHPRI_2		0x02
#define DMAC_CSR_CHPRI_3		0x03


/* DMA channel 0~3 Configuration Registers (CH[n]_BASE + 0x04) */
#define DMAC_CFG_INT_TC_MSK_BIT		0
#define DMAC_CFG_INT_ERR_MSK_BIT	1
#define DMAC_CFG_INT_ABT_MSK_BIT	2
#define DMAC_CFG_INT_SRC_RS_SHIFT	3
#define DMAC_CFG_INT_SRC_HE_BIT		7
#define DMAC_CFG_BUSY_BIT		8
#define DMAC_CFG_INT_DST_RS_SHIFT	9
#define DMAC_CFG_INT_DST_HE_BIT		13
#define DMAC_CFG_INT_LLPCNT_SHIFT	16
#define DMAC_CFG_INT_TC_MSK		0x00000001 /* Channel terminal count interrupt mask */
#define DMAC_CFG_INT_ERR_MSK		0x00000002 /* Channel error interrupt mask */
#define DMAC_CFG_INT_ABT_MSK		0x00000004 /* (FTDMAC only?) Channel abort interrupt mask */
#define DMAC_CFG_INT_SRC_RS_MASK	0x00000078 /* (FTDMAC only?) (HW-HS mode only) Source DMA request number (PMU_REQN_~) */
#define DMAC_CFG_INT_SRC_HE_MASK	0x00000080 /* (FTDMAC only?) (HW-HS mode only) Enable source HW-HS so DMA will start transfer without waiting source request. */
#define DMAC_CFG_BUSY_MASK		0x00000100 /* DMA channel busy flag */
#define DMAC_CFG_INT_DST_RS_MASK	0x00001e00 /* (FTDMAC only?) (HW-HS mode only) Destination DMA request number (PMU_REQN_~) */
#define DMAC_CFG_INT_DST_HE_MASK	0x00002000 /* (FTDMAC only?) (HW-HS mode only) Enable destination HW-HS so DMA will start transfer without waiting destination request. */
#define DMAC_CFG_INT_LLPCNT_MASK	0x000f0000 /* LLP count ? */

#define DMAC_REQN_NONE			PMU_REQN_NONE
#define DMAC_REQN_CFC			PMU_REQN_CFC
#define DMAC_REQN_SSP			PMU_REQN_SSP
#define DMAC_REQN_UART1TX		PMU_REQN_UART1TX
#define DMAC_REQN_UART1RX		PMU_REQN_UART1RX
#define DMAC_REQN_UART2TX		PMU_REQN_UART2TX
#define DMAC_REQN_UART2RX		PMU_REQN_UART2RX
#define DMAC_REQN_SDC			PMU_REQN_SDC
#define DMAC_REQN_I2SAC97		PMU_REQN_I2SAC97
#define DMAC_REQN_USB			PMU_REQN_USB
#define DMAC_REQN_EXT0			PMU_REQN_EXT0
#define DMAC_REQN_EXT1			PMU_REQN_EXT1

#define AHB_REQN_NONE			   0
#define AHB_REQN_RESERVED		   0xFFFFFFFF
/* Request numbers are valid from 0 ~ 7 in old AG101P */
#define XC5_AHB_REQN_NONE
#define XC5_AHB_REQN_CFC                   1
#define XC5_AHB_REQN_SSP                   2
#define XC5_AHB_REQN_UART1TX               3
#define XC5_AHB_REQN_UART1RX               4
#define XC5_AHB_REQN_UART2TX               5
#define XC5_AHB_REQN_UART2RX               6
#define XC5_AHB_REQN_SDC                   7
#define XC5_AHB_REQN_I2SAC97               2
#define XC5_AHB_REQN_USB                   2
#define XC5_AHB_REQN_EXT0                  7
#define XC5_AHB_REQN_EXT1                  7
/* Request numbers are fixed as below in new AG101P*/
#define XC7_AHB_REQN_SSP1TX               1
#define XC7_AHB_REQN_SSP1RX               2
#define XC7_AHB_REQN_UART2TX              3
#define XC7_AHB_REQN_UART2RX              4
#define XC7_AHB_REQN_UART4TX              5
#define XC7_AHB_REQN_UART4RX              6
#define XC7_AHB_REQN_SDC                  7
#define XC7_AHB_REQN_SSP2TX               8
#define XC7_AHB_REQN_I2SAC97              XC7_AHB_REQN_SSP2TX //8
#define XC7_AHB_REQN_SSP2RX               9
#define XC7_AHB_REQN_USB_2_0              10
#define XC7_AHB_REQN_USB_1_1_EP1          XC7_AHB_REQN_USB_2_0 //10
#define XC7_AHB_REQN_USB_1_1_EP2          11
#define XC7_AHB_REQN_USB_1_1_EP3          12
#define XC7_AHB_REQN_USB_1_1_EP4          13
#define XC7_AHB_REQN_EXTREQ0              14
#define XC7_AHB_REQN_EXTREQ1              15

/* DMA channel 0~3 Linked List Descriptor Registers (CH[n]_BASE + 0x10) */
#define DMAC_LLP_ADDR_SHIFT		2
#define DMAC_LLP_MASTER_BIT		0
#define DMAC_LLP_ADDR_MASK		0xfffffffc /* LLP pointer address */
#define DMAC_LLP_MASTER_MASK		0x00000001 /* Master for loading next LLP */

#define DMAC_LLP_MASTER_0		0
#define DMAC_LLP_MASTER_1		1

/* DMA channel 0~3 Transfer Size Registers (CH[n]_BASE + 0x14) */
#define DMAC_TOT_SIZE_MASK		0x00000fff /* Transfer size, in units of SRC_WIDTH */
#define DMAC_TOT_SIZE_SHIFT		0

/*****************************************************************************
 * APBBR - AG101 AHB to APB Bridge
 ****************************************************************************/

/* APBBR slave#n (n = 1~6, 8, 11, 16~23) base/size register */
#define APBBR_SLAVE_SIZE_SHIFT		16
#define APBBR_SLAVE_BASE_SHIFT		20
#define APBBR_SLAVE_SIZE_MASK		0x000f0000 /* Size of address space */
#define APBBR_SLAVE_BASE_MASK		0x3ff00000 /* Base address (10 bits, should be (integer multiples of SIZE)) */
#define APBBR_SIZE_1M			0
#define APBBR_SIZE_2M			1
#define APBBR_SIZE_4M			2
#define APBBR_SIZE_8M			3
#define APBBR_SIZE_16M			4
#define APBBR_SIZE_32M			5
#define APBBR_SIZE_64M			6
#define APBBR_SIZE_128M			7
#define APBBR_SIZE_256M			8


/* APBBR DMA channel transfer cycles register */
#define APBBR_DMA_CYC_MASK		0x00ffffff /* DMA cycles (data size), 1 or 4 bus data transfer cycles per DMA cycle */
#define APBBR_DMA_CYC_SHIFT		0 /* (transfer size = cycles * data_width * burst(1 or 4), */
/*  so, max = 16M*4*4 = 256M) */
/* APBBR DMA channel command register */
#define APBBR_DMA_CHEN_BIT		0
#define APBBR_DMA_FINTST_BIT		1
#define APBBR_DMA_FINTEN_BIT		2
#define APBBR_DMA_BURST_BIT		3
#define APBBR_DMA_ERRINTST_BIT		4
#define APBBR_DMA_ERRINTEN_BIT		5
#define APBBR_DMA_SRCADDRSEL_BIT	6
#define APBBR_DMA_DSTADDRSEL_BIT	7
#define APBBR_DMA_SRCADDRINC_SHIFT	8
#define APBBR_DMA_DSTADDRINC_SHIFT	12
#define APBBR_DMA_DREQSEL_SHIFT		16
#define APBBR_DMA_DATAWIDTH_SHIFT	20
#define APBBR_DMA_SREQSEL_SHIFT		24
#define APBBR_DMA_CHEN_MASK		0x00000001 /* DMA channel enable or disable/stop */
#define APBBR_DMA_FINTST_MASK		0x00000002 /* Finishing interrupt status */
#define APBBR_DMA_FINTEN_MASK		0x00000004 /* Finishing interrupt enable/disable */
#define APBBR_DMA_BURST_MASK		0x00000008 /* Burst mode (0: no burst 1-, 1: burst 4- data cycles per dma cycle) */
#define APBBR_DMA_ERRINTST_MASK		0x00000010 /* Error response interrupt status */
#define APBBR_DMA_ERRINTEN_MASK		0x00000020 /* Error response interrupt enable/disable */
#define APBBR_DMA_SRCADDRSEL_MASK	0x00000040 /* Source address selection (0: APB, 1: AHB) */
#define APBBR_DMA_DSTADDRSEL_MASK	0x00000080 /* Destination address selection (0: APB, 1: AHB) */
#define APBBR_DMA_SRCADDRINC_MASK	0x00000700 /* Source address increment */
#define APBBR_DMA_DSTADDRINC_MASK	0x00007000 /* Destination address increment */
#define APBBR_DMA_DREQSEL_MASK		0x000f0000 /* Request signal multiplexter select of destination for DMA hardware mode */
#define APBBR_DMA_SREQSEL_MASK		0x0f000000 /* Request signal multiplexter select of source for DMA hardware mode */
#define APBBR_DMA_DATAWIDTH_MASK	0x00300000 /* Data width of transfer */

#define APBBR_ADDRSEL_APB		0
#define APBBR_ADDRSEL_AHB		1

#define APBBR_ADDRINC_FIXED		0 /* no increment */
#define APBBR_ADDRINC_I1X		1 /* +1 (burst = 0),  +4 (burst = 1) */
#define APBBR_ADDRINC_I2X		2 /* +2 (burst = 0),  +8 (burst = 1) */
#define APBBR_ADDRINC_I4X		3 /* +4 (burst = 0), +16 (burst = 1) */
#define APBBR_ADDRINC_D1		5 /* -1 */
#define APBBR_ADDRINC_D2		6 /* -2 */
#define APBBR_ADDRINC_D4		7 /* -4 */

#define APBBR_REQN_NONE                     0
#define APBBR_REQN_RESERVED		    0xFFFFFFFF
/* Request numbers in old AG101P */
#define XC5_APBBR_REQN_CFC                  1
#define XC5_APBBR_REQN_SSP                  2
#define XC5_APBBR_REQN_BTUART               4
#define XC5_APBBR_REQN_SDC                  5
#define XC5_APBBR_REQN_I2SAC97              6 /* spec */
#define XC5_APBBR_REQN_STUART               8
#define XC5_APBBR_REQN_I2S                  6 /* boot_code */
#define XC5_APBBR_REQN_SSP2                 8 /* boot_code */
#define XC5_APBBR_REQN_MAX                  XC5_APBBR_REQN_STUART
/* Request numbers in new AG101P */
#define XC7_APBBR_REQN_SSP1TX               1
#define XC7_APBBR_REQN_SSP1RX               2
#define XC7_APBBR_REQN_UART2TX              3
#define XC7_APBBR_REQN_UART2RX              4
#define XC7_APBBR_REQN_UART4TX              5
#define XC7_APBBR_REQN_UART4RX              6
#define XC7_APBBR_REQN_SDC                  7
#define XC7_APBBR_REQN_SSP2TX               8
#define XC7_APBBR_REQN_I2SAC97              8
#define XC7_APBBR_REQN_SSP2RX               9
#define XC7_APBBR_REQN_USB_2_0              10
#define XC7_APBBR_REQN_USB_1_1_EP1          10
#define XC7_APBBR_REQN_USB_1_1_EP2          11
#define XC7_APBBR_REQN_USB_1_1_EP3          12
#define XC7_APBBR_REQN_USB_1_1_EP4          13
#define XC7_APBBR_REQN_EXTREQ0              14
#define XC7_APBBR_REQN_EXTREQ1              15
#define XC7_APBBR_REQN_MAX                  XC7_APBBR_REQN_EXTREQ1

#define APBBR_DATAWIDTH_4		0 /* word */
#define APBBR_DATAWIDTH_2		1 /* half-word */
#define APBBR_DATAWIDTH_1		2 /* byte */


/*****************************************************************************
 * LCDC - AG101 AHB
 ****************************************************************************/

/* LCD Timing0 / Horizontal Timing Register (+0x00) */
#define LCDC_PL_SHIFT			2
#define LCDC_HW_SHIFT			8
#define LCDC_HFP_SHIFT			16
#define LCDC_HBP_SHIFT			24
#define LCDC_PL_MASK			0x000000fc /* PL (pixels per line = 16 / (PL + 1)) */
#define LCDC_HW_MASK			0x0000ff00 /* Horizontal cync (LC_HS) pulse width = HW + 1 (PCLKs) */
#define LCDC_HFP_MASK			0x00ff0000 /* Horizontal front porch = HFP + 1 (PCLKs) */
#define LCDC_HBP_MASK			0xff000000 /* Horizontal back porch = HBP + 1 (PCLKs) */

/* LCD Timing1 / Vertical Timing Register (+0x04) */
#define LCDC_LF_SHIFT			0
#define LCDC_VW_SHIFT			10
#define LCDC_VFP_SHIFT			16
#define LCDC_VBP_SHIFT			24
#define LCDC_LF_MASK			0x000003ff /* lines (LC_DE) per frame = LF + 1 */
#define LCDC_VW_MASK			0x0000fc00 /* Vertical cync (LC_VS) pulse width = HW + 1 (PCLKs) */
#define LCDC_VFP_MASK			0x00ff0000 /* Vertical front porch = HFP + 1 (PCLKs) */
#define LCDC_VBP_MASK			0xff000000 /* Vertical back porch = VBP + 1 (PCLKs) */

/* LCD Timing2 / Clock and Signal Polarity Register (+0x08) */
#define LCDC_CLKDIV_SHIFT		0
#define LCDC_UNDOC08_BIT5		5
#define LCDC_UNDOC08_BIT6		6
#define LCDC_IVS_BIT			11
#define LCDC_IHS_BIT			12
#define LCDC_ICK_BIT			13
#define LCDC_IDE_BIT			14
#define LCDC_ADPEN_BIT			15
#define LCDC_UNDOC08_SHIFT16		16
#define LCDC_UNDOC08_SHIFT26		26
#define LCDC_CLKDIV_MASK		0x0000003f /* LCD panel clock divisor = CLKDIV + 1 */
#define LCDC_UNDOC08_MASK5		0x00000020
#define LCDC_UNDOC08_MASK6		0x000007c0
#define LCDC_IVS_MASK			0x00000800 /* Vertical sync signal polarity, 0: assert high, 1: assert low */
#define LCDC_IHS_MASK			0x00001000 /* Horizontal sync signal polarity, 0: assert high, 1: assert low */
#define LCDC_ICK_MASK			0x00002000 /* Panel data line driven edge, 0: LC_PCLK rising edge, 1: LC_PCLK faling edge */
#define LCDC_IDE_MASK			0x00004000 /* output enable (LC_DE) signal polarity, 0: assert high, 1: assert low */
#define LCDC_ADPEN_MASK			0x00008000 /* Adaptive pixel rate control, 0: disable, 1: enable */
#define LCDC_UNDOC08_MASK16		0x03ff0000
#define LCDC_UNDOC08_MASK26		0xfc000000

/* LCD Frame Base Address (+0x10) */
#define LCDC_FRAMEBASE_SHIFT		6
#define LCDC_FRAME420SIZE_SHIFT		0
#define LCDC_FRAMEBASE_MASK		0xffffffc0 /* LCD frame base address, 64-byte aligned; YCbCr420 Y-plane base address */
#define LCDC_FRAME420SIZE_MASK		0x0000003f /* YCbCr420 frame buffer size = 1.5 // x(FRAME420SIZE << 16) */

/* LCD Interrupt Enable Mask Register (+0x18) */
#define LCDC_FIFO_UR_EN_BIT		1
#define LCDC_FB_ADDR_EN_BIT		2
#define LCDC_VSTATUS_EN_BIT		3
#define LCDC_BUS_ERR_EN_BIT		4
#define LCDC_FIFO_UR_EN_MASK		0x00000002 /* FIFO under-run interrupt enable */
#define LCDC_FB_ADDR_EN_MASK		0x00000004 /* Next frame buffer base address updated interrupt enable */
#define LCDC_VSTATUS_EN_MASK		0x00000008 /* Vertical duration comparison interrupt enable */
#define LCDC_BUS_ERR_EN_MASK		0x00000010 /* AHB bus error interrupt enable */

/* LCD Pnael Pixel Parameters (+0x1c) */
#define LCDC_LCDEN_BIT			0
#define LCDC_BPP_SHIFT			1
#define LCDC_UNDOC1C_BIT4		4
#define LCDC_TFT_BIT			5
#define LCDC_UNDOC1C_BIT6		6
#define LCDC_UNDOC1C_BIT7		7
#define LCDC_BGR_BIT			8
#define LCDC_ENDIAN_SHIFT		9
#define LCDC_LCDON_BIT			11
#define LCDC_VCOMP_SHIFT		12
#define LCDC_PANEL_TYPE_BIT		15
#define LCDC_FIFO_THRESHOLD_BIT		16
#define LCDC_YCBCR420_EN_BIT		17
#define LCDC_YCBCR_EN_BIT		18

#define LCDC_LCDEN_MASK			0x00000001 /* LCD controller enable */
#define LCDC_BPP_MASK			0x0000000e /* Frame buffer pixel format */
#define LCDC_UNDOC1C_MASK4		0x00000010 /* Frame buffer pixel format */
#define LCDC_TFT_MASK			0x00000020 /* TFT panel enable */
#define LCDC_UNDOC1C_MASK6		0x00000040
#define LCDC_UNDOC1C_MASK7		0x00000080
#define LCDC_BGR_MASK			0x00000100 /* BGR (RGB R-B-swap) enable */
#define LCDC_ENDIAN_MASK		0x00000600 /* Frame buffer data endian, 00b: L-byte, L-pix, 01b: B-byte, B-pix, 10b(WinCE): L-byte, B-pix */
#define LCDC_LCDON_MASK			0x00000800 /* LCD on-off control (back-light?) */
#define LCDC_VCOMP_MASK			0x00003000 /* Generate interrupt at, 00b: VS, 01b: VBP, 10b: VData, 11b: VFP */
#define LCDC_PANEL_TYPE_MASK		0x00008000 /* TFT panel color depth per channel, 0: 6-bit, 1: 8-bit */
#define LCDC_FIFO_THRESHOLD_MASK	0x00010000 /* LCD DMA FIFO threshold level */
#define LCDC_YCBCR420_EN_MASK		0x00020000 /* YCbCr420 input mode enable control */
#define LCDC_YCBCR_EN_MASK		0x00040000 /* YCbCr input mode enable control */

#define LCDC_BPP_1			0
#define LCDC_BPP_2			1
#define LCDC_BPP_4			2
#define LCDC_BPP_8			3
#define LCDC_BPP_16			4
#define LCDC_BPP_24			5

/* LCD Interrupt Status Clear Register (+0x20) */
#define LCDC_FIFO_UR_CLR_BIT		1
#define LCDC_FB_ADDR_CLR_BIT		2
#define LCDC_VSTATUS_CLR_BIT		3
#define LCDC_BUS_ERR_CLR_SHIFT		4
#define LCDC_FIFO_UR_CLR_MASK		0x00000002 /* FIFO under-run interrupt status clear */
#define LCDC_FB_ADDR_CLR_MASK		0x00000004 /* Frame buffer base address updated interrupt status clear */
#define LCDC_VSTATUS_CLR_MASK		0x00000008 /* Vertical duration comparison interrupt status clear */
#define LCDC_BUS_ERR_CLR_MASK		0x00000010 /* AHB bus error interrupt status clear */

/* LCD Interrupt Status Register (+0x24) */
#define LCDC_FIFO_UR_IS_BIT		1
#define LCDC_FB_ADDR_IS_BIT		2
#define LCDC_VSTATUS_IS_BIT		3
#define LCDC_BUS_ERR_IS_SHIFT		4
#define LCDC_FIFO_UR_IS_MASK		0x00000002 /* FIFO under-run interrupt status */
#define LCDC_FB_ADDR_IS_MASK		0x00000004 /* Frame buffer base address updated interrupt status */
#define LCDC_VSTATUS_IS_MASK		0x00000008 /* Vertical duration comparison interrupt status */
#define LCDC_BUS_ERR_IS_MASK		0x00000010 /* AHB bus error interrupt status */

/* OSD Scaling and Dimension Control Register (+0x34) */
#define LCDC_OSD_EN_BIT			0
#define LCDC_OSD_VSCALE_SHIFT		1
#define LCDC_OSD_HSCALE_SHIFT		3
#define LCDC_OSD_VDIM_SHIFT		5
#define LCDC_OSD_HDIM_SHIFT		10
#define LCDC_OSD_EN_MASK		0x00000001 /* OSD window on/off control */
#define LCDC_OSD_VSCALE_MASK		0x00000006 /* OSD window vertical scaling factor */
#define LCDC_OSD_HSCALE_MASK		0x00000018 /* OSD window horizontal scaling factor */
#define LCDC_OSD_VDIM_MASK		0x000003e0 /* OSD window vertical dimension (rows) */
#define LCDC_OSD_HDIM_MASK		0x0000fc00 /* OSD window horizontal dimension (columns) */

/* OSD Position Control Register (+0x38) */
#define LCDC_OSD_VPOS_SHIFT		0
#define LCDC_OSD_HPOS_SHIFT		10
#define LCDC_OSD_VPOS_MASK		0x000003ff /* OSD window vertical position (1~1023) */
#define LCDC_OSD_HPOS_MASK		0x000ffc00 /* OSD window horizontal position = 3 + HPOS(1~1023) */

/* OSD Foreground Color Control Register (+0x3c) */
#define LCDC_OSD_FRPAL0_SHIFT		0
#define LCDC_OSD_FRPAL1_SHIFT		8
#define LCDC_OSD_FRPAL2_SHIFT		16
#define LCDC_OSD_FRPAL3_SHIFT		24
#define LCDC_OSD_FRPAL0_MASK		0x00000000 /* OSD foreground color palette entry 0 */
#define LCDC_OSD_FRPAL1_MASK		0x0000ff00 /* OSD foreground color palette entry 1 */
#define LCDC_OSD_FRPAL2_MASK		0x00ff0000 /* OSD foreground color palette entry 2 */
#define LCDC_OSD_FRPAL3_MASK		0xff000000 /* OSD foreground color palette entry 3 */

/* OSD Background Color Control Register (+0x40) */
#define LCDC_OSD_TRANS_SHIFT		6 /* [7:6] is transparency bits (spec says [5:4] --> wrong) */
#define LCDC_OSD_BGPAL1_SHIFT		8
#define LCDC_OSD_BGPAL2_SHIFT		16
#define LCDC_OSD_BGPAL3_SHIFT		24
#define LCDC_OSD_TRANS_MASK		0x000000c0 /* OSD background transparency */
#define LCDC_OSD_BGPAL1_MASK		0x0000ff00 /* OSD background color palette entry 1 */
#define LCDC_OSD_BGPAL2_MASK		0x00ff0000 /* OSD background color palette entry 2 */
#define LCDC_OSD_BGPAL3_MASK		0xff000000 /* OSD background color palette entry 3 */

/* LCD GPIO Control Register (+0x40) */
#define LCDC_LCD_GPI_SHIFT		0
#define LCDC_LCD_GPO_SHIFT		4
#define LCDC_LCD_GPI_MASK		0x0000000f /* Return the status of the 4 GPI ports */
#define LCDC_LCD_GPO_MASK		0x000000f0 /* Directly drive the level of the 4 GPO ports */

/* OSD window attribute write access port (+0xc000) */
#define LCDC_OSD_ATTR_BKCIDX_MASK	0x0003 /* Background color palette index */
#define LCDC_OSD_ATTR_FRCIDX_MASK	0x000c /* Foreground color palette index */
#define LCDC_OSD_ATTR_FONTIDX_MASK	0x0ff0 /* Font index */
#define LCDC_OSD_ATTR_BKCIDX_SHIFT	0
#define LCDC_OSD_ATTR_FRCIDX_SHIFT	2
#define LCDC_OSD_ATTR_FONTIDX_SHIFT	4

#define LCDC_OSD_MAX_FONTIDX		(LCDC_OSD_ATTR_FONTIDX_MASK >> LCDC_OSD_ATTR_FONTIDX_SHIFT)
#define LCDC_OSD_MAX_FRCIDX		3
#define LCDC_OSD_MAX_BKCIDX		3
#define LCDC_OSD_MAX_START		511 /* max start position to present a font in the osd window */

#define LCDC_OSD_FONT_WIDTH		12
#define LCDC_OSD_FONT_HEIGHT		16


/*****************************************************************************
 * MAC - AG101 AHB
 ****************************************************************************/

/* Interrupt Status Register (+0x00) */
#define MAC_ISR_RPKT_FINISH_BIT		0 /* RXDMA has received packets in RX FIFO */
#define MAC_ISR_NORXBUF_BIT		1 /* RX buffer unavailable */
#define MAC_ISR_XPKT_FINISH_BIT		2 /* TXDMA has moved data to TX FIFO */
#define MAC_ISR_NOTXBUF_BIT		3 /* TX buffer unavailable */
#define MAC_ISR_XPKT_OK_BIT		4 /* Packets has been transmitted to ethernet */
#define MAC_ISR_XPKT_LOST_BIT		5 /* Packets transmitted to ethernet lost due to late collision or excessive collision */
#define MAC_ISR_RPKT_SAV_BIT		6 /* Packets received in RX FIFO */
#define MAC_ISR_RPKT_LOST_BIT		7 /* Receviced packets lost due to RX FIFO full */
#define MAC_ISR_AHB_ERR_BIT		8 /* AHB error */
#define MAC_ISR_PHYSTS_CHG_BIT		9 /* PHY link status change */

#define MAC_ISR_RPKT_FINISH_MASK	(0x01U << MAC_ISR_RPKT_FINISH_BIT)
#define MAC_ISR_NORXBUF_MASK		(0x01U << MAC_ISR_NORXBUF_BIT)
#define MAC_ISR_XPKT_FINISH_MASK	(0x01U << MAC_ISR_XPKT_FINISH_BIT)
#define MAC_ISR_NOTXBUF_MASK		(0x01U << MAC_ISR_NOTXBUF_BIT)
#define MAC_ISR_XPKT_OK_MASK		(0x01U << MAC_ISR_XPKT_OK_BIT)
#define MAC_ISR_XPKT_LOST_MASK		(0x01U << MAC_ISR_XPKT_LOST_BIT)
#define MAC_ISR_RPKT_SAV_MASK		(0x01U << MAC_ISR_RPKT_SAV_BIT)
#define MAC_ISR_RPKT_LOST_MASK		(0x01U << MAC_ISR_RPKT_LOST_BIT)
#define MAC_ISR_AHB_ERR_MASK		(0x01U << MAC_ISR_AHB_ERR_BIT)
#define MAC_ISR_PHYSTS_CHG_MASK		(0x01U << MAC_ISR_PHYSTS_CHG_BIT)

/* Interrupt Enable Register (+0x04) */
#define MAC_IME_RPKT_FINISH_BIT		0 /* Interrupt enable of ISR[0] */
#define MAC_IME_NORXBUF_BIT		1 /* Interrupt enable of ISR[1] */
#define MAC_IME_XPKT_FINISH_BIT		2 /* Interrupt enable of ISR[2] */
#define MAC_IME_NOTXBUF_BIT		3 /* Interrupt enable of ISR[3] */
#define MAC_IME_XPKT_OK_BIT		4 /* Interrupt enable of ISR[4] */
#define MAC_IME_XPKT_LOST_BIT		5 /* Interrupt enable of ISR[5] */
#define MAC_IME_RPKT_SAV_BIT		6 /* Interrupt enable of ISR[6] */
#define MAC_IME_RPKT_LOST_BIT		7 /* Interrupt enable of ISR[7] */
#define MAC_IME_AHB_ERR_BIT		8 /* Interrupt enable of ISR[8] */
#define MAC_IME_PHYSTS_CHG_BIT		9 /* Interrupt enable of ISR[9] */

#define MAC_IME_RPKT_FINISH_MASK	(0x01U << MAC_IME_RPKT_FINISH_BIT)
#define MAC_IME_NORXBUF_MASK		(0x01U << MAC_IME_NORXBUF_BIT)
#define MAC_IME_XPKT_FINISH_MASK	(0x01U << MAC_IME_XPKT_FINISH_BIT)
#define MAC_IME_NOTXBUF_MASK		(0x01U << MAC_IME_NOTXBUF_BIT)
#define MAC_IME_XPKT_OK_MASK		(0x01U << MAC_IME_XPKT_OK_BIT)
#define MAC_IME_XPKT_LOST_MASK		(0x01U << MAC_IME_XPKT_LOST_BIT)
#define MAC_IME_RPKT_SAV_MASK		(0x01U << MAC_IME_RPKT_SAV_BIT)
#define MAC_IME_RPKT_LOST_MASK		(0x01U << MAC_IME_RPKT_LOST_BIT)
#define MAC_IME_AHB_ERR_MASK		(0x01U << MAC_IME_AHB_ERR_BIT)
#define MAC_IME_PHYSTS_CHG_MASK		(0x01U << MAC_IME_PHYSTS_CHG_BIT)

/* Interrupt Timer Control Register (+0x28) */
#define MAC_ITC_RXINT_CNT_SHIFT		0  /* Waiting time before interrupt after a packet has been received (cycles) */
#define MAC_ITC_RXINT_THR_SHIFT		4  /* Receive interrupt threshold (number of packets) */
#define MAC_ITC_RXINT_TIME_SEL_BIT	7  /* Period of rx cycle time (0: 5.12/51.2 us, 1: 81.92/819.2 us) */
#define MAC_ITC_TXINT_CNT_SHIFT		8  /* Waiting time after a packet has been transmitted (cycles) */
#define MAC_ITC_TXINT_THR_SHIFT		12 /* Transmit interrupt threshold (number of packets) */
#define MAC_ITC_TXINT_TIME_SEL_BIT	15 /* Period of tx cycle time (0: 5.12/51.2 us, 1: 81.92/819.2 us) */

#define MAC_ITC_RXINT_CNT_MASK		(0x0fU << MAC_ITC_RXINT_CNT_SHIFT)
#define MAC_ITC_RXINT_THR_MASK		(0x07U << MAC_ITC_RXINT_THR_SHIFT)
#define MAC_ITC_RXINT_TIME_SEL_MASK	(0x01U << MAC_ITC_RXINT_TIME_SEL_BIT)
#define MAC_ITC_TXINT_CNT_MASK		(0x0fU << MAC_ITC_TXINT_CNT_SHIFT)
#define MAC_ITC_TXINT_THR_MASK		(0x07U << MAC_ITC_TXINT_THR_SHIFT)
#define MAC_ITC_TXINT_TIME_SEL_MASK	(0x01U << MAC_ITC_TXINT_TIME_SEL_BIT)

/* Automatic Polling Timer Control Register (+0x2c) */
#define MAC_APTC_RXPOLL_CNT_SHIFT	0  /* Number of rx polling cycle (polling rx descriptor) */
#define MAC_APTC_RXPOLL_TIME_SEL_BIT	4  /* Time of rx polling cycle (0: 5.12/51.2 us, 1: 81.92/819.2 us) */
#define MAC_APTC_TXPOLL_CNT_SHIFT	8  /* Number of tx polling cycle (polling tx descriptor) */
#define MAC_APTC_TXPOLL_TIME_SEL_BIT	12 /* Time of tx polling cycle (0: 5.12/51.2 us, 1: 81.92/819.2 us) */
#define MAC_APTC_RXPOLL_CNT_MASK	(0x0fU << MAC_APTC_RXPOLL_CNT_SHIFT)
#define MAC_APTC_RXPOLL_TIME_SEL_MASK	(0x01U << MAC_APTC_RXPOLL_TIME_SEL_BIT)
#define MAC_APTC_TXPOLL_CNT_MASK	(0x0fU << MAC_APTC_TXPOLL_CNT_SHIFT)
#define MAC_APTC_TXPOLL_TIME_SEL_MASK	(0x01U << MAC_APTC_TXPOLL_TIME_SEL_BIT)

/* DMA Burst Length And Arbitration Control Register (+0x30) */
#define MAC_DBLAC_INCR4_EN_BIT		0  /* Enable MAC controller use the AHB INCR4 burst command */
#define MAC_DBLAC_INCR8_EN_BIT		1  /* Enable MAC controller use the AHB INCR8 burst command */
#define MAC_DBLAC_INCR16_EN_BIT		2  /* Enable MAC controller use the AHB INCR16 burst command */
#define MAC_DBLAC_RXFIFO_LTHR_SHIFT	3  /* TXDMA has higher priority when space in RX FIFO <= LTHR */
#define MAC_DBLAC_RXFIFO_HTHR_SHIFT	6  /* RXDMA has higher priority when space in RX FIFO >= HTHR */
#define MAC_DBLAC_RX_THR_EN_BIT		9  /* Enable/disable DMA arbitration according to RX FIFO threshold */
#define MAC_DBLAC_INCR_SEL_SHIFT	14 /* Maximum data byte count of a bust */

#define MAC_DBLAC_INCR4_EN_MASK		(0x01U << MAC_DBLAC_INCR4_EN_BIT)
#define MAC_DBLAC_INCR8_EN_MASK		(0x01U << MAC_DBLAC_INCR8_EN_BIT)
#define MAC_DBLAC_INCR16_EN_MASK	(0x01U << MAC_DBLAC_INCR16_EN_BIT)
#define MAC_DBLAC_RXFIFO_LTHR_MASK	(0x07U << MAC_DBLAC_RXFIFO_LTHR_SHIFT)
#define MAC_DBLAC_RXFIFO_HTHR_MASK	(0x07U << MAC_DBLAC_RXFIFO_HTHR_SHIFT)
#define MAC_DBLAC_RX_THR_EN_MASK	(0x01U << MAC_DBLAC_RX_THR_EN_BIT)
#define MAC_DBLAC_INCR_SEL_MASK		(0x03U << MAC_DBLAC_INCR_SEL_SHIFT)

#define MAC_FIFO_0			0 /* Threshold = 0 */
#define MAC_FIFO_256			1 /* Threshold = 256 bytes  (1/8 FIFO depth) */
#define MAC_FIFO_512			2 /* Threshold = 512 bytes  (2/8 FIFO depth) */
#define MAC_FIFO_768			3 /* Threshold = 768 bytes  (3/8 FIFO depth) */
#define MAC_FIFO_1024			4 /* Threshold = 1024 bytes (4/8 FIFO depth) */
#define MAC_FIFO_1280			5 /* Threshold = 1280 bytes (5/8 FIFO depth) */
#define MAC_FIFO_1536			6 /* Threshold = 1536 bytes (6/8 FIFO depth) */
#define MAC_FIFO_1792			7 /* Threshold = 1792 bytes (7/8 FIFO depth) */

/* MAC Control Register (+0x88) */
#define MAC_MACCR_XDMA_EN_BIT		0  /* Enable TX DMA */
#define MAC_MACCR_RDMA_EN_BIT		1  /* Enable RX DMA */
#define MAC_MACCR_SW_RST_BIT		2  /* Software reset (last 64 AHB bus cycles, then auto clear) */
#define MAC_MACCR_LOOP_EN_BIT		3  /* Enable internal loop back */
#define MAC_MACCR_CRC_DIS_BIT		4  /* Disable RX CRC check */
#define MAC_MACCR_XMT_EN_BIT		5  /* TX (transmitter) enable */
#define MAC_MACCR_ENRX_IN_HALFTX_BIT	6  /* Enable rx when transmitting packets in half duplex mode */
#define MAC_MACCR_RCV_EN_BIT		8  /* Receiver enable */
#define MAC_MACCR_HT_MULTI_EN_BIT	9  /* Enable rx if rx packet pass hash table address filtering and is a multicast packet */
#define MAC_MACCR_RX_RUNT_BIT		10 /* Stores rx packet even if its length < 64 bytes */
#define MAC_MACCR_RX_FLT_BIT		11 /* Stores rx packet even if its length > 1518 bytes */
#define MAC_MACCR_RCV_ALL_BIT		12 /* Bypass rx packet's dest address checking */
#define MAC_MACCR_CRC_APD_BIT		14 /* Appends CRC to transmitted packet */
#define MAC_MACCR_FULLDUP_BIT		15 /* Full duplex */
#define MAC_MACCR_RX_MULTIPKT_BIT	16 /* Receives multicast packets */
#define MAC_MACCR_RX_BROADPKT_BIT	17 /* Receives broadcast packets */

#define MAC_MACCR_XDMA_EN_MASK		(0x01U << MAC_MACCR_XDMA_EN_BIT)
#define MAC_MACCR_RDMA_EN_MASK		(0x01U << MAC_MACCR_RDMA_EN_BIT)
#define MAC_MACCR_SW_RST_MASK		(0x01U << MAC_MACCR_SW_RST_BIT)
#define MAC_MACCR_LOOP_EN_MASK		(0x01U << MAC_MACCR_LOOP_EN_BIT)
#define MAC_MACCR_CRC_DIS_MASK		(0x01U << MAC_MACCR_CRC_DIS_BIT)
#define MAC_MACCR_XMT_EN_MASK		(0x01U << MAC_MACCR_XMT_EN_BIT)
#define MAC_MACCR_ENRX_IN_HALFTX_MASK	(0x01U << MAC_MACCR_ENRX_IN_HALFTX_BIT)
#define MAC_MACCR_RCV_EN_MASK		(0x01U << MAC_MACCR_RCV_EN_BIT)
#define MAC_MACCR_HT_MULTI_EN_MASK	(0x01U << MAC_MACCR_HT_MULTI_EN_BIT)
#define MAC_MACCR_RX_RUNT_MASK		(0x01U << MAC_MACCR_RX_RUNT_BIT)
#define MAC_MACCR_RX_FLT_MASK		(0x01U << MAC_MACCR_RX_FLT_BIT)
#define MAC_MACCR_RCV_ALL_MASK		(0x01U << MAC_MACCR_RCV_ALL_BIT)
#define MAC_MACCR_CRC_APD_MASK		(0x01U << MAC_MACCR_CRC_APD_BIT)
#define MAC_MACCR_FULLDUP_MASK		(0x01U << MAC_MACCR_FULLDUP_BIT)
#define MAC_MACCR_RX_MULTIPKT_MASK	(0x01U << MAC_MACCR_RX_MULTIPKT_BIT)
#define MAC_MACCR_RX_BROADPKT_MASK	(0x01U << MAC_MACCR_RX_BROADPKT_BIT)

/* MAC Status Register (+0x8c) */
#define MAC_MACSR_MULTICAST_BIT		0  /* Incoming packet for multicast address */
#define MAC_MACSR_BROADCAST_BIT		1  /* Incoming packet for broadcast address */
#define MAC_MACSR_COL_BIT		2  /* Incoming packet dropped due to collision */
#define MAC_MACSR_RPKT_SAVE_BIT		3  /* Packets received to RX FIFO successfully */
#define MAC_MACSR_RPKT_LOST_BIT		4  /* Received packet lost due to RX FIFO full */
#define MAC_MACSR_CRC_ERR_BIT		5  /* Incoming packet CRC check fail */
#define MAC_MACSR_FTL_BIT		6  /* Receiver detects a frame which is too long */
#define MAC_MACSR_RUNT_BIT		7  /* Receiver detects a runt packet */
#define MAC_MACSR_XPKT_OK_BIT		8  /* Packets transmitted to ethernet successfully */
#define MAC_MACSR_XPKT_LOST_BIT		9  /* Packets transmitted to ethernet lost due to late or excessive collision */
#define MAC_MACSR_LATE_COL_BIT		10 /* Transmitter detects late collision */
#define MAC_MACSR_COL_EXCEED_BIT	11 /* Collision amount exceeds 16 */

#define MAC_MACSR_MULTICAST_MASK	(0x01U << MAC_MACSR_MULTICAST_BIT)
#define MAC_MACSR_BROADCAST_MASK	(0x01U << MAC_MACSR_BROADCAST_BIT)
#define MAC_MACSR_COL_MASK		(0x01U << MAC_MACSR_COL_BIT)
#define MAC_MACSR_RPKT_SAVE_MASK	(0x01U << MAC_MACSR_RPKT_SAVE_BIT)
#define MAC_MACSR_RPKT_LOST_MASK	(0x01U << MAC_MACSR_RPKT_LOST_BIT)
#define MAC_MACSR_CRC_ERR_MASK		(0x01U << MAC_MACSR_CRC_ERR_BIT)
#define MAC_MACSR_FTL_MASK		(0x01U << MAC_MACSR_FTL_BIT)
#define MAC_MACSR_RUNT_MASK		(0x01U << MAC_MACSR_RUNT_BIT)
#define MAC_MACSR_XPKT_OK_MASK		(0x01U << MAC_MACSR_XPKT_OK_BIT)
#define MAC_MACSR_XPKT_LOST_MASK	(0x01U << MAC_MACSR_XPKT_LOST_BIT)
#define MAC_MACSR_LATE_COL_MASK		(0x01U << MAC_MACSR_LATE_COL_BIT)
#define MAC_MACSR_COL_EXCEED_MASK	(0x01U << MAC_MACSR_COL_EXCEED_BIT)

/* PHY Control Register (+0x90) */
#define MAC_PHYCR_MIIRDATA_SHIFT	0  /* Read data from PHY */
#define MAC_PHYCR_PHYAD_SHIFT		16 /* PHY address */
#define MAC_PHYCR_REGAD_SHIFT		21 /* PHY register address */
#define MAC_PHYCR_MIIRD_BIT		26 /* Initialize a read sequence from PHY */
#define MAC_PHYCR_MIIWR_BIT		27 /* Initialize a write sequence to PHY */

#define MAC_PHYCR_MIIRDATA_MASK		(0xffffU << MAC_PHYCR_MIIRDATA_SHIFT)
#define MAC_PHYCR_PHYAD_MASK		(0x1fU << MAC_PHYCR_PHYAD_SHIFT)
#define MAC_PHYCR_REGAD_MASK		(0x1fU << MAC_PHYCR_REGAD_SHIFT)
#define MAC_PHYCR_MIIRD_MASK		(0x01U << MAC_PHYCR_MIIRD_BIT)
#define MAC_PHYCR_MIIWR_MASK		(0x01U << MAC_PHYCR_MIIWR_BIT)

/* PHY Write Data Register (+0x94) */
#define MAC_PHYWDATA_SHIFT		0 /* Write data to PHY */

#define MAC_PHYWDATA_MASK		(0xffffU << MAC_PHYWDATA_SHIFT)

/* Flow Control Register (+0x98) */
#define MAC_FCR_FC_EN_BIT		0  /* Flow control mode enable */
#define MAC_FCR_TX_PAUSE_BIT		1  /* Send pause frame */
#define MAC_FCR_FCTHR_EN_BIT		2  /* Enables flow control threshold mode (high/low pause threshold) */
#define MAC_FCR_TX_PAUSED_BIT		3  /* Packet transmission is paused due to receiving of pause frame */
#define MAC_FCR_RX_PAUSE_BIT		4  /* Receives pause frame */
#define MAC_FCR_FC_LOW_SHIFT		8  /* RX FIFO free space low threshold (unit: 128 bytes) */
#define MAC_FCR_FC_HIGH_SHIFT		12 /* RX FIFO free space high threshold (unit: 128 bytes) */
#define MAC_FCR_PAUSE_TIME_SHIFT	16 /* Pause time in pause frame (unit: 512 bit times, 5.12/51.2 us) */

#define MAC_FCR_FC_EN_MASK		(0x01U << MAC_FCR_FC_EN_BIT)
#define MAC_FCR_TX_PAUSE_MASK		(0x01U << MAC_FCR_TX_PAUSE_BIT)
#define MAC_FCR_FCTHR_EN_MASK		(0x01U << MAC_FCR_FCTHR_EN_BIT)
#define MAC_FCR_TX_PAUSED_MASK		(0x01U << MAC_FCR_TX_PAUSED_BIT)
#define MAC_FCR_RX_PAUSE_MASK		(0x01U << MAC_FCR_RX_PAUSE_BIT)
#define MAC_FCR_FC_LOW_MASK		(0x0fU << MAC_FCR_FC_LOW_SHIFT)
#define MAC_FCR_FC_HIGH_MASK		(0x0fU << MAC_FCR_FC_HIGH_SHIFT)
#define MAC_FCR_PAUSE_TIME_MASK		(0xffffU << MAC_FCR_PAUSE_TIME_SHIFT)

/* Back Pressure Register (+0x9c) */
#define MAC_BPR_BK_EN_BIT		0 /* Back pressure mode enable */
#define MAC_BPR_BK_MODE_BIT		1 /* Back pressure address mode */
#define MAC_BPR_BKJAM_LEN_SHIFT		4 /* Back pressure jam length */
#define MAC_BPR_BK_LOW_SHIFT		8 /* Jam pattern generation RX FIFO low threshold (unit: 128 bytes) */

#define MAC_BPR_BK_EN_MASK		(0x01U << MAC_BPR_BK_EN_BIT)
#define MAC_BPR_BK_MODE_MASK		(0x01U << MAC_BPR_BK_MODE_BIT)
#define MAC_BPR_BKJAM_LEN_MASK		(0x0fU << MAC_BPR_BKJAM_LEN_SHIFT)
#define MAC_BPR_BK_LOW_MASK		(0x0fU << MAC_BPR_BK_LOW_SHIFT)


#define MAC_BK_MODE_ANY			0 /* Generate jam pattern when any packet is received */
#define MAC_BK_MODE_ADDR_MATCH		1 /* Generate jam pattern when packet address matches */

#define MAC_BKJAM_LEN_4			0  /* 4 bytes */
#define MAC_BKJAM_LEN_8			1  /* 8 bytes */
#define MAC_BKJAM_LEN_16		2  /* 16 bytes */
#define MAC_BKJAM_LEN_32		3  /* 32 bytes */
#define MAC_BKJAM_LEN_64		4  /* 64 bytes */
#define MAC_BKJAM_LEN_128		5  /* 128 bytes */
#define MAC_BKJAM_LEN_256		6  /* 256 bytes */
#define MAC_BKJAM_LEN_512		7  /* 512 bytes */
#define MAC_BKJAM_LEN_1024		8  /* 1024 bytes */
#define MAC_BKJAM_LEN_1518		9  /* 1518 bytes */
#define MAC_BKJAM_LEN_2048		10 /* 2048 bytes */

/* Wake-On-Lan Control Register (+0xa0) */
#define MAC_WOLCR_LINKCHG0_EN_BIT	0  /* Link change to 0 event table */
#define MAC_WOLCR_LINKCHG1_EN_BIT	1  /* Link change to 1 event table */
#define MAC_WOLCR_MAGICPKT_BIT		2  /* Magic packet event table */
#define MAC_WOLCR_WAKEUP1_BIT		3  /* Wakeup frame 1 event table */
#define MAC_WOLCR_WAKEUP2_BIT		4  /* Wakeup frame 2 event table */
#define MAC_WOLCR_WAKEUP3_BIT		5  /* Wakeup frame 3 event table */
#define MAC_WOLCR_WAKEUP4_BIT		6  /* Wakeup frame 4 event table */
#define MAC_WOLCR_POWER_STATE_SHIFT	14 /* Current power state */
#define MAC_WOLCR_WAKEUP_SEL_SHIFT	16 /* Wakeup frame select */
#define MAC_WOLCR_SW_PDNPHY_BIT		18 /* Software power down PHY */
#define MAC_WOLCR_WOL_TYPE_SHIFT	24 /* WOL output signal type */

#define MAC_WOLCR_LINKCHG0_EN_MASK	(0x01U << MAC_WOLCR_LINKCHG0_EN_BIT)
#define MAC_WOLCR_LINKCHG1_EN_MASK	(0x01U << MAC_WOLCR_LINKCHG1_EN_BIT)
#define MAC_WOLCR_MAGICPKT_MASK		(0x01U << MAC_WOLCR_MAGICPKT_BIT)
#define MAC_WOLCR_WAKEUP1_MASK		(0x01U << MAC_WOLCR_WAKEUP1_BIT)
#define MAC_WOLCR_WAKEUP2_MASK		(0x01U << MAC_WOLCR_WAKEUP2_BIT)
#define MAC_WOLCR_WAKEUP3_MASK		(0x01U << MAC_WOLCR_WAKEUP3_BIT)
#define MAC_WOLCR_WAKEUP4_MASK		(0x01U << MAC_WOLCR_WAKEUP4_BIT)
#define MAC_WOLCR_POWER_STATE_MASK	(0x03U << MAC_WOLCR_POWER_STATE_SHIFT)
#define MAC_WOLCR_WAKEUP_SEL_MASK	(0x03U << MAC_WOLCR_WAKEUP_SEL_SHIFT)
#define MAC_WOLCR_SW_PDNPHY_MASK	(0x01U << MAC_WOLCR_SW_PDNPHY_BIT)
#define MAC_WOLCR_WOL_TYPE_MASK		(0x03U << MAC_WOLCR_WOL_TYPE_SHIFT)

#define MAC_POWER_STATE_D0		0
#define MAC_POWER_STATE_D1		1
#define MAC_POWER_STATE_D2		2
#define MAC_POWER_STATE_D3		3

#define MAC_WAKEUP_SEL_FRAME1		0
#define MAC_WAKEUP_SEL_FRAME2		1
#define MAC_WAKEUP_SEL_FRAME3		2
#define MAC_WAKEUP_SEL_FRAME4		3

#define MAC_WOL_ACTIVE_HIGH		0
#define MAC_WOL_ACTIVE_LOW		1
#define MAC_WOL_POSITIVE_PULSE		2
#define MAC_WOL_NEGTIVE_PULSE		3

/* Wake-On-Lan Status Register (+0xa4) */
#define MAC_WOLSR_LINKCHG0_BIT		0 /* Link change to 0 event status */
#define MAC_WOLSR_LINKCHG1_BIT		1 /* Link change to 1 event status */
#define MAC_WOLSR_MAGICPKT_BIT		2 /* Magic packet event status */
#define MAC_WOLSR_WAKEUP1_BIT		3 /* Wakeup frame 1 event status */
#define MAC_WOLSR_WAKRUP2_BIT		4 /* Wakeup frame 2 event status */
#define MAC_WOLSR_WAKRUP3_BIT		5 /* Wakeup frame 3 event status */
#define MAC_WOLSR_WAKRUP4_BIT		6 /* Wakeup frame 4 event status */

#define MAC_WOLSR_LINKCHG0_MASK		(0x01U << MAC_WOLSR_LINKCHG0_BIT)
#define MAC_WOLSR_LINKCHG1_MASK		(0x01U << MAC_WOLSR_LINKCHG1_BIT)
#define MAC_WOLSR_MAGICPKT_MASK		(0x01U << MAC_WOLSR_MAGICPKT_BIT)
#define MAC_WOLSR_WAKRUP1_MASK		(0x01U << MAC_WOLSR_WAKEUP1_BIT)
#define MAC_WOLSR_WAKRUP2_MASK		(0x01U << MAC_WOLSR_WAKRUP2_BIT)
#define MAC_WOLSR_WAKRUP3_MASK		(0x01U << MAC_WOLSR_WAKRUP3_BIT)
#define MAC_WOLSR_WAKRUP4_MASK		(0x01U << MAC_WOLSR_WAKRUP4_BIT)

/* Test Seed Register (+0xc4) */
#define MAC_TS_SHIFT			0 /* Test seed */
#define MAC_TS_MASK			(0x3fffU << MAC_TS_SHIFT)

/* DMA FIFO State Register (+0xc8) */
#define MAC_RXDMA1_SM_SHIFT		0  /* RX DMA1 state machine */
#define MAC_RXDMA2_SM_SHIFT		4  /* RX DMA2 state machine */
#define MAC_TXDMA1_SM_SHIFT		8  /* TX DMA1 state machine */
#define MAC_TXDMA2_SM_SHIFT		12 /* TX DMA2 state machine */
#define MAC_RXFIFO_EMPTY_BIT		26 /* RX FIFO is empty */
#define MAC_TXFIFO_EMPTY_BIT		27 /* TX FIFO is empty */
#define MAC_DARB_RXGNT_BIT		28 /* RXDMA grant */
#define MAC_DARB_TXGNT_BIT		29 /* TXDMA grant */
#define MAC_RXD_REQ_BIT			30 /* RXDMA request */
#define MAC_TXD_REQ_BIT			31 /* TXDMA request */

#define MAC_RXDMA1_SM_MASK		(0x0fU << MAC_RXDMA1_SM_SHIFT)
#define MAC_RXDMA2_SM_MASK		(0x07U << MAC_RXDMA2_SM_SHIFT)
#define MAC_TXDMA1_SM_MASK		(0x0fU << MAC_TXDMA1_SM_SHIFT)
#define MAC_TXDMA2_SM_MASK		(0x07U << MAC_TXDMA2_SM_SHIFT)
#define MAC_RXFIFO_EMPTY_MASK		(0x01U << MAC_RXFIFO_EMPTY_BIT)
#define MAC_TXFIFO_EMPTY_MASK		(0x01U << MAC_TXFIFO_EMPTY_BIT)
#define MAC_DARB_RXGNT_MASK		(0x01U << MAC_DARB_RXGNT_BIT)
#define MAC_DARB_TXGNT_MASK		(0x01U << MAC_DARB_TXGNT_BIT)
#define MAC_RXD_REQ_MASK		(0x01U << MAC_RXD_REQ_BIT)
#define MAC_TXD_REQ_MASK		(0x01U << MAC_TXD_REQ_BIT)

/* Test Mode Register (+0xcc) */
#define MAC_TM_TEST_EXCEL_SHIFT		5  /* Excessive collision test for transmission */
#define MAC_TM_TEST_TIME_SHIFT		10 /* Transmission back off time test */
#define MAC_TM_TEST_MODE_BIT		20 /* Transmission test mode */
#define MAC_TM_SEED_SEL_BIT		21 /* Seed select */
#define MAC_TM_TEST_SEED_SEL_BIT	22 /* Test seed select */
#define MAC_TM_ITIMER_TEST_BIT		24 /* Interrupt timer test mode */
#define MAC_TM_PTIMER_TEST_BIT		25 /* Automatic polling timer test mode */
#define MAC_TM_SINGLE_PKT_BIT		26 /* Single packet mode (TXDMA moves one packet to TX FIFO at one time) */

#define MAC_TM_TEST_EXCEL_MASK		(0x1fU << MAC_TM_TEST_EXCEL_SHIFT)
#define MAC_TM_TEST_TIME_MASK		(0x3ffU << MAC_TM_TEST_TIME_SHIFT)
#define MAC_TM_TEST_MODE_MASK		(0x01U << MAC_TM_TEST_MODE_BIT)
#define MAC_TM_SEED_SEL_MASK		(0x01U << MAC_TM_SEED_SEL_BIT)
#define MAC_TM_TEST_SEED_SEL_MASK	(0x01U << MAC_TM_TEST_SEED_SEL_BIT)
#define MAC_TM_ITIMER_TEST_MASK		(0x01U << MAC_TM_ITIMER_TEST_BIT)
#define MAC_TM_PTIMER_TEST_MASK		(0x01U << MAC_TM_PTIMER_TEST_BIT)
#define MAC_TM_SINGLE_PKT_MASK		(0x01U << MAC_TM_SINGLE_PKT_BIT)

#define MAC_SEED_SEL_EXT		0 /* Select external data as seed */
#define MAC_SEED_SEL_INT		1 /* Select internal counter as seed */
#define MAC_TEST_SEED_TS		0 /* Select MAC_TS as seed */
#define MAC_TEST_SEED_MAC_LADR		1 /* Select MAC_LADR[13:0] as seed */

/* TX_MCOL & TX_SCOL Counter Register (+0xd4) */
#define MAC_TX_SCOL_SHIFT		0  /* Counter for counting packets transmitted ok with single collision */
#define MAC_TX_MCOL_SHIFT		16 /* Counter for counting packets transmitted ok with 2~15 collision */

#define MAC_TX_SCOL_MASK		(0xffffU << MAC_TX_SCOL_SHIFT)
#define MAC_TX_MCOL_MASK		(0xffffU << MAC_TX_MCOL_SHIFT)

/* RPF & AEP Counter Register (+0xd8) */
#define MAC_AEP_SHIFT			0  /* Reseived pause frame counter */
#define MAC_RPF_SHIFT			16 /* Counter for counting packets with alignment error */

#define MAC_AEP_MASK			(0xffffU << MAC_AEP_SHIFT)
#define MAC_RPF_MASK			(0xffffU << MAC_RPF_SHIFT)

/* XM(tx fail, late col, col>=16) & PG(tx fail, col>=16) Counter Register (+0xdc) */
#define MAC_PG_SHIFT			0  /* Counter for counting tx packets failed (col>=16) */
#define MAC_XM_SHIFT			16 /* Counter for counting tx packets failed (late col, col>=16) */

#define MAC_PG_MASK			(0xffffU << MAC_PG_SHIFT)
#define MAC_XM_MASK			(0xffffU << MAC_XM_SHIFT)

/* RUNT_CNT(rx runt) & TLCC(late collision) Counter Register (+0xe0) */
#define MAC_TLCC_SHIFT			0  /* Late collision counter */
#define MAC_RUNT_CNT_SHIFT		16 /* Counter for counting received runt packets */

#define MAC_TLCC_MASK			(0xffffU << MAC_TLCC_SHIFT)
#define MAC_RUNT_CNT_MASK		(0xffffU << MAC_RUNT_CNT_SHIFT)

/* CRCER_CNT(crc err) & FTL_CNT(ftl, >1518) Counter Register (+0xe4) */
#define MAC_FTL_CNT_SHIFT		0  /* Counter for counting received FTL packets (packet length > 1518) */
#define MAC_CRCER_CNT_SHIFT		16 /* Counter for counting CRC error packets */

#define MAC_FTL_CNT_MASK		(0xffffU << MAC_FTL_CNT_SHIFT)
#define MAC_CRCER_CNT_MASK		(0xffffU << MAC_CRCER_CNT_SHIFT)

/* RLC(rx lost) & RCC(rx collision) Counter Register (+0xe8) */
#define MAC_RCC_SHIFT			0  /* Receiver collision counter */
#define MAC_RLC_SHIFT			16 /* Counter for counting lost packets deu to RX FIFO full */

#define MAC_RCC_MASK			(0xffffU << MAC_RCC_SHIFT)
#define MAC_RLC_MASK			(0xffffU << MAC_RLC_SHIFT)

/* Definitions for Transmit Descriptor #0 */
#define MAC_TXDES0_LATECOL_BIT		0  /* Frame owned by MAC controller indicator */
#define MAC_TXDES0_EXSCOL_BIT		1  /* Frame transmission abort indicator due to late collision */
#define MAC_TXDES0_TXDMA_OWN_BIT	31 /* Frame transmission abort indicator due to 16 collisions */

#define MAC_TXDES0_LATECOL_MASK		(0x01U << MAC_TXDES0_LATECOL_BIT)
#define MAC_TXDES0_EXSCOL_MASK		(0x01U << MAC_TXDES0_EXSCOL_BIT)
#define MAC_TXDES0_TXDMA_OWN_MASK	(0x01U << MAC_TXDES0_TXDMA_OWN_BIT)

/* Definitions for Transmit Descriptor #1 */
#define MAC_TXDES1_TXBUF_SIZE_SHIFT	0  /* Transmit buffer size in bytes */
#define MAC_TXDES1_LTS_BIT		27 /* Last transmit segment descriptor of a packet */
#define MAC_TXDES1_FTS_BIT		28 /* First transmit segment descriptor of a packet */
#define MAC_TXDES1_TX2FIC_BIT		29 /* Interrupt on completion of tranmission to TX FIFO */
#define MAC_TXDES1_TXIC_BIT		30 /* Interrupt on completion of tranmission of current frame */
#define MAC_TXDES1_EDOTR_BIT		31 /* Last descriptor of the transmit ring */

#define MAC_TXDES1_TXBUF_SIZE_MASK	(0x3ffU << MAC_TXDES1_TXBUF_SIZE_SHIFT)
#define MAC_TXDES1_LTS_MASK		(0x01U << MAC_TXDES1_LTS_BIT)
#define MAC_TXDES1_FTS_MASK		(0x01U << MAC_TXDES1_FTS_BIT)
#define MAC_TXDES1_TX2FIC_MASK		(0x01U << MAC_TXDES1_TX2FIC_BIT)
#define MAC_TXDES1_TXIC_MASK		(0x01U << MAC_TXDES1_TXIC_BIT)
#define MAC_TXDES1_EDOTR_MASK		(0x01U << MAC_TXDES1_EDOTR_BIT)

/* Definitions for Transmit Descriptor #2 */
#define MAC_TXDES2_TXBUF_BADR_SHIFT	0 /* Base address of the tx buffer */

#define MAC_TXDES2_TXBUF_BADR_MASK	(0xffffffffU << MAC_TXDES2_TXBUF_BADR_SHIFT)

/* Definitions for Receive Descriptor #0 */
#define MAC_RXDES0_RFL_SHIFT		0  /* Receive frame length (valid only when FRS == 1) */
#define MAC_RXDES0_MULTICAST_BIT	16 /* Multicast frame indicator (valid only when FRS == 1) */
#define MAC_RXDES0_BROADCAST_BIT	17 /* Broadcast frame indicator (valid only when FRS == 1) */
#define MAC_RXDES0_RX_ERR_BIT		18 /* Receive error indicator (valid only when FRS == 1) */
#define MAC_RXDES0_CRC_ERR_BIT		19 /* Crc error indicator (valid only when FRS == 1) */
#define MAC_RXDES0_FTL_BIT		20 /* Frame to long indicator (valid only when FRS == 1) */
#define MAC_RXDES0_RUNT_BIT		21 /* Runt indicator (valid only when FRS == 1) */
#define MAC_RXDES0_RX_ODD_NB_BIT	22 /* Receive odd nibbles indicator (valid only when FRS == 1) */
#define MAC_RXDES0_LRS_BIT		28 /* Last receive segment indicator */
#define MAC_RXDES0_FRS_BIT		29 /* First receive segment indicator */
#define MAC_RXDES0_RXDMA_OWN_BIT	31 /* Frame owned by MAC controller indicator */

#define MAC_RXDES0_RFL_MASK		(0x3ffU << MAC_RXDES0_RFL_SHIFT)
#define MAC_RXDES0_MULTICAST_MASK	(0x01U << MAC_RXDES0_MULTICAST_BIT)
#define MAC_RXDES0_BROADCAST_MASK	(0x01U << MAC_RXDES0_BROADCAST_BIT)
#define MAC_RXDES0_RX_ERR_MASK		(0x01U << MAC_RXDES0_RX_ERR_BIT)
#define MAC_RXDES0_CRC_ERR_MASK		(0x01U << MAC_RXDES0_CRC_ERR_BIT)
#define MAC_RXDES0_FTL_MASK		(0x01U << MAC_RXDES0_FTL_BIT)
#define MAC_RXDES0_RUNT_MASK		(0x01U << MAC_RXDES0_RUNT_BIT)
#define MAC_RXDES0_RX_ODD_NB_MASK	(0x01U << MAC_RXDES0_RX_ODD_NB_BIT)
#define MAC_RXDES0_LRS_MASK		(0x01U << MAC_RXDES0_LRS_BIT)
#define MAC_RXDES0_FRS_MASK		(0x01U << MAC_RXDES0_FRS_BIT)
#define MAC_RXDES0_RXDMA_OWN_MASK	(0x01U << MAC_RXDES0_RXDMA_OWN_BIT)

/* Definitions for Receive Descriptor #1 */
#define MAC_RXDES1_RXBUF_SIZE_SHIFT	0  /* Receive buffer size in bytes */
#define MAC_RXDES1_EDORR_BIT		31 /* Last descriptor of the receive ring */

#define MAC_RXDES1_RXBUF_SIZE_MASK	(0x3ffU << MAC_RXDES1_RXBUF_SIZE_SHIFT)
#define MAC_RXDES1_EDORR_MASK		(0x01U << MAC_RXDES1_EDORR_BIT)

/* Definitions for Receive Descriptor #2 */
#define MAC_RXDES2_TXBUF_BADR_SHIFT	0 /* Base address of the rx buffer */
#define MAC_RXDES2_TXBUF_BADR_MASK	(0xffffffffU << MAC_RXDES2_TXBUF_BADR_SHIFT)


/*****************************************************************************
 * PMU - AG101 Core APB
 ****************************************************************************/

/* Platform define */
#define AG101P_EMERALD			0x41471000
#define PRODUCT_ID_MASK			0xFFFFF000

/* ID number 0 register (+0x00) */
#define PMU_ECOID_SHIFT			0
#define PMU_VERID_SHIFT			4
#define PMU_SYSID_SHIFT			8
#define PMU_DEVID_SHIFT			16
#define PMU_ECOID_MASK			0x0000000f /* ECO ID */
#define PMU_VERID_MASK			0x000000f0 /* Version ID */
#define PMU_SYSID_MASK			0x0000ff00 /* System ID */
#define PMU_DEVID_MASK			0xffff0000 /* Device ID */

/* Frequency scaling status register (+0x04) */
#define PMU_DIVAHBCLK_SHIFT		4
#define PMU_FSF_SHIFT			8
#define PMU_DIVAHBCLK_MASK		0x000000f0 /* CPU:BUS (BUS/CPU) clock ratio */
#define PMU_FSF_MASK			0x00000f00 /* Frequency scaling factor */

/* OSC control register (+0x08) */
#define PMU_OSCLOFF_BIT			0
#define PMU_OSCLSTABLE_BIT		1
#define PMU_RTCLSEL_BIT			2
#define PMU_OSCLTRI_BIT			3
#define PMU_OSCHOFF_BIT			8
#define PMU_OSCHSTABLE_BIT		9
#define PMU_OSCHTRI_BIT			11
#define PMU_OSCLOFF_MASK		0x00000001 /* Enable/disable OSCL oscillator */
#define PMU_OSCLSTABLE_MASK		0x00000002 /* Indicate the OSCL oscillator is ready. Stable time is 1.5s. */
#define PMU_RTCLSEL_MASK		0x00000004 /* RTC/WDT/TIMER clock selection */
#define PMU_OSCLTRI_MASK		0x00000008 /* Disable OSCL function */
#define PMU_OSCHOFF_MASK		0x00000100 /* Disable OSCH oscillator */
#define PMU_OSCHSTABLE_MASK		0x00000200 /* Indicate the OSCH oscillator is ready. Stable time is 26.67ms. */
#define PMU_OSCHTRI_MASK		0x00000800 /* Disable OSCH function */

/* Power mode register (+0x0c) */
#define PMU_SLEEP_BIT			0
#define PMU_FS_BIT			1
#define PMU_FCS_BIT			2
#define PMU_EDIVAHBCLK_SHIFT		4
#define PMU_EFSF_SHIFT			8
#define PMU_SLEEP_MASK			0x00000001 /* Enable sleep mode */
#define PMU_FS_MASK			0x00000002 /* Enter frequency scaling mode */
#define PMU_FCS_MASK			0x00000004 /* Enable frequency change sequence */
#define PMU_EDIVAHBCLK_MASK		0x000000f0 /* Enable clock ratio division */
#define PMU_EFSF_MASK			0x00000f00 /* Enable frequency scaling factor */

#define PMU_DIVAHBCLK_1			0 /* 1 */
#define PMU_DIVAHBCLK_D2		1 /* 1/2 */
#define PMU_DIVAHBCLK_D3		2 /* 1/3 */
#define PMU_DIVAHBCLK_D4		3 /* 1/4 */
#define PMU_DIVAHBCLK_D5		4 /* 1/5 */
#define PMU_DIVAHBCLK_D6		5 /* 1/6 */
#define PMU_DIVAHBCLK_2D3		6 /* 2/3 */
#define PMU_DIVAHBCLK_2D5		7 /* 2/5 */
#define PMU_DIVAHBCLK_1D8		8 /* 1/8 */
#define PMU_DIVAHBCLK_1D10		9 /* 1/10 */
#define PMU_DIVAHBCLK_1D12		10 /* 1/12 */
#define PMU_DIVAHBCLK_1D14		11 /* 1/14 */
#define PMU_DIVAHBCLK_1D15		12 /* 1/15 */
#define PMU_DIVAHBCLK_1D18		13 /* 1/18 */
#define PMU_DIVAHBCLK_1D20		14 /* 1/20 */

#define PMU_FSF_1			0 /* 1 */
#define PMU_FSF_D2			1 /* 1/2 */
#define PMU_FSF_D3			2 /* 1/3 */
#define PMU_FSF_D4			3 /* 1/4 */
#define PMU_FSF_D5			4 /* 1/5 */
#define PMU_FSF_D6			5 /* 1/6 */

/* Power manager control register (+0x10) */
#define PMU_WEGPIO_SHIFT		0
#define PMU_WERTC_BIT			16
#define PMU_WDTCLR_BIT			17
#define PMU_WAITPD_BIT			18
#define PMU_PWRLOWMSK_BIT		19
#define PMU_WEGPIO_MASK			0x0000ffff /* Wake up from sleep mode as GPIO#n is enabled */
#define PMU_WERTC_MASK			0x00010000 /* Wake up from sleep mode as RTC alarm is enabled */
#define PMU_WDTCLR_MASK			0x00020000 /* Enable reset type be cleared by watchdog */
#define PMU_WAITPD_MASK			0x00040000 /* CPU wait until PMU_MISC:PDCNT counts down to 0 before enter power down mode */
#define PMU_PWRLOWMSK_MASK		0x00080000 /* Mask X_powerlow_b pin. */

/* Power manager edge detect register (+0x14) */
#define PMU_GPIOFE_SHIFT		0
#define PMU_GPIORE_SHIFT		16
#define PMU_GPIOFE_MASK			0x0000ffff /* Sleep mode falling edge wake up enable */
#define PMU_GPIORE_MASK			0xffff0000 /* Sleep mode rising edge wake up enable */

/* Power manager edge detect status register (+0x18) */
#define PMU_GPIOED_SHIFT		0
#define PMU_GPIOED_MASK			0x0000ffff /* Sleep mode edge detect status */

/* Power manager status register (+0x20) */
#define PMU_PMSR_CKEHLOW_BIT		0
#define PMU_PMSR_PH_BIT			1
#define PMU_PMSR_RDH_BIT		2
#define PMU_PMSR_HWR_BIT		8
#define PMU_PMSR_WDT_BIT		9
#define PMU_PMSR_SMR_BIT		10
#define PMU_PMSR_INTFS_BIT		16
#define PMU_PMSR_INTFCS_BIT		17
#define PMU_PMSR_INTPWRLOW_BIT		18
#define PMU_PMSR_PWRLOW_BIT		19
#define PMU_PMSR_CKEHLOW_MASK		0x00000001 /* SDRAM clock enable (CKE) forced low */
#define PMU_PMSR_PH_MASK		0x00000002 /* Peripheral control hold */
#define PMU_PMSR_RDH_MASK		0x00000004 /* GPIO read-to-disable-hold */
#define PMU_PMSR_HWR_MASK		0x00000100 /* Reboot by hardware reset */
#define PMU_PMSR_WDT_MASK		0x00000200 /* Reboot by watchdog reset */
#define PMU_PMSR_SMR_MASK		0x00000400 /* Wake up from sleep mode */
#define PMU_PMSR_INTFS_MASK		0x00010000 /* Interrupt status for completing frequency scaling mode */
#define PMU_PMSR_INTFCS_MASK		0x00020000 /* Interrupt status for completing frequency change sequence */
#define PMU_PMSR_INTPWRLOW_MASK		0x00040000 /* Interrupt status for power low detection */
#define PMU_PMSR_PWRLOW_MASK		0x00080000 /* Power low pin (X_powerlow_b) status */

/* Power manager GPIO sleep state register (+0x24) */
#define PMU_PGSR_SSVAL_SHIFT		0
#define PMU_PGSR_SS_SHIFT		16
#define PMU_PGSR_SSVAL_MASK		0x0000ffff /* Programmed output value when system enters sleep mode, when SS is enabled */
#define PMU_PGSR_SS_MASK		0xffff0000 /* Output SSVAL value when system enters sleep mode */

/* Multi-function port setting register (+0x28) */
#define PMU_AHBDBG_BIT			0
#define PMU_AHBDIS_BIT			1
#define PMU_AC97PINSEL_BIT		3
#define PMU_AC97CLKSEL_BIT		4
#define PMU_I2SCLKSEL_BIT		5
#define PMU_SSPCLKSEL_BIT		6
#define PMU_UARTCLKSEL_BIT		8
#define PMU_IRDACLKSEL_BIT		9
#define PMU_PWM0PINSEL_BIT		10
#define PMU_PWM1PINSEL_BIT		11
#define PMU_AC97CLKOUTSEL_BIT		13
#define PMU_DMA1PINSEL_BIT		15
#define PMU_DMA0PINSEL_BIT		16
#define PMU_DEBUGSEL_BIT		17
#define PMU_AHBDBG_MASK			0x00000001 /* Enable AHB bus debug function */
#define PMU_AHBDIS_MASK			0x00000002 /* Disable AHB bus function */
#define PMU_AC97PINSEL_MASK		0x00000008 /* Selects (0: X_I2Ssclkout/I2SCLK) or (1: X_ac97_resetn/50MHz in AG101) */
#define PMU_AC97CLKSEL_MASK		0x00000010 /* Selects AC97 clock source (0: AC97CLK, 1: GPIO22) */
#define PMU_I2SCLKSEL_MASK		0x00000020 /* Selects I2S clock source (0: I2SCLK, 1: GPIO22) */
#define PMU_SSPCLKSEL_MASK		0x00000040 /* Selects SSP clock source (0: SSPCLK, 1: GPIO25) */
#define PMU_UARTCLKSEL_MASK		0x00000100 /* Selects UCLK clock source (0: UCLK, 1: GPIO23) */
#define PMU_IRDACLKSEL_MASK		0x00000200 /* Selects IrDA clock source (0: IRDACLK, 1: GPIO24) */
#define PMU_PWM0PINSEL_MASK		0x00000400 /* Selects GPIO30 function (0: GPIO, 1: PWM0 out) */
#define PMU_PWM1PINSEL_MASK		0x00000800 /* Selects GPIO31 function (0: GPIO, 1: PWM1 out) */
#define PMU_AC97CLKOUTSEL_MASK		0x00002000 /* Selects GPIO26 function (0: GPIO, 1: AC97CLK out) */
#define PMU_DMA1PINSEL_MASK		0x00008000 /* Selects GPIO[15:13] function (0: GPIO, 1: [15] dma_tc1 [14] dma_ack1 [13] dma_reg1) */
#define PMU_DMA0PINSEL_MASK		0x00010000 /* Selects GPIO[12:10] function (0: GPIO, 1: [12] dma_tc0 [11] dma_ack0 [10] dma_reg0) */
#define PMU_DEBUGSEL_MASK		0x00020000 /* Selects debug group signal (0: group0, 1: group1) */

/* Misc register (+0x2c) */
#define PMU_TURNDIS_BIT			0
#define PMU_PDCNT_BIT			8
#define PMU_TURNDIS_MASK		0x00000001 /* EBI autorun function disable */
#define PMU_PDCNT_MASK			0x00007f00 /* Counter to wait before CPU enters power down state */

/* PLL/DLL control register 0 (+0x30) */
#define PMU_PDLL1DIS_BIT		0
#define PMU_PLL1STABLE_BIT		1
#define PMU_PLL1STSEL_BIT		2
#define PMU_PLL1NS_SHIFT		3
#define PMU_PLL1FRANG_SHIFT		12
#define PMU_DLLDIS_BIT			16
#define PMU_DLLSTABLE_BIT		17
#define PMU_DLLSTSEL_BIT		18
#define PMU_DLLFRANG_SHIFT		19
#define PMU_HCLKOUTDIS_SHIFT		21
#define PMU_PDLL1DIS_MASK		0x00000001 /* Disable PLL1 */
#define PMU_PLL1STABLE_MASK		0x00000002 /* PLL1 stable indicator */
#define PMU_PLL1STSEL_MASK		0x00000004 /* PLL1 stable wait method (0: wait 1.22ms, 1: wait PLL1 stable signal) */
#define PMU_PLL1NS_MASK			0x00000ff8 /* Control PLL1 output (PLL1_out = PLL1NS * OSCCLK) */
#define PMU_PLL1FRANG_MASK		0x00003000 /* PLL1 output frequency range */
#define PMU_DLLDIS_MASK			0x00010000 /* Disable DLL */
#define PMU_DLLSTABLE_MASK		0x00020000 /* DLL stable indicator */
#define PMU_DLLSTSEL_MASK		0x00040000 /* DLL stable wait method (0: wait 1.22ms, 1: wait DLL stable signal) */
#define PMU_DLLFRANG_MASK		0x00180000 /* DLL output frequency range */
#define PMU_HCLKOUTDIS_MASK		0x01700000 /* Disable SDRAM clock output (X_sdclk[3:0]) */

#define PMU_PLL1FRANG_100_MHZ		0
#define PMU_PLL1FRANG_200_MHZ		1
#define PMU_PLL1FRANG_300_500_MHZ	2
#define PMU_PLL1FRANG_500_700_MHZ	3
#define PMU_DLLFRANG_33_50_MHZ		0
#define PMU_DLLFRANG_50_100_MHZ		1
#define PMU_DLLFRANG_100_200_MHZ	2
/* PLL/DLL control register 1 (+0x34) */
#define PMU_PDLL3DIS_BIT		0
#define PMU_PLL3STABLE_BIT		1
#define PMU_PLL3STSEL_BIT		2
#define PMU_PDLL2DIS_BIT		8
#define PMU_PLL2STABLE_BIT		9
#define PMU_PLL2STSEL_BIT		10
#define PMU_I2SCLKDIV_SHIFT		16
#define PMU_PDLLCR0_PWMCLKDIV_SHIFT	20
#define PMU_PDLL3DIS_MASK		0x00000001 /* Disable PLL3 */
#define PMU_PLL3STABLE_MASK		0x00000002 /* PLL3 stable indicator */
#define PMU_PLL3STSEL_MASK		0x00000004 /* PLL3 stable wait method (0: wait 1.22ms, 1: wait PLL3 stable signal) */
#define PMU_PDLL2DIS_MASK		0x00000100 /* Disable PLL2 */
#define PMU_PLL2STABLE_MASK		0x00000200 /* PLL2 stable indicator */
#define PMU_PLL2STSEL_MASK		0x00000400 /* PLL2 stable wait method (0: wait 1.22ms, 1: wait PLL2 stable signal) */
#define PMU_I2SCLKDIV_MASK		0x000f0000 /* I2S main clock = PLL3CLK / I2SCLKDIV ? */
/*#define PMU_I2SCLKDIV_?		0 */
/*#define PMU_I2SCLKDIV_?		1 */
/*#define PMU_I2SCLKDIV_?		2 */
#define PMU_PDLLCR0_PWMCLKDIV_MASK	0x00f00000 /* PWM clock = OSCCLK / (PWMCLKDIV + 1) */

/* AHB module clock off control register (+0x38) */
#define PMU_APBBRGOFF_BIT		1
#define PMU_SMCOFF_BIT			3
#define PMU_EBIOFF_BIT			4
#define PMU_SDRAMOFF_BIT		5
#define PMU_DMAOFF_BIT			7
#define PMU_MACOFF_BIT			12
#define PMU_USBOFF_BIT			14
#define PMU_APBBRGOFF_MASK		0x00000002 /* Turn off clock of the AHB to APB bridge module */
#define PMU_SMCOFF_MASK			0x00000008 /* Turn off clock of the SRAM controller */
#define PMU_EBIOFF_MASK			0x00000010 /* Turn off clock of the external bus interface module */
#define PMU_SDRAMOFF_MASK		0x00000020 /* Turn off clock of the SDRAM controller */
#define PMU_DMAOFF_MASK			0x00000040 /* Turn off clock of the DMA controller */
#define PMU_MACOFF_MASK			0x00001000 /* Turn off clock of the ethernet MAC controller */
#define PMU_USBOFF_MASK			0x00004000 /* Turn off clock of the USB 2.0 device controller */

/* APB module clock off control register (+0x3c) */
#define PMU_CFCOFF_BIT			1
#define PMU_SSPOFF_BIT			2
#define PMU_BTUARTOFF_BIT		4
#define PMU_SDOFF_BIT			5
#define PMU_I2SAC97OFF_BIT		6
#define PMU_STUARTOFF_BIT		8
#define PMU_TIMEROFF_BIT		17
#define PMU_WDTOFF_BIT			18
#define PMU_RTCOFF_BIT			19
#define PMU_GPIOOFF_BIT			20
#define PMU_INTCOFF_BIT			21
#define PMU_I2COFF_BIT			22
#define PMU_PWMOFF_BIT			23
#define PMU_CFCOFF_MASK			0x00000002 /* Turn off clock of the CF controller */
#define PMU_SSPOFF_MASK			0x00000004 /* Turn off clock of the SSP controller */
#define PMU_BTUARTOFF_MASK		0x00000010 /* Turn off clock of the BTUART controller */
#define PMU_SDOFF_MASK			0x00000020 /* Turn off clock of the SD controller */
#define PMU_I2SAC97OFF_MASK		0x00000040 /* Turn off clock of the I2S/AC97 controller */
#define PMU_STUARTOFF_MASK		0x00000100 /* Turn off clock of the STUART controller */
#define PMU_TIMEROFF_MASK		0x00020000 /* Turn off clock of the TIMER controller */
#define PMU_WDTOFF_MASK			0x00040000 /* Turn off clock of the WDT controller */
#define PMU_RTCOFF_MASK			0x00080000 /* Turn off clock of the RTC controller */
#define PMU_GPIOOFF_MASK		0x00100000 /* Turn off clock of the GPIO controller */
#define PMU_INTCOFF_MASK		0x00200000 /* Turn off clock of the interrupt controller */
#define PMU_I2COFF_MASK			0x00400000 /* Turn off clock of the I2C controller */
#define PMU_PWMOFF_MASK			0x00800000 /* Turn off clock of the PWM controller */

/* Driving capability and slew rate control register 0 (+0x40) */
#define PMU_SRAM_DCSR_SHIFT		0 /* X_memaddr[24:15], X_smc_be_b[3:0], X_smc_we_b, X_smc_cs_b[3:0], and X_smc_ow_b pins. */
#define PMU_SDCLK_DC_SHIFT		4
#define PMU_DQM_DC_SHIFT		8
#define PMU_CKE_DC_SHIFT		12
#define PMU_SDRAMCTL_DC_SHIFT		16
#define PMU_SDRAMCS_DC_SHIFT		20
#define PMU_EBIDATA_DC_SHIFT		24
#define PMU_EBICTRL_DC_SHIFT		28

#define PMU_SRAM_DCSR_MASK		0x0000000f /* Control the slew rate and output driving capability of */
#define PMU_SDCLK_DC_MASK		0x000000f0 /* Fine tune the output driving capability the SDRAMC's clock output pins */
#define PMU_DQM_DC_MASK			0x00000f00 /* Fine tune the output driving capability the SDRAMC's DQM pins */
#define PMU_CKE_DC_MASK			0x0000f000 /* Fine tune the output driving capability the SDRAMC's CKE pins */
#define PMU_SDRAMCTL_DC_MASK		0x000f0000 /* Fine tune the output driving capability the SDRAMC's control pins */
#define PMU_SDRAMCS_DC_MASK		0x00f00000 /* Fine tune the output driving capability the SDRAMC's chip select pins */
#define PMU_EBIDATA_DC_MASK		0x0f000000 /* Fine tune the output driving capability the X_memdata[31:0] */
#define PMU_EBICTRL_DC_MASK		0xf0000000 /* Fine tune the output driving capability the X_memaddr[14:0] */

#define PMU_SLEWRATE_HIGH		0x0 /* Slew rate of */
#define PMU_SLEWRATE_LOW		0x8 /* SRAM_DCSR[3], and all DCSRCR1, DCSRCR2 registers */
#define PMU_DRIVECAP_2MA		0x0 /* Driving capability of */
#define PMU_DRIVECAP_4MA		0x1 /* SRAM_DCSR[2:0], and all DCSRCR1, DCSRCR2 registers */
#define PMU_DRIVECAP_6MA		0x2
#define PMU_DRIVECAP_8MA		0x3
#define PMU_DRIVECAP_10MA		0x4
#define PMU_DRIVECAP_12MA		0x5
#define PMU_DRIVECAP_14MA		0x6
#define PMU_DRIVECAP_16MA		0x7

/* Driving capability and slew rate control register 1 (+0x44) */
#define PMU_INTC_DCSR_SHIFT		0 /* X_int_irqn, X_int_fiqn pins */
#define PMU_GPIO_DCSR_SHIFT		4 /* GPIO output pins */
#define PMU_CFC_DCSR_SHIFT		8 /* CFC controller output pins */
#define PMU_MAC_DCSR_SHIFT		12 /* Ethernet MAC controller output pins */
#define PMU_I2C_DCSR_SHIFT		16 /* CPU ICE out pins and I2C output pins */
#define PMU_USBDEV_DCSR_SHIFT		24 /* TM&T output pins */
#define PMU_TRIAHB_DCSR_SHIFT		28 /* AMBA bus output pins */
#define PMU_INTC_DCSR_MASK		0x0000000f /* Control the slew rate and output driving capability of the */
#define PMU_GPIO_DCSR_MASK		0x000000f0 /* Control the slew rate and output driving capability of the */
#define PMU_CFC_DCSR_MASK		0x00000f00 /* Control the slew rate and output driving capability of the */
#define PMU_MAC_DCSR_MASK		0x0000f000 /* Control the slew rate and output driving capability of the */
#define PMU_I2C_DCSR_MASK		0x000f0000 /* Control the slew rate and output driving capability of the */
#define PMU_USBDEV_DCSR_MASK		0x0f000000 /* Control the slew rate and output driving capability of the */
#define PMU_TRIAHB_DCSR_MASK		0xf0000000 /* Control the slew rate and output driving capability of the */

/* Driving capability and slew rate control register 2 (+0x48) */
#define PMU_STUART_DCSR_SHIFT		0 /* STUART controller output pins */
#define PMU_BTUART_DCSR_SHIFT		8 /* BTUART controller output pins */
#define PMU_FFUART_DCSR_SHIFT		12 /* FFUART controller output pins */
#define PMU_PMU_DCSR_SHIFT		16 /* X_reset_b and X_pwren pins */
#define PMU_I2SAC97_DCSR_SHIFT		20 /* I2S/AC97 controller output pins */
#define PMU_SSP_DCSR_SHIFT		24 /* SSD controller output pins */
#define PMU_SD_DCSR_SHIFT		28 /* SD controller output pins */
#define PMU_STUART_DCSR_MASK		0x0000000f /* Control the slew rate and output driving capability of the */
#define PMU_BTUART_DCSR_MASK		0x00000f00 /* Control the slew rate and output driving capability of the */
#define PMU_FFUART_DCSR_MASK		0x0000f000 /* Control the slew rate and output driving capability of the */
#define PMU_PMU_DCSR_MASK		0x000f0000 /* Control the slew rate and output driving capability of the */
#define PMU_I2SAC97_DCSR_MASK		0x00f00000 /* Control the slew rate and output driving capability of the */
#define PMU_SSP_DCSR_MASK		0x0f000000 /* Control the slew rate and output driving capability of the */
#define PMU_SD_DCSR_MASK		0xf0000000 /* Control the slew rate and output driving capability of the */

/* SDRAM signal hold time control (+0x4c) */
#define PMU_SDCLK_SR_SHIFT		12
#define PMU_DQM_SR_SHIFT		13
#define PMU_CKE_SR_SHIFT		14
#define PMU_SDRAMCTL_SR_SHIFT		15
#define PMU_SDRAMCS_SR_SHIFT		16
#define PMU_EBIDATA_SR_SHIFT		17
#define PMU_EBICTRL_SR_SHIFT		18
#define PMU_DAT_WCLK_DLY_SHIFT		20
#define PMU_CTL_WCLK_DLY_SHIFT		24 /* RASB, CASB, WEB, OEB, BA, ADDR, and QAM */
#define PMU_RCLK_WCLK_DLY_SHIFT		28
#define PMU_RCLK_WCLK_DLY_MASK		0xf0000000 /* Control the delay value of SDRAMC clock to latch X_memdata */
#define PMU_SDCLK_SR_MASK		0x00001000 /* Control the slew rate of the SDRAMC's clock output pins */
#define PMU_DQM_SR_MASK			0x00002000 /* Control the slew rate of the SDRAMC's DQM pins */
#define PMU_CKE_SR_MASK			0x00004000 /* Control the slew rate of the SDRAMC's CKE pins */
#define PMU_SDRAMCTL_SR_MASK		0x00008000 /* Control the slew rate of the SDRAMC's control pins */
#define PMU_SDRAMCS_SR_MASK		0x00010000 /* Control the slew rate of the SDRAMC's chip select pins */
#define PMU_EBIDATA_SR_MASK		0x00020000 /* Control the slew rate of the X_memdata[31:0] */
#define PMU_EBICTRL_SR_MASK		0x00040000 /* Control the slew rate of the X_memaddr[14:0] */
#define PMU_DAT_WCLK_DLY_MASK		0x00f00000 /* Control the delay value of SDRAMC clock to clock out of X_memdata */
#define PMU_CTL_WCLK_DLY_MASK		0x0f000000 /* Control the delay value of SDRAMC clock to clock out of CKE, CSB, */

#define PMU_DLY_LEAD_2P4NS		0 /* lead 2.4ns */
#define PMU_DLY_LEAD_1P8NS		1
#define PMU_DLY_LEAD_1P2NS		2
#define PMU_DLY_LEAD_0P6NS		3
#define PMU_DLY_SAME_PHASE		4
#define PMU_DLY_LAG_0P6NS		5 /* lag 0.6ns */
#define PMU_DLY_LAG_1P2NS		6
#define PMU_DLY_LAG_1P8NS		7

/* AHB DMA REQ/ACK connection configuration status register (+0x90) */
#define PMU_CH0_REQACK_SHIFT		0
#define PMU_CH1_REQACK_SHIFT		4
#define PMU_CH2_REQACK_SHIFT		8
#define PMU_CH3_REQACK_SHIFT		12
#define PMU_CH4_REQACK_SHIFT		16
#define PMU_CH5_REQACK_SHIFT		20
#define PMU_CH6_REQACK_SHIFT		24
#define PMU_CH7_REQACK_SHIFT		28
#define PMU_CH0_REQACK_MASK		0x0000000f
#define PMU_CH1_REQACK_MASK		0x000000f0
#define PMU_CH2_REQACK_MASK		0x00000f00
#define PMU_CH3_REQACK_MASK		0x0000f000
#define PMU_CH4_REQACK_MASK		0x000f0000
#define PMU_CH5_REQACK_MASK		0x00f00000
#define PMU_CH6_REQACK_MASK		0x0f000000
#define PMU_CH7_REQACK_MASK		0xf0000000

#define PMU_REQN_NONE			0
#define PMU_REQN_CFC			1
#define PMU_REQN_SSP			2
#define PMU_REQN_UART1TX		3
#define PMU_REQN_UART1RX		4
#define PMU_REQN_UART2TX		5
#define PMU_REQN_UART2RX		6
#define PMU_REQN_SDC			7
#define PMU_REQN_I2SAC97		8
#define PMU_REQN_USB			11
#define PMU_REQN_EXT0			14
#define PMU_REQN_EXT1			15

/* External jummper setting status register (+0x9c) */
#define PMU_JMP_PLLDIS_BIT		0
#define PMU_JMP_INI_MBW_SHIFT		1
#define PMU_JMP_INTCPUOFF_BIT		3
#define PMU_JMP_PLL1SETTING_SHIFT	4
#define PMU_JMP_DEBUG_EN_BIT		7
#define PMU_JMP_DEBUG_SEL_BIT		8
#define PMU_JMP_TIMER_TEST_BIT		9
#define PMU_JMP_ENDIAN_BIT		10
#define PMU_JMP_DIVAHBCLK_SHIFT		11
/* please refer to value of PMU_DIVAHBCLK_~ */
#define PMU_JMP_DLLSETTING_SHIFT	15
#define PMU_JMP_PLLDIS_MASK		0x00000001 /* (ro) Jumper setting of PLL/DLL disable. System clock is OSC. */
#define PMU_JMP_INI_MBW_MASK		0x00000006 /* (ro) Jumper setting of ROM/FLASH data bus width */
#define PMU_JMP_INTCPUOFF_MASK		0x00000008 /* (ro) Jumper setting of internal CPU turn off */
#define PMU_JMP_PLL1SETTING_MASK	0x00000070 /* (ro) Jumper setting of PLL1 */
#define PMU_JMP_DEBUG_EN_MASK		0x00000080 /* (ro) Jumper setting of debug enable */
#define PMU_JMP_DEBUG_SEL_MASK		0x00000100 /* (ro) Jumper setting of debug selection */
#define PMU_JMP_TIMER_TEST_MASK		0x00000200 /* (ro) Jumper setting of timer test */
#define PMU_JMP_ENDIAN_MASK		0x00000400 /* (ro) Jumper setting of endian setting */
#define PMU_JMP_DIVAHBCLK_MASK		0x00007800 /* (ro) Jumper setting of DIVAHBCLK */
#define PMU_JMP_DLLSETTING_MASK		0x00018000 /* (ro) Jumper setting of DLL output frequency range */

#define PMU_JMP_INI_MBW_8BIT		0
#define PMU_JMP_INI_MBW_16BIT		1
#define PMU_JMP_INI_MBW_32BIT		2


#define PMU_JMP_PLL1_20X		0
#define PMU_JMP_PLL1_40X		1
#define PMU_JMP_PLL1_60X		2
#define PMU_JMP_PLL1_70X		3
#define PMU_JMP_PLL1_80X		4
#define PMU_JMP_PLL1_100X		5
#define PMU_JMP_PLL1_120X		6
#define PMU_JMP_PLL1_140X		7


#define PMU_JMP_LITTLEENDIAN		0
#define PMU_JMP_BIGENDIAN		1

/* please refer to value of PMU_DLLFRANG_~ */

/* CFC ..., etc, REQ/ACK connection configuration registers (0xa0 ~ 0xd8) */
#define PMU_CHANNEL_SHIFT		0
#define PMU_DMACUSED_BIT		3
#define PMU_CHANNEL_MASK		0x00000007 /* Indicate which channel is used in DMAC (ch0 ~ ch7) */
#define PMU_DMACUSED_MASK		0x00000008 /* 0: APB DMA, 1: AHB DMA */


/*****************************************************************************
 * Timer - AG101 Core APB
 ****************************************************************************/

/* Timer control register (+0x30) */
#define TMRC_TM1_ENABLE_MASK		0x00000001 /* Timer 1 enable bit */
#define TMRC_TM1_ENABLE_BIT		0
#define TMRC_TM1_CLOCK_MASK		0x00000002 /* Timer 1 clock source */
#define TMRC_TM1_CLOCK_BIT		1
#define TMRC_TM1_OFENABLE_MASK		0x00000004 /* Timer 1 overflow interrupt enable bit */
#define TMRC_TM1_OFENABLE_BIT		2
#define TMRC_TM2_ENABLE_MASK		0x00000008 /* Timer 2 enable bit */
#define TMRC_TM2_ENABLE_BIT		3
#define TMRC_TM2_CLOCK_MASK		0x00000010 /* Timer 2 clock source */
#define TMRC_TM2_CLOCK_BIT		4
#define TMRC_TM2_OFENABLE_MASK		0x00000020 /* Timer 2 overflow interrupt enable bit */
#define TMRC_TM2_OFENABLE_BIT		5
#define TMRC_TM3_ENABLE_MASK		0x00000040 /* Timer 3 enable bit */
#define TMRC_TM3_ENABLE_BIT		6
#define TMRC_TM3_CLOCK_MASK		0x00000080 /* Timer 3 clock source */
#define TMRC_TM3_CLOCK_BIT		7
#define TMRC_TM3_OFENABLE_MASK		0x00000100 /* Timer 3 overflow interrupt enable bit */
#define TMRC_TM3_OFENABLE_BIT		8
#define TMRC_TM1_UPDOWN_MASK		0x00000200 /* Timer 1 upward/downward counting */
#define TMRC_TM1_UPDOWN_BIT		9
#define TMRC_TM2_UPDOWN_MASK		0x00000400 /* Timer 2 upward/downward counting */
#define TMRC_TM2_UPDOWN_BIT		10
#define TMRC_TM3_UPDOWN_MASK		0x00000800 /* Timer 3 upward/downward counting */
#define TMRC_TM3_UPDOWN_BIT		11

/* Timer interrupt state (+0x34) */
#define TMRC_TM1_MATCH1_MASK		0x00000001 /* Timer 1 match 1 interrupt */
#define TMRC_TM1_MATCH1_BIT		0
#define TMRC_TM1_MATCH2_MASK		0x00000002 /* Timer 1 match 2 interrupt */
#define TMRC_TM1_MATCH2_BIT		1
#define TMRC_TM1_OVERFLOW_MASK		0x00000004 /* Timer 1 overflow interrupt */
#define TMRC_TM1_OVERFLOW_BIT		2

#define TMRC_TM2_MATCH1_MASK		0x00000008 /* Timer 2 match 1 interrupt */
#define TMRC_TM2_MATCH1_BIT		3
#define TMRC_TM2_MATCH2_MASK		0x00000010 /* Timer 2 match 2 interrupt */
#define TMRC_TM2_MATCH2_BIT		4
#define TMRC_TM2_OVERFLOW_MASK		0x00000020 /* Timer 2 overflow interrupt */
#define TMRC_TM2_OVERFLOW_BIT		5

#define TMRC_TM3_MATCH1_MASK		0x00000040 /* Timer 3 match 1 interrupt */
#define TMRC_TM3_MATCH1_BIT		6
#define TMRC_TM3_MATCH2_MASK		0x00000080 /* Timer 3 match 2 interrupt */
#define TMRC_TM3_MATCH2_BIT		7
#define TMRC_TM3_OVERFLOW_MASK		0x00000100 /* Timer 3 overflow interrupt */
#define TMRC_TM3_OVERFLOW_BIT		8

/* Timer interrupt mask (+0x38) */
#define TMRC_MTM1_MATCH1_MASK		0x00000001 /* Timer 1 match 1 interrupt mask */
#define TMRC_MTM1_MATCH1_BIT		0
#define TMRC_MTM1_MATCH2_MASK		0x00000002 /* Timer 1 match 2 interrupt mask */
#define TMRC_MTM1_MATCH2_BIT		1
#define TMRC_MTM1_OVERFLOW_MASK		0x00000004 /* Timer 1 overflow interrupt mask */
#define TMRC_MTM1_OVERFLOW_BIT		2

#define TMRC_MTM2_MATCH1_MASK		0x00000008 /* Timer 2 match 1 interrupt mask */
#define TMRC_MTM2_MATCH1_BIT		3
#define TMRC_MTM2_MATCH2_MASK		0x00000010 /* Timer 2 match 2 interrupt mask */
#define TMRC_MTM2_MATCH2_BIT		4
#define TMRC_MTM2_OVERFLOW_MASK		0x00000020 /* Timer 2 overflow interrupt mask */
#define TMRC_MTM2_OVERFLOW_BIT		5

#define TMRC_MTM3_MATCH1_MASK		0x00000040 /* Timer 3 match 1 interrupt mask */
#define TMRC_MTM3_MATCH1_BIT		6
#define TMRC_MTM3_MATCH2_MASK		0x00000080 /* Timer 3 match 2 interrupt mask */
#define TMRC_MTM3_MATCH2_BIT		7
#define TMRC_MTM3_OVERFLOW_MASK		0x00000100 /* Timer 3 overflow interrupt mask */
#define TMRC_MTM3_OVERFLOW_BIT		8


/*****************************************************************************
 * WDT - AG101 Core APB
 ****************************************************************************/


/*****************************************************************************
 * RTC - AG101 Core APB
 ****************************************************************************/

/* RTC control register (+0x20) */
#define RTC_ENABLE_MASK			0x00000001 /* RTC interrupt enable/disable */
#define RTC_ENABLE_BIT			0
#define RTC_INTSEC_MASK			0x00000002 /* RTC interrupt for every second */
#define RTC_INTSEC_BIT			1
#define RTC_INTMIN_MASK			0x00000004 /* RTC interrupt for every minute */
#define RTC_INTMIN_BIT			2
#define RTC_INTHOUR_MASK		0x00000008 /* RTC interrupt for every hour */
#define RTC_INTHOUR_BIT			3
#define RTC_INTDAY_MASK			0x00000010 /* RTC interrupt for every day */
#define RTC_INTDAY_BIT			4
#define RTC_ALARM_MASK			0x00000020 /* RTC alarm interrupt enable/disable */
#define RTC_ALARM_BIT			5
#define RTC_COUNTERLOAD_MASK		0x00000040 /* RTC counter reload enable/disable register */
#define RTC_COUNTERLOAD_BIT		6

/* RTC interrupt register (+34) */
#define RTC_ISR_SEC_MASK		0x00000001 /* sec interrupt */
#define RTC_ISR_SEC_BIT			0
#define RTC_ISR_MIN_MASK		0x00000002 /* minute interrupt */
#define RTC_ISR_MIN_BIT			1
#define RTC_ISR_HOUR_MASK		0x00000004 /* hour interrupt */
#define RTC_ISR_HOUR_BIT		2
#define RTC_ISR_DAY_MASK		0x00000008 /* day interrupt */
#define RTC_ISR_DAY_BIT			3
#define RTC_ISR_ALARM_MASK		0x00000010 /* alarm interrupt */
#define RTC_ISR_ALARM_BIT		4

/* RTC frequency divider register (+0x38) */
#define RTC_DIV_CYCLE_MASK		0x7fffffff /* divider, so that 1Hz = ext_clock / (divider * 1000) */
#define RTC_DIV_CYCLE_SHIFT		0
#define RTC_DIV_ENABLE_MASK		0x80000000 /* sec interrupt */
#define RTC_DIV_ENABLE_BIT		31


/*****************************************************************************
 * GPIO - AG101 Core APB
 ****************************************************************************/

/* GPIO direction register (+0x08) */
#define GPIOC_INPUT			0 /* gpio_en set to 0 */
#define GPIOC_OUTPUT			1 /* gpio_en set to 1 */

/* GPIO pin bypass register (+0x0c) */
#define GPIOC_DISABLE			0 /* disable bypass mode */
#define GPIOC_ENABLE			1 /* gpio_en -> gpio_bps_en, gpio_in -> gpio_bps_out, gpio_bps_in -> gpio_out */

/* GPIO data bit set register (+0x10) */
/* GPIO data bit clear register (+0x14) */
#define GPIOC_BIT(pin)			((uint32_t)(1 << (pin)))

/* GPIO pull enable register (+0x18) */
#define GPIOC_DISABLE			0 /* the pin is not pulled */
#define GPIOC_ENABLE			1 /* the pin is pulled */

/* GPIO pull hi/lo register (+0x1c) */
#define GPIOC_PULL_LOW			0 /* the pin is pulled low */
#define GPIOC_PULL_HIGH			1 /* the pin is pulled high */

/* GPIO interrupt enable register (+0x20) */
#define GPIOC_DISABLE			0 /* the pin interrupt is disabled */
#define GPIOC_ENABLE			1 /* the pin interrupt is enabled */

/* GPIO interrupt raw status register (+0x24) */
/* GPIO interrupt masked status register (+0x28) */
#define GPIOC_NONE			0 /* (+0x24) the pin interrupt is not detected */
/* (+0x28) the pin interrupt is not detected or is masked */
#define GPIOC_ACT			1 /* (+0x24) the pin interrupt is detected */
/* (+0x28) the pin interrupt is detected and not masked */

/* GPIO interrupt mask register (+0x2c) */
#define GPIOC_NO_MASK			0 /* the pin interrupt mask is disabled */
#define GPIOC_MASK			1 /* the pin interrupt mask is enabled */

/* GPIO interrupt clear register (+0x30) */
#define GPIOC_NO_CLEAR			0 /* no effect */
#define GPIOC_CLEAR			1 /* the pin interrupt is cleared */

/* GPIO interrupt trigger method register (+0x34) */
#define GPIOC_EDGE			0 /* the pin interrupt is edge trigger */
#define GPIOC_LEVEL			1 /* the pin interrupt is level trigger */

/* GPIO interrupt edge trigger type register (+0x38) */
#define GPIOC_SINGLE			0 /* the pin interrupt is single edge trigger */
#define GPIOC_BOTH			1 /* the pin interrupt is triggered in both edge */

/* GPIO interrupt trigger side/level register (+0x3c) */
#define GPIOC_RISING			0 /* the pin interrupt is rise edge trigger */
#define GPIOC_FALLING			1 /* the pin interrupt is falling edge trigger */

#define GPIOC_HIGH			0 /* the pin interrupt is high level trigger */
#define GPIOC_LOW			1 /* the pin interrupt is low level trigger */

/* GPIO interrupt pre-scale clock enable register (+0x40) */
#define GPIOC_DISABLE			0 /* the pin interrupt is sampled by pclk */
#define GPIOC_ENABLE			1 /* the pin interrupt is sampled by (pclk / pre_scale_value) */

/* GPIO interrupt pre-scale div value register (+0x44) */
#define GPIOC_BP_MASK			0xffff /* valid range of pre-scale div value */


/*****************************************************************************
 * INTC - AG101 Core APB
 ****************************************************************************/

/* Interrupt Routing Table */
#define INTC_CFC_CD			0x00000001 /* CFC card insertion detection */
#define INTC_CFC_DMA			0x00000002 /* CFC DMA completion */
#define INTC_SSP1			0x00000004 /* SSP1 (SSP) interrupt */
#define INTC_SSP			INTC_SSP1
#define INTC_I2C			0x00000008 /* I2C interrupt */

#define INTC_SDC			0x00000020 /* SDC interrupt */
#define INTC_SSP2			0x00000040 /* SSP2 (I2S/AC97) interrupt */
#define INTC_I2S			INTC_SSP2
#define INTC_AC97			INTC_SSP2
#define INTC_STUART			0x00000080 /* STUART interrupt */

#define INTC_PMU			0x00000100 /* PMU interrupt */
#define INTC_BTUART			0x00000800 /* BTUART interrupt */

#define INTC_HW5			0x00001000 /* external-device/HW5 interrupt */
#define INTC_GPIO			0x00002000 /* GPIO interrupt */
#define INTC_TM2			0x00004000 /* Timer2 interrupt */
#define INTC_TM3			0x00008000 /* Timer3 interrupt */

#define INTC_WDT			0x00010000 /* WDT interrupt */
#define INTC_RTC_ALARM			0x00020000 /* RTC alarm interrupt */
#define INTC_RTC_SEC			0x00040000 /* RTC sec interrupt */
#define INTC_TM1			0x00080000 /* Timer1 interrupt */

#define INTC_DMA			0x00200000 /* DMA interrupt */

#define INTC_APB			0x01000000 /* APB bridge interface status */
#define INTC_MAC			0x02000000 /* Ethernet MAC interrupt */
#define INTC_USB			0x04000000 /* USB 2.0 interrupt */
#define INTC_HW4			0x08000000 /* external-device/HW4 interrupt */

#define INTC_HW0			0x10000000 /* external-device/HW0 interrupt */
#define INTC_HW1			0x20000000 /* external-device/HW1 interrupt */
#define INTC_HW2			0x40000000 /* external-device/HW2 interrupt */
#define INTC_HW3			0x80000000 /* external-device/HW3 interrupt */

/* Interrupt Bit Shift */
#define INTC_CFC_CD_BIT			0 /* CFC card insertion detection */
#define INTC_CFC_DMA_BIT		1 /* CFC DMA completion */
#define INTC_SSP1_BIT			2 /* SSP1 (SSP) interrupt */
#define INTC_SSP_BIT			INTC_SSP1_BIT
#define INTC_I2C_BIT			3 /* I2C interrupt */

#define INTC_SDC_BIT			5 /* SDC interrupt */
#define INTC_SSP2_BIT			6 /* SSP2 (I2S/AC97) interrupt */
#define INTC_I2S_BIT			INTC_SSP2_BIT
#define INTC_AC97_BIT			INTC_SSP2_BIT
#define INTC_STUART_BIT			7 /* STUART interrupt */

#define INTC_PMU_BIT			8 /* PMU interrupt */
#define INTC_BTUART_BIT			11 /* BTUART interrupt */

#define INTC_HW5_BIT			12 /* external-device/HW5 interrupt */
#define INTC_GPIO_BIT			13 /* GPIO interrupt */
#define INTC_TM2_BIT			14 /* Timer2 interrupt */
#define INTC_TM3_BIT			15 /* Timer3 interrupt */

#define INTC_WDT_BIT			16 /* WDT interrupt */
#define INTC_RTC_ALARM_BIT		17 /* RTC alarm interrupt */
#define INTC_RTC_SEC_BIT		18 /* RTC sec interrupt */
#define INTC_TM1_BIT			19 /* Timer1 interrupt */

#define INTC_DMA_BIT			21 /* DMA interrupt */

#define INTC_APB_BIT			24 /* APB bridge interface status */
#define INTC_MAC_BIT			25 /* Ethernet MAC interrupt */
#define INTC_USB_BIT			26 /* USB 2.0 interrupt */
#define INTC_HW4_BIT			27 /* external-device/HW4 interrupt */

#define INTC_HW0_BIT			28 /* external-device/HW0 interrupt */
#define INTC_HW1_BIT			29 /* external-device/HW1 interrupt */
#define INTC_HW2_BIT			30 /* external-device/HW2 interrupt */
#define INTC_HW3_BIT			31 /* external-device/HW3 interrupt */

/* Trigger Mode Register */
#define INTC_TMR_LEVEL			0x00 /* level trigger */
#define INTC_TMR_EDGE			0x01 /* edge trigger */

/* Trigger Level Register */
#define INTC_TLR_AL			0x00 /* Assert Low */
#define INTC_TLR_AH			0x01 /* Assert High */


/*****************************************************************************
 * SSP/I2S/AC97 - AG101 DMA APB
 ****************************************************************************/

/* SSP control register 0 (+0x00) */
#define SSPC_C0_SCLKPH_BIT		0
#define SSPC_C0_SCLKPO_BIT		1
#define SSPC_C0_OPM_SHIFT		2
#define SSPC_C0_FSJSTFY_BIT		4
#define SSPC_C0_FSPO_BIT		5
#define SSPC_C0_LSB_BIT			6
#define SSPC_C0_LBM_BIT			7
#define SSPC_C0_FSDIST_SHIFT		8
#define SSPC_C0_FFMT_SHIFT		12

#define SSPC_C0_SCLKPH_MASK		0x00000001 /* SCLK phase (SPI, NSMW) */
#define SSPC_C0_SCLKPO_MASK		0x00000002 /* SClK polarity (SPI, NSMW) */
#define SSPC_C0_OPM_MASK		0x0000000c /* Operation mode */
#define SSPC_C0_FSJSTFY_MASK		0x00000010 /* Data justify (I2S) */
#define SSPC_C0_FSPO_MASK		0x00000020 /* Frame sync polarity (non-I2S) */
#define SSPC_C0_LSB_MASK		0x00000040 /* Bit sequence indicator, 0: msb first, 1: lsb first */
#define SSPC_C0_LBM_MASK		0x00000080 /* Loop back mode */
#define SSPC_C0_FSDIST_MASK		0x00000300 /* Frame sync and data distance (I2S) */
#define SSPC_C0_FFMT_MASK		0x00007000 /* Frame format */

#define SSPC_SSP_SLAVE			0 /* (SSP, SPI, NSMW) */
#define SSPC_SSP_MASTER			2

#define SSPC_I2S_SLAVE_MONO		0
#define SSPC_I2S_SLAVE_STEREO		1 /* (I2S) */
#define SSPC_I2S_MASTER_MONO		2
#define SSPC_I2S_MASTER_STEREO		3

#define SSPC_TI_SSP			0 /* Texas Instrument Synchronous Serial Port */
#define SSPC_MOTO_SPI			1 /* Motorola Serial Peripheral Interface */
#define SSPC_NS_MW			2 /* National Semiconductor Microwire */
#define SSPC_PHILIPS_I2S		3 /* Philips I2S */
#define SSPC_INTEL_ACLINK		4 /* Intel AC-link */

/* SSP control register 1 (+0x04) */
#define SSPC_C1_SCLKDIV_SHIFT		0
#define SSPC_C1_SDL_SHIFT		16
#define SSPC_C1_PDL_SHIFT		24
#define SSPC_C1_SCLKDIV_MASK		0x0000ffff /* SCLK divider */
#define SSPC_C1_SDL_MASK		0x001f0000 /* Serial data "bit" length = (SDL + 1) bits (ignored in AC97(auto-set)) */
#define SSPC_C1_PDL_MASK		0xff000000 /* Padding data length (I2S, NSMW) */

/* SSP control register 2 (+0x08) */
#define SSPC_C2_SSPEN_BIT		0
#define SSPC_C2_TXDOE_BIT		1 /* I2S - (0) receving/recording mode (1) simultaneous transmitting/receiveing mode */
#define SSPC_C2_RXFCLR_BIT		2
#define SSPC_C2_TXFCLR_BIT		3
#define SSPC_C2_ACWRST_BIT		4
#define SSPC_C2_ACCRST_BIT		5
#define SSPC_C2_SSPRST_BIT		6
#define SSPC_C2_SSPEN_MASK		0x00000001 /* SSP enable (kickoff/stop, ignored if AC97 ACCRST is set) */
#define SSPC_C2_TXDOE_MASK		0x00000002 /* SSP - Transmit data output enable */
#define SSPC_C2_RXFCLR_MASK		0x00000004 /* Receive FIFO clear */
#define SSPC_C2_TXFCLR_MASK		0x00000008 /* Transmit FIFO clear */
#define SSPC_C2_ACWRST_MASK		0x00000010 /* AC-link warm reset enable */
#define SSPC_C2_ACCRST_MASK		0x00000020 /* AC-link cold reset enable */
#define SSPC_C2_SSPRST_MASK		0x00000040 /* SSP reset */

/* SSP status register (+0x0c) */
#define SSPC_SR_RFF_BIT			0
#define SSPC_SR_TFNF_BIT		1
#define SSPC_SR_BUSY_BIT		2
#define SSPC_SR_RFVE_SHIFT		4
#define SSPC_SR_TFVE_SHIFT		12
#define SSPC_SR_RFF_MASK		0x00000001 /* Receive FIFO full */
#define SSPC_SR_TFNF_MASK		0x00000002 /* Transmit FIFO not-full (available for DMA or core write) */
#define SSPC_SR_BUSY_MASK		0x00000004 /* Busy indicator (whether busy in tx or rx) */
#define SSPC_SR_RFVE_MASK		0x000001f0 /* Receive FIFO valid entries (waiting in FIFO for read) */
#define SSPC_SR_TFVE_MASK		0x0001f000 /* Transmit FIFO valid entries (waiting in FIFO to be transmitted) */

/* SSP interrupt control register (+0x10) */
#define SSPC_INTCR_RFORIEN_BIT		0
#define SSPC_INTCR_TFURIEN_BIT		1
#define SSPC_INTCR_RFTHIEN_BIT		2
#define SSPC_INTCR_TFTHIEN_BIT		3
#define SSPC_INTCR_RFDMAEN_BIT		4
#define SSPC_INTCR_TFDMAEN_BIT		5
#define SSPC_INTCR_AC97FCENEN_BIT	6
#define SSPC_INTCR_RFTHOD_SHIFT		8
#define SSPC_INTCR_TFTHOD_SHIFT		12
#define SSPC_INTCR_RFORIEN_MASK		0x00000001 /* Receive FIFO over-run interrupt enable */
#define SSPC_INTCR_TFURIEN_MASK		0x00000002 /* Transmit FIFO under-run interrupt enable */
#define SSPC_INTCR_RFTHIEN_MASK		0x00000004 /* Receive FIFO threshold interrupt enable (int when >= threshold) */
#define SSPC_INTCR_TFTHIEN_MASK		0x00000008 /* Transmit FIFO threshold interrupt enable (int when <= threshold) */
#define SSPC_INTCR_RFDMAEN_MASK		0x00000010 /* Receive DMA request enable */
#define SSPC_INTCR_TFDMAEN_MASK		0x00000020 /* Transmit DMA request enable */
#define SSPC_INTCR_AC97FCEN_MASK	0x00000040 /* AC97 frame complete interrupt enable */
#define SSPC_INTCR_RFTHOD_MASK		0x00000f00 /* Receive FIFO threshold */
#define SSPC_INTCR_TFTHOD_MASK		0x0000f000 /* Transmit FIFO threshold */

/* SSP interrupt status register (+0x14) */
#define SSPC_INTSR_RFORI_BIT		0
#define SSPC_INTSR_TFURI_BIT		1
#define SSPC_INTSR_RFTHI_BIT		2
#define SSPC_INTSR_TFTHI_BIT		3
#define SSPC_INTSR_AC97FCI_BIT		4
#define SSPC_INTSR_RFORI_MASK		0x00000001 /* Receive FIFO over-run interrupt status clear */
#define SSPC_INTSR_TFURI_MASK		0x00000002 /* Transmit FIFO under-run interrupt status clear */
#define SSPC_INTSR_RFTHI_MASK		0x00000004 /* Receive FIFO threshold interrupt status clear */
#define SSPC_INTSR_TFTHI_MASK		0x00000008 /* Transmit FIFO threshold interrupt status clear */
#define SSPC_INTSR_AC97FCI_MASK		0x00000010 /* AC97 frame complete interrupt status clear */

/* AC-Link slot valid register (+0x20) */
#define SSPC_AC97_CODECID_SHIFT		0
#define SSPC_AC97_SLOT12V_BIT		3
#define SSPC_AC97_SLOT11V_BIT		4
#define SSPC_AC97_SLOT10V_BIT		5
#define SSPC_AC97_SLOT9V_BIT		6
#define SSPC_AC97_SLOT8V_BIT		7
#define SSPC_AC97_SLOT7V_BIT		8
#define SSPC_AC97_SLOT6V_BIT		9
#define SSPC_AC97_SLOT5V_BIT		10
#define SSPC_AC97_SLOT4V_BIT		11
#define SSPC_AC97_SLOT3V_BIT		12
#define SSPC_AC97_SLOT2V_BIT		13
#define SSPC_AC97_SLOT1V_BIT		14
#define SSPC_AC97_SLOT0V_BIT		15
#define SSPC_AC97_CODECID_MASK		0x00000003 /* CODEDC ID which will be shifted out at TAG slot */
#define SSPC_AC97_SLOT12V_MASK		0x00000008 /* 12th slot valid bit */
#define SSPC_AC97_SLOT11V_MASK		0x00000010 /* 11th slot valid bit */
#define SSPC_AC97_SLOT10V_MASK		0x00000020 /* 10th slot valid bit */
#define SSPC_AC97_SLOT9V_MASK		0x00000040 /* 9th slot valid bit */
#define SSPC_AC97_SLOT8V_MASK		0x00000080 /* 8th slot valid bit */
#define SSPC_AC97_SLOT7V_MASK		0x00000100 /* 7th slot valid bit */
#define SSPC_AC97_SLOT6V_MASK		0x00000200 /* 6th slot valid bit */
#define SSPC_AC97_SLOT5V_MASK		0x00000400 /* 5th slot valid bit */
#define SSPC_AC97_SLOT4V_MASK		0x00000800 /* 4th slot valid bit */
#define SSPC_AC97_SLOT3V_MASK		0x00001000 /* 3rd slot valid bit */
#define SSPC_AC97_SLOT2V_MASK		0x00002000 /* 2nd slot valid bit */
#define SSPC_AC97_SLOT1V_MASK		0x00004000 /* 1st slot valid bit */
#define SSPC_AC97_SLOT0V_MASK		0x00008000 /* 0st slot valid bit */

/* Codec id helper macro */
#define SSPC_AC97_MAKE_CODECID(id)	(((uint32_t)(id) << SSPC_AC97_CODECID_SHIFT) & SSPC_AC97_CODECID_MASK)

/* Pre-defined slot-valid values for issuing AC97 read/write commands */
#define SSPC_AC97_WCMD_SLOTS_MASK	((uint32_t)(SSPC_AC97_SLOT0V_MASK | SSPC_AC97_SLOT1V_MASK | SSPC_AC97_SLOT2V_MASK))
#define SSPC_AC97_WCMD_SLOTS		2

#define SSPC_AC97_RCMD_SLOTS_MASK	((uint32_t)(SSPC_AC97_SLOT0V_MASK | SSPC_AC97_SLOT1V_MASK | SSPC_AC97_SLOT2V_MASK))
#define SSPC_AC97_RCMD_SLOTS		2

#define SSPC_AC97_PCM_SLOTS_MASK	((uint32_t)(SSPC_AC97_SLOT0V_MASK | SSPC_AC97_SLOT3V_MASK | SSPC_AC97_SLOT4V_MASK))
#define SSPC_AC97_PCM_SLOTS		2

#define SSPC_AC97_PCM51_SLOTS_MASK	((uint32_t)(SSPC_AC97_SLOT0V_MASK | SSPC_AC97_SLOT3V_MASK | SSPC_AC97_SLOT4V_MASK |\
			SSPC_AC97_SLOT6V_MASK | SSPC_AC97_SLOT7V_MASK | SSPC_AC97_SLOT8V_MASK |\
			SSPC_AC97_SLOT9V_MASK))
#define SSPC_AC97_PCM51_SLOTS		6


/*****************************************************************************
 * CFC - AG101 DMA APB
 ****************************************************************************/

/* CFC Properties */
#define CFC_FIFO_WORD_DEPTH		16
#define CFC_FIFO_BYTE_DEPTH		(CFC_FIFO_WORD_DEPTH * 4)

/* CF host status register (+0x00) */
#define CFC_HSR_RDY_IREQ_BIT		0
#define CFC_HSR_CD_BIT			1
#define CFC_HSR_VS1_BIT			2
#define CFC_HSR_VS2_BIT			3
#define CFC_HSR_BVD1_STSCHG_BIT		4
#define CFC_HSR_BVD2_SPKR_BIT		5
#define CFC_HSR_ACTB_BUSY_BIT		8
#define CFC_HSR_ACTB_WRDY_BIT		9
#define CFC_HSR_ACTB_INT_BIT		10 /*			Also the status of the cfc_int_data_cmp_r pin. */
#define CFC_HSR_ACTB_SIZE_SHIFT		12
#define CFC_HSR_CD_INT_BIT		16
#define CFC_HSR_IO_INT_BIT		17


#define CFC_HSR_RDY_IREQ_MASK		0x00000001 /* Status of the 37th CF interface pin, RDY/-IREQ */
#define CFC_HSR_CD_MASK			0x00000002 /* The status of card insertion/removal, 0: removed; 1: inserted */
#define CFC_HSR_VS1_MASK		0x00000004 /* Status of 33rd CF interface pin, -VS1 (Voltage Sense 1) */
#define CFC_HSR_VS2_MASK		0x00000008 /* Status of 40rd CF interface pin, -VS2 (Voltage Sense 2) */
#define CFC_HSR_BVD1_STSCHG_MASK	0x00000010 /* Status of 46rd CF interface pin, BVD1/-STSCHG */
#define CFC_HSR_BVD2_SPKR_MASK		0x00000020 /* Status of 45rd CF interface pin, BVD2/-SPKR */
#define CFC_HSR_ACTB_BUSY_MASK		0x00000100 /* (Active buffer) This bit is high until total length of data has been transferred. */
#define CFC_HSR_ACTB_WRDY_MASK		0x00000200 /* (Active buffer) High indicates ready for a word of data transfer (r: high to have data in, w: high to have free slot) */
#define CFC_HSR_ACTB_INT_MASK		0x00000400 /* (Active buffer) High indicates ready for a byte of data transfer. Write 1 to clear. */
#define CFC_HSR_ACTB_SIZE_MASK		0x0000f000 /* Indicates the configuration buffer size */
#define CFC_HSR_CD_INT_MASK		0x00010000 /* Card state change (removed->inserted, inserted/removed) interrupt status.			Write 1 to clear. */
#define CFC_HSR_IO_INT_MASK		0x00020000 /* Asserted when CF card signals an interrupt in I/O mode mode through cfc_rdy pin. */

#define CFC_RDY_IREQ_NOT_READY		0
#define CFC_RDY_IREQ_READY		1

#define CFC_CARD_REMOVED		0x00000000
#define CFC_CARD_DETECTED		0x00000002

#define CFC_ACTB_READY			0x00000000 /* Active buffer is ready */
#define CFC_ACTB_BUSY			0x00000100 /* Active buffer is busy transferring data */
#define CFC_ACTB_WORD_BUSY		0x00000000 /* Active buffer is busy and no room for a word of data transferr */
#define CFC_ACTB_WORD_READY		0x00000200 /* Active buffer is ready for a word of data transfer */

#define CFC_ACTB_INT_BUSY		0x00000000 /* Active buffer is shifting out data */
#define CFC_ACTB_INT_TRIGGERED		0x00000400 /* Active buffer has shifted out all data */
#define CFC_ACTB_INT_CLEAR		0x00000400 /* Clear the active buffer transfer completion status */

/* CF host control register (+0x04) */
#define CFC_HCR_PWRCTL_SHIFT		0
#define CFC_HCR_FLOATCTL_BIT		4
#define CFC_HCR_RESET_BIT		5
#define CFC_HCR_8BIT_MODE_BIT		6
#define CFC_HCR_DMA_EN_BIT		8
#define CFC_HCR_CD_INT_BIT		9
#define CFC_HCR_ACTB_INT_BIT		10
#define CFC_HCR_IO_INT_BIT		11

#define CFC_HCR_PWRCTL_MASK		0x0000000f /* Control power supply through output pin cfc_pctrl0~3. */
#define CFC_HCR_FLOATCTL_MASK		0x00000010 /* When the pin is low, following control outputs will be floated: -OE, -IORD, -IOWR, -CE1, -CE2, -WE and RESET. */
#define CFC_HCR_RESET_MASK		0x00000020 /* Directly connects to the RESET pin of CF interface */
#define CFC_HCR_8BIT_MODE_MASK		0x00000040 /* CF host treats CF card as an 8-bit if this bit was set */

#define CFC_HCR_DMA_EN_MASK		0x00000100 /* DMA mode(1) or PIO mode(0) */
#define CFC_HCR_CD_INT_MASK		0x00000200 /* Enable(1)/Disable(0) the interrupt signal of cfc_int_cd_r */
#define CFC_HCR_ACTB_INT_MASK		0x00000400 /* Enable(1)/Disable(0) the interrupt signal of cfc_int_data_cmp_r */
#define CFC_HCR_IO_INT_MASK		0x00000800 /* Enable(1)/Disable(0) the interrupt signal of cfc_io_int_r */

/* Access timing configuration register (+0x08) */
#define CFC_ATCR_BSA_SHIFT		0
#define CFC_ATCR_BSM_SHIFT		4
#define CFC_ATCR_BSIO_SHIFT		8
#define CFC_ATCR_BSMOW_SHIFT		12
#define CFC_ATCR_BSIORW_SHIFT		14
#define CFC_ATCR_BSA_MASK		0x0000000f /* Attribute memory access timing configuration register */
#define CFC_ATCR_BSM_MASK		0x000000f0 /* Common memory access timing configuration register */
#define CFC_ATCR_BSIO_MASK		0x00000f00 /* I/O access timing configuration register */
#define CFC_ATCR_BSMOW_MASK		0x00003000 /* Timing configuration of -OE/-WE pulse width */
#define CFC_ATCR_BSIORW_MASK		0x0000c000 /* Timing configuration of -IORD/-IOWR pulse width */

/* Active buffer controller register (+0x0c) */
#define CFC_ABCR_ADR_SHIFT		0
#define CFC_ABCR_TYPE_SHIFT		12
#define CFC_ABCR_RW_BIT			15
#define CFC_ABCR_SIZE_SHIFT		16
#define CFC_ABCR_ADR_MASK		0x000007ff /* Target/Source address */
#define CFC_ABCR_TYPE_MASK		0x00003000 /* Access type */
#define CFC_ABCR_RW_MASK		0x00008000 /* Write(1) or read(0) access to/from CF card */
#define CFC_ABCR_SIZE_MASK		0x000f0000 /* Transfer size. (note: I/O mode max is 512 bytes) */

#define CFC_TYPE_ATTR_MEM_FUNC		0 /* Attribute memory function */
#define CFC_TYPE_COMM_MEM_FUNC		2 /* Common memory function */
#define CFC_TYPE_IO_FUNC		3 /* I/O function */

#define CFC_ABCR_INCADR_MASK		0x00004000 /* Increment(1) or keep(0) the target/source address after each access */
#define CFC_ABCR_INCADR_BIT		14
#define CFC_INCADR_FIX			0
#define CFC_INCADR_INC			1

#define CFC_AB_READ			0
#define CFC_AB_WRITE			1

#define CFC_ABCR_SIZE_1			1 /* 1 byte */
#define CFC_ABCR_SIZE_2			2 /* 2 bytes */
#define CFC_ABCR_SIZE_4			3 /* 4 bytes */
#define CFC_ABCR_SIZE_8			4 /* 8 bytes */
#define CFC_ABCR_SIZE_16		5 /* 16 bytes */
#define CFC_ABCR_SIZE_32		6 /* 32 bytes */
#define CFC_ABCR_SIZE_64		7 /* 64 bytes */
#define CFC_ABCR_SIZE_128		8 /* 128 bytes */
#define CFC_ABCR_SIZE_256		9 /* 256 bytes */
#define CFC_ABCR_SIZE_512		10 /* 512 bytes */
#define CFC_ABCR_SIZE_1024		11 /* 1024 bytes */
#define CFC_ABCR_SIZE_2048		12 /* 2048 bytes */

#define CFC_MAKE_ABCR(rw, type, addr, inc, size)		\
	((((uint32_t)(addr) << CFC_ABCR_ADR_SHIFT) & CFC_ABCR_ADR_MASK)			|	\
			(((uint32_t)(type) << CFC_ABCR_TYPE_SHIFT) & CFC_ABCR_TYPE_MASK)	|	\
			(((uint32_t)(inc) << CFC_ABCR_INCADR_BIT) & CFC_ABCR_INCADR_MASK)	|	\
			(((uint32_t)(rw) << CFC_ABCR_RW_BIT) & CFC_ABCR_RW_MASK)		|	\
			(((uint32_t)(size) << CFC_ABCR_SIZE_SHIFT) & CFC_ABCR_SIZE_MASK))


/* Multi-sector register (+0x14) */
#define CFC_MSR_ENABLE_BIT		0
#define CFC_MSR_TIMEUP_SHIFT		1
#define CFC_MSR_ENABLE_MASK		0x00000001 /* Multi-sector read/write enable */
#define CFC_MSR_TIMEUP_MASK		0x000000fe /* Number of PCLK to wait before checking cfc_rdy each time one sector is transferred */

/* Transfer size mode2 enable register (+0x18) */
#define CFC_MER_ENABLE_BIT		0
#define CFC_MER_ENABLE_MASK		0x00000001 /* Transfer size is controlled by 0: CFC_ABCR_SIZE; 1: CFC_MCR_SIZE */

/* Transfer size mode2 counter register (+0x1c) */
#define CFC_MCR_SIZE_SHIFT		0 /* (note: I/O mode max is 512 bytes) */
#define CFC_MCR_SIZE_MASK		0x0001ffff /* Mode 2 transfer size = (CFC_MCR_SIZE + 1) bytes (1 ~ 128K) */
#define CFC_MCR_SIZE_MIN		0
#define CFC_MCR_SIZE_MAX		((uint32_t)0x0001ffff)

/*****************************************************************************
 * MMC/SDC - AG101 DMA APB
 ****************************************************************************/

/* SDC Properties */
#define SDC_FIFO_WORD_DEPTH		4
#define SDC_FIFO_BYTE_DEPTH		(SDC_FIFO_WORD_DEPTH * 4)

/* Command register (+0x00) */
#define SDC_CMD_IDX_SHIFT		0
#define SDC_NEED_RSP_BIT		6
#define SDC_LONG_RSP_BIT		7
#define SDC_APP_CMD_BIT			8
#define SDC_CMD_EN_BIT			9
#define SDC_SDC_RST_BIT			10
#define SDC_CMD_IDX_MASK		0x0000003f /* Command index, sent as part of a command */
#define SDC_NEED_RSP_MASK		0x00000040 /* If set, the command waits for a response from card */
#define SDC_LONG_RSP_MASK		0x00000080 /* If set, the command waits for a 136-bit long response from card */
#define SDC_APP_CMD_MASK		0x00000100 /* Indicates the command is application specific */
#define SDC_CMD_EN_MASK			0x00000200 /* If set, this command is enabled */
#define SDC_SDC_RST_MASK		0x00000400 /* Set to reset the SD host controller */

/* Responded command register (+0x18) */
#define SDC_RSP_CMD_IDX_SHIFT		0
#define SDC_RSP_CMD_APP_BIT		6
#define SDC_RSP_CMD_IDX_MASK		0x0000003f /* Response command index */
#define SDC_RSP_CMD_APP_MASK		0x00000060 /* Indicates the command is application specific */

/* Data control register (+0x1c) */
#define SDC_BLK_SIZE_SHIFT		0
#define SDC_DATA_WRITE_BIT		4
#define SDC_DMA_EN_BIT			5
#define SDC_DATA_EN_BIT			6
#define SDC_BLK_SIZE_MASK		0x0000000f /* Data size per block = 2^BLK_SIZE (bytes), max 2^11 = 2048 (bytes) */
#define SDC_DATA_WRITE_MASK		0x00000010 /* 0: read data from card; 1: write data to card */
#define SDC_DMA_EN_MASK			0x00000020 /* 0: disable DMA; 1: enable DMA */
#define SDC_DATA_EN_MASK		0x00000040 /* Enable data transfer cycle */

/* Data length register (+0x24) */
#define SDC_DATA_LEN_SHIFT		0
#define SDC_DATA_LEN_MASK		0x0000ffff /* Data bytes to be transferred */

/* Status register (+0x28) */
#define SDC_SR_RSP_CRC_FAIL_BIT		0
#define SDC_SR_DATA_CRC_FAIL_BIT	1
#define SDC_SR_RSP_TIMEOUT_BIT		2
#define SDC_SR_DATA_TIMEOUT_BIT		3
#define SDC_SR_RSP_CRC_OK_BIT		4
#define SDC_SR_DATA_CRC_OK_BIT		5
#define SDC_SR_CMD_SENT_BIT		6
#define SDC_SR_DATA_END_BIT		7
#define SDC_SR_FIFO_URUN_BIT		8
#define SDC_SR_FIFO_ORUN_BIT		9
#define SDC_SR_CARD_CHANGE_BIT		10
#define SDC_SR_CARD_DETECT_BIT		11
#define SDC_SR_WRITE_PROT_BIT		12
#define SDC_SR_RSP_CRC_FAIL_MASK	0x00000001 /* Command response received but CRC check failed */
#define SDC_SR_DATA_CRC_FAIL_MASK	0x00000002 /* Data block sent/received but CRC check failed */
#define SDC_SR_RSP_TIMEOUT_MASK		0x00000004 /* Command response timeout */
#define SDC_SR_DATA_TIMEOUT_MASK	0x00000008 /* Data read/programming timeout */
#define SDC_SR_RSP_CRC_OK_MASK		0x00000010 /* Command response received and CRC check passed */
#define SDC_SR_DATA_CRC_OK_MASK		0x00000020 /* Data block sent/received and CRC check passed */
#define SDC_SR_CMD_SENT_MASK		0x00000040 /* Command sent (no response required) */
#define SDC_SR_DATA_END_MASK		0x00000080 /* Data transfer finished */
#define SDC_SR_FIFO_URUN_MASK		0x00000100 /* Data FIFO underrun */
#define SDC_SR_FIFO_ORUN_MASK		0x00000200 /* Data FIFO overrun */
#define SDC_SR_CARD_CHANGE_MASK		0x00000400 /* Card is inserted or removed */
#define SDC_SR_CARD_DETECT_MASK		0x00000800 /* 0: card inserted; 1: card removed */
#define SDC_SR_WRITE_PROT_MASK		0x00001000 /* 0: not write protect; 1: card is write protect */

/* Clear register (+0x2c) */
/* Interrupt mask register (+0x30) */
/* (note: spec "mask" means "enable", spec uses wrong term to describe this register) */
#define SDC_RSP_CRC_FAIL_BIT		0
#define SDC_DATA_CRC_FAIL_BIT		1
#define SDC_RSP_TIMEOUT_BIT		2
#define SDC_DATA_TIMEOUT_BIT		3
#define SDC_RSP_CRC_OK_BIT		4
#define SDC_DATA_CRC_OK_BIT		5
#define SDC_CMD_SENT_BIT		6
#define SDC_DATA_END_BIT		7
#define SDC_FIFO_URUN_BIT		8
#define SDC_FIFO_ORUN_BIT		9
#define SDC_CARD_CHANGE_BIT		10
#define SDC_RSP_CRC_FAIL_MASK		0x00000001 /* Mask RSP_CRC_FAIL flag */
#define SDC_DATA_CRC_FAIL_MASK		0x00000002 /* Mask DATA_CRC_FAIL flag */
#define SDC_RSP_TIMEOUT_MASK		0x00000004 /* Mask RSP_TIMEOUT flag */
#define SDC_DATA_TIMEOUT_MASK		0x00000008 /* Mask DATA_TIMEOUT flag */
#define SDC_RSP_CRC_OK_MASK		0x00000010 /* Mask RSP_CRC_OK flag */
#define SDC_DATA_CRC_OK_MASK		0x00000020 /* Mask DATA_CRC_OK flag */
#define SDC_CMD_SENT_MASK		0x00000040 /* Mask CMD_SENT flag */
#define SDC_DATA_END_MASK		0x00000080 /* Mask DATA_END flag */
#define SDC_FIFO_URUN_MASK		0x00000100 /* Mask FIFO underrun flag */
#define SDC_FIFO_ORUN_MASK		0x00000200 /* Mask FIFO overrun flag */
#define SDC_CARD_CHANGE_MASK		0x00000400 /* Mask CARD_CHANGE flag */

/* all mask bits */
#define SDC_CLEAR_ALL			(uint32_t)(SDC_RSP_CRC_FAIL_MASK | SDC_DATA_CRC_FAIL_MASK | SDC_RSP_TIMEOUT_MASK |			\
			SDC_DATA_TIMEOUT_MASK | SDC_RSP_CRC_OK_MASK | SDC_DATA_CRC_OK_MASK |			\
			SDC_CMD_SENT_MASK | SDC_DATA_END_MASK | SDC_FIFO_URUN_MASK |			\
			SDC_FIFO_ORUN_MASK | SDC_CARD_CHANGE_MASK) /* should be 0x7ff */
#define SDC_MASK_ALL			((uint32_t)0x00) /*((uint32_t)~(SDC_MASK_ALL)) */

/* all clear bits */
#define SDC_UNMASK_ALL			SDC_CLEAR_ALL

/* Power control register (+0x34) */
#define SDC_SD_POWER_SHIFT		0
#define SDC_SD_POWER_ON_BIT		4
#define SDC_SD_POWER_MASK		0x0000000f /* Control the external power supply output range */
#define SDC_SD_POWER_ON_MASK		0x00000010 /* Control the external power supply on/off (0: off; 1: on) */

#define SDC_POWER_ON			1
#define SDC_POWER_OFF			0

/* Clock control register (+0x38) */
#define SDC_CLK_DIV_SHIFT		0
#define SDC_CLK_SD_BIT			7
#define SDC_CLK_DIS_BIT			8
#define SDC_CLK_DIV_MASK		0x0000007f /* Control the clock frequency of SD_CLK */
#define SDC_CLK_SD_MASK			0x00000080 /* 0: MMC memory card; 1: SD memory card */
#define SDC_CLK_DIS_MASK		0x00000100 /* 0: SD_CLK enable; 1: SD_CLK disable */

#define SDC_CLK_MMC			0
#define SDC_CLK_SD			1


#define SDC_CLK_ON			0
#define SDC_CLK_OFF			1

/* Bus width register (+0x3c) */
#define SDC_SINGLE_BUS_BIT		0
#define SDC_WIDE_BUS_BIT		2
#define SDC_WIDE_BUS_SUPPORT_BIT	3
#define SDC_SINGLE_BUS_MASK		0x00000001 /* Bus width = 1 (DAT0) */
#define SDC_WIDE_BUS_MASK		0x00000004 /* Bus width = 4 (DAT0~3) */
#define SDC_WIDE_BUS_SUPPORT_MASK	0x00000008 /* (ro) 1: 4-bit bus is implemented */


/*****************************************************************************
 * UARTC - AG101 DMA APB
 ****************************************************************************/
/* todo: */
/* For historical reason, UART registers did not all follow MASK/SHIFT/BIT */
/* definition logic. This was left as an todo item. */

/* Macros for specifying which UART to use. */
#define UARTC_NUM_DEVICES		2

/* IER Register (+0x04) */
#define UARTC_IER_RDR			0x01 /* Data Ready Enable */
#define UARTC_IER_THRE			0x02 /* THR Empty Enable */
#define UARTC_IER_RLS			0x04 /* Receive Line Status Enable */
#define UARTC_CIER_MS			0x08 /* Modem Staus Enable */

/* IIR Register (+0x08) */
#define UARTC_IIR_NONE			0x01 /* No interrupt pending */
#define UARTC_IIR_RLS			0x06 /* Receive Line Status */
#define UARTC_IIR_RDR			0x04 /* Receive Data Ready */
#define UARTC_IIR_RTO			0x0c /* Receive Time Out */
#define UARTC_IIR_THRE			0x02 /* THR Empty */
#define UARTC_IIR_MODEM			0x00 /* Modem Status */
#define UARTC_IIR_INT_MASK		0x0f /* Initerrupt Status Bits Mask */

#define UARTC_IIR_TFIFO_FULL		0x10 /* TX FIFO full */
#define UARTC_IIR_FIFO_EN		0xc0 /* FIFO mode is enabled, set when FCR[0] is 1 */

/* FCR Register (+0x08) */
#define UARTC_FCR_FIFO_EN		0x01 /* FIFO Enable */
#define UARTC_FCR_RFIFO_RESET		0x02 /* Rx FIFO Reset */
#define UARTC_FCR_TFIFO_RESET		0x04 /* Tx FIFO Reset */
#define UARTC_FCR_DMA_EN		0x08 /* Select UART DMA mode */

#define UARTC_FCR_TFIFO16_TRGL1		0x00 /* TX 16-byte FIFO int trigger level - 1 char */
#define UARTC_FCR_TFIFO16_TRGL3		0x10 /* TX 16-byte FIFO int trigger level - 3 char */
#define UARTC_FCR_TFIFO16_TRGL9		0x20 /* TX 16-byte FIFO int trigger level - 9 char */
#define UARTC_FCR_TFIFO16_TRGL13	0x30 /* TX 16-byte FIFO int trigger level - 13 char */

#define UARTC_FCR_RFIFO16_TRGL1		0x00 /* RX 16-byte FIFO int trigger level - 1 char */
#define UARTC_FCR_RFIFO16_TRGL4		0x40 /* RX 16-byte FIFO int trigger level - 4 char */
#define UARTC_FCR_RFIFO16_TRGL8		0x80 /* RX 16-byte FIFO int trigger level - 8 char */
#define UARTC_FCR_RFIFO16_TRGL14	0xc0 /* RX 16-byte FIFO int trigger level - 14 char */

/* FCR Register (+0x08) */
#define UARTC_FCR_FIFO_EN_MASK		0x01 /* FIFO Enable */
#define UARTC_FCR_FIFO_EN_BIT		0
#define UARTC_FCR_RFIFO_RESET_MASK	0x02 /* Rx FIFO Reset */
#define UARTC_FCR_RFIFO_RESET_BIT	1
#define UARTC_FCR_TFIFO_RESET_MASK	0x04 /* Tx FIFO Reset */
#define UARTC_FCR_TFIFO_RESET_BIT	2
#define UARTC_FCR_DMA_EN_MASK		0x08 /* Select UART DMA mode */
#define UARTC_FCR_DMA_EN_BIT		3

#define UARTC_FCR_TXFIFO_TRGL_MASK	0x30 /* TX FIFO int trigger level */
#define UARTC_FCR_TXFIFO_TRGL_SHIFT	4
#define UARTC_FCR_RXFIFO_TRGL_MASK	0xc0 /* RX FIFO int trigger level */
#define UARTC_FCR_RXFIFO_TRGL_SHIFT	6

/* LCR Register (+0x0c) */
#define UARTC_LCR_BITS5			0x00
#define UARTC_LCR_BITS6			0x01
#define UARTC_LCR_BITS7			0x02
#define UARTC_LCR_BITS8			0x03
#define UARTC_LCR_STOP1			0x00
#define UARTC_LCR_STOP2			0x04

#define UARTC_LCR_PARITY_EN		0x08 /* Parity Enable */
#define UARTC_LCR_PARITY_NONE		0x00 /* No Parity Check */
#define UARTC_LCR_PARITY_EVEN		0x18 /* Even Parity */
#define UARTC_LCR_PARITY_ODD		0x08 /* Odd Parity */
#if 0
#define UARTC_LCR_PARITY_1		0x21 /* 1 Parity Bit */
#define UARTC_LCR_PARITY_0		0x31 /* 0 Parity Bit */
#endif
#define UARTC_LCR_SETBREAK		0x40 /* Set Break condition */
#define UARTC_LCR_DLAB			0x80 /* Divisor Latch Access Bit */

/* MCR Register (+0x10) */
#define UARTC_MCR_DTR			0x01 /* Data Terminal Ready */
#define UARTC_MCR_RTS			0x02 /* Request to Send */
#define UARTC_MCR_OUT1			0x04 /* output1 */
#define UARTC_MCR_OUT2			0x08 /* output2 or global interrupt enable */
#define UARTC_MCR_LPBK			0x10 /* loopback mode */
#define UARTC_MCR_DMAMODE2		0x20 /* DMA mode2 */
#define UARTC_MCR_OUT3			0x40 /* output 3 */

/* LSR Register (+0x14) */
#define UARTC_LSR_RDR			0x1 /* Data Ready */
#define UARTC_LSR_OE			0x2 /* Overrun Error */
#define UARTC_LSR_PE			0x4 /* Parity Error */
#define UARTC_LSR_FE			0x8 /* Framing Error */
#define UARTC_LSR_BI			0x10 /* Break Interrupt */
#define UARTC_LSR_THRE			0x20 /* THR/FIFO Empty */
#define UARTC_LSR_TE			0x40 /* THR/FIFO and TFR Empty */
#define UARTC_LSR_DE			0x80 /* FIFO Data Error */

/* MSR Register (+0x18) */
#define UARTC_MSR_DELTACTS		0x1 /* Delta CTS */
#define UARTC_MSR_DELTADSR		0x2 /* Delta DSR */
#define UARTC_MSR_TERI			0x4 /* Trailing Edge RI */
#define UARTC_MSR_DELTACD		0x8 /* Delta CD */
#define UARTC_MSR_CTS			0x10 /* Clear To Send */
#define UARTC_MSR_DSR			0x20 /* Data Set Ready */
#define UARTC_MSR_RI			0x40 /* Ring Indicator */
#define UARTC_MSR_DCD			0x80 /* Data Carrier Detect */

/* MDR register (+0x20) */
#define UARTC_MDR_MODE_SEL_SHIFT	0
#define UARTC_MDR_SIP_BYCPU_BIT		2
#define UARTC_MDR_FMEND_MD_BIT		3
#define UARTC_MDR_DMA_EN_BIT		4
#define UARTC_MDR_FIR_INV_RX_BIT	5
#define UARTC_MDR_IR_INV_TX_BIT		6
#define UARTC_MDR_MODE_SEL_MASK		0x03
#define UARTC_MDR_SIP_BYCPU_MASK	0x04 /* 0: 1.6us end pulse; 1: depends on ACR[4] */
#define UARTC_MDR_FMEND_MD_MASK		0x08 /* 0: Frame length counter method; 1: Set end of transmission bit method */
#define UARTC_MDR_DMA_EN_MASK		0x10 /* Enable DMA mode. (PIO int should turn off) */
#define UARTC_MDR_FIR_INV_RX_MASK	0x20 /* (FIR only) Invert receiver input signal */
#define UARTC_MDR_IR_INV_TX_MASK	0x40 /* (FIR/SIR) Invert pulse during transmission */

#define UARTC_MDR_MODE_UART		0
#define UARTC_MDR_MODE_SIR		1
#define UARTC_MDR_MODE_FIR		2

/* ACR register (+0x24) */
#define UARTC_ACR_IR_TX_EN		0x01
#define UARTC_ACR_IR_RX_EN		0x02
#define UARTC_ACR_FIR_SETEOT		0x04

/*****************************************************************************
 * Clock Definition
 ****************************************************************************/

/*****************************************************************************
 *
 *
 * System clock reference value
 * (Note: This is for refernece only)
 *
 *   AG101 real chip board:
 *     OSCCLK            10000000    ( 10.0 MHz )
 *     OSCLCLK           32768       ( 32.768 KHz )
 *     PLL2CLK           OSCCLK*20   ( 200 MHz (internal use only) )
 *     PLL3CLK           OSCCLK*30   ( 300 MHz )
 *     SSPCLK_GPIO22     24576000 or 25175000   ( 24.576 or 25.175 MHz (SSP1) )
 *     AC97CLK_GPIO25    24576000 or 25175000   ( 24.576 or 25.175 MHz (SSP2/I2S/AC97) )
 *     UCLK              36864000    ( 36.864 MHz (UARTC) )
 *
 *   FPGA
 *     OSCCLK            3686400     ( 3.6864 MHz )
 *     OSCLCLK           32768       ( 32.768 KHz )
 *     PLL2CLK           47923200    ( 47.9232 MHz (IrDA) )
 *     PLL3CLK           147465000   ( 147.465 MHz )
 *     SSPCLK_GPIO22     25175000    ( 25.175 MHz (SSP1) )
 *     AC97CLK_GPIO25    24576000    ( 24.576 MHz (SSP2/AC97) )
 *     UCLK              18432000    ( 18.432 MHz (UARTC) )
 *
 * System clock derivation:
 * (todo: auto configuration)
 *
 *   PLL1_out = OSCCLK * PLL1NS (FCS)
 *   CPUCLK   = PLL1_out * FSF (/1 /2 /3 /4 /5 /6)
 *   HCLK     = CPUCLK * DIVAHBCLK (2/3 2/5 /1 /2 ... /20)
 *   PCLK     = HCLK / 2
 *
 *   AG101 note
 *     Todo: Should be able to auto configure. Spec:
 *       Known: PLL1NS, FSF, DIVAHBCLK, counter
 *       Derived by kernel: CPU_CLK, HCLK, PCLK
 *       Derived by driver: bus clock, device clock
 *
 *   FPGA note
 *     HCLK = 48 MHz
 *     CPU  = 96 MHz
 *
 * I2S clock derivation:
 *
 *   I2SCLK = PLL3CLK / I2SCLKDIV[3:0]  (PDLLCR1[19:16] I2SCLKDIV)
 *            (or from GPIO 22)
 *
 * SSP clock derivation:
 *
 *   SSPCLK = PLL3CLK / 6
 *            (or from GPIO25)
 *
 *                              SSP_CLK
 *   SCLK (Serial Clock) = -----------------
 *                          2 (SCLKDIV + 1)
 *
 * AC97 clock-1 derivation: (main clock)
 *
 *   AC97CLK1 = PLL3CLK / 3
 *              (or from GPIO 22)
 *
 * AC97 clock-2 derivation: (BIT_CLK * 2 source)
 *
 *   AC97CLK2 = PLL3CLK / 6
 *              (or from GPIO 22)
 *
 *   AC97 AC-link signals
 *
 *     BIT_CLK (SSP input form AC97 codec) = AC97CLK2 / 2 (or AC97CLK1 / 4 ?)
 *
 *     SYNC (SSP output to AC97 codec)     = BIT_CLK / 256 (should be 48KHZ)
 *
 *     Note in the AG101 case, it has silghtly deviation that may cause trouble
 *
 *       BIT_CLK = 25MHz / 2     = 12.5 MHZ   (12.288 MHz is required)
 *       SYNC    = 12.5MHZ / 256 = 48.828 KHz (48.0 KHz is required)
 *
 * UART baudrate derivation:
 *
 *   UCLK     = PLL3CLK / 8
 *
 *                   UCLK
 *   baudrate = ------------------
 *               (16) (PSR) (DL)
 *
 *   where ...
 *     PSR [4:0]
 *       0    : no output to DL;
 *       1~31 : (default: 1)
 *     DL [15:0]  DLM:DLL
 *       0    : no baudrate (power-save)
 *       1~65535
 *
 *   Special note for AG101: (UCLK = 18750000)
 *
 *     target_baudrate  PSR  DL(ideal)   DLM   DLL   actual_baudrate    err
 *     ---------------  ---  ---------  ----  ----  ----------------  -----
 *      115200           10    1.01725  0x00  0x01   117187.5           17% (usable?)
 *       38400           10    3.05175  0x00  0x03    39062             17% (usable?)
 *        9600            1  122.07000  0x00  0x7a     9605           0.05%
 *        4800            2  122.07000  0x00  0x7a     4802.766       0.05%
 *        1200            8  122.07000  0x00  0x7a     1200.6915      0.05%
 *        1200           16   61.03500  0x00  0x3d     1200.6915      0.05%
 *
 *   Special note for AG101: (UCLK = 37500000)
 *
 *     target_baudrate  PSR  DL(ideal)   DLM   DLL   actual_baudrate    err
 *     ---------------  ---  ---------  ----  ----  ----------------  -----
 *      115200           10    2.03450  0x00  0x01   117187.5           17% (usable?)
 *       38400           10    6.10350  0x00  0x03    39062             17% (usable?)
 *        9600            1  244.14000  0x00  0x7a     9605           0.05%
 *        4800            2  122.14000  0x00  0x7a     4802.766       0.05%
 *        1200            8  122.14000  0x00  0x7a     1200.6915      0.05%
 *        1200           16  122.07000  0x00  0x3d     1200.6915      0.05%
 *
 * SD clock
 *
 *                 PCLK
 *   SD_CLK = --------------------
 *             2 (CLK_DIV + 1)
 *
 *   For AG101, assume FSF = 1/1, then the maximum frequency range of CPU, HCLK,
 *   PCLK, SD_CLK that could be derived are listed as following:
 *
 *     OSC    PLL1_core    CPU_CLK    HCLK    PCLK   SD_CLK(CLK_DIV=0x00) SD_CLK(CLK_DIV=0xff)
 *   ------  -----------  ---------  ------  ------  -------------------- --------------------
 *     5M       100M        100M      100M     50M           25M               97.66K
 *               |           |         |        |             |                  |
 *              700M        700M      700M    350M          175M              683.59K
 *
 *   SD Memory Card specification:
 *     Fod (card identification mode)                = 100K ~ 400K
 *     Fpp (data transfer state default mode) max    = 0 ~ 25M
 *     Fpp (data transfer state high-speed mode) max = 0 ~ 50M
 *
 ****************************************************************************/

/*****************************************************************************
 * Compile options for selecting device clock source
 ****************************************************************************/

#define KHz			1000
#define MHz			1000000

#  define MB_OSCCLK		(10 * MHz)
#  define MB_PCLK		(15 * MHz)

/* Device Clock Source Selection */
#ifdef CONFIG_PLAT_AG101_4GB
#  define MB_SSP_EXT_CLK	1 /* non-zero to use external clock source */
#  define MB_AC97_EXT_CLK	1 /* non-zero to use external clock source */
#else
#  define MB_SSP_EXT_CLK	0 /* non-zero to use external clock source */
#  define MB_AC97_EXT_CLK	0 /* non-zero to use external clock source */
#endif

#if 0
#if defined("CONFIG_PLAT_AG101_4GB")

#  define MB_SSP_EXT_CLK	1 /* non-zero to use external clock source */
#  define MB_AC97_EXT_CLK	1 /* non-zero to use external clock source */

#elif defined("CONFIG_PLAT_AG101P_4GB")
#  define MB_SSP_EXT_CLK	0 /* non-zero to use external clock source */
#  define MB_AC97_EXT_CLK	0 /* non-zero to use external clock source */

#elif defined("CONFIG_PLAT_QEMU")

#  define MB_SSP_EXT_CLK	0 /* non-zero to use external clock source */
#  define MB_AC97_EXT_CLK	0 /* non-zero to use external clock source */

#elif defined("CONFIG_PLAT_AG101P_16MB")
#  define MB_SSP_EXT_CLK	0 /* non-zero to use external clock source */
#  define MB_AC97_EXT_CLK	0 /* non-zero to use external clock source */

#else

#  error "No valid platform is defined!"

#endif
#endif
/*****************************************************************************
 * Interface & Definitions
 ****************************************************************************/

/* todo: timer-polling method */
#if (defined(CONFIG_CPU_ICACHE_ENABLE) && defined(CONFIG_CPU_DCACHE_ENABLE))

#define _nds_kwait(count)						\
	do {								\
		volatile uint32_t i = 0;				\
		while (i++ < (uint32_t)(count))				\
			;						\
	} while(0)
#else

#define _nds_kwait(count)						\
	do {								\
		volatile uint32_t i = 0;				\
		uint32_t c = (count > 0x10) ? count / 0x10 : 0x10;	\
		while (i++ < (uint32_t)(c))				\
			;						\
	} while(0)
#endif

#endif /* __AG101_DEFS_H__ */
