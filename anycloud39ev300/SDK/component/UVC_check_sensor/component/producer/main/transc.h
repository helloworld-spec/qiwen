#ifndef _TRANSC_H_
#define _TRANSC_H_

//#include "burn_result.h"
#include "fwl_usb_transc.h"

#define    TRANS_NULL  						0
#define    TRANS_SWITCH_USB					1		//ÇÐ»»USBËÙ¶ÈÎªHigh Speed
#define    TRANS_TEST_CONNECT				2		//²âÊÔUSbÍ¨Ñ¶
#define    TRANS_SET_MODE					3 		//ÉèÖÃÉÕÂ¼Ä£Ê½£¬ÍêÕûÉÕÂ¼»òÊÇÉý¼¶»òÊÇSPIÉÕÂ¼
#define    TRANS_GET_FLASHID				4 		//»ñÈ¡nandflash»òspi flash id
#define    TRANS_SET_NANDPARAM				5 		//ÉèÖÃnandflashµÄ²ÎÊý
#define    TRANS_DETECT_NANDPARAM			6 		//²ânandflash²ÎÊý£¬¶ÔÓÚ²»ÔÚnandÁÐ±íÀïµÄflashÊ¹ÓÃ
#define    TRANS_INIT_SECAREA				7		//³õÊ¼»¯°²È«Çø
#define    TRANS_SET_RESV					8		//ÉèÖÃ±£ÁôÇø´óÐ¡
#define    TRANS_CREATE_PARTITION			9		//´´½¨·ÖÇø
#define    TRANS_FORMAT_DRIVER				10		//¸ñÊ½»¯·ÖÇø
#define    TRANS_MOUNT_DRIVER				11		//¹ÒÔØ·ÖÇø
#define    TRANS_DOWNLOAD_BOOT_START		12		//¿ªÊ¼ÏÂÔØboot
#define    TRANS_DOWNLOAD_BOOT_DATA			13		//·¢ËÍbootÊý¾Ý
#define    TRANS_COMPARE_BOOT_START		    14		//¿ªÊ¼±È½Ïboot
#define    TRANS_COMPARE_BOOT_DATA			15		//·¢ËÍ±È½ÏbootÊý¾Ý
#define    TRANS_DOWNLOAD_BIN_START			16		//¿ªÊ¼ÏÂÔØbinÎÄ¼þ
#define    TRANS_DOWNLOAD_BIN_DATA			17		//·¢ËÍbinÎÄ¼þÊý¾Ý
#define    TRANS_COMPARE_BIN_START			18		//¿ªÊ¼±È½ÏbinÎÄ¼þ
#define    TRANS_COMPARE_BIN_DATA			19		//·¢ËÍ±È½ÏbinÎÄ¼þÊý¾Ý
#define    TRANS_DOWNLOAD_IMG_START			20		//¿ªÊ¼ÏÂÔØIMAGEÎÄ¼þ
#define    TRANS_DOWNLOAD_IMG_DATA			21		//·¢ËÍIMAGEÊý¾Ý
#define    TRANS_COMPARE_IMG_START			22		//¿ªÊ¼±È½ÏIMAGEÎÄ¼þ
#define    TRANS_COMPARE_IMG_DATA			23		//·¢ËÍ±È½ÏIMAGEÊý¾Ý
#define    TRANS_DOWNLOAD_FILE_START		24		//¿ªÊ¼ÏÂÔØÎÄ¼þÏµÍ³ÎÄ¼þ
#define    TRANS_DOWNLOAD_FILE_DATA			25		//·¢ËÍÎÄ¼þÏµÍ³ÎÄ¼þÊý¾Ý
#define    TRANS_COMPARE_FILE_START		    26		//¿ªÊ¼±È½ÏÎÄ¼þÏµÍ³ÎÄ¼þ
#define    TRANS_COMPARE_FILE_DATA			27		//·¢ËÍ±È½ÏÎÄ¼þÏµÍ³ÎÄ¼þÊý¾Ý
#define    TRANS_UPLOAD_BIN_START			28	    //¿ªÊ¼ÉÏ´«BINÎÄ¼þ
#define    TRANS_UPLOAD_BIN_DATA			29	    //ÉÏ´«BINÎÄ¼þ
#define    TRANS_UPLOAD_FILE_START			30      //¿ªÊ¼ÉÏ´«ÎÄ¼þÏµÍ³ÎÄ¼þ
#define    TRANS_UPLOAD_FILE_DATA			31		//ÉÏ´«ÎÄ¼þÏµÍ³ÎÄ¼þÊý¾Ý
#define    TRANS_SET_GPIO					32 		//ÉèÖÃGPIO
#define    TRANS_RESET						33		//ÖØÆôÉè±¸¶Ë
#define    TRANS_CLOSE						34		//Close
#define    TRANS_SET_REG					35
#define    TRANS_DOWNLOAD_PRODUCER_START	36
#define    TRANS_DOWNLOAD_PRODUCER_DATA		37
#define    TRANS_RUN_PRODUCER				38

#define    TRANS_UPDATESELF_BIN_START		40		//¿ªÊ¼ÏÂÔØ×ÔÉý¼¶Êý¾Ý
#define    TRANS_UPDATESELF_BIN_DATA		41		//´«×ÔÉý¼¶Êý¾Ý
#define    TRANS_UPLOAD_BIN_LEN				43	    //ÉÏ´«BIN³¤¶È
#define	   TRANS_WRITE_ASA_FILE				44	    //Ð´°²È«ÇøÎÄ¼þ

#define    TRANS_DOWNLOAD_CLIENT_BOOT_START			45		//¿ªÊ¼ÏÂÔØ¿Í»§BOOTÎÄ¼þ
#define    TRANS_DOWNLOAD_CLIENT_BOOT_DATA			46		//·¢ËÍ¿Í»§BOOTÎÄ¼þÊý¾Ý
#define    TRANS_COMPARE_CLIENT_BOOT_START			47		//¿ªÊ¼±È½Ï¿Í»§BOOTÎÄ¼þ
#define    TRANS_COMPARE_CLIENT_BOOT_DATA			48		//·¢ËÍ±È½Ï
#define    TRANS_SET_SPIPARAM          			    49		//ÉèÖÃPSI²ÎÊý
#define    TRANS_GET_FLASH_HIGH_ID                   50      //»ñÈ¡nandflashµÄhigh id

#define    TRANS_UPLOAD_SPIDATA_START			    53	    //¿ªÊ¼ÉÏ´«SPIDATA
#define    TRANS_UPLOAD_SPIDATA_DATA			    54	    //¿ªÊ¼ÉÏ´«SPIDATA

#define    TRANS_WRITE_OTP_SERIAL			        55	    //Ð´otpÐòÁÐºÅ
#define    TRANS_READ_OTP_SERIAL			        56	    //¶ÁotpÐòÁÐºÅ

#define    TRANS_SET_EX_PARAMETER			        57	    //´«Ò»¸ö²ÎÊýÏÂÀ´
#define    TRANS_DOWNLOAD_SPIDATA_START			    58	    //´«Ò»¸ö²ÎÊýÏÂÀ´
#define    TRANS_DOWNLOAD_SPIDATA_DATA			    59	    //´«Ò»¸ö²ÎÊýÏÂÀ´
#define    TRANS_ERASE_PARTITION_SIZE			    60	    //·ÖÅú½øÐÐ²Á¿é,ÊÇÎªÁË¼´²å¼´ÉÕ¹¦ÄÜ




#define	   TRANS_GET_CHANNEL_ID						100
#define	   TRANS_GET_RAM_VALUE						101	    //
#define	   TRANS_GET_SCSI_STATUS				    102	    //
#define	   TRANS_SET_CHANNEL_ID						103
#define    TRANS_SET_BURNEDPARAM                    104     // ÏÂ·¢ÉÕÂ¼Íê³É²ÎÊý
#define    TRANS_SET_NANDFLASH_CHIP_SEL             105     // NAND Æ¬Ñ¡ÉèÖÃ
#define	   TRANS_READ_ASA_FILE				        106	    //Ð´°²È«ÇøÎÄ¼þ

#define	   TRANS_UPLOAD_BOOT_START				    107	    //»ñÈ¡bootÎÄ¼þ¿ªÊ¼
#define	   TRANS_UPLOAD_BOOT_DATA				    108	    //»ñÈ¡bootÎÄ¼þµÄÊý¾Ý
#define	   TRANS_UPLOAD_BOOT_LEN				    109	    //»ñÈ¡bootÎÄ¼þµÄ³¤¶

#define	   TRANS_SET_ERASE_NAND_MODE			    111	    //ÉèÖÃnandµÄ²ÁblockµÄÄ£Ê½
#define	   TRANS_SET_BIN_RESV_SIZE			        112	    //ÉèÖÃBINÇøµÄÊ£Óà¿Õ¼ä´óÐ¡.


typedef struct
{
    unsigned long   chip_id;
    unsigned long   total_size;             ///< flash total size in bytes
    unsigned long	page_size;       ///< total bytes per page
    unsigned long	program_size;    ///< program size at 02h command
    unsigned long	erase_size;      ///< erase size at d8h command 
    unsigned long	clock;           ///< spi clock, 0 means use default clock 
    
    //chip character bits:
    //bit 0: under_protect flag, the serial flash under protection or not when power on
    //bit 1: fast read flag    
    unsigned char    flag;            ///< chip character bits
    unsigned char	protect_mask;    ///< protect mask bits in status register:BIT2:BP0, 
                             //BIT3:BP1, BIT4:BP2, BIT5:BP3, BIT7:BPL
    unsigned char    reserved1;
    unsigned char    reserved2;
    unsigned char   des_str[32];		   //ÃèÊö·û                                    
}T_SFLASH_PHY_INFO;



static bool Transc_SwitchUsbHighSpeed(unsigned char data[], unsigned long len, T_CMD_RESULT *result);
static bool Transc_TestConnection(unsigned char data[], unsigned long len, T_CMD_RESULT *result);
static bool Transc_SetMode(unsigned char data[], unsigned long len, T_CMD_RESULT *result);
static bool Transc_GetFlashID(unsigned char data[], unsigned long len, T_CMD_RESULT *result);
static bool Transc_SetNandParam(unsigned char data[], unsigned long len, T_CMD_RESULT *result);
static bool Transc_DetectNandParam(unsigned char buf[], unsigned long len, T_CMD_RESULT *result);
static bool Transc_InitSecArea(unsigned char data[], unsigned long len, T_CMD_RESULT *result);
static bool Transc_SetResvAreaSize(unsigned char data[], unsigned long len, T_CMD_RESULT *result);
static bool Transc_StartDLBin(unsigned char data[], unsigned long len, T_CMD_RESULT *result);
static bool Transc_DLBin(unsigned char data[], unsigned long len, T_CMD_RESULT *result);
static bool Transc_GetDiskInfo(unsigned char data[], unsigned long len, T_CMD_RESULT *result);
static bool Transc_CreatePartion(unsigned char data[], unsigned long len, T_CMD_RESULT *result);
static bool Transc_FormatDriver(unsigned char data[], unsigned long len, T_CMD_RESULT *result);
static bool Transc_MountDriver(unsigned char data[], unsigned long len, T_CMD_RESULT *result);
static bool Transc_StartDLImg(unsigned char data[], unsigned long len, T_CMD_RESULT *result);
static bool Transc_DLImg(unsigned char data[], unsigned long len, T_CMD_RESULT *result);
static bool Transc_StartDLBoot(unsigned char data[], unsigned long len, T_CMD_RESULT *result);
static bool Transc_DLBoot(unsigned char data[], unsigned long len, T_CMD_RESULT *result);
static bool Transc_StartDLFile(unsigned char data[], unsigned long len, T_CMD_RESULT *result);
static bool Transc_DLFile(unsigned char data[], unsigned long len, T_CMD_RESULT *result);
static bool Transc_Reset(unsigned char data[], unsigned long len, T_CMD_RESULT *result);
static bool Transc_Close(unsigned char data[], unsigned long len, T_CMD_RESULT *result);
static bool Transc_GetBinStart(unsigned char data[], unsigned long len, T_CMD_RESULT *result);
static bool Transc_GetBinLength(unsigned char data[], unsigned long len, T_CMD_RESULT *result);
static bool Transc_GetBinData(unsigned char data[], unsigned long len, T_CMD_RESULT *result);
static bool Transc_WriteAsaFile(unsigned char data[], unsigned long len, T_CMD_RESULT *result);
static bool Transc_SetSPIParam(unsigned char data[], unsigned long len, T_CMD_RESULT *result);

static bool Transc_ReadAsaFile(unsigned char data[], unsigned long len, T_CMD_RESULT *result);
static bool Transc_GetChannel_ID(unsigned char data[], unsigned long len, T_CMD_RESULT *result);
static bool Transc_SetChannel_ID(unsigned char data[], unsigned long len, T_CMD_RESULT *result);
static bool Transc_GetBootStart(unsigned char data[], unsigned long len, T_CMD_RESULT *result);
static bool Transc_GetBootData(unsigned char data[], unsigned long len, T_CMD_RESULT *result);
static bool Transc_GetBootLen(unsigned char data[], unsigned long len, T_CMD_RESULT *result);
static bool Transc_SetErase_Mode(unsigned char data[], unsigned long len, T_CMD_RESULT *result);
static bool Transc_Set_Bin_Resv_Size(unsigned char data[], unsigned long len, T_CMD_RESULT *result);
static bool Transc_ReadAsaFile(unsigned char data[], unsigned long len, T_CMD_RESULT *result);
static bool Transc_GetSpiDataStart(unsigned char data[], unsigned long len, T_CMD_RESULT *result);
static bool Transc_GetSpiData(unsigned char data[], unsigned long len, T_CMD_RESULT *result);
static bool Transc_GetFlashHighID(unsigned char data[], unsigned long len, T_CMD_RESULT *result);
static bool Transc_write_otp_serial(unsigned char data[], unsigned long len, T_CMD_RESULT *result);
static bool Transc_read_otp_serial(unsigned char data[], unsigned long len, T_CMD_RESULT *result);
static bool Transc_Set_EX_parameter(unsigned char data[], unsigned long len, T_CMD_RESULT *result);
static bool Transc_BurnedParam(unsigned char data[], unsigned long len, T_CMD_RESULT *result);
static bool Transc_Burned_Spidata_Data(unsigned char data[], unsigned long len, T_CMD_RESULT *result);
static bool Transc_Burned_Spidata_Start(unsigned char data[], unsigned long len, T_CMD_RESULT *result);
static bool Transc_Erase64K_Partition_Size(unsigned char data[], unsigned long len, T_CMD_RESULT *result);


#endif
