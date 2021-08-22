/**
 * @FILENAME: gpio_cfg.c
 * @BRIEF gpio configuartion driver file
 * Copyright (C) 2008 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @AUTHOR liaozhijun
 * @DATE 2008-06-17
 * @VERSION 1.0
 * @REF
 */
#include "anyka_cpu.h"
#include "anyka_types.h" 
#include "drv_api.h"
#include "gpio.h"
#include "drv_gpio.h"

#define     LINE_ITEM                   6
#define     PIN_ATTE_LINE               6
#define     GPIO_ATTTR_FIXED_1          1
#define     GPIO_ATTTR_FIXED_0          0
#define     GPIO_ATTR_UNSUPPORTED       0xffff
#define     MADD_A                      200
#define     MDAT_A                      201
#define     END_FLAG                    0xff
#define     SHARE_PIN_CONFIG_END        0xffffffff
typedef enum
{
    PULLUP = 0,
    PULLDOWN,
    PULLUPDOWN,
    UNDEFINED
}T_GPIO_TYPE;


//gpio pullup/pulldown reg
static const unsigned long gpio_pull_set_reg[] = {
    GPIO_PULLUPDOWN_REG1, GPIO_PULLUPDOWN_REG2, 
    GPIO_PULLUPDOWN_REG3, GPIO_PULLUPDOWN_REG4
};

//gpio io control reg
static const unsigned long gpio_io_control_reg[] = {
    GPIO_IO_CONTROL_REG1
};

//gpio sharepin config reg
static const unsigned long gpio_sharepin_con_reg[] = {
    GPIO_SHAREPIN_CONTROL1, GPIO_SHAREPIN_CONTROL2,
	GPIO_SHAREPIN_CONTROL3, GPIO_SHAREPIN_CONTROL4
};

//special share pin config fore module
T_SHARE_CFG_FUNC_MODULE special_share_cfg_module[] = {
	{ePIN_AS_PWM1_S0, (1<<4) , (1<<4) , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00},
	{ePIN_AS_PWM1_S1, (1<<12) , (1<<12) , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00},
	{ePIN_AS_PWM3_S0, 0x00 , 0x00 , 0x00 , 0x00 , (3<<22) , (3<<22) , 0x00 , 0x00},
	{ePIN_AS_PWM3_S1, (1<<19) , (1<<19) , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00},

	{ePIN_AS_I2S_S0, (1<<24)|(1<<14)|(1<<15)|(1<<16)|(1<<17) , (0<<24)|(1<<14)|(1<<15)|(1<<16)|(1<<17) , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00},
	{ePIN_AS_I2S_S1, (1<<24) , (1<<24) , 0x00 , 0x00 , (3<<0)|(3<<2)|(3<<5)|(3<<8)|(3<<10) , (2<<0)|(2<<2)|(2<<5)|(2<<8)|(2<<10) , 0x00 , 0x00},

	{ePIN_AS_SPI0_S0, (1<<25) , (0<<25) , 0x00 , 0x00 , 0x00 , 0x00 , (1<<0)|(1<<1)|(3<<14)|(3<<16)|(3<<18) , (1<<0)|(1<<1)|(3<<14)|(3<<16)|(3<<18)},
	{ePIN_AS_SPI0_S1, (1<<25) , (1<<25) , 0x00 , 0x00 , 0x00 , 0x00 , (1<<0)|(1<<1)|(3<<8)|(3<<10)|(3<<12) , (1<<0)|(1<<1)|(3<<8)|(3<<10)|(3<<12)},

	{ePIN_AS_SPI1_S0, (1<<26) , (0<<26) , 0x00 , 0x00 , 0x00 , 0x00 , (3<<2)|(3<<4)|(3<<22)|(3<<24)|(3<<26) , (3<<2)|(3<<4)|(3<<22)|(3<<24)|(3<<26)},
	{ePIN_AS_SPI1_S1, (1<<26) , (1<<26) , 0x00 , 0x00 , (3<<2)|(3<<5)|(3<<8)|(3<<10)|(3<<16)|(3<<18) , (3<<2)|(3<<5)|(3<<8)|(3<<10)|(3<<16)|(3<<18) , 0x00 , 0x00},

	{ePIN_AS_SDIO_S0, (1<<27) , (0<<27) , 0x00 , 0x00 , 0x00 , 0x00 , (1<<20)|(1<<21)|(3<<22)|(3<<24)|(3<<26) , (1<<20)|(1<<21)|(1<<22)|(1<<24)|(1<<26)},
	#if 0
	{ePIN_AS_SDIO_S1, (1<<27) , (1<<27) , 0x00 , 0x00 , (3<<16)|(3<<18)|(3<<22)|(3<<24) , (2<<16)|(2<<18)|(2<<22)|(2<<24) , (1<<20)|(1<<21) , (1<<20)|(1<<21)},
	#else
	{ePIN_AS_SDIO_S1, (1<<27) , (1<<27) , 0x00 , 0x00 , (3<<16)|(3<<18)|(3<<22)|(3<<24) , (2<<24) , (1<<20)|(1<<21) , (1<<20)|(1<<21)},
	#endif
};
//share pin config fore module
T_SHARE_CFG_FUNC_MODULE share_cfg_module[] = 
{
	{ePIN_AS_JTAG , (1<<0)|(3<<5) , (1<<0)|(2<<5) ,(3<<4)|(3<<6)|(3<<8)|(3<<10) , (3<<4)|(3<<6)|(3<<8)|(3<<10),0x00,0x00,0x00,0x00},
	{ePIN_AS_UART0 , (1<<1)|(1<<2) , (1<<1)|(1<<2) , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00},
	{ePIN_AS_UART1 , 0x00 , 0x00 , (3<<4)|(3<<6), (2<<4)|(2<<6), 0x00 , 0x00 , 0x00 , 0x00},
	{ePIN_AS_PWM0 , (1<<11) , (1<<11) , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00},
	{ePIN_AS_PWM1 , (1<<4) , (1<<4) , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00},
	{ePIN_AS_PWM3, 0x00 , 0x00 , 0x00 , 0x00 , (3<<22) , (3<<22) , 0x00 , 0x00},
	{ePIN_AS_CAMERA , 0x00 , 0x00 , 0xFFFFF , 0x00 , 0x00 , 0x00 , 0x00 , 0x00},
	{ePIN_AS_MAC , 0x00 , 0x00 , 0x00 , 0x00 , 0x7FFFFFF , (1<<0)|(1<<2)|(1<<4)|(1<<5)|(1<<8)|(1<<10)|(1<<12)|(1<<13)|(1<<14)|(1<<16)|(1<<18)|(1<<20)|(1<<21)|(1<<22)|(1<<24)|(1<<7)|(1<<15)|(1<<26) , 0x00 , 0x00},
	{ePIN_AS_I2S , (1<<24)|(1<<14)|(1<<15)|(1<<16)|(1<<17) , (0<<24)|(1<<14)|(1<<15)|(1<<16)|(1<<17) , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00},
	{ePIN_AS_SPI1, (1<<26) , (0<<26) , 0x00 , 0x00 , 0x00 , 0x00 , (3<<2)|(3<<4)|(3<<22)|(3<<24)|(3<<26) , (3<<2)|(3<<4)|(3<<22)|(3<<24)|(3<<26)},
	{ePIN_AS_SDIO , (1<<27) , (0<<27) , 0x00 , 0x00 , 0x00 , 0x00 , (1<<20)|(1<<21)|(3<<22)|(3<<24)|(3<<26) , (1<<20)|(1<<21)|(1<<22)|(1<<24)|(1<<26)},

	{ePIN_AS_SPI0 , (1<<25) , (1<<25) , 0x00 , 0x00 , 0x00 , 0x00 , (1<<0)|(1<<1)|(3<<8)|(3<<10)|(3<<12) , (1<<0)|(1<<1)|(3<<8)|(3<<10)|(3<<12)},
	{ePIN_AS_TWI , (1<<7)|(1<<8) , (1<<7)|(1<<8) , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00},
	{ePIN_AS_MMCSD , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , (1<<6)|(1<<7)|(3<<8)|(3<<10)|(3<<12)|(3<<14)|(3<<16)|(3<<18) , (1<<6)|(1<<7)|(1<<8)|(1<<10)|(1<<12)|(1<<14)|(1<<16)|(1<<18)},
	{ePIN_AS_OPCLK , (3<<9) , (1<<9) , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00}
};

//this used to clr in gpio chare pin cfg1
T_SHARE_CFG_GPIO share_cfg_gpio[] = 
{
    //start         end             bit
        {0 , 2 , 0 , (1<<0) , 1 , (0<<0)},
    	{4 , 4 , 0 , (1<<4) , 1 , (0<<4)},
    	{5 , 5 , 0 , (3<<5) , 2 , (0<<5)},
    	{6 , 9 , 1 , (3<<4) , 2 , (1<<4)},
        {10 , 11 , 2 , (3<<0) , 2 , (0<<0)},
        {12 , 12 , 2 , (1<<4) , 1 , (0<<4)},
        {13 , 13 , 2 , 3<<5 , 2 , (0<<5)},
        {14 , 15 , 2 , 3<<8 , 2 , (0<<8)},
        {16 , 18 , 2 , 1<<12 , 1 , (0<<12)},
        {19 , 20 , 2 , 3<<16 , 2 , (0<<16)},
        {21 , 22 , 2 , 1<<20 , 1 , (0<<20)},
        {23 , 24 , 2 , 3<<22 , 2 , (0<<22)},
        {25 , 26 , 3 , 1<<0 , 1 , (0<<0)},
        {27 , 28 , 0 , 1<<7 , 1 , (0<<7)},
        {29 , 30 , 3 , 3<<2 , 2 , (0<<2)},
        {31 , 32 , 3 , 1<<6 , 1 , (0<<6)},
        {33 , 34 , 3 , 3<<8 , 2 , (0<<8)},
        {39 , 40 , 3 , 3<<16 , 2 , (0<<16)},
        {41 , 42 , 3 , 1<<20 , 1 , (0<<20)},
        {43 , 44 , 3 , 3<<22 , 2 , (0<<22)},
        {47 , 47 , 0 , 3<<9 , 2 , (0<<9)},
        {48 , 48 , 0 , 1<<11 , 1 , (0<<11)},
        {50 , 51 , 0 , 1<<12 , 1 , (0<<12)},
        {52 , 55 , 0 , 1<<14 , 1 , (0<<14)},
        {57 , 58 , 0 , 1<<19 , 1 , (0<<19)},
        {64 , 67 , 1 , 1<<0 , 1 , (1<<0)},
        {68 , 75 , 1 , 1<<12 , 1 , (1<<12)},
        {76 , 76 , 2 , 1<<7 , 1 , (0<<7)},
        {77 , 77 , 2 , 1<<15 , 1 , (0<<15)},
        {78 , 78 , 2 , 1<<26 , 1 , (0<<26)}

};


T_SHARE_CFG_FUNC_PULL share_cfg_func_pull[] =
{
		{0 , 5 , 0 , 0},
		{6 , 9 , 1 , 4},
		{10 , 13 , 2 , 0},
		{14 , 18 , 2 , 5},
		{19 , 24 , 2 , 11},
		{25 , 26 , 3 , 0},
		{27 , 28 , 0 , 6},
		{29 , 46 , 3 , 2},
		{47 , 48 , 0 , 8},
		{50 , 63 , 0 , 10},
		{64 , 67 , 1 , 0},
		{68 , 75 , 1 , 8},
		{76 , 76 , 2 , 4},
		{77 , 77 , 2 , 10},
		{78 , 78 , 2 , 17}
				
};



/*
    this table contains all the GPIOs attribute: each GPIO is represented by 2-bit,
    which equals to:   00-pullup
                    01-pulldown
                    10-pullup/pulldown
                    11-undefined
*/

const unsigned long gpio_pull_attribute_table_39XX[][2] = {  
    0x551550C0,/*gpio[0:15]*/		0x15555, /*gpio[16:31]*/
    0x40000000,/*gpio[32:47]*/	0x543500D, /*gpio[48:63]*/
    0xD5055555,	/*gpio[64:79]*/	0xFFFFFFFF /*no significance */
};
/**
 * @brief get gpio share pin as uart
 * @author  liao_zhijun
 * @date 2010-07-28
 * @param uart_id [in]  uart id
 * @param clk_pin [in]  clk pin
 * @param data_pin [in]  data pin
 * @return bool
 * @retval true get successfully
 * @retval false fail to get
 */
bool gpio_get_uart_pin(T_UART_ID uart_id, unsigned long* clk_pin, unsigned long* data_pin)
{
    if (uart_id >= MAX_UART_NUM)
    {
        return false;
    }
    switch (uart_id)
    {
        case uiUART0:
            *clk_pin = 1;
            *data_pin = 2;
            break;
        case uiUART1:
            *clk_pin = 3;
            *data_pin = 4;
            break;
        default:
            return false;
    }

    return true;
}


/**
 * @brief set gpio share pin as gpio 
 * @author  liao_zhijun
 * @date 2010-07-28
 * @param pin [in]  gpio pin ID
 * @return bool
 * @retval true set successfully
 * @retval false fail to set
 */
bool  gpio_set_pin_as_gpio (unsigned long pin)
{
    unsigned long i;
    unsigned long regvalue;
    unsigned long bit = 0;

    //check param
    if(false == gpio_assert_legal(pin))
    {
    	akprintf(C2, M_DRVSYS, "this pin number not exist\n");
        return false;
    }
    if(pin ==49)
    {
    	akprintf(C2, M_DRVSYS, "this gpio only enable usb!\n");
        return false;
    }
	if(((pin >=59)&&(pin <= 63)))
	{
		return true;
	}
    if ((pin == 35)||(pin == 36)) 
    {
        regvalue = REG32(gpio_sharepin_con_reg[3]);
        regvalue &= ~(3<<12);
        regvalue |= (0<<12);
        REG32(gpio_sharepin_con_reg[3]) = regvalue;
        return true;
    }
    if ((pin == 37)||(pin == 38)) 
    {
        regvalue = REG32(gpio_sharepin_con_reg[3]);
        regvalue &= ~(3<<14);
        regvalue |= (0<<14);
        REG32(gpio_sharepin_con_reg[3]) = regvalue;
        return true;
    }
    if ((pin == 45)||(pin == 46)) 
    {
        regvalue = REG32(gpio_sharepin_con_reg[3]);
        regvalue &= ~(3<<26);
        regvalue |= (0<<26);
        REG32(gpio_sharepin_con_reg[3]) = regvalue;
		return true;
	}
	
	if(((pin >= 6) && (pin <= 9)) || ((pin >= 64) && (pin <= 75)))
		{
			akprintf(C2, M_DRVSYS, "this pin dafault is cemare!\n");
		}
	
    //loop to find the correct bits to clr in share ping cfg1
    for(i = 0; i < sizeof(share_cfg_gpio)/sizeof(T_SHARE_CFG_GPIO); i++)
    {
        if((pin >= share_cfg_gpio[i].gpio_start) && (pin <= share_cfg_gpio[i].gpio_end))
        {
            regvalue = REG32(gpio_sharepin_con_reg[share_cfg_gpio[i].rig_num]);

            regvalue &= ~((share_cfg_gpio[i].bit_start_mask)<<((pin-share_cfg_gpio[i].gpio_start)*share_cfg_gpio[i].bit_num));
            regvalue |= ((share_cfg_gpio[i].valu)<<(((pin-share_cfg_gpio[i].gpio_start)*share_cfg_gpio[i].bit_num)));
            
            REG32(gpio_sharepin_con_reg[share_cfg_gpio[i].rig_num]) = regvalue;

            return true;
        }
    }
    
    return false;
}

/**
 * @brief set gpio pin group as specified module used
 * @author  liao_zhijun
 * @date 2010-07-28
 * @param[in] PinCfg enum data. the specified module
 * @return bool
 * @retval AK_TURE setting successful
 * @retval false setting failed
*/
bool gpio_pin_group_cfg(E_GPIO_PIN_SHARE_CONFIG PinCfg)
{
    unsigned long i,j;
    unsigned long reg;


    for(i = 0; i<(sizeof(share_cfg_module)/sizeof(T_SHARE_CFG_FUNC_MODULE)); i++)
    {
        //if(ePIN_AS_DUMMY == share_cfg_module[i].func_module)
       //     break;

        if(PinCfg == share_cfg_module[i].func_module)
        {
            //set pull attribute for module
            //gpio_set_group_attribute(PinCfg);

            //set share pin cfg reg1
            reg = REG32(gpio_sharepin_con_reg[0]);
            
            reg &= ~(share_cfg_module[i].reg1_bit_mask);
            reg |= share_cfg_module[i].reg1_bit_value;

            REG32(gpio_sharepin_con_reg[0]) = reg;

            //set share pin cfg reg2    
            reg = REG32(gpio_sharepin_con_reg[1]);

            reg &= ~(share_cfg_module[i].reg2_bit_mask);
            reg |= share_cfg_module[i].reg2_bit_value;

            REG32(gpio_sharepin_con_reg[1]) = reg;

            //set share pin cfg reg3
            reg = REG32(gpio_sharepin_con_reg[2]);
            
            reg &= ~(share_cfg_module[i].reg3_bit_mask);
            reg |= share_cfg_module[i].reg3_bit_value;

            REG32(gpio_sharepin_con_reg[2]) = reg;

            //set share pin cfg reg4    
            reg = REG32(gpio_sharepin_con_reg[3]);

            reg &= ~(share_cfg_module[i].reg4_bit_mask);
            reg |= share_cfg_module[i].reg4_bit_value;

            REG32(gpio_sharepin_con_reg[3]) = reg;

		
            return true;
        }
    }

    return false;
}

/*
    enable: 1:enable pullup 0:disable pullup function

    for 7801,commonly three rules:
    1.  if the pin is attached pullup/pulldown resistor only, then, 
        wrting 0 to the corresponding register bit to enable pullup/pulldown, 
        1 to disable pullup/pulldown!
        
    2.  if the pin is attached pullup and pulldown resistor, then writing 1 to enable
        pullup, 0 to enable pulldown, if you want to disable pullup/pulldown, then 
        disable the PE parameter
*/
bool gpio_set_pull_up_r(const unsigned long pin, bool enable)
{
    unsigned long index, reg_data, shift, residual, rs, regvalue;
    T_GPIO_TYPE reg_bit;
	unsigned char i = 0;
    const unsigned long (*gpio_pull_attribute_table)[2] = NULL;

    if(false == gpio_assert_legal(pin))
    {
        return false;
    }
    gpio_pull_attribute_table = gpio_pull_attribute_table_39XX;
    index = pin / 32;
    residual = pin % 32;
    if (residual < 16)
        shift = 0;
    else
        shift = 1;  
    
    //get pin pullup/pulldown attribute
    reg_data = gpio_pull_attribute_table[index][shift];
    if (shift)
        rs = (residual - 16) * 2;
    else
        rs = residual * 2;
    reg_bit = (reg_data >> rs) & 0x3;   

    if ((reg_bit == PULLDOWN) || (reg_bit == UNDEFINED))
    {
        return false;
    }
    for(i = 0; i < sizeof(share_cfg_func_pull)/sizeof(T_SHARE_CFG_FUNC_PULL); i++)
    {
        if((pin >= share_cfg_func_pull[i].gpio_start) && (pin <= share_cfg_func_pull[i].gpio_end))
        {
            regvalue = REG32(gpio_pull_set_reg[share_cfg_func_pull[i].rig_num]);

            regvalue &= ~(1<<(pin-share_cfg_func_pull[i].gpio_start+share_cfg_func_pull[i].bit_start_mask));
            if(!enable)regvalue |= (1<<(pin-share_cfg_func_pull[i].gpio_start+share_cfg_func_pull[i].bit_start_mask));
            
            REG32(gpio_pull_set_reg[share_cfg_func_pull[i].rig_num]) = regvalue;
            return true;
        }
    }    

    return false; 
}


//1.enable pulldown 0.disable pulldown
bool gpio_set_pull_down_r(const unsigned long pin, bool enable)
{
    unsigned long index, reg_data, shift, residual, rs, regvalue;
    T_GPIO_TYPE reg_bit;
	unsigned char i = 0;
    const unsigned long (*gpio_pull_attribute_table)[2] = NULL;

    if(false == gpio_assert_legal(pin))
    {
        return false;
    }
    gpio_pull_attribute_table = gpio_pull_attribute_table_39XX;
    index = pin / 32;
    residual = pin % 32;
    if (residual < 16)
        shift = 0;
    else
        shift = 1;  
    
    //get pin attribute
    reg_data = gpio_pull_attribute_table[index][shift];
    if (shift)
        rs = (residual - 16) * 2;
    else
        rs = residual * 2;
    reg_bit = (reg_data >> rs) & 0x3;

    if ((reg_bit == PULLUP) || (reg_bit == UNDEFINED))
    {
        return false;
    }
	for(i = 0; i < sizeof(share_cfg_func_pull)/sizeof(T_SHARE_CFG_FUNC_PULL); i++)
    {
        if((pin >= share_cfg_func_pull[i].gpio_start) && (pin <= share_cfg_func_pull[i].gpio_end))
        {
            regvalue = REG32(gpio_pull_set_reg[share_cfg_func_pull[i].rig_num]);

            regvalue &= ~(1<<(pin-share_cfg_func_pull[i].gpio_start+share_cfg_func_pull[i].bit_start_mask));
            if(!enable)regvalue |= (1<<(pin-share_cfg_func_pull[i].gpio_start+share_cfg_func_pull[i].bit_start_mask));
            
            REG32(gpio_pull_set_reg[share_cfg_func_pull[i].rig_num]) = regvalue;

            return true;
        }
    } 

    return false; 
}

//get wakeup gpio bit mask
unsigned char  get_wGpio_Bit(unsigned char pin )
{
    unsigned char mask_bit = INVALID_GPIO;

    if (pin <= 2)
        mask_bit = pin;
	else if ((pin >= 4) && (pin <= 11))
        mask_bit = pin;
    else if ((pin >= 13) && (pin <= 15))
        mask_bit = pin - 1;
    else if ((pin >= 19) && (pin <= 20))
        mask_bit = pin - 4;
    else if ((pin >= 23) && (pin <= 24))
        mask_bit = pin - 6;
	else if ((pin >= 52) && (pin <= 55))
        mask_bit = pin - 32;
	else if ((pin >= 57) && (pin <= 63))
        mask_bit = pin - 32;
	else if (49 == pin)
        mask_bit = 19;
    else 
    {
        akprintf(C3, M_DRVSYS, "pin %d isn't wakeup GPIO!\n", pin);
        return INVALID_GPIO;
    }


    return mask_bit;

}

unsigned long get_wGpio_Pin(unsigned long mask_bit)
{
    unsigned long pin = INVALID_GPIO;
	
	if(mask_bit <= 2)
		pin = mask_bit;
	else if((mask_bit >= 4) && (mask_bit <= 11))
		pin = mask_bit;
	else if((mask_bit >= 12) && (mask_bit <= 14))
		pin = mask_bit + 1;
	else if((mask_bit >= 15) && (mask_bit <= 16))
		pin = mask_bit + 4;
	else if((mask_bit >= 17) && (mask_bit <= 18))
		pin = mask_bit + 6;
	else if((mask_bit >= 20) && (mask_bit <= 23))
		pin = mask_bit + 32;
	else if((mask_bit >= 25) && (mask_bit <= 31))
		pin = mask_bit + 32;
	else if(19 == mask_bit)
		pin = 49;
    else 
    {
        return INVALID_GPIO;
    }
   
    return pin;
}

//polarity: 0: rising edge triggered 1: falling edge triggered
void gpio_set_wakeup_p(unsigned long pin, bool polarity)
{
    unsigned char mask_bit;

    if(false == gpio_assert_legal(pin))
    {
        return;
    }

    mask_bit = get_wGpio_Bit(pin);
    
    if (polarity)
        REG32(RTC_WAKEUP_GPIO_P_REG) &= ~(1 << mask_bit);
    else        
        REG32(RTC_WAKEUP_GPIO_P_REG) |= 1 << mask_bit;
}
/**
 * @brief cancel gpio pin group cfg with GPIO
 * @author  jiankui
 * @date 2016-08-30
 * @param[in] PinCfg enum data. the specified module
 * @return void
*/
static void gpio_cancel_group_cfg(unsigned char group_num)
{
	unsigned long reg;

    //set share pin cfg reg1
    reg = REG32(gpio_sharepin_con_reg[0]);
    reg &= ~(share_cfg_module[group_num].reg1_bit_mask);
    reg |= 0x00;        
	REG32(gpio_sharepin_con_reg[0]) = reg;
	
    //set share pin cfg reg2    
    reg = REG32(gpio_sharepin_con_reg[1]);
    reg &= ~(share_cfg_module[group_num].reg2_bit_mask);
    reg |= 0x00;
    REG32(gpio_sharepin_con_reg[1]) = reg;

    //set share pin cfg reg3
    reg = REG32(gpio_sharepin_con_reg[2]);          
    reg &= ~(share_cfg_module[group_num].reg3_bit_mask);
    reg |= 0x00;
    REG32(gpio_sharepin_con_reg[2]) = reg;

    //set share pin cfg reg4    
    reg = REG32(gpio_sharepin_con_reg[3]);
    reg &= ~(share_cfg_module[group_num].reg4_bit_mask);
    reg |= 0x00;
    REG32(gpio_sharepin_con_reg[3]) = reg;
}
/**
 * @brief swap Array share cfg module and special share module
 * @author  jiankui
 * @date 2016-08-30
 * @param[in] group_num share cfg Array index
 * @param[in] special_num special share module Array index
 * @return void
*/
static void share_group_cfg_swap(unsigned char group_num, unsigned char special_num)
{
	share_cfg_module[group_num].reg1_bit_mask = special_share_cfg_module[special_num].reg1_bit_mask;
	share_cfg_module[group_num].reg1_bit_value = special_share_cfg_module[special_num].reg1_bit_value;
	share_cfg_module[group_num].reg2_bit_mask = special_share_cfg_module[special_num].reg2_bit_mask;
	share_cfg_module[group_num].reg2_bit_value = special_share_cfg_module[special_num].reg2_bit_value;
	share_cfg_module[group_num].reg3_bit_mask = special_share_cfg_module[special_num].reg3_bit_mask;
	share_cfg_module[group_num].reg3_bit_value = special_share_cfg_module[special_num].reg3_bit_value;
	share_cfg_module[group_num].reg4_bit_mask = special_share_cfg_module[special_num].reg4_bit_mask;
	share_cfg_module[group_num].reg4_bit_value = special_share_cfg_module[special_num].reg4_bit_value;
}
/**
 * @brief set specified module  gpio pin group different config
 * @author  jiankui
 * @date 2016-08-30
 * @param[in] PinCfg enum data. the specified module
  * @param[in] share_num special module config numer
 * @return bool
 * @retval AK_TURE setting successful
 * @retval false setting failed
*/
bool gpio_share_pin_set(E_GPIO_PIN_SHARE_CONFIG PinCfg, unsigned char share_num)
{
	unsigned char i;
	for(i = 0; i<(sizeof(share_cfg_module)/sizeof(T_SHARE_CFG_FUNC_MODULE)); i++)
    {
        if(PinCfg == share_cfg_module[i].func_module)
			break;
	}
	if(i ==(sizeof(share_cfg_module)/sizeof(T_SHARE_CFG_FUNC_MODULE)))
	{
		akprintf(C2, M_DRVSYS, "this Pincfg unexist!\n");
				return true;
	}
	switch(PinCfg)
	{
		case ePIN_AS_PWM1:
			if(share_num >= 2)
			{
				akprintf(C1, M_DRVSYS, "PWM1 share num error!\n");
				return false;
			}
			gpio_cancel_group_cfg(i);
			share_group_cfg_swap(i,ePIN_AS_PWM1_S0+share_num);
			break;		
		case ePIN_AS_PWM3:
			if(share_num >= 2)
			{
				akprintf(C1, M_DRVSYS, "PWM3 share num error!\n");
				return false;
			}
			gpio_cancel_group_cfg(i);
			share_group_cfg_swap(i,ePIN_AS_PWM3_S0+share_num);
			break;
		case ePIN_AS_I2S:
			if(share_num >= 2)
			{
				akprintf(C1, M_DRVSYS, "I2S share num error!\n");
				return false;
			}
			gpio_cancel_group_cfg(i);
			share_group_cfg_swap(i,ePIN_AS_I2S_S0+share_num);
			break;
		case ePIN_AS_SPI0:
			if(share_num >= 2)
			{
				akprintf(C1, M_DRVSYS, "SPI0 share num error!\n");
				return false;
			}
			gpio_cancel_group_cfg(i);
			share_group_cfg_swap(i,ePIN_AS_SPI0_S0+share_num);
			break;
		case ePIN_AS_SPI1:
			if(share_num >= 2)
			{
				akprintf(C1, M_DRVSYS, "SPI1 share num error!\n");
				return false;
			}
			gpio_cancel_group_cfg(i);
			share_group_cfg_swap(i,ePIN_AS_SPI1_S0+share_num);
			break;
		case ePIN_AS_SDIO:
			if(share_num >= 2)
			{
				akprintf(C1, M_DRVSYS, "SDIO share num error!\n");
				return false;
			}
			gpio_cancel_group_cfg(i);
			share_group_cfg_swap(i,ePIN_AS_SDIO_S0+share_num);
			break;
			default:
				akprintf(C2, M_DRVSYS, "share pin haven't multiple config\n");
			break;
	}
	return true;
}
