/*
 * (C) Copyright 2008
 * Texas Instruments, <www.ti.com>
 * Syed Mohammed Khasim <khasim@ti.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation's version 2 of
 * the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef AK_SDHSMMC_H_
#define AK_SDHSMMC_H_

/*
mmc/sd base addr:0x2010,0000 ; sdio:0x20108,000
*/
struct akhsmmc {
	unsigned int sdiointrcfgreg;/* 0x00 */
#if 0
	unsigned int clkctrlreg;	/* 0x04 */
	unsigned int cmdargreg;		/* 0x08 */
	unsigned int cmdreg;        /* 0x0c */
	unsigned int cmdresp;		/* 0x10 */
	unsigned int dataresp[4];   /* 0x14~0x20 */
	unsigned int datatimereg;	/* 0x24 */
	unsigned int datalenreg;	/* 0x28 */
	unsigned int datactrlreg;	/* 0x2C */
	unsigned int datacntreg;	/* 0x30 */
	unsigned int statusreg;		/* 0x34 */
	unsigned int interenreg;	/* 0x38 */
	unsigned int dmamodereg;	/* 0x3C */
	unsigned int cpumodereg;	/* 0x40 */
#endif
};

/**
 * @brief share pins
 * 
 */
#if 0
enum
{
    ePIN_AS_MMCSD = 0,             ///< share pin as MDAT1, 8 lines
    ePIN_AS_SDIO,               ///< share pin as SDIO
    ePIN_AS_DUMMY
};
#endif

/**
 * @brief share pins
 * 
 */
typedef enum
{
    ePIN_AS_MMCSD = 0,             ///< share pin as MDAT1, 8 lines
    ePIN_AS_I2S,                ///< share pin as I2S bit[24]:0
    ePIN_AS_PWM0,               ///< share pin as PWM0   
    ePIN_AS_PWM1,               ///< share pin as PWM1
    ePIN_AS_PWM2,               ///< share pin as PWM2
    ePIN_AS_PWM3,               ///< share pin as PWM3
    ePIN_AS_PWM4,               ///< share pin as PWM4
	
    ePIN_AS_SDIO,               ///< share pin as SDIO
    ePIN_AS_UART1,              ///< share pin as UART1
    ePIN_AS_UART2,              ///< share pin as UART2
    ePIN_AS_CAMERA,             ///< share pin as CAMERA
    ePIN_AS_SPI0,               ///< share pin as SPI1 bit[25]:0
    ePIN_AS_SPI1,               ///< share pin as SPI2  bit[26]:1
    ePIN_AS_JTAG,               ///< share pin as JTAG
    ePIN_AS_TWI,                ///< share pin as I2C
    ePIN_AS_MAC,                ///< share pin as Ethernet MAC
    ePIN_AS_OPCLK,

    ePIN_AS_DUMMY

}E_GPIO_PIN_SHARE_CONFIG;

typedef enum
{
    INTERFACE_NOT_SD,
    INTERFACE_SDMMC4,
    INTERFACE_SDMMC8,
    INTERFACE_SDIO
}
T_eCARD_INTERFACE;

/**
*@brief card type define
*/

typedef enum _CARD_TYPE
{
    CARD_UNUSABLE=0,                            ///< unusable card
    CARD_MMC,                                   ///< mmc card
    CARD_SD,                                    ///< sd card, mem only
    CARD_SDIO,                                  ///< sdio card, io only
    CARD_COMBO                                  ///< combo card,mem and sdio
}T_eCARD_TYPE;

typedef enum _BUS_MODE
{
    USE_ONE_BUS,
    USE_FOUR_BUS,
    USE_EIGHT_BUS
}T_eBUS_MODE;


typedef enum
{    
    PARTITION_USER,
    PARTITION_BOOT1,
    PARTITION_BOOT2,
    PARTITION_RPMB,
    PARTITION_GP1,
    PARTITION_GP2,
    PARTITION_GP3,
    PARTITION_GP4,
    PARTITION_NUM
}
T_eCARD_PARTITION;

//sd card status
#define SD_CURRENT_STATE_OFFSET             9
#define SD_CURRENT_STATE_MASK               (0xF<<9)
#define SD_CURRENT_STATE_IDLE               0
#define SD_CURRENT_STATE_READY              1 
#define SD_CURRENT_STATE_IDENT              2
#define SD_CURRENT_STATE_STBY               3
#define SD_CURRENT_STATE_TRAN               4
#define SD_CURRENT_STATE_DATA               5
#define SD_CURRENT_STATE_RCV                6
#define SD_CURRENT_STATE_PRG                7
#define SD_CURRENT_STATE_DIS                8
#define SD_CURRENT_STATE_IO_MODE            15
//sd dma operation block length             
#define SD_DMA_BLOCK_64K                    (64*2)
#define SD_DMA_BLOCK_32K                    (32*2)
#define SD_DMA_BLOCK_8K                     (8*2)
#define SD_DMA_BLOCK_2K                     (2*2)
#define SD_DMA_BLOCK_4K                     (4*2)
                                            
#define SD_HIGH_SPEED_MODE                  1
#define SD_DEFAULT_SPEED_MODE               0
#define SD_MMC_INVALID_SPEC_VERSION         0xff
#define SD_FUNC_SUPPORTED_GROUP1(status)    ((status[51]<<8) | status[50])  
#define SD_FUNC_SUPPORTED_GROUP2(status)    ((status[53]<<8) | status[52])  
#define SD_FUNC_SUPPORTED_GROUP3(status)    ((status[55]<<8) | status[54])  
#define SD_FUNC_SUPPORTED_GROUP4(status)    ((status[57]<<8) | status[56])  
#define SD_FUNC_SUPPORTED_GROUP5(status)    ((status[59]<<8) | status[58]) 
#define SD_FUNC_SUPPORTED_GROUP6(status)    ((status[61]<<8) | status[60])  
#define SD_FUNC_SWITCHED_GROUP1(status)     (status[47]&0x0f)
#define SD_FUNC_SWITCHED_GROUP2(status)     ((status[47]>>4)&0x0f)
#define SD_FUNC_SWITCHED_GROUP3(status)     (status[48]&0x0f)
#define SD_FUNC_SWITCHED_GROUP4(status)     ((status[48]>>4)&0x0f)
#define SD_FUNC_SWITCHED_GROUP5(status)     (status[49]&0x0f)
#define SD_FUNC_SWITCHED_GROUP6(status)     ((status[49]>>4)&0x0f)

#define MMC4_CARD_TYPE(extcsd)              (extcsd[49]&0xff)
#define MMC4_SECTOR_CNT(extcsd)             (extcsd[53])
#define MMC4_POWER_CLASS(extcsd)            (extcsd[50])
#define MMC4_EXT_CSD_REV(extcsd)            (extcsd[48]&0xff)
#define MMC4_PARTITION_CFG(extcsd)          ((extcsd[44]>>24)&0xff)
#define MMC4_PARTITION_SZ(extcsd)           ((extcsd[56]>>16)&0xff)


typedef enum
{
    SD_DATA_MODE_SINGLE,                    ///< read or write single block
    SD_DATA_MODE_MULTI                      ///< read or wirte multiply block
}
T_eCARD_DATA_MODE;

typedef enum _SD_STATUS
{
    SD_GET_OCR_VALID,                       ///<get ocr valid
    SD_GET_OCR_FAIL,                        ///<get ocr fial
    SD_GET_OCR_INVALID,                     ///<get ocr invalid
    SD_NEGO_SUCCESS,                        ///< sd nego voltage success
    SD_NEGO_FAIL,                           ///< sd nego voltage fail
    SD_NEGO_TIMEOUT                         ///< sd nego voltage timeout
}T_eSD_STATUS;


#define SD_IDENTIFICATION_MODE_CLK  (400*1000)              ///<400k
#define SD_TRANSFER_MODE_CLK        (20*1000*1000)          ///<20M
#define HS_TRANSFER_MODE_CLK1       (30*1000*1000)          ///<30M
#define HS_TRANSFER_MODE_CLK2       (26*1000*1000)          ///<26M
#define MMC_DEFAULT_MODE_20M        (20*1000*1000)
#define SD_DEFAULT_MODE_25M         (25*1000*1000)
#define MMC_HS_MODE_26M             (26*1000*1000)
#define MMC_HS_MODE_52M             (52*1000*1000)
#define SD_HS_MODE_50M              (50*1000*1000)

#define SD_HCS                      (1<<30)
#define SD_STATUS_POWERUP           (1UL<<31)
#define SD_CCS                      (1<<30)
#define SD_OCR_MASK                 (0xffffffff)
#define SD_DEFAULT_VOLTAGE          (0x00FF8000)
#define ERROR_INVALID_RCA            T_U32_MAX


//clock ctrl reg
#define CLK_DIV_L_OFFSET           0
#define CLK_DIV_H_OFFSET           8
#define SD_CLK_ENABLE              (1<<16)
#define PWR_SAVE_ENABLE            (1<<17)
#define FALLING_TRIGGER            (1<<19)
#define SD_INTERFACE_ENABLE        (1<<20)

//cmd reg
#define CMD_INDEX_OFFSET           1
#define CPSM_ENABLE                (1<<0)
#define WAIT_REP_OFFSET            7
#define WAIT_CMD_PEND              (1<<9)
#define RSP_CRC_NO_CHK             (1<<10)
        
//status reg
#define CMD_CRC_FAIL               (1 << 0)
#define DATA_CRC_FAIL              (1 << 1)
#define CMD_TIME_OUT               (1 << 2)
#define DATA_TIME_OUT              (1 << 3)
#define CMD_RESP_END               (1 << 4)
#define CMD_SENT                   (1 << 5)
#define DATA_END                   (1 << 6)
#define DATA_BLOCK_END             (1 << 7)
#define DATA_START_BIT_ERR         (1 << 8)
#define CMD_ACTIVE                 (1 << 9)
#define TX_ACTIVE                  (1 << 10)
#define RX_ACTIVE                  (1 << 11)
#define DATA_BUF_FULL              (1 << 12)
#define DATA_BUF_EMPTY             (1 << 13)
#define DATA_BUF_HALF_FULL         (1 << 14)
#define DATA_BUF_HALF_EMPTY        (1 << 15)

//dma reg
#define BUF_SIZE_OFFSET         (17)
#define DMA_EN                  (1<<16)
#define START_ADDR_OFFSET       (1)
#define START_ADDR_MASK         (0x7fff<<1)
#define BUF_EN                  (1)

//data ctrl reg
#define SD_DATA_CTL_ENABLE              1
#define SD_DATA_CTL_DISABLE             0
#define SD_DATA_CTL_TO_HOST             1
#define SD_DATA_CTL_TO_CARD             0
#define SD_DATA_CTL_BUS_MODE_OFFSET     3
#define SD_DATA_CTL_DIRECTION_OFFSET    1
#define SD_DATA_CTL_BLOCK_LEN_OFFSET    16
#define SD_DAT_MAX_TIMER_V              0x800000

#define SD_CMD(n)                       n       
#define SD_NO_RESPONSE                  0                           
#define SD_SHORT_RESPONSE               1                               
#define SD_LONG_RESPONSE                3
#define SD_NO_ARGUMENT                  0
#define SD_POWER_SAVE_ENABLE            1
#define SD_POWER_SAVE_DISABLE           0 
#define SD_DEFAULT_BLOCK_LEN            512
#define SD_DEFAULT_BLOCK_LEN_BIT        9
#define SDIO_MAX_BLOCK_LEN              2048
#define SD_BUS_WIDTH_1BIT               0
#define SD_BUS_WIDTH_4BIT               2
#define SD_DMA_SIZE_32K                 (32*1024)
#define SD_DMA_SIZE_64K                 (64*1024)
#define INNER_BUF_MODE                  0
#define L2_CPU_MODE                     1
#define L2_DMA_MODE                     2


/*
 * OMAP HS MMC Bit definitions
 */
#define MMC_SOFTRESET			(0x1 << 1)
#define SDIO_SOFTRESET			(0x1 << 2)

#define RESETDONE			(0x1 << 0)
#define NOOPENDRAIN			(0x0 << 0)
#define OPENDRAIN			(0x1 << 0)
#define OD				(0x1 << 0)
#define INIT_NOINIT			(0x0 << 1)
#define INIT_INITSTREAM			(0x1 << 1)
#define HR_NOHOSTRESP			(0x0 << 2)
#define STR_BLOCK			(0x0 << 3)
#define MODE_FUNC			(0x0 << 4)
#define DW8_1_4BITMODE			(0x0 << 5)
#define MIT_CTO				(0x0 << 6)
#define CDP_ACTIVEHIGH			(0x0 << 7)
#define WPP_ACTIVEHIGH			(0x0 << 8)
#define RESERVED_MASK			(0x3 << 9)
#define CTPL_MMC_SD			(0x0 << 11)
#define BLEN_512BYTESLEN		(0x200 << 0)
#define NBLK_STPCNT			(0x0 << 16)
#define DE_DISABLE			(0x0 << 0)
#define BCE_DISABLE			(0x0 << 1)
#define BCE_ENABLE			(0x1 << 1)
#define ACEN_DISABLE			(0x0 << 2)
#define DDIR_OFFSET			(4)
#define DDIR_MASK			(0x1 << 4)
#define DDIR_WRITE			(0x0 << 4)
#define DDIR_READ			(0x1 << 4)
#define MSBS_SGLEBLK			(0x0 << 5)
#define MSBS_MULTIBLK			(0x1 << 5)
#define RSP_TYPE_OFFSET			(16)
#define RSP_TYPE_MASK			(0x3 << 16)
#define RSP_TYPE_NORSP			(0x0 << 16)
#define RSP_TYPE_LGHT136		(0x1 << 16)
#define RSP_TYPE_LGHT48			(0x2 << 16)
#define RSP_TYPE_LGHT48B		(0x3 << 16)
#define CCCE_NOCHECK			(0x0 << 19)
#define CCCE_CHECK			(0x1 << 19)
#define CICE_NOCHECK			(0x0 << 20)
#define CICE_CHECK			(0x1 << 20)
#define DP_OFFSET			(21)
#define DP_MASK				(0x1 << 21)
#define DP_NO_DATA			(0x0 << 21)
#define DP_DATA				(0x1 << 21)
#define CMD_TYPE_NORMAL			(0x0 << 22)
#define INDEX_OFFSET			(24)
#define INDEX_MASK			(0x3f << 24)
#define INDEX(i)			(i << 24)
#define DATI_MASK			(0x1 << 1)
#define CMDI_MASK			(0x1 << 0)
#define DTW_1_BITMODE			(0x0 << 1)
#define DTW_4_BITMODE			(0x1 << 1)
#define DTW_8_BITMODE                   (0x1 << 5) /* CON[DW8]*/
#define SDBP_PWROFF			(0x0 << 8)
#define SDBP_PWRON			(0x1 << 8)
#define SDVS_1V8			(0x5 << 9)
#define SDVS_3V0			(0x6 << 9)
#define ICE_MASK			(0x1 << 0)
#define ICE_STOP			(0x0 << 0)
#define ICS_MASK			(0x1 << 1)
#define ICS_NOTREADY			(0x0 << 1)
#define ICE_OSCILLATE			(0x1 << 0)
#define CEN_MASK			(0x1 << 2)
#define CEN_DISABLE			(0x0 << 2)
#define CEN_ENABLE			(0x1 << 2)
#define CLKD_OFFSET			(6)
#define CLKD_MASK			(0x3FF << 6)
#define DTO_MASK			(0xF << 16)
#define DTO_15THDTO			(0xE << 16)
#define SOFTRESETALL			(0x1 << 24)
#define CC_MASK				(0x1 << 0)
#define TC_MASK				(0x1 << 1)
#define BWR_MASK			(0x1 << 4)
#define BRR_MASK			(0x1 << 5)
#define ERRI_MASK			(0x1 << 15)
#define IE_CC				(0x01 << 0)
#define IE_TC				(0x01 << 1)
#define IE_BWR				(0x01 << 4)
#define IE_BRR				(0x01 << 5)
#define IE_CTO				(0x01 << 16)
#define IE_CCRC				(0x01 << 17)
#define IE_CEB				(0x01 << 18)
#define IE_CIE				(0x01 << 19)
#define IE_DTO				(0x01 << 20)
#define IE_DCRC				(0x01 << 21)
#define IE_DEB				(0x01 << 22)
#define IE_CERR				(0x01 << 28)
#define IE_BADA				(0x01 << 29)

#define VS30_3V0SUP			(1 << 25)
#define VS18_1V8SUP			(1 << 26)

/* Driver definitions */
#define MMCSD_SECTOR_SIZE		512
#define MMC_CARD			0
#define SD_CARD				1
#define BYTE_MODE			0
#define SECTOR_MODE			1
#define CLK_INITSEQ			0
#define CLK_400KHZ			1
#define CLK_MISC			2

#define RSP_TYPE_NONE	(RSP_TYPE_NORSP   | CCCE_NOCHECK | CICE_NOCHECK)
#define MMC_CMD0	(INDEX(0)  | RSP_TYPE_NONE | DP_NO_DATA | DDIR_WRITE)

/* Clock Configurations and Macros */
#define MMC_CLOCK_REFERENCE	96 /* MHz */

#define mmc_reg_out(addr, mask, val)\
	writel((readl(addr) & (~(mask))) | ((val) & (mask)), (addr))

int ak_sdhsmmc_init(int dev_index, uint host_caps_mask, uint f_max, int cd_gpio,
		int wp_gpio);


#endif /* AK_SDHSMMC_H_ */
