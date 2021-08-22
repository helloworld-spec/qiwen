#include "rom_map.h"
#include "hw_types.h"
#include "pin.h"
#include "adc.h"
#include "hw_memmap.h"

#include "ak_adc.h"

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
int ak_adc_init(unsigned char adc_channel)
{
	unsigned long pin_num;
	unsigned long uiChannel;
	
	switch(adc_channel)
    {
            case 1:
				pin_num = PIN_58;
                uiChannel = ADC_CH_1;
                break;
            case 2:
				pin_num = PIN_59;
                uiChannel = ADC_CH_2;
                break;
            case 3:
				pin_num = PIN_60;
                uiChannel = ADC_CH_3;
                break;
            default:
				return -1;
                break;
     }
	
	MAP_PinTypeADC(pin_num,PIN_MODE_255);
	
		 //
    // Configure ADC timer which is used to timestamp the ADC data samples
    //
    MAP_ADCTimerConfig(ADC_BASE,2^17);

    //
    // Enable ADC timer which is used to timestamp the ADC data samples
    //
    MAP_ADCTimerEnable(ADC_BASE);

    //
    // Enable ADC module
    //
    MAP_ADCEnable(ADC_BASE);

	
	
	return 0;
}

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
unsigned long ak_getValue_adc(unsigned char adc_channel)
{
	unsigned long uiChannel;
	unsigned long ulSample;
	
	switch(adc_channel)
    {
            case 1:
                uiChannel = ADC_CH_1;
                break;
            case 2:
                uiChannel = ADC_CH_2;
                break;
            case 3:
                uiChannel = ADC_CH_3;
                break;
            default:
				return -1;
                break;
     }
	MAP_ADCChannelEnable(ADC_BASE, uiChannel);
	
	if(MAP_ADCFIFOLvlGet(ADC_BASE, uiChannel))
	{
		ulSample = MAP_ADCFIFORead(ADC_BASE, uiChannel);
		return ((ulSample >> 2 ) & 0xFFF)*1460/4096;
					
	}
	return -1;
}

int ak_adc_realse(unsigned char adc_channel)
{
	unsigned long uiChannel;

	switch(adc_channel)
    {
            case 1:
                uiChannel = ADC_CH_1;
                break;
            case 2:
                uiChannel = ADC_CH_2;
                break;
            case 3:
                uiChannel = ADC_CH_3;
                break;
            default:
				return -1;
                break;
     }
	  MAP_ADCChannelDisable(ADC_BASE, uiChannel);
}

