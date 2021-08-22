#include <common.h>
#include <fdtdec.h>
#include <i2c.h>
#include <asm/gpio.h>

/** Use this flag to set I2C transmit freq, if I2C_DEFAULT_DELAY_UNIT is become more, the default freq is become less. */
#define  I2C_DEFAULT_DELAY_UNIT        100

static unsigned long I2C_SCL = 27;    /* I2C serial interface clock input */
static unsigned long I2C_SDA = 28;    /* I2C serial interface data I/O */


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


static void i2c_delay(unsigned long time);
static void i2c_begin(void);
static void i2c_end(void);
static void i2c_write_ask(unsigned char flag);
static int i2c_read_ack(void);
static unsigned char   i2c_read_byte(void);
static int i2c_write_byte(unsigned char data);

static unsigned long i2c_delay_unit = I2C_DEFAULT_DELAY_UNIT;


int i2c_write_bytes(unsigned char chip, unsigned char *addr, unsigned long addrlen, unsigned char *data, unsigned long size)
{
    unsigned long i;     
      
    // start transmite
    i2c_begin();

	if(i2c_write_byte(chip) < 0)
	{
		
        i2c_end();
        return -1;		
	}
	
    // write address to I2C device, first is device address, second is the register address
    for (i=0; i<addrlen; i++)
    {
        if (i2c_write_byte(addr[i]) < 0)
        {
            i2c_end();
            return -1;
        }
    }   
    
    // transmite data
    for (i=0; i<size; i++)
    {
        if (i2c_write_byte(data[i]) < 0)
        {
            i2c_end();
            return -1;
        }
    }
    
    // stop transmited
    i2c_end();
    
    return 0;
}


int i2c_read_bytes(unsigned char chip, unsigned char *addr, unsigned long addrlen, unsigned char *data, unsigned long size)
{
    unsigned long i;     
 
    // start transmite
    i2c_begin();

	if(i2c_write_byte(chip) < 0)
	{
        i2c_end();
        return -1;		
	}

	//printf("i2c_write_byte(chip) success! \n");
	
    // write address to I2C device, first is device address, second is the register address
    for (i=0; i<addrlen; i++)
    {
        if (i2c_write_byte(addr[i]) < 0)
        {
            i2c_end();
            return -1;
        }
    }

    i2c_end();
    
    // restart transmite
    i2c_begin();
    
    // send message to I2C device to transmite data
    if (i2c_write_byte((unsigned char)(chip | 1)) < 0)
    {
        i2c_end();
        return -1;
    }
    
    // transmite datas
    for(i=0; i<size; i++)
    {
        data[i] = i2c_read_byte();
        (i<size-1) ? i2c_write_ask(0) : i2c_write_ask(1);
    }
    
    // stop transmite
    i2c_end();
    
    return 0;
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
static int i2c_write_byte(unsigned char data)
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


void i2c_init(int speed, int slaveadd)
{
	
  	gpio_set_pin_as_gpio(I2C_SCL);
    gpio_set_pin_as_gpio(I2C_SDA);
	
	//disable pullup gpio
	gpio_set_pull_up_r(I2C_SCL, 0);
	gpio_set_pull_up_r(I2C_SDA, 0);
	
    init_i2c_pin(I2C_SCL, I2C_SDA);
	
    i2c_delay_unit = I2C_DEFAULT_DELAY_UNIT;
}



void i2c_release(unsigned long pin_scl, unsigned long pin_sda)
{    
    release_i2c_pin(pin_scl, pin_sda);
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
    while(time--) 
    {   
        ;
    }
}


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
 * @retval false: -1
 * @retval true: 0
 */
static int i2c_read_ack(void)
{   
    int ret;
    set_i2c_data_in(I2C_SDA);
    i2c_delay(i2c_delay_unit << 3);
    set_i2c_pin(I2C_SCL);
    i2c_delay(i2c_delay_unit << 2);
    if (!get_i2c_pin(I2C_SDA))
    {
        ret = 0;
    }
    else
    {
        ret = -1;
    }

    i2c_delay(i2c_delay_unit << 2);
    clr_i2c_pin(I2C_SCL);
    i2c_delay(i2c_delay_unit << 2);

    set_i2c_data_out(I2C_SDA);
    
    i2c_delay(i2c_delay_unit << 2); 
    return ret;
}


int i2c_probe(uchar chip)
{
	unsigned char buf[1];
	int ret;
	buf[0] = 0;
	//i2c_init(0,0);

	ret = i2c_read(chip, 0, 0, buf, 1);
	
}

int i2c_write(uchar chip, uint addr, int alen, uchar *buffer, int len)
{
	unsigned char xaddr[2];

	if (alen > 2) {
		debug("I2C write: addr len %d not supported\n", alen);
		return -1;
	}

	if (alen > 0) {
		xaddr[0] = (addr >> 8) & 0xFF;
		xaddr[1] = addr & 0xFF;
	}
	
	return i2c_write_bytes(chip << 1, xaddr, alen, buffer, len);
}


int i2c_read(uchar chip, uint addr, int alen, uchar *buffer, int len)
{
	
	unsigned char xaddr[2];

	if (alen > 2) {
		printf("I2C read: addr len %d not supported\n", alen);
		return -1;
	}
	
	if (alen > 0) {
		xaddr[0] = (addr >> 8) & 0xFF;
		xaddr[1] = addr & 0xFF;
	}

	return i2c_read_bytes(chip << 1, xaddr, alen, buffer, len);

}

