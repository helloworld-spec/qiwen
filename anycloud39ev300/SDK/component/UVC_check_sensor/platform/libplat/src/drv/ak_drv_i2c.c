#include "dev_drv.h"
#include "command.h"

#include "ak_drv_i2c.h"
#include "ak_common.h"

typedef struct 
{
	int fd;
	unsigned char device_addr; 
	unsigned char reg_type;
	unsigned char use_flag;
}T_HAL_I2C;

#define I2C_DEV_NUM 10
static T_HAL_I2C hal_i2c_handle_list[I2C_DEV_NUM];

/**
 * @brief I2C port device open
 * @author Jian_Kui
 * @date 2016-11-23
 * @param [in] device_no:IIC device address
 * @return viod *
 * @retval I2C device handle
  */
void* ak_drv_i2c_open(unsigned char device_no, unsigned reg_type)
{


	#if 0	
	//i2c_handle = malloc(sizeof(T_HAL_I2C));
	
	
	if(NULL == i2c_handle)
	{
		return NULL;
	}
	#else
	
	unsigned  char i;
	static char open_flg = 0;
	static T_HAL_I2C *i2c_handle = NULL;
	if(0 == open_flg)//just for case...
	{
		open_flg = 1;
		for(i = 0; i < I2C_DEV_NUM; i++ )
		{
			hal_i2c_handle_list[i].use_flag = 0;	
		}	
	}
	
	for(i = 0; i < I2C_DEV_NUM; i++ )		
	{		
		if(0 == hal_i2c_handle_list[i].use_flag)
		{
			i2c_handle = (T_HAL_I2C *)&hal_i2c_handle_list[i];
			 hal_i2c_handle_list[i].use_flag = 1;	
			 break;
		}				
	}

	if(i == I2C_DEV_NUM)
	{
		ak_print_error("open i2c fail\n");
		return NULL;
	}
	
	#endif
	i2c_handle->device_addr = device_no;
	i2c_handle->reg_type = reg_type;
	i2c_handle->fd = dev_open(DEV_I2C);
	if(-1 == i2c_handle->fd)
	{
		//free(i2c_handle);
		i2c_handle->use_flag = 0;
		ak_print_error("dev open i2c fail\n");
		i2c_handle = NULL;
	}
	return i2c_handle;
}

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
int ak_drv_i2c_read(void* handle, const unsigned short reg_addr, unsigned char *data, unsigned int len)
{
	int retval = -1;
	unsigned long cmd_data[2];
	unsigned char *p_data;
	unsigned long data_addr;
	T_HAL_I2C *i2c_handle = NULL;
	if(NULL == handle)
	{
		return -1;
	}
	i2c_handle = (T_HAL_I2C *)handle;

	if(0 == i2c_handle->use_flag)
	{
		return -1;
	}

	data_addr = data;
	p_data = (unsigned char *)cmd_data;
	p_data[0] = i2c_handle->device_addr;

	p_data[1] = len;

	p_data[2] = reg_addr;
	p_data[3] = reg_addr>>8;
	
	p_data[4] = data_addr;
	p_data[5] = data_addr>>8;
	p_data[6] = data_addr>>16;
	p_data[7] = data_addr>>24;
	if(i2c_handle->reg_type)
		retval = dev_ioctl(i2c_handle->fd,IO_I2C_WORD_READ,cmd_data);
	else
		retval = dev_ioctl(i2c_handle->fd,IO_I2C_BYTE_READ,cmd_data);
	return retval;
	
}

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
int ak_drv_i2c_write(void* handle, const unsigned short reg_addr, const unsigned char *data, unsigned int len)
{

	int retval = -1;
	unsigned long cmd_data[2];
	unsigned char *p_data;
	unsigned long data_addr;
	T_HAL_I2C *i2c_handle = NULL;
	if(NULL == handle)
	{
		return -1;
	}
	i2c_handle = (T_HAL_I2C *)handle;
	
	if(0 == i2c_handle->use_flag)
	{
		return -1;
	}

	data_addr = data;
	p_data = (unsigned char *)cmd_data;
	p_data[0] = i2c_handle->device_addr;

	p_data[1] = len;

	p_data[2] = reg_addr;
	p_data[3] = reg_addr>>8;
	
	p_data[4] = data_addr;
	p_data[5] = data_addr>>8;
	p_data[6] = data_addr>>16;
	p_data[7] = data_addr>>24;

	if(i2c_handle->reg_type)
		retval = dev_ioctl(i2c_handle->fd,IO_I2C_WORD_WRITE,cmd_data);
	else
		retval = dev_ioctl(i2c_handle->fd,IO_I2C_BYTE_WRITE,cmd_data);
	return retval;

}

/**
 * @brief I2C port device close
 * @author Jian_Kui
 * @date 2016-11-23
 * @param [in] handle: I2C device handle
 * @return int
 * @retval -1 operation failed
 * @retval 0 operation successful
  */
int ak_drv_i2c_close(void* handle)
{
	int retval = -1;
	T_HAL_I2C *i2c_handle = NULL;
	if(NULL == handle)
	{
		return -1;
	}
	i2c_handle = (T_HAL_I2C *)handle;

	if(0 == i2c_handle->use_flag)
	{
		return -1;
	}
	i2c_handle->use_flag = 0;
	retval = dev_close(i2c_handle->fd);
	
	return retval;
	
}

