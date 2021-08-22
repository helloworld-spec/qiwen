/**
* @FILENAME adc.h
*
* Copyright (C) 2008 Anyka (Guangzhou) Software Technology Co., LTD
* @DATE  2010-08-21
* @VERSION 1.0
* @REF 
*/

#ifndef __ADC_H__
#define __ADC_H__

#define ADC_X ((REG32(X_COORDINATE_REG)>>10) & 0x000003ff)
#define ADC_Y ((REG32(Y_COORDINATE_REG)>>10) & 0x000003ff)

//CLK_DIV_REG2
#define ADC2_DIV         4
#define ADC2_CLK_EN      (1 << 12)
#define ADC2_RST         (1 << 19)
#define ADC2_GATE        (1 << 25)

//ADC2_MODE_CFG: 0x20072000
#define WORD_LENGTH_MASK (0XF << 8)
#define I2S_EN           (1 << 4)
#define CH_POLARITY_SEL  (1 << 3)
#define HOST_RD_INT_EN   (1 << 2)
#define L2_EN            (1 << 1)
#define ADC2_CTRL_EN     (1 << 0)

//ANALOG_CONTROL_REG4
#define PL_VCM3          (1 << 2)  //pull down vcm3 with a 2Kohm resistor to ground
#define PD_ADC2          (1 << 26)  //power off adc2
#define PD_LINEIN        (1 << 21) //power off linein
#define ADC_LIM          (1 << 21) //limit the amplitude of adc2 input voltage between 0 to vcm3
#define PD_S2D           (1 << 22) //power off single differential conversion
#define PD_MICP          (1 << 19) //power off mono mic
#define PD_MICN          (1 << 20) //power off differential mic

#define VCM3_CTRL        (1 << 22)
#define LINE_IN          (2 << 12)
#define PTM_CHG          (16)
#define PD_VW            (1 << 15)

#define ADC2_IN          (29)

//ANALOG_CONTROL_REG2
#define ADC_OSR           12

#endif //__ADC_H__
