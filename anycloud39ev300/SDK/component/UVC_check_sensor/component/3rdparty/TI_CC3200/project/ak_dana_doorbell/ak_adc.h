#ifndef __AK_ADC_H__
#define __AK_ADC_H__
/**
init adc channel
@parm adc_chanel:
    1 : gpio 3  pin 58
    2 : gpio 4  pin 59 
    3 : gpio 5  pin 60
@returnval
    0  :success
    -1:fail
*/
int ak_adc_init(unsigned char adc_channel);
/**
init adc channel
@parm adc_chanel:
    1 : gpio 3  pin 58
    2 : gpio 4  pin 59 
    3 : gpio 5  pin 60
@returnval :adc value unit :mv
    -1:no adc value
@note:  must after ak_adc_init
*/
unsigned long ak_getValue_adc(unsigned char adc_channel);

#endif

