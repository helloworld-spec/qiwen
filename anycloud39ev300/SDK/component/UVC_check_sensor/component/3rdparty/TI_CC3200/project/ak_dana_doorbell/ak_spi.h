
#ifndef __AK_SPI_H__
#define __AK_SPI_H__

#define SPI_BUFF_SIZE     1024

#define SPI_WRITE_DATA 0

#define SPI_GPIO_INT_FLG  (1<<0) 


#if SPI_WRITE_DATA

typedef void(*T_fspireadcb)(unsigned char *buf,unsigned int len);
extern void ak_spi_master_read_set_cb(T_fspireadcb setcallcb);
extern void ak_spi_master_write(unsigned char* buf);
#endif

extern void ak_spi_init(void);


#endif //#ifndef __AK_SPI_H__


