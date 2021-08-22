#include <asm/arch/anyka_cpu.h>


#define	UART0_CONF_REG			0x20130000
#define	UART0_STATUS_REG		0x20130004
#define	UART0_COUNT_REG			0x20130008
#define	UART0_THRESHOLD_REG		0x2013000C

//baurate: 38400[0~15], rts& is 1[20],  uart enable[21], not receive timeout[23], 
//not even odd check[26], little endian[27], tx rx clear state[28~29]
#define	UART_CONF_DEFAULT_VAL				0x30300619
#define	UART_BAUD_REATE_115200				0x208
#define	UART_BAUD_REATE_38400				0x619
#define	UART_BAUD_REATE_38400_ASPEN2		0x64d   // Aspen boot up sequency is 62MHZ
#define	UART_ENABLE							(0x1<<21)
#define	UART_DIV_ADJ_ENA					(0x1<<22)
#define	UART_TIMEOUT_ENA					(0x1<<23)
#define	UART_CHECK_EVEN_ODD_ENA	   		    (0x1<<26)
#define	UART_BIG_ENDIAN_ENA	   	   		    (0x1<<27)
#define	UART_CLEAR_TX_STAT	   	   		    (0x1<<28)
#define	UART_CLEAR_RX_STAT	   	   		    (0x1<<29)

#define	UART_THRESHOLD_RX_INT				(0x1<<29)
#define	UART_THRESHOLD_RX_INT_ASPEN2		(0x1<<30)

#define UART_ALWAYS_WAIT_ENABLE				(1)
#define UART_ALWAYS_WAIT_DISABLE			(0)
#define	UART_TIMEOUT_STAT					(0x1<<2)



void test_print(char *s);
void test_init_serial(void)
{   
	unsigned long baudrate;
	unsigned long asic_freq;

	asic_freq = get_asic_freq();
	baudrate = asic_freq/115200 - 1;

	l2_init();

#if 1
	*(volatile unsigned int*)(0x0800001c) &= ~(0x1 << 7);
	*(volatile unsigned int*)(0x08000074) |= (0x3 << 14);
	*(volatile unsigned int*)(0x08000080) |= (0x3 << 19);

	*(volatile unsigned int *)UART0_CONF_REG = (UART_CLEAR_RX_STAT|UART_CLEAR_TX_STAT
					|UART_ENABLE|baudrate);
	
//	console_init(baudrate);
	//test_print("aimer39 test serial init.\n");
#endif
}

void test_putc(char c)
{
	*(volatile unsigned int*)(0x20130018) = ((0x1F << 16)|(0x1 << 0));

	*(volatile unsigned int*)(0x2014008c) |= (0x1<<16); //Clear the uart tx buf
	*(volatile unsigned int*)(0x48001000) = (unsigned int)c; 
	*(volatile unsigned int*)(0x48001000 + 60) = (unsigned int)'\0'; 
	*(volatile unsigned int*)(UART0_CONF_REG) |= (0x1 << 28);
	*(volatile unsigned int*)(UART0_STATUS_REG) |= ((0x1 << 4)|(0x1 << 16));
	
	while((*(volatile unsigned int*)(UART0_STATUS_REG) & (0x1 << 19)) == 0)
		;
}

void test_print(char *s)
{
	char c;
	while((c = *(s++)) != '\0')
		test_putc(c);
}

