#ifndef __AK_DRV_I2C_H__
#define __AK_DRV_I2C_H__

/**
 * @brief I2C port device open
 * @author Jian_Kui
 * @date 2016-11-23
 * @param [in] device_no:IIC device address
 * @return viod *
 * @retval I2C device handle
  */
void* ak_drv_i2c_open(unsigned char device_no, unsigned reg_type);

/**
 * @brief I2C read data function
 * @author Jian_Kui
 * @date 2016-11-23
 * @param [in] handle: I2C device handle
 * @param[in] reg_raddr register address
 * @param[out] *data read output data store address
 * @param[in] size read data size
 * @return int
 * @retval -1 operation failed
 * @retval 0 operation successful
 */
int ak_drv_i2c_read(void* handle, const unsigned short reg_addr, unsigned char *data, unsigned int len);

/**
 * @brief I2C write data function
 * @author Jian_Kui
 * @date 2016-11-23
 * @param [in] handle: I2C device handle
 * @param[in] reg_raddr register address
 * @param[in] *data write data's pointer
 * @param[in] size write data size
 * @return int
 * @retval -1 operation failed
 * @retval 0 operation successful
 */
int ak_drv_i2c_write(void* handle, const unsigned short reg_addr, const unsigned char *data, unsigned int len);

/**
 * @brief I2C port device close
 * @author Jian_Kui
 * @date 2016-11-23
 * @param [in] handle: I2C device handle
 * @return int
 * @retval -1 operation failed
 * @retval 0 operation successful
  */
int ak_drv_i2c_close(void* handle);

#endif


