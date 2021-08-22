#include "ak_common.h"
#include "ak_drv_ircut.h"
#include "drv_ircut.h"

#include "dev_drv.h"


typedef struct 
{
	int fd;
	
}T_HAL_CAMERA;

static  T_HAL_CAMERA m_hal_camera = {0};


/** 
 * ak_drv_ir_init: init ircut ,get ircut control mode 
 * return: 0 - success; otherwise -1; 
 */
int ak_drv_ir_init(void)
{
    int status;
    int ret;

	ircut_open();
	
	m_hal_camera.fd = dev_open(DEV_CAMERA);
	
    ret = dev_ioctl(m_hal_camera.fd, IO_CAMERA_IRFEED_GPIO_OPEN, &status);
    if (0 !=ret)
    {
		ak_print_error_ex("dev_ioctl IO_CAMERA_IRFEED_GPIO_OPEN fail!\n");
		return -1;
    }
	return 0;
}

/** 
 * ak_drv_ir_get_input_level: get ir status
 * return:  success:0 or 1; fail:-1
 */
int ak_drv_ir_get_input_level(void)
{
    int status;
    int ret;
	
	ret = dev_ioctl(m_hal_camera.fd, IO_CAMERA_IRFEED_GPIO_GET, &status);
    if (0 !=ret)
    {
		ak_print_error_ex("dev_ioctl IO_CAMERA_IRFEED_GPIO_GET fail!\n");
        dev_ioctl(m_hal_camera.fd, IO_CAMERA_IRFEED_GPIO_CLOSE, &status); 
		return -1;
    }
	return status;
}

/** 
 * ak_drv_ir_set_cut: set ircut to switch  
 * @value[IN]:  status replace day or night
 * return: 0 - success; otherwise -1; 
 */
int ak_drv_ir_set_ircut(enum ircut_status status)
{
	int ret = 0;
	enum ircut_mode mode;

	switch (status) {
		case IRCUT_STATUS_DAY:
		mode = IRCUT_MODE_CLOSED;
		ircut_ioctl(IRCUT_CMD_SET_STATUS, &mode);
		break;

		case IRCUT_STATUS_NIGHT:
		mode = IRCUT_MODE_OPENED;
		ircut_ioctl(IRCUT_CMD_SET_STATUS, &mode);
		break;

		default:
		ak_print_error_ex("status:%d error\n", status);
		ret = -1;
		break;
	}

	return ret;
}
