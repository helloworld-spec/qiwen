
/**
 * @file drv_i2c.h
 * @brief I2C interface driver header file
 *
 * This file provides I2C APIs: I2C initialization, write to I2C & read data from I2C.
 * Copyright (C) 2004 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author Guanghua Zhang
 * @date 2004-09-21
 * @version 1.0
 * @note refer to ANYKA chip technical manual.
 */

#ifndef __DRV_I2C_H__
#define __DRV_I2C_H__

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup I2C_Interface I2C group
 *  @ingroup Drv_Lib
 */

/*@{*/
/**
 * @brief I2C interface initialization function
 *
 * setup I2C interface
 * @author Guanghua Zhang
 * @date 2004-09-21
 * @param[in] pin_scl the pin assigned to SCL
 * @param[in] pin_sda the pin assigned to SDA
 * @return void
 */
void i2c_init(unsigned long pin_scl, unsigned long pin_sda);

/**
 * @brief I2C interface release function
 *
 * setup I2C interface
 * @author Guanghua Zhang
 * @date 2004-09-21
 * @param[in] pin_scl the pin assigned to SCL
 * @param[in] pin_sda the pin assigned to SDA
 * @return void
 */
void i2c_release(unsigned long pin_scl, unsigned long pin_sda);

/**
 * @brief set I2C device cycle delay time , the I2C transmit cycle and freq will be changed by setting this. 
 *
 * It should be call after i2c_init if want to reset the I2C transmit freq.It should not be call in transmiting data.
 * The default delay time value is 90, because we use cpu loop to delay, the cycle and freq will be relate to the cpu freq too.
 * We have test that, in default delay time and the cpu run between 15M and 84M, the I2C transmit ok.  
 * if the data transmit ok, we suggest not reset the delay time.
 * But if data transmit error, and the I2C freq is too high or too low. can use this function to reset the freq of I2C.
 * @author Zhuobin Li
 * @date 2006-11-08
 * @param[in] delay set delay time in every cycle by this value, this delay value is not exactitude to delay time, but can change the I2C cycle and freq by it 
 * @return unsigned long
 * @retval the delay value which is set 
 */
unsigned long i2c_set_cycle_delay(unsigned long delay);

/**
 * @brief write data to I2C device
 *
 * write size length data to daddr's raddr register, raddr and data is byte width
 * @author Guanghua Zhang
 * @date 2004-09-21
 * @param[in] daddr I2C device address
 * @param[in] raddr register address
 * @param[in] *data write data's pointer
 * @param[in] size write data's length
 * @return bool return operation successful or failed
 * @retval false operation failed
 * @retval true operation successful
 */
bool i2c_write_data(unsigned char daddr, unsigned char raddr, unsigned char *data, unsigned long size);

/**
 * @brief write data to I2C device
 *
 * write size length data to daddr's raddr register, raddr is word width, data is byte width
 * @author Guanghua Zhang
 * @date 2004-09-21
 * @param[in] daddr I2C device address
 * @param[in] raddr register address
 * @param[in] *data write data's pointer
 * @param[in] size write data's length
 * @return bool return operation successful or failed
 * @retval false operation failed
 * @retval true operation successful
 */
bool i2c_write_data2(unsigned char daddr, unsigned short raddr, unsigned char *data, unsigned long size);

/**
 * @brief write data to I2C device
 *
 * write size length data to daddr's raddr register, raddr and data is word width
 * @author Guanghua Zhang
 * @date 2004-09-21
 * @param[in] daddr I2C device address
 * @param[in] raddr register address
 * @param[in] *data write data's pointer
 * @param[in] size write data's length
 * @return bool return operation successful or failed
 * @retval false operation failed
 * @retval true operation successful
 */
bool i2c_write_data3(unsigned char daddr, unsigned short raddr, unsigned short *data, unsigned long size);

/**
 * @brief write data to I2C device
 *
 * write size length data to daddr's raddr register, raddr is not required, data is byte width
 * @author guoshaofeng
 * @date 2008-03-10
 * @param[in] daddr I2C device address
 * @param[in] *data write data's pointer
 * @param[in] size write data's length
 * @return bool return operation successful or failed
 * @retval false operation failed
 * @retval true operation successful
 */
bool i2c_write_data4(unsigned char daddr, unsigned char *data, unsigned long size);

/**
 * @brief read data from I2C device function
 *
 * read data from daddr's raddr register, raddr and data is byte width
 * @author Guanghua Zhang
 * @date 2004-09-21
 * @param[in] daddr I2C device address
 * @param[in] raddr register address
 * @param[out] *data read output data store address
 * @param[in] size read data size
 * @return bool return operation successful or failed
 * @retval false operation failed
 * @retval true operation successful
 */
bool i2c_read_data(unsigned char daddr, unsigned char raddr, unsigned char *data, unsigned long size);

/**
 * @brief read data from I2C device function
 *
 * read data from daddr's raddr register, raddr is word width, data is byte width
 * @author Guanghua Zhang
 * @date 2004-09-21
 * @param[in] daddr I2C device address
 * @param[in] raddr register address
 * @param[out] *data read output data store address
 * @param[in] size read data size
 * @return bool return operation successful or failed
 * @retval false operation failed
 * @retval true operation successful
 */
bool i2c_read_data2(unsigned char daddr, unsigned short raddr, unsigned char *data, unsigned long size);

/**
 * @brief read data from I2C device function
 *
 * read data from daddr's raddr register, raddr and data is word width
 * @author Guanghua Zhang
 * @date 2004-09-21
 * @param[in] daddr I2C device address
 * @param[in] raddr register address
 * @param[out] data read output data store address
 * @param[in] size read data size
 * @return bool return operation successful or failed
 * @retval false operation failed
 * @retval true operation successful
 */
bool i2c_read_data3(unsigned char daddr, unsigned short raddr, unsigned short *data, unsigned long size);

/**
 * @brief read data from I2C device function
 *
 * read data from daddr's raddr register, raddr is not required, data is byte width
 * @author guoshaofeng
 * @date 2008-03-10
 * @param[in] daddr I2C device address
 * @param[out] data read output data store address
 * @param[in] size read data size
 * @return bool return operation successful or failed
 * @retval false operation failed
 * @retval true operation successful
 */
bool i2c_read_data4(unsigned char daddr, unsigned char *data, unsigned long size);


/**
 * @brief seed cmd and get data from I2C device function
 *
 * this function is for ts ic cp2007
 * @author luheshan
 * @date 2012-10-19
 * @param[in] daddr I2C device address
 * @param[in] raddr register address
 * @param[in] Resize read data size
 * @param[in/out] data read output data store address
 * @return bool return operation successful or failed
 * @retval false operation failed
 * @retval true operation successful
 */
bool i2c_write_cmd_read_data(unsigned char daddr, unsigned char raddr, unsigned char *data, unsigned long ReSize);

/*@}*/
#ifdef __cplusplus
}
#endif

#endif  /* __DRV_I2C_H__ */

