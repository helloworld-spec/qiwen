
/**
 * @file dev_drv.h
 * @brief: Implement  operations of how to use devices driver.
 *
 * This file describe devices ioctl command and how to use the devices.
 * Copyright (C) 2017 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  KeJianping
 * @date 2017-2-09
 * @version 1.0
 */

#include "ioctl.h"

#ifndef  __DEV_INFO_H__
#define  __DEV_INFO_H__

#ifdef __cplusplus
extern "C"{
#endif

#define LIBDEV_DRV_VERSION "libdev_drv_1.4.00"


//device  names   
#define DEV_UART_1              "uart1"
#define DEV_SPI_1               "spi1"
#define DEV_LED_1               "led1"
#define DEV_WIFI                "wifi"
#define DEV_KEYPAD              "keypad"
#define DEV_SPEAKER             "speaker"
#define DEV_CAMERA              "camera"
#define DEV_USBDISK             "usbdisk"
#define DEV_TCARD				"tcard"
#define DEV_I2C					"i2c"
/*
*device ioctl command
*/

//block command
#define CMD_BLOCK 'b'

#define BLOCK_NO      0  // unblock function
#define BLOCK_WAIT    1  // block function
#define BLOCK_MS      2 //wait ms 

#define IO_BLOCK_NO      _DEVIO(CMD_BLOCK,BLOCK_NO)  // unblock command
#define IO_BLOCK_WAIT    _DEVIO(CMD_BLOCK,BLOCK_WAIT)  // block command
#define IO_BLOCK_MS      _DEVIOW(CMD_BLOCK,BLOCK_MS,int) //wait for MS(unit :ms)

//mask command
#define CMD_MASK 'm'

#define MASK_DISABLE  	0
#define MASK_ENABLE		1

#define IO_MASK_DISABLE  _DEVIO(CMD_MASK, MASK_DISABLE)
#define IO_MASK_ENABLE   _DEVIO(CMD_MASK, MASK_ENABLE)

//led command 
#define CMD_LED 'l'

#define LED_CTL 0
#define LED_BLINK 1
#define LED_GETNUM 2

#define IO_LED_CTL	    _DEVIOW(CMD_LED, LED_CTL,int)
#define IO_LED_BLINK	_DEVIOW(CMD_LED, LED_BLINK,int)
#define IO_LED_GETNUM	_DEVIOR(CMD_LED, LED_GETNUM,char)


//i2c command
#define CMD_I2C 'i'


#define I2C_BYTE_WRITE 	0
#define I2C_BYTE_READ  	1
#define I2C_WORD_WRITE  2
#define I2C_WORD_READ   3

#define IO_I2C_BYTE_WRITE		_DEVIOW(CMD_I2C, I2C_BYTE_WRITE, int)
#define IO_I2C_BYTE_READ		_DEVIOW(CMD_I2C, I2C_BYTE_READ, int)
#define IO_I2C_WORD_WRITE		_DEVIOW(CMD_I2C, I2C_WORD_WRITE, int)
#define IO_I2C_WORD_READ		_DEVIOW(CMD_I2C, I2C_WORD_READ, int)


//uart command
#define CMD_UART 'u'

#define UART_BAUD_RATE      1  // set baudrate function
#define UART_PARITY         2  // set parity function

#define IO_UART_BAUD_RATE     _DEVIOW(CMD_UART,UART_BAUD_RATE,int) //set baudrate command
#define IO_UART_PARITY        _DEVIOW(CMD_UART,UART_PARITY,int) //set parity command

//TFcard command
#define CMD_TFCARE 't'

#define TFCARE_GET_DATA      1  // Get data

#define IO_TFCARE_GET_PLATFORM_DATA     _DEVIOR(CMD_TFCARE,TFCARE_GET_DATA,int) //Get platform device data


//spi command
#define CMD_SPI 's'

#define SPI_BUFFER_SIZE                1  // set baudrate function
#define SPI_BUFFER_NB                  2  // 
#define SPI_WR_BUF                     3  // 
#define SPI_ROLE_GET                   4  //
#define SPI_SLACE_DATA_SIZ_GET         5  //

#define IO_SPI_BUFFER_SIZE          _DEVIOW(CMD_SPI,SPI_BUFFER_SIZE,int) //set baudrate command
#define IO_SPI_BUFFER_NB            _DEVIOW(CMD_SPI,SPI_BUFFER_NB,int) //set parity command
#define IO_SPI_WR_BUF               _DEVIOW(CMD_SPI,SPI_WR_BUF,int) //set parity command
#define IO_SPI_ROLE_GET	            _DEVIOR(CMD_SPI, SPI_ROLE_GET, int) // open irfeed gpio
#define IO_SPI_SLACE_DATA_SIZ_GET	_DEVIOR(CMD_SPI, SPI_SLACE_DATA_SIZ_GET, int) // get irfeed gpio logic level


//camera command
#define CMD_CAMERA 'c'

#define CAMERA_IRFEED_GPIO_OPEN		1  // open irfeed gpio
#define CAMERA_IRFEED_GPIO_GET		2  // get irfeed gpio logic level
#define CAMERA_IRFEED_GPIO_CLOSE	3  // close irfeed gpio

#define IO_CAMERA_IRFEED_GPIO_OPEN	_DEVIOR(CMD_CAMERA, CAMERA_IRFEED_GPIO_OPEN, int) // open irfeed gpio
#define IO_CAMERA_IRFEED_GPIO_GET	_DEVIOR(CMD_CAMERA, CAMERA_IRFEED_GPIO_GET, int) // get irfeed gpio logic level
#define IO_CAMERA_IRFEED_GPIO_CLOSE	_DEVIOR(CMD_CAMERA, CAMERA_IRFEED_GPIO_CLOSE, int) // close irfeed gpio

//camera->irfeed gpio status
#define CAMERA_IRFEED_GPIO_STATUS_DAY		0
#define CAMERA_IRFEED_GPIO_STATUS_NIGHT		1

#ifdef __cplusplus
}
#endif

#endif  //#ifndef  __DEV_DRV_H__


