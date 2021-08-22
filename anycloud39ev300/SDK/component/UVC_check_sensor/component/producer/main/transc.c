/**
 * @FILENAME transc.c
 * @BRIEF handle all burn transc
 * Copyright (C) 2009 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @AUTHOR Jiang Dihui
 * @DATE 2009-09-10
 * @version 1.0
 */
#include "partition_lib.h"
#include "partition_init.h"
#include "transc.h"
#include "nand_list.h"
//#include "fha.h"
//#include "fha_asa.h"
//#include "fsa.h"
//#include "arch_nand.h"
#include <string.h>
#include "Fwl_osMalloc.h"
//#include "Fwl_osFS.h"
//#include "fs.h"
#include "fwl_nandflash.h"
#include "fwl_spiflash.h"
//#include "fwl_emmc.h"
#include "fwl_usb_transc.h"
#include "hal_print.h"


//#include "file.h"

#define  CHECK_FAIL(result,ret)  {ret = (0 != (result)) ? false : true;}
#define  SPIBOOTAREA_PAGE_NUM   257
#define  SD_SECTOR_SIZE         512

#define  ONE_K_SIZE             1024


#define  FS_S_IFDIR             0x04000000

#define  GET_ALL_SD_BEFORE_FS_DATA   "ALLDATA"

#define  FORMAT_SPIFLASH_FS   "_FORMAT_FS"


typedef enum
{
    MODE_NEWBURN = 1,           //new burn
    MODE_UPDATE,                //update mode
    MODE_DEBUG,
}E_BURN_MODE;

typedef enum
{
    MEDIUM_NAND,                //nand
    MEDIUM_SPIFLASH,            //spiflash
    MEDIUM_EMMC,                //sd, emmc, inand
    MEDIUM_SPI_NAND,                //nand
}E_FHA_MEDIUM_TYPE;

typedef struct
{
    unsigned long	num;
    unsigned char	dir;
    unsigned char	level;
	unsigned char	Pullup;
	unsigned char	Pulldown;
}T_GPIO_PARAM;

typedef struct  
{
    unsigned long  burned_mode;
    unsigned long  bGpio;
    T_GPIO_PARAM  pwr_gpio_param;
    unsigned long  bWakup;
    unsigned long  rtc_wakup_level;
}T_BURNED_PARAM;

#define BURNED_NONE      1
#define BURNED_RESET     2
#define BURNED_POWER_OFF 3



typedef struct tag_FHABinParam
{
    unsigned long   data_length;    //数据长度
    unsigned char    file_name[8];  //文件名称
    bool  bCheck;         //是否校验
    unsigned char    rev1; 
    unsigned char    rev2;
    unsigned char    rev3;
}T_BIN_PARAM, *T_PBIN_PARAM;

typedef struct
{
	unsigned long start_page;
	unsigned long spiflash_totalsize;
}T_SPI_FS_INFO;


typedef struct DownLoadControl
{
    unsigned long file_total_len;
    unsigned long data_total;
}T_DL_CONTROL;

typedef struct
{
	unsigned char status;
	unsigned char mode;
	unsigned char fileName[PARTITION_NAME_LEN];
}T_WRITE_ASA_FILE;

typedef enum
{
    DL_START,
    DL_WRITE,
}T_DL_STATUS;

typedef struct
{
	unsigned long chip_ID;
    unsigned long chip_cnt;
}T_CHIP_INFO;

typedef struct
{
    unsigned char DiskName;        
    unsigned char Rsv1[3];            
    unsigned long PageCnt;          
    unsigned long Rsv2;
    unsigned long Rsv3;
}T_DRIVER_INFO;
typedef struct
{
    unsigned char eMedium;
	unsigned char burn_mode;
    unsigned char erase_partition_size;
    unsigned char no_need_erase;
}T_MODE_CONTROL;


typedef struct
{
    unsigned long start_block;
	unsigned long end_block;
}T_ERASE_BLOCK;



typedef struct
{
	unsigned char *pLibName;
	unsigned char *(*pVerFun)(void);
}T_VERSION_INFO;

typedef struct
{
    unsigned long bin_len;
    unsigned long bin_pos;
    unsigned long bin_type;
}T_UP_BIN_INFO;

typedef struct
{
    unsigned long img_len;
    unsigned long img_pos;
    unsigned char  img_ID;
}T_DOWN_IMG_INFO;


typedef struct 
{
    unsigned long FormatStartPage;
	unsigned long Spi_TotalSize;
}T_WRITE_SPI_CONFIG_INFO;
//T_WRITE_SPI_CONFIG_INFO g_write_config_info = {0};


static unsigned long            m_transc_state = TRANS_NULL;
static unsigned long            m_transc_channelID = 0;
static T_CHIP_INFO      m_chipInfo;
static T_DL_CONTROL     m_dl_control;

static T_UP_BIN_INFO    m_bin_info = {0};

unsigned long                   g_parttion_start_page = 0;   // boot len
unsigned long                   g_burn_erase = 0;   // the 7th bit of burn_mode. 
unsigned char                   g_burn_medium = 0; 
unsigned char                   g_burn_mode = 0;
unsigned char                   g_burn_action = BURNED_NONE;
unsigned char                   g_erase_partition_size = 0;
unsigned char                   g_no_need_erase = 0;
T_PNANDFLASH            m_nandbase = NULL; 
//T_PMEDIUM               m_SDMedium = NULL; 
static T_DOWN_IMG_INFO  m_Down_Img_Info = {0};
static unsigned long m_chip_high_ID;/* used for nand flash high id*/

static T_SPI_FS_INFO g_spi_fs_info;

static unsigned long g_boot_page_num = 0;
static unsigned long g_file_lenght = 0;
static void *g_file_handle = NULL;
static unsigned long g_StartPage = 0;
static unsigned long g_current_erase_block = T_U32_MAX;
static bool g_check = false;
static bool g_first_flag = true;

extern T_PSPIFLASH m_pSpiflash;
extern signed long pp_printf(unsigned char * s, ...);
static void * fhalib_init(unsigned long eMedium, unsigned long eMode, void * pPhyInfo);
static bool DL_Check_Len(T_DL_STATUS status, unsigned long len);
static bool Handle_Transc_Data(unsigned long data, unsigned long len);
static bool ReadAsa_senddata(unsigned long data, unsigned long len);
static bool ReadAsa_getfilename(unsigned long data, unsigned long len);
static bool GetChannel_ID(unsigned long data, unsigned long len);
bool Sflash_Write_Boot_Data(unsigned char *pData, unsigned long data_len);
bool Sflash_Read_Boot_Data(unsigned char *pData, unsigned long data_len);
unsigned long Sflash_Read_spidata_Data(unsigned char *pData, unsigned long data_len);
unsigned long Sflash_BurnWrite_AllData(unsigned char *pData, unsigned long data_len);
unsigned long Sflash_Write_Data(unsigned char *pData, unsigned long data_len, unsigned long StartPage, unsigned long page_cnt, bool check);
bool Set_Burned_Param(unsigned char data[], unsigned long len);
unsigned long Test_Ex_Attr(void *file);
unsigned long Test_change_partition(void);




bool g_format_spiflash_fs_flag = false;
unsigned long g_format_spiflash_fs_startpage = 0;
unsigned long g_format_spiflash_fs_len = 0;
unsigned long g_format_spiflash_fs_len_temp = 0;


//Table of transc handle function
static T_ANYKA_TRANSC m_producer_transc[]=
{
    {TRANS_SWITCH_USB,            Transc_SwitchUsbHighSpeed,      NULL,                  NULL},
    {TRANS_TEST_CONNECT,          Transc_TestConnection,          NULL,                  NULL},
    {TRANS_SET_MODE,              Transc_SetMode,                 NULL,                  NULL},
    {TRANS_GET_FLASH_HIGH_ID,     Transc_GetFlashHighID,          NULL,                  Handle_Transc_Data}, 
    {TRANS_SET_ERASE_NAND_MODE,   Transc_SetErase_Mode,           NULL,                  NULL},
    {TRANS_GET_FLASHID,           Transc_GetFlashID,              NULL,                  Handle_Transc_Data}, 
    {TRANS_SET_NANDPARAM,         Transc_SetNandParam,            Handle_Transc_Data  ,  NULL},
    {TRANS_INIT_SECAREA,          Transc_InitSecArea,             NULL,                  NULL},
    {TRANS_SET_RESV,              Transc_SetResvAreaSize,         NULL,                  NULL},
    {TRANS_DOWNLOAD_BIN_START,    Transc_StartDLBin,              Handle_Transc_Data,    NULL},
    {TRANS_DOWNLOAD_BIN_DATA,     Transc_DLBin,                   Handle_Transc_Data,    NULL},
    {TRANS_DOWNLOAD_BOOT_START,   Transc_StartDLBoot,             Handle_Transc_Data,    NULL},
    {TRANS_DOWNLOAD_BOOT_DATA,    Transc_DLBoot,                  Handle_Transc_Data,    NULL},
    {TRANS_CLOSE,                 Transc_Close,                   NULL,                  NULL},
    {TRANS_CREATE_PARTITION,      Transc_CreatePartion,           Handle_Transc_Data,    NULL},
    {TRANS_MOUNT_DRIVER,          Transc_MountDriver,             Handle_Transc_Data,    NULL},
    {TRANS_FORMAT_DRIVER,         Transc_FormatDriver,            Handle_Transc_Data,    NULL},
    {TRANS_DOWNLOAD_FILE_START,   Transc_StartDLFile,             Handle_Transc_Data,    NULL},
    {TRANS_DOWNLOAD_FILE_DATA,    Transc_DLFile,                  Handle_Transc_Data,    NULL},
    {TRANS_DOWNLOAD_IMG_START,    Transc_StartDLImg,              Handle_Transc_Data,    NULL},
    {TRANS_DOWNLOAD_IMG_DATA,     Transc_DLImg,                   Handle_Transc_Data,    NULL},
    {TRANS_SET_SPIPARAM,          Transc_SetSPIParam,             Handle_Transc_Data,    NULL},
    {TRANS_UPLOAD_BIN_START,      Transc_GetBinStart,             Handle_Transc_Data,    NULL},
    {TRANS_UPLOAD_BIN_LEN,        Transc_GetBinLength,            NULL,                  Handle_Transc_Data},
    {TRANS_UPLOAD_BIN_DATA,       Transc_GetBinData,              NULL,                  Handle_Transc_Data},	
	{TRANS_RESET,                 Transc_Reset,                   NULL,                  NULL},
	{TRANS_SET_CHANNEL_ID,        Transc_SetChannel_ID,           NULL,                  NULL},
	{TRANS_GET_CHANNEL_ID,        Transc_GetChannel_ID,           NULL,                  GetChannel_ID},
	{TRANS_READ_ASA_FILE,	      Transc_ReadAsaFile, 		      ReadAsa_getfilename,   ReadAsa_senddata},
    {TRANS_UPLOAD_BOOT_START,     Transc_GetBootStart,            Handle_Transc_Data,    NULL},
    {TRANS_UPLOAD_BOOT_DATA,      Transc_GetBootData,             NULL,                  Handle_Transc_Data},	
    {TRANS_UPLOAD_BOOT_LEN,       Transc_GetBootLen,              NULL,                  Handle_Transc_Data},
    {TRANS_UPLOAD_SPIDATA_START,  Transc_GetSpiDataStart,         Handle_Transc_Data,    NULL},
    {TRANS_UPLOAD_SPIDATA_DATA,   Transc_GetSpiData,              NULL,                  Handle_Transc_Data},
    {TRANS_SET_EX_PARAMETER,      Transc_Set_EX_parameter,        NULL,                  NULL},
    {TRANS_SET_BURNEDPARAM,       Transc_BurnedParam,             Handle_Transc_Data,    NULL},
    
    {TRANS_DOWNLOAD_SPIDATA_START,Transc_Burned_Spidata_Start,    Handle_Transc_Data,    NULL},
    {TRANS_DOWNLOAD_SPIDATA_DATA, Transc_Burned_Spidata_Data,     Handle_Transc_Data,    NULL},
    {TRANS_ERASE_PARTITION_SIZE,  Transc_Erase64K_Partition_Size, NULL,                  NULL},
    
};


/**
 * @BREIF    Initial burn transc process to enter main loop by calling usb function
 * @AUTHOR   Jiang Dihui
 * @DATE     2009-09-10
 * @RETURN   bool
 * @retval   true :  succeed
 * @retval   false : fail
 */
bool Prod_Transc_Main()
{
    Fwl_Usb_Set_Trans(m_producer_transc, sizeof(m_producer_transc) / sizeof(T_ANYKA_TRANSC));
      
    Fwl_Usb_Main();

    return true;
}

  
/**
 * @BREIF    handle function of transc data stage(send or receive)
 * @AUTHOR   Jiang Dihui
 * @DATE     2009-09-10
 * @PARAM    [in] data buffer
 * @PARAM    [in] length of data
 * @RETURN   bool
 * @retval   true :  succeed
 * @retval   false : fail
 */

bool Handle_Transc_Data_inter(unsigned char data[], unsigned long len)
{
    bool ret = true;
    T_NAND_PHY_INFO* pNandInfo = NULL;
    T_PBIN_PARAM pBinInfo = NULL;
    T_CREAT_PARTITION_INFO *partition;
    unsigned long partition_cnt = 0;
    unsigned long tmp;
    unsigned long* pTmp;
    unsigned long i, DriverID;
    void *File = NULL;
    unsigned char partition_name[PARTITION_NAME_LEN +1] = {0};
   
    //pp_printf("PR_T state:%d, len:%d\r\n", m_transc_state, len);
   
    switch(m_transc_state)
    {
        case TRANS_GET_FLASHID:     //Get flash chip ID
            memcpy(data, &m_chipInfo, sizeof(T_CHIP_INFO));
            break;
        case TRANS_SET_BURNEDPARAM:
            Set_Burned_Param(data, len);
            break;  
        case TRANS_GET_FLASH_HIGH_ID:     //Get flash chip high ID
            memcpy(data, &m_chip_high_ID, sizeof(unsigned long));
            break;
            
        case TRANS_SET_NANDPARAM:   //Set nandflash param
            pNandInfo = (T_NAND_PHY_INFO*)data;
            // pNandInfo->flag &= ~0x10000000;      // 强行关闭multiplane
            m_nandbase = (T_PNANDFLASH)fhalib_init(g_burn_medium, g_burn_mode, pNandInfo);
                        
            if (NULL == m_nandbase)
            {
                ret = false;
            }

            break;
        case TRANS_SET_SPIPARAM:  //set spiflash
            if (NULL == fhalib_init(g_burn_medium, g_burn_mode, (T_SFLASH_PARAM *)data))
            {
                ret = false;
            }
            break;
        case TRANS_DOWNLOAD_IMG_START:
        case TRANS_DOWNLOAD_BIN_START:  //receive bin file information
        	pBinInfo = (T_PBIN_PARAM)data;
            //Test_change_partition();
            //pp_printf("test end:%s \n",pBinInfo->file_name);


            memset(partition_name, 0 , PARTITION_NAME_LEN +1);
            memcpy(partition_name, pBinInfo->file_name, PARTITION_NAME_LEN);
            //根据文件名进行打开分区
            pp_printf("pBinInfo->data_length:%d\n",pBinInfo->data_length);
            pp_printf("partition_name:%s\n",partition_name);
            pp_printf("pBinInfo->bCheck:%d\n",pBinInfo->bCheck);
            g_file_handle = partition_open(partition_name);
            if(NULL == g_file_handle)
            {
                ret = false;
            }
            else
            {
                ret = true;
                g_file_lenght = pBinInfo->data_length;
            }
            
            //Test_Ex_Attr(g_file_handle);
            
            
        	break;
        case TRANS_DOWNLOAD_IMG_DATA:
    	case TRANS_DOWNLOAD_BIN_DATA:   //receive bin file data
    	    //写分区的数据
            if(partition_write(g_file_handle, data, len) == -1)
            {
                ret = false;
            }
            else
            {
                g_file_lenght -= len;
                if(g_file_lenght == 0)
                {
                    if(partition_close(g_file_handle) == -1)
                    {
                        ret = false;
                    }
                    else
                    {
                        pp_printf("file write finish\n",len);
                        ret = true;
                    }
                    g_file_handle = NULL;
                }
            }
            
        	break;
		case TRANS_DOWNLOAD_BOOT_START: //Get download boot information
            g_file_lenght = *(unsigned long*)data;
            pp_printf("boot lenght: %d\n", g_file_lenght);

            //页数
            g_boot_page_num = (g_file_lenght + m_pSpiflash->page_size - 1) /m_pSpiflash->page_size;
            pp_printf("boot page num: %d\n", g_boot_page_num);
            
         
        	break;
    	case TRANS_DOWNLOAD_BOOT_DATA:  //download boot data
            //升级模块，原分区表的位置不变
            if(g_burn_mode != MODE_NEWBURN && g_first_flag)
    	    {
                unsigned long offset = 0;
                unsigned char  partition_block = 0;
                
                g_first_flag = false;
                //设置原分区表的位置
                
                partition_block = g_parttion_start_page/m_pSpiflash->PagesPerBlock;
                if(partition_block == 0 || partition_block == 0xFFFFFFFF)
                {
                    pp_printf("parttion start pos is error: %d\n", partition_block);
                    return false;
                }
                pp_printf("update partition block pos:%d\n", partition_block);

                //前三个页
                for (i = 0; i < (768-4); i++)
            	{
            		if (('S' == data[i]) && ('P' == data[i+1]) && ('I' == data[i+2]) && ('P' == data[i+3]))
            		{
            			offset = i;
            			break;
            		}
            	}

                if(i == (768- 4))
                {
                    pp_printf("not find partition block pos\n");
                    return false; 
                }

                //获取spi参数最后一保留成员的值
                pp_printf("update  boot partition block offset:%d\n", offset + 4 + sizeof(T_SFLASH_PHY_INFO) - 33);
                data[offset + 4 + sizeof(T_SFLASH_PHY_INFO) - 33] = partition_block;
                
                pp_printf("update data[%d]:%d\n", offset + 4 + sizeof(T_SFLASH_PHY_INFO) - 33, data[offset + 4 + sizeof(T_SFLASH_PHY_INFO) - 33]);
                
    	    }
        	if(!Sflash_Write_Boot_Data(data, len))
        	{
                return false;
        	}
        	break; 
        case TRANS_CREATE_PARTITION:    //create zone partion
        
            partition_cnt = ((unsigned long *)data)[0];
            pp_printf("partition_cnt:%d\n",partition_cnt);
            partition = (T_CREAT_PARTITION_INFO *)(data + sizeof(unsigned long));

            //全新烧录时先把分区表擦掉
            if(MODE_NEWBURN == g_burn_mode)
            {
                
                if(g_erase_partition_size)
                {
                    g_no_need_erase = 1; //由于全擦了，所以烧录过程中，就不需要再擦了
                    
                }
                else
                
                {
                    unsigned long block = g_parttion_start_page/m_pSpiflash->PagesPerBlock;
                    if(SF_FAIL == Fwl_SPIFlash_EraseBlock(m_pSpiflash, block, false))
                    {
                        pp_printf("Fwl_SPIFlash_EraseBlock partition block:%d fail\n", g_parttion_start_page / m_pSpiflash->PagesPerBlock);
                        return false;
                    }
                }
            }
            
            for(i = 0; i < partition_cnt; i++)
            {
                pp_printf("partition->name:%s\n",partition->name);
                pp_printf("partition->type:%d\n",partition->type);
                pp_printf("partition->r_w_flag:%d\n",partition->r_w_flag);
                pp_printf("partition->hidden_flag:%d\n",partition->hidden_flag);
                pp_printf("partition->Size:%d\n",partition->Size);
                pp_printf("partition->backup:%d\n",partition->backup);
                pp_printf("partition->check:%d\n",partition->check);
                pp_printf("partition->ld_addr:%x\n",partition->ld_addr);

                File = partition_creat(partition);
                if(0 == File)
                {
                    return false;
                }
                else
                {
                   if(partition_close(File) != 0)
                   {
                        return false;
                   }
                }
                partition++;
            }
            ret = true;
            break;    
           
        case TRANS_FORMAT_DRIVER:       //format driver
            pp_printf("not support format\r\n");
            ret = false;
            break;
        case TRANS_DOWNLOAD_FILE_START: //start download udisk file
            pp_printf("not support download file\r\n");
            ret = false;
            break;
        case TRANS_DOWNLOAD_FILE_DATA:  //download udisk file data
            pp_printf("not support download file\r\n");
            ret = false;
            break;
            
        case TRANS_UPLOAD_SPIDATA_START:
            g_file_lenght = ((unsigned long*)data)[0];
            pp_printf("g_file_lenght: %d\n", g_file_lenght);

            //页数
            g_file_lenght = (g_file_lenght + m_pSpiflash->page_size - 1) /m_pSpiflash->page_size;
            g_StartPage = 0;
            pp_printf("spidata page num: %d, g_StartPage:%d\n", g_file_lenght, g_StartPage);
            
           break;
        case  TRANS_UPLOAD_SPIDATA_DATA:
            ret = Sflash_Read_spidata_Data(data, len);
            break;
            
        case TRANS_DOWNLOAD_SPIDATA_START:
            pBinInfo = (T_PBIN_PARAM)data;
            g_StartPage = 0;
            g_file_lenght = pBinInfo->data_length;
            g_check = pBinInfo->bCheck;
            g_boot_page_num = (g_file_lenght + m_pSpiflash->page_size - 1) /m_pSpiflash->page_size;
            pp_printf("spidata len: %d, g_StartPage:%d\n", g_file_lenght, g_StartPage);
            pp_printf("spidata g_check: %d, spidata page num:%d\n", g_check, g_boot_page_num);
            if(g_no_need_erase == 0)
            {
                if(Fwl_SPIFlash_Erase_ALL_Block() == -1)
                {
                    pp_printf("Fwl_SPIFlash_EraseBlock partition block:%d fail\n", g_parttion_start_page / m_pSpiflash->PagesPerBlock);
                    return false;    
                }
                
                g_no_need_erase = 1;
            }
            break;
        case  TRANS_DOWNLOAD_SPIDATA_DATA:

            ret = Sflash_BurnWrite_AllData(data, len);

            break;
        case TRANS_UPLOAD_BIN_START:
            {
                T_BIN_PARAM bin_param = {0};
                
                memcpy(&bin_param.file_name, data, sizeof(bin_param.file_name));
                pp_printf("++ bin_param.file_name=%s ++\r\n", bin_param.file_name);
                memset(partition_name, 0, PARTITION_NAME_LEN +1);
                memcpy(partition_name, bin_param.file_name, PARTITION_NAME_LEN);
                g_file_handle = partition_open(partition_name);
                if(NULL == g_file_handle)
                {
                    return false;
                }
             }
            
            break;
        case TRANS_UPLOAD_BIN_LEN:
            g_file_lenght = partition_get_data_size(g_file_handle);
            ((unsigned long*)data)[0] = g_file_lenght;
            if(g_file_lenght == 0)
            {
                partition_close(g_file_handle);
                pp_printf("partition_get_data_size len=%d\r\n", g_file_lenght);
                return true;
            }
            pp_printf("++ upload bin len=%d ++\r\n", g_file_lenght);
            
            break;
        case TRANS_UPLOAD_BIN_DATA:
            pp_printf(".");
            if(partition_read(g_file_handle, data, len) == -1)
            {
               return false;
            }
            else
            {
                g_file_lenght -= len;
                if(g_file_lenght == 0)
                {
                    if(partition_close(g_file_handle) == -1)
                    {
                        return false;
                    }
                    else
                    {
                        return true;
                    }
                }
            }
            break;    
        case TRANS_UPLOAD_BOOT_START:

            g_boot_page_num = g_parttion_start_page - 1;
            g_StartPage = 0;//从0开始回读
            pp_printf("g_boot_page_num=%d ++\r\n", g_boot_page_num);
            break;
            
        case TRANS_UPLOAD_BOOT_LEN:
            pp_printf("g_boot_len =%d ++\r\n", g_boot_page_num*m_pSpiflash->page_size);
            ((unsigned long*)data)[0] = g_boot_page_num*m_pSpiflash->page_size;
            break;
            
        case TRANS_UPLOAD_BOOT_DATA:
            //pp_printf("++len=%d ++\r\n", len);
            ret = (true == Sflash_Read_Boot_Data(data, len));
            break; 
        default:
            break;           
    }

    return ret;    
}

bool Handle_Transc_Data(unsigned long data, unsigned long len)
{
    return Handle_Transc_Data_inter((unsigned char *)data,  len);
}



/**
 * @BREIF    transc of switch usb to high speed
 * @AUTHOR   Jiang Dihui
 * @DATE     2009-09-10
 * @PARAM    [in] data buffer
 * @PARAM    [in] length of data
 * @PARAM    [out] handle result
 * @RETURN   bool
 * @retval   true :  succeed
 * @retval   false : fail
 */
bool Transc_SwitchUsbHighSpeed(unsigned char data[], unsigned long len, T_CMD_RESULT *result)
{
    unsigned long i,j;
    pp_printf("++switch usb++\r\n");
    m_transc_state = TRANS_SWITCH_USB;

    //disable first
    pp_printf("T disable usb\r\n");
    ausb_disable();

//#ifdef CHIP_AK37XX
//    pp_printf("asic_before: %d\n", get_asic_freq());
//    pp_printf("pll_before: %dMhz\n", get_pll_value());
//#endif


     //wait again
    for(i = 0; i < 60*1000000/1; i++)
    {
        j++;
    }

//#ifdef CHIP_AK37XX
//    set_pll_value(248); //set pll to 248M
//    set_cpu_2x_asic(true);
//    pp_printf("asic: %d\n", get_asic_freq());
//    pp_printf("pll: %dMhz\n", get_pll_value());  
//#endif
    
    //enable
    pp_printf("T enable usb\r\n");

    ausb_enable(1);

    return true;
}


bool Set_Burned_Param(unsigned char data[], unsigned long len)
{
    bool ack = true;
    //unsigned long* pData = data;
    T_BURNED_PARAM burned_param = {0};
    unsigned long i = 0;
  
    pp_printf("++Transc_BurnedParam++\r\n");

    for(i = 0; i < len; i++)
    {
        if(i %16 == 0)
        {
            pp_printf("\r\n");
        }
        pp_printf("%02x ", data[i]);
    }
    
    memset(&burned_param, 0, sizeof(burned_param));
	memcpy(&burned_param, data, sizeof(burned_param));
    pp_printf("burned_param.burned_mode:%d\r\n", burned_param.burned_mode);
    
	if (burned_param.burned_mode == BURNED_POWER_OFF)
	{
        pp_printf("BurnedParam power off\r\n");

        g_burn_action = BURNED_POWER_OFF;
	}
    else if (burned_param.burned_mode == BURNED_RESET)
    {
        pp_printf("BurnedParam reset\r\n");
         //未实现
         g_burn_action = BURNED_RESET;
    }
    else
    {
        pp_printf("BurnedParam none\r\n");
        g_burn_action = BURNED_NONE;
    }

    return ack;
}



bool Transc_BurnedParam(unsigned char data[], unsigned long len, T_CMD_RESULT *result)
{
    bool ack = true;
    //unsigned long* pData = data;
    T_BURNED_PARAM burned_param = {0};
    unsigned long i = 0;
  
    pp_printf("++Transc_BurnedParam++\r\n");
    m_transc_state = TRANS_SET_BURNEDPARAM;
    return ack;
}



/**
 * @BREIF    transc of test connection
 * @AUTHOR   Jiang Dihui
 * @DATE     2009-09-10
 * @PARAM    [in] data buffer
 * @PARAM    [in] length of data
 * @PARAM    [out] handle result
 * @RETURN   bool
 * @retval   true :  succeed
 * @retval   false : fail
 */
bool Transc_TestConnection(unsigned char data[], unsigned long len, T_CMD_RESULT *result)
{
    bool ack = true;
    unsigned long* pData = (unsigned long*)data;
  
    pp_printf("++test connect++\r\n");
    m_transc_state = TRANS_TEST_CONNECT;

    if('B' != pData[0] || 'T' != pData[1])
    {
        ack = false;
    }
    
    return ack;
}

/**
 * @BREIF    transc of Get nand flash high ID(5th,6th bytes)
 * @AUTHOR   Jiang Dihui
 * @DATE     2013-09-1
 * @PARAM    [in] data buffer
 * @PARAM    [in] length of data
 * @PARAM    [out] handle result
 * @RETURN   bool
 * @retval   true :  succeed
 * @retval   false : fail
 */

bool Transc_GetFlashHighID(unsigned char data[], unsigned long len, T_CMD_RESULT *result)
{
    m_transc_state = TRANS_GET_FLASH_HIGH_ID;
    pp_printf("++get chip ID++\r\nnand high id:%x\n",m_chip_high_ID);    
    return true;
}



/**
 * @BREIF    transc of set EX parameter 
 * @AUTHOR   lixingjian
 * @DATE     2016-9-30
 * @PARAM    [in] data buffer
 * @PARAM    [in] length of data
 * @PARAM    [out] handle result
 * @RETURN   bool
 * @retval   true :  succeed
 * @retval   false : fail
 */
bool Transc_Set_EX_parameter(unsigned char data[], unsigned long len, T_CMD_RESULT *result)
{
    bool ack = true;
    unsigned long* ptmp = (unsigned long*)data;

    
    m_transc_state = TRANS_SET_EX_PARAMETER;

    if(MODE_NEWBURN == g_burn_mode)
    {
        g_parttion_start_page = ptmp[0]; //获取boot的大小
        
    }
    
    pp_printf("g_parttion_start_page: %d\r\n", g_parttion_start_page);
    return ack;
}



/**
 * @BREIF    transc of set erase nand mode
 * @AUTHOR   lixingjian
 * @DATE     2012-10-26
 * @PARAM    [in] data buffer
 * @PARAM    [in] length of data
 * @PARAM    [out] handle result
 * @RETURN   bool
 * @retval   true :  succeed
 * @retval   false : fail
 */
bool Transc_SetErase_Mode(unsigned char data[], unsigned long len, T_CMD_RESULT *result)
{
    bool ack = true;
    unsigned long* ptmp = (unsigned long*)data;

    
    m_transc_state = TRANS_SET_ERASE_NAND_MODE;

    g_burn_erase = ptmp[0]; //setmode后重新给值
    //实现erase功能
    if (g_burn_erase && MODE_NEWBURN == g_burn_mode && (MEDIUM_NAND == g_burn_medium || MEDIUM_SPI_NAND == g_burn_medium ))   // ERASE
    {
        pp_printf("++set erase nand mode: %d++\r\n", ptmp[0]);
        if(0)//false == Fwl_Erase_block())
        {
            pp_printf("Fwl_Erase_block fail\r\n");
            return false;
        }
    }

    if (g_burn_erase && MODE_NEWBURN == g_burn_mode && (MEDIUM_SPIFLASH == g_burn_medium))   // ERASE
    {
        pp_printf("++set erase spiflash mode: %d++\r\n", ptmp[0]);
        if(-1 == Fwl_SPIFlash_Erase_ALL_Block())
        {
            pp_printf("Fwl_SPIFlash_Erase_ALL_Block fail\r\n");
            return false;
        }
    }
    
    pp_printf("++g_burn_erase: %d++\r\n", g_burn_erase);
    return ack;
}



/**
 * @BREIF    transc of set burn mode
 * @AUTHOR   Jiang Dihui
 * @DATE     2009-09-10
 * @PARAM    [in] data buffer
 * @PARAM    [in] length of data
 * @PARAM    [out] handle result
 * @RETURN   bool
 * @retval   true :  succeed
 * @retval   false : fail
 */
bool Transc_SetMode(unsigned char data[], unsigned long len, T_CMD_RESULT *result)
{
    bool ack = true;
    T_MODE_CONTROL ModeCtrl;

    pp_printf("++set mode++\r\n");
    m_transc_state = TRANS_SET_MODE;

    memcpy(&ModeCtrl, data, sizeof(T_MODE_CONTROL));
    g_burn_medium = ModeCtrl.eMedium;
    g_burn_mode = ModeCtrl.burn_mode;
    g_erase_partition_size = ModeCtrl.erase_partition_size;
    g_no_need_erase = ModeCtrl.no_need_erase;

    pp_printf("T mode:%d, medium:%d, erase:%d, no erase:%d\r\n", g_burn_mode, g_burn_medium, g_erase_partition_size, g_no_need_erase);
    #if 0
    if (MEDIUM_EMMC == g_burn_medium)
    {
        if (NULL == (m_SDMedium = fhalib_init(g_burn_medium, g_burn_mode, NULL)))
        {
            ack = false;
        }    
    }
    #endif
    
    return ack;
}

/**
 * @BREIF    transc of Get flash ID
 * @AUTHOR   Jiang Dihui
 * @DATE     2009-09-10
 * @PARAM    [in] data buffer
 * @PARAM    [in] length of data
 * @PARAM    [out] handle result
 * @RETURN   bool
 * @retval   true :  succeed
 * @retval   false : fail
 */
bool Transc_GetFlashID(unsigned char data[], unsigned long len, T_CMD_RESULT *result)
{
    bool ack = true;
    unsigned long *pTmp;
    unsigned long nChipCnt;
    unsigned long nChipID[2];
    
    pp_printf("++get chip ID++\r\n");
    m_transc_state = TRANS_GET_FLASHID;

    //Get flash ID and chip count
    pTmp = (unsigned long*)data;
    if (MEDIUM_SPI_NAND == g_burn_medium)
    {
        m_chip_high_ID = 0;
        //ack = Fwl_NandHWInit(pTmp[0], pTmp[1], nChipID, &nChipCnt);
        m_chipInfo.chip_ID = nChipID[0];
        m_chip_high_ID = nChipID[1];
        m_chipInfo.chip_cnt = nChipCnt;
        //pp_printf("nand id:%x,%x\n",m_chip_high_ID,m_chipInfo.chip_ID);
    }
    else if (MEDIUM_SPIFLASH  == g_burn_medium)
    {
	    ack = Fwl_SPIHWInit(&m_chipInfo.chip_ID, &m_chipInfo.chip_cnt);
    }
    
    return ack;
}

/**
 * @BREIF    transc of Initial sec area
 * @AUTHOR   Jiang Dihui
 * @DATE     2009-09-10
 * @PARAM    [in] data buffer
 * @PARAM    [in] length of data
 * @PARAM    [out] handle result
 * @RETURN   bool
 * @retval   true :  succeed
 * @retval   false : fail
 */
bool Transc_InitSecArea(unsigned char data[], unsigned long len, T_CMD_RESULT *result)
{
    bool ack = true;
    unsigned long ret = 0;
    pp_printf("++init secArea++\r\n");
    m_transc_state = TRANS_INIT_SECAREA;

    #if 0
    ret = FHA_asa_scan(true);

    if (0 != ret)
    {
        if(MODE_NEWBURN == g_burn_mode)
        {
           pp_printf("T asa format\r\n");

           if(-1 == FHA_asa_format(((unsigned long*)data)[0]))
           {
                pp_printf("T asa format err\r\n");
                ack = false;
           }
        }
        else
        {
            pp_printf("T scan asa err\r\n");
            ack = false;
        }
    }
    #endif
    
    return ack;
}

/**
 * @BREIF    transc of set nand param
 * @AUTHOR   Jiang Dihui
 * @DATE     2009-09-10
 * @PARAM    [in] data buffer
 * @PARAM    [in] length of data
 * @PARAM    [out] handle result
 * @RETURN   bool
 * @retval   true :  succeed
 * @retval   false : fail
 */
bool Transc_SetNandParam(unsigned char data[], unsigned long len, T_CMD_RESULT *result)
{
    pp_printf("++set nand param++\r\n");
    m_transc_state = TRANS_SET_NANDPARAM;
    
    return true;
}

/**
 * @BREIF    transc of set SPI param
 * @AUTHOR   Jiang Dihui
 * @DATE     2011-05-23
 * @PARAM    [in] data buffer
 * @PARAM    [in] length of data
 * @PARAM    [out] handle result
 * @RETURN   bool
 * @retval   true :  succeed
 * @retval   false : fail
 */
bool Transc_SetSPIParam(unsigned char data[], unsigned long len, T_CMD_RESULT *result)
{
    pp_printf("++set SPI param++\r\n");
    m_transc_state = TRANS_SET_SPIPARAM;
    
    return true;
}


/**
 * @BREIF    transc of set user reserver area size
 * @AUTHOR   Jiang Dihui
 * @DATE     2009-09-10
 * @PARAM    [in] data buffer
 * @PARAM    [in] length of data
 * @PARAM    [out] handle result
 * @RETURN   bool
 * @retval   true :  succeed
 * @retval   false : fail
 */
bool Transc_SetResvAreaSize(unsigned char data[], unsigned long len, T_CMD_RESULT *result)
{
    bool ack = true;
    unsigned long* ptmp = (unsigned long*)data;
    
    pp_printf("++set ResvArea++\r\n");
    m_transc_state = TRANS_SET_RESV;

    pp_printf("T ResvArea size:%d, bErase:%d\r\n", ptmp[0], ptmp[1]);

    #if 0
    if (FHA_FAIL == FHA_set_resv_zone_info(ptmp[0], (bool)ptmp[1]))
    {
        ack = false;
    } 
    #endif
      
    return ack;
}


/**
 * @BREIF    transc of start download bin
 * @AUTHOR   Jiang Dihui
 * @DATE     2009-09-10
 * @PARAM    [in] data buffer
 * @PARAM    [in] length of data
 * @PARAM    [out] handle result
 * @RETURN   bool
 * @retval   true :  succeed
 * @retval   false : fail
 */
bool Transc_StartDLBin(unsigned char data[], unsigned long len, T_CMD_RESULT *result)
{
    pp_printf("++start bin++\r\n");
    m_transc_state = TRANS_DOWNLOAD_BIN_START;

    return true;
}

/**
 * @BREIF    transc of download bin data
 * @AUTHOR   Jiang Dihui
 * @DATE     2009-09-10
 * @PARAM    [in] data buffer
 * @PARAM    [in] length of data
 * @PARAM    [out] handle result
 * @RETURN   bool
 * @retval   true :  succeed
 * @retval   false : fail
 */
bool Transc_DLBin(unsigned char data[], unsigned long len, T_CMD_RESULT *result)
{
    bool ack = true;

   // pp_printf("++DL bin, len:%d++\r\n", len);
    m_transc_state = TRANS_DOWNLOAD_BIN_DATA;

    //check length of data
    ack = DL_Check_Len(DL_WRITE, len);

    return ack;
}

/**
 * @BREIF    transc of start download boot
 * @AUTHOR   Jiang Dihui
 * @DATE     2009-09-10
 * @PARAM    [in] data buffer
 * @PARAM    [in] length of data
 * @PARAM    [out] handle result
 * @RETURN   bool
 * @retval   true :  succeed
 * @retval   false : fail
 */
bool Transc_StartDLBoot(unsigned char data[], unsigned long len, T_CMD_RESULT *result)
{
    pp_printf("++start boot++\r\n");
    m_transc_state = TRANS_DOWNLOAD_BOOT_START;

    return true;
}

/**
 * @BREIF    transc of download boot data
 * @AUTHOR   Jiang Dihui
 * @DATE     2009-09-10
 * @PARAM    [in] data buffer
 * @PARAM    [in] length of data
 * @PARAM    [out] handle result
 * @RETURN   bool
 * @retval   true :  succeed
 * @retval   false : fail
 */
bool Transc_DLBoot(unsigned char data[], unsigned long len, T_CMD_RESULT *result)
{
    bool ack = true;

    pp_printf("++DL boot, len:%d++\r\n", len);
    m_transc_state = TRANS_DOWNLOAD_BOOT_DATA;

   //check length of data
    ack = DL_Check_Len(DL_WRITE, len);

    return ack;
}

/**
 * @BREIF    transc of close to write config information
 * @AUTHOR   Jiang Dihui
 * @DATE     2009-09-10
 * @PARAM    [in] data buffer
 * @PARAM    [in] length of data
 * @PARAM    [out] handle result
 * @RETURN   bool
 * @retval   true :  succeed
 * @retval   false : fail
 */
bool Transc_Close(unsigned char data[], unsigned long len, T_CMD_RESULT *result)
{
    bool ack = true;
    unsigned long page,offset;
	
    pp_printf("burn finish\r\n");
	m_transc_state = TRANS_CLOSE;

    if(BURNED_RESET == g_burn_action)
    {
        pp_printf("burn reset\r\n");
        watchdog_timer_start(2);
    }
    else if(BURNED_POWER_OFF == g_burn_action)
    {
        pp_printf("burn power off\r\n");
        //39这边暂没有这个
    }
    else
    {
         pp_printf("burn none\r\n");
         
    }

    #if 0
    if(PARTITION_SUCCESS != FHA_close())
    {
        ack = false;
    } 
    #endif

	pp_printf("Burn %s!\n", (true == ack)?"successful":"unsuccessful");
   
    return ack;
}

/**
 * @BREIF    transc of create file system partion
 * @AUTHOR   Jiang Dihui
 * @DATE     2009-09-10
 * @PARAM    [in] data buffer
 * @PARAM    [in] length of data
 * @PARAM    [out] handle result
 * @RETURN   bool
 * @retval   true :  succeed
 * @retval   false : fail
 */

static bool  Transc_CreatePartion(unsigned char data[], unsigned long len, T_CMD_RESULT *result)
{
    bool ack = true;
 
    pp_printf("++create partion++\r\n");
    m_transc_state = TRANS_CREATE_PARTITION;

    #if 0
    if (Fwl_MountInit())
    {
        if (!Fwl_FSAInit())
        {
            ack = false;
        }    
    }    
    #endif

    return ack;
}

/**
 * @BREIF    transc of format driver
 * @AUTHOR   Jiang Dihui
 * @DATE     2009-09-10
 * @PARAM    [in] data buffer
 * @PARAM    [in] length of data
 * @PARAM    [out] handle result
 * @RETURN   bool
 * @retval   true :  succeed
 * @retval   false : fail
 */
bool  Transc_FormatDriver(unsigned char data[], unsigned long len, T_CMD_RESULT *result)
{
    pp_printf("++format driver++\r\n");
    m_transc_state = TRANS_FORMAT_DRIVER;
    
    return true;
}

/*
 * @BREIF    transc of mount driver
 * @AUTHOR   Jiang Dihui
 * @DATE     2009-09-10
 * @PARAM    [in] data buffer
 * @PARAM    [in] length of data
 * @PARAM    [out] handle result
 * @RETURN   bool
 * @retval   true :  succeed
 * @retval   false : fail
 */
bool  Transc_MountDriver(unsigned char data[], unsigned long len, T_CMD_RESULT *result)
{
    pp_printf("++mount driver++\r\n");
    m_transc_state = TRANS_MOUNT_DRIVER;
    
    return true;
}

/**
 * @BREIF    transc of start download udisk file
 * @AUTHOR   Jiang Dihui
 * @DATE     2009-09-10
 * @PARAM    [in] data buffer
 * @PARAM    [in] length of data
 * @PARAM    [out] handle result
 * @RETURN   bool
 * @retval   true :  succeed
 * @retval   false : fail
 */
bool Transc_StartDLFile(unsigned char data[], unsigned long len, T_CMD_RESULT *result)
{
    pp_printf("++start file++\r\n");
    m_transc_state = TRANS_DOWNLOAD_FILE_START;

    return true;
}

bool Transc_Burned_Spidata_Start(unsigned char data[], unsigned long len, T_CMD_RESULT *result)
{
    pp_printf("++Spidata_Start++\r\n");
    m_transc_state = TRANS_DOWNLOAD_SPIDATA_START;

    return true;
}

bool Transc_Burned_Spidata_Data(unsigned char data[], unsigned long len, T_CMD_RESULT *result)
{
    //pp_printf("++start Spidata_Data++\r\n");
    m_transc_state = TRANS_DOWNLOAD_SPIDATA_DATA;

    return true;
}


bool Transc_Erase64K_Partition_Size(unsigned char data[], unsigned long len, T_CMD_RESULT *result)
{
    unsigned long* ptmp = (unsigned long*)data;
    T_ERASE_BLOCK erase_b;
    unsigned long i = 0;

    
    m_transc_state = TRANS_SET_ERASE_NAND_MODE;

    if(g_erase_partition_size)
    {

        erase_b.start_block = ptmp[0]; 
        erase_b.end_block = ptmp[1]; 
        
        //pp_printf("erase partition startb:%d, endb:%d++\r\n", erase_b.start_block, erase_b.end_block);
        //实现erase功能
        for(i = erase_b.start_block; i < erase_b.end_block; i++)
        {
            //64K擦
            //pp_printf("erase 64K block:%d\n", i);
            pp_printf("e.");
            if(SF_FAIL == Fwl_SPIFlash_EraseBlock(m_pSpiflash, i, true))
            {
                pp_printf("Fwl_SPIFlash_EraseBlock partition block:%d fail\n", i);
                return false;
            }
        }
        
        //pp_printf("g_burn_erase: %d\r\n", g_no_need_erase);
    }
    
    
    return true;
}







/**
 * @BREIF    transc of download udisk file
 * @AUTHOR   Jiang Dihui
 * @DATE     2009-09-10
 * @PARAM    [in] data buffer
 * @PARAM    [in] length of data
 * @PARAM    [out] handle result
 * @RETURN   bool
 * @retval   true :  succeed
 * @retval   false : fail
 */
bool Transc_DLFile(unsigned char data[], unsigned long len, T_CMD_RESULT *result)
{
    bool ack = true;

    //gpf.fDriver.Printf("PR_T ++DL file, len:%d++\r\n", len);
    m_transc_state = TRANS_DOWNLOAD_FILE_DATA;

    //check length of data
    ack = DL_Check_Len(DL_WRITE, len);

    return ack;
}

/**
 * @BREIF    transc of start download image
 * @AUTHOR   Jiang Dihui
 * @DATE     2009-09-10
 * @PARAM    [in] data buffer
 * @PARAM    [in] length of data
 * @PARAM    [out] handle result
 * @RETURN   bool
 * @retval   true :  succeed
 * @retval   false : fail
 */
bool Transc_StartDLImg(unsigned char data[], unsigned long len, T_CMD_RESULT *result)
{
     pp_printf("++start img++\r\n");
     m_transc_state = TRANS_DOWNLOAD_IMG_START;
         
     return true;
}

/**
 * @BREIF    transc of download image data
 * @AUTHOR   Jiang Dihui
 * @DATE     2009-09-10
 * @PARAM    [in] data buffer
 * @PARAM    [in] length of data
 * @PARAM    [out] handle result
 * @RETURN   bool
 * @retval   true :  succeed
 * @retval   false : fail
 */
bool Transc_DLImg(unsigned char data[], unsigned long len, T_CMD_RESULT *result)
{
    bool ack = true;
 
    m_transc_state = TRANS_DOWNLOAD_IMG_DATA;

    //check length of data
    ack = DL_Check_Len(DL_WRITE, len);
   
    return ack;
}

//*************************************************************
static bool DL_Check_Len(T_DL_STATUS status, unsigned long len)
{
    return true;
/*
    bool ret = true;
    
    switch(status)
    {
        case DL_START:  
            m_dl_control.file_total_len = len;
            m_dl_control.data_total = 0;
            break;
         case DL_WRITE:
            m_dl_control.data_total += len;

            if(m_dl_control.data_total > m_dl_control.file_total_len 
                || ((0 == len) && (m_dl_control.data_total < m_dl_control.file_total_len)))
            {
                pp_printf("LOST DATA,data_toal:%d, file_len:%d, len:%d\r\n", m_dl_control.data_total, m_dl_control.file_total_len, len);
                ret = false;
            }
            break;
    }

    return ret;
    */
}


unsigned long get_partition_start_page(T_PSPIFLASH pspiflash)
{
    unsigned long boot_len = 0;//
    unsigned char boot_temp = 0;//
    unsigned char *buf = NULL;
    unsigned long offset = 0;
    unsigned long page_cnt = 3;//读三个页
    unsigned long i=0;

    buf =  (unsigned char *)Fwl_Malloc(sizeof(unsigned char)*page_cnt*pspiflash->page_size);
    if(NULL == buf)
    {
        pp_printf("update burn buf malloc fail\n");
        return 0;
    }

    memset(buf, 0, sizeof(unsigned char)*page_cnt*pspiflash->page_size);
    
    //获取  bin分区大小
    if (!spi_flash_read(0, buf, page_cnt))//读3个页
	{
        pp_printf("update burn spi_flash_read fail\n");
		return 0;
	}
    
    for (i = 0; i < page_cnt*pspiflash->page_size - 4; i++)
	{
		if (('S' == buf[i]) && ('P' == buf[i+1]) && ('I' == buf[i+2]) && ('P' == buf[i+3]))
		{
			offset = i;
			break;
		}
	}

    if(i == page_cnt*pspiflash->page_size- 4)
    {
       pp_printf("not find partition start page\n");
        return 0; 
    }

    //获取spi参数最后一保留成员的值
    pp_printf("boot block num offset:%d\n", offset + 4 + sizeof(T_SFLASH_PHY_INFO) - 33);
    memcpy(&boot_temp, buf + offset + 4 + sizeof(T_SFLASH_PHY_INFO) - 33, 1);//获取到的值是块数
 
    Fwl_Free(buf);
    
    boot_len = boot_temp*pspiflash->PagesPerBlock;//获取到页数
    pp_printf("get_upadte_boot_size g_boot_len:%d\n", boot_len);

    return boot_len;
}


//**********************************************************************
static void * fhalib_init(unsigned long eMedium, unsigned long eMode, void * pPhyInfo)
{
    T_FHA_LIB_CALLBACK  pCB;
    //T_FHA_INIT_INFO     pInit;
    T_PNANDFLASH        pNandFlash = NULL;
    //T_PMEDIUM           pmedium = NULL;
    T_PSPIFLASH         pspiflash = NULL;
    T_SPI_INIT_INFO     spi_fha; 
    void *             pFwlInfo = NULL;
    void *             pInfo = NULL;
    unsigned long  temp = 0;
   
    if (MEDIUM_SPI_NAND == eMedium )
    {
        //nand init
        #if 0
        pCB.Erase = FHA_Nand_EraseBlock;
        pCB.Write = FHA_Nand_WritePage;
        pCB.Read  = FHA_Nand_ReadPage;
        pCB.ReadNandBytes = ASA_ReadBytes;

        pNandFlash = Nand_Init((T_NAND_PHY_INFO *)pPhyInfo);
        pFwlInfo   = (void *)pNandFlash;
        pInfo = pPhyInfo;
        #endif
    }

    else if (MEDIUM_SPIFLASH == eMedium)
    {
        //for spi init
        pCB.Erase  = FHA_Spi_Erase;
        pCB.Write  = FHA_Spi_Write;
        pCB.Read   = FHA_Spi_Read;
        pCB.ReadNandBytes = NULL;

        pspiflash = Fwl_SPIFlash_Init((T_SFLASH_PARAM *)pPhyInfo);

        if(g_burn_mode != MODE_NEWBURN)
        {
            g_parttion_start_page = get_partition_start_page(pspiflash);//
            if(g_parttion_start_page == 0)
            {
                pp_printf("error update g_parttion_start_page == 0\n");
                return NULL;
            }
            
        }
        else
        {   
            //传过来是已进行块对齐的页数
            g_parttion_start_page = g_parttion_start_page * pspiflash->PagesPerBlock;
            
        }
        
        pp_printf("partition table start page:%d\n", g_parttion_start_page);
        spi_fha.page_size = pspiflash->page_size;
        spi_fha.pages_per_block = pspiflash->PagesPerBlock;
        //spi_fha.partition_table_start= g_parttion_start_page;
        spi_fha.total_size = pspiflash->total_page*pspiflash->page_size;
        //g_write_config_info.Spi_TotalSize = pspiflash->page_size * pspiflash->total_page;

        pFwlInfo = (void *)pspiflash;
        pInfo    = (void *)(&spi_fha);

        
    }
    else
    {
        pp_printf("medium type is err\n");
        return NULL;
    } 

    
    pCB.RamAlloc = Fwl_Malloc; 
    pCB.RamFree  = Fwl_Free;
    pCB.MemSet   = (void *)memset;
    pCB.MemCpy   = (void *)memcpy;
    pCB.MemCmp   = (void *)memcmp;
    pCB.MemMov   = (void *)memmove;
    pCB.Printf   = pp_printf;

    
    if (0 == partition_init(&pCB, pInfo, g_parttion_start_page))
    {
        return pFwlInfo;
    }
    else
    {
        pp_printf("partition_init inits fail\n");
        return NULL;
    }    
}


#if 0
static bool SetLibVersion(void)
{

	T_LIB_VER_INFO Lib_version[sizeof(version_info)/sizeof(version_info[0])];
	unsigned long i;

	for(i = 0; i < sizeof(version_info)/sizeof(version_info[0]); ++i)
	{
		strncpy(Lib_version[i].lib_name,version_info[i].pLibName,
			sizeof(Lib_version[i].lib_name));
		strncpy(Lib_version[i].lib_version,version_info[i].pVerFun(),
			sizeof(Lib_version[i].lib_version));

		pp_printf("%s->%s\n", Lib_version[i].lib_name, 
			Lib_version[i].lib_version);
	}

	i = FHA_set_lib_version(&Lib_version, 
			sizeof(version_info)/sizeof(version_info[0]));	

	return (FHA_SUCCESS == i)? true:false;	
}



static bool CheckLibVersion(void)
{
	T_LIB_VER_INFO Lib_version[sizeof(version_info)/sizeof(version_info[0])];
	unsigned long i;
	unsigned long uRet;

	for(i = 0; i < sizeof(version_info)/sizeof(version_info[0]); ++i)
	{
		strncpy(Lib_version[i].lib_name,version_info[i].pLibName,
			sizeof(Lib_version[i].lib_name));
		strncpy(Lib_version[i].lib_version,version_info[i].pVerFun(),
			sizeof(Lib_version[i].lib_version));

		pp_printf("call FHA_check_lib_version\n");
		uRet = FHA_check_lib_version(&(Lib_version[i]));
		if(FHA_FAIL == uRet)
		{
			pp_printf("%s no mathc\n", Lib_version[i].lib_name);
			pp_printf("%s->%s\n", Lib_version[i].lib_name, 
				Lib_version[i].lib_version);

			return false;
		}
	}

	return true;
	
}

#endif



bool Transc_GetBootStart(unsigned char data[], unsigned long len, T_CMD_RESULT *result)
{
    pp_printf("PR_T ++start upload boot++\r\n");
    m_transc_state = TRANS_UPLOAD_BOOT_START;

    return true;
}
bool Transc_GetBootData(unsigned char data[], unsigned long len, T_CMD_RESULT *result)
{
    //pp_printf("PR_T ++upload boot data++\r\n");
    m_transc_state = TRANS_UPLOAD_BOOT_DATA;

    //Ack_Transc(USB_STATUS_SUCCESS, BT_SUCCESS, DATA_STAGE_SEND, len, result);

    return true;
}

bool Transc_GetBootLen(unsigned char data[], unsigned long len, T_CMD_RESULT *result)
{
    pp_printf("PR_T ++upload boot data++\r\n");
    m_transc_state = TRANS_UPLOAD_BOOT_LEN;

    //Ack_Transc(USB_STATUS_SUCCESS, BT_SUCCESS, DATA_STAGE_SEND, len, result);

    return true;
 }

bool Transc_GetSpiDataStart(unsigned char data[], unsigned long len, T_CMD_RESULT *result)
{
    pp_printf("PR_T ++start upload spi data++\r\n");
    m_transc_state = TRANS_UPLOAD_SPIDATA_START;

    return true;
}

bool Transc_GetSpiData(unsigned char data[], unsigned long len, T_CMD_RESULT *result)
{
    //pp_printf("PR_T ++upload bin len:%d++\r\n", len);
    m_transc_state = TRANS_UPLOAD_SPIDATA_DATA;

    return true;
}





bool Transc_GetBinStart(unsigned char data[], unsigned long len, T_CMD_RESULT *result)
{
    pp_printf("PR_T ++start upload bin++\r\n");
    m_transc_state = TRANS_UPLOAD_BIN_START;

    return true;
}

bool Transc_GetBinLength(unsigned char data[], unsigned long len, T_CMD_RESULT *result)
{
    //pp_printf("PR_T ++upload bin len:%d++\r\n", len);
    m_transc_state = TRANS_UPLOAD_BIN_LEN;

    return true;
}

bool Transc_GetBinData(unsigned char data[], unsigned long len, T_CMD_RESULT *result)
{
    //pp_printf("PR_T ++upload bin data++\r\n");
    m_transc_state = TRANS_UPLOAD_BIN_DATA;

    //Ack_Transc(USB_STATUS_SUCCESS, BT_SUCCESS, DATA_STAGE_SEND, len, result);

    return true;
}

static T_WRITE_ASA_FILE* pAsaFile = NULL;

static bool Transc_Reset(unsigned char data[], unsigned long len, T_CMD_RESULT *result)
{
    void (*f)() = 0;
    unsigned long i, j;
        
    pp_printf("T ++reset++\r\n");
    
    ausb_disable();

    for(i = 0; i < 1000*1000; i++)
    {
        j = i;
    }
    
    f();
}


 /**
  * @BREIF    transc of set cannel id
  * @AUTHOR   lixingjian
  * @DATE     2012-5-29
  * @PARAM    [in] data buffer
  * @PARAM    [in] length of data
  * @PARAM    [out] handle result
  * @RETURN   bool
  * @retval   true :  succeed
  * @retval   false : fail
  */
 
bool Transc_SetChannel_ID(unsigned char data[], unsigned long len, T_CMD_RESULT *result)
{
	unsigned long* pBuf = NULL;

	 pBuf = (unsigned long *)data;
	
    pp_printf("++set cannel id:%d ++\r\n", pBuf[0]);
    m_transc_channelID = pBuf[0];
    pp_printf("++set cannel id:%d ++\r\n", m_transc_channelID);
    
    return true;
}


 /**
 * @BREIF    transc of get cannel id
 * @AUTHOR   lixingjian
 * @DATE     2012-5-29
 * @PARAM    [in] data buffer
 * @PARAM    [in] length of data
 * @PARAM    [out] handle result
 * @RETURN   bool
 * @retval   true :  succeed
 * @retval   false : fail
 */
 
bool GetChannel_ID_inter(unsigned char data[], unsigned long len)
{
	unsigned long* pBuf = NULL;
	
	memcpy(data, &m_transc_channelID, sizeof(m_transc_channelID));
    pp_printf("++get cannel id:%d,%d,%d,%d ++\r\n", data[0], data[1], data[2], data[3]);
    
    return true;
}

bool GetChannel_ID(unsigned long data, unsigned long len)
{
    return GetChannel_ID_inter((unsigned char *)data, len);
}

/**
 * @BREIF    transc of get cannel id
 * @AUTHOR   lixingjian
 * @DATE     2012-5-29
 * @PARAM    [in] data buffer
 * @PARAM    [in] length of data
 * @PARAM    [out] handle result
 * @RETURN   bool
 * @retval   true :  succeed
 * @retval   false : fail
 */
 
bool Transc_GetChannel_ID(unsigned char data[], unsigned long len, T_CMD_RESULT *result)
{
	unsigned long* pBuf = NULL;
	pp_printf("++get cannel id:%d ++\r\n", m_transc_channelID);
    m_transc_state = TRANS_GET_CHANNEL_ID;
    
    return true;
}


/**
 * @BREIF   read asa file
 * @AUTHOR   lixingjian
 * @DATE     2012-5-29
 * @PARAM    [in] data buffer
 * @PARAM    [in] length of data
 * @PARAM    [out] handle result
 * @RETURN   bool
 * @retval   true :  succeed
 * @retval   false : fail
 */
 
bool Transc_ReadAsaFile(unsigned char data[], unsigned long len, T_CMD_RESULT *result)
{
  	unsigned long* pBuf = NULL;
    
    pp_printf("++read asa file++\r\n");

    pBuf = (unsigned long *)data;

    if(0 == pBuf[0])
    {
        pAsaFile = Fwl_Malloc(sizeof(T_WRITE_ASA_FILE));

        if(NULL == pAsaFile)
        {
		    pp_printf("++read asa file fail ++\r\n");		
            return false;
        }	
       pAsaFile->status = 0;
    }
    else
    {
        pAsaFile->status = 1;
    }
   
    return true;
}


static bool ReadAsa_getfilename_inter(unsigned char data[], unsigned long len)
{
    unsigned long table_page = 0;
    unsigned char name[PARTITION_NAME_LEN + 1] = {0};
    
	pp_printf("++read asa file++\r\n");		
	if(0 == pAsaFile->status)
    {	
		pp_printf("++read asa fil++: %s\r\n",data);		
        memcpy(pAsaFile->fileName, data, sizeof(pAsaFile->fileName));
        pp_printf("asafile name:%s\r\n", pAsaFile->fileName);

        //读取分区表
        table_page = get_partition_start_page(m_pSpiflash);
        if(table_page == 0)
        {
            pp_printf("ReadAsa_getfilename fail table_page == 0 \r\n");
            memset(data, 0, len);
            return true;
        }
        
        if(g_parttion_start_page != 0 && g_parttion_start_page != table_page)
        {
            //由于新的分区表位置与旧的分区表位置不一样，所以重设
            partition_set_partition_startpage(table_page);
        }

        memset(name, 0, PARTITION_NAME_LEN + 1);
        memcpy(name, pAsaFile->fileName, PARTITION_NAME_LEN);
        pp_printf("open name:%s\r\n", name);
        g_file_handle = partition_open(name);
        if(NULL == g_file_handle)
        {
            pp_printf("ReadAsa_getfilename partition_open fail \r\n");
            memset(data, 0, len);
            partition_close(g_file_handle);
            return true;
        }
        else
        {
            g_file_lenght = partition_get_data_size(g_file_handle);
        }

        //设回新的分区表位置
        if(g_parttion_start_page != 0 && g_parttion_start_page != table_page)
        {
            partition_set_partition_startpage(g_parttion_start_page);
        }
        
    }
    return true;
}

static bool ReadAsa_getfilename(unsigned long data, unsigned long len)
{
    return ReadAsa_getfilename_inter((unsigned char *)data, len);
}


static bool ReadAsa_senddata_inter(unsigned char data[], unsigned long len)
{
	pp_printf("++read asa file++ \r\n");		
	if(1 == pAsaFile->status)	
    {	
        if(partition_read(g_file_handle, data, len) == -1)
        {
            pp_printf("ReadAsa_senddata partition_read fail \r\n");
            memset(data, 0, len);
            partition_close(g_file_handle);
            return true;
        }
        else
        {
            
            if(g_file_lenght <= len)
            {
                if(partition_close(g_file_handle) == -1)
                {
                   pp_printf("ReadAsa_senddata partition_close fail \r\n");
                   memset(data, 0, len);
                   return true;
                }
            }
            g_file_lenght = g_file_lenght - len;
            pp_printf("g_file_lenght:%d, len:%d \r\n", g_file_lenght, len);
        }
    }

    return true;
}

static bool ReadAsa_senddata(unsigned long data, unsigned long len)
{
    return ReadAsa_senddata_inter((unsigned char *)data, len);
}


unsigned long Sflash_Write_Page(unsigned char *pData, unsigned long startPage, unsigned long PageCnt)
{
    unsigned long i; 
    unsigned long page_temp = 0;
    unsigned long BlockStart, BlockEnd;
    unsigned long block_64K_size = 65536;
    unsigned long block_64K_perpage = block_64K_size/m_pSpiflash->page_size;

    if(g_no_need_erase == 0)
    {
        if(startPage%block_64K_perpage == 0 && PageCnt == 256)
        {
            BlockStart = startPage/block_64K_perpage;
            pp_printf(" erase 64K block:%d\r\n", BlockStart);
            if (SF_SUCCESS != Fwl_SPIFlash_EraseBlock(m_pSpiflash, BlockStart, true))
            {
                pp_printf(" erase 64K fail at block:%d\r\n", BlockStart);
                return -1;
            }
        }
        else if(startPage%block_64K_perpage == 0 && PageCnt > 256)
        {
            BlockStart = startPage / block_64K_perpage;
            BlockEnd = (startPage + PageCnt) / block_64K_perpage;
            for (i=BlockStart; i<=BlockEnd; i++)
            {
                pp_printf("erase 64K:%d\r\n", i);
                if (SF_SUCCESS != Fwl_SPIFlash_EraseBlock(m_pSpiflash, i, true))
                {
                    pp_printf(" erase 64K fail at block:%d\r\n", i);
                    return -1;
                }
            }

            //不够64K的，按4K擦
            page_temp = (startPage + PageCnt) % block_64K_perpage;
            if(page_temp != 0)
            {
                BlockStart = (startPage + PageCnt - page_temp) / m_pSpiflash->PagesPerBlock;
                BlockEnd = (startPage + PageCnt) / m_pSpiflash->PagesPerBlock;
                for (i=BlockStart; i<=BlockEnd; i++)
                {

                    pp_printf("erase:%d\r\n", i);
                    if (SF_SUCCESS != Fwl_SPIFlash_EraseBlock(m_pSpiflash, i, false))
                    {
                        pp_printf(" erase fail at block:%d\r\n", i);
                        return -1;
                    }            
                }
            }
        }
        
        else if(startPage%block_64K_perpage != 0 && PageCnt > 256 
            && (PageCnt - startPage%block_64K_perpage) >= 256)
        {
            //先统计出64K对齐的块，并前面按4K擦
            page_temp = startPage%block_64K_perpage;
            BlockStart = startPage / m_pSpiflash->PagesPerBlock;
            BlockEnd = (startPage + page_temp) / m_pSpiflash->PagesPerBlock;
        
            for (i=BlockStart; i<=BlockEnd; i++)
            {
                pp_printf("erase:%d\r\n", i);
                if (SF_SUCCESS != Fwl_SPIFlash_EraseBlock(m_pSpiflash, i, false))
                {
                    pp_printf(" erase fail at block:%d\r\n", i);
                    return -1;
                }
            } 
            
            //再按64K擦
            BlockStart = (startPage+page_temp) / block_64K_perpage;
            BlockEnd = (startPage + PageCnt) / block_64K_perpage;
            for (i=BlockStart; i<=BlockEnd; i++)
            {
                pp_printf("erase 64K:%d\r\n", i);
                if (SF_SUCCESS != Fwl_SPIFlash_EraseBlock(m_pSpiflash, i, true))
                {
                    pp_printf(" erase 64K fail at block:%d\r\n", i);
                    return -1;
                }
            }
            
            //再按4K
            page_temp = (startPage + PageCnt) % block_64K_perpage;
            if(page_temp != 0)
            {
                BlockStart = (startPage + PageCnt - page_temp) / m_pSpiflash->PagesPerBlock;
                BlockEnd = (startPage + PageCnt) / m_pSpiflash->PagesPerBlock;
                for (i=BlockStart; i<=BlockEnd; i++)//分区都是按块对齐的
                {
                    pp_printf("erase:%d\r\n", i);
                    if (SF_SUCCESS != Fwl_SPIFlash_EraseBlock(m_pSpiflash, i, false))
                    {
                        pp_printf(" erase fail at block:%d\r\n", i);
                        return -1;
                    }
                }
            }
        }
        else
        {
            BlockStart = startPage / m_pSpiflash->PagesPerBlock;
            BlockEnd = (startPage + PageCnt) / m_pSpiflash->PagesPerBlock;
            for (i=BlockStart; i<=BlockEnd; i++)
            {
                pp_printf("erase:%d\r\n", i);
                if (SF_SUCCESS != Fwl_SPIFlash_EraseBlock(m_pSpiflash, i, false))
                {
                    pp_printf(" erase fail at block:%d\r\n", i);
                    return -1;
                }
            } 
        }
    }
    //pp_printf("Partition_Write_Page page start:%d, cnt:%d\r\n", startPage, PageCnt); 
    if(0 != Partition_Write_Page(startPage, PageCnt, pData))
    {
        pp_printf("Sflash_Write_Page write fail at page start:%d, cnt:%d\r\n", startPage, PageCnt);
        return -1;
    }


    return 0;
}




unsigned long Sflash_Write_Data(unsigned char *pData, unsigned long data_len, unsigned long StartPage, unsigned long page_cnt, bool check)
{
    unsigned long i;
    unsigned char *pBuf = NULL;
    
    if(0 != Sflash_Write_Page(pData, StartPage, page_cnt))
    {
        return -1;
    }
    else
    {    
        if (check)
        {
            //malloc buffer to read data
            pBuf = Fwl_Malloc(page_cnt * m_pSpiflash->page_size);

            if(NULL == pBuf)
            {
                return -1;
            }

            memset(pBuf, 0, page_cnt * m_pSpiflash->page_size);

            if(0 != Partition_Read_Page(StartPage, page_cnt, pBuf))
            {
                pp_printf("Sflash_Write_Data page start:%d, cnt:%d\r\n", StartPage, page_cnt);
                Fwl_Free(pBuf);
                return -1;
            }
   
            if(0 != memcmp(pData, pBuf, data_len))
            {
                pp_printf("Sflash_Write_Data compare fail\r\n");
                for (i=0; i<data_len; i++)
                {
                    if (pData[i] != pBuf[i])
                    {
                        pp_printf("St:%d, i:%d,S:%02x_D:%02x\r\n", StartPage, i, pData[i],pBuf[i]);
                        break;
                    }
                }
                
                pp_printf("\r\n");    
                
                Fwl_Free(pBuf);
                return -1;
            }

            Fwl_Free(pBuf);
        }
    }

    return 0;
}



bool Sflash_Write_Boot_Data(unsigned char *pData, unsigned long data_len)
{
    //unsigned long i;
    unsigned long page_cnt = 0, i = 0;
    unsigned long page_size = m_pSpiflash->page_size;
    unsigned char* pBuf = NULL;

    page_cnt = (data_len + page_size - 1) / page_size;

    pp_printf("start page:%d,  cnt:%d\r\n", g_StartPage, page_cnt);
    if (page_cnt > g_boot_page_num)
    {
        pp_printf("Sflash_Write_Boot_Data data_len is too large\r\n", page_cnt, g_boot_page_num);
        return false;
    }
    else if (page_cnt < g_boot_page_num)
    {
        if (data_len != page_size * page_cnt)
        {
            pp_printf("Sflash_Write_Boot_Data data_len is not page multiple\r\n");
            return false;
        }
    }
    else
    {
        if ((data_len % page_size) != (g_file_lenght % page_size))
        {
            pp_printf("Sflash_Write_Boot_Data data_len is error!\r\n");
            return false;
        }        
    }

    if(g_StartPage > (g_parttion_start_page - 1))
    {
        pp_printf("error: boot g_StartPage:%d  more than boot page :%d\r\n", g_StartPage, g_parttion_start_page - 1);
        return false;
    }

    //pp_printf("Partition_Write_Page page start:%d, cnt:%d\r\n", g_StartPage, page_cnt);
    if (0 != Sflash_Write_Data(pData, data_len, g_StartPage, page_cnt , true))
    {
        pp_printf("error: Sflash_Write_Data fail:%d\r\n", g_StartPage);
        return false;
    }

    g_StartPage += page_cnt;
    g_boot_page_num -= page_cnt;

     pp_printf("g_StartPage:%d,  g_boot_page_num:%d\r\n", g_StartPage, g_boot_page_num);
    
    return true;
}


bool Sflash_Read_Boot_Data(unsigned char *pData, unsigned long data_len)
{
    unsigned long page_cnt;
    unsigned long page_size = m_pSpiflash->page_size;
    unsigned char* pBuf = NULL;
    unsigned long i = 0;

    page_cnt = (data_len + page_size - 1) / page_size;

    pp_printf("g_StartPage:%d, boot page cnt:%d, page_cnt:%d\r\n", g_StartPage, g_parttion_start_page- 1, page_cnt);
    if(g_StartPage > (g_parttion_start_page - 1))
    {
        pp_printf("error:Sflash_Read_Boot_Data boot len more than the max\r\n");
        return false;
    }

    if(-1 == Partition_Read_Page(g_StartPage, page_cnt, pData))
    {
        pp_printf("Fwl_SPIFlash_ReadPage fail start:%d, cnt:%d\r\n", g_StartPage, page_cnt);
        return false;
    }

    g_StartPage += page_cnt;
    return true;
}

unsigned long Sflash_Read_spidata_Data(unsigned char *pData, unsigned long data_len)
{
    unsigned long page_cnt, i = 0;
    unsigned long page_size = m_pSpiflash->page_size;
    unsigned long* pStartPage;

    page_cnt = (data_len + page_size - 1) / page_size;

    pp_printf("start page:%d, page_cnt:%d, data_len:%d, g_file_lenght:%d\r\n", g_StartPage, page_cnt, data_len, g_file_lenght);
    if (page_cnt > g_file_lenght)
    {
        pp_printf("Sflash_Read_spidata_Data data_len is too large\r\n");
        return false;
    }
    else if (page_cnt < g_file_lenght)
    {
        if (data_len != page_size * page_cnt)
        {
            pp_printf("Sflash_Read_spidata_Data bin data_len is not page multiple\r\n");
            return false;
        }
    } 

    if(-1 == Partition_Read_Page(g_StartPage, page_cnt, pData))
    {
        pp_printf("Sflash_Read_spidata_Data fail start:%d, cnt:%d\r\n", g_StartPage, page_cnt);
        return false;
    }

    g_StartPage += page_cnt;
    g_file_lenght -= page_cnt;
    
    return true;
}


unsigned long Sflash_BurnWrite_AllData(unsigned char *pData, unsigned long data_len)
{
    unsigned long page_cnt;
    unsigned long page_size = m_pSpiflash->page_size;
    unsigned char* pBuf = NULL;

    page_cnt = (data_len + page_size - 1) / page_size;

    //pp_printf("g_StartPage:%d,  cnt:%d\r\n", g_StartPage, page_cnt);
    pp_printf(".");
    if (page_cnt > g_boot_page_num)
    {
        pp_printf("Sflash_BurnWrite_AllData bin data_len is too large\r\n");
        return false;
    }
    else if (page_cnt < g_boot_page_num)
    {
        if (data_len != page_size * page_cnt)
        {
            pp_printf("Sflash_BurnWrite_AllData bin data_len is not page multiple\r\n");
            return false;
        }
    }
    else
    {
        if ((data_len % page_size) != (g_file_lenght % page_size))
        {
            pp_printf("Sflash_BurnWrite_AllData bin data_len is error!\r\n");
            return false;
        }        
    }

    if((g_StartPage + page_cnt) > m_pSpiflash->total_page)
    {
        pp_printf("error: spidata page more than spi  page num\r\n");
        return false;
    }

    if (0 != Sflash_Write_Data(pData, data_len, g_StartPage, page_cnt, g_check))
    {
        pp_printf("Sflash_BurnWrite_AllData fail\r\n");
        return false;
    }

    g_StartPage += page_cnt;
    g_boot_page_num -= page_cnt;

    return true;
}

/*

//测试代码

unsigned long Test_Ex_Attr(void *file)
{
    T_EX_PARTITION_CONFIG ex_attr;
    T_BIN_CONFIG bin_config = {0};
    
    partition_get_attr(file, &ex_attr);

    memcpy(&bin_config, &ex_attr, sizeof(T_EX_PARTITION_CONFIG));
    
    pp_printf("file_length:%d\r\n", bin_config.file_length);
    pp_printf("check:%d\r\n", bin_config.check);
    pp_printf("backup_page:%d\r\n", bin_config.backup_pos);
    pp_printf("ld_addr:%d\r\n", bin_config.ld_addr);
    pp_printf("rev1:%02x\r\n", bin_config.rev1);
    pp_printf("rev2:%02x\r\n", bin_config.rev2);
    pp_printf("rev3:%02x\r\n", bin_config.rev3);

    
    bin_config.rev1 = 0xAA;
    bin_config.rev2 = 0xBB;
    bin_config.rev3 = 0xCC;
    memcpy(&ex_attr, &bin_config, sizeof(T_EX_PARTITION_CONFIG));
    
    partition_set_attr(file, &ex_attr);

    

    partition_get_attr(file, &ex_attr);

    memcpy(&bin_config, &ex_attr, sizeof(T_EX_PARTITION_CONFIG));
    

    pp_printf("file_length:%d\r\n", bin_config.file_length);
    pp_printf("check:%d\r\n", bin_config.check);
    pp_printf("backup_page:%d\r\n", bin_config.backup_pos);
    pp_printf("ld_addr:%d\r\n", bin_config.ld_addr);
    pp_printf("rev1:%02x\r\n", bin_config.rev1);
    pp_printf("rev2:%02x\r\n", bin_config.rev2);
    pp_printf("rev3:%02x\r\n", bin_config.rev3);
    
    return true;
}


unsigned long Test_change_partition(void)
{
    unsigned long i = 0;
    unsigned char buf[512] = {0};
    T_PARTITION_TABLE_CONFIG part_talbe;
    
    partition_get_partition_table(&part_talbe);
    
    pp_printf("part_talbe->pos:%d ", part_talbe.pos);
    for(i = 0; i < 512; i++)
    {
        if(i %16 == 0)
        {
            pp_printf("\r\n");
        }
         pp_printf("%02x ", part_talbe.table[i]);
    }
    //partition_delete("BIOS");    

    partition_resize("BIOS", 2048);  

    partition_get_partition_table(&part_talbe);
    pp_printf("01 part_talbe->pos:%d ", part_talbe.pos);
    for(i = 0; i < 512; i++)
    {
        if(i %16 == 0)
        {
            pp_printf("\r\n");
        }
        pp_printf("%02x ", part_talbe.table[i]);
    }
    
    return true;
}

*/









