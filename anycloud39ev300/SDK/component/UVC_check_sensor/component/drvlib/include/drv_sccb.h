/**@file drv_sccb.h
 * @brief sccb interface driver header file
 *
 * This file provides SCCB APIs: SCCB initialization, write to SCCB & read data from SCCB.
 * Copyright (C) 2004 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author Guanghua Zhang
 * @date 2004-09-21
 * @version 1.0
 * @note refer to AK chip technical manual.
 */

#ifndef __DRV_SCCB_H__
#define __DRV_SCCB_H__

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup SCCB_Interface SCCB group
 *  @ingroup Drv_Lib
 */

/*@{*/

/**
 * @brief select soft or hard with IIC 
 * @author Jiankui
 * @date 2016-09-09
 * @param[in] iic_config 0:TWI   1:IIC
 * @return void
 */
void sccb_set_soft_hard_flag(bool iic_config);

/**
 * @brief SCCB interface initialize function
 *
 * setup SCCB interface
 * @author Guanghua Zhang
 * @date 2004-09-20
 * @param[in] pin_scl the pin assigned to SCL
 * @param[in] pin_sda the pin assigned to SDA
 * @return void
 */
void sccb_init(unsigned long pin_scl, unsigned long pin_sda);

/**
 * @brief write data to SCCB device
 *
 * write size length data to daddr's raddr register
 * @author Guanghua Zhang
 * @date 2004-09-20
 * @param[in] daddr SCCB device address
 * @param[in] raddr register address
 * @param[in] data write data's point
 * @param[in] size write data's length
 * @return bool return write success or failed
 * @retval false operate failed
 * @retval true operate success
 */
bool sccb_write_data(unsigned char daddr, unsigned char raddr, unsigned char *data, unsigned long size);

/**
 * @brief write data to SCCB device
 *
 * write size length data to daddr's raddr register
 * @author Guanghua Zhang
 * @date 2004-09-20
 * @param[in] daddr SCCB device address
 * @param[in] raddr register address
 * @param[in] data write data's point
 * @param[in] size write data's length
 * @return bool return write success or failed
 * @retval false operate failed
 * @retval true operate success
 */
bool sccb_write_data3(unsigned char daddr, unsigned short raddr, unsigned char *data, unsigned long size);

/**
 * @brief write data to SCCB device
 *
 * write size length data to daddr's raddr register
 * @author Guanghua Zhang
 * @date 2004-09-20
 * @param[in] daddr SCCB device address
 * @param[in] raddr register address
 * @param[in] data write data's point
 * @param[in] size write data's length
 * @return bool return write success or failed
 * @retval false operate failed
 * @retval true operate success
 */
bool sccb_write_data4(unsigned char daddr, unsigned short raddr, unsigned short *data, unsigned long size);

/**
 * @brief read data from SCCB device function
 *
 * read data from daddr's raddr register
 * @author Guanghua Zhang
 * @date 2004-09-20
 * @param[in] daddr SCCB device address
 * @param[in] raddr register address
 * @return unsigned char
 * @retval read-back data
 */
unsigned char sccb_read_data(unsigned char daddr, unsigned char raddr);

/**
 * @brief read data from SCCB device function
 *
 * read data from daddr's raddr register
 * @author Guanghua Zhang
 * @date 2004-09-20
 * @param[in] daddr SCCB device address
 * @param[in] raddr register address
 * @param[out] data read output data store address
 * @param[in] size read data size, in bytes
 * @return bool return read success or failed
 * @retval false operate failed
 * @retval true operate success
 */
bool sccb_read_data2(unsigned char daddr, unsigned char raddr, unsigned char *data, unsigned long size);

/**
 * @brief read data from SCCB device function
 *
 * read data from daddr's raddr register
 * @author Guanghua Zhang
 * @date 2004-09-20
 * @param[in] daddr SCCB device address
 * @param[in] raddr register address
 * @param[out] data read output data store address
 * @param[in] size read data size, in bytes
 * @return bool return read success or failed
 * @retval false operate failed
 * @retval true operate success
 */
bool sccb_read_data3(unsigned char daddr, unsigned short raddr, unsigned char *data, unsigned long size);

/**
 * @brief read data from SCCB device function
 *
 * read data from daddr's raddr register
 * @author Guanghua Zhang
 * @date 2004-09-20
 * @param[in] daddr SCCB device address
 * @param[in] raddr register address
 * @param[out] data read output data store address
 * @param[in] size read data size, in bytes
 * @return bool return read success or failed
 * @retval false operate failed
 * @retval true operate success
 */
bool sccb_read_data4(unsigned char daddr, unsigned short raddr, unsigned short *data, unsigned long size);

/*@}*/

#ifdef __cplusplus
}
#endif

#endif    /* end of __ARCH_SCCB_H__ */

