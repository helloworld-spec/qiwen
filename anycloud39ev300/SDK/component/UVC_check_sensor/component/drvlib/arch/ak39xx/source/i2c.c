/**
 * @file i2c.c
 * @brief I2C interface driver, define I2C interface APIs.
 *
 * This file provides I2C APIs: I2C initialization, write data to I2C & read data from I2C.
 * Copyright (C) 2004 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author Guanghua Zhang
 * @date 2004-09-21
 * @version 1.0
 * @ref AK3210M technical manual.
 */
#include "anyka_cpu.h"
#include "anyka_types.h"
#include "drv_api.h"
#include "drv_gpio.h"
#include "drv_module.h"


#ifdef OS_ANYKA

/** Use this flag to set I2C transmit freq, if I2C_DEFAULT_DELAY_UNIT is become more, the default freq is become less. */
#define  I2C_DEFAULT_DELAY_UNIT        24

static unsigned long I2C_SCL = INVALID_GPIO;    /* I2C serial interface clock input */
static unsigned long I2C_SDA = INVALID_GPIO;    /* I2C serial interface data I/O */

#define init_i2c_pin(scl, sda) \
    do{ \
        gpio_set_pin_level(scl, GPIO_LEVEL_HIGH); \
        gpio_set_pin_level(sda, GPIO_LEVEL_HIGH); \
		gpio_set_pin_dir(scl, GPIO_DIR_OUTPUT); \
        gpio_set_pin_dir(sda, GPIO_DIR_OUTPUT); \
    }while(0);

#define release_i2c_pin(scl, sda) \
    do{ \
        gpio_set_pin_level(scl, GPIO_LEVEL_LOW); \
        gpio_set_pin_level(sda, GPIO_LEVEL_LOW); \
        gpio_set_pin_dir(scl, GPIO_DIR_OUTPUT); \
        gpio_set_pin_dir(sda, GPIO_DIR_OUTPUT); \
    }while(0);

    
#define set_i2c_pin(pin) \
    gpio_set_pin_level(pin, GPIO_LEVEL_HIGH)

#define clr_i2c_pin(pin) \
    gpio_set_pin_level(pin, GPIO_LEVEL_LOW)

#define get_i2c_pin(pin) \
    gpio_get_pin_level(pin)

#define set_i2c_data_in(pin) \
    gpio_set_pin_dir(pin, GPIO_DIR_INPUT)

#define set_i2c_data_out(pin) \
    gpio_set_pin_dir(pin, GPIO_DIR_OUTPUT)
    
#if 0    
static void set_i2c_pin(unsigned long pin);
static void clr_i2c_pin(unsigned long pin);
static unsigned char   get_i2c_pin(unsigned long pin);
#endif    
static void i2c_delay(unsigned long time);

static void i2c_begin(void);
static void i2c_end(void);
static void i2c_write_ask(unsigned char flag);
static bool i2c_read_ack(void);
static unsigned char   i2c_read_byte(void);
static bool i2c_write_byte(unsigned char data);

static unsigned long i2c_delay_unit = I2C_DEFAULT_DELAY_UNIT;


bool i2c_write_bytes(unsigned char *addr, unsigned long addrlen, unsigned char *data, unsigned long size)
{
    unsigned long i;     
      
    // start transmite
    i2c_begin();
    
    // write address to I2C device, first is device address, second is the register address
    for (i=0; i<addrlen; i++)
    {
        if (!i2c_write_byte(addr[i]))
        {
            i2c_end();
            return false;
        }
    }   
    
    // transmite data
    for (i=0; i<size; i++)
    {
        if (!i2c_write_byte(data[i]))
        {
            i2c_end();
            return false;
        }
    }
    
    // stop transmited
    i2c_end();
    
    return true;
}

bool i2c_read_bytes(unsigned char *addr, unsigned long addrlen, unsigned char *data, unsigned long size)
{
    unsigned long i;     
 
    // start transmite
    i2c_begin();
    
    // write address to I2C device, first is device address, second is the register address
    for (i=0; i<addrlen; i++)
    {
        if (!i2c_write_byte(addr[i]))
        {
            i2c_end();
            return false;
        }
    }

    i2c_end();
    
    // restart transmite
    i2c_begin();
    
    // send message to I2C device to transmite data
    if (!i2c_write_byte((unsigned char)(addr[0] | 1)))
    {
        i2c_end();
        return false;
    }
    
    // transmite datas
    for(i=0; i<size; i++)
    {
        data[i] = i2c_read_byte();
        (i<size-1) ? i2c_write_ask(0) : i2c_write_ask(1);
    }
    
    // stop transmite
    i2c_end();
    
    return true;
}

unsigned long i2c_set_cycle_delay(unsigned long delay)
{
    i2c_delay_unit = (delay >= 1) ? delay : 1;
    
    return i2c_delay_unit;
}

/**
 * @brief receive one byte from I2C interface function
 * receive one byte data from I2C bus
 * @author Junhua Zhao
 * @date 2005-04-05
 * @param void
 * @return unsigned char: received the data
 * @retval
 */
static unsigned char i2c_read_byte(void)
{
    unsigned char i;
    unsigned char ret = 0;

    set_i2c_data_in(I2C_SDA);
    
    for (i=0; i<8; i++)
    {           
        i2c_delay(i2c_delay_unit << 2);
        set_i2c_pin(I2C_SCL);   
        i2c_delay(i2c_delay_unit << 2);
        ret = ret<<1;
        if (get_i2c_pin(I2C_SDA))
            ret |= 1;
        i2c_delay(i2c_delay_unit << 2);
        clr_i2c_pin(I2C_SCL);
        i2c_delay(i2c_delay_unit << 1);
        if (i==7)
        {
            set_i2c_data_out(I2C_SDA);
        }
        i2c_delay(i2c_delay_unit << 1);
    }       

    return ret;
}

/**
 * @brief write one byte to I2C interface function
 * write one byte data to I2C bus
 * @author Junhua Zhao
 * @date 2005-04-05
 * @param unsigned char data: send the data
 * @return bool: return write success or failed
 * @retval false: operate failed
 * @retval true: operate success
 */
static bool i2c_write_byte(unsigned char data)
{
    unsigned char i;

    for (i=0; i<8; i++)
    {
        i2c_delay(i2c_delay_unit << 2);
        if (data & 0x80)
            set_i2c_pin(I2C_SDA);
        else
            clr_i2c_pin(I2C_SDA);
        data <<= 1;

        i2c_delay(i2c_delay_unit << 2);
        set_i2c_pin(I2C_SCL);
        i2c_delay(i2c_delay_unit << 3);
        clr_i2c_pin(I2C_SCL);       
    }   
    
    return i2c_read_ack();
}

/**
 * @brief I2C interface initialize function
 * setup I2C interface
 * @author Guanghua Zhang
 * @date 2004-09-21
 * @param void
 * @return void
 * @retval
 */
void i2c_init(unsigned long pin_scl, unsigned long pin_sda)
{
    I2C_SCL = pin_scl;
    I2C_SDA = pin_sda;
	
	DrvModule_Protect(DRV_MODULE_SDMMC); 
	
  	gpio_set_pin_as_gpio(pin_scl);
    gpio_set_pin_as_gpio(pin_sda);
	
	//disable pullup gpio
	gpio_set_pull_up_r(pin_scl, 0);
	gpio_set_pull_up_r(pin_sda, 0);
	
    init_i2c_pin(I2C_SCL, I2C_SDA);
	
    DrvModule_UnProtect(DRV_MODULE_SDMMC); 
	
    i2c_delay_unit = I2C_DEFAULT_DELAY_UNIT;
}

void i2c_release(unsigned long pin_scl, unsigned long pin_sda)
{
    DrvModule_Protect(DRV_MODULE_SDMMC); 
    
    release_i2c_pin(pin_scl, pin_sda);

    DrvModule_UnProtect(DRV_MODULE_SDMMC); 
    
    i2c_delay_unit = I2C_DEFAULT_DELAY_UNIT;
}

/**
 * @brief delay function
 * delay the time
 * @author Guanghua Zhang
 * @date 2004-09-21
 * @param unsigned long time: delay time
 * @return void
 * @retval
 */
static void i2c_delay(unsigned long time)
{
    asm_loop(time);
}

#if 0 //replace by macro definition
static void init_i2c_pin(unsigned long pin_scl, unsigned long pin_sda)
{
    REG32(GPIO_DIR_REG1) |= pin_scl;
    REG32(GPIO_DIR_REG1) |= pin_sda;
    
    REG32(GPIO_OUT_REG1) |= pin_scl;
    REG32(GPIO_OUT_REG1) |= pin_sda;

}

/**
 * @brief set  I2C input function
 * set I2C input: 1
 * @author Guanghua Zhang
 * @date 2004-09-21
 * @param unsigned long pin: pin number
 * @return void
 * @retval
 */
static void set_i2c_pin(unsigned long pin)
{
    REG32(GPIO_OUT_REG1) |= pin;
}

/**
 * @brief clear I2C input function
 * set I2C input: 0
 * @author Guanghua Zhang
 * @date 2004-09-20
 * @param unsigned long pin: pin number
 * @return void
 * @retval
 */
static void clr_i2c_pin(unsigned long pin)
{
    REG32(GPIO_OUT_REG1) &= (~pin);
}

/**
 * @brief get I2C output function
 * get I2C output data
 * @author Guanghua Zhang
 * @date 2004-09-20
 * @param unsigned long pin: pin number
 * @return unsigned char: get I2C output data
 * @retval
 */
static unsigned char get_i2c_pin(unsigned long pin)
{
    unsigned char ret;
    unsigned long value;

    value = REG32(GPIO_IN_REG1);
    if ((value & pin) == 0)
        ret = 0;
    else
        ret = 1;

    return ret;
}

/**
 * @brief set I2C input function
 * set I2C input: 0
 * @author Guanghua Zhang
 * @date 2004-09-20
 * @param unsigned long pin: pin number
 * @return void
 * @retval
 */
static void set_i2c_data_in()
{
    REG32(GPIO_DIR_REG1) &= (~I2C_SDA);
}

/**
 * @brief set I2C output function
 * set I2C input: 0
 * @author Guanghua Zhang
 * @date 2004-09-20
 * @param unsigned long pin: pin number
 * @return void
 * @retval
 */
static void set_i2c_data_out()
{
    REG32(GPIO_DIR_REG1) |= I2C_SDA;
}
#endif//if 0

/**
 * @brief I2C interface start function
 * start I2C transmit
 * @author Junhua Zhao
 * @date 2004-04-05
 * @param void
 * @return void
 * @retval
 */
static void i2c_begin(void)
{
	DrvModule_Protect(DRV_MODULE_SDMMC); 

    gpio_set_pin_as_gpio(I2C_SCL);
    gpio_set_pin_as_gpio(I2C_SDA);
    i2c_delay(i2c_delay_unit << 2);
    set_i2c_pin(I2C_SDA);   
    i2c_delay(i2c_delay_unit << 2);
    set_i2c_pin(I2C_SCL);
    i2c_delay(i2c_delay_unit << 3);
    clr_i2c_pin(I2C_SDA);   
    i2c_delay(i2c_delay_unit << 3);
    clr_i2c_pin(I2C_SCL);
    i2c_delay(i2c_delay_unit << 4);
}

/**
 * @brief I2C interface stop function
 * stop I2C transmit
 * @author Junhua Zhao
 * @date 2004-05-04
 * @param void
 * @return void
 * @retval
 */
static void i2c_end(void)
{
    i2c_delay(i2c_delay_unit << 2);
    clr_i2c_pin(I2C_SDA);
    i2c_delay(i2c_delay_unit << 2);
    set_i2c_pin(I2C_SCL);
    i2c_delay(i2c_delay_unit << 3);
    set_i2c_pin(I2C_SDA);
    i2c_delay(i2c_delay_unit << 4);
	
	DrvModule_UnProtect(DRV_MODULE_SDMMC); 
}

/**
 * @brief I2C interface send asknowlege function
 * send a asknowlege to I2C bus
 * @author Junhua Zhao
 * @date 2005-04-05
 * @param unsigned char
 *   0:send bit 0
 *   not 0:send bit 1
 * @return void
 * @retval
 */
static void i2c_write_ask(unsigned char flag)
{
    if(flag)
        set_i2c_pin(I2C_SDA);
    else
        clr_i2c_pin(I2C_SDA);
    i2c_delay(i2c_delay_unit << 2);
    set_i2c_pin(I2C_SCL);
    i2c_delay(i2c_delay_unit << 3);
    clr_i2c_pin(I2C_SCL);
    i2c_delay(i2c_delay_unit << 2);
    set_i2c_pin(I2C_SDA);
    i2c_delay(i2c_delay_unit << 2);
}

/**
 * @brief I2C receive anknowlege
 * receive anknowlege from i2c bus
 * @author Junhua Zhao
 * @date 2005-04-05
 * @param void
 * @return bool: return received anknowlege bit
 * @retval false: 0
 * @retval true: 1
 */
static bool i2c_read_ack(void)
{   
    bool ret;
    set_i2c_data_in(I2C_SDA);
    i2c_delay(i2c_delay_unit << 3);
    set_i2c_pin(I2C_SCL);
    i2c_delay(i2c_delay_unit << 2);
    if (!get_i2c_pin(I2C_SDA))
    {
        ret = true;
    }
    else
    {
        ret = false;
    }

    i2c_delay(i2c_delay_unit << 2);
    clr_i2c_pin(I2C_SCL);
    i2c_delay(i2c_delay_unit << 2);

    set_i2c_data_out(I2C_SDA);
    
    i2c_delay(i2c_delay_unit << 2); 
    return ret;
}

bool i2c_write_data(unsigned char daddr, unsigned char raddr, unsigned char *data, unsigned long size)
{
    unsigned char addr[2];
    
    addr[0] = daddr;
    addr[1] = raddr;
    
    return i2c_write_bytes(addr, 2, data, size);
}

bool i2c_write_data2(unsigned char daddr, unsigned short raddr, unsigned char *data, unsigned long size)
{
    unsigned char addr[3];

    addr[0] = daddr;
    addr[1] = (unsigned char)(raddr >> 8);   //hight 8bit
    addr[2] = (unsigned char)(raddr);	      //low 8bit

    return i2c_write_bytes(addr, 3, data, size);
}

bool i2c_write_data3(unsigned char daddr, unsigned short raddr, unsigned short *data, unsigned long size)
{
    unsigned long i;
    unsigned char high_8bit,low_8bit;

    high_8bit = (unsigned char)(raddr >> 8);   //hight 8bit
    low_8bit = (unsigned char)(raddr);            //low 8bit

    i2c_begin();
    if (!i2c_write_byte(daddr))
    {
        i2c_end();
        return false;
    }
    if (!i2c_write_byte(high_8bit))
    {
        i2c_end();
        return false;
    }
    if (!i2c_write_byte(low_8bit))
    {
        i2c_end();
        return false;
    }

    for(i=0; i<size; i++)
    {
        low_8bit = (unsigned char)(*data);
        high_8bit = (unsigned char)((*data) >> 8);

        if (!i2c_write_byte(high_8bit))
        {
            i2c_end();
            return false;
        }
        if (!i2c_write_byte(low_8bit ))
        {
            i2c_end();
            return false;
        }		
        data++;
    }
    i2c_end();

    return true;
}

bool i2c_write_data4(unsigned char daddr, unsigned char *data, unsigned long size)
{ 
    return i2c_write_bytes(&daddr, 1, data, size);
}

bool i2c_read_data(unsigned char daddr, unsigned char raddr, unsigned char *data, unsigned long size)
{
    unsigned char addr[2];
    
    addr[0] = daddr;
    addr[1] = raddr;
    
    return i2c_read_bytes(addr, 2, data, size);
}

bool i2c_read_data2(unsigned char daddr, unsigned short raddr, unsigned char *data, unsigned long size)
{
    unsigned char addr[3];

    addr[0] = daddr;
    addr[1] = (unsigned char)(raddr >> 8);   //hight 8bit
    addr[2] = (unsigned char)(raddr);	    //low 8bit

    return i2c_read_bytes(addr, 3, data, size);
}

bool i2c_read_data3(unsigned char daddr, unsigned short raddr, unsigned short *data, unsigned long size)
{
    unsigned long i;
    unsigned char high_8bit,low_8bit;

    high_8bit = (unsigned char)(raddr >> 8);   //hight 8bit
    low_8bit = (unsigned char)(raddr);            //low 8bit

    i2c_begin();
    if (!i2c_write_byte(daddr))
    {
        i2c_end();
        return false;
    }
    if (!i2c_write_byte(high_8bit))
    {
        i2c_end();
        return false;
    }
    if (!i2c_write_byte(low_8bit))
    {
        i2c_end();
        return false;
    }

    i2c_begin();

    if (!i2c_write_byte((unsigned char)(daddr | 1)))
    {
        i2c_end();
        return false;
    }
    for(i=0; i<size; i++)
    {
        high_8bit = i2c_read_byte();
        i2c_write_ask(0);		
        low_8bit = i2c_read_byte();		
        (i<size-1)?i2c_write_ask(0):i2c_write_ask(1);

        *data = (unsigned short)(high_8bit) << 8 | low_8bit;
        data++;
    }

    i2c_end();

    return true;
}

bool i2c_read_data4(unsigned char daddr, unsigned char *data, unsigned long size)
{  
    return i2c_read_bytes(&daddr, 1, data, size);
}

#endif

/* end of file */
