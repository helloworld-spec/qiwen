//*****************************************************************************
// pinmux.h 
// 
// Header file for pinmuxconfig
// 
// Copyright (C) 2014 Texas Instruments Incorporated - http://www.ti.com/ 
// 
// 
//  Redistribution and use in source and binary forms, with or without 
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

//*****************************************************************************
// This file was automatically generated on 7/21/2014 at 3:06:20 PM
// by TI PinMux version 3.0.334
//
//*****************************************************************************

#ifndef __PINMUX_H__
#define __PINMUX_H__

	

#define GPIO_04                 4   /* wake up and ring call gpio */
#define GPIO_08                 8   /*ak39 power control gpio*/
#define GPIO_10					10	/*WiFi led*/
#define GPIO_28                 28	/*SPI slave control*/
#define GPIO_30					30  /*call key*/

	


#define GPIO_PIR         GPIO_04
#define GPIO_POWER_AK    GPIO_08
#define GPIO_LED1		 GPIO_10
#define GPIO_SPI_SIG	 GPIO_28
#define GPIO_KEY         GPIO_30


#ifdef DEBUG_GPIO
#define GPIO_10                    10 /*LPDS indication gpio with led*/
#endif


void PinMuxConfig(void);


#endif //  __PINMUX_H__
