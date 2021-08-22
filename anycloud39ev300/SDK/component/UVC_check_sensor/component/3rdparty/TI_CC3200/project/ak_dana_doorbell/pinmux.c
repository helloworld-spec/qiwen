//*****************************************************************************
// pinmux.c
//
// configure the device pins for different peripheral signals
//
// Copyright (C) 2014 Texas Instruments Incorporated - http://www.ti.com/ 
// 
// 
//  Redistribution and use in source and binary forms, with or without 
//  modification, are permitted provided that the following conditions 
//  are met:
//
//    Redistributions of source code must retain the above copyright 
//    notice, this list of conditions and the following disclaimer.
//
//    Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the 
//    documentation and/or other materials provided with the   
//    distribution.
//
//    Neither the name of Texas Instruments Incorporated nor the names of
//    its contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
//  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
//  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
//  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
//  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
//  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
//  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
//  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
//  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
//  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
//  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//*****************************************************************************

// This file was automatically generated on 7/21/2014 at 3:06:20 PM
// by TI PinMux version 3.0.334
//
//*****************************************************************************

#include "pinmux.h"
#include "hw_types.h"
#include "hw_memmap.h"
#include "hw_gpio.h"
#include "pin.h"
#include "rom.h"
#include "rom_map.h"
#include "gpio.h"
#include "prcm.h"

#define I2C_FUN


//*****************************************************************************
void
PinMuxConfig(void)
{
	/*----------------for----UART------------*/
    #ifndef NOTERM
    MAP_PRCMPeripheralClkEnable(PRCM_UARTA0, PRCM_RUN_MODE_CLK);
	#endif
#if 1  //jk
		//
		// Configure PIN_55 for UART0 UART0_TX
		//
		MAP_PinTypeUART(PIN_55, PIN_MODE_3);
	
		//
		// Configure PIN_57 for UART0 UART0_RX
		//
		MAP_PinTypeUART(PIN_57, PIN_MODE_3);
		
#else
	
		//
		// Configure PIN_03 for UART0 UART0_TX
		//
		MAP_PinTypeUART(PIN_03, PIN_MODE_7);
	
		//
		// Configure PIN_04 for UART0 UART0_RX
		//
		MAP_PinTypeUART(PIN_04, PIN_MODE_7);
#endif

	

    
	
 /*----------------for----I2C------------*/
#ifdef I2C_FUN  //jk
	MAP_PRCMPeripheralClkEnable(PRCM_I2CA0, PRCM_RUN_MODE_CLK);

	MAP_PinTypeGPIO(PIN_01, PIN_MODE_0, false);
	MAP_PinTypeGPIO(PIN_02, PIN_MODE_0, false);
	   
	//
    // Configure PIN_3 for I2C CLK
    //
	MAP_PinTypeI2C(PIN_03, PIN_MODE_5);

	//
    // Configure PIN_4 for I2C SDA
    //
	MAP_PinTypeI2C(PIN_04, PIN_MODE_5);
#endif


/*----------------for----GPIO------------*/
	MAP_PRCMPeripheralClkEnable(PRCM_GPIOA0, PRCM_RUN_MODE_CLK);
	MAP_PRCMPeripheralClkEnable(PRCM_GPIOA1, PRCM_RUN_MODE_CLK);
	MAP_PRCMPeripheralClkEnable(PRCM_GPIOA3, PRCM_RUN_MODE_CLK);
    //
    // GPIO04  PIN_59 for  Input 
    //
    MAP_PinTypeGPIO(PIN_59, PIN_MODE_0, false);
	//TODO: 0x10 mean ?
    //MAP_GPIODirModeSet(GPIOA0_BASE, 0x10, GPIO_DIR_MODE_IN);	

	//GPIO08  PIN_63
    MAP_PinTypeGPIO(PIN_63, PIN_MODE_0, false);
   // MAP_GPIODirModeSet(GPIOA1_BASE, 0x01, GPIO_DIR_MODE_OUT);

	//GPIO10  PIN_01
    MAP_PinTypeGPIO(PIN_01, PIN_MODE_0, false);
    //MAP_GPIODirModeSet(GPIOA1_BASE, 0x03, GPIO_DIR_MODE_OUT);
   // MAP_GPIODirModeSet(GPIOA1_BASE, 0x04, GPIO_DIR_MODE_OUT);

	//GPIO28  PIN_18
    MAP_PinTypeGPIO(PIN_18, PIN_MODE_0, false);
   // MAP_GPIODirModeSet(GPIOA1_BASE, 0x10, GPIO_DIR_MODE_IN);

   //GPIO30 PIN_53
   MAP_PinTypeGPIO(PIN_53, PIN_MODE_0, false);

/*----------for-----SPI FUNCTION--------------------*/

	//MAP_PRCMPeripheralClkEnable(PRCM_GSPI, PRCM_RUN_MODE_CLK);

	//
    // Configure PIN_05 for SPI0 GSPI_CLK
    //
    MAP_PinTypeSPI(PIN_05, PIN_MODE_7);

    //
    // Configure PIN_06 for SPI0 GSPI_MISO
    //
    MAP_PinTypeSPI(PIN_06, PIN_MODE_7);

    //
    // Configure PIN_07 for SPI0 GSPI_MOSI
    //
    MAP_PinTypeSPI(PIN_07, PIN_MODE_7);

    //
    // Configure PIN_08 for SPI0 GSPI_CS
    //
    MAP_PinTypeSPI(PIN_08, PIN_MODE_7);
}
