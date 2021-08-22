#include "anyka_types.h"

#include "isp_conf.h"
#include "drv_ircut.h"

#include "dev_drv.h"
#include "platform_devices.h"

static unsigned int GPIO_IRCUT_CTRL ;               

static void get_camera_gpio(void)
{
	unsigned int data;
	int dev_id;
	T_CAMERA_PLATFORM_INFO * camera_info;
	
	dev_id = dev_open(DEV_CAMERA);
	
	if(dev_id > 0)
	{
	 	dev_read(dev_id, &data, 1);
		camera_info = (T_CAMERA_PLATFORM_INFO *)data;

		GPIO_IRCUT_CTRL = camera_info->ircut_info.gpio_ctrl.nb;
		printf("$$$$$$$ IRCUT_CTRL=%d\r\n",GPIO_IRCUT_CTRL);
	}
	else
	{
		printf("$$$$$$$[%d]%s\r\n",__LINE__,__func__);
	}	
	
}


/**
 * @brief ircut device opened
 * @author ye_guohong   
 * @date 2016-10-19
 * @param[]
 * @return void
 */
int ircut_open(void)
{
	//gpio_init();
	get_camera_gpio();
	
	gpio_set_pin_as_gpio(GPIO_IRCUT_CTRL);

	//set output dir 
	gpio_set_pin_dir(GPIO_IRCUT_CTRL, 1);

	/*  init ircut level*/
	gpio_set_pin_level(GPIO_IRCUT_CTRL, 0);
	gpio_set_pin_level(GPIO_IRCUT_CTRL, 1);
	return 0;
}

/**
 * @brief set ircut statue
 * @author ye_guohong   
 * @date 2016-10-19
 * @param[in] mode  day or night mode
 * @return void
 */
int ircut_ioctl(int cmd, void *arg)
{
	int ret = 0;
	int level;
	enum ircut_mode mode;


	switch (cmd) {
		case IRCUT_CMD_SET_STATUS:
		mode = *((enum ircut_mode *)arg);
		if (mode == IRCUT_MODE_CLOSED)
			level = IRCUT_CTRL_DAY_LEVEL;
		else if (mode == IRCUT_MODE_OPENED)
			level = IRCUT_CTRL_NIGHT_LEVEL;
		else {
			printk("%s mode:%d not defined\n", __func__, mode);
			ret = -1;
			break;
		}
		gpio_set_pin_level(GPIO_IRCUT_CTRL, level);
		break;

		case IRCUT_CMD_GET_STATUS:
		level = gpio_get_pin_level(GPIO_IRCUT_CTRL);
		*((int *)arg) = level;
		break;

		default:
		printk("%s cmd:%d not defined\n", __func__, cmd);
		ret = -1;
		break;
	}

	return ret;
}

/**
 * @brief ircut device closed
 * @author ye_guohong   
 * @date 2016-10-19
 * @param[]
 * @return void
 */
int ircut_close(void)
{
	return 0;
}
