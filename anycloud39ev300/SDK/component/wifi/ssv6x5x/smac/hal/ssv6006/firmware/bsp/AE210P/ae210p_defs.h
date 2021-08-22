/*****************************************************************************
 *
 *            Copyright Andes Technology Corporation 2014
 *                         All Rights Reserved.
 *
 *  Revision History:
 *
 *    Jan.11.2014     Created.
 ****************************************************************************/

#ifndef __AE210_DEFS_H__
#define __AE210_DEFS_H__

/*****************************************************************************
 * AHB_SLAVE_4_7 - AE210P AHB
 ****************************************************************************/


/*****************************************************************************
 * BMC (APB Decoder)- AE210P AHB
 ****************************************************************************/


/*****************************************************************************
 * DMAC - AE210P AHB
 ****************************************************************************/

/*****************************************************************************
 * AHB_SLAVE_0_3 - AE210P AHB
 ****************************************************************************/

//TODO
//finish this table
/*****************************************************************************
 * APBBR(N/A) - AE210P AHB to APB Bridge
 ****************************************************************************/

/*****************************************************************************
 * SMU - AE210P Core APB
 ****************************************************************************/

/*****************************************************************************
 * UARTx - AE210P Core APB
 ****************************************************************************/
/* Macros for specifying which UART to use. */
#define UARTC_NUM_DEVICES               2

/* IER Register (+0x04) */
#define UARTC_IER_RDR                   0x01 /* Data Ready Enable */
#define UARTC_IER_THRE                  0x02 /* THR Empty Enable */
#define UARTC_IER_RLS                   0x04 /* Receive Line Status Enable */
#define UARTC_CIER_MS                   0x08 /* Modem Staus Enable */

/* IIR Register (+0x08) */
#define UARTC_IIR_NONE                  0x01 /* No interrupt pending */
#define UARTC_IIR_RLS                   0x06 /* Receive Line Status */
#define UARTC_IIR_RDR                   0x04 /* Receive Data Ready */
#define UARTC_IIR_RTO                   0x0c /* Receive Time Out */
#define UARTC_IIR_THRE                  0x02 /* THR Empty */
#define UARTC_IIR_MODEM                 0x00 /* Modem Status */
#define UARTC_IIR_INT_MASK              0x0f /* Initerrupt Status Bits Mask */

#define UARTC_IIR_TFIFO_FULL            0x10 /* TX FIFO full */
#define UARTC_IIR_FIFO_EN               0xc0 /* FIFO mode is enabled, set when FCR[0] is 1 */

/* FCR Register (+0x08) */
#define UARTC_FCR_FIFO_EN               0x01 /* FIFO Enable */
#define UARTC_FCR_RFIFO_RESET           0x02 /* Rx FIFO Reset */
#define UARTC_FCR_TFIFO_RESET           0x04 /* Tx FIFO Reset */
#define UARTC_FCR_DMA_EN                0x08 /* Select UART DMA mode */

#define UARTC_FCR_TFIFO16_TRGL1         0x00 /* TX 16-byte FIFO int trigger level - 1 char */
#define UARTC_FCR_TFIFO16_TRGL3         0x10 /* TX 16-byte FIFO int trigger level - 3 char */
#define UARTC_FCR_TFIFO16_TRGL9         0x20 /* TX 16-byte FIFO int trigger level - 9 char */
#define UARTC_FCR_TFIFO16_TRGL13        0x30 /* TX 16-byte FIFO int trigger level - 13 char */

#define UARTC_FCR_RFIFO16_TRGL1         0x00 /* RX 16-byte FIFO int trigger level - 1 char */
#define UARTC_FCR_RFIFO16_TRGL4         0x40 /* RX 16-byte FIFO int trigger level - 4 char */
#define UARTC_FCR_RFIFO16_TRGL8         0x80 /* RX 16-byte FIFO int trigger level - 8 char */
#define UARTC_FCR_RFIFO16_TRGL14        0xc0 /* RX 16-byte FIFO int trigger level - 14 char */

/* FCR Register (+0x08) */
#define UARTC_FCR_FIFO_EN_MASK          0x01 /* FIFO Enable */
#define UARTC_FCR_FIFO_EN_BIT           0
#define UARTC_FCR_RFIFO_RESET_MASK      0x02 /* Rx FIFO Reset */
#define UARTC_FCR_RFIFO_RESET_BIT       1
#define UARTC_FCR_TFIFO_RESET_MASK      0x04 /* Tx FIFO Reset */
#define UARTC_FCR_TFIFO_RESET_BIT       2
#define UARTC_FCR_DMA_EN_MASK           0x08 /* Select UART DMA mode */
#define UARTC_FCR_DMA_EN_BIT            3

#define UARTC_FCR_TXFIFO_TRGL_MASK      0x30 /* TX FIFO int trigger level */
#define UARTC_FCR_TXFIFO_TRGL_SHIFT     4
#define UARTC_FCR_RXFIFO_TRGL_MASK      0xc0 /* RX FIFO int trigger level */
#define UARTC_FCR_RXFIFO_TRGL_SHIFT     6

/* LCR Register (+0x0c) */
#define UARTC_LCR_BITS5                 0x00
#define UARTC_LCR_BITS6                 0x01
#define UARTC_LCR_BITS7                 0x02
#define UARTC_LCR_BITS8                 0x03
#define UARTC_LCR_STOP1                 0x00
#define UARTC_LCR_STOP2                 0x04

#define UARTC_LCR_PARITY_EN             0x08 /* Parity Enable */
#define UARTC_LCR_PARITY_NONE           0x00 /* No Parity Check */
#define UARTC_LCR_PARITY_EVEN           0x18 /* Even Parity */
#define UARTC_LCR_PARITY_ODD            0x08 /* Odd Parity */
#if 0
#define UARTC_LCR_PARITY_1              0x21 /* 1 Parity Bit */
#define UARTC_LCR_PARITY_0              0x31 /* 0 Parity Bit */
#endif
#define UARTC_LCR_SETBREAK              0x40 /* Set Break condition */
#define UARTC_LCR_DLAB                  0x80 /* Divisor Latch Access Bit */

/* MCR Register (+0x10) */
#define UARTC_MCR_DTR                   0x01 /* Data Terminal Ready */
#define UARTC_MCR_RTS                   0x02 /* Request to Send */
#define UARTC_MCR_OUT1                  0x04 /* output1 */
#define UARTC_MCR_OUT2                  0x08 /* output2 or global interrupt enable */
#define UARTC_MCR_LPBK                  0x10 /* loopback mode */
#define UARTC_MCR_DMAMODE2              0x20 /* DMA mode2 */
#define UARTC_MCR_OUT3                  0x40 /* output 3 */

/* LSR Register (+0x14) */
#define UARTC_LSR_RDR                   0x1 /* Data Ready */
#define UARTC_LSR_OE                    0x2 /* Overrun Error */
#define UARTC_LSR_PE                    0x4 /* Parity Error */
#define UARTC_LSR_FE                    0x8 /* Framing Error */
#define UARTC_LSR_BI                    0x10 /* Break Interrupt */
#define UARTC_LSR_THRE                  0x20 /* THR/FIFO Empty */
#define UARTC_LSR_TE                    0x40 /* THR/FIFO and TFR Empty */
#define UARTC_LSR_DE                    0x80 /* FIFO Data Error */

/* MSR Register (+0x18) */
#define UARTC_MSR_DELTACTS              0x1 /* Delta CTS */
#define UARTC_MSR_DELTADSR              0x2 /* Delta DSR */
#define UARTC_MSR_TERI                  0x4 /* Trailing Edge RI */
#define UARTC_MSR_DELTACD               0x8 /* Delta CD */
#define UARTC_MSR_CTS                   0x10 /* Clear To Send */
#define UARTC_MSR_DSR                   0x20 /* Data Set Ready */
#define UARTC_MSR_RI                    0x40 /* Ring Indicator */
#define UARTC_MSR_DCD                   0x80 /* Data Carrier Detect */

/* MDR register (+0x20) */
#define UARTC_MDR_MODE_SEL_SHIFT        0
#define UARTC_MDR_SIP_BYCPU_BIT         2
#define UARTC_MDR_FMEND_MD_BIT          3
#define UARTC_MDR_DMA_EN_BIT            4
#define UARTC_MDR_FIR_INV_RX_BIT        5
#define UARTC_MDR_IR_INV_TX_BIT         6
#define UARTC_MDR_MODE_SEL_MASK         0x03
#define UARTC_MDR_SIP_BYCPU_MASK        0x04 /* 0: 1.6us end pulse; 1: depends on ACR[4] */
#define UARTC_MDR_FMEND_MD_MASK         0x08 /* 0: Frame length counter method; 1: Set end of transmission bit method */
#define UARTC_MDR_DMA_EN_MASK           0x10 /* Enable DMA mode. (PIO int should turn off) */
#define UARTC_MDR_FIR_INV_RX_MASK       0x20 /* (FIR only) Invert receiver input signal */
#define UARTC_MDR_IR_INV_TX_MASK        0x40 /* (FIR/SIR) Invert pulse during transmission */

#define UARTC_MDR_MODE_UART             0
#define UARTC_MDR_MODE_SIR              1
#define UARTC_MDR_MODE_FIR              2

/* ACR register (+0x24) */
#define UARTC_ACR_IR_TX_EN              0x01
#define UARTC_ACR_IR_RX_EN              0x02
#define UARTC_ACR_FIR_SETEOT            0x04

/*****************************************************************************
 * PIT - AG101 Core APB
 ****************************************************************************/

/* Interrupt Enable Register */
#define PIT_CH_NUM_MASK			0x7

/* Channel & Interrupt Enable Reg */
#define PIT_C0_TMR0_EN			0x1 
#define PIT_C0_TMR1_EN			0x2
#define PIT_C0_TMR2_EN			0x4
#define PIT_C0_TMR3_EN			0x8

#define PIT_C1_TMR0_EN			0x10
#define PIT_C1_TMR1_EN			0x20
#define PIT_C1_TMR2_EN			0x40
#define PIT_C1_TMR3_EN			0x80

#define PIT_C2_TMR0_EN			0x100
#define PIT_C2_TMR1_EN			0x200
#define PIT_C2_TMR2_EN			0x400
#define PIT_C2_TMR3_EN			0x800

#define PIT_C3_TMR0_EN			0x1000
#define PIT_C3_TMR1_EN			0x2000
#define PIT_C3_TMR2_EN			0x4000
#define PIT_C3_TMR3_EN			0x8000

/* Interrupt Status Register */
/* Clean Timer interrupt pending bit, write 1 clean */
#define PIT_C0_TMR0_PEND_W1C		0x1
#define PIT_C0_TMR1_PEND_W1C		0x2
#define PIT_C0_TMR2_PEND_W1C		0x4
#define PIT_C0_TMR3_PEND_W1C		0x8

#define PIT_C1_TMR0_PEND_W1C		0x10
#define PIT_C1_TMR1_PEND_W1C		0x20
#define PIT_C1_TMR2_PEND_W1C		0x40
#define PIT_C1_TMR3_PEND_W1C		0x80

#define PIT_C2_TMR0_PEND_W1C		0x100
#define PIT_C2_TMR1_PEND_W1C		0x200
#define PIT_C2_TMR2_PEND_W1C		0x400
#define PIT_C2_TMR3_PEND_W1C		0x800

#define PIT_C3_TMR0_PEND_W1C		0x1000
#define PIT_C3_TMR1_PEND_W1C		0x2000
#define PIT_C3_TMR2_PEND_W1C		0x4000
#define PIT_C3_TMR3_PEND_W1C		0x8000

/* channel 0~3 control register */
/* ChClk*/
#define PIT_CH_CTL_APBCLK		0x8 
/* ChMode*/
#define PIT_CH_CTL_TMR32		0x1
#define PIT_CH_CTL_TMR16		0x2
#define PIT_CH_CTL_TMR8			0x3
#define PIT_CH_CTL_PWM			0x4
#define PIT_CH_CTL_MIX16		0x6
#define PIT_CH_CTL_MIX8			0x7


/*****************************************************************************
 * WDT - AG101 Core APB
 ****************************************************************************/

//TODO
//finish this table
/*****************************************************************************
 * RTC - AE210P APB
 ****************************************************************************/


//TODO
//Finish this table
/*****************************************************************************
 * GPIO - AE210P APB
 ****************************************************************************/

/*****************************************************************************
 * I2C - AG101 Core APB
 ****************************************************************************/

/*****************************************************************************
 * SPI1 - AG101 Core APB
 ****************************************************************************/

/*****************************************************************************
 * SPI2 - AG101 Core APB
 ****************************************************************************/

/*****************************************************************************
 * APB_SLAVE_0_4 - AG101 Core APB
 ****************************************************************************/


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

//#  define MB_OSCCLK		(10 * MHz)
#  define MB_PCLK		(40 * MHz)

/* Device Clock Source Selection */
#  define MB_SSP_EXT_CLK	0 /* non-zero to use external clock source */
#  define MB_AC97_EXT_CLK	0 /* non-zero to use external clock source */


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

/* TODO: timer-polling method */
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

#endif /* __AE210P_DEFS_H__ */
