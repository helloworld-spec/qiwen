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
 
#include <common.h>
#include <asm/io.h>
#include <asm/errno.h>
#include <asm/arch/anyka_cpu.h>
#include <asm/gpio.h>

//#include "ak_gpio.h"

#define INVALID_GPIO                    0xfe
#define GPIO_NUMBER                     79


static unsigned long gpio_pin_dir_reg[] = {GPIO_DIR_REG1,       GPIO_DIR_REG2,      GPIO_DIR_REG3};
static unsigned long gpio_pin_in_reg[]  = {GPIO_IN_REG1,        GPIO_IN_REG2,       GPIO_IN_REG3};
static unsigned long gpio_pin_out_reg[] = {GPIO_OUT_REG1,       GPIO_OUT_REG2,      GPIO_OUT_REG3};
static unsigned long gpio_pin_inte_reg[]= {GPIO_INT_EN1,        GPIO_INT_EN2,       GPIO_INT_EN3};
static unsigned long gpio_pin_intp_reg[]= {GPIO_INT_LEVEL_REG1, GPIO_INT_LEVEL_REG2, GPIO_INT_LEVEL_REG3};

static volatile unsigned long gpio_pin_inte[4] = {0};
static volatile unsigned long gpio_pin_intp[4] = {0};


static const unsigned long gpio_sharepin_con_reg[] = {
    GPIO_SHAREPIN_CONTROL1, GPIO_SHAREPIN_CONTROL2,
	GPIO_SHAREPIN_CONTROL3, GPIO_SHAREPIN_CONTROL4
};


//gpio pullup/pulldown reg
static const unsigned long gpio_pull_set_reg[] = {
    GPIO_PULLUPDOWN_REG1, GPIO_PULLUPDOWN_REG2, 
    GPIO_PULLUPDOWN_REG3, GPIO_PULLUPDOWN_REG4
};


const unsigned long gpio_pull_attribute_table_39XX[][2] = {  
    0x551550C0,/*gpio[0:15]*/		0x15555, /*gpio[16:31]*/
    0x40000000,/*gpio[32:47]*/	0x543500D, /*gpio[48:63]*/
    0xD5055555,	/*gpio[64:79]*/	0xFFFFFFFF /*no significance */
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



/*
** GPIO CONTROL FUNCTION.
** customer need detecting gpio to meet their self-defined requirement.
*/
int g_ak39_setpin_as_gpio(unsigned int pin);
int g_ak39_gpio_set_mode(unsigned int pin, unsigned int to);
int g_ak39_gpio_get_value(unsigned int pin);
int g_ak39_gpio_set_value(unsigned int pin, unsigned int value);



#define AK_GPIO_MAX				85
#define GPIO_UPLIMIT			AK_GPIO_MAX
#define AK_PA_SYSCTRL		    (0x08000000)
#define AK_SHAREPIN_CON1		(AK_PA_SYSCTRL + 0x74)
#define AK_SHAREPIN_CON2		(AK_PA_SYSCTRL + 0x78)
#define AK_SHAREPIN_CON3		(AK_PA_SYSCTRL + 0x7C)
#define AK_SHAREPIN_CON4		(AK_PA_SYSCTRL + 0xDC)

#define AK_GPIO_DIR1			(0x20170000 + 0x00)
#define AK_GPIO_OUT1			(0x2017000C + 0x00)
#define AK_GPIO_IN1				(0x20170018 + 0x00)

#define AK_GPIO_DIR_BASE(pin)	(((pin)>>5)*4 + AK_GPIO_DIR1)
#define AK_GPIO_OUT_BASE(pin)	(((pin)>>5)*4 + AK_GPIO_OUT1)
#define AK_GPIO_IN_BASE(pin)	(((pin)>>5)*4 + AK_GPIO_IN1)
#define	AK_GPIO_DIR_INPUT		0
#define	AK_GPIO_DIR_OUTPUT		1

typedef enum  {
	AS_GPIO_CFG_BIT1 = 0,   // share cfg1  1
	AS_GPIO_CFG_BIT2,       // share cfg2  00
	AS_GPIO_CFG_BIT3,	      // share cfg2  01
	AS_GPIO_CFG_BIT4,	      // share cfg2  11	
}T_AS_GPIO_CFG;

struct sharepin_as_gpio {
    unsigned char gpio_start;
    unsigned char gpio_end;
	int index;
	T_AS_GPIO_CFG flag;
};

//this used to clr in gpio chare pin cfg1
struct sharepin_as_gpio sharepin_cfg_gpio1[] = {
    {0,		0,		0,	AS_GPIO_CFG_BIT2},
    {1,		1,		2,	AS_GPIO_CFG_BIT1},  
    {2,		2,		3,	AS_GPIO_CFG_BIT1},
    {3,	  3,		4,	AS_GPIO_CFG_BIT1},
    {4,	  4,		5,	AS_GPIO_CFG_BIT1},
    {5,	  5,		6,	AS_GPIO_CFG_BIT2},
    {27,	27,		8,	AS_GPIO_CFG_BIT1},    
    {28,	28,		9,	AS_GPIO_CFG_BIT1},
    {47,	47,		10,	AS_GPIO_CFG_BIT2},
    {48,	48,		12,	AS_GPIO_CFG_BIT2},
    {50,	50,		13,	AS_GPIO_CFG_BIT1},
    {51,	51,		14,	AS_GPIO_CFG_BIT1},
    {52,	52,		15,	AS_GPIO_CFG_BIT1},
    {53,	53,		16,	AS_GPIO_CFG_BIT1},
    {54,	54,		17,	AS_GPIO_CFG_BIT1},
    {55,	55,		18,	AS_GPIO_CFG_BIT1},
    {56,	56,		19,	AS_GPIO_CFG_BIT1},
    {57,	57,		20,	AS_GPIO_CFG_BIT1},
    {58,	58,		21,	AS_GPIO_CFG_BIT1},        
    {79,	79,		25,	AS_GPIO_CFG_BIT1},
    {80,	80,		22,	AS_GPIO_CFG_BIT1},
    {81,	81,		23,	AS_GPIO_CFG_BIT1},
};
//this used to clr in gpio chare pin cfg2
struct sharepin_as_gpio sharepin_cfg_gpio2[] = {
    {64,	64,		0,	AS_GPIO_CFG_BIT3},
    {65,	65,		2,	AS_GPIO_CFG_BIT1},
    {66,	66,		3,	AS_GPIO_CFG_BIT3},
    {67,	67,		5,	AS_GPIO_CFG_BIT3},
    {6,	  6,		7,	AS_GPIO_CFG_BIT3},
    {7,	  7,		9,	AS_GPIO_CFG_BIT3},
    {8,	  8,		11,	AS_GPIO_CFG_BIT3},    
    {9,	  9,		13,	AS_GPIO_CFG_BIT3},
    {68,	68,		15,	AS_GPIO_CFG_BIT1},
    {69,	69,		17, AS_GPIO_CFG_BIT1},
    {70,	70,		19, AS_GPIO_CFG_BIT1},
    {71,	71,		21, AS_GPIO_CFG_BIT1},
    {72,	72,		23, AS_GPIO_CFG_BIT1},
    {73,	73,		25, AS_GPIO_CFG_BIT1},
    {74,	74,		26, AS_GPIO_CFG_BIT1},
    {75,	75,		27, AS_GPIO_CFG_BIT1},
};
//this used to clr in gpio chare pin cfg3
struct sharepin_as_gpio sharepin_cfg_gpio3[] = {
    {10,	10,		0,	AS_GPIO_CFG_BIT2},
    {11,	11,		2,	AS_GPIO_CFG_BIT2},
    {12,	12,		4,	AS_GPIO_CFG_BIT1},
    {13,	13,		5,	AS_GPIO_CFG_BIT2},
    {76,	76,		7,	AS_GPIO_CFG_BIT1},
    {14,	14,		8,	AS_GPIO_CFG_BIT2},
    {15,	15,		10,	AS_GPIO_CFG_BIT2},
    {16,	16,		12, AS_GPIO_CFG_BIT1},
    {17,	17,		13, AS_GPIO_CFG_BIT1},
    {18,	18,		14, AS_GPIO_CFG_BIT1},
    {77,	77,		15, AS_GPIO_CFG_BIT1},
    {19,	19,		16, AS_GPIO_CFG_BIT2},
    {20,	20,		18, AS_GPIO_CFG_BIT2},    
    {21,	21,		20, AS_GPIO_CFG_BIT1},    
    {22,	22,		21, AS_GPIO_CFG_BIT1}, 
    {23,	23,		22, AS_GPIO_CFG_BIT2}, 
    {24,	24,		24, AS_GPIO_CFG_BIT2},    
    {78,	78,		26, AS_GPIO_CFG_BIT1},                
};
//this used to clr in gpio chare pin cfg4
struct sharepin_as_gpio sharepin_cfg_gpio4[] = {
    {25,	25,		0,	AS_GPIO_CFG_BIT1},
    {26,	26,		1,	AS_GPIO_CFG_BIT1},
    {29,	29,		2,	AS_GPIO_CFG_BIT2},
    {30,	30,		4,	AS_GPIO_CFG_BIT2},
    {31,	31,		6,	AS_GPIO_CFG_BIT1},
    {32,	32,		7,	AS_GPIO_CFG_BIT1},
    {33,	33,		8,	AS_GPIO_CFG_BIT1},
    {79,	79,		9,	AS_GPIO_CFG_BIT1},
    {34,	34,		10,	AS_GPIO_CFG_BIT2},    
    {35,	36,		12,	AS_GPIO_CFG_BIT2},
    {37,	38,		14, AS_GPIO_CFG_BIT2},
    {39,	39,		16, AS_GPIO_CFG_BIT2},
    {40,	40,		18, AS_GPIO_CFG_BIT2},
    {41,	41,		20, AS_GPIO_CFG_BIT1},
    {42,	42,		21, AS_GPIO_CFG_BIT1},
    {43,	43,		22, AS_GPIO_CFG_BIT2},
    {44,	44,		24, AS_GPIO_CFG_BIT2}, 
    {45,	46,		26, AS_GPIO_CFG_BIT2},          
};
/**
 * @brief set gpio share pin as gpio 
 * @param pin [in]  gpio pin ID
 */
int g_ak39_setpin_as_gpio(unsigned int pin)
{
    int i, bit = 0;

    if ((pin < 0) || (pin > GPIO_UPLIMIT)) {
        return 0;
    }

    //loop to find the correct bits to clr in share ping cfg1
    for(i = 0; i < ARRAY_SIZE(sharepin_cfg_gpio1); i++){
        if((pin >= sharepin_cfg_gpio1[i].gpio_start) 
			&& (pin <= sharepin_cfg_gpio1[i].gpio_end))
        {
					bit = sharepin_cfg_gpio1[i].index;
					if (sharepin_cfg_gpio1[i].flag == AS_GPIO_CFG_BIT1)
					REG32(AK_SHAREPIN_CON1) &= ~(1 << bit);
					else if (sharepin_cfg_gpio1[i].flag == AS_GPIO_CFG_BIT2)
					REG32(AK_SHAREPIN_CON1) &= ~(0x3 << bit);
					else if (sharepin_cfg_gpio1[i].flag == AS_GPIO_CFG_BIT3)
						{
							REG32(AK_SHAREPIN_CON1) &= ~(0x3 << bit);	
							REG32(AK_SHAREPIN_CON1) |= (0x1 << bit);								
					  }			
					return 0;   		
        }  
    }
	 for(i = 0; i < ARRAY_SIZE(sharepin_cfg_gpio2); i++){
	        if((pin >= sharepin_cfg_gpio2[i].gpio_start) 
				&& (pin <= sharepin_cfg_gpio2[i].gpio_end))
	        {
						
						bit = sharepin_cfg_gpio2[i].index;
						if (sharepin_cfg_gpio2[i].flag == AS_GPIO_CFG_BIT1)
						REG32(AK_SHAREPIN_CON2) &= ~(1 << bit);
						else if (sharepin_cfg_gpio2[i].flag == AS_GPIO_CFG_BIT2)
						REG32(AK_SHAREPIN_CON2) &= ~(0x3 << bit);
						else if (sharepin_cfg_gpio2[i].flag == AS_GPIO_CFG_BIT3)
							{
								REG32(AK_SHAREPIN_CON2) &= ~(0x3 << bit);	
								REG32(AK_SHAREPIN_CON2) |= (0x1 << bit);								
						  }								
						return 0;   		
	        }
	   
	    }
	 for(i = 0; i < ARRAY_SIZE(sharepin_cfg_gpio3); i++){
	        if((pin >= sharepin_cfg_gpio3[i].gpio_start) 
				&& (pin <= sharepin_cfg_gpio3[i].gpio_end))
	        {
						
						bit = sharepin_cfg_gpio3[i].index;
						if (sharepin_cfg_gpio3[i].flag == AS_GPIO_CFG_BIT1)
						REG32(AK_SHAREPIN_CON3) &= ~(1 << bit);
						else if (sharepin_cfg_gpio3[i].flag == AS_GPIO_CFG_BIT2)
						REG32(AK_SHAREPIN_CON3) &= ~(0x3 << bit);
						else if (sharepin_cfg_gpio3[i].flag == AS_GPIO_CFG_BIT3)
							{
								REG32(AK_SHAREPIN_CON3) &= ~(0x3 << bit);	
								REG32(AK_SHAREPIN_CON3) |= (0x1 << bit);								
						  }								
						return 0;   		
	        }
	    }
	 for(i = 0; i < ARRAY_SIZE(sharepin_cfg_gpio4); i++){
	        if((pin >= sharepin_cfg_gpio4[i].gpio_start) 
				&& (pin <= sharepin_cfg_gpio4[i].gpio_end))
	        {

						bit = sharepin_cfg_gpio4[i].index;
						if (sharepin_cfg_gpio4[i].flag == AS_GPIO_CFG_BIT1)
						REG32(AK_SHAREPIN_CON4) &= ~(1 << bit);
						else if (sharepin_cfg_gpio4[i].flag == AS_GPIO_CFG_BIT2)
						REG32(AK_SHAREPIN_CON4) &= ~(0x3 << bit);
						else if (sharepin_cfg_gpio4[i].flag == AS_GPIO_CFG_BIT3)
							{
								REG32(AK_SHAREPIN_CON4) &= ~(0x3 << bit);	
								REG32(AK_SHAREPIN_CON4) |= (0x1 << bit);								
						  }		
						return 0;   		
	        }
	    }

    return 0;
}

/* 
 * configuration gpio pin
 * 0: corresponding port is input mode
 * 1: corresponding port is output mode
 */
int g_ak39_gpio_set_mode(unsigned int pin, unsigned int to)
{
    void __iomem *base = AK_GPIO_DIR_BASE(pin);
    unsigned int offset = ((pin) & 31);
    unsigned long flags;
    
    if ((pin < 0) || (pin > GPIO_UPLIMIT)) {
        return 0;
    }

    if (AK_GPIO_DIR_INPUT == to)
        REG32(base) &= ~(1 << offset);
    else if (AK_GPIO_DIR_OUTPUT == to)
        REG32(base) |= (1 << offset);
	
    return 0;
}

/* 
 * get gpio value
 */
int g_ak39_gpio_get_value(unsigned int pin)
{
    void __iomem *base = AK_GPIO_IN_BASE(pin);
    unsigned int offset = ((pin) & 31);
    unsigned long value;
    
    if ((pin < 0) || (pin > GPIO_UPLIMIT)) {
        return 0;
    }

    value = REG32(base);
	debug("base 0x%08x, value is 0x%08x, pin %d pin-value %d,\n", 
			base, value, pin, ((value >> offset) & 0x01));
	value = (value >> offset) & 0x1;
    
    return value;
}

int g_ak39_gpio_set_value(unsigned int pin, unsigned int value)
{
    void __iomem *base = AK_GPIO_OUT_BASE(pin);
    unsigned int offset = ((pin) & 31);
    
    if ((pin > GPIO_UPLIMIT))
        return -1;
	
	/* 
	 * write hith or low level
	 */
	if (value == 0) {
		REG32(base) &= ~(0x1 << offset);
	} else {
		REG32(base) |= (0x1 << offset);
	}
	printf("write %u to pin %u on address 0x%08x down\n", value, pin, base);
    
    return 0;

}




int gpio_assert_legal(unsigned long pin)
{
    if((pin == INVALID_GPIO) || (pin >= GPIO_NUMBER))
    {
        return -1;
    }
    else
    {
        return 0;
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

    //disable gpio int before enable its interrupt
    gpio_int_disableall();

}


/**
 * @brief: gpio_request.
 * @author luoyanliang
 * @date 2017-01-09
 * @return void
 * @retval
 */
int gpio_request(unsigned gpio, const char *label)
{
	gpio_init();
	return 0;
}


/**
 * @brief: gpio_free.
 * @author luoyanliang
 * @date 2017-01-09
 * @return void
 * @retval
 */
int gpio_free(unsigned gpio)
{
	return 0;
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
    
    if(-1 == gpio_assert_legal(pin))
    {
        return;
    }
	
    index = pin / 32;
    residual = pin % 32;
	
    if(level)
    { 
        *(volatile unsigned long*)gpio_pin_out_reg[index] |= (1 << residual);
	}
    else
    {
        *(volatile unsigned long*)gpio_pin_out_reg[index] &= ~(1 << residual);
    }
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

    if(-1 == gpio_assert_legal(pin))
    {
        return 0xff;
    }

	index = pin / 32;
    residual = pin % 32;

    if(REG32(gpio_pin_in_reg[index]) & (1 << residual))
        level = 1;
    else
        level = 0;
	
    return level;
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
    
    if(-1 == gpio_assert_legal(pin))
    {
        return;
    }

	index = pin / 32;
    residual = pin % 32;
    if(dir == 0)//input mode
    {
        *(volatile unsigned long*)gpio_pin_dir_reg[index] &= ~(1 << residual);
    }
    else
	{
        *(volatile unsigned long*)gpio_pin_dir_reg[index] |= (1 << residual);
    }   

}




/**
 * @brief: gpio_direction_input.
 * @author luoyanliang
 * @date 2017-01-09
 * @return void
 * @retval
 */
int gpio_direction_input(unsigned gpio)
{
	gpio_set_pin_dir(gpio, GPIO_DIR_INPUT);	
	return 0;
}


/**
 * @brief: gpio_direction_output.
 * @author luoyanliang
 * @date 2017-01-09
 * @return 0 1
 * @retval
 */
int gpio_direction_output(unsigned gpio, int value)
{
	unsigned long index, residual, i;
    
	gpio_set_pin_dir(gpio, GPIO_DIR_OUTPUT);	
	gpio_set_pin_level(gpio, value);
	
	return 0;
}



/**
 * @brief: gpio_get_value.
 * @author luoyanliang
 * @date 2017-01-09
 * @return 1 0
 * @retval
 */
int gpio_get_value(unsigned gpio)
{
    unsigned long level = 0;   
	level = gpio_get_pin_level(gpio);
	
    return level;

}


int gpio_set_pull_up_r(const unsigned long pin, int enable)
{
    unsigned long index, reg_data, shift, residual, rs, regvalue;
    T_GPIO_TYPE reg_bit;
	unsigned char i = 0;
    const unsigned long (*gpio_pull_attribute_table)[2] = NULL;

    if(-1 == gpio_assert_legal(pin))
    {
        return -1;
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
        return -1;
    }
    for(i = 0; i < sizeof(share_cfg_func_pull)/sizeof(T_SHARE_CFG_FUNC_PULL); i++)
    {
        if((pin >= share_cfg_func_pull[i].gpio_start) && (pin <= share_cfg_func_pull[i].gpio_end))
        {
            regvalue = REG32(gpio_pull_set_reg[share_cfg_func_pull[i].rig_num]);

            regvalue &= ~(1<<(pin-share_cfg_func_pull[i].gpio_start+share_cfg_func_pull[i].bit_start_mask));
            if(!enable)regvalue |= (1<<(pin-share_cfg_func_pull[i].gpio_start+share_cfg_func_pull[i].bit_start_mask));
            
            REG32(gpio_pull_set_reg[share_cfg_func_pull[i].rig_num]) = regvalue;
            return 0;
        }
    }    

    return -1; 
}


//1.enable pulldown 0.disable pulldown
int gpio_set_pull_down_r(const unsigned long pin, int enable)
{
    unsigned long index, reg_data, shift, residual, rs, regvalue;
    T_GPIO_TYPE reg_bit;
	unsigned char i = 0;
    const unsigned long (*gpio_pull_attribute_table)[2] = NULL;

    if(-1 == gpio_assert_legal(pin))
    {
        return -1;
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
        return -1;
    }
	for(i = 0; i < sizeof(share_cfg_func_pull)/sizeof(T_SHARE_CFG_FUNC_PULL); i++)
    {
        if((pin >= share_cfg_func_pull[i].gpio_start) && (pin <= share_cfg_func_pull[i].gpio_end))
        {
            regvalue = REG32(gpio_pull_set_reg[share_cfg_func_pull[i].rig_num]);

            regvalue &= ~(1<<(pin-share_cfg_func_pull[i].gpio_start+share_cfg_func_pull[i].bit_start_mask));
            if(!enable)regvalue |= (1<<(pin-share_cfg_func_pull[i].gpio_start+share_cfg_func_pull[i].bit_start_mask));
            
            REG32(gpio_pull_set_reg[share_cfg_func_pull[i].rig_num]) = regvalue;

            return 0;
        }
    } 

    return -1; 
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
int gpio_set_pin_as_gpio (unsigned long pin)
{
    unsigned long i;
    unsigned long regvalue;
    unsigned long bit = 0;

    //check param
    if(-1 == gpio_assert_legal(pin))
    {
        return -1;
    }
    if(pin ==49)
    {
        return -1;
    }
	if(((pin >=59)&&(pin <= 63)))
	{
		return 0;
	}
    if ((pin == 35)||(pin == 36)) 
    {
        regvalue = REG32(gpio_sharepin_con_reg[3]);
        regvalue &= ~(3<<12);
        regvalue |= (0<<12);
        REG32(gpio_sharepin_con_reg[3]) = regvalue;
        return 0;
    }
    if ((pin == 37)||(pin == 38)) 
    {
        regvalue = REG32(gpio_sharepin_con_reg[3]);
        regvalue &= ~(3<<14);
        regvalue |= (0<<14);
        REG32(gpio_sharepin_con_reg[3]) = regvalue;
        return 0;
    }
    if ((pin == 45)||(pin == 46)) 
    {
        regvalue = REG32(gpio_sharepin_con_reg[3]);
        regvalue &= ~(3<<26);
        regvalue |= (0<<26);
        REG32(gpio_sharepin_con_reg[3]) = regvalue;
		return 0;
	}

	#if 0
	if(((pin >= 6) && (pin <= 9)) || ((pin >= 64) && (pin <= 75)))
		{
			akprintf(C2, M_DRVSYS, "this pin dafault is cemare!\n");
		}
	#endif
    //loop to find the correct bits to clr in share ping cfg1
    for(i = 0; i < sizeof(share_cfg_gpio)/sizeof(T_SHARE_CFG_GPIO); i++)
    {
        if((pin >= share_cfg_gpio[i].gpio_start) && (pin <= share_cfg_gpio[i].gpio_end))
        {
            regvalue = REG32(gpio_sharepin_con_reg[share_cfg_gpio[i].rig_num]);

            regvalue &= ~((share_cfg_gpio[i].bit_start_mask)<<((pin-share_cfg_gpio[i].gpio_start)*share_cfg_gpio[i].bit_num));
            regvalue |= ((share_cfg_gpio[i].valu)<<(((pin-share_cfg_gpio[i].gpio_start)*share_cfg_gpio[i].bit_num)));
            
            REG32(gpio_sharepin_con_reg[share_cfg_gpio[i].rig_num]) = regvalue;

            return 0;
        }
    }
    
    return -1;
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

