#include "drv_api.h"
#include "fs.h"
#include "print.h"

typedef struct
{
    T_pCARD_HANDLE   pHandle;
	unsigned char             driverCnt;
	unsigned char             firstDrvNo;
    bool           bIsMount;
}T_SD_PARM,*T_pSD_PARM;

static T_SD_PARM   gSdParam={NULL,0,0,false};


bool sd_read_block(T_pCARD_HANDLE handle,unsigned long block_src, unsigned char *databuf, unsigned long block_count);

static unsigned long sd_read(T_PMEDIUM medium, unsigned char *buf,unsigned long BlkAddr, unsigned long BlkCnt)
{
    unsigned long ret = 0;

	if (sd_read_block(gSdParam.pHandle, BlkAddr, buf, BlkCnt) )
		ret = BlkCnt;
		
	return ret;
}


static unsigned long sd_write(T_PMEDIUM medium, const unsigned char* buf, unsigned long BlkAddr, unsigned long BlkCnt)
{
	unsigned long ret;
	
    if (sd_write_block(gSdParam.pHandle, BlkAddr, buf, BlkCnt))
    {
        ret = BlkCnt;
    }

    return ret;
}

bool mount_sd()
{
    DRIVER_INFO driver;
    unsigned long capacity = 0;
    unsigned long BytsPerSec = 0;
    unsigned char firstNo = 0;
    unsigned char drvCnt = 0;    
    bool ret = false;

    if (gSdParam.bIsMount)
    {
        printk("sd have already mounted!\n");
        return true;
    }
    
	gSdParam.pHandle = sd_initial(INTERFACE_SDMMC4 , USE_ONE_BUS);

	if (gSdParam.pHandle ==NULL)
		return false;

    
	sd_get_info(gSdParam.pHandle , &capacity, &BytsPerSec);

    if (capacity !=0)
    {
        driver.nBlkCnt     = capacity;
        driver.nBlkSize  = BytsPerSec;
        driver.nMainType = MEDIUM_SD;
        driver.nSubType  = USER_PARTITION;
        driver.fRead     = sd_read;
        driver.fWrite     = sd_write;
         firstNo = FS_MountMemDev(&driver, &drvCnt, (unsigned char)-1);
        mini_delay(20);
    }
    
    printk("[MntSd]:FirstDrvNo = %d Count = %d\n",firstNo,drvCnt);
    if (0XFF == firstNo)
    {        
        printk("sd mount faile\n");
        sd_free(gSdParam.pHandle);
        return false;
    }

    gSdParam.bIsMount =true;
    gSdParam.firstDrvNo = firstNo;
    gSdParam.driverCnt  = drvCnt;
    return true;
}

bool unmount_sd()
{
    unsigned long i;
    bool mntRet = false;


    if (!gSdParam.bIsMount)
    {
		printf("sd don't mount , need't unmount\n");
        return true;
    }
    
    for (i = 0; i<gSdParam.driverCnt; i++)
    {
        mntRet = FS_UnMountMemDev(i + gSdParam.firstDrvNo);
        printf("SD UnMount:driver ID:%d,UnMount State:%d\n" \
                                    ,i + gSdParam.firstDrvNo,mntRet);
    }

    
    sd_free(gSdParam.pHandle);
    
    memset(&gSdParam ,0,sizeof(gSdParam));


    printk("sd unmount finish\n");
    gSdParam.bIsMount =false;
    
    return true;
}




