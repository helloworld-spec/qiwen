/**
 * @file 
 * @brief:
 *
 * This file provides 
 * Copyright (C) 2017 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author 
 * @date 2017-2-09
 * @version 1.0
 */

#include "anyka_types.h"

#include "drv_api.h"



#include "dev_drv.h"
#include "device.h"
#include "platform_devices.h"


#ifdef AKOS
#include "akos_api.h"
#endif



#ifdef __cpluscplus
extern "C"{
#endif


#define UART_DATA_TOOL  (1024*8)

//access meida type
typedef enum {
    NO_MEDIA = 0 ,
    SDCARD_DISK,
    SDIO_DISK,
    UHOST_DISK,
    DUMMYDISK1
}T_ACCESS_MEDIUM;

T_pCARD_HANDLE pSD_CARD=NULL;


static T_DEV_INFO usbdisk_dev = {
	.dev_open_flg  = false,
	.dev_name      = DEV_USBDISK,	
};


typedef struct
{
	unsigned char   * pdatapool;             ///< buffer number
	
}T_USBDISK_INFO;

static T_USBDISK_INFO  m_usbdisk_para;


static bool SD_Read(unsigned char *buf,unsigned int BlkAddr, unsigned int BlkCnt, unsigned int NandAddInfo)
{
    sd_read_block(pSD_CARD,BlkAddr, buf, BlkCnt);
    return true;
}
static bool SD_Write(unsigned char *buf,unsigned int BlkAddr, unsigned int BlkCnt, unsigned int NandAddInfo)
{
    sd_write_block(pSD_CARD,BlkAddr, (const unsigned char*)buf, BlkCnt);
    return true;
}

static bool Fwl_Usb_MountLUN(T_ACCESS_MEDIUM disk_type)
{
    T_LUN_INFO add_lun;
    unsigned int i, LunAddInfo;
    char* pVendorStr = NULL ;
    char* pProductStr = NULL;
    char* pRevisionStr= NULL;
 
    /* index of this LUN(logic unit number),it is must defferent for every LUN */
    add_lun.LunIdx = (unsigned int)disk_type;
    memset(add_lun.Vendor,' ',INQUIRY_STR_VENDOR_SIZE);
    memset(add_lun.Product, ' ',INQUIRY_STR_PRODUCT_SIZE);
    memset(add_lun.Revision, ' ',INQUIRY_STR_REVISION_SIZE);

    /* disk accses information */
    switch (disk_type)
    {
    case  SDCARD_DISK:
        pSD_CARD = sd_initial(INTERFACE_SDMMC4,USE_ONE_BUS);
        if (pSD_CARD == NULL)
            return false;
        add_lun.Read = SD_Read;
        add_lun.Write = SD_Write;
        add_lun.FastBlkCnt = 1;//(512 byte is best for Nandflash accessing)
        add_lun.sense = MEDIUM_NOSENSE;
        pVendorStr = "anyka";
        pProductStr = "sd disk";
        pRevisionStr = "1.00";

        memcpy(add_lun.Vendor, pVendorStr, strlen(pVendorStr)<INQUIRY_STR_VENDOR_SIZE ? strlen(pVendorStr) : INQUIRY_STR_VENDOR_SIZE);
        memcpy(add_lun.Product, pProductStr, strlen(pProductStr)<INQUIRY_STR_PRODUCT_SIZE ? strlen(pProductStr) : INQUIRY_STR_PRODUCT_SIZE);
        memcpy(add_lun.Revision, pRevisionStr,strlen(pRevisionStr)<INQUIRY_STR_REVISION_SIZE ? strlen(pRevisionStr) : INQUIRY_STR_REVISION_SIZE);
        
        add_lun.LunAddInfo = 0;
        //medium = (T_PMEDIUM)(pPartiton_Group[LunAddInfo]);
        sd_get_info(pSD_CARD, &add_lun.BlkCnt,&add_lun.BlkSize);
        printf("mount sd\n");
        usbdisk_addLUN(&add_lun);        
        break;

   
    default:
        printf("unknown medium type!");
        return false;
        break;
    }
    return true;
}


static int usbdisk_dev_open(int dev_id, void *data)
{
	T_USBDISK_INFO *usbdisk_info;
	int ret = -1;
    unsigned int mode = 0;
    char  manu[10]="anyka";
    char  prod[10]="usbdisk";
    unsigned char disk[8],i,disknum=0;
    unsigned char usbdisk_state;

    unsigned int tmp = 2;
    
    mode = USB_MODE_20;
	
    usbdisk_init(mode);
    usbdisk_set_str_desc(STR_MANUFACTURER,manu);    
    usbdisk_set_str_desc(STR_PRODUCT,prod);
	
	Fwl_Usb_MountLUN(SDCARD_DISK);

    //start udisk
    if(!usbdisk_start())
    {
        goto exit;
    }

   //udisk loop
    while(1)
    {
        usbdisk_proc(); //for non-AKOS polling
        usbdisk_state = usb_slave_getstate();
        //USB_START_STOP:收到STOP UNIT 命令,macos在弹出盘时会发送此命令
        if(USB_SUSPEND == usbdisk_state)
        {
            usb_slave_set_state(USB_NOTUSE);
            akprintf(C3, M_DRVSYS, "out!\n");
            break;
        }
        if(USB_START_STOP == usbdisk_state)
        {
            akprintf(C3, M_DRVSYS, "out!\n");
            break;
        }

        //USB_TEST_UNIT_STOP:3秒内没收到SCSI命令
        //CheckCount:在出现3秒内没收到SCSI命令的情况下,用于确认是否为win98或macos
        if(USB_TEST_UNIT_STOP == udisk_get_testunit_state())
        {
            usb_slave_set_state(USB_NOTUSE);
            akprintf(C3, M_DRVSYS, "WIN7 out!\n");
            break;
        }
		AK_Sleep(10);
		
    }
  	
	ret = dev_id;
exit:	
	return ret;
}


static int usbdisk_dev_close(int dev_id)
{

	int usbdisk_num;
	T_USBDISK_INFO *usbdisk_info;

	usbdisk_stop();
	
	if(NULL != pSD_CARD)
	{
		sd_free(pSD_CARD);
		pSD_CARD = NULL;
	}
	

	return 0;

}

static int usbdisk_dev_read(int dev_id, void *data, unsigned int len)
{
	//TBD
#if 0
	int ret ;

	
	int usbdisk_num;
	T_USBDISK_INFO *usbdisk_info;
	
	usbdisk_info = (T_USBDISK_INFO *)usbdisk_dev.dev_data;
	usbdisk_num   = usbdisk_info->usbdisk_id;

    ret = usbdisk_read(usbdisk_num, data, len);
	
	return ret;
#endif
}


static int usbdisk_dev_write(int dev_id, const void *data, unsigned int len)
{
	//TBD
#if 0
	unsigned long ret;
	int usbdisk_num;
	T_USBDISK_INFO *usbdisk_info;
	
	usbdisk_info = (T_USBDISK_INFO *)usbdisk_dev.dev_data;
	usbdisk_num   = usbdisk_info->usbdisk_id;

	ret = usbdisk_write(usbdisk_num, data,len);

	return (int)ret;
#endif	
}

static int usbdisk_dev_ioctl(int dev_id, unsigned long cmd, void *data)
{
	//TBD
#if 0	
	unsigned long ret;
	int usbdisk_num;
	T_USBDISK_INFO *usbdisk_info;
	int tmp = *(int *)data;
	bool parity_enable;
	bool parity_odd_even;
	
	usbdisk_info = (T_USBDISK_INFO *)usbdisk_dev.dev_data;
	usbdisk_num   = usbdisk_info->usbdisk_id;
	usbdisk_info->baudrate = tmp;
	switch(cmd)
	{
		case IO_UART_BAUD_RATE:
			
			//usbdisk_setbaudrate(usbdisk_num,tmp);
			
			usbdisk_dev_close(usbdisk_num);
			usbdisk_dev_open(usbdisk_num, &tmp);
			break;
			
		case IO_UART_PARITY:
			#if 1
			printk( "nonsupport usbdisk parity!\r\n");
			#else
			parity_enable   = (tmp >> 7)&0x1;
			parity_odd_even = tmp&0x1;
		
			usbdisk_setdataparity(usbdisk_num,parity_enable,parity_odd_even);
			#endif
			break;
		default :
			printk( "usbdisk_dev_ioctl command error!\r\n");
			return -1;

	}
#endif
	return 0;
	
}



static T_DEV_DRV_HANDLER usbdisk_function_handler = 
{

    .drv_open_func  = usbdisk_dev_open,
    .drv_close_func = usbdisk_dev_close,
    .drv_read_func  = usbdisk_dev_read,
    .drv_write_func = usbdisk_dev_write,
    .drv_ioctl_func = usbdisk_dev_ioctl,
};


static int usbdisk_device_reg(void)
{
	void *devcie = NULL;
	#if 0
	devcie  = platform_get_devices_info(usbdisk_dev.dev_name);
	if(NULL == devcie)
	{
		printk("usbdisk devcie register fail!");
		return 0;
	}
	#endif
	if (0 !=  dev_alloc_id(&usbdisk_dev,0, usbdisk_dev.dev_name))
	{
		printk("usbdisk devcie alloc id fail!");
		return 0;
	}

	usbdisk_dev.dev_data               = devcie;
	usbdisk_dev.drv_handler            = &usbdisk_function_handler;
	usbdisk_function_handler.devcie    = &usbdisk_dev;
		
	usbdisk_function_handler.device_id = usbdisk_dev.dev_id;
	
	dev_drv_reg(usbdisk_dev.dev_id, &usbdisk_dev);
	
	return 0;
	
}

dev_module_init(usbdisk_device_reg)//要在set_platform_devices_info之后,串口正常工作

#ifdef __cplusplus
}
#endif

