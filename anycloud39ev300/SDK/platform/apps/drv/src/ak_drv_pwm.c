#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "internal_error.h"
#include "ak_common.h"
#include "ak_drv_pwm.h"

#define AK_PWM_SELET_DEV	0x0600	
#define AK_PWM_SET			0x0601	

#define PWM_DEV_PATH "/dev/ak_pwm"
static const char drv_pwm_version[] = "plat_drv_key V1.0.00";

struct ak_duty{
   int numerator;
   int denominator;
};

struct ak_pwm_cycle {
	int frq;
	struct ak_duty duty;
};

/* 
 * ak_drv_pwm_open - open pwm device
 * device_no[IN]: pwm device minor-number
 * return: pointer to pwm handle on success, NULL on failed.
 */
void* ak_drv_pwm_open(int device_no)
{
	int fd = -1;
	int ret=0;
	
	//open i2c bus char devices "/dev/ak_pwm"
	fd = open(PWM_DEV_PATH, O_RDWR);
	if (fd < 0) {
		set_error_no(ERROR_TYPE_DEV_OPEN_FAILED);
		ak_print_error("Open pwm dev fail\n");
		return NULL;
	}

	//pwm 1-5 select
	ret = ioctl(fd, AK_PWM_SELET_DEV, device_no-1);
	if (ret < 0) {
		set_error_no(ERROR_TYPE_DEV_CTRL_FAILED);
		ak_print_error("request pwm dev fail\n");
        close(fd);
		return NULL;
	}

	return (void*)fd;
}

/* 
 * ak_drv_pwm_set - set pwd working param
 * handle[IN]: return by ak_drv_pwm_open();
 * clock[IN]: pwm clock
 * duty_cycle[IN]: pwm duty cycle
 * return: 0 on success, -1 failed.
 */
int ak_drv_pwm_set(void *handle, unsigned int clock, unsigned int duty_num, unsigned int duty_den)
{
	int fd = (int)handle;
	int ret;
	struct ak_pwm_cycle pwm_data;
	/* arguments check */
	if (NULL == handle) {
		set_error_no(ERROR_TYPE_POINTER_NULL);
		ak_print_error_ex("para err\n");
		return -1;
	}

	pwm_data.frq = clock;
	pwm_data.duty.numerator = duty_num;
    pwm_data.duty.denominator = duty_den;

	/* set to the pwm driver */
	ret = ioctl(fd, AK_PWM_SET, &pwm_data);
	if (ret) {
		set_error_no(ERROR_TYPE_POINTER_NULL);
		ak_print_error_ex("para err\n");
        close(fd);
		return -1;
	}
	
	return ret;
}

/* 
 * ak_drv_pwm_close - close pwm
 * handle[IN]: return by ak_drv_pwm_open();
 * return: 0 on success, -1 failed.
 */
int ak_drv_pwm_close(void *handle)
{
	int fd = (int)handle;
	
	if (NULL == handle) {
		set_error_no(ERROR_TYPE_POINTER_NULL);
		ak_print_error_ex("para err\n");
		return -1;
	}
	close(fd);

	return AK_SUCCESS;
}
