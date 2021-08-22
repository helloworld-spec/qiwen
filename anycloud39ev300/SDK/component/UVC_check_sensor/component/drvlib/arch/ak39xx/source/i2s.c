/**
 * @file i2s.c
 * @brief the source code of I2S controller
 * This file is the source code of I2S controller
 * Copyright (C) 2010 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author Liao_Zhijun
 * @date 2010-11-30
 * @version 1.0
 */

#include "anyka_types.h"
#include "anyka_cpu.h"
#include "drv_api.h"
#include "hal_sound.h"
#include "dac.h"
#include "sysctl.h"
#include "drv_gpio.h"

/**
 * @brief  open a i2s send device 
 * @author Liao_Zhijun
 * @date   2010-11-30
 * @return bool
 * @retval true  open successful
 * @retval false open failed
 */
bool i2s_send_open(void)
{
    unsigned long reg_value;

    akprintf(C3, "", "i2s send open\n");

    //enable dac
    sysctl_clock(CLOCK_DAC_ENABLE);

    //reset dac controller
    sysctl_reset(RESET_DAC);

    //set dac clock as M clock
    REG32(ADC_CLK_DIV) &= ~(0x3 << 27);
    REG32(ADC_CLK_DIV) |= (0x2 << 27);

    //enable and provide dac clock
    REG32(ADC_CLK_DIV) &= ~(1 << 26);
    REG32(ADC_CLK_DIV) |= (1 << 21);
    
    //reset dacs
    REG32(ADC_CLK_DIV) &= ~(1<<24);
    REG32(ADC_CLK_DIV) |= (1<<24);

    //enable l2
    REG32(DAC_CONFIG_REG) |= (1<<1);

    //enable dac
    REG32(DAC_CONFIG_REG) |= (1<<0);

    //set mem format
    REG32(DAC_CONFIG_REG) |= (1<<4);

    //set channel polarity
    REG32(I2S_CONFIG_REG) |= (1<<5);

    //set work length to 24
    REG32(I2S_CONFIG_REG) &= ~(0x1f << 0);
    REG32(I2S_CONFIG_REG) |= 24; 

    //enable mode
    REG32(MUL_FUN_CTL_REG) &= ~(3 << 24);
    REG32(MUL_FUN_CTL_REG) |= (1 << 24); 

    //enable extern dac
    REG32(MUL_FUN_CTL_REG) &= ~(1 << 23);

    //share pin
    gpio_pin_group_cfg(ePIN_AS_I2S);

    return true;
}

/**
 * @brief   Close a i2s send device
 * @author  Liao_Zhijun
 * @date    2010-11-30
 * @return  bool
 * @retval  true close successful
 * @retval  false close failed
 */
bool i2s_send_close(void)
{
    unsigned long reg_value;

    sysctl_clock(~CLOCK_DAC_ENABLE);

    //disable dac & l2
    REG32(DAC_CONFIG_REG) &= ~((1<<1) | (1<<0));

    //disable external adc/dac
    REG32(0x08000058) |= (1 << 23);

    //disable dac clock
    REG32(ADC_CLK_DIV) &= ~(1 << 21);

    //inhibot dac clock
    REG32(ADC_CLK_DIV) |= (1<<26);

    return true;
}

/**
 * @brief  open a i2s send device 
 * @author Liao_Zhijun
 * @date   2010-11-30
 * @return bool
 * @retval true  open successful
 * @retval false open failed
 */
bool i2s_recv_open(void)
{
    unsigned long reg_value;

    //reset adc2/3
    REG32(0x08000008) &= ~(1 << 23);
    REG32(0x08000008) |= (1 << 23);

    //soft reset adc2
    sysctl_reset(RESET_ADC2);

    //enable adc2 working clock
    sysctl_clock(CLOCK_ADC2_ENABLE);

    //enable adc2 clock
    REG32(0x08000008) |= (1 << 12);

    //provide adc2 clock
    REG32(0x08000008) &= ~(1 << 25);

    REG32(0x2002d000) = 0;
    REG32(0x2002d000) |= (1 << 4); 
    REG32(0x2002d000) |= (1 << 3); 
    REG32(0x2002d000) |= (1 << 1); 
    REG32(0x2002d000) |= (1 << 0); 
    REG32(0x2002d000) |= (0xf << 8); 
    
    //enable mode
    REG32(0x08000058) &= ~(3 << 24);
    REG32(0x08000058) |= (1 << 25); 

    //enable external adc
    REG32(0x08000058) &= ~(1 << 23);

    //share pin
    gpio_pin_group_cfg(ePIN_AS_I2S);

    return true;
}

/**
 * @brief   Close a i2s send device
 * @author  Liao_Zhijun
 * @date    2010-11-30
 * @return  bool
 * @retval  true close successful
 * @retval  false close failed
 */
bool i2s_recv_close(void)
{
    unsigned long reg_value;

    //disable adc2 interface clock    
    sysctl_clock(~CLOCK_ADC2_ENABLE);

    //disable adc2 clock
    REG32(0x08000008) &= ~(1 << 12);

    //inhibit adc2 clock
    REG32(0x08000008) |= (1 << 25);

    //disable external adc/dac
    REG32(0x08000058) |= (1 << 23);

    return true;
}

/**
 * @brief   Set sound sample rate, channel, bits per sample of the sound device
 * @author  Liao_Zhijun
 * @date    2010-11-30
 * @param[in] info     refer to SOUND_INFO
 * @return  bool
 * @retval  true set successful
 * @retval  false set failed
 */
bool i2s_send_setinfo(SOUND_INFO *info)
{
    SOUND_INFO tInfo;

    tInfo.BitsPerSample = info->BitsPerSample;
    tInfo.nChannel = info->nChannel;

    if(info->BitsPerSample == 8)
        tInfo.nSampleRate =info->nSampleRate;
    else
        tInfo.nSampleRate =info->nSampleRate*2;

    return dac_setinfo(&tInfo);
}

/**
 * @brief   Set adc sample rate, channel, bits per sample of the sound device
 * @author  Liao_Zhijun
 * @date    2010-11-28
 * @param[in] info     refer to SOUND_INFO
 * @return  bool
 * @retval  true set successful
 * @retval  false set failed
 */
bool i2s_recv_setinfo(SOUND_INFO *info)
{
    SOUND_INFO tInfo;

    tInfo.BitsPerSample = info->BitsPerSample;
    tInfo.nChannel = info->nChannel;
    
    if(info->BitsPerSample == 8)
        tInfo.nSampleRate =info->nSampleRate;
    else
        tInfo.nSampleRate =info->nSampleRate*2;

    return adc_setinfo(&tInfo);
}


