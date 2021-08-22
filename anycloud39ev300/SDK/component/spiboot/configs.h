#ifndef __SPIBOOT_CONFIGS_H__
#define __SPIBOOT_CONFIGS_H__

#define SPI_BOOT_VERSION       "spiboot V1.1.00"

#define RAM_ADDR_START           	(0x80000000)

#define BOOT_TMP_BUFFER_ADDR   		0x81000000
#define SPIBOOT_DATA_START_PAGE   	(559) //545//65 /*33 */

#define UIMAGE_HEADER_LEN 			(0x40)

#define SPIBOOT_PARAM_ADDR_BASE		(0x48000200)
#define SPIBOOT_PARAM_LEN		  	(512)


#endif
