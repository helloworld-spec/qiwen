/**
 * @file twi.c
 * @brief TWI interface driver, define TWI interface APIs.
 *
 * This file provides TWI APIs: TWI initialization, write data to TWI & read data from TWI.
 * Copyright (C) 2004 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author Jiankui
 * @date 2016-09-08
 * @version 1.0
 * @ref AK3916E technical manual.
 */
#include "anyka_cpu.h"
#include "anyka_types.h"
#include "drv_api.h"
#include "drv_module.h"
#include "arch_twi.h"
#include "sysctl.h"
#include "drv_gpio.h"

#ifdef OS_ANYKA

#define OUT_TIME_MAX 3
#define TX_CLK 400000

/**
 * @brief 32bit number change to num 8bit number
 * @author Jiankui
 * @date 2016-09-13
 * @param data: save after change 8bit number
 * @param data32: before change 32bit number
  * @param num: unsigned char's number
 * @return void
 * @retval
 */
static void long_to_char(unsigned char *data, unsigned long data32, unsigned char num)
{
	unsigned char i;
	if(num > 4)
	{
		akprintf(C1, M_DRVSYS, "unsigned long change unsigned char longest is four!\n");
	}
	for(i = 0; i < num; i++)
	data[i] = data32>>(i*8);
}


/**
 * @brief TWI interface initialize function
 * setup TWI interface
 * @author Jiankui
 * @date 2016-09-08
 * @param void
 * @return void
 * @retval
 */
void twi_init(void)
{
	unsigned char clk_div;
	unsigned long asic_clk;
	
    /* clock gate & reset */
	sysctl_clock(CLOCK_TWI_ENABLE);

	 // soft reset TWI
    sysctl_reset(RESET_TWI);
	 
	/* share-pins */
	gpio_pin_group_cfg(ePIN_AS_TWI);
	/* pull-up */
	gpio_set_pull_up_r(27, 0);
	gpio_set_pull_up_r(28, 0);
	
	/* set TX_CLK*/
	/*calculate clk_div*/
	asic_clk = get_asic_freq();

	clk_div = (asic_clk/(32*TX_CLK))-1;
	if(clk_div > 0x0fL){
		clk_div = ((asic_clk/(1024*TX_CLK)-1) | (1<<6));
	}
	REG32(TWI_CTRL) = clk_div;

	
}

/**
 * @brief TWI interface release function
 *
 * setup TWI interface
 * @author Jiankui
 * @date 2016-09-12
 * @return void
 */
void twi_release()
{
	 /* clock gate & reset */
	sysctl_clock(~CLOCK_TWI_ENABLE);
	 // soft reset TWI
    sysctl_reset(RESET_TWI);
	/* share pin with gpio*/
	gpio_set_pin_as_gpio(27);
	gpio_set_pin_as_gpio(28);
}

/**
 * @brief write data to TWI device
 *
 * write size length data to daddr's raddr register, raddr and data is byte width
 * @author Jiankui
 * @date 2016-09-12
 * @param[in] daddr TWI device address
 * @param[in] raddr register address
 * @param[in] *data write data's pointer
 * @param[in] size write data's length
 * @return bool return operation successful or failed
 * @retval false operation failed
 * @retval true operation successful
 */
bool twi_write_data(unsigned char daddr, unsigned char raddr, unsigned char *data, unsigned long size)
{
	unsigned long regv, t1, t2;
	unsigned char j;
	unsigned long index, residual;

	if(16 < size)
	{
		akprintf(C2, M_DRVSYS, "every send data must lower 16 Byte!\n");
		return false;
	}
	

	irq_mask();
	/* set start bit,daddr,cmd enable*/
	/* write device address*/
	regv = REG32(TWI_CMD1);
	regv =(1<<18) | (1<<17) | daddr;
	REG32(TWI_CMD1) = regv;

	/* write register address*/
	regv = REG32(TWI_CMD2);
	regv = (1<<18) | raddr;
	REG32(TWI_CMD2) = regv;

	/* write DATA to TWI DATA*/
	index = size/4;
	residual = size%4;
	regv = 0;
	for(j = 0; j < index; j++)
	{
		regv = *((unsigned long *)data + j);    //register is four Byte, Byte->unsigned long
		REG32(TWI_DATA0 + (j * 4)) = regv;
	}
	if(0 != residual)
	{
		regv = *((unsigned long *)data + j); //register is four Byte, Byte->unsigned long
		REG32(TWI_DATA0 + (j * 4)) = regv;
	}
	
	/* set data number,enable TWI interrupt,set w/r bit, and start send*/
	regv = REG32(TWI_CTRL);
	regv &= ~((1<<14) | (1<<13) | (0xF<<9) | (1<<5) | (1<<4));
	regv |= ((1<<14)| (0<<13) | ((size-1)<<9) | (1<<5));
	REG32(TWI_CTRL) = regv;
	irq_unmask();

	/* wait for write finish */
	t1 = get_tick_count();
	do{
		regv = REG32(TWI_CTRL);
		t2 = get_tick_count();
	}while(((regv & (1<<4)) != (1<<4))&&(t2< OUT_TIME_MAX+t1));
	if(t2 > t1 + OUT_TIME_MAX)
	{
		return false;
	}
	
	
	/* wait 5 ms,for device operate data to register*/
	//AK_Sleep(1);
	return true;
}

/**
 * @brief write data to TWI device
 *
 * write size length data to daddr's raddr register, raddr is word width, data is byte width
 * @author Jiankui
 * @date 2016-09-12
 * @param[in] daddr TWI device address
 * @param[in] raddr register address
 * @param[in] *data write data's pointer
 * @param[in] size write data's length
 * @return bool return operation successful or failed
 * @retval false operation failed
 * @retval true operation successful
 */
bool twi_write_data2(unsigned char daddr, unsigned short raddr, unsigned char *data, unsigned long size)
{

	unsigned long regv, t1, t2;
	unsigned char j;
	unsigned long index, residual;
	unsigned char raddr_low,raddr_high;

	
	if(16 < size)
	{
		akprintf(C2, M_DRVSYS, "every send data must lower 16 Byte!\n");
		return false;
	}
	
	raddr_low = raddr & 0xFF;
	raddr_high = (raddr>>8)&0xFF;

	irq_mask();
	/* set start bit,daddr,cmd enable*/
	/* write device address*/
	regv = REG32(TWI_CMD1);
	regv =(1<<18) | (1<<17) | daddr;
	REG32(TWI_CMD1) = regv;

	/* write register high address*/
	regv = REG32(TWI_CMD2);
	regv = (1<<18) | raddr_high;
	REG32(TWI_CMD2) = regv;

	/* write register low address*/
	regv = REG32(TWI_CMD3);
	regv = (1<<18) | raddr_low;
	REG32(TWI_CMD3) = regv;

	/* write DATA to TWI DATA*/
	index = size/4;
	residual = size%4;
	regv = 0;
	for(j = 0; j < index; j++)
	{
		regv = *((unsigned long *)data + j); //register is four Byte, Byte->unsigned long
		REG32(TWI_DATA0 + (j * 4)) = regv;
	}
	regv = *((unsigned long *)data + j); //register is four Byte, Byte->unsigned long
	REG32(TWI_DATA0 + (j * 4)) = regv;
		
	/* set data number,enable TWI interrupt,set w/r bit, and start send*/
	regv = REG32(TWI_CTRL);
	regv &= ~((1<<14) | (1<<13) | (0xF<<9) | (1<<5) | (1<<4));
	regv |= ((1<<14)| (0<<13) | ((size-1)<<9) | (1<<5));
	REG32(TWI_CTRL) = regv;
	irq_unmask();
		
	/* wait for write finish */
	t1 = get_tick_count();
	do{
		regv = REG32(TWI_CTRL);
		t2 = get_tick_count();
	}while(((regv & (1<<4)) != (1<<4))&&(t2< OUT_TIME_MAX+t1));
	if(t2 > t1 + OUT_TIME_MAX)
	{
		return false;
	}
	
	
	/* wait 5 ms,for device operate data to register*/
	//AK_Sleep(1);
    return true;
}

/**
 * @brief write data to TWI device
 *
 * write size length data to daddr's raddr register, raddr and data is word width
 * @author Jiankui
 * @date 2016-09-12
 * @param[in] daddr TWI device address
 * @param[in] raddr register address
 * @param[in] *data write data's pointer
 * @param[in] size write data's length
 * @return bool return operation successful or failed
 * @retval false operation failed
 * @retval true operation successful
 */
bool twi_write_data3(unsigned char daddr, unsigned short raddr, unsigned short *data, unsigned long size)
{
 	unsigned long regv, t1, t2;
	unsigned char j;
	unsigned long index, residual;

	if(8 < size)
	{
		akprintf(C2, M_DRVSYS, "every send data must lower 16 Byte!\n");
		return false;
	}
	

	irq_mask();
	/* set start bit,daddr,cmd enable*/
	/* write device address*/
	regv = REG32(TWI_CMD1);
	regv =(1<<18) | (1<<17) | daddr;
	REG32(TWI_CMD1) = regv;

	/* write register address*/
	regv = REG32(TWI_CMD2);
	regv = (1<<18) | (1<<16) | raddr;
	REG32(TWI_CMD2) = regv;

	/* write DATA to TWI DATA*/
	index = size/2;
	residual = size%2;
	regv = 0;
	for(j = 0; j < index; j++)
	{
		regv = *((unsigned long *)data + j); //register is four Byte, Byte->unsigned long
		REG32(TWI_DATA0 + (j * 4)) = regv;
	}
	regv = *((unsigned long *)data + j); //register is four Byte, Byte->unsigned long
	REG32(TWI_DATA0 + (j * 4)) = regv;
	
	/* set data number,enable TWI interrupt,set w/r bit, and start send*/
	regv = REG32(TWI_CTRL);
	regv &= ~((1<<14) | (1<<13) | (0xF<<9) | (1<<5) | (1<<4));
	regv |= ((1<<14) | (0<<13) | ((2 * size - 1)<<9) | (1<<5));
	REG32(TWI_CTRL) = regv;
	irq_unmask();
	
	/* wait for write finish */
	t1 = get_tick_count();
	do{
		regv = REG32(TWI_CTRL);
		t2 = get_tick_count();
	}while(((regv & (1<<4)) != (1<<4))&&(t2< OUT_TIME_MAX+t1));
	if(t2 > t1 + OUT_TIME_MAX)
	{
		return false;
	}
	

	/* wait 5 ms,for device operate data to register*/
	//AK_Sleep(1);
    return true;
}

/**
 * @brief write data to TWI device
 *
 * write size length data to daddr's raddr register, raddr is not required, data is byte width
 * @author Jiankui
 * @date 2016-09-12
 * @param[in] daddr TWI device address
 * @param[in] *data write data's pointer
 * @param[in] size write data's length
 * @return bool return operation successful or failed
 * @retval false operation failed
 * @retval true operation successful
 */
bool twi_write_data4(unsigned char daddr, unsigned char *data, unsigned long size)
{ 
	unsigned long regv, t1, t2;
	unsigned char j;
	unsigned long index, residual;
	
    if(8 < size)
	{
		akprintf(C2, M_DRVSYS, "every send data must lower 16 Byte!\n");
		return false;
	}

	irq_mask();
	/* set start bit,daddr,cmd enable*/
	/* write device address*/
	regv = REG32(TWI_CMD1);
	regv =(1<<18) | (1<<17) | daddr;
	REG32(TWI_CMD1) = regv;

	/* write DATA to TWI DATA*/
	index = size/2;
	residual = size%2;
	regv = 0;
	for(j = 0; j < index; j++)
	{
		regv = *((unsigned long *)data + j); //register is four Byte, Byte->unsigned long
		REG32(TWI_DATA0 + (j * 4)) = regv;
	}
	regv = *((unsigned long *)data + j); //register is four Byte, Byte->unsigned long
	REG32(TWI_DATA0 + (j * 4)) = regv;
	
	/* set data number,enable TWI interrupt,set w/r bit, and start send*/
	regv = REG32(TWI_CTRL);
	regv &= ~((1<<14) | (1<<13) | (0xF<<9) | (1<<5) | (1<<4));
	regv |= ((1<<14)| (0<<13) | ((size-1)<<9) | (1<<5));
	REG32(TWI_CTRL) = regv;
	irq_unmask();
	
	/* wait for write finish */
	t1 = get_tick_count();
	do{
		regv = REG32(TWI_CTRL);
		t2 = get_tick_count();
	}while(((regv & (1<<4)) != (1<<4))&&(t2< OUT_TIME_MAX+t1));
	if(t2 > t1 + OUT_TIME_MAX)
	{
		return false;
	}
	

	/* wait 5 ms,for device operate data to register*/
	//AK_Sleep(1);
    return true;
}

/**
 * @brief read data from TWI device function
 *
 * read data from daddr's raddr register, raddr and data is byte width
 * @author Jiankui
 * @date 2016-09-12
 * @param[in] daddr TWI device address
 * @param[in] raddr register address
 * @param[out] *data read output data store address
 * @param[in] size read data size
 * @return bool return operation successful or failed
 * @retval false operation failed
 * @retval true operation successful
 */
bool twi_read_data(unsigned char daddr, unsigned char raddr, unsigned char *data, unsigned long size)
{
    unsigned long regv;
	unsigned char i;
	unsigned long index, residual, t1, t2;

	if(16 < size)
	{
		akprintf(C2, M_DRVSYS, "every send data must lower 16 Byte!\n");
		return false;
	}

	irq_mask();
	/* set start bit,daddr,cmd enable*/
	/* write device address*/
	regv = REG32(TWI_CMD1);
	regv =(1<<18) | (1<<17) | daddr;
	REG32(TWI_CMD1) = regv;

	/* write register address*/
	regv = REG32(TWI_CMD2);
	regv = (1<<18) | raddr;
	REG32(TWI_CMD2) = regv;

	/* write device address*/
	regv = REG32(TWI_CMD3);
	regv =(1<<18) | (1<<17) | daddr | 0x01;
	REG32(TWI_CMD3) = regv;	

	/* set data number,enable TWI interrupt,set w/r bit, and start send*/
	regv = REG32(TWI_CTRL);
	regv &= ~((1<<15) | (1<<14) | (1<<13) | (0xF<<9) | (1<<5) | (1<<4) | (3<<7));
	regv |= ((1<<15) | (1<<14)| (1<<13) | ((size-1)<<9) | (1<<5) | (3<<7));
	REG32(TWI_CTRL) = regv;
	irq_unmask();
	/* wait for write finish */
	t1 = get_tick_count();
	do{
		regv = REG32(TWI_CTRL);
		t2 = get_tick_count();
	}while(((regv & (1<<4)) != (1<<4))&&(t2< OUT_TIME_MAX+t1));
	if(t2 > t1 + OUT_TIME_MAX)
	{
		return false;
	}
	/* read DATA from TWI DATA*/
	index = size/4;
	residual = size%4;
	irq_mask();
	for(i = 0; i < index; i++)
	{
		long_to_char(data+(i*4), REG32(TWI_DATA0+(i*4)), 4);
	}
	long_to_char(data+(i*4), REG32(TWI_DATA0+(i*4)), residual);
	irq_unmask();
	
    return true;
}


/**
 * @brief read data from TWI device function
 *
 * read data from daddr's raddr register, raddr is word width, data is byte width
 * @author Jiankui
 * @date 2016-09-12
 * @param[in] daddr TWI device address
 * @param[in] raddr register address
 * @param[out] *data read output data store address
 * @param[in] size read data size
 * @return bool return operation successful or failed
 * @retval false operation failed
 * @retval true operation successful
 */
bool twi_read_data2(unsigned char daddr, unsigned short raddr, unsigned char *data, unsigned long size)
{
    unsigned long regv;
	unsigned char i;
	unsigned long index, residual, t1, t2;
	unsigned char raddr_low,raddr_high;

	if(16 < size)
	{
		akprintf(C2, M_DRVSYS, "every send data must lower 16 Byte!\n");
		return false;
	}
	
	raddr_low = raddr & 0xFF;
	raddr_high = (raddr>>8)&0xFF;
	//akprintf(C3, M_DRVSYS, "jiankui test twi read data2! %x,%x\n", raddr_high, raddr_low);
	irq_mask();
	/* set start bit,daddr,cmd enable*/
	/* write device address*/
	regv = REG32(TWI_CMD1);
	regv = (1<<18) | (1<<17) | daddr;
	REG32(TWI_CMD1) = regv;

	/* write register address*/
	regv = REG32(TWI_CMD2);
	regv = (1<<18) | raddr_high;
	REG32(TWI_CMD2) = regv;

	/* write register address*/
	regv = REG32(TWI_CMD3);
	regv = (1<<18)| raddr_low;
	REG32(TWI_CMD3) = regv;
	
	/* write device address*/
	regv = REG32(TWI_CMD4);
	regv =(1<<18) | (1<<17) | daddr | 0x01;
	//regv =(1<<18) | daddr | 0x01;
	REG32(TWI_CMD4) = regv;
	
	/* set data number,enable TWI interrupt,set w/r bit, and start send*/
	regv = REG32(TWI_CTRL);
	regv &= ~((1<<15) | (1<<14) | (1<<13) | (0xF<<9) | (1<<5) | (1<<4) | (3<<7));
	regv |= ((1<<15) | (1<<14)| (1<<13) | ((size-1)<<9) | (1<<5) | (3<<7));
	REG32(TWI_CTRL) = regv;
	irq_unmask();
	
	/* wait for write finish */
	t1 = get_tick_count();
	do{
		regv = REG32(TWI_CTRL);
		t2 = get_tick_count();
	}while(((regv & (1<<4)) != (1<<4))&&(t2< OUT_TIME_MAX+t1));
	if(t2 > t1 + OUT_TIME_MAX)
	{
		return false;
	}
	/* read DATA from TWI DATA*/
	index = size/4;
	residual = size%4;
	irq_mask();
	for(i = 0; i < index; i++)
	{
		long_to_char(data+(i*4), REG32(TWI_DATA0+(i*4)), 4);
	}
	long_to_char(data+(i*4), REG32(TWI_DATA0+(i*4)), residual);
	irq_unmask();
	
    return true;
}

/**
 * @brief read data from TWI device function
 *
 * read data from daddr's raddr register, raddr and data is word width
 * @author Jiankui
 * @date 2016-09-12
 * @param[in] daddr TWI device address
 * @param[in] raddr register address
 * @param[out] data read output data store address
 * @param[in] size read data size
 * @return bool return operation successful or failed
 * @retval false operation failed
 * @retval true operation successful
 */
bool twi_read_data3(unsigned char daddr, unsigned short raddr, unsigned short *data, unsigned long size)
{
    unsigned long regv;
	unsigned char i;
	unsigned long index, residual, t1, t2;

	if(8 < size)
	{
		akprintf(C2, M_DRVSYS, "every send data must lower 16 Byte!\n");
		return false;
	}

	irq_mask();
	/* set start bit,daddr,cmd enable*/
	/* write device address*/
	regv = REG32(TWI_CMD1);
	regv =(1<<18) | (1<<17) | daddr;
	REG32(TWI_CMD1) = regv;

	/* write register address*/
	regv = REG32(TWI_CMD2);
	regv = (1<<18) | (1<<16) | raddr;
	REG32(TWI_CMD2) = regv;

	/* write device address*/
	regv = REG32(TWI_CMD3);
	regv =(1<<18) | (1<<17) | daddr | 0x01;
	REG32(TWI_CMD3) = regv;
	
	/* set data number,enable TWI interrupt,set w/r bit, and start send*/	
	regv = REG32(TWI_CTRL);
	regv &= ~((1<<15) | (1<<14) | (1<<13 )| (0xF<<9) | (1<<5) | (1<<4) | (3<<7));
	regv |= ((1<<15) | (1<<14)| (1<<13) | (((size*2)-1)<<9) | (1<<5) | (3<<7));
	REG32(TWI_CTRL) = regv;
	irq_unmask();
	/* wait for write finish */
	t1 = get_tick_count();
	do{
		regv = REG32(TWI_CTRL);
		t2 = get_tick_count();
	}while(((regv & (1<<4)) != (1<<4))&&(t2< OUT_TIME_MAX+t1));
	if(t2 > t1 + OUT_TIME_MAX)
	{
		return false;
	}
	
	/* read DATA from TWI DATA*/
	index = size/2;
	residual = size%2;
	irq_mask();
	for(i = 0; i < index; i++)
	{
		long_to_char((unsigned char *)(data+(i*2)), REG32(TWI_DATA0+(i*4)), 4);
	}
	long_to_char((unsigned char *)(data+(i*2)), REG32(TWI_DATA0+(i*4)), residual*2);
	irq_unmask();
	
    return true;
}

/**
 * @brief read data from TWI device function
 *
 * read data from daddr's raddr register, raddr is not required, data is byte width
 * @author Jiankui
 * @date 2016-09-12
 * @param[in] daddr TWI device address
 * @param[out] data read output data store address
 * @param[in] size read data size
 * @return bool return operation successful or failed
 * @retval false operation failed
 * @retval true operation successful
 */
bool twi_read_data4(unsigned char daddr, unsigned char *data, unsigned long size)
{  
	unsigned long regv;
	unsigned char i;
	unsigned long index, residual, t1, t2;

	if(16 < size)
	{
		akprintf(C2, M_DRVSYS, "every send data must lower 16 Byte!\n");
		return false;
	}

	irq_mask();
    /* write device address*/
	regv = REG32(TWI_CMD1);
	regv =(1<<18) | (1<<17) | (daddr | 0x01);
	REG32(TWI_CMD1) = regv;
	
	/* set data number,enable TWI interrupt,set w/r bit, and start send*/
	regv = REG32(TWI_CTRL);
	regv &= ~((1<<15) | (1<<14) | (1<<13) | (0xF<<9) | (1<<5) | (1<<4) | (3<<7));
	regv |= ((1<<15) | (1<<14)| (1<<13) | ((size-1)<<9) | (1<<5) | (3<<7));
	REG32(TWI_CTRL) = regv;
	irq_unmask();
	
	/* wait for write finish */
	t1 = get_tick_count();
	do{
		regv = REG32(TWI_CTRL);
		t2 = get_tick_count();
	}while(((regv & (1<<4)) != (1<<4))&&(t2< OUT_TIME_MAX+t1));
	if(t2 > t1 + OUT_TIME_MAX)
	{
		return false;
	}
	
	/* read DATA from TWI DATA*/
	index = size/2;
	residual = size%2;
	irq_mask();
	for(i = 0; i < index; i++)
	{
		long_to_char((unsigned char *)(data+(i*2)), REG32(TWI_DATA0+(i*4)), 4);
	}
	long_to_char((unsigned char *)(data+(i*2)), REG32(TWI_DATA0+(i*4)), residual*2);
	irq_unmask();	
    return true;
}

#endif

/* end of file */
