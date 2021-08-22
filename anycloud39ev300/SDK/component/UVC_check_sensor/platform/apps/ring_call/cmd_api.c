/**
*cmd_api.c   process cmd read write 
*/


#include "ak_drv_uart1.h"
#include "ak_drv_spi.h"
#include "ak_common.h"
#include "kernel.h"


#include "cmd_api.h"


int cmd_uart_init()
{
	
 	if(ak_drv_uart1_open(115200, 0) < 0)
	{
		ak_print_error("ak_drv_uart2_open fail\n");
	}
	return 0;
}


int cmd_uart_deinit()
{
	
 	ak_drv_uart1_close();
	return 0;
}


/*
*   1bytes         1byte        2bytes
*| preamble | cmd_id  | cmd_len |  cmd_data | 
*
*read 
*/

int read_cmd_preamble(char *buf, unsigned short len)
{
    int ret;
    
    ret = ak_drv_uart1_read(buf, len, BLOCKED);
	if( ret > 0)
	{
		/*return first byte value*/
		return buf[0];
	}
	else
	{
		ak_print_error("read cmd preamble error %d\n", ret);
		return AK_ERROR;
	}
}

/*
*   1bytes         1byte        2bytes
*| preamble | cmd_id  | cmd_len |  cmd_data | 
*/
int read_cmd_len(char *buf, unsigned short len)
{
	unsigned short cmd_len;
	/*read cmd_id and cmd_len*/
	if(ak_drv_uart1_read(buf, len, UNBLOCK) >0)
	{
		cmd_len =  (*(unsigned short*)(buf + 1));
		return cmd_len;
	}
	else
	{
		ak_print_error("read cmd len error\n");
		return AK_ERROR;
	}
}

/*
*   1bytes         1byte        2bytes
*| preamble | cmd_id  | cmd_len |  cmd_data | 
*/

int read_cmd_data(char *buf, unsigned short len)
{
	/*read cmd data*/
	return ak_drv_uart1_read(buf, len, UNBLOCK); 
		
}

/*
*only read one cmd every time
*
*/
int read_one_cmd(char *buf, unsigned short len)
{
	unsigned char preamble;
	unsigned short cmd_len;
	int read_len;
    int get_len;
    do
	{
		preamble = read_cmd_preamble(buf, 1);/*keep read util read the cmd preamble*/
		ak_sleep_ms(2);
	}
	while(preamble != CMD_PREAMBLE && preamble!= 'F');
    
    if(preamble == 'F') {
        #if 0
        read_len = ak_drv_uart1_read(&buf[1], len-1, BLOCKED);
        if(read_len != len-1) {
            ak_print_error("uart read len %d\n",read_len);
            return AK_ERROR;
        } else {
            return read_len;
        }
       #else
        read_len = ak_drv_uart1_read(&buf[1], 3, BLOCKED);
        if(buf[1] == 'R' && buf[2] == 'A' && buf[3] == 'M') {
            get_len = len-4;
            do {
               read_len = ak_drv_uart1_read(&buf[len-get_len],get_len, BLOCKED); 
               if(read_len > 0) {
                   get_len -= read_len;
               } else {
                    ak_print_warning("uart1 read ret %d",read_len);
               }
               ak_sleep_ms(2);
            } while(get_len > 0);
            return len;
        } else {
            return -2;
        }
        #endif
    }
    else if(preamble == CMD_PREAMBLE) {
    	buf += 1;
        
    	cmd_len = read_cmd_len(buf, 3);
    	if( cmd_len > 0 )
    	{
    		if(cmd_len < len)
    		{
    			buf += 3;
    			read_len = read_cmd_data(buf, cmd_len);
    			if(read_len != cmd_len)
    			{
    				ak_print_error("cmd length should be %d, actually read %d\n", cmd_len, read_len);
    				return AK_ERROR;
    			}
    			return AK_OK;
    		}
    		else
    		{
    			ak_print_error("cmd data length %d is greater than buf length %d \n", cmd_len, len);
    			return AK_ERROR;
    		}
    	}
    	else 
    	{
    		ak_print_error("read cmd len error\n");
    		return AK_ERROR;
    	}
    }
		
}


/*
*send one cmd
* len should not greater 50 bytes because cc3200 reveive buf is 50bytes
*/
int send_one_cmd(char *buf, unsigned short len)
{
	return(ak_drv_uart1_write(buf, len));
}



/*spi data process*/

int data_spi_init()
{
	ak_drv_spi_open(1 , SPI_BUFF_SIZE);
	return 0;

}

int data_spi_deinit()
{
	return 0;

}


#define     SPI_DATA_SIZE       (SPI_BUFF_SIZE-4)
/*
* send video data to cc3200
* len should not greater than 1024 bytes , as spi driver send 1024 bytes every time
*/

int data_send(char *buf, unsigned short len)
{

#if 0  // 1 add spi pack check
    char bufTemp[SPI_BUFF_SIZE];
    unsigned int check_sum = 0;
    int i;
	unsigned short remain_len;
	int ret = 0;
    
	for(remain_len = len; remain_len > SPI_DATA_SIZE; remain_len -= SPI_DATA_SIZE)
	{
	    memcpy(&bufTemp[0],buf,SPI_DATA_SIZE);
        check_sum = 0;
        for(i = 0; i < SPI_DATA_SIZE;i++) {
            check_sum += bufTemp[i];
        }
        
        memcpy(&bufTemp[SPI_DATA_SIZE],&check_sum,4);
        ret += ak_drv_spi_write(bufTemp, SPI_BUFF_SIZE);
        ret -= 4;
		buf += SPI_DATA_SIZE;

        //ak_print_normal("check %d",check_sum);
	}
	
    memcpy(&bufTemp[0],buf,SPI_DATA_SIZE);
    check_sum = 0;
    for(i = 0; i < SPI_DATA_SIZE;i++) {
        check_sum += bufTemp[i];
    }
    
    memcpy(&bufTemp[SPI_DATA_SIZE],&check_sum,4);
    ret += ak_drv_spi_write(bufTemp, SPI_BUFF_SIZE);
    ret -= (SPI_BUFF_SIZE-remain_len);
    

#else
	unsigned short remain_len;
	int ret = 0;
	for(remain_len = len; remain_len > SPI_BUFF_SIZE; remain_len -= SPI_BUFF_SIZE)
	{
		ret += ak_drv_spi_write(buf, SPI_BUFF_SIZE);
		buf += SPI_BUFF_SIZE;
	}
	
	ret += ak_drv_spi_write(buf, remain_len);
#endif

	return ret;
}


