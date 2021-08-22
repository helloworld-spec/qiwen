/**
 * @file camera_h62.c
 * @brief camera driver file
 * Copyright (C) 2011 Anyka (Guangzhou) Microelectronics Technology Co., Ltd
 * @author yang_mengxia 
 * @date 2017-02-21
 * @version 1.0
 * @ref
 */ 
#include "drv_api.h"

#include "drv_gpio.h"

#include "dev_drv.h"
#include "platform_devices.h"

#include "ak_isp_drv.h"
#include "drv_camera.h"

#define SENSOR_PWDN_LEVEL		1 
#define SENSOR_RESET_LEVEL		0
#define SENSOR_I2C_ADDR         0x60 
#define SENSOR_ID               0xa062
#define SENSOR_MCLK             27
#define SENSOR_REGADDR_BYTE     1
#define SENSOR_DATA_BYTE        1
#define MAX_FPS					25
#define SENSOR_OUTPUT_WIDTH		1280
#define SENSOR_OUTPUT_HEIGHT	720
#define SENSOR_BUS_TYPE		BUS_TYPE_RAW

#define OV_MAX(a, b)            (((a) < (b) ) ?  (b) : (a))
#define OV_MIN(a, b)            (((a) > (b) ) ?  (b) : (a))
#define OV_CLIP3(low, high, x)  (OV_MAX(OV_MIN((x), high), low))

static int h62_cmos_updata_d_gain(const unsigned int d_gain);
static int h62_cmos_updata_a_gain(const unsigned int a_gain);
static int h62_cmos_updata_exp_time(unsigned int exp_time);


static int g_fps = MAX_FPS;
static int g_a_gain = 0;
static AK_ISP_SENSOR_CB h62_callback;
static T_CAMERA_FUNCTION_HANDLER h62_handler;
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

static int h62_sensor_read_register(int reg)
{
#if 0
	return sccb_read_data(SENSOR_I2C_ADDR, reg);
#else	
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
	
	ret = dev_ioctl(drv_i2c_id, IO_I2C_BYTE_READ, cmd_data);

	//ret = sccb_read_data3(SENSOR_I2C_ADDR, reg, &data, 1);
	if (-1 != ret)
		return data;
	return 0;
#endif	
}

static int h62_sensor_write_register(int reg, int data)
{
#if 1
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
	
	ret = dev_ioctl(drv_i2c_id, IO_I2C_BYTE_WRITE, cmd_data);
	if(-1 != ret)
		return ucdata;
#else
	return sccb_write_data(SENSOR_I2C_ADDR, reg, &data, 1);
#endif	
}

static void h62_set_poweron(void)
{  
	drv_i2c_id = dev_open(DEV_I2C); //jk+

	printf("GPIO_CAMERA_AVDD:%d,GPIO_CAMERA_RESET:%d\n",GPIO_CAMERA_AVDD,GPIO_CAMERA_RESET);
	//gpio_init();
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

static bool h62_set_poweroff(void)
{
		dev_close(drv_i2c_id);  //jk+
		drv_i2c_id = -1;
#if 0
    unsigned char Reg0x3d = 0x48;
    unsigned char Reg0xc3 = 0x00;
#endif

#if 0
	//sccb software standby mode
    
    h62_sensor_write_register(0x3d, Reg0x3d);
    h62_sensor_write_register(0xc3, Reg0xc3);
#endif

    gpio_set_pin_level(GPIO_CAMERA_AVDD, SENSOR_PWDN_LEVEL);
    gpio_set_pin_dir(GPIO_CAMERA_RESET, GPIO_DIR_INPUT);
	printf("%s\n",__func__);

	return 0;
}
static int h62_init(const AK_ISP_SENSOR_INIT_PARA *para)
{
	int i;
	AK_ISP_SENSOR_REG_INFO *preg_info;
	
	preg_info = para->reg_info;
	for (i = 0; i < para->num; i++) {
//value=h62_sensor_read_register(preg_info->reg_addr);
//printf("before w reg:0x%.x, val:0x%.x, to write:0x%.x\n", preg_info->reg_addr, value, preg_info->value);
		h62_sensor_write_register(preg_info->reg_addr, preg_info->value);
//value=h62_sensor_read_register(preg_info->reg_addr);
//printf("after w reg:0x%.x, val:0x%.x\n", preg_info->reg_addr, value);
		preg_info++;
	}

	return 0;
}

/**
 * @brief get sensor mclk requestion
 * @author yang_mengxia   
 * @date 2017-02-06
 * @param[]
 * @return int mclk unit:MHz
 */
static int h62_get_mclk(void)
{
	return SENSOR_MCLK;
}

static int h62_get_id(void)
{

    unsigned char value;
    unsigned int id;

	printf("==============\r\n");
	get_camera_gpio();
	printf("==============\r\n");
	
	//sccb_set_soft_hard_flag(0);
	//sccb_init(GPIO_I2C_SCL, GPIO_I2C_SDA);

    value = h62_sensor_read_register(0x0a);
    id = value << 8;
    value = h62_sensor_read_register(0x0b);
    id |= value;    

  //  value = h62_sensor_read_register(0x09);
	//printf("%s 0x%x\n", __func__, value);
	printf("%s 0x%x\n", __func__, id);

	if (id != SENSOR_ID)
		goto fail;

	return id;
fail:
    return 0;
}

/**
 * @brief set sensor framerate
 * @author yang_mengxia   
 * @date 2016-02-06
 * @param[in] fps sensor's framerate
 * @return int
 * @retval 0 if successfully
 * @retval others if unsuccessfully
 */
static int h62_set_fps(const int fps)
{
	int ret = 0;

	switch (fps) {
	case 30:
		h62_sensor_write_register(0x23,0x02);
		h62_sensor_write_register(0x22,0xf6);
		break;

	case 25:
		h62_sensor_write_register(0x23,0x03);
		h62_sensor_write_register(0x22,0x8d);
		break;
	
	case 15:
		h62_sensor_write_register(0x23,0x05);
		h62_sensor_write_register(0x22,0xdc);
		break;

	case 13:	//actual: 12.5fps
	case 12:
		h62_sensor_write_register(0x23,0x07);
		h62_sensor_write_register(0x22,0x08);
		break;

	case 10:     // add by john for night 10fps
		h62_sensor_write_register(0x23,0x08);
		h62_sensor_write_register(0x22,0xca);
		break;
	case 8:		//actual: 8.333fps
		h62_sensor_write_register(0x23,0x0a);
		h62_sensor_write_register(0x22,0xf0);
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
 * @author yang_mengxia   
 * @date 2017-02-06
 * @param[]
 * @return int sensor's framerate
 */
static int h62_get_fps(void)
{
	return g_fps;
}

/**
 * @brief get sensor data output bus type
 * @author yang_mengxia   
 * @date 2017-02-06
 * @param[]
 * @return enum sensor_bus_type sensor data type
 */
static enum sensor_bus_type h62_get_bus_type(void)
{
	return SENSOR_BUS_TYPE;
}

/**
 * @brief set sensor's all callback functions
 * @author yang_mengxia   
 * @date 2017-02-06
 * @param[]
 * @return int
 * @retval 0 if successfully
 * @retval others if unsuccessfully
 */
static int camera_h62_reg(void)
{
	h62_callback.sensor_init_func 				= h62_init;
	h62_callback.sensor_read_reg_func			= h62_sensor_read_register;
	h62_callback.sensor_write_reg_func			= h62_sensor_write_register;
	h62_callback.sensor_read_id_func			= h62_get_id;
	h62_callback.sensor_update_a_gain_func		= h62_cmos_updata_a_gain;
	h62_callback.sensor_update_d_gain_func		= h62_cmos_updata_d_gain;
	h62_callback.sensor_updata_exp_time_func	= h62_cmos_updata_exp_time;

	
	h62_handler.isp_sensor_cb		= &h62_callback;
	h62_handler.cam_width			= SENSOR_OUTPUT_WIDTH;
	h62_handler.cam_height			= SENSOR_OUTPUT_HEIGHT;
	h62_handler.cam_mclk			= SENSOR_MCLK;
	h62_handler.cam_open_func		= h62_set_poweron;
	h62_handler.cam_close_func		= h62_set_poweroff;
	h62_handler.cam_read_id_func	= h62_get_id;
	h62_handler.cam_set_framerate_func = h62_set_fps;	
	h62_handler.cam_get_framerate_func = h62_get_fps;

	h62_handler.cam_get_bus_type_func	= h62_get_bus_type;

	h62_handler.sensor_id = SENSOR_ID;

    camera_reg_dev(SENSOR_ID, &h62_handler);
    return 0;
}

#ifdef __CC_ARM
#pragma arm section rwdata = "__initcall_", zidata = "__initcall_"
#endif
module_init(camera_h62_reg)
#ifdef __CC_ARM
#pragma arm section
#endif
///////////////////////////////////////////////////////////////////////////////////////////

static unsigned int h62_cmos_gains_convert(unsigned int a_gain, unsigned int d_gain,
		unsigned int *a_gain_out ,unsigned int *d_gain_out)
{
	//ģ??????????ת???????????㷨??????????ת????sensor????????ʽ
	unsigned int ag_interget_table[17] = {0,  1, 2, 2, 4, 4, 4, 4, 8, 8, 8, 8,
        8, 8, 8, 8, 16};
    unsigned int ag_integer, ag_fraction, tmp;
	unsigned int ag_return;
 
	tmp =  a_gain / 16;
	if(tmp == 0)
	{
		tmp = 1;
		printf("Div ERROR 000!\n");
	}
	//ak_sensor_print("tmp  = %d \n",tmp);
	if(tmp<16)
	{
       tmp =  OV_CLIP3(0, 16, tmp);
       ag_integer =  ag_interget_table[tmp];
	}
	else if ((16<=tmp)&&(tmp<32))
		ag_integer = 16;
	else if ((32<=tmp)&&(tmp<64))
		ag_integer = 32;
	else// if((64<=tmp)&&(tmp<128))
		ag_integer = 64;

	
    ag_fraction = (a_gain / ag_integer) - 16;
    ag_fraction =  OV_CLIP3(0, 15, ag_fraction);
 
    if (((ag_fraction + 16) * ag_integer) < a_gain)
    {
        if (ag_fraction < 15)
        {
            ag_fraction++;
        }
       // else if (ag_integer < 16)
       else if (ag_integer <128)
        {
            tmp++;
			/*
            ag_integer  =  ag_interget_table[tmp];
            ag_fraction = 0;
            */
		    if(tmp<16)
			{
		       tmp =  OV_CLIP3(0, 16, tmp);
		       ag_integer =  ag_interget_table[tmp];
			}
			else if ((16<=tmp)&&(tmp<32))
				ag_integer = 16;
			else if ((32<=tmp)&&(tmp<64))
				ag_integer = 32;
			else if((64<=tmp)&&(tmp<128))
				ag_integer = 64;
				
				ag_fraction = 0;
        }
        else
        {
        }
    }
 
    switch (ag_integer)
    {
        case 1 :
            ag_integer = 0x00;
            break;
        case 2 :
            ag_integer = 0x10;
            break;
        case 4 :
            ag_integer = 0x20;
            break;
        case 8 :
            ag_integer = 0x30;
            break;
		case 16 :
            ag_integer = 0x40;
            break;
        case 32 :
            ag_integer = 0x50;
			break;
		case 64 :
            ag_integer = 0x60;
			break;
		default: 
			ag_integer = 0x00;
            break;
    }

    ag_return = ag_integer|ag_fraction;
	*a_gain_out = ag_return ; 
 
	return ag_return;
}

static int h62_cmos_updata_d_gain(const unsigned int d_gain)
{
	//?????????????Ļص?????
	return 0;
}

static int h62_cmos_updata_a_gain(const unsigned int a_gain)
{
	//??????ģ???????Ļص?????
	unsigned short ag_value;
	unsigned int tmp_a_gain;
	unsigned int tmp_d_gain;
	unsigned int tmp_a_gain_out;
	unsigned int tmp_d_gain_out;

	tmp_a_gain = a_gain>>4;
	tmp_d_gain = 0;

	//ag_value = cmos_gains_update(isp,isp->aec_param.current_a_gain);
	h62_cmos_gains_convert(tmp_a_gain, tmp_d_gain,  &tmp_a_gain_out ,&tmp_d_gain_out);
	ag_value = tmp_a_gain_out;
	//ak_sensor_print("a_gain=%d\n",a_gain);
	h62_sensor_write_register(0x00, tmp_a_gain_out);
	// updata_auto_exposure_num++;
	//isp->aec_param.current_exposure_time = tmp_exposure_time;
	return 0;
}


static int h62_cmos_updata_exp_time(unsigned int exp_time)
{
    //?????ع?ʱ???Ļص?????
    unsigned char exposure_time_msb;
	unsigned char exposure_time_lsb;
//	ak_sensor_print("exp_time=%d\n",exp_time);
    exposure_time_msb =(exp_time>>8)&0xff;
    exposure_time_lsb = exp_time&0xff;
//	ak_sensor_print("msb = %d\n",exposure_time_msb);
//	ak_sensor_print("lsb = %d\n",exposure_time_lsb);
	h62_sensor_write_register(0x02,exposure_time_msb);
	h62_sensor_write_register(0x01,exposure_time_lsb);
//	ak_sensor_print("0x16=%d\n",h62_sensor_read_register(0x16));
    return 0;
}

