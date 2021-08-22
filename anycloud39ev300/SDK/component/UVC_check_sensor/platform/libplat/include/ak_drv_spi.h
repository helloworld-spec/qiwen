/**
 * @file 
 * @brief: 
 *
 * This file describe driver of spi.
 * Copyright (C) 2010 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  
 * @date    2016-12-04
 * @version 1.0
 */

#ifndef __AK_SPI_H_
#define __AK_SPI_H_

#ifdef __cplusplus
extern "C" {
#endif


 /**
 * @brief
 * @author 
 * @date 2016-12-04
 * @param 
 * @param 
 * @return bool
 * @version 
 */
int ak_drv_spi_open(unsigned long buff_nb, unsigned long size);


 /**
 * @brief
 * @author 
 * @date 2016-12-04
 * @param 
 * @param 
 * @return bool
 * @version 
 */
int ak_drv_spi_write_read(unsigned char *wr_data, unsigned char *rd_data, int len);

 /**
 * @brief
 * @author 
 * @date 2016-12-04
 * @param 
 * @param 
 * @return bool
 * @version 
 */
int ak_drv_spi_write(unsigned char *buf,unsigned long len);

 /**
 * @brief
 * @author 
 * @date 2016-12-04
 * @param 
 * @param 
 * @return bool
 * @version 
 */
int ak_drv_spi_read(unsigned char *buf,unsigned long len,long ms);
 
 /**
 * @brief
 * @author 
 * @date 2016-12-04
 * @param 
 * @param 
 * @return bool
 * @version 
 */
int ak_drv_spi_close(void);


/*@}*/
#ifdef __cplusplus
}
#endif

#endif //#ifndef __AK_SPI_H_






