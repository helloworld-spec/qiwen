/** @file moal_sdio.h
  *
  * @brief This file contains definitions for SDIO interface.
  * driver. 
  *
  * Copyright (C) 2008-2011, Marvell International Ltd. 
  * 
  * This software file (the "File") is distributed by Marvell International 
  * Ltd. under the terms of the GNU General Public License Version 2, June 1991 
  * (the "License").  You may use, redistribute and/or modify this File in 
  * accordance with the terms and conditions of the License, a copy of which 
  * is available by writing to the Free Software Foundation, Inc.,
  * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA or on the
  * worldwide web at http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt.
  *
  * THE FILE IS DISTRIBUTED AS-IS, WITHOUT WARRANTY OF ANY KIND, AND THE 
  * IMPLIED WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE 
  * ARE EXPRESSLY DISCLAIMED.  The License provides additional details about 
  * this warranty disclaimer.
  *
  */
/****************************************************
Change log:
****************************************************/

#ifndef	_MOAL_SDIO_H
#define	_MOAL_SDIO_H
/*
#include        <linux/mmc/sdio.h>
#include        <linux/mmc/sdio_ids.h>
#include        <linux/mmc/sdio_func.h>
#include        <linux/mmc/card.h>
#include        <linux/mmc/host.h>
*/

#include "moal_main.h"

#ifndef BLOCK_MODE
/** Block mode */
#define BLOCK_MODE	1
#endif

#ifndef BYTE_MODE
/** Byte Mode */
#define BYTE_MODE	0
#endif

#ifndef FIXED_ADDRESS
/** Fixed address mode */
#define FIXED_ADDRESS	0
#endif

#ifdef STA_SUPPORT
/** Default firmware name */

#define DEFAULT_FW_NAME	"mrvl/sd8797_uapsta.bin"
#define DEFAULT_FW_NAME_8782 "mrvl/sd8782_uapsta.bin"

#ifndef DEFAULT_FW_NAME
#define DEFAULT_FW_NAME ""
#endif
#endif /* STA_SUPPORT */

#ifdef UAP_SUPPORT
/** Default firmware name */

#define DEFAULT_AP_FW_NAME "mrvl/sd8797_uapsta.bin"
#define DEFAULT_AP_FW_NAME_8782 "mrvl/sd8782_uapsta.bin"

#ifndef DEFAULT_AP_FW_NAME
#define DEFAULT_AP_FW_NAME ""
#endif
#endif /* UAP_SUPPORT */

/** Default firmaware name */

#define DEFAULT_AP_STA_FW_NAME "mrvl/sd8797_uapsta.bin"
#define DEFAULT_AP_STA_FW_NAME_8782 "mrvl/sd8782_uapsta.bin"

#ifndef DEFAULT_AP_STA_FW_NAME
#define DEFAULT_AP_STA_FW_NAME ""
#endif

#define SD_IO_SEND_OP_COND          5 /* bcr  [23:0] OCR         R4  */
#define SD_IO_RW_DIRECT            52 /* ac   [31:0] See below   R5  */
#define SD_IO_RW_EXTENDED          53 /* adtc [31:0] See below   R5  */

#define MMC_CARD_BUSY	0x80000000	/* Card Power up status bit */


struct sdio_cccr {
	unsigned int		sdio_vsn;
	unsigned int		sd_vsn;
	unsigned int		multi_block:1,
	low_speed:1,
	wide_bus:1,
	high_power:1,
	high_speed:1,
	disable_cd:1;
};

#define R5_COM_CRC_ERROR	(1 << 15)	/* er, b */
#define R5_ILLEGAL_COMMAND	(1 << 14)	/* er, b */
#define R5_ERROR		(1 << 11)	/* erx, c */
#define R5_FUNCTION_NUMBER	(1 << 9)	/* er, c */
#define R5_OUT_OF_RANGE		(1 << 8)	/* er, c */
#define R5_STATUS(x)		(x & 0xCB00)
#define R5_IO_CURRENT_STATE(x)	((x & 0x3000) >> 12) /* s, b */

	
#define SDIO_CCCR_CCCR		0x00
	
#define  SDIO_CCCR_REV_1_00	0	/* CCCR/FBR Version 1.00 */
#define  SDIO_CCCR_REV_1_10	1	/* CCCR/FBR Version 1.10 */
#define  SDIO_CCCR_REV_1_20	2	/* CCCR/FBR Version 1.20 */
	
#define  SDIO_SDIO_REV_1_00	0	/* SDIO Spec Version 1.00 */
#define  SDIO_SDIO_REV_1_10	1	/* SDIO Spec Version 1.10 */
#define  SDIO_SDIO_REV_1_20	2	/* SDIO Spec Version 1.20 */
#define  SDIO_SDIO_REV_2_00	3	/* SDIO Spec Version 2.00 */
	
#define SDIO_CCCR_SD		0x01
	
#define  SDIO_SD_REV_1_01	0	/* SD Physical Spec Version 1.01 */
#define  SDIO_SD_REV_1_10	1	/* SD Physical Spec Version 1.10 */
#define  SDIO_SD_REV_2_00	2	/* SD Physical Spec Version 2.00 */
	
#define SDIO_CCCR_IOEx		0x02
#define SDIO_CCCR_IORx		0x03
	
#define SDIO_CCCR_IENx		0x04	/* Function/Master Interrupt Enable */
#define SDIO_CCCR_INTx		0x05	/* Function Interrupt Pending */
	
#define SDIO_CCCR_ABORT		0x06	/* function abort/card reset */
	
#define SDIO_CCCR_IF		0x07	/* bus interface controls */
	
#define  SDIO_BUS_WIDTH_1BIT	0x00
#define  SDIO_BUS_WIDTH_4BIT	0x02
#define  SDIO_BUS_ECSI		0x20	/* Enable continuous SPI interrupt */
#define  SDIO_BUS_SCSI		0x40	/* Support continuous SPI interrupt */
	
#define  SDIO_BUS_ASYNC_INT	0x20
	
#define  SDIO_BUS_CD_DISABLE     0x80	/* disable pull-up on DAT3 (pin 1) */
	
#define SDIO_CCCR_CAPS		0x08
	
#define  SDIO_CCCR_CAP_SDC	0x01	/* can do CMD52 while data transfer */
#define  SDIO_CCCR_CAP_SMB	0x02	/* can do multi-block xfers (CMD53) */
#define  SDIO_CCCR_CAP_SRW	0x04	/* supports read-wait protocol */
#define  SDIO_CCCR_CAP_SBS	0x08	/* supports suspend/resume */
#define  SDIO_CCCR_CAP_S4MI	0x10	/* interrupt during 4-bit CMD53 */
#define  SDIO_CCCR_CAP_E4MI	0x20	/* enable ints during 4-bit CMD53 */
#define  SDIO_CCCR_CAP_LSC	0x40	/* low speed card */
#define  SDIO_CCCR_CAP_4BLS	0x80	/* 4 bit low speed card */
	
#define SDIO_CCCR_CIS		0x09	/* common CIS pointer (3 bytes) */
	
	/* Following 4 regs are valid only if SBS is set */
#define SDIO_CCCR_SUSPEND	0x0c
#define SDIO_CCCR_SELx		0x0d
#define SDIO_CCCR_EXECx		0x0e
#define SDIO_CCCR_READYx	0x0f
	
#define SDIO_CCCR_BLKSIZE	0x10
	
#define SDIO_CCCR_POWER		0x12
	
#define  SDIO_POWER_SMPC	0x01	/* Supports Master Power Control */
#define  SDIO_POWER_EMPC	0x02	/* Enable Master Power Control */
	
#define  SDIO_CCCR_SPEED		0x13
	
#define  SDIO_SPEED_SHS		0x01	/* Supports High-Speed mode */
#define  SDIO_SPEED_EHS		0x02	/* Enable High-Speed mode */
	
	/*
	 * Function Basic Registers (FBR)
	 */
	
#define SDIO_FBR_BASE(f)	((f) * 0x100) /* base of function f's FBRs */
	
#define SDIO_FBR_STD_IF		0x00
	
#define  SDIO_FBR_SUPPORTS_CSA	0x40	/* supports Code Storage Area */
#define  SDIO_FBR_ENABLE_CSA	0x80	/* enable Code Storage Area */
	
#define SDIO_FBR_STD_IF_EXT	0x01
	
#define SDIO_FBR_POWER		0x02
	
#define  SDIO_FBR_POWER_SPS	0x01	/* Supports Power Selection */
#define  SDIO_FBR_POWER_EPS	0x02	/* Enable (low) Power Selection */
	
#define SDIO_FBR_CIS		0x09	/* CIS pointer (3 bytes) */
	
	
#define SDIO_FBR_CSA		0x0C	/* CSA pointer (3 bytes) */
	
#define SDIO_FBR_CSA_DATA	0x0F
	
#define SDIO_FBR_BLKSIZE	0x10	/* block size (2 bytes) */



#define IF_SDIO_IOPORT		0x00

#define IF_SDIO_H_INT_MASK	0x04
#define   IF_SDIO_H_INT_OFLOW	0x08
#define   IF_SDIO_H_INT_UFLOW	0x04
#define   IF_SDIO_H_INT_DNLD	0x02
#define   IF_SDIO_H_INT_UPLD	0x01

#define IF_SDIO_H_INT_STATUS	0x05
#define IF_SDIO_H_INT_RSR	       0x06
#define IF_SDIO_H_INT_STATUS2	0x07

#define IF_SDIO_RD_BASE		0x10

#define IF_SDIO_STATUS		0x20
#define   IF_SDIO_IO_RDY	0x08
#define   IF_SDIO_CIS_RDY	0x04
#define   IF_SDIO_UL_RDY	0x02
#define   IF_SDIO_DL_RDY	0x01

#define IF_SDIO_C_INT_MASK	0x24
#define IF_SDIO_C_INT_STATUS	0x28
#define IF_SDIO_C_INT_RSR	0x2C

#define IF_SDIO_SCRATCH		0x34
#define IF_SDIO_SCRATCH_OLD	0x80fe
#define IF_SDIO_FW_STATUS	0x40
#define   IF_SDIO_FIRMWARE_OK	0xfedc

#define IF_SDIO_RX_LEN		0x42
#define IF_SDIO_RX_UNIT		0x43

#define IF_SDIO_EVENT           0x80fc

#define IF_SDIO_BLOCK_SIZE	256
#define CONFIGURATION_REG               0x03
#define HOST_POWER_UP                   (0x1U << 1)


/********************************************************
		Global Functions
********************************************************/
/** Function to update the SDIO card type */
t_void woal_sdio_update_card_type(moal_handle * handle, t_void * card);

/** Function to write register */
mlan_status woal_write_reg(moal_handle * handle, t_u32 reg, t_u32 data);
/** Function to read register */
mlan_status woal_read_reg(moal_handle * handle, t_u32 reg, t_u32 * data);
/** Function to write data to IO memory */
mlan_status woal_write_data_sync(moal_handle * handle, mlan_buffer * pmbuf,
                                 t_u32 port, t_u32 timeout);
/** Function to read data from IO memory */
mlan_status woal_read_data_sync(moal_handle * handle, mlan_buffer * pmbuf,
                                t_u32 port, t_u32 timeout);

/** Register to bus driver function */
mlan_status woal_bus_register(void);
/** Unregister from bus driver function */
void woal_bus_unregister(void);

/** Register device function */
mlan_status woal_register_dev(moal_handle * handle);
/** Unregister device function */
void woal_unregister_dev(moal_handle * handle);

int woal_sdio_set_bus_clock(moal_handle * handle, t_u8 option);

#ifdef SDIO_SUSPEND_RESUME
#ifdef MMC_PM_FUNC_SUSPENDED
/** Notify SDIO bus driver that WLAN is suspended */
void woal_wlan_is_suspended(moal_handle * handle);
#endif
/** SDIO Suspend */
int woal_sdio_suspend(struct device *dev);
/** SDIO Resume */
int woal_sdio_resume(struct device *dev);
#endif /* SDIO_SUSPEND_RESUME */

/** Structure: SDIO MMC card */
struct sdio_mmc_card
{
        /** sdio_func structure pointer */
    struct sdio_func *func;
        /** moal_handle structure pointer */
    moal_handle *handle;
        /** saved host clock value */
    unsigned int host_clock;
};

/*
 * SDIO function devices
 */
struct sdio_func {

	unsigned int		num;		/* function number */

	unsigned char		class;		/* standard interface class */
	unsigned short		vendor;		/* vendor id */
	unsigned short		device;		/* device id */

	unsigned		max_blksize;	/* maximum block size */
	unsigned		cur_blksize;	/* current block size */

	unsigned		enable_timeout;	/* max enable timeout in msec */

	unsigned int		state;		/* function state */
#define SDIO_STATE_PRESENT	(1<<0)		/* present in sysfs */

	unsigned char			tmpbuf[4];	/* DMA:able scratch buffer */

	unsigned		num_info;	/* number of info strings */
	const char		**info;		/* info strings */

	void *dev_data;
	bool	sdio_int_pending;

};


/** cmd52 read write */
int woal_sdio_read_write_cmd52(moal_handle * handle, int func, int reg,
                               int val);
unsigned char sdio_f0_readb(unsigned int addr, int *err_ret);
void sdio_f0_writeb(unsigned char b, unsigned int addr,
	int *err_ret);
int sdio_readsb(void *dst, unsigned int addr,
	int count);


int sdio_writesb(unsigned int addr, void *src,
	int count);
int sdio_set_block_size(int fn,unsigned blksz);
u8 sdio_readb(int fn,unsigned int addr, int *err_ret);
void sdio_writeb(int fn,u8 b, unsigned int addr, int *err_ret);
int woal_sdio_probe(struct sdio_func *func);
int sdio_reset(void);

#endif /* _MOAL_SDIO_H */
