// SpiImageCreate.cpp: implementation of the SpiImageCreate class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "burntool.h"
#include "burn.h"
#include "SpiImageCreate.h"
#include "Config.h"
#include "stdio.h"
extern "C"
{
	#include "partition_lib.h"
	#include "partition_init.h"
};


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#define RAM_REG_MAX_NUM         100      //
#define BURN_OBJECT_MAX_NUM     32       //


extern UINT g_download_mtd_count;// 下载镜像的文件个数
extern T_DOWNLOAD_MTD g_download_mtd[MAX_DOWNLOAD_MTD];

typedef struct 
{
    T_RAM_REG ramReg[RAM_REG_MAX_NUM];
    UINT      numReg;
}T_RAM_REG_PARA;

typedef struct
{
    HANDLE hBurnThread;//hBurnThread
    HANDLE hEvent;//hEvent
    HANDLE hUSBDevice;//hUSBDevice
	HANDLE t_usb_init_event;//t_usb_init_event
    UINT   nID;
    UINT   status;
	UINT   download_length;//download_length
    TCHAR  strUSBName[MAX_PATH+1];
    UINT   NandChipCnt;
    UINT   AttachUSBCnt;
	TCHAR  strHubport[MAX_PATH+1];//用于保存初始化的通道HUB port ID
	union
	{
		T_NAND_PHY_INFO NandPhyInfo;//NandPhyInfo
		T_SFLASH_PHY_INFO SpiphyInfo;//SpiphyInfo
	};
    
}T_BURN_OBJECT;

T_SFLASH_PHY_INFO SpiInfo;  //spi参数
HANDLE Spi_hHandle = NULL;//Spi_hHandle

extern CConfig theConfig;//config
extern CBurnToolApp theApp;//theApp

static T_BURN_OBJECT g_pBurnObject[BURN_OBJECT_MAX_NUM] = {0};

T_pVOID	Spi_Malloc(T_U32 size);
T_pVOID Spi_Free(T_pVOID var);
T_pVOID Spi_Memcpy(T_pVOID dst, T_pCVOID src, T_U32 count);
T_pVOID Spi_MemSet(T_pVOID buf, T_S32 value, T_U32 count);
T_S32 Spi_MemCmp(T_pCVOID buf1, T_pCVOID buf2, T_U32 count);
T_pVOID Spi_MemMov(T_pVOID dst, T_pCVOID src, T_U32 count);
T_S32 Spi_Printf(T_pCSTR s, ...);
void config_spiboot_aspen(BYTE *sflashboot, DWORD dwSize, UINT clock);
void config_spiboot_snowbird(BYTE *sflashboot, UINT chip_type, DWORD dwSize, UINT clock);
void ConfigSpibootParam(UINT nID, BYTE *buf);
T_pVOID fhalib_init(T_U32 eMedium, T_U32 eMode);
T_BOOL spi_flash_erase(T_U32 sector);//
T_BOOL spi_flash_write(T_U32 page, const T_U8 *buf);
T_BOOL spi_flash_read(T_U32 page, T_U8 *buf, T_U32 page_cnt);
int FHA_Spi_Erase(T_U32 nChip,  T_U32 nPage);//
int FHA_Spi_Read(T_U32 nChip,  T_U32 nPage, T_U8 *pData, T_U32 nDataLen,  T_U8 *pOob, T_U32 nOobLen , T_U32 eDataType);
int FHA_Spi_Write(T_U32 nChip, T_U32 nPage, const T_U8 *pData, T_U32 nDataLen,  T_U8 *pOob, T_U32 nOobLen, T_U32 eDataType);
BOOL  FHA_download_bin(T_VOID);//
BOOL  FHA_download_boot(T_VOID);//

SpiImageCreate::SpiImageCreate()
{

}

SpiImageCreate::~SpiImageCreate()
{

}

UINT Get_Filelenght(TCHAR *pstrPath)
{
	UINT lenght = 0;
	HANDLE hFile  = INVALID_HANDLE_VALUE;
	
	hFile = CreateFile(pstrPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if(INVALID_HANDLE_VALUE == hFile)
    {
        return AK_FALSE;
    }
	
    lenght = GetFileSize(hFile, NULL);
	if(0xFFFFFFFF == lenght)
	{
		return 0;
	}
	CloseHandle(hFile);
	
	return lenght;
}

T_U8 Burn_Set_Boot_block_num(TCHAR *pstrPath, T_SFLASH_PHY_INFO *sFlashPhyInfo)
{
	UINT dwSize = 0;
	UINT boot_blcok_size = 0;
	UINT dwSize_temp = 0;
	
	dwSize = Get_Filelenght(pstrPath);
	if (dwSize == 0)
	{
		return 0;
	}
	
	
	//页对齐(文件大小+文件大小10%)
	dwSize_temp = (dwSize/10 + sFlashPhyInfo->page_size - 1)/sFlashPhyInfo->page_size;
	boot_blcok_size = (dwSize + sFlashPhyInfo->page_size - 1)/sFlashPhyInfo->page_size + dwSize_temp;
	
	//块对齐
	boot_blcok_size = (boot_blcok_size + sFlashPhyInfo->erase_size/sFlashPhyInfo->page_size - 1)/(sFlashPhyInfo->erase_size/sFlashPhyInfo->page_size);
	return boot_blcok_size;
}

//媒介初始化
T_pVOID fhalib_init(T_U32 eMedium, T_U32 eMode)
{
	T_FHA_LIB_CALLBACK  pCB;
	T_SPI_INIT_INFO     spi_fha; 
	T_pVOID             pInfo = AK_NULL;
	T_U8 partition_block = 0;

	if (TRANSC_MEDIUM_SPIFLASH == eMedium)
    {
        //for spi init
        pCB.Erase  = FHA_Spi_Erase;//Erase
        pCB.Write  = FHA_Spi_Write;//Write
        pCB.Read   = FHA_Spi_Read;//Read
        pCB.ReadNandBytes = (FHA_ReadNandBytes)AK_NULL;

		spi_fha.total_size = SpiInfo.total_size;
        spi_fha.page_size = SpiInfo.page_size;//page_size
        spi_fha.pages_per_block = SpiInfo.erase_size / SpiInfo.page_size;//PagesPerBlock
		partition_block = Burn_Set_Boot_block_num(theApp.ConvertAbsolutePath(theConfig.path_nandboot), &SpiInfo);
		SpiInfo.reserved2 = partition_block;//记录块
		//spi_fha.partition_table_start = partition_block*spi_fha.PagesPerBlock; /*partition table start addr*/
        pInfo    = (T_pVOID)(&spi_fha);
	}

    pCB.RamAlloc = Spi_Malloc; //RamAlloc
    pCB.RamFree  = Spi_Free;//RamFree
    pCB.MemSet   = Spi_MemSet;//MemSet
    pCB.MemCpy   = Spi_Memcpy;//MemCpy
    pCB.MemCmp   = Spi_MemCmp;//MemCmp
    pCB.MemMov   = Spi_MemMov;//MemMov
    pCB.Printf   = Spi_Printf;//Printf

    if (0 == partition_init(&pCB, pInfo, partition_block*spi_fha.pages_per_block))//FHA_burn_init
    {
        return pInfo;
    }
    else
    {
    	printf("Partition_Init fail\n");
        return AK_NULL;
    }    
}
//调用spi的低层擦
int FHA_Spi_Erase(T_U32 nChip,  T_U32 block)
{
	if (!spi_flash_erase(block))//spi_flash_erase
	{
		printf("fw:erase fail\n");
		return -1;
	}

	printf("+fw:erase :%d \r\n", block);

	return 0;
}
//调用spi的低层读
int FHA_Spi_Read(T_U32 nChip,  T_U32 nPage, T_U8 *pData, T_U32 nDataLen,  T_U8 *pOob, T_U32 nOobLen , T_U32 eDataType)
{ 
	T_U32 i = 0;

	for (i = nPage; i < nDataLen+nPage; i++)
	{
		if (!spi_flash_read(i, &pData[(i-nPage) * SpiInfo.page_size], 1))//
		{
			return -1;
		}
	}
    
	return 0;
}
//调用spi的低层写
int FHA_Spi_Write(T_U32 nChip, T_U32 nPage, const T_U8 *pData, T_U32 nDataLen,  T_U8 *pOob, T_U32 nOobLen, T_U32 eDataType)
{	
	T_U32 i = 0;

	for (i = nPage; i < nDataLen + nPage; i++)
	{
		if (!spi_flash_write(i, &pData[(i-nPage) * SpiInfo.page_size]))//
		{
			return -1;
		} 
	}
    
	return 0;
}


BOOL download_data(char *data, UINT data_len,  char *file_name)
{
	void *file_handle = NULL;

	if(NULL == data || data_len == 0 || file_name == NULL)
	{
		AfxMessageBox(_T("data == null or data_len == 0 or file_name == null"), MB_OK);
		return FALSE;
	}
	
	file_handle = partition_open((unsigned char *)file_name);
	if(file_handle == NULL)
	{
		AfxMessageBox(_T("Partition_Open fail"), MB_OK);
		return FALSE;
	}
	
	if(partition_write(file_handle, (unsigned char *)data, data_len) == 0)
    {
		AfxMessageBox(_T("Partition_Write fail"), MB_OK);
		partition_close(file_handle);
        return FALSE;
    }

	if(partition_close(file_handle) != 0)
	{
		AfxMessageBox(_T("Partition_Close fail"), MB_OK);
		return FALSE;
	}
	return TRUE;
}


BOOL download_file(TCHAR *file_path, char *file_name)
{
	void *file_handle = 0;
	UINT file_len = 0;
	char *buf = NULL;
	UINT read_len = 4096;
	UINT dwSize = 0;
	DWORD count = 0;

	buf = (char *)malloc(read_len*sizeof(char));
	if (NULL == buf)
	{
		AfxMessageBox(_T("buf malloc fail"), MB_OK);
		return FALSE;
	}

	HANDLE hFile = CreateFile((const unsigned short *)file_path, 
        GENERIC_READ, 
        FILE_SHARE_READ, 
        NULL, 
        OPEN_EXISTING, 
        FILE_ATTRIBUTE_NORMAL, 
        NULL);//创建
	
	if(INVALID_HANDLE_VALUE == hFile)//INVALID_HANDLE_VALUE
	{
		AfxMessageBox(_T("creat file fail"), MB_OK);
		free(buf);//
		return FALSE;
	}
	
	//获取bin文件大小
	dwSize = GetFileSize(hFile, NULL);//GetFileSize
	
	if(0xFFFFFFFF == dwSize || dwSize == 0)
	{
		AfxMessageBox(_T("get file lenght error"), MB_OK);
		CloseHandle(hFile);//关闭
		free(buf);
		return FALSE;
	}
	
	file_handle = partition_open((unsigned char *)file_name);
	if(file_handle == NULL)
	{
		AfxMessageBox(_T("Partition_Open fail"), MB_OK);
		partition_close(file_handle);
		CloseHandle(hFile);//关闭
		free(buf);
		return FALSE;
	}

	file_len = dwSize;
	//每次读4096 byte 进buf 并写bin
	while(1)//ReadFile
	{
		memset(buf, 0, read_len);
		ReadFile(hFile, buf, read_len, &count, NULL);
		if(0 == count)
		{
			break;
		}

		if(partition_write(file_handle, (unsigned char *)buf, count) == 0)
        {
			AfxMessageBox(_T("Partition_Write fail"), MB_OK);
			partition_close(file_handle);
			CloseHandle(hFile);//关闭
			free(buf);
            return FALSE;
        }
        else
        {
            file_len -= count;
            if(file_len == 0)
            {
                break;
            }
        }
	}

	if(partition_close(file_handle) != 0)
	{
		AfxMessageBox(_T("Partition_Close fail"), MB_OK);
		CloseHandle(hFile);//关闭
		free(buf);
		return FALSE;
	}

	CloseHandle(hFile);//CloseHandle
	free(buf);
	return TRUE;
}

    
void config_spiboot_aspen(BYTE *sflashboot, DWORD dwSize, UINT clock)
{
	T_RAM_REG_PARA g_RamReg = {NULL, 0};
	UINT i;
	UINT Asciclock = 60000000; //初始频率60M
	BYTE clock_temp = 0; //临时变量


	if(NULL == sflashboot)
	{
		return;
	}

	*(BYTE *)(sflashboot + 0x4) = 'A';
	*(BYTE *)(sflashboot + 0x5) = 'K';
	*(BYTE *)(sflashboot + 0x6) = 'S';
	*(BYTE *)(sflashboot + 0x7) = 'H';
	*(BYTE *)(sflashboot + 0x8) = '_';
	*(BYTE *)(sflashboot + 0x9) = '2';
	*(BYTE *)(sflashboot + 0xA) = '4';
	*(BYTE *)(sflashboot + 0xB) = '0';

	*(DWORD *)(sflashboot + 0xC) = dwSize;  //data size


	g_RamReg.numReg = config_ram_param(g_RamReg.ramReg);

	if (Asciclock/clock >= 2)
	{
		clock_temp = (Asciclock/clock)/2-1;  //根据烧录工具的传进来的参数进行决定
	}
	*(WORD *)(sflashboot + 0x10)  = clock_temp; //spi参数的clock
	
	*(WORD *)(sflashboot + 0x18)  = 0; //read speed mode
	
	*(WORD *)(sflashboot + 0x1C)  = 0x00000006; //read speed mode
	
	*(DWORD *)(sflashboot + 0x20) = 0x80E00000;	//outside ram address
	
	*(DWORD *)(sflashboot + 0x24) = 0;  //0-> CPU , 1->DMA
	

	g_RamReg.numReg = config_ram_param(g_RamReg.ramReg);
	for(i = 0; i < g_RamReg.numReg; i++)
	{
		*(DWORD *)(sflashboot + 0x2C + 8*i) = g_RamReg.ramReg[i].reg_addr;      //地址
		*(DWORD *)(sflashboot + 0x2C + 8*i + 4) = g_RamReg.ramReg[i].reg_value;  //值
	}
}


//修改boot分区前512字节的内容中的密码
void config_spiboot_snowbird(BYTE *sflashboot, UINT chip_type, DWORD dwSize, UINT clock)
{
	UINT Asciclock = 60000000; //初始频率60M
	BYTE clock_temp = 0; //临时变量

	if(NULL == sflashboot)
	{
		return;
	}

	*(BYTE *)(sflashboot + 0x25) = 'N';//
	
#if 0
	if ((CHIP_11XX == chip_type) || (CHIP_1080L == chip_type)  || (CHIP_10XXC == chip_type))//nn2
	{
		*(BYTE *)(sflashboot + 0x26) = 'N';//
	}
	else
	{
		*(BYTE *)(sflashboot + 0x26) = 'B';//
	}
	
	if (CHIP_10X6 == chip_type)//nb1
	{
		*(BYTE *)(sflashboot + 0x27) = '1';//
	}
	else if (CHIP_1080L == chip_type)//nnL
	{
		*(BYTE *)(sflashboot + 0x27) = 'L';
	}
	else if (CHIP_10XXC == chip_type)//nnc
	{
		*(BYTE *)(sflashboot + 0x27) = 'C';
	}
	else
	{
		*(BYTE *)(sflashboot + 0x27) = '2';//
	}
#endif
	*(DWORD *)(sflashboot + 0x28) = dwSize;  //data size

	if (Asciclock/clock >= 2)
	{
		clock_temp = (Asciclock/clock)/2-1;  //根据烧录工具的传进来的参数进行决定
	}
	*(WORD *)(sflashboot + 0x2C)  = clock_temp; //spi参数的clock
#if 0
	if(CHIP_1080L == chip_type || CHIP_10XXC == chip_type)
	{
		*(DWORD *)(sflashboot + 0x30) = 0x800000; //10L
	}
#endif
}
//修spi 的参数
void ConfigSpibootParam(UINT nID, BYTE *buf)
{
	UINT offset = 0;

	for (UINT i = 0; i < 600; i++)
	{
		if (('S' == buf[i]) && ('P' == buf[i+1]) && ('I' == buf[i+2]) && ('P' == buf[i+3]))
		{
			offset = i;//
			break;//
		}
	}

	if (i < 600)
	{
		memcpy(buf + offset + 4, &SpiInfo, sizeof(T_SFLASH_PHY_INFO));
	}
}
//实现spi的低层擦
T_BOOL spi_flash_erase(T_U32 sector)
{
	T_U32  pos = 0;
	T_U32 dwWriteSize = 0;
	T_U32 i = 0;
	T_S8  *ptmp = (signed char *)AK_NULL;
	HANDLE filehandle = Spi_hHandle;
	T_U32 high_pos = 0;
	T_U32 *pSpare = (unsigned long *)AK_NULL;
    T_U32  NotSpareSize = SpiInfo.page_size;//NotSpareSize
    T_U32  dwptr;
    T_U32 tmptestbyte = SpiInfo.erase_size;//tmptestbyte

	pos = sector * SpiInfo.erase_size;

    dwptr = SetFilePointer( filehandle, pos , (long *)&high_pos, FILE_BEGIN);//SetFilePointer
    if ( T_U32_MAX == dwptr && (GetLastError() != NO_ERROR) )
    {
        printf("spi_flash_erase: SetFilePointer faild  !\r\n");
		return AK_FALSE;
    }
  
     
	ptmp = (T_S8 *)Spi_Malloc(SpiInfo.page_size);
	if(ptmp == AK_NULL)
	{
		printf("spi_flash_erase: ptmp malloc  size faild !\r\n");
		return AK_FALSE;
  	}
 
	memset(ptmp, 0xff, SpiInfo.page_size);
 
	for(i = 0; i < SpiInfo.erase_size/SpiInfo.page_size; i++)//循环写
	{
	
		if(WriteFile(filehandle, ptmp, NotSpareSize, &dwWriteSize, NULL) == 0)//WriteFile
        {	
            DWORD result;
		    result = GetLastError();
            printf("erase err:%d\r\n",result);
			return AK_FALSE;
		}
		else if(dwWriteSize!=NotSpareSize )//写长度不对
		{
			printf("spi_flash_erase: WriteFile faild  !\r\n");
			return AK_FALSE;
		}
	
	}
	
	Spi_Free(ptmp);

	return AK_TRUE;
}
//实现spi的低层写
T_BOOL spi_flash_write(T_U32 page, const T_U8 *buf)
{
	T_U32 blockId = page / (SpiInfo.erase_size/SpiInfo.page_size);//blockId
	T_U32 dwWriteSize = 0;
	HANDLE filehandle = Spi_hHandle;//filehandle
	T_U32  low_pos = page*SpiInfo.page_size;//low_pos
	T_U32  NotSpareSize = SpiInfo.page_size;//NotSpareSize
	T_U32  high_pos = 0;
    T_U32  dwptr;
    T_U32 tmptestbyte = page*SpiInfo.page_size;//tmptestbyte


	if(filehandle==INVALID_HANDLE_VALUE)
	{
		printf("spi_flash_write : filehandle==INVALID_HANDLE_VALUE\r\n");
		return AK_FALSE;
	}
	
    dwptr = SetFilePointer( filehandle, low_pos , (long *)&high_pos, FILE_BEGIN);//SetFilePointer
    if ( T_U32_MAX == dwptr && (GetLastError() != NO_ERROR) )
    {
        printf("spi_flash_write  : SetFilePointer faild\r\n");
		return AK_FALSE;
    }
	
	if(WriteFile(filehandle, buf, NotSpareSize, &dwWriteSize, NULL)==0)//WriteFile
	{
		DWORD result;
		result = GetLastError();
		printf("spi_flash_write : WriteFile NFC_FIFO_SIZE faild,err:%d\r\n", result);
		return AK_FALSE;
	}
	
	if(dwWriteSize != NotSpareSize)
	{
		printf("spi_flash_write : dwReadSize!=NFC_FIFO_SIZE\r\n");
		return AK_FALSE;
	}
	
	return AK_TRUE;

}
//实现spi的低层读
T_BOOL spi_flash_read(T_U32 page, T_U8 *buf, T_U32 page_cnt)
{
	T_U32 blockId = page /(SpiInfo.erase_size/SpiInfo.page_size);//blockId
	HANDLE filehandle = Spi_hHandle;
	T_U32  low_pos = page*SpiInfo.page_size;//low_pos
	T_U32  dwReadSize = 0;
	T_U32  NotspareSize = page_cnt*SpiInfo.page_size;//NotspareSize
	T_U32  high_pos=0, dwptr;
    T_U32 tmptestbyte = page*SpiInfo.page_size;//tmptestbyte
		
	if(filehandle==INVALID_HANDLE_VALUE)
	{
		printf("nand_readsector_large : filehandle==INVALID_HANDLE_VALUE\r\n");
		return AK_FALSE;
	}
	
    dwptr = SetFilePointer( filehandle, low_pos , (long *)&high_pos, FILE_BEGIN);//SetFilePointer
    if ( T_U32_MAX == dwptr && (GetLastError() != NO_ERROR) )
    {
        printf("nand_readsector_large: SetFilePointer faild  !\r\n");
		return AK_FALSE;
    }
	
	
	if(ReadFile(filehandle,buf, NotspareSize, &dwReadSize, NULL)==0)//读
	{
        DWORD result;
		result = GetLastError();
		printf("nand_readsector_large : WriteFile NFC_FIFO_SIZE faild,err:%d\r\n",result);
		return AK_FALSE;
	}
	
	if(dwReadSize!= NotspareSize)
	{
		printf("nand_readsector_large : dwReadSize!=NFC_FIFO_SIZE\r\n");
		return AK_FALSE;
	}
		
	return AK_TRUE;
	
}

T_pVOID	Spi_Malloc(T_U32 size)
{
	T_pVOID ptr = NULL;

	ptr =  (T_pVOID)malloc(size);

	memset(ptr, 0, size);//

	return ptr;
}

T_pVOID Spi_Free(T_pVOID var)
{
	free(var);//

	return AK_NULL;
}

T_pVOID Spi_Memcpy(T_pVOID dst, T_pCVOID src, T_U32 count)
{
	PBYTE pDst = (PBYTE)dst;
	PBYTE pSrc = (PBYTE)src;
	UINT i;

	if(NULL == dst || NULL == src)
	{
		return NULL;
	}

	for(i = 0; i < count; i++)
	{
		pDst[i] = pSrc[i];//
	}

	return dst;
}

T_pVOID Spi_MemSet(T_pVOID buf, T_S32 value, T_U32 count)
{
	if(NULL == buf)
	{
		return NULL;
	}

	UINT i;
	PBYTE pBuf = (PBYTE)buf;

	for(i = 0; i < count; i++)
	{
		pBuf[i] = (BYTE)value;//
	}

	return buf;
}

T_S32 Spi_MemCmp(T_pCVOID buf1, T_pCVOID buf2, T_U32 count)
{
	return memcmp(buf1, buf2, count);
}

T_pVOID Spi_MemMov(T_pVOID dst, T_pCVOID src, T_U32 count)
{
	return memmove(dst, src, count);
}

T_S32 Spi_Printf(T_pCSTR s, ...)
{
	return 0;
}


BOOL spi_check_fs_type(TCHAR *name, char *fs_type)
{	
	UINT len_src = 0;
	UINT len_dst = 0;
	UINT i = 0;
	char path_temp[MAX_PATH] = {0};
	
	USES_CONVERSION;
	
	for (i = 0; i < g_download_mtd_count; i++)
	{
		len_src = _tcslen(g_download_mtd[i].disk_name);
		len_dst = _tcslen(name);
		if (len_src == len_dst)
		{
			if (_tcsncmp(g_download_mtd[i].disk_name, name, len_dst) == 0)
			{
				break;
			}
		}
	}
	
	if (i == g_download_mtd_count)
	{
		return FALSE;
	}
	
	
	len_dst = _tcslen(g_download_mtd[i].pc_path);
	memcpy(path_temp, T2A(g_download_mtd[i].pc_path), len_dst);
	len_dst = strlen(path_temp);
	if (path_temp[len_dst - 5] == 's' && path_temp[len_dst - 4] == 'q'
		&& path_temp[len_dst -3] == 's' && path_temp[len_dst - 2] == 'h'
		&& path_temp[len_dst - 1] == '4' )
	{
		*fs_type = FS_SQSH4;
	}
	else if (path_temp[len_dst - 5] == 'j' && path_temp[len_dst - 4] == 'f'
		&& path_temp[len_dst - 3] == 'f' && path_temp[len_dst - 2] == 's'
		&& path_temp[len_dst - 1] == '2' )
	{
		*fs_type = FS_JFFS2;
	}
	else if (path_temp[len_dst - 6] == 'y' && path_temp[len_dst - 5] == 'a'
		&& path_temp[len_dst - 4] == 'f' && path_temp[len_dst - 3] == 'f'
		&& path_temp[len_dst - 2] == 's' && path_temp[len_dst - 1] == '2')
	{
		*fs_type = FS_YAFFS2;
	}
	else
	{
		*fs_type = 0xFF;
		return FALSE;
	}
	
	
	return TRUE;
}


BOOL spi_Creat_Partition(void)
{
	T_CREAT_PARTITION_INFO *partInfo = NULL;
	UINT i = 0, j = 0;
	void *File = 0;
	char fs_type = 0;
	UINT namelen = 0;
	
	USES_CONVERSION;
	
	//升级烧录不需要创建分区
	partInfo = new T_CREAT_PARTITION_INFO[theConfig.partition_count];
	if (partInfo == NULL)
	{
		AfxMessageBox(_T("partInfo malloc fail"));
		return FALSE;
	}
	memset(partInfo, 0, theConfig.partition_count * sizeof(T_CREAT_PARTITION_INFO));
	
	for (i = 0; i < theConfig.partition_count; i++)
	{
		if (theConfig.partition_data[i].Size == 0)
		{
			AfxMessageBox(_T("partition size must be not zero"));
			return FALSE;
		}

		namelen = strlen(T2A(theConfig.partition_data[i].Disk_Name));
		if(namelen > 6)
		{
			AfxMessageBox(_T("name len more than 7"));
			return FALSE;
		}

		memcpy(partInfo[i].name, T2A(theConfig.partition_data[i].Disk_Name), namelen);
		partInfo[i].type = theConfig.partition_data[i].type;
		partInfo[i].r_w_flag = theConfig.partition_data[i].r_w_flag;
		partInfo[i].hidden_flag = theConfig.partition_data[i].hidden_fag;
		partInfo[i].Size = theConfig.partition_data[i].Size;
		partInfo[i].mtd_idex = i + 1;
		
		if (theConfig.partition_data[i].type == FHA_FS_TYPE)
		{
			if (!spi_check_fs_type(theConfig.partition_data[i].Disk_Name, &fs_type))
			{
				AfxMessageBox(_T("check fs type fail"));
				return FALSE;
			}
			
			partInfo[i].fs_type = fs_type;
		}
		else if (theConfig.partition_data[i].type == FHA_BIN_TYPE)
		{
			UINT src_len = 0;
			UINT dst_len = 0;
			src_len = _tcslen(theConfig.partition_data[i].Disk_Name);
			for(j = 0; j < theConfig.download_nand_count; j++)
			{
				dst_len = _tcslen(theConfig.download_nand_data[j].file_name);
				if (src_len == dst_len)
				{
					if (_tcsnccmp(theConfig.partition_data[i].Disk_Name, theConfig.download_nand_data[j].file_name, dst_len) == 0)
					{
						break;
					}
				}
				
			}
			if (j == theConfig.download_nand_count)
			{
				AfxMessageBox(_T("it not have the bin partition"));
				return FALSE;
			}
			partInfo[i].check = theConfig.download_nand_data[j].bCompare;
			partInfo[i].ld_addr = theConfig.download_nand_data[j].ld_addr;
			partInfo[i].backup = theConfig.download_nand_data[j].bBackup;
		}
	}
	
	for(i = 0; i < theConfig.partition_count; i++)
    {
        File = partition_creat(partInfo);
        if(NULL == File)
        {
			AfxMessageBox(_T("creat the partitionfail"), MB_OK);
            return AK_FALSE;
        }
        else
        {
			if(partition_close(File) != 0)
			{
				AfxMessageBox(_T("close the partition fail"), MB_OK);
                return AK_FALSE;
			}
        }
		partInfo++;
    }
	
	return TRUE;
}


BOOL check_size(UINT data_size, char *fileName)
{
	UINT j = 0;
	
	USES_CONVERSION;
	
	for (j = 0; j < theConfig.partition_count; j++)
	{
		UINT down_name_len = strlen(fileName);
		UINT part_name_len = strlen(T2A(theConfig.partition_data[j].Disk_Name));
		if (down_name_len == part_name_len)
		{
			if (memcmp(fileName, T2A(theConfig.partition_data[j].Disk_Name), down_name_len) == 0)
			{
				break;
			}
		}
	}
	if(j == theConfig.partition_count)
	{
		AfxMessageBox(_T("the name is no in partition table"), MB_OK);
		return FALSE;	
	}
	
	if (theConfig.partition_data[j].Size == 0)
	{
		AfxMessageBox(_T("the name of partition size is zero"), MB_OK);
		return FALSE;
	}
	
	if (data_size == 0 && data_size > theConfig.partition_data[j].Size)
	{
		AfxMessageBox(_T("the name file size is zero"), MB_OK);
		return FALSE;
	}
	return TRUE;
}


BOOL spi_download_MAC(void)
{
	TCHAR buf[32] = {0};
	UINT len = 0;
	char tbuf[256] = {0};

	USES_CONVERSION;

	//if(theConfig.macaddr_flag) 
	{
		memset(buf, 0, 32);
		memset(tbuf, 0xFF, 256);
		_tcscpy(buf, _T("FF:FF:FF:00:00:00"));
		len = wcslen(buf);
		memcpy(&tbuf[0], &len, 4);
		memcpy(&tbuf[4], T2A(buf), len);
		
		if (!check_size(len + 4, "MAC"))
		{
			AfxMessageBox(_T("check_size MAC fail"), MB_OK);
			return FALSE;
		}
		
		//写mac地址到安全区内
		if (!download_data(tbuf, len + 4, "MAC"))
		{
			AfxMessageBox(_T("down load MAC fail"), MB_OK);
			return FALSE;
		}
	}


	return TRUE;
}

BOOL spi_download_SEQ(void)
{
	TCHAR buf[64] = {0};
	UINT len = 0;
	char tbuf[256] = {0};

	USES_CONVERSION;
	
	if(theConfig.sequenceaddr_flag) 
	{
		memset(buf, 0, 64);
		memset(tbuf, 0xFF, 256);
		_tcscpy(buf, _T("AABBCCXXDDEEFFGGHH11223344556677"));
		len = wcslen(buf);
		memcpy(&tbuf[0], &len, 4);
		memcpy(&tbuf[4], T2A(buf), len);
		
		if (!check_size(len + 4, "SEQ"))
		{
			AfxMessageBox(_T("check_size SEQ fail"), MB_OK);
			return FALSE;
		}
		
		//写mac地址到安全区内
		if (!download_data(tbuf, len + 4, "SEQ"))
		{
			AfxMessageBox(_T("down load SEQ fail"), MB_OK);
			return FALSE;
		}
	}
	
	return TRUE;
}

BOOL spi_download_ENV(void)
{
	char *tbuf= NULL;
	UINT page = 0;

	UINT len = strlen(CONFIG_EXTRA_ENV_SETTINGS);

	page = ((len*sizeof(char)+4) + SpiInfo.page_size - 1)/SpiInfo.page_size;
	page = page *SpiInfo.page_size;
		
	tbuf = (char *)malloc(page*sizeof(char));
	if (tbuf == NULL)
	{
		AfxMessageBox(_T("tbuf malloc fail"), MB_OK);
		return FALSE;
	}
	memset(tbuf, 0xFF, page*sizeof(char));
	
	memcpy(&tbuf[0], &len, 4);
	memcpy(&tbuf[4], CONFIG_EXTRA_ENV_SETTINGS, len);
	
	
	if (!check_size(len, "ENV"))
	{
		free(tbuf);
		AfxMessageBox(_T("check_size ENV fail"), MB_OK);
		return FALSE;
	}
	
	if (!download_data(tbuf, len + 4, "ENV"))
	{
		free(tbuf);
		AfxMessageBox(_T("down load ENV fail"), MB_OK);
		return FALSE;
	}

	free(tbuf);
	
	return TRUE;
}

//下载bin 文件
BOOL spi_download_bin(T_VOID)
{
	T_DOWNLOAD_BIN download_bin;
	UINT j = 0, i= 0;
	TCHAR file_path[MAX_PATH+1] = {0};
	DWORD count = 0;
	UINT file_len  = 0;
	
	USES_CONVERSION;
	for(i = 0; i < theConfig.download_nand_count; i++)
	{
		download_bin.bCompare = theConfig.download_nand_data[i].bCompare;
		_tcscpy(download_bin.pc_path , theApp.ConvertAbsolutePath(theConfig.download_nand_data[i].pc_path));
		memcpy(download_bin.file_name, T2A(theConfig.download_nand_data[i].file_name), MAX_PATH);
		for (j = 0; j < theConfig.partition_count; j++)
		{
			UINT down_name_len = strlen(download_bin.file_name);
			UINT part_name_len = strlen(T2A(theConfig.partition_data[j].Disk_Name));
			if (down_name_len == part_name_len)
			{
				if (memcmp(download_bin.file_name, T2A(theConfig.partition_data[j].Disk_Name), down_name_len) == 0)
				{
					break;
				}
			}
		}
		if(j == theConfig.partition_count)
		{
			return FALSE;	
		}
		
		if (theConfig.partition_data[j].Size == 0)
		{
			return FALSE;
		}
		
		file_len = Get_Filelenght(theApp.ConvertAbsolutePath(download_bin.pc_path));
		if (file_len == 0 && file_len > theConfig.partition_data[j].Size)
		{
			return FALSE;
		}
		
		//写文件
		if (!download_file(download_bin.pc_path, download_bin.file_name))
		{
			return FALSE;
		}
		
	}
	
    return TRUE;
}


BOOL spi_download_FS_IMG(void)
{
	UINT i = 0, j = 0;
	T_DOWNLOAD_IMG download_img;
	UINT part_size = 0;

	USES_CONVERSION;

	for(i = 0; i < theConfig.download_mtd_count; i++)
	{
		UINT dst_len = 0;
		UINT src_len = _tcslen(theConfig.download_mtd_data[i].disk_name);
		if (src_len == 0)
		{
			AfxMessageBox(_T("disk_name is null"), MB_OK);
			return FALSE;
		}
		download_img.bCompare = theConfig.download_mtd_data[i].bCompare;
		memset(download_img.pc_path, 0, MAX_PATH);
		_tcscpy(download_img.pc_path, theApp.ConvertAbsolutePath(theConfig.download_mtd_data[i].pc_path));
		memset(download_img.disk_name, 0, MAX_PATH);
		memcpy(download_img.disk_name, T2A(theConfig.download_mtd_data[i].disk_name), src_len);
		
		for(j = 0; j < theConfig.partition_count; j++)
		{
			dst_len = _tcslen(theConfig.partition_data[j].Disk_Name);
			if (theConfig.partition_data[j].type == FHA_FS_TYPE && dst_len == src_len)
			{
				if (_tcsnccmp(theConfig.download_mtd_data[i].disk_name, theConfig.partition_data[j].Disk_Name, src_len) == 0)
				{
					part_size = theConfig.partition_data[j].Size*1024;//K为单位
					break;
				}	
			}
			
		}
		if (i == theConfig.partition_count)
		{
			AfxMessageBox(_T("the disk name is not in the partition table"), MB_OK);
			return FALSE;
		}
		
		if (part_size == 0 || part_size > SpiInfo.total_size)
		{
			AfxMessageBox(_T("the disk name partition size is error"), MB_OK);
			return FALSE;
		}
		
		if (!download_file(download_img.pc_path, download_img.disk_name))
		{
			return FALSE;
		}
	}

	return TRUE;
}

BOOL spi_download_BOOT(void)
{
	UINT file_handle = 0;
	DWORD count;
	UINT ChipType = theConfig.chip_type;
	BYTE *buf = NULL;
	DWORD dwWriteSize = 0;
	T_U32  high_pos = 0;
    T_U32  dwptr;
	
	HANDLE boot_hFile = CreateFile(theApp.ConvertAbsolutePath(theConfig.path_nandboot), 
        GENERIC_READ, 
        FILE_SHARE_READ, 
        NULL, 
        OPEN_EXISTING, 
        FILE_ATTRIBUTE_NORMAL, 
        NULL);//CreateFile
	
    if(INVALID_HANDLE_VALUE == boot_hFile)//判断
    {
		AfxMessageBox(_T("creat boot file fail"), MB_OK);
        return FALSE;
    }
	
    UINT boot_dwSize = GetFileSize(boot_hFile, NULL);//boot_dwSize
	
	if(0xFFFFFFFF == boot_dwSize || boot_dwSize  == 0)
	{
		AfxMessageBox(_T("creat boot len is zero"), MB_OK);
		CloseHandle(boot_hFile);
		return FALSE;
	}
	
	buf = (BYTE *)Spi_Malloc(boot_dwSize * sizeof(BYTE));
	Spi_MemSet(buf, 0xFF, boot_dwSize * sizeof(BYTE));
	
	if(NULL == buf)//分配失败
	{
		AfxMessageBox(_T("buf mallco fail"), MB_OK);
		CloseHandle(boot_hFile);
		return FALSE;
	}

	//读boot 的前512 byte
	if (!ReadFile(boot_hFile, buf, 512,  &count, NULL))//ReadFile
	{
		AfxMessageBox(_T("read file fail"), MB_OK);
		CloseHandle(boot_hFile);
		Spi_Free(buf);
		return FALSE;
	}
	
	if (512 != count)
	{
		AfxMessageBox(_T("read len error"), MB_OK);
		CloseHandle(boot_hFile);
		Spi_Free(buf);
		return FALSE;
	}
	
	//根据不同的芯片改变某些值
	switch(ChipType)
	{
		case CHIP_39XX_H2:
		case CHIP_39XX_H240:
			config_spiboot_aspen(buf, boot_dwSize - 512, SpiInfo.clock);//config_spiboot_aspen
			break;
		case CHIP_10XX_T:
			config_spiboot_snowbird(buf, ChipType, boot_dwSize, SpiInfo.clock);//config_spiboot_snowbird
			break;		
		default:
			break;
	}
	
	//读boot 512 后的byte
	if (!ReadFile(boot_hFile, &buf[512], boot_dwSize - 512,  &count, NULL))
	{
		AfxMessageBox(_T("read file fail"), MB_OK);
		CloseHandle(boot_hFile);
		Spi_Free(buf);
		return FALSE;
	}
	
	if ((boot_dwSize - 512) != count)//读长度不一样
	{
		AfxMessageBox(_T("read len fail"), MB_OK);
		CloseHandle(boot_hFile);
		Spi_Free(buf);
		return FALSE;
	}
	
	ConfigSpibootParam(0, buf);//ConfigSpibootParam

	if(Spi_hHandle==INVALID_HANDLE_VALUE)
	{
		AfxMessageBox(_T("Spi_hHandle==INVALID_HANDLE_VALUE"), MB_OK);
		return AK_FALSE;
	}
	
    dwptr = SetFilePointer( Spi_hHandle, 0, (long *)&high_pos, FILE_BEGIN);//SetFilePointer
    if ( T_U32_MAX == dwptr && (GetLastError() != NO_ERROR) )
    {
		AfxMessageBox(_T("spi_download_BOOT: SetFilePointer faild"), MB_OK);
		return AK_FALSE;
    }
	

	if (WriteFile(Spi_hHandle, buf, boot_dwSize, &dwWriteSize, NULL) == 0)
	{
		AfxMessageBox(_T("read len fail"), MB_OK);
		CloseHandle(boot_hFile);
		Spi_Free(buf);
		return FALSE;
	}

	if(dwWriteSize != boot_dwSize)
	{
		AfxMessageBox(_T("WriteFile len fail"), MB_OK);
		CloseHandle(boot_hFile);
		Spi_Free(buf);
		return FALSE;

	}

	Spi_Free(buf);
	CloseHandle(boot_hFile);//CloseHandle

	return TRUE;
	
}

//入口
void SpiImageCreate::Spi_Enter(CString spiName, CString strPath)
{
	UINT i = 0;
	char *buf = NULL;
	UINT write_len = 1024*1024;
	DWORD dwWriteSize = 0;
	UINT write_num = 0;

	//根据spi型号获取参数
	USES_CONVERSION;

	for(i = 0; i < theConfig.spiflash_parameter_count; i++)//循环查找
	{
		if (_tcscmp(spiName, A2T(theConfig.spiflash_parameter[i].des_str)) == 0)
		{
			memcpy(&SpiInfo, &theConfig.spiflash_parameter[i], sizeof(T_SFLASH_PHY_INFO_TRANSC));
			break;
		}
	}

	if (i == theConfig.spiflash_parameter_count)//没有找到
	{
		MessageBox(NULL, _T("not find the spiflash!"), NULL,MB_OK);
		return;
	}
	
	
	//打开handle
	Spi_hHandle = CreateFile(strPath, GENERIC_WRITE | GENERIC_READ,
					     FILE_SHARE_WRITE | FILE_SHARE_READ,
					     NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (INVALID_HANDLE_VALUE == Spi_hHandle)
	{
        MessageBox(NULL, _T("creat spiimage CreateFile fail!"), NULL,MB_OK);
		return;
	}

	buf = (char *)Spi_Malloc(write_len*sizeof(char));
	if (buf == NULL)
	{
		AfxMessageBox(_T("buf malloc fail"), MB_OK);
		return;
	}
	memset(buf, 0xFF, write_len*sizeof(char));

	write_num = SpiInfo.total_size/write_len;

	if (SpiInfo.total_size%write_len != 0)
	{
		AfxMessageBox(_T("spiflash total size is error"), MB_OK);
		return ;
	}

	//写一个全FF的文件
	for(i = 0; i < write_num; i++)
	{
		if(WriteFile(Spi_hHandle, buf, write_len, &dwWriteSize, NULL) == 0)//WriteFile
		{
			AfxMessageBox(_T("WriteFile file ff fail"), MB_OK);
			return ;
		}
	}
	
	
	if(dwWriteSize != write_len)
	{
		AfxMessageBox(_T("WriteFile len error"), MB_OK);
		return;
	}


	//初始spi	
	if (AK_NULL == fhalib_init(TRANSC_MEDIUM_SPIFLASH, MODE_NEWBURN))//fhalib_init(MEDIUM_SPIFLASH, MODE_NEWBURN, &SpiInfo))
    {
		printf("fhalib_init fail\n");
		MessageBox(NULL, _T("creat spiimage fhalib_init fail!"), NULL,MB_OK);
		return;
    }
	

	//创建分区表
	if (!spi_Creat_Partition())
	{
		return;
	}

	//下载bin	
	if (!spi_download_bin()) //
	{
		return;
	}

	//写mac
	if (!spi_download_MAC()) //
	{
		return;
	}

	//写序列号
	if (!spi_download_SEQ()) //
	{
		return;
	}

	//写env
	if (!spi_download_ENV()) //
	{
		return;
	}

	//下载mtd
	if (!spi_download_FS_IMG()) //
	{
		return;
	}

	//下载boot
	if (!spi_download_BOOT())//
	{
		printf("FHA_download_boot fail\n");
		MessageBox(NULL, _T("creat spiimage download boot fail!"), NULL,MB_OK);
		return;
	}
	
	CloseHandle(Spi_hHandle);//关闭handle
	AfxMessageBox(_T("完成SPI镜像制作"));//提示完成
	return;
}
