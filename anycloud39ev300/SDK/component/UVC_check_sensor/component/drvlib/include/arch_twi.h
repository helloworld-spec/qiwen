
/**
 * @file arch_twi.h
 * @brief TWI interface driver header file
 *
 * This file provides TWI APIs: TWI initialization, write to TWI & read data from TWI.
 * Copyright (C) 2004 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author Jiankui
 * @date 2016-09-12
 * @version 1.0
 * @note refer to ANYKA chip technical manual.
 */

#ifndef __ARCH_TWI_H__
#define __ARCH_TWI_H__

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup TWI_Interface TWI group
 *  @ingroup Drv_Lib
 */

/*@{*/
/**
 * @brief TWI interface initialization function
 *
 * setup TWI interface
 * @author Jiankui
 * @date 2016-09-12
 * @return void
 */
void twi_init(void);

/**
 * @brief TWI interface release function
 *
 * setup TWI interface
 * @author Jiankui
 * @date 2016-09-12
 * @return void
 */
void twi_release(void);

/**
 * @brief write data to TWI device
 *
 * write size length data to daddr's raddr register, raddr and data is byte width
 * @author Jiankui
 * @date 2016-09-12
 * @param[in] daddr TWI device address
 * @param[in] raddr register address
 * @param[in] *data write data's pointer
 * @param[in] size write data's length
 * @return bool return operation successful or failed
 * @retval false operation failed
 * @retval true operation successful
 */
bool twi_write_data(unsigned char daddr, unsigned char raddr, unsigned char *data, unsigned long size);

/**
 * @brief write data to TWI device
 *
 * write size length data to daddr's raddr register, raddr is word width, data is byte width
 * @author Jiankui
 * @date 2016-09-12
 * @param[in] daddr TWI device address
 * @param[in] raddr register address
 * @param[in] *data write data's pointer
 * @param[in] size write data's length
 * @return bool return operation successful or failed
 * @retval false operation failed
 * @retval true operation successful
 */
bool twi_write_data2(unsigned char daddr, unsigned short raddr, unsigned char *data, unsigned long size);

/**
 * @brief write data to TWI device
 *
 * write size length data to daddr's raddr register, raddr and data is word width
 * @author Jiankui
 * @date 2016-09-12
 * @param[in] daddr TWI device address
 * @param[in] raddr register address
 * @param[in] *data write data's pointer
 * @param[in] size write data's length
 * @return bool return operation successful or failed
 * @retval false operation failed
 * @retval true operation successful
 */
bool twi_write_data3(unsigned char daddr, unsigned short raddr, unsigned short *data, unsigned long size);

/**
 * @brief write data to TWI device
 *
 * write size length data to daddr's raddr register, raddr is not required, data is byte width
 * @author Jiankui
 * @date 2016-09-12
 * @param[in] daddr TWI device address
 * @param[in] *data write data's pointer
 * @param[in] size write data's length
 * @return bool return operation successful or failed
 * @retval false operation failed
 * @retval true operation successful
 */
bool twi_write_data4(unsigned char daddr, unsigned char *data, unsigned long size);

/**
 * @brief read data from TWI device function
 *
 * read data from daddr's raddr register, raddr and data is byte width
 * @author Jiankui
 * @date 2016-09-12
 * @param[in] daddr TWI device address
 * @param[in] raddr register address
 * @param[out] *data read output data store address
 * @param[in] size read data size
 * @return bool return operation successful or failed
 * @retval false operation failed
 * @retval true operation successful
 */
bool twi_read_data(unsigned char daddr, unsigned char raddr, unsigned char *data, unsigned long size);

/**
 * @brief read data from TWI device function
 *
 * read data from daddr's raddr register, raddr is word width, data is byte width
 * @author Jiankui
 * @date 2016-09-12
 * @param[in] daddr TWI device address
 * @param[in] raddr register address
 * @param[out] *data read output data store address
 * @param[in] size read data size
 * @return bool return operation successful or failed
 * @retval false operation failed
 * @retval true operation successful
 */
bool twi_read_data2(unsigned char daddr, unsigned short raddr, unsigned char *data, unsigned long size);

/**
 * @brief read data from TWI device function
 *
 * read data from daddr's raddr register, raddr and data is word width
 * @author Jiankui
 * @date 2016-09-12
 * @param[in] daddr TWI device address
 * @param[in] raddr register address
 * @param[out] data read output data store address
 * @param[in] size read data size
 * @return bool return operation successful or failed
 * @retval false operation failed
 * @retval true operation successful
 */
bool twi_read_data3(unsigned char daddr, unsigned short raddr, unsigned short *data, unsigned long size);

/**
 * @brief read data from TWI device function
 *
 * read data from daddr's raddr register, raddr is not required, data is byte width
 * @author Jiankui
 * @date 2016-09-12
 * @param[in] daddr TWI device address
 * @param[out] data read output data store address
 * @param[in] size read data size
 * @return bool return operation successful or failed
 * @retval false operation failed
 * @retval true operation successful
 */
bool twi_read_data4(unsigned char daddr, unsigned char *data, unsigned long size);

/*@}*/
#ifdef __cplusplus
}
#endif

#endif  /* __ARCH_TWI_H__ */


