/*
 * serial flash driver
 */

#ifndef _SERIAL_DRV_H_
#define _SERIAL_DRV_H_

#include "anyka_types.h"


#define SPI_MASTER_INPROGRESS    	(1<<9)
#define SPI_MASTER_FINISH	        (1<<8)


#define SPI_MODE0_V      			(0x0<<2)
#define SPI_MODE3_V      			(0x3<<2)
#define SPI_MASTER_V     			(0x1<<4)
#define SPI_SLAVE_V      			(0x0<<4)
#define SPI_CTRL_FORCE_CS         	(1<<5)
#define SPI_CTRL_GARBAGE_MODE    	(1<<0)
#define SPI_CTRL_RX_REJECT        	(1<<1)
#define	SPI_CTRL_ENABLE				(1<<6)
#define SPI_SPICON_WIRE				(0x3<<16)

#if 1
#define SPI_FLASH_COM_READ          0x03
#define SPI_FLASH_FAST_READ      	0x0b    /* Read Data Bytes at Higher Speed */ 
//2-wire mode
#define SPI_FLASH_COM_D_READ      	0x3B
//4-wire mode
#define SPI_FLASH_COM_Q_READ      	0x6B
#endif

#define SPI_NAND_READ_ID            0x9F

#define SPI_NAND_WRITE_ENABLE       0x06
#define SPI_NAND_WRITE_DISABLE      0x04

#define SPI_NAND_SET_FEATURE        0x1F
#define SPI_NAND_GET_FEATURE        0x0F

#define SPI_NAND_PAGE_READ          0x13
#define SPI_NAND_READ_CACHE         0x03
#define SPI_NAND_READ_CACHE_X2      0x3B
#define SPI_NAND_READ_CACHE_X4      0x6B

#define SPI_NAND_PROGRAM_LOAD       0x02
#define SPI_NAND_PROGRAM_LOAD_X4    0x32
#define SPI_NAND_PROGRAM_EXECUTE    0x10

#define SPI_NAND_BLOCK_ERASE        0xD8

#define SPI_NAND_RESET              0xFF

#define ADDR_PROTECTION 0xA0
#define ADDR_FEATURE    0xB0
#define ADDR_STATUS     0xC0


#define STATUS_ECCS1    (1<<5)
#define STATUS_ECCS0    (1<<4)
#define STATUS_P_FAIL   (1<<3)
#define STATUS_E_FAIL   (1<<2)
#define STATUS_OIP      (1<<0)

#define CONFIG_SPIP_START_PAGE			0    // cdh:add for searching SPIP flag start flash offset address
#define CONFIG_SPIP_PAGE_CNT			3     // cdh:add for searching SPIP flag
#define CONFIG_PARATION_PAGE_CNT		2     // cdh:add for searching  paration table page cnt

#define CONFIG_PARATION_TAB_SIZE 512    // cdh:add for paration table section size
#define PARTITION_NAME_LEN              6 

typedef struct {
    T_U32 dl_size;			// download size(unit: byte)
    /*
	 *  new divide value. only low 15 bits is valid.
	 *	<2: NOTE update spi clock(use default clock)
	 *	>=2: update clock with this divide;
	 *  SPI clock frequency = ASIC_clock/2/new_divide, (new_divide >= 2)
	 */	
    T_U32 new_divide;		
}serial_flash_infor_t;


//outside ram config information offset
#define SPI_OUTSIDE_RAM_USE	  (PASSWORD_ADDR_OFFSET + PASSWORD_LENGTH + sizeof(serial_flash_infor_t) )

#define SPI_OUTSIDE_RAM_CONFIG  (PASSWORD_ADDR_OFFSET + PASSWORD_LENGTH + sizeof(serial_flash_infor_t) + 4)

typedef T_VOID (*T_fSPI_CALLBACK)(T_VOID);

typedef enum
{
    SPI_ID0 = 0,
    SPI_ID1,
    SPI_NUM
}T_eSPI_ID;

typedef enum
{
	SPI_BUS1 = 0,
	SPI_BUS2,
	SPI_BUS4,
	SPI_BUS_NUM,
}T_eSPI_BUS;


typedef enum
{
    SPI_MODE0 = 0,///<CPHA = 0, CPOL = 0
    SPI_MODE1,    ///<CPHA = 0, CPOL = 1
    SPI_MODE2,    ///<CPHA = 1, CPOL = 0
    SPI_MODE3,     ///<CPHA = 1, CPOL = 1
    SPI_MODE_NUM
}T_eSPI_MODE;


typedef enum
{
    SPI_SLAVE = 0,
    SPI_MASTER,
    SPI_ROLE_NUM
}T_eSPI_ROLE;


typedef enum
{
    ADDR_USB_EP2 = 0,           ///< usb ep2      0
    ADDR_USB_EP3 = 1,           ///< usb ep3      1
    ADDR_RESERVED = 2,          ///< usb ep4      2
    ADDR_NFC = 3,               ///< nfc            3
    ADDR_MMC_SD = 4,            ///< sdmmc       4
    ADDR_SDIO = 5,              ///< sdio           5
    ADDR_SPI1_RX = 7,           ///< spi1 rx        7
    ADDR_SPI1_TX = 8,           ///< spi1 tx        8
    ADDR_DAC = 9,               ///< dac            9
    ADDR_ADC = 14              ///< adc            14
}DEVICE_SELECT;


typedef struct {
    T_BOOL  bOpen;
    T_U8    ucRole;
    T_U8    ucMode;       //phase mode
    T_U32   clock;        //clock div
    T_U8    ucRxBufferID;
    T_U8    ucTxBufferID;
    T_U32   ulBaseAddr;
    T_U8    ucL2Tx;
    T_U8    ucL2Rx;
    T_U8 	ucBusWidth;
	
    T_fSPI_CALLBACK pfCallBack;
}T_SPI;

typedef struct
{
	T_U32 file_length;
    T_U32 ld_addr;
#ifdef CONFIG_SPI_NAND_FLASH
    T_U32 map_index;
#endif      
    T_U32 start_page;
	T_U32 backup_page;        //backup data start page
    T_U8 file_name[16];
}T_FILE_CONFIG;



typedef struct
{
    unsigned char         type;        //data,/bin/fs  ,E_PARTITION_TYPE
    volatile unsigned char  r_w_flag:4;               //only read or write
    unsigned char         hidden_flag:4;            //hidden or no hidden
    unsigned char         name[PARTITION_NAME_LEN]; //分区名
    unsigned long         ksize;                    //分区大小，K为单位
    unsigned long         start_pos;                //分区的开始位置，字节为单位     
}T_PARTITION_CONFIG;

typedef struct
{
    T_U32  parameter1;    //bin:file_length  fs:未定      
    T_U32  parameter2;    //bin: ld_addr     fs:未定
    T_U32  parameter3;    //bin:backup_page  fs:未定     
    T_U32  parameter4;    //bin:check          fs:未定     
}T_EX_PARTITION_CONFIG;

typedef struct
{
    T_PARTITION_CONFIG        partition_info;
    T_EX_PARTITION_CONFIG     ex_partition_info;
}T_PARTITION_TABLE_INFO;

typedef struct
{
    T_U32  file_length;    //bin:file_length  fs:未定      
    T_U32  ld_addr;    //bin: ld_addr     fs:未定
    T_U32  backup_page;    //bin:backup_page  fs:未定     
    T_U8   check;    //bin:check          fs:未定  
    T_U8   rev1;
    T_U8   rev2;
    T_U8   rev3;
}T_BIN_CONFIG;

typedef struct
{
    T_U32 chip_id;
    T_U32 total_size;             ///< flash total size in bytes
    T_U32	page_size;       ///< total bytes per page
    T_U32	program_size;    ///< program size at 02h command
    T_U32	erase_size;      ///< erase size at d8h command 
    T_U32	clock;           ///< spi clock, 0 means use default clock 
    
    //chip character bits:
    //bit 0: under_protect flag, the serial flash under protection or not when power on
    //bit 1: fast read flag    
    T_U8  flag;            ///< chip character bits
    T_U8	protect_mask;
	T_U8  reserved1;
    T_U8  reserved2;
    T_U8  des_str[32];		   //描述符                                    
}T_SFLASH_PHY_INFO;



/**************************************************************************
  Function:       T_VOID SpiCtrlInit(T_U32 mode, T_U32 role, T_U32 clk_div);
  Description:    initial the spi ctrl
  Input:          role: master or slaver                 
  Output:         none
  Return:         none
  Author:         Zou Tianxiang
****************************************************************************/
T_VOID spi_ctrl_init(T_eSPI_ID ID,T_U32 mode, T_U32 role, T_U32 hz);

/**
 * @brief read data from one page of spiflash
 * 
 * @author liao_zhijun
 * @date 2010-04-29
 * @param page [in]  page number
 * @param buf [in]  buffer to store read data 
 * @param page_cnt [in]  the page count to read  
 * @return T_BOOL
 */
T_BOOL spi_flash_read(T_U32 page, T_U8 *buf, T_U32 page_cnt);


/**
 * @brief read data from one fine of spiflash
 * 
 * @author lu_heshan
 * @date 
 * @param
 * @param 
 */

T_S32   Fwl_SPI_FileRead(T_PARTITION_TABLE_INFO *cfg, T_U8* buffer, T_U32 count);



#endif	// _SERIAL_DRV_H_


