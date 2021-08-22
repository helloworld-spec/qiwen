/**
 * @file camera_sc1135.c
 * @brief camera driver file
 * Copyright (C) 2011 Anyka (Guangzhou) Microelectronics Technology Co., Ltd
 * @author luweichun
 * @date 2015-11-05
 * @version 1.0
 * @ref
 */ 
#include "drv_api.h"

#include "dev_drv.h"
#include "platform_devices.h"

#include "drv_gpio.h"


#include "ak_isp_drv.h"
#include "drv_camera.h"


#define SENSOR_PWDN_LEVEL		1 
#define SENSOR_RESET_LEVEL		0
#define SENSOR_I2C_ADDR         0x60 
#define SENSOR_ID               0x1135
#define SENSOR_MCLK             27
#define SENSOR_REGADDR_BYTE     2
#define SENSOR_DATA_BYTE        1
#define MAX_FPS					30
#define DEFAULT_FPS				25

#define SENSOR_WIDTH			1280
#define SENSOR_HEIGHT			960
#define SENSOR_BUS_TYPE			BUS_TYPE_RAW

#define OV_MAX(a, b)            (((a) < (b) ) ?  (b) : (a))
#define OV_MIN(a, b)            (((a) > (b) ) ?  (b) : (a))
#define OV_CLIP3(low, high, x)  (OV_MAX(OV_MIN((x), high), low))

static int sc1135_set_fps(int fps);
static int sc1135_get_id(void);
static int sc1135_cmos_updata_d_gain(const unsigned int d_gain);
static int sc1135_cmos_updata_a_gain(const unsigned int a_gain);
static int sc1135_cmos_updata_exp_time(unsigned int exp_time);
static int sc1135_cmos_update_a_gain_timer(void);

//static const struct aksensor_win_size sc1135_win[];

static int g_fps = DEFAULT_FPS;
static int read_id_ok = 0;
static int g_a_gain = 0;
static AK_ISP_SENSOR_CB sc1135_callback;
static T_CAMERA_FUNCTION_HANDLER sc1135_handler;
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
 * @author ye_guohong   
 * @date 2016-10-19
 * @param[in] reg sensor register
 * @return int
 * @retval register's value 
 */
static int sc1135_sensor_read_register(int reg)
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
 * @author ye_guohong   
 * @date 2016-10-19
 * @param[in] reg sensor register
 * @param[in] data sensor register's value
 * @return int
 * @retval 0 if write successfully
 * @retval others if write unsuccessfully
 */
static int sc1135_sensor_write_register(int reg, int data)
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
 * @author ye_guohong   
 * @date 2016-10-19
 * @param[]
 * @return void
 */
static void sc1135_set_poweron(void)
{  
	drv_i2c_id = dev_open(DEV_I2C); //jk+

	//gpio_init();
	gpio_set_pin_as_gpio(GPIO_CAMERA_AVDD);
	gpio_set_pin_as_gpio(GPIO_CAMERA_RESET);	
	gpio_set_pin_dir(GPIO_CAMERA_AVDD, GPIO_DIR_OUTPUT);
    gpio_set_pin_level(GPIO_CAMERA_AVDD, SENSOR_PWDN_LEVEL);   
    gpio_set_pin_dir(GPIO_CAMERA_RESET, GPIO_DIR_OUTPUT);
    gpio_set_pin_level(GPIO_CAMERA_RESET, SENSOR_RESET_LEVEL);
	mini_delay(5);
    gpio_set_pin_level(GPIO_CAMERA_AVDD, !SENSOR_PWDN_LEVEL);   
    gpio_set_pin_level(GPIO_CAMERA_RESET, !SENSOR_RESET_LEVEL);
	g_fps = DEFAULT_FPS;
	printf("%s\n",__func__);
}

/**
 * @brief set sensor power off
 * @author ye_guohong   
 * @date 2016-10-19
 * @param[]
 * @return int
 * @retval 0 if set successfully
 * @retval others if set unsuccessfully
 */
static bool sc1135_set_poweroff(void)
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
 * @author ye_guohong   
 * @date 2016-10-19
 * @param[in] para register and value table
 * @return int
 * @retval 0 if successfully
 * @retval others if unsuccessfully
 */
static int sc1135_init(const AK_ISP_SENSOR_INIT_PARA *para)
{
	int i;
	AK_ISP_SENSOR_REG_INFO *preg_info;
	int value;
	
	preg_info = para->reg_info;
	for (i = 0; i < para->num; i++) {
//value=sc1135_sensor_read_register(preg_info->reg_addr);
//printf("before w reg:0x%.x, val:0x%.x, to write:0x%.x\n", preg_info->reg_addr, value, preg_info->value);
		sc1135_sensor_write_register(preg_info->reg_addr, preg_info->value);
//value=sc1135_sensor_read_register(preg_info->reg_addr);
//printf("after w reg:0x%.x, val:0x%.x\n", preg_info->reg_addr, value);
		preg_info++;
	}

	return 0;
}

/**
 * @brief get sensor mclk requestion
 * @author ye_guohong   
 * @date 2016-10-19
 * @param[]
 * @return int mclk unit:MHz
 */
static int sc1135_get_mclk(void)
{
	return SENSOR_MCLK;
}

/**
 * @brief get sensor id
 * @author ye_guohong   
 * @date 2016-10-19
 * @param[]
 * @return int sensor id
 */
static int sc1135_get_id(void)
{
	/* sc1135 virtual-id registers are same as sc1037*/
	unsigned char value;

	printf("==============\r\n");
	get_camera_gpio();
	printf("==============\r\n");



	//sccb_set_soft_hard_flag(0);
	//sccb_init(GPIO_I2C_SCL, GPIO_I2C_SDA);

	if (read_id_ok)
		goto success;

#if 1

	value = sc1135_sensor_read_register(0x3107);
	if (value != 0x00)
		goto fail;

	value = sc1135_sensor_read_register(0x3108);
	if (value != 0x00)
		goto fail;

	value = sc1135_sensor_read_register(0x3109);
	if (value != 0x00)
		goto fail;

	value = sc1135_sensor_read_register(0x3c05);
	if (value != 0x00)
		goto fail;

	value = sc1135_sensor_read_register(0x580b);
	if (value != 0x00)
		goto fail;
#endif

	read_id_ok = 1;

success:
	//printf("%s ok\n",__func__);
	return SENSOR_ID;

fail:
	//printf("%s fail\n",__func__);
	return 0;

}

/**
 * @brief set sensor framerate
 * @author ye_guohong   
 * @date 2016-10-19
 * @param[in] fps sensor's framerate
 * @return int
 * @retval 0 if successfully
 * @retval others if unsuccessfully
 */
static int sc1135_set_fps(int fps)
{
	int ret = 0;

	switch (fps) {
	case 30:
		sc1135_sensor_write_register(0x320e, 0x03);
		sc1135_sensor_write_register(0x320f, 0xe8);//ymx need 0xf1

		sc1135_sensor_write_register(0x3336, (0x3e8 - 0x2e8) >> 8);
		sc1135_sensor_write_register(0x3337, (0x3e8 - 0x2e8) & 0xff);
		sc1135_sensor_write_register(0x3338, 0x03);
		sc1135_sensor_write_register(0x3339, 0xe8);
		break;
	case 25:
		sc1135_sensor_write_register(0x320e, 0x04);
		sc1135_sensor_write_register(0x320f, 0xbc);

		sc1135_sensor_write_register(0x3336, (0x4bc - 0x2e8) >> 8);
		sc1135_sensor_write_register(0x3337, (0x4bc - 0x2e8) & 0xff);
		sc1135_sensor_write_register(0x3338, 0x04);
		sc1135_sensor_write_register(0x3339, 0xbc);
		break;
	case 13:	//actual: 12.5fps
	case 12:
		sc1135_sensor_write_register(0x320e, 0x09);
		sc1135_sensor_write_register(0x320f, 0x60);

		sc1135_sensor_write_register(0x3336, (0x960 - 0x2e8) >> 8);
		sc1135_sensor_write_register(0x3337, (0x960 - 0x2e8) & 0xff);
		sc1135_sensor_write_register(0x3338, 0x09);
		sc1135_sensor_write_register(0x3339, 0x60);
		break;
	case 10:
		sc1135_sensor_write_register(0x320e, 0x0b);
		sc1135_sensor_write_register(0x320f, 0xb8);

		sc1135_sensor_write_register(0x3336, (0xbb8 - 0x2e8) >> 8);
		sc1135_sensor_write_register(0x3337, (0xbb8 - 0x2e8) & 0xff);
		sc1135_sensor_write_register(0x3338, 0x0b);
		sc1135_sensor_write_register(0x3339, 0xb8);
		break;
	case 8:
		sc1135_sensor_write_register(0x320e, 0x0e);
		sc1135_sensor_write_register(0x320f, 0xa6);

		sc1135_sensor_write_register(0x3336, (0xea6 - 0x2e8) >> 8);
		sc1135_sensor_write_register(0x3337, (0xea6 - 0x2e8) & 0xff);
		sc1135_sensor_write_register(0x3338, 0x0e);
		sc1135_sensor_write_register(0x3339, 0xa6);
		break;
	default:
		printf("%s set fps fail\n", __func__);
		ret = -1;
		break;
	}

	if (!ret)
		g_fps = fps;

	return ret;

}

/**
 * @brief get sensor framerate
 * @author ye_guohong   
 * @date 2016-10-19
 * @param[]
 * @return int sensor's framerate
 */
static int sc1135_get_fps(void)
{
	return g_fps;
}

/**
 * @brief get sensor data output bus type
 * @author ye_guohong   
 * @date 2016-10-19
 * @param[]
 * @return enum sensor_bus_type sensor data type
 */
static enum sensor_bus_type sc1135_get_bus_type(void)
{
	return SENSOR_BUS_TYPE;
}

/**
 * @brief set sensor's all callback functions
 * @author ye_guohong   
 * @date 2016-10-19
 * @param[]
 * @return int
 * @retval 0 if successfully
 * @retval others if unsuccessfully
 */
static int camera_sc1135_reg(void)
{
	sc1135_callback.sensor_init_func 				= sc1135_init,
	sc1135_callback.sensor_read_reg_func			= sc1135_sensor_read_register,
	sc1135_callback.sensor_write_reg_func			= sc1135_sensor_write_register,
	sc1135_callback.sensor_read_id_func			= sc1135_get_id,
	sc1135_callback.sensor_update_a_gain_func		= sc1135_cmos_updata_a_gain,
	sc1135_callback.sensor_update_d_gain_func		= sc1135_cmos_updata_d_gain,
	sc1135_callback.sensor_updata_exp_time_func	= sc1135_cmos_updata_exp_time,
	sc1135_callback.sensor_timer_func = sc1135_cmos_update_a_gain_timer,

	sc1135_handler.isp_sensor_cb		= &sc1135_callback;
	sc1135_handler.cam_width			= SENSOR_WIDTH;
	sc1135_handler.cam_height			= SENSOR_HEIGHT;
	sc1135_handler.cam_mclk			= SENSOR_MCLK;
	sc1135_handler.cam_open_func		= sc1135_set_poweron;
	sc1135_handler.cam_close_func		= sc1135_set_poweroff;
	sc1135_handler.cam_read_id_func	= sc1135_get_id;
	sc1135_handler.cam_set_framerate_func = sc1135_set_fps;
	sc1135_handler.cam_get_framerate_func = sc1135_get_fps;
	sc1135_handler.cam_get_bus_type_func	= sc1135_get_bus_type;

	sc1135_handler.sensor_id = SENSOR_ID;

    camera_reg_dev(SENSOR_ID, &sc1135_handler);
    return 0;
}

#ifdef __CC_ARM
#pragma arm section rwdata = "__initcall_", zidata = "__initcall_"
#endif
module_init(camera_sc1135_reg)
#ifdef __CC_ARM
#pragma arm section
#endif

///////////////////////////////////////////////////////////////////////////////////////////
//for SC1135
#define SENSOR_BLC_TOP_VALUE (0x58)
#define SENSOR_BLC_BOT_VALUE (0x45)
#define SENSOR_AGAIN_ADAPT_STEP (1)
#define SENSOR_MAX_AGAIN (0x7f)
static const unsigned short sensor_gain_map[] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 
	0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f, 
	0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f, 
	0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f, 
	0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, 
	0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f
};

static unsigned short gainmap_gain2index(unsigned short Gain)
{
	unsigned short i = 0;
	for(i = 0; i < sizeof(sensor_gain_map)/sizeof(sensor_gain_map[0]); i++){
		if(sensor_gain_map[i] == Gain){
			break;
		}
	}
	return i;
}

//return limited Again
//param:Again that ISP calculated
static unsigned int Again_limit(unsigned int Again)
{
	unsigned int ret_Again;
	unsigned short BLC_top = SENSOR_BLC_TOP_VALUE, BLC_bot = SENSOR_BLC_BOT_VALUE, BLC_reg/*, Again_step = SENSOR_AGAIN_ADAPT_STEP*/, gain_index;
	static unsigned short MaxAgain = SENSOR_MAX_AGAIN;

	//I2C read reg of sensor
	BLC_reg = sc1135_sensor_read_register(0x3911);

	gain_index = gainmap_gain2index(MaxAgain);

	if(BLC_reg > BLC_top){//>0x58
		if(gain_index>0){
			//limit max Again by step
			gain_index -= SENSOR_AGAIN_ADAPT_STEP;
		}
		MaxAgain = sensor_gain_map[gain_index];
	}else if(BLC_reg < BLC_bot){//<0x45
		//release Again limit by step
		gain_index += SENSOR_AGAIN_ADAPT_STEP;
		if(gain_index > sizeof(sensor_gain_map)/sizeof(sensor_gain_map[0])-1){
			gain_index = sizeof(sensor_gain_map)/sizeof(sensor_gain_map[0]) - 1;
		}
		MaxAgain = sensor_gain_map[gain_index];
	}else{//0x45 < BLC_reg < 0x58
		//do nothing
	}
	ret_Again = Again > MaxAgain ? MaxAgain : Again;
	//printf("limit gain:ret_Again=%d;Again=%d;MaxAgain=%d\n", ret_Again, Again, MaxAgain);
	return ret_Again;
}

#define SKIP_FRAME_NUM	2
static int sc1135_curr_2x_again = 0;
static unsigned int _cmos_gains_convert(unsigned int a_gain, unsigned int d_gain,
		unsigned int *a_gain_out ,unsigned int *d_gain_out)
{
	//模??????????转???????????惴??????????转????sensor?????????
	unsigned int  fine_gain= 0;
	unsigned int ag_value= 0x00;
	unsigned int corse_gain=0,sensor_d_gain = 0,sensor_2x_again=0;
	int tmp = 0;
	//printf(KERN_ERR "a_gain =%d\n",a_gain);
	if(a_gain<=1980)                           //1 - 1.8824
	{
		corse_gain = 0;
		tmp = a_gain-1024;
    }
    else if(a_gain>1980 && a_gain<=3960)       // 2 - 3.7648
    {
    	corse_gain = 1;
		tmp = a_gain/2-1024;
    }
    else if(a_gain>3960 && a_gain<=7920)       //4-7.5296
    {
    	corse_gain = 3;
		tmp = a_gain/4-1024;
    }
    else if(a_gain>7920 && a_gain<=15840)       //8 - 15.0592
    {
        corse_gain = 7;
		tmp = a_gain/8-1024;
    }
    else if(a_gain>15840 && a_gain<=31680)    // 16 - 30.11
    {
    	corse_gain = 7;
		sensor_2x_again =1;
		a_gain=a_gain>>1;
		tmp = a_gain/8-1024;
    }
    else if(a_gain>31680)   				// 大于30 - 60.23
    {
    	corse_gain = 7;
		sensor_2x_again =1;
		a_gain = 15840;
		tmp = 956;
    }
    
    if(tmp>=0){
		fine_gain = tmp/60;
	}
	else{			
		fine_gain = 0;
	}
	
	*a_gain_out = a_gain;
	*d_gain_out = sensor_d_gain;
    ag_value = corse_gain<<4|(fine_gain&0xf);
	
	//printf(KERN_ERR "corse=%d fine=%d\n",corse_gain,fine_gain);
	sc1135_sensor_write_register(0x3e09, Again_limit(ag_value));
	//sc1135_sensor_write_register(0x3e08, sensor_d_gain);
	if(sensor_2x_again==1)
		sc1135_sensor_write_register(0x3635, 0x0c);		//2x again
	else
		sc1135_sensor_write_register(0x3635, 0x0);		
		
	if(corse_gain<1)      //2 倍增益
	{
		sc1135_sensor_write_register(0x3630, 0xb8);
		sc1135_sensor_write_register(0x3631, 0x82);
	}
	else if(sensor_2x_again<1)      //2-16倍增益
	{
		sc1135_sensor_write_register(0x3630, 0x70);
		sc1135_sensor_write_register(0x3631, 0x8e);
	}
	else
	{
		sc1135_sensor_write_register(0x3630, 0x70);
		sc1135_sensor_write_register(0x3631, 0x8c);
	}
	//printf(" 0x3e09 = %d\n", sc1135_sensor_read_register(0x3e09));
	//printf(" 0x3e0f = %d\n", sc1135_sensor_read_register(0x360f));

	if(sc1135_curr_2x_again!=sensor_2x_again)
	{
		sc1135_curr_2x_again=sensor_2x_again;
		return SKIP_FRAME_NUM;
	}
	else 
		return 0;
}

static int sc1135_cmos_updata_d_gain(const unsigned int d_gain)
{
	//?????????????幕氐?????
	return 0;
}

static int sc1135_cmos_updata_a_gain(unsigned int a_gain)
{
	//??????模???????幕氐?????
	unsigned int tmp_a_gain;
	unsigned int tmp_d_gain;
	unsigned int tmp_a_gain_out;
	unsigned int tmp_d_gain_out;

	g_a_gain = a_gain;

	tmp_a_gain = a_gain<<2;
	tmp_d_gain = 0;


	return _cmos_gains_convert(tmp_a_gain, tmp_d_gain,  &tmp_a_gain_out ,&tmp_d_gain_out);
	
	
	//printf(" 0x3e09 = %d\n", sc1135_sensor_read_register(0x3e09));
	//printf(" 0x3610 = %d\n", sc1135_sensor_read_register(0x3610));
	//printf(" 0x3605 = %d\n", sc1135_sensor_read_register(0x3605));
	//printf(" tmp_a_gain = %d ",tmp_a_gain);
	//printf(" tmp_a_gain_out = %d\n", tmp_a_gain_out);
	//updata_auto_exposure_num++;
	//isp->aec_param.current_exposure_time = tmp_exposure_time;
	//return 0;
}


static int sc1135_cmos_updata_exp_time(unsigned int exp_time)
{
    //?????毓?时???幕氐?????	
	unsigned char exposure_time_msb;
	unsigned char exposure_time_lsb;
	unsigned char tem;
	unsigned int  ex_time_max, reg_value_hight, reg_value_low;
	//printk("exp_time=%d ",exp_time);
	exposure_time_msb =(exp_time>>4)&0xff;
	exposure_time_lsb =((exp_time)&0xf)<<4;
	tem=sc1135_sensor_read_register(0x3e02)&0x0f;
	exposure_time_lsb =exposure_time_lsb|tem;
	//printk("msb = %d	",exposure_time_msb);
	//printk("lsb = %d\n",exposure_time_lsb);
	sc1135_sensor_write_register(0x3e01,exposure_time_msb);
	sc1135_sensor_write_register(0x3e02,exposure_time_lsb);
	
	reg_value_hight = sc1135_sensor_read_register(0x320e);
	reg_value_low = sc1135_sensor_read_register(0x320f);
	ex_time_max = ((reg_value_hight<<8)|reg_value_low)-4;
	/*
	if(exp_time<=ex_time_max-300)
	{
		sc1135_sensor_write_register(0x331e,0xe0);
	}
	else
	{
		sc1135_sensor_write_register(0x331e,224-(exp_time-(ex_time_max-300))*198/300);
	}
	*/
	return 0;

}

static int sc1135_cmos_update_a_gain_timer(void)
{
#if 0
	#define UPDATE_A_GAIN_TIMER_PERIOD	(5)	//s
	static struct timeval stv = {.tv_sec = 0,};
	struct timeval tv;

	do_gettimeofday(&tv);

	if (tv.tv_sec >= stv.tv_sec + UPDATE_A_GAIN_TIMER_PERIOD) {
		stv.tv_sec = tv.tv_sec;
		sc1135_cmos_updata_a_gain(g_a_gain);
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
		sc1135_cmos_updata_a_gain(g_a_gain);
	}

	return 0;
}

