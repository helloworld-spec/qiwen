/**
 * @file camera_sc1145.c
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
#define SENSOR_ID               0x1145
#define SENSOR_MCLK             24
#define SENSOR_REGADDR_BYTE     2
#define SENSOR_DATA_BYTE        1
#define MAX_FPS					25
#define SENSOR_WIDTH			1280
#define SENSOR_HEIGHT			720
#define SENSOR_BUS_TYPE			BUS_TYPE_RAW

#define OV_MAX(a, b)            (((a) < (b) ) ?  (b) : (a))
#define OV_MIN(a, b)            (((a) > (b) ) ?  (b) : (a))
#define OV_CLIP3(low, high, x)  (OV_MAX(OV_MIN((x), high), low))

static int sc1145_set_fps(int fps);
static int sc1145_get_id(void);
static int sc1145_cmos_updata_d_gain(const unsigned int d_gain);
static int sc1145_cmos_updata_a_gain(const unsigned int a_gain);
static int sc1145_cmos_updata_exp_time(unsigned int exp_time);
static int sc1145_cmos_update_a_gain_timer(void);

//static const struct aksensor_win_size sc1145_win[];


static unsigned int GPIO_CAMERA_RESET;            
static unsigned int GPIO_CAMERA_AVDD;            
static unsigned int GPIO_I2C_SCL ;               
static unsigned int GPIO_I2C_SDA ;                




static int g_fps = MAX_FPS;
static int g_a_gain = 0;
static AK_ISP_SENSOR_CB sc1145_callback;
static T_CAMERA_FUNCTION_HANDLER sc1145_handler;
static int drv_i2c_id = -1;     //jk+


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
static int sc1145_sensor_read_register(int reg)
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
static int sc1145_sensor_write_register(int reg, int data)
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
static void sc1145_set_poweron(void)
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
	mini_delay(1);
	
	g_fps = MAX_FPS;
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
static bool sc1145_set_poweroff(void)
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
static int sc1145_init(const AK_ISP_SENSOR_INIT_PARA *para)
{
	int i;
	AK_ISP_SENSOR_REG_INFO *preg_info;
	int value;
	
	preg_info = para->reg_info;
	for (i = 0; i < para->num; i++) {
//value=sc1145_sensor_read_register(preg_info->reg_addr);
//printf("before w reg:0x%.x, val:0x%.x, to write:0x%.x\n", preg_info->reg_addr, value, preg_info->value);
		sc1145_sensor_write_register(preg_info->reg_addr, preg_info->value);
//value=sc1145_sensor_read_register(preg_info->reg_addr);
//printf("after w reg:0x%.x, val:0x%.x\n", preg_info->reg_addr, value);
		preg_info++;
#if 0
		if ((i == para->num - 2) && (init_exp_attr.exp != 0)) {
			sc1145_cmos_updata_exp_time(init_exp_attr.exp);
			sc1145_cmos_updata_a_gain(init_exp_attr.again);
			sc1145_cmos_updata_d_gain(init_exp_attr.dgain);
		}
#endif
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
static int sc1145_get_mclk(void)
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
static int sc1145_get_id(void)
{
    unsigned char value;
    unsigned int id;

	printf("==============\r\n");
	get_camera_gpio();
	printf("==============\r\n");

	//sccb_set_soft_hard_flag(0);
	//sccb_init(GPIO_I2C_SCL, GPIO_I2C_SDA);

    value = sc1145_sensor_read_register(0x3107);
    id = value << 8;

    value = sc1145_sensor_read_register(0x3108);
    id |= value;

	printf("%s id:0x%x\n", __func__, id);
	if (id != SENSOR_ID)
		goto fail;

    value = sc1145_sensor_read_register(0x3109);
	printf("%s reg[0x3109]:0x%x\n", __func__, value);
	if (value != 0x01)
		goto fail;
           
    return SENSOR_ID;

fail:
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
static int sc1145_set_fps(int fps)
{
	int ret = 0;

	switch (fps) {
	case 30:
		sc1145_sensor_write_register(0x320e, 0x02);
		sc1145_sensor_write_register(0x320f, 0xee);
		sc1145_sensor_write_register(0x3338, 0x02);
		sc1145_sensor_write_register(0x3339, 0xee);
		sc1145_sensor_write_register(0x3336, 0x00);
		sc1145_sensor_write_register(0x3337, 0x00);
		break;
	case 25:
		sc1145_sensor_write_register(0x320e, 0x03);
		sc1145_sensor_write_register(0x320f, 0x84);
		sc1145_sensor_write_register(0x3338, 0x03);
		sc1145_sensor_write_register(0x3339, 0x84);
		sc1145_sensor_write_register(0x3336, 0x00);
		sc1145_sensor_write_register(0x3337, 0x96);
		break;
	case 13:	//actual: 12.5fps
	case 12:
		sc1145_sensor_write_register(0x320e, 0x07);
		sc1145_sensor_write_register(0x320f, 0x08);
		sc1145_sensor_write_register(0x3338, 0x07);
		sc1145_sensor_write_register(0x3339, 0x08);
		sc1145_sensor_write_register(0x3336, 0x04);
		sc1145_sensor_write_register(0x3337, 0x1a);
		break;
	case 10:
		sc1145_sensor_write_register(0x320e, 0x08);
		sc1145_sensor_write_register(0x320f, 0xca);
		sc1145_sensor_write_register(0x3338, 0x08);
		sc1145_sensor_write_register(0x3339, 0xca);
		sc1145_sensor_write_register(0x3336, 0x05);
		sc1145_sensor_write_register(0x3337, 0xdc);
		break;
	case 8:
		sc1145_sensor_write_register(0x320e, 0x0a);
		sc1145_sensor_write_register(0x320f, 0x8c);
		sc1145_sensor_write_register(0x3338, 0x0a);
		sc1145_sensor_write_register(0x3339, 0x8c);
		sc1145_sensor_write_register(0x3336, 0x07);
		sc1145_sensor_write_register(0x3337, 0x9e);
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
static int sc1145_get_fps(void)
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
static enum sensor_bus_type sc1145_get_bus_type(void)
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
static int camera_sc1145_reg(void)
{
	sc1145_callback.sensor_init_func 				= sc1145_init,
	sc1145_callback.sensor_read_reg_func			= sc1145_sensor_read_register,
	sc1145_callback.sensor_write_reg_func			= sc1145_sensor_write_register,
	sc1145_callback.sensor_read_id_func			= sc1145_get_id,
	sc1145_callback.sensor_update_a_gain_func		= sc1145_cmos_updata_a_gain,
	sc1145_callback.sensor_update_d_gain_func		= sc1145_cmos_updata_d_gain,
	sc1145_callback.sensor_updata_exp_time_func	= sc1145_cmos_updata_exp_time,
	sc1145_callback.sensor_timer_func = sc1145_cmos_update_a_gain_timer,

	sc1145_handler.isp_sensor_cb		= &sc1145_callback;
	sc1145_handler.cam_width			= SENSOR_WIDTH;
	sc1145_handler.cam_height			= SENSOR_HEIGHT;
	sc1145_handler.cam_mclk			= SENSOR_MCLK;
	sc1145_handler.cam_open_func		= sc1145_set_poweron;
	sc1145_handler.cam_close_func		= sc1145_set_poweroff;
	sc1145_handler.cam_read_id_func	= sc1145_get_id;
	sc1145_handler.cam_set_framerate_func = sc1145_set_fps;
	sc1145_handler.cam_get_framerate_func = sc1145_get_fps;
	sc1145_handler.cam_get_bus_type_func	= sc1145_get_bus_type;

	sc1145_handler.sensor_id = SENSOR_ID;


    camera_reg_dev(SENSOR_ID, &sc1145_handler);
    return 0;
}

#ifdef __CC_ARM
#pragma arm section rwdata = "__initcall_", zidata = "__initcall_"
#endif
module_init(camera_sc1145_reg)
#ifdef __CC_ARM
#pragma arm section
#endif

///////////////////////////////////////////////////////////////////////////////////////////
static int a_gain_2x_now=1;
static int a_gain_4x_now=1;
static int a_gain_2x_prev=1;
static int a_gain_4x_prev=1;
//static int a_gain_2x_4x_locked=0;
static unsigned int _cmos_gains_convert(unsigned int a_gain, unsigned int d_gain,
		unsigned int *a_gain_out ,unsigned int *d_gain_out)
{
	//ģ??????????ת???????????㷨??????????ת????sensor????????ʽ
	unsigned int ag_interget_table[61] = {0,  1, 2, 2, 4, 4, 4, 4, 8, 8, 8, 8,
        8, 8, 8, 8,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,32,32,32,32,
        32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32};
    unsigned int ag_integer, ag_fraction, tmp;
	unsigned int ag_return;
 	tmp =  a_gain>>4;
	if(tmp == 0)
	{
		tmp = 1;
		printf("Div ERROR 000!\n");
	}
	//printk(" a_gain = %d ",a_gain);
    tmp =  OV_CLIP3(0, 60, tmp);
    ag_integer =  ag_interget_table[tmp];
    ag_fraction = (a_gain*17)/(16*ag_integer)- 17;
    ag_fraction =  OV_CLIP3(0, 15, ag_fraction);
    //printk(" ag_integer = %d ",ag_integer);
	//printk(" ag_fraction = %d\n", ag_fraction);
    
    switch (ag_integer)
    {
        case 1 :
            //printk("case 1\n");
        	//sc1145_sensor_write_register(0x3610,0x03);
        	//sc1145_sensor_write_register(0x3605,0x0c);
            ag_integer = 0x00;
			a_gain_2x_now = 1;
			a_gain_4x_now = 1;
            break;
        case 2 :
        	//printk("case 2\n");
        	//sc1145_sensor_write_register(0x3610,0x03);
        	//sc1145_sensor_write_register(0x3605,0x0c);
            ag_integer = 0x10;
			a_gain_2x_now = 1;
			a_gain_4x_now = 1;
            break;
        case 4 :
        	//printk("case 4\n");
        	//sc1145_sensor_write_register(0x3610,0x03);
        	//sc1145_sensor_write_register(0x3605,0x0c);
            ag_integer = 0x30;
			a_gain_2x_now = 1;
			a_gain_4x_now = 1;
            break;
        case 8 :
        	//printk("case 8\n");
        	//sc1145_sensor_write_register(0x3610,0x03);
        	//sc1145_sensor_write_register(0x3605,0x0c);
            ag_integer = 0x70;
			a_gain_2x_now = 1;
			a_gain_4x_now = 1;
            break;
        case 16 :
        	//printk("case 16\n");
        	//sc1145_sensor_write_register(0x3610,0x1b);
        	//sc1145_sensor_write_register(0x3605,0x0c);
            ag_integer = 0x70;
			a_gain_2x_now = 2;
			a_gain_4x_now = 1;
            break;
        case 32 :
        	//printk("case 32\n");
       	 	//sc1145_sensor_write_register(0x3610,0x1b);
       	 	//sc1145_sensor_write_register(0x3605,0x3c);
            ag_integer = 0x70;
			a_gain_2x_now = 2;
			a_gain_4x_now = 2;
            break;
        default:
        	//printk("case default\n");
        	//sc1145_sensor_write_register(0x3610,0x03);
        	//sc1145_sensor_write_register(0x3605,0x0c);
            ag_integer = 0x00;
            break;
    }
    //printk("ag_integer=%x ag_fraction=%x a_gain_2x_now=%x, a_gain_4x_now=%x\n",
	//	  ag_integer,ag_fraction,a_gain_2x_now,a_gain_4x_now);
    ag_return = ag_integer|ag_fraction;
	*a_gain_out = ag_return ; 
 
	return ag_return;
}

static int sc1145_cmos_updata_d_gain(const unsigned int d_gain)
{
	//?????????????Ļص?????
	return 0;
}

/*
static int sc1145_updata_2x_4x_a_gain(void)
{
     //a_gain_2x_now = 2;
	// a_gain_4x_now = 2;
	int all_2x_4x_gain=0;
	    a_gain_2x_4x_locked=0;
	 if((a_gain_2x_prev==1)&&(a_gain_2x_now==2))
	 {
		a_gain_2x_4x_locked =AK_TRUE;
		all_2x_4x_gain = a_gain_2x_prev*a_gain_4x_now;
	 }		
	 else if((a_gain_4x_prev==1)&&(a_gain_4x_now==2))
	 {
		a_gain_2x_4x_locked = AK_TRUE;
		all_2x_4x_gain = a_gain_2x_now*a_gain_4x_prev;
	 }
	 else if((a_gain_2x_prev==2)&&(a_gain_2x_now==1))
	 {
		a_gain_2x_4x_locked = AK_TRUE;
		all_2x_4x_gain = a_gain_2x_prev*a_gain_4x_now;
	 }
	 else if((a_gain_4x_prev==2)&&(a_gain_4x_now==1))
	 {
		a_gain_2x_4x_locked = AK_TRUE;
		all_2x_4x_gain = a_gain_2x_now*a_gain_4x_prev;
	 }
	 
	   if(a_gain_2x_4x_locked!=AK_TRUE)
	   {
	     all_2x_4x_gain=a_gain_2x_now*a_gain_4x_now;
	     switch(all_2x_4x_gain)
	     {
		 	case 1:
			sc1145_sensor_write_register(0x3610,0x03);
        	sc1145_sensor_write_register(0x3605,0x0c);
		
			  break;
			case 2:
			sc1145_sensor_write_register(0x3610,0x1b);
        	sc1145_sensor_write_register(0x3605,0x0c);
			
			  break;
			case 4:
			sc1145_sensor_write_register(0x3610,0x1b);
       	 	sc1145_sensor_write_register(0x3605,0x3c);
			
			   break;
			default:
			   break;
	     }
	   
	   }
	   else   //
	   {
	     //all_2x_4x_gain=a_gain_2x_now*a_gain_4x_now;
	     switch(all_2x_4x_gain)
	     {
		 	case 1:
			sc1145_sensor_write_register(0x3610,0x03);
        	sc1145_sensor_write_register(0x3605,0x0c);
			
			  break;
			case 2:
			sc1145_sensor_write_register(0x3610,0x1b);
        	sc1145_sensor_write_register(0x3605,0x0c);
		
			  break;
			case 4:
			sc1145_sensor_write_register(0x3610,0x1b);
       	 	sc1145_sensor_write_register(0x3605,0x3c);
			
			   break;
			default:
			   break;
	     }
	      
	   }

	 a_gain_2x_prev = a_gain_2x_now;
	 a_gain_4x_prev = a_gain_4x_now;
	 return 0;	 
}*/
static int sc1145_updata_2x_4x_a_gain(void)
{
    int   all_2x_4x_gain;
	   {
	     all_2x_4x_gain=a_gain_2x_now*a_gain_4x_now;
	     switch(all_2x_4x_gain)
	     {
		 	case 1:
			sc1145_sensor_write_register(0x3610,0x03);
        	sc1145_sensor_write_register(0x3605,0x0c);
		
			  break;
			case 2:
			sc1145_sensor_write_register(0x3610,0x1b);
        	sc1145_sensor_write_register(0x3605,0x0c);
			
			  break;
			case 4:
			sc1145_sensor_write_register(0x3610,0x1b);
       	 	sc1145_sensor_write_register(0x3605,0x3c);
			
			   break;
			default:
			   break;
	     }
	   
	   }
	   
	 a_gain_2x_prev = a_gain_2x_now;
	 a_gain_4x_prev = a_gain_4x_now;
	 return 0;	 
}
//for SC1145
#define SENSOR_BLC_TOP_VALUE (0x1c)
#define SENSOR_BLC_BOT_VALUE (0x19)
#define SENSOR_AGAIN_ADAPT_STEP (1)
#define SENSOR_MAX_AGAIN (0x7f)
static const unsigned short sensor_gain_map[48] = {
	0x10, 0x11, 0x12, 0x13, 0x14,0x15, 0x16, 0x17, 0x18, 0x19,
	0x1a, 0x1b, 0x1c,0x1d, 0x1e, 0x1f, 0x30, 0x31,
	0x32, 0x33, 0x34, 0x35,0x36, 0x37, 0x38, 0x39,
	0x3a, 0x3b, 0x3c, 0x3d,0x3e, 0x3f, 0x70, 0x71,
	0x72, 0x73, 0x74, 0x75,0x76, 0x77, 0x78, 0x79,
	0x7a, 0x7b, 0x7c, 0x7d,0x7e, 0x7f
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
	BLC_reg = sc1145_sensor_read_register(0x3911);

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
	//printk("limit gain:ret_Again=%d;Again=%d;MaxAgain=%d\n", ret_Again, Again, MaxAgain);
	return ret_Again;
}

#define SKIP_FRAME_NUM	2
static int sc1145_cmos_updata_a_gain(const unsigned int a_gain)
{
	//??????ģ???????Ļص?????
	unsigned short ag_value;
	unsigned int tmp_a_gain;
	unsigned int tmp_d_gain;
	unsigned int tmp_a_gain_out;
	unsigned int tmp_d_gain_out;
	int ret = 0;
	
	g_a_gain  = a_gain;

	tmp_a_gain = a_gain>>4;
	tmp_d_gain = 0;

	//ag_value = cmos_gains_update(isp,isp->aec_param.current_a_gain);
	_cmos_gains_convert(tmp_a_gain, tmp_d_gain,  &tmp_a_gain_out ,&tmp_d_gain_out);
	
	if(a_gain_2x_prev!=a_gain_2x_now || a_gain_4x_now!=a_gain_4x_prev)
		ret = SKIP_FRAME_NUM;
	ag_value = tmp_a_gain_out;
	sc1145_updata_2x_4x_a_gain();
	sc1145_sensor_write_register(0x3e09, Again_limit(tmp_a_gain_out));
	//printk(" 0x3e09 = %d\n", sc1145_sensor_read_register(0x3e09));
	//printk(" 0x3610 = %d\n", sc1145_sensor_read_register(0x3610));
	//printk(" 0x3605 = %d\n", sc1145_sensor_read_register(0x3605));
	//printk(" tmp_a_gain = %d ",tmp_a_gain);
	//printk(" tmp_a_gain_out = %d\n", tmp_a_gain_out);
	//updata_auto_exposure_num++;
	//isp->aec_param.current_exposure_time = tmp_exposure_time;
	return ret;
}


static int sc1145_cmos_updata_exp_time(unsigned int exp_time)
{
    //?????ع?ʱ???Ļص?????
    unsigned char exposure_time_msb;
	unsigned char exposure_time_lsb;
	unsigned char tem;
 	//printk("exp_time=%d ",exp_time);
    exposure_time_msb =(exp_time>>4)&0xff;
    exposure_time_lsb =((exp_time)&0xf)<<4;
    tem=sc1145_sensor_read_register(0x3e02)&0x0f;
    exposure_time_lsb =exposure_time_lsb|tem;
	//printk("msb = %d  ",exposure_time_msb);
	//printk("lsb = %d\n",exposure_time_lsb);
	sc1145_sensor_write_register(0x3e01,exposure_time_msb);
	sc1145_sensor_write_register(0x3e02,exposure_time_lsb);
    return 0;
}

static int sc1145_cmos_update_a_gain_timer(void)
{
#if 0
	#define UPDATE_A_GAIN_TIMER_PERIOD	(5)	//s
	static struct timeval stv = {.tv_sec = 0,};
	struct timeval tv;

	do_gettimeofday(&tv);

	if (tv.tv_sec >= stv.tv_sec + UPDATE_A_GAIN_TIMER_PERIOD) {
		stv.tv_sec = tv.tv_sec;
		sc1145_cmos_updata_a_gain(g_a_gain);
	}

	return 0;
#endif

	#define UPDATE_A_GAIN_TIMER_PERIOD	(5)	//s
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
		sc1145_cmos_updata_a_gain(g_a_gain);
	}

	return 0;
}
