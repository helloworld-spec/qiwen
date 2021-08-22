/**
 * @file camera_sc1235.c
 * @brief camera driver file
 * Copyright (C) 2011 Anyka (Guangzhou) Microelectronics Technology Co., Ltd
 * @author yang_mengxia
 * @date 2017-08-02
 * @version 1.0
 * @ref
 */ 
#include "drv_api.h"

#include "dev_drv.h"
#include "platform_devices.h"

#include "drv_gpio.h"


#include "ak_isp_drv.h"
#include "drv_camera.h"


#define SENSOR_PWDN_LEVEL		0 
#define SENSOR_RESET_LEVEL		0
#define SENSOR_I2C_ADDR         0x60 
#define SENSOR_ID               0x1235
#define SENSOR_MCLK             24
#define SENSOR_REGADDR_BYTE     2
#define SENSOR_DATA_BYTE        1
#define MAX_FPS					25

#define SENSOR_WIDTH			1280
#define SENSOR_HEIGHT			960
#define SENSOR_BUS_TYPE			BUS_TYPE_RAW

#define OV_MAX(a, b)            (((a) < (b) ) ?  (b) : (a))
#define OV_MIN(a, b)            (((a) > (b) ) ?  (b) : (a))
#define OV_CLIP3(low, high, x)  (OV_MAX(OV_MIN((x), high), low))

static int sc1235_set_fps(int fps);
static int sc1235_get_id(void);
static int sc1235_cmos_updata_d_gain(const unsigned int d_gain);
static int sc1235_cmos_updata_a_gain(const unsigned int a_gain);
static int sc1235_cmos_updata_exp_time(unsigned int exp_time);
static int sc1235_cmos_update_a_gain_timer(void);

//static const struct aksensor_win_size sc1235_win[];

static int g_fps = MAX_FPS;
static int read_id_ok = 0;
static int g_a_gain = 0;
static AK_ISP_SENSOR_CB sc1235_callback;
static T_CAMERA_FUNCTION_HANDLER sc1235_handler;
static int drv_i2c_id = -1;     //jk+


static unsigned int GPIO_CAMERA_RESET;            
static unsigned int GPIO_CAMERA_AVDD;            
static unsigned int GPIO_I2C_SCL ;               
static unsigned int GPIO_I2C_SDA ;      




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

		GPIO_CAMERA_RESET = camera_info->sensor_info.gpio_reset.nb;            
		GPIO_CAMERA_AVDD  = camera_info->sensor_info.gpio_avdd.nb;
		GPIO_I2C_SCL      = camera_info->sensor_info.gpio_scl.nb;
		GPIO_I2C_SDA      = camera_info->sensor_info.gpio_sda.nb;
		printf("$$$$$$$rest=%d,avdd=%d,scl=%d,sda=%d\r\n",GPIO_CAMERA_RESET,GPIO_CAMERA_AVDD,GPIO_I2C_SCL,GPIO_I2C_SDA);

	}
	else
	{
		printf("$$$$$$$[%d]%s\r\n",__LINE__,__func__);
	}


	
	
}





/**
 * @brief sensor register read
 * @author yang_mengxia   
 * @date 2017-08-02
 * @param[in] reg sensor register
 * @return int
 * @retval register's value 
 */
static int sc1235_sensor_read_register(int reg)
{
	unsigned char data;
	int ret;
	unsigned long cmd_data[2];
	unsigned char *p_data;
	unsigned long data_addr;
	
	p_data = (unsigned char *)cmd_data;
	p_data[0] = SENSOR_I2C_ADDR;

	p_data[1] = 1;

	p_data[2] = reg;
	p_data[3] = reg>>8;

	data_addr = &data;
	p_data[4] = data_addr;
	p_data[5] = data_addr>>8;
	p_data[6] = data_addr>>16;
	p_data[7] = data_addr>>24;
	
	ret = dev_ioctl(drv_i2c_id, IO_I2C_WORD_READ, cmd_data);
	//ret = sccb_read_data3(SENSOR_I2C_ADDR, reg, &data, 1);
	if (-1 != ret)
		return data;
	return 0;
}

/**
 * @brief sensor register write
 * @author yang_mengxia   
 * @date 2017-08-02
 * @param[in] reg sensor register
 * @param[in] data sensor register's value
 * @return int
 * @retval 0 if write successfully
 * @retval others if write unsuccessfully
 */
static int sc1235_sensor_write_register(int reg, int data)
{
	int ret;
	unsigned char ucdata = data;
	unsigned long cmd_data[2];
	unsigned char *p_data;
	unsigned long data_addr;
	p_data = (unsigned char *)cmd_data;
	p_data[0] = SENSOR_I2C_ADDR;

	p_data[1] = 1;

	p_data[2] = reg;
	p_data[3] = reg>>8;

	data_addr = &ucdata;
	p_data[4] = data_addr;
	p_data[5] = data_addr>>8;
	p_data[6] = data_addr>>16;
	p_data[7] = data_addr>>24;
	
	ret = dev_ioctl(drv_i2c_id, IO_I2C_WORD_WRITE, cmd_data);
	if(-1 != ret)
		return ucdata;
	//return sccb_write_data3(SENSOR_I2C_ADDR, reg, &ucdata, 1);
}

/**
 * @brief set sensor power on
 * @author yang_mengxia   
 * @date 2017-08-02
 * @param[]
 * @return void
 */
static void sc1235_set_poweron(void)
{  

	drv_i2c_id = dev_open(DEV_I2C); //jk+


    gpio_set_pin_as_gpio(GPIO_CAMERA_AVDD);
    gpio_set_pin_dir(GPIO_CAMERA_AVDD, GPIO_DIR_OUTPUT);
    gpio_set_pin_level(GPIO_CAMERA_AVDD, !SENSOR_PWDN_LEVEL);    
    mini_delay(10);
    gpio_set_pin_as_gpio(GPIO_CAMERA_RESET);
    gpio_set_pin_dir(GPIO_CAMERA_RESET, GPIO_DIR_OUTPUT);
    gpio_set_pin_level(GPIO_CAMERA_RESET, SENSOR_RESET_LEVEL);
    mini_delay(10);
    gpio_set_pin_level(GPIO_CAMERA_RESET, !SENSOR_RESET_LEVEL);
    mini_delay(20);

	g_fps = MAX_FPS;
	printf("%s\n",__func__);
}

/**
 * @brief set sensor power off
 * @author yang_mengxia   
 * @date 2017-08-02
 * @param[]
 * @return int
 * @retval 0 if set successfully
 * @retval others if set unsuccessfully
 */
static bool sc1235_set_poweroff(void)
{
	dev_close(drv_i2c_id);  //jk+
	drv_i2c_id = -1;
	
	gpio_set_pin_level(GPIO_CAMERA_AVDD, SENSOR_PWDN_LEVEL);    
    gpio_set_pin_dir(GPIO_CAMERA_RESET, GPIO_DIR_INPUT);
	printf("%s\n",__func__);

    return 0;
}


/**
 * @brief sensor initiation
 * @author yang_mengxia   
 * @date 2017-08-02
 * @param[in] para register and value table
 * @return int
 * @retval 0 if successfully
 * @retval others if unsuccessfully
 */
static int sc1235_init(const AK_ISP_SENSOR_INIT_PARA *para)
{
	int i;
	AK_ISP_SENSOR_REG_INFO *preg_info;
	int value;
	
	preg_info = para->reg_info;
	for (i = 0; i < para->num; i++) {
//value=sc1235_sensor_read_register(preg_info->reg_addr);
//printk(KERN_ERR "before w reg:0x%.x, val:0x%.x, to write:0x%.x\n", preg_info->reg_addr, value, preg_info->value);
		sc1235_sensor_write_register(preg_info->reg_addr, preg_info->value);
//value=sc1235_sensor_read_register(preg_info->reg_addr);
//printk(KERN_ERR "after w reg:0x%.x, val:0x%.x\n", preg_info->reg_addr, value);
		preg_info++;
	}

	return 0;
}

/**
 * @brief get sensor mclk requestion
 * @author yang_mengxia   
 * @date 2017-08-02
 * @param[]
 * @return int mclk unit:MHz
 */
static int sc1235_get_mclk(void)
{
	return SENSOR_MCLK;
}

/**
 * @brief get sensor id
 * @author yang_mengxia   
 * @date 2017-08-02
 * @param[]
 * @return int sensor id
 */
static int sc1235_get_id(void)
{
	unsigned char value;

	printf("==============\r\n");
	get_camera_gpio();
	printf("==============\r\n");



	//sccb_set_soft_hard_flag(0);
	//sccb_init(GPIO_I2C_SCL, GPIO_I2C_SDA);

	if (read_id_ok)
		goto success;
	
	unsigned int id;
    value = sc1235_sensor_read_register(0x3107);
    id = value << 8;

    value = sc1235_sensor_read_register(0x3108);
    id |= value;

	//printk(KERN_ERR "%s id:0x%x\n", __func__, id);
    //return SENSOR_ID;
	if (id != SENSOR_ID)
		goto fail;

	read_id_ok = 1;

success:
    return SENSOR_ID;

fail:
	return 0;
}

/**
 * @brief set sensor framerate
 * @author yang_mengxia   
 * @date 2017-08-02
 * @param[in] fps sensor's framerate
 * @return int
 * @retval 0 if successfully
 * @retval others if unsuccessfully
 */
static int sc1235_set_fps(int fps)
{
	int ret = 0;
	int tmp=0;

	/* tmp = 54M / (reg0x320c~d=2160) / fps */
	switch (fps) {
		case 25:
			tmp = 0x03e8;
			break;
		case 13:    //actual: 12.5fps
		case 12:
			tmp = 0x07e0;
			break;
		case 10:
			tmp = 0x09c4;
			break;
		case 8:
			tmp = 0x0c35;
			break;
		case 5:
			tmp = 0x1388;
			break;
		default:
		printf("%s set fps fail\n", __func__);
		ret = -1;
		break;
	}

	if (!ret) {
		sc1235_sensor_write_register(0x320e, tmp>>8);
		sc1235_sensor_write_register(0x320f, tmp & 0xff);
#if 0
		sc1235_sensor_write_register(0x3336, (tmp - 0x2e8) >> 8);
		sc1235_sensor_write_register(0x3337, (tmp - 0x2e8) & 0xff);
		sc1235_sensor_write_register(0x3338, tmp>>8);
		sc1235_sensor_write_register(0x3339, tmp & 0xff);
#endif

		g_fps = fps;
	}

	return ret;
}

/**
 * @brief get sensor framerate
 * @author yang_mengxia   
 * @date 2017-08-02
 * @param[]
 * @return int sensor's framerate
 */
static int sc1235_get_fps(void)
{
	return g_fps;
}

/**
 * @brief get sensor data output bus type
 * @author yang_mengxia   
 * @date 2017-08-02
 * @param[]
 * @return enum sensor_bus_type sensor data type
 */
static enum sensor_bus_type sc1235_get_bus_type(void)
{
	return SENSOR_BUS_TYPE;
}

/**
 * @brief set sensor's all callback functions
 * @author yang_mengxia   
 * @date 2017-08-02
 * @param[]
 * @return int
 * @retval 0 if successfully
 * @retval others if unsuccessfully
 */
static int camera_sc1235_reg(void)
{
	sc1235_callback.sensor_init_func 				= sc1235_init,
	sc1235_callback.sensor_read_reg_func			= sc1235_sensor_read_register,
	sc1235_callback.sensor_write_reg_func			= sc1235_sensor_write_register,
	sc1235_callback.sensor_read_id_func			= sc1235_get_id,
	sc1235_callback.sensor_update_a_gain_func		= sc1235_cmos_updata_a_gain,
	sc1235_callback.sensor_update_d_gain_func		= sc1235_cmos_updata_d_gain,
	sc1235_callback.sensor_updata_exp_time_func	= sc1235_cmos_updata_exp_time,
	sc1235_callback.sensor_timer_func = sc1235_cmos_update_a_gain_timer,

	sc1235_handler.isp_sensor_cb		= &sc1235_callback;
	sc1235_handler.cam_width			= SENSOR_WIDTH;
	sc1235_handler.cam_height			= SENSOR_HEIGHT;
	sc1235_handler.cam_mclk			= SENSOR_MCLK;
	sc1235_handler.cam_open_func		= sc1235_set_poweron;
	sc1235_handler.cam_close_func		= sc1235_set_poweroff;
	sc1235_handler.cam_read_id_func	= sc1235_get_id;
	sc1235_handler.cam_set_framerate_func = sc1235_set_fps;
	sc1235_handler.cam_get_framerate_func = sc1235_get_fps;
	sc1235_handler.cam_get_bus_type_func	= sc1235_get_bus_type;

	sc1235_handler.sensor_id = SENSOR_ID;

    camera_reg_dev(SENSOR_ID, &sc1235_handler);
    return 0;
}

#ifdef __CC_ARM
#pragma arm section rwdata = "__initcall_", zidata = "__initcall_"
#endif
module_init(camera_sc1235_reg)
#ifdef __CC_ARM
#pragma arm section
#endif

///////////////////////////////////////////////////////////////////////////////////////////
static int sc1235_curr_again_level = 0;

/*
 gain_level:
 0: <2x
 1: <4x
 2: >4x
 */
static void sc1235_ext(int gain_level)
{
	sc1235_sensor_write_register(0x3903, 0x84);
	sc1235_sensor_write_register(0x3903, 0x04);
	sc1235_sensor_write_register(0x3812, 0x00);
	switch (gain_level) {
		case 0:
			sc1235_sensor_write_register(0x3631, 0x84);
			sc1235_sensor_write_register(0x3301, 0x05);
			break;
		case 1:
			sc1235_sensor_write_register(0x3631, 0x88);
			sc1235_sensor_write_register(0x3301, 0x1f);
			break;
		case 2:
			sc1235_sensor_write_register(0x3631, 0x88);
			sc1235_sensor_write_register(0x3301, 0xff);
			break;
		default:
			printk("%s gain_level error\n", __func__);
			break;
	}

	sc1235_sensor_write_register(0x3812, 0x30);
}

static int sc1235_cmos_updata_d_gain(const unsigned int d_gain)
{
	return 0;
}

static int sc1235_cmos_updata_a_gain(const unsigned int a_gain)
{
	//??????ģ???????Ļص?????
	int tmp;

	g_a_gain  = a_gain;

	tmp = a_gain >> 4;
	sc1235_sensor_write_register(0x3e08, tmp >> 8);
	sc1235_sensor_write_register(0x3e09, tmp & 0xff);
	if (tmp < (2 << 4)) {
		sc1235_curr_again_level = 0;
		sc1235_ext(0);
	} else if (tmp < (4 << 4)) {
		sc1235_curr_again_level = 1;
		sc1235_ext(1);
	} else {
		sc1235_curr_again_level = 2;
		sc1235_ext(2);
	}

	return 0;
}

static int sc1235_cmos_updata_exp_time(unsigned int exp_time)
{
    //?????ع?ʱ???Ļص?????
    unsigned char exposure_time_msb;
	unsigned char exposure_time_lsb;
	unsigned char tem;
 	//printk("exp_time=%d ",exp_time);
    exposure_time_msb =(exp_time>>4)&0xff;
    exposure_time_lsb =((exp_time)&0xf)<<4;
    tem=sc1235_sensor_read_register(0x3e02)&0x0f;
    exposure_time_lsb =exposure_time_lsb|tem;
	//printk("msb = %d  ",exposure_time_msb);
	//printk("lsb = %d\n",exposure_time_lsb);
	sc1235_sensor_write_register(0x3e01,exposure_time_msb);
	sc1235_sensor_write_register(0x3e02,exposure_time_lsb);

	sc1235_ext(sc1235_curr_again_level);

    return 0;
}

static int sc1235_cmos_update_a_gain_timer(void)
{
#if 0
	#define UPDATE_A_GAIN_TIMER_PERIOD	(5)	//s
	static struct timeval stv = {.tv_sec = 0,};
	struct timeval tv;

	do_gettimeofday(&tv);

	if (tv.tv_sec >= stv.tv_sec + UPDATE_A_GAIN_TIMER_PERIOD) {
		stv.tv_sec = tv.tv_sec;
		sc1235_cmos_updata_a_gain(g_a_gain);
	}

	return 0;
#endif

	#define UPDATE_A_GAIN_TIMER_PERIOD	(5) //s
	static unsigned long stick = 0;
	unsigned long tick;
	unsigned long sub;

	tick = get_tick_count();
	if (tick >= stick) {
		sub = tick - stick;
	} else {
		sub = (~((unsigned long )0)) - stick + tick;
	}

	if (sub >= UPDATE_A_GAIN_TIMER_PERIOD * 1000) {
		stick = tick;
		sc1235_cmos_updata_a_gain(g_a_gain);
	}

	return 0;
}

