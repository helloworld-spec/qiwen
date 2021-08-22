/**
 * @file gpio.c
 * @brief gpio function file
 * This file provides gpio APIs: initialization, set gpio, get gpio,
 * gpio interrupt handler.
 * Copyright (C) 2004 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author tangjianlong
 * @date 2008-01-10
 * @version 1.0
 * @ref anyka technical manual.
 */
#include "anyka_cpu.h"
#include "anyka_types.h" 
#include "drv_api.h"
#include "interrupt.h"
#include "gpio.h"
#include "drv_gpio.h"
#include "sysctl.h"

static bool gpio_interrupt_handler(void);

static T_fGPIO_CALLBACK m_fGPIOCallback[GPIO_NUMBER];

static unsigned long gpio_pin_dir_reg[] = {GPIO_DIR_REG1,       GPIO_DIR_REG2,      GPIO_DIR_REG3};
static unsigned long gpio_pin_in_reg[]  = {GPIO_IN_REG1,        GPIO_IN_REG2,       GPIO_IN_REG3};
static unsigned long gpio_pin_out_reg[] = {GPIO_OUT_REG1,       GPIO_OUT_REG2,      GPIO_OUT_REG3};
static unsigned long gpio_pin_inte_reg[]= {GPIO_INT_EN1,        GPIO_INT_EN2,       GPIO_INT_EN3};
static unsigned long gpio_pin_intp_reg[]= {GPIO_INT_LEVEL_REG1, GPIO_INT_LEVEL_REG2, GPIO_INT_LEVEL_REG3};
static unsigned long gpio_pin_intm_reg[]= {GPIO_INT_MODE_REG1, GPIO_INT_MODE_REG2, GPIO_INT_MODE_REG3};
static unsigned long gpio_pin_ints_reg[]= {GPIO_EDGE_INT_STATUS_REG1, GPIO_EDGE_INT_STATUS_REG2, GPIO_EDGE_INT_STATUS_REG3};
	
static volatile unsigned long gpio_pin_inte[4] = {0};
static volatile unsigned long gpio_pin_intp[4] = {0};
static volatile unsigned long gpio_pin_intm[4] = {0};

static volatile unsigned char  usb_vbus_level = GPIO_LEVEL_LOW;  //special gpio 51

#define USB_DETECT_GPIO     89

/* GPIO int before call back register, must be fatal error*/
static void DummyGPIOCallback(unsigned long pin, unsigned char polarity)
{
    gpio_int_control(pin, 0);
}

unsigned char gpio_assert_legal(unsigned long pin)
{
    if((pin == INVALID_GPIO) || (pin >= GPIO_NUMBER))
    {
        return false;
    }
    else
    {
        return true;
    }
}

/**
 * @brief: Init gpio.
 * @author tangjianlong
 * @date 2008-01-10
 * @return void
 * @retval
 */
void gpio_init( void )
{
    unsigned long pin;

	sysctl_clock(CLOCK_GPIO_ENABLE);
    //clean gpio callback function array.
    for( pin=0; pin<GPIO_NUMBER; pin++ )
    {
        m_fGPIOCallback[ pin ] = DummyGPIOCallback;
    }

    //disable gpio int before enable its interrupt
    gpio_int_disableall();
    
    //enable GPIO interrupt here.
    int_register_irq(INT_VECTOR_GPIO, gpio_interrupt_handler);
    INTR_ENABLE(IRQ_MASK_GPIO_BIT);

}

/**
 * @brief: Set gpio output level
 * @author: tangjianlong
 * @date 2008-01-10
 * @param: pin: gpio pin ID.
 * @param: level: 0 or 1.
 * @return void
 * @retval
 */
void gpio_set_pin_level( unsigned long pin, unsigned char level )
{
    unsigned long index, residual;
    
    if(false == gpio_assert_legal(pin))
    {
        return;
    }

	if (pin == 49) 
		return;
	
    index = pin / 32;
    residual = pin % 32;
	
    irq_mask();
    if(level)
    { 
        *(volatile unsigned long*)gpio_pin_out_reg[index] |= (1 << residual);
	}
    else
    {
        *(volatile unsigned long*)gpio_pin_out_reg[index] &= ~(1 << residual);
    }
	irq_unmask();
}


static unsigned char gpio_get_vbus_level()
{
#if 0
    if(REG32(GPIO_INT_EN3) & (1<<4))
    {
        return usb_vbus_level;
    }

    //As the vbus level cannot get directly, we use another way
    //we enable gpio 51 interrupt, and set to high level trigger
    //then check the interrupt bit, if a interrupt comes, 
    //then vbus is high level, otherwise it's low level

    store_all_int();

    REG32(GPIO_INT_EN3) &= ~(1<<9);
    REG32(GPIO_INT_LEVEL_REG3) &= ~(1 << 4);
    REG32(GPIO_INT_EN3) |= (1<<4);

    us_delay(1);

    if(REG32(INT_SYS_MODULE_REG) & INT_STATUS_GPIO_BIT)
    {
        usb_vbus_level = 1;
    }
    else
    {
        usb_vbus_level = 0;
    }

    REG32(GPIO_INT_EN3) |= (1<<9);
    REG32(GPIO_INT_LEVEL_REG3) |= (1 << 4);
    REG32(GPIO_INT_EN3) &= ~(1<<4);

    restore_all_int();

    return usb_vbus_level;
#endif
}

/**
 * @brief: Get gpio input level
 * @author: tangjianlong
 * @param: pin: gpio pin ID.
 * @date 2008-01-10
 * @return unsigned char
 * @retval: pin level; 1: high; 0: low;
 */
unsigned char gpio_get_pin_level( unsigned long pin )
{
    unsigned long index, level = 0, residual;   

    if(false == gpio_assert_legal(pin))
    {
        return 0xff;
    }

    if (pin == USB_DETECT_GPIO)
    {
        return gpio_get_vbus_level();
    }
	index = pin / 32;
    residual = pin % 32;
	
    irq_mask();
    if(REG32(gpio_pin_in_reg[index]) & (1 << residual))
        level = 1;
    else
        level = 0;
    irq_unmask();
	
    return level;
}

static unsigned char gpio_get_pin_int_status(unsigned long pin)
{
	unsigned long index, status = 0, residual;   

    if(false == gpio_assert_legal(pin))
    {
        return 0xff;
    }

    if (pin == USB_DETECT_GPIO)
    {
        return gpio_get_vbus_level();
    }
	index = pin / 32;
    residual = pin % 32;
	
    irq_mask();
    if(REG32(gpio_pin_ints_reg[index]) & (1 << residual))
        status = 1;
    else
        status = 0;
    irq_unmask();
	
    return status;

}
/**
 * @brief: Set gpio direction
 * @author: tangjianlong
 * @date 2008-01-10
 * @param: pin: gpio pin ID.
 * @param: dir: 0 means input; 1 means output;
 * @return void
 * @retval
 */
void gpio_set_pin_dir( unsigned long pin, unsigned char dir )
{
    unsigned long index, residual, i;
    
    if(false == gpio_assert_legal(pin))
    {
        return;
    }

     //This bit can not be set, for GPIO[49] can only be set in input mode.
	if (pin == 49)
	{
		return;
	}
	
	index = pin / 32;
    residual = pin % 32;
	irq_mask();
    if(dir == 0)//input mode
    {
        *(volatile unsigned long*)gpio_pin_dir_reg[index] &= ~(1 << residual);
    }
    else
	{
        *(volatile unsigned long*)gpio_pin_dir_reg[index] |= (1 << residual);
    }   

	irq_unmask();

}

/**
 * @brief: gpio interrupt control
 * @author: tangjianlong
 * @date 2008-01-10
 * @param: pin: gpio pin ID.
 * @param: enable: 1 means enable interrupt. 0 means disable interrupt.
 * @return void
 * @retval
 */
void gpio_int_control( unsigned long pin, unsigned char enable )
{
    unsigned long index, residual;
    
    if(false == gpio_assert_legal(pin))
    {
        return;
    }
	index = pin / 32;
	residual = pin % 32;

	irq_mask();
    if(enable)
        gpio_pin_inte[index] |= 1 << residual;
    else
        gpio_pin_inte[index] &= ~(1 << residual);
    if(enable)
        *(volatile unsigned long*)gpio_pin_inte_reg[index] |= (1 << residual);
    else
        *(volatile unsigned long*)gpio_pin_inte_reg[index] &= ~(1 << residual);
    irq_unmask();
}


void gpio_int_disableall()
{
    *(volatile unsigned long*)gpio_pin_inte_reg[0] = 0;
    *(volatile unsigned long*)gpio_pin_inte_reg[1] = 0;
    *(volatile unsigned long*)gpio_pin_inte_reg[2] = 0;
}

void gpio_int_restoreall()
{
    *(volatile unsigned long*)gpio_pin_inte_reg[0] = gpio_pin_inte[0];
    *(volatile unsigned long*)gpio_pin_inte_reg[1] = gpio_pin_inte[1];
    *(volatile unsigned long*)gpio_pin_inte_reg[2] = gpio_pin_inte[2];
}
/**
 * @brief: set gpio interrupt mode
 * @author: jiankui
 * @date 2016-08-17
 * @param: pin: gpio pin ID.
 * @param: mode: 1 means edge triggered interrupt. 0 means level triggered interrupt.
 * @return void
 * @retval
 */
void gpio_set_int_mode( unsigned long pin, unsigned char mode)
{
    unsigned long index,  residual;
    
    if(false == gpio_assert_legal(pin))
    {
        return;
    }
    
    irq_mask();
    index = pin / 32;
    residual = pin % 32;
	
	if(mode)   //edge triggered interrupt
		gpio_pin_intm[index] |= (1 << residual); 
	else
		gpio_pin_intm[index] &= ~(1 << residual);

    if (GPIO_LEVEL_INTERRUPT == mode)
        *(volatile unsigned long*)gpio_pin_intm_reg[index] &= ~(1 << residual);
    else
        *(volatile unsigned long*)gpio_pin_intm_reg[index] |= (1 << residual);
    irq_unmask();
}
/**
 * @brief: set gpio interrupt mode
 * @author: jiankui
 * @date 2016-08-17
 * @param: pin: gpio pin ID.
 * @param: polarity: (mode=level)0 means low triggered. 1 means high triggered.(else)0 means falling triggered.1 means rising triggered.
 * @return void
 * @retval
 */

void gpio_set_int_p( unsigned long pin, unsigned char polarity )
{
    unsigned long index,  residual;
    
    if(false == gpio_assert_legal(pin))
    {
        return;
    }

    irq_mask();
    index = pin / 32;
    residual = pin % 32;

    if(polarity)   //high level active
        gpio_pin_intp[index] |= (1 << residual); 
    else
        gpio_pin_intp[index] &= ~(1 << residual);

    if(polarity)
        *(volatile unsigned long*)gpio_pin_intp_reg[index] &= ~(1 << residual);
    else
        *(volatile unsigned long*)gpio_pin_intp_reg[index] |= (1 << residual);
    irq_unmask();
}
    

/**
 * @brief: Register one gpio interrupt callback function.
 * @author: tangjianlong
 * @date 2008-01-10
 * @param: pin: gpio pin ID.
 * @param: polarity: 1 means active high interrupt. 0 means active low interrupt.
 * @param: enable: Initial interrupt state--enable or disable.
 * @param: callback: gpio interrupt callback function.
 * @return void
 * @retval
 */
void gpio_register_int_callback( unsigned long pin, unsigned char polarity, unsigned char enable, T_fGPIO_CALLBACK callback )
{
    if((false == gpio_assert_legal(pin)) || (NULL == callback))
    {
        akprintf(C1, M_DRVSYS, "gpio_register_int_callback param error\n");
        return;
    }
    m_fGPIOCallback[pin] = callback;
    gpio_set_int_p(pin, polarity);
    gpio_int_control( pin, enable);
	
}

static bool gpio_interrupt_handler(void)
{       
    unsigned long i, pin, inte, intp, intm;
    unsigned long index, residual;   
    
    for(i = 0; i <= USB_DETECT_GPIO; i++)
    {
        index = i / 32;
        residual = i % 32;

        inte = (gpio_pin_inte[index] >> residual) & 0x1;
        intp = (gpio_pin_intp[index] >> residual) & 0x1;
		intm = (gpio_pin_intm[index] >> residual) & 0x1;
        if(inte)
        {
        	if(intm)
        	{
				if(gpio_get_pin_int_status(i)) //read gpio stauts, and clean gpio int stauts
				{
					goto do_irq;
				}
			}
			else
			{
				//gpio 51 in sundance3 is a vinner gpio for usb vbus,
				//we cannot get the pin level, we use edge trigger interrupt		  
				if(gpio_get_pin_level(i) == intp)
				{
					goto do_irq;
				}

			}
            
        }
    }

    if (i > USB_DETECT_GPIO)
    {
        //akprintf(C3, M_DRVSYS, "gpio jitter!\n");
        return true;
    }

do_irq: 
    //pin number
    pin = i;       
    m_fGPIOCallback[pin](pin, intp);

    return true;
}


