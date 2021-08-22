#include "turismo_defs.h"
#include "turismo_regs.h"

#include <nds32_intrinsic.h>

#define DEFAULT_BAUDRATE	38400

#define IN8(reg) 		(uint8_t)( (*(volatile unsigned long *)(reg)) & 0x000000FF )
#define READ_CLR(reg) 	(*(volatile unsigned long *)(reg))

int drv_uart_set_baudrate(int baudrate)
{
    /* CPU=40MHz, Baudrate=115200 */
    /* CPU=10MHz, Baudrate=28800 */
    #define UART0_SPR               (0x0015U)
    #define UART1_SPR               (0x0005U)

    SET_BRDC_DIV(UART0_SPR);
#if 0
	unsigned long baud_div;	/* baud rate divisor */
	unsigned long temp_word;

	/* Set the baud rate */
#ifdef CONFIG_PLAT_AG101_4GB
	/* We use cpu_ver to identify A0/B0 in TTP board or AG101 board */
#define AG101B0 0x0c020003
	unsigned long cpu_ver;
	unsigned long UARTC_CLOCK;
	cpu_ver = __nds32__mfsr(NDS32_SR_CPU_VER);
	if(cpu_ver != AG101B0) {
		*(volatile unsigned long *)0x902ffffc = 0xa0;
		UARTC_CLOCK = 36864000; /* in AG101A0 */
	} else {
		*(volatile unsigned long *)0x902ffffc = 0xb0;
		UARTC_CLOCK = 18432000; /* in AG101B0_TTP */
	}

#elif defined(CONFIG_PLAT_AE210P)
	#define UARTC_CLOCK  (20*1000000)		/* in XC7, 20M Hz*/
#else
	#define UARTC_CLOCK          14745600           /* in XC5, 15M Hz */
#endif

	baud_div = (UARTC_CLOCK / (16 * baudrate));

	/* Save LCR temporary */
	temp_word = IN8(STUARTC_BASE + UARTC_LCR_OFFSET);

	/* Setup dlab bit for baud rate setting */
	OUT8(STUARTC_BASE + UARTC_LCR_OFFSET, (temp_word | UARTC_LCR_DLAB));

	/* Apply baud rate */
	OUT8(STUARTC_BASE + UARTC_DLM_OFFSET, (unsigned char)(baud_div >> 8));
	OUT8(STUARTC_BASE + UARTC_DLL_OFFSET, (unsigned char)baud_div);
	OUT8(STUARTC_BASE + UARTC_PSR_OFFSET, (unsigned char)1);

	/* Restore LCR */
	OUT8(STUARTC_BASE + UARTC_LCR_OFFSET, temp_word);
#endif // 0
	return 0;
}

int drv_uart_is_kbd_hit(void)
{
	return IN8(STUARTC_BASE + UARTC_LSR_OFFSET) & UARTC_LSR_RDR;
}

char drv_uart_get_char(void)
{
	while (!IN8(STUARTC_BASE + UARTC_LSR_OFFSET) & UARTC_LSR_RDR)
		;

	return IN8(STUARTC_BASE + UARTC_RBR_OFFSET);
}

void drv_uart_put_char(char ch)
{
	while (!(IN8(STUARTC_BASE + UARTC_LSR_OFFSET) & UARTC_LSR_THRE))
		;

	OUT8(STUARTC_BASE + UARTC_THR_OFFSET, ch);
}

int drv_uart_initialize(void)
{
	/* Clear everything */
	OUT8(STUARTC_BASE + UARTC_IER_OFFSET, 0x0);
	OUT8(STUARTC_BASE + UARTC_LCR_OFFSET, 0x0);

	/* Setup baud rate */
	drv_uart_set_baudrate(DEFAULT_BAUDRATE);

	/* Setup parity, data bits, and stop bits */
	OUT8(STUARTC_BASE + UARTC_LCR_OFFSET, (UARTC_LCR_PARITY_NONE |
				UARTC_LCR_BITS8 | UARTC_LCR_STOP1));

	/* Enable and reset FIFO, setup FIFO INT trigger threshold TX=9, RX=8 */
	OUT8(STUARTC_BASE + UARTC_FCR_OFFSET,
			(UARTC_FCR_FIFO_EN |
			 UARTC_FCR_RFIFO_RESET  | UARTC_FCR_TFIFO_RESET |
			 UARTC_FCR_TFIFO16_TRGL9 | UARTC_FCR_RFIFO16_TRGL8));

	/* Read registers to clear all pending bits */
	/*
	IN8(STUARTC_BASE + UARTC_LSR_OFFSET);
	IN8(STUARTC_BASE + UARTC_RBR_OFFSET);
	IN8(STUARTC_BASE + UARTC_IIR_OFFSET);
	IN8(STUARTC_BASE + UARTC_MSR_OFFSET);
	*/
	READ_CLR(STUARTC_BASE + UARTC_LSR_OFFSET);
	READ_CLR(STUARTC_BASE + UARTC_RBR_OFFSET);
	READ_CLR(STUARTC_BASE + UARTC_IIR_OFFSET);
	READ_CLR(STUARTC_BASE + UARTC_MSR_OFFSET);
	/* Enable UART and receive interrupts */
	OUT8(STUARTC_BASE + UARTC_IER_OFFSET, (UARTC_IER_RDR));

	/* Read registers to clear all pending bits again for lucky */
	/*
	IN8(STUARTC_BASE + UARTC_LSR_OFFSET);
	IN8(STUARTC_BASE + UARTC_RBR_OFFSET);
	IN8(STUARTC_BASE + UARTC_IIR_OFFSET);
	IN8(STUARTC_BASE + UARTC_MSR_OFFSET);
	*/
	READ_CLR(STUARTC_BASE + UARTC_LSR_OFFSET);
	READ_CLR(STUARTC_BASE + UARTC_RBR_OFFSET);
	READ_CLR(STUARTC_BASE + UARTC_IIR_OFFSET);
	READ_CLR(STUARTC_BASE + UARTC_MSR_OFFSET);
	return 0;
}
