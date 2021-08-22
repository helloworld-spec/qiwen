#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "internal_error.h"
#include "ak_common.h"
#include "ak_drv_i2c.h"

#define I2C_SLAVE	0x0703	/* Use this slave address */
#define I2C_TENBIT	0x0704	/* 0 for 7 bit addrs, != 0 for 10 bit */
#define I2C_RDWR	0x0707	/* Combined R/W transfer (one STOP only) */

#define I2C_DEV_PATH "/dev/i2c-0"
static const char drv_i2c_version[] = "plat_drv_key V1.0.00";

/* 
 * ak_drv_i2c_open - i2c device open
 * device_no[IN]: pwm device minor-number
 * return: pointer to i2c handle on success, NULL on failed.
 */
void* ak_drv_i2c_open(int device_no)
{
	int fd = -1;
	int ret;
	struct ak_i2c_dev* dev;
	
	//open i2c bus char devices "/dev/i2c-0"
	fd = open(I2C_DEV_PATH, O_RDWR);
	if (fd < 0) {
		set_error_no(ERROR_TYPE_DEV_OPEN_FAILED);
		ak_print_error("Open i2c dev fail\n");
		return NULL;
	}

	//set i2c not 10 bit
	ret = ioctl(fd, I2C_TENBIT, 0);
	if (ret < 0) {
		set_error_no(ERROR_TYPE_DEV_OPEN_FAILED);
		ak_print_error("I2C_TENBIT i2c dev fail\n");
		goto err;
	}

#if 0

	//set i2c device addr
	ret = ioctl(fd, I2C_SLAVE, device_no);
	if (ret < 0) {
		set_error_no(ERROR_TYPE_DEV_OPEN_FAILED);
		ak_print_error("I2C_SLAVE i2c dev fail\n");
		goto err;
	}

	buf[0] = (device_no>>8);
	buf[1] = device_no;
	//dected if there is a devices
	ret = write(fd, buf, 2);
	if (ret < 0) {
		set_error_no(ERROR_TYPE_DEV_OPEN_FAILED);
		ak_print_error("i2c dev : 0x%02x is not exit\n", device_no);
		goto err;
	}


	printf("read...\n");
	ret = read(fd, buf, 2);
	if (ret < 0) {
		set_error_no(ERROR_TYPE_DEV_OPEN_FAILED);
		ak_print_error("i2c dev : 0x%02x is not exit\n", device_no);
		goto err;
	}
	printf("end read... buf[0]=0x%02x, buf[1]=0x%02x \n", buf[0], buf[1]);
	
#endif

	//alloc mem for dev handle
	dev = (struct ak_i2c_dev*)malloc(sizeof(struct ak_i2c_dev)); 
	dev->fd = fd;
	dev->device_no = device_no;
	
	return (void *)dev;
	
err:
	close(fd);
	return NULL;
	
}

/*
 * ak_drv_i2c_read - read i2c data
 * handle[IN]: return by ak_drv_i2c_open();
 * reg_addr[IN]: i2c register address
 * data[OUT]: pointer to buffer which use to store result
 * len[IN]: indicate result buffer len
 * return: 0 on success, -1 failed.
 */
int ak_drv_i2c_read(void *handle, const unsigned short reg_addr,
	unsigned char *data, unsigned int len)
{
	struct ak_i2c_dev* dev = (struct ak_i2c_dev*)handle;
	struct i2c_rdwr_ioctl_data i2c_data;
	unsigned char tmp[2] = {0};
	int ret;
	
	/* allocate communicate node */
	i2c_data.msgs  = (struct i2c_msg *)malloc(2*sizeof(struct i2c_msg));
	if(!i2c_data.msgs){	
		ak_print_error_ex("i2c_data.msgs alloc fail!\n");
		return -1;
	}

	/* assignment for node */
	i2c_data.nmsgs = 2;
	i2c_data.msgs[0].len = 2;
	i2c_data.msgs[0].addr = dev->device_no;
	i2c_data.msgs[0].flags = 0;

	tmp[0] = reg_addr>>8;	
	tmp[1] = reg_addr & 0xff;
	i2c_data.msgs[0].buf = tmp;
	
	i2c_data.msgs[1].len = len;
	i2c_data.msgs[1].addr = dev->device_no;
	i2c_data.msgs[1].flags = 1;
	i2c_data.msgs[1].buf = data;
	
	ret = ioctl(dev->fd, I2C_RDWR, &i2c_data);
	if(ret < -1) {
		set_error_no(ERROR_TYPE_DEV_OPEN_FAILED);
		ak_print_error("i2c read reg : 0x%x fail!\n", reg_addr);	
		free(i2c_data.msgs);
		return ret;
	}
	
	free(i2c_data.msgs);
	return len;
	
}

/*
 * ak_drv_i2c_write - write i2c data
 * handle[IN]: return by ak_drv_i2c_open();
 * reg_addr[IN]: i2c register address
 * data[IN]: pointer to buffer which data will be write
 * len[IN]: indicate buffer len
 * return: 0 on success, -1 failed.
 */
int ak_drv_i2c_write(void *handle, const unsigned short reg_addr, 
   const unsigned char *data, unsigned int len)
{
	int ret;
	struct ak_i2c_dev* dev = (struct ak_i2c_dev*)handle;
	struct i2c_rdwr_ioctl_data i2c_data;
	
	/* arguments check */
	if ((NULL == handle || NULL == data)) {
		set_error_no(ERROR_TYPE_POINTER_NULL);
		ak_print_error_ex("para err\n");
		return -1;
	}

	/* allocate communicate node */
	i2c_data.msgs  = (struct i2c_msg *)malloc(sizeof(struct i2c_msg));
	if(!i2c_data.msgs){	
		ak_print_error_ex("i2c_data.msgs alloc fail!\n");
		return -1;
	}
	
	/* assignment for node */
	i2c_data.nmsgs = 1;
	i2c_data.msgs[0].addr = dev->device_no;
	i2c_data.msgs[0].flags = 0;
	i2c_data.msgs[0].buf = (unsigned char *)malloc(len + sizeof(reg_addr));
	if(!i2c_data.msgs[0].buf){
		ak_print_error_ex("i2c_data.msgs[0].buf alloc fail!\n");
		free(i2c_data.msgs);
		return -1;
	}
	
	i2c_data.msgs[0].len = sizeof(reg_addr) + len;
	i2c_data.msgs[0].buf[0] = reg_addr>>8;
	i2c_data.msgs[0].buf[1] = reg_addr&0xff;
	memcpy(&(i2c_data.msgs[0].buf[2]), data, len);
	
	ret = ioctl(dev->fd, I2C_RDWR, &i2c_data);
	if(ret < -1) {
		set_error_no(ERROR_TYPE_DEV_OPEN_FAILED);
		ak_print_error("i2c write reg : 0x%x fail!\n", reg_addr);	
		free(i2c_data.msgs[0].buf);
		free(i2c_data.msgs);
		
		return ret;
	}

	free(i2c_data.msgs[0].buf);
	free(i2c_data.msgs);
	return len;

}

/*
 * ak_drv_i2c_close - close i2c operate, close device
 * handle[IN]: return by ak_drv_i2c_open();
 * return: always return 0; 
 */
int ak_drv_i2c_close(void *handle)
{
	struct ak_i2c_dev *dev = (struct ak_i2c_dev*)handle;
	close(dev->fd);
	free(dev);

	return AK_SUCCESS;
}
