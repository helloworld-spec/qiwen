#include "command.h"
#include "ak_common.h"
#include "ak_drv_spi.h"

#define TR_BUFF_SIZE     1024
#define REPORT_BUFFER_SIZE 256

static unsigned char spi_tx_buff[TR_BUFF_SIZE];

static unsigned char spi_rx_buff[TR_BUFF_SIZE];

void cmd_spi_demo(int argc, char **args) 
{
	volatile unsigned long i,j,k=0;
	int len = 0, number;
	int tmp = 0;
	for(i = 0; i < argc; i++)
	{
		switch (i)
		{
			case 0:
				len = atoi(args[0]);
				break;
				
			default:
				break;
		
		}
			
	}

	if(len == 0)
	{
		len = TR_BUFF_SIZE;
	}

	number = len;
	ak_print_normal("spi_write =%d\r\n",len);
	ak_drv_spi_open(1 , TR_BUFF_SIZE);

	ak_print_normal("spi_write\r\n");
	j = 0;
	
	for(i = 0; i < TR_BUFF_SIZE; i++)  //TR_BUFF_SIZE
	{

		spi_rx_buff[i] = 0;

	}
	for(i = 0; i < len; i++)  //TR_BUFF_SIZE
	{

		spi_tx_buff[i] = j;
		j++;
		if(j%REPORT_BUFFER_SIZE == 0)
		{
			j = 0;
		}

	}
	ak_print_normal("==spi_write\r\n");
	i = 0;
	j = 0;
	while(len > 0)
	{
		tmp =  ak_drv_spi_write_read(spi_tx_buff, spi_rx_buff, len);
	
		ak_print_normal("===~===%d\r\n",tmp);	
		len -= tmp;
		//mini_delay(20);
		
	}

	for(i = 0; i < number; i++)
	{
		if(0 ==  i%16)
		ak_print_normal("\r\n");	
		
		ak_print_normal("%3d ",spi_rx_buff[i]);	

	}

	
	ak_print_normal("======ak_spi1_slave_write\r\n");	

}

static char *help[]={
	"spi module demo",
	""
};

static int cmd_spi_reg(void)
{
    cmd_register("spidemo", cmd_spi_demo, help);
    return 0;
}

cmd_module_init(cmd_spi_reg)



