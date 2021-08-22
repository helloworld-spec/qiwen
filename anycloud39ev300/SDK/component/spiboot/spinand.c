/**
 * @file spi_flash.c
 * @brief spiflash driver source file
 * Copyright (C) 2016 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author luoyongchuang
 * @date 2016-06-3
 * @v
 */
 
 
#include "spi.h"
#include "spinand.h"
#include "anyka_types.h"
#include "system.h"
#include "anyka_cpu.h"


#define NAND_TEST_NAME  "T_UVC"



#define NAND_BOIS_NAME  "KERNEL"
#define NAND_BOIS_NAME_BAK  "KERBAK"

#define NAND_A_NAME  "A"
#define NAND_A_NAME_BAK  "AK"


#define NAND_B_NAME  "B"
#define NAND_B_NAME_BAK  "BK"



#define UPDATE_NULL  ""       // NO UPDATA
#define UPDATE_ING   "U_ING"  //UPDATE ING
#define UPDATE_END   "U_END"  //UPDATE FINISH
#define UPDATE_ERR   "U_ERR"  //UPDATE ERROR, is not to update


typedef enum
{
    U_UPDATE_NULL = 0,
    U_UPDATE_ING,
    U_UPDATE_ENG,
    U_UPDATE_ERR
}E_UPDATE_FLAG;
	
typedef enum
{
   PART_DATA_TYPE = 0,
   PART_BIN_TYPE,
   PART_FS_TYPE,
}PART_TYPE;



#define printf 	printf

#define ALL_SECTION 0xFF
#define		SPINAND_PARAM_OFFSET	            0x200  


typedef struct tag_NandConfig{
    T_U8  Resv[4];
    T_U32 BinFileCount;
    T_U32 ResvStartBlock;
    T_U32 ResvBlockCount;
    T_U32 BootLength;
    T_U32 BinStartBlock;
    T_U32 BinEndBlock;
    T_U32 ASAStartBlock;
    T_U32 ASAEndBlock;
    T_U8  Resv_ex[28];
}T_NAND_CONFIG;


typedef struct tag_update_info{
	T_U32  start_page;
	T_U32  data_len;
	T_U32  partitin_len;
    T_U8   partition_name[8];

	T_U32  start_page_bak;
	T_U32  data_len_bak;
	T_U32  partitin_len_bak;
    T_U8   partition_name_bak[8];

	T_U8   update_flag; //0表示没有升级, 1表示正在升级, 2表示已升级, 3表示无法再升级
	T_U8   partition_type;
}T_UPDATE_INFO;



typedef enum
{
    AREA_P0 = 0,
    AREA_BOOT,
    AREA_FSA,
    AREA_ASA,
    AREA_CNT
}E_AREA;

typedef enum
{
    SAMSUNG = 0xEC,
    HYNIX = 0xAD,
    TOSHIBA = 0x98,
    MICRON = 0x2C
}E_MAKER;


#define CONTINU_MAX_BD_NUM  50 


static T_U8 nRwPage;
static T_U8 nEBlock;
static T_NAND_ECC_CTRL  m_ASAEcc;
T_U32 g_nPageSize = 2048;
T_U32 g_nOobpos = 4;  //OOB usr data pos
T_U32 g_nPagePerBlock = 64;
T_U32 g_Blocknum = 1024;

T_U32 g_continu_badBlocknum = 0;


static unsigned char m_buf_stat = 0;
T_U8 m_pBuf_BadBlk[2048] = {0};
T_U32 m_aBuffer[2048];
T_U32 m_partiton_buf[512] = {0};
T_U8 g_startblock = 0;//
T_U8 g_startblock_backup = 0;//

//static T_U8 m_aName[] = NAND_BOIS_NAME;
//static T_U8 m_aName_spinand[8] = {0};//NAND_BOIS_NAME;
//static T_U8 m_aName_bak_spinand[8] = {0};//NAND_BOIS_NAME;
//static T_U8 m_A_spinand[8] = {0};//NAND_BOIS_NAME;
//static T_U8 m_A_bak_spinand[8] = {0};//NAND_BOIS_NAME;



T_U8 g_bios_bak_flag = 0;
T_U8 g_A_bak_flag = 0;
T_U8 g_A_flag = 0;



T_ASA_PARAM m_fha_asa_param = {0};
T_ASA_BLOCK m_fha_asa_block = {0};
unsigned char m_hinfo_data[HEAD_SIZE] = {0x41, 0x4E, 0x59, 0x4B, 0x41, 0x41, 0x53, 0x41};


unsigned long g_oob_size = 64;
unsigned char g_page_buf_temp[2048 + 64] = {0};
unsigned char g_oob_buf_temp[64] = {0};


T_BOOL asa_read_page(unsigned long page, unsigned char data[], unsigned long count);
static void asa_update_page_data(unsigned long page, unsigned char data[], unsigned long bad_blocks[], unsigned long bb_count);


T_U32 cmp(T_U8 *pStr1, T_U8 *pStr2, T_U32 nLength)
{
    T_U32 i;

    for (i = 0; i < nLength; i++)
    {
        if (*pStr1++ != *pStr2++)
            break;
    }
    
    if (i != nLength)
        return 1;
    else
    {
        return 0;
    }
}

static T_BOOL get_feature(T_U8 addr, T_U8 *val)
{
	T_U8 buf[2];

	buf[0] = SPI_NAND_GET_FEATURE;
	buf[1] = addr;

	if (!spi_master_write(SPI_ID0, buf, 2, AK_FALSE))
	{
		printf("get feature fail.\r\n");
		return AK_FALSE;
	}

	if (!spi_master_read(SPI_ID0, val, 1, AK_TRUE))
	{
		printf("get feature fail 2.\r\n");
		return AK_FALSE;
	}
	return AK_TRUE;
}

static T_BOOL wait_cmd_finish(T_U8 *status)
{
	T_U32 timeout = 0;

	while(timeout++ < 1000000)
	{
		if(get_feature(ADDR_STATUS, status))
		{
			if(!(*status&STATUS_OIP))
				return AK_TRUE;
		}
	}
	return AK_FALSE;
}


static T_BOOL check_ECC_status(void)
{
	T_U32 timeout = 0;
	T_U8 ecc = 0;
	T_U8 status = 0;

	while(timeout++ < 1000000)
	{
		if(get_feature(ADDR_STATUS, &status))
		{
			ecc = (status >> 4) & 0x3;
			//printf("status:%x, ecc:%d\n", status, ecc);
	        if(2 == ecc)    //bit error and not corrected
	        {
	        	printf("r ecc error sr:%x\r\n", status);
	            return AK_FALSE;
	        }
			else
			{
				return AK_TRUE;
			}
		}
	}
	return AK_FALSE;
}




T_BOOL spi_nand_write(unsigned long row, unsigned long column, unsigned char *data, unsigned long len)
{
    unsigned char buf[4];
    unsigned long count = len;
    unsigned char status;
    T_BOOL ret = AK_FALSE;

	if(!spi_IsInit())
    {
        return AK_FALSE;
    }

	//printf("row:%d, column:%d, len:%d\r\n", row, column, len);
	//printf("data:%02x, %02x, %02x, %02x\r\n", data[0], data[1], data[2], data[3]);
    //write enable
    write_enable_spinand();

    //program load
    buf[0] = SPI_NAND_PROGRAM_LOAD;    
    buf[1] = (column >> 8) & 0x0F;
    buf[2] = column & 0xFF;
    //buf[3] = data[0];

    if (!spi_master_write(SPI_ID0, buf, 3, AK_FALSE))
    {
        printf("spi_master_write 00 fail\r\n");
        goto WRITE_EXIT;
    }

 
    if (!spi_data_mode(SPI_ID0, SPI_BUS1))
    {
        printf("spi_data_mode fail\r\n");
    }


    if (!spi_master_write(SPI_ID0, data, count, AK_TRUE))
    {
        printf("spi_master_write 04 fail\r\n");
        goto WRITE_EXIT;
    }

    if (!spi_data_mode(SPI_ID0, SPI_BUS1))
    {
        printf("spi_data_mode fail\r\n");
    }


    //program execute
    buf[0] = SPI_NAND_PROGRAM_EXECUTE;
    buf[1] = (row >> 16) & 0xFF;
    buf[2] = (row >> 8) & 0xFF;
    buf[3] = (row >> 0) & 0xFF;

    if (!spi_master_write(SPI_ID0, buf, 4, AK_TRUE))
    {
        goto WRITE_EXIT;
    }

    //check status
    if (wait_cmd_finish(&status))
    {
        if(!(status & STATUS_P_FAIL))
        {
        	//printf("status:%x\r\n", status);
            ret = AK_TRUE;
        }
        else
        {
        	printf("fail status == STATUS_P_FAIL\r\n");
        }
    }


WRITE_EXIT:

    return ret;

}




//T_BOOL ak_spi_nandflash_read(T_U32 page, T_U8 *buf)
T_BOOL ak_spi_nandflash_read(unsigned long page, unsigned long column, unsigned char *buf, unsigned long len)
{
    T_U8 buf1[4];
    T_U32 i, addr = page *g_nPageSize;
    T_BOOL bReleaseCS=AK_FALSE;
    T_U32 count, readlen;
	T_U8 status;

#ifndef  MAX_RD_SIZE  
#define MAX_RD_SIZE (32*1024)
#endif
    if((page > 0xffff) || (AK_NULL == buf))
    {
    	printf("ak_spi_nandflash_read:%d\r\n", page);
        return AK_FALSE;
    }

    if(!spi_IsInit())
    {
    	printf("spi_IsInit fail\r\n");
        return AK_FALSE;
    }
    
    buf1[0] = SPI_NAND_PAGE_READ;
    buf1[1] = (page >> 16) & 0xFF;
    buf1[2] = (page >> 8) & 0xFF;
    buf1[3] = (page >> 0) & 0xFF;
		
    spi_master_write(SPI_ID0, buf1, 4, AK_TRUE);

	//printf("read page:%d.\r\n", page);
	if(!wait_cmd_finish(&status))
	{
		return AK_FALSE;
	}

	buf1[0] = SPI_NAND_READ_CACHE;   
	buf1[1] = (column >> 8) & 0x0F;
	buf1[2] = column & 0xFF;
	buf1[3] = 0x0;

    spi_master_write(SPI_ID0, buf1, 4, AK_FALSE);

	//printf("len:%d\r\n", len);
	count = len ;//g_nPageSize;
    while(count>0)
    {
        readlen = (count > MAX_RD_SIZE)? MAX_RD_SIZE: count;
        count -= readlen;
        //only the last read, release CS
        bReleaseCS = (count>0)? AK_FALSE: AK_TRUE;
        if(!spi_master_read(SPI_ID0, buf, readlen, bReleaseCS))
            return AK_FALSE;
        buf += readlen;        
    }

	if(!check_ECC_status())
	{
		return AK_FALSE;
	}
    
    return AK_TRUE;
}


T_BOOL spi_nand_read(T_U32 page, T_U32 column, T_U8 *buf, T_U32 buf_len)
{
	//int ii;
	//for(ii=page; ii<page+page_cnt; ii++)
	return ak_spi_nandflash_read(page, column, buf, buf_len);
}





static T_VOID  config_device_data(T_NAND_DATA *pData, T_U8 *pMain, T_U8 *pAdd,T_U8 nPageCnt, T_U8 nSectCnt, E_AREA eArea)
{
    T_U32 nMaxSectCnt;
    
    pData->pEccCtrl = &m_ASAEcc;
    nMaxSectCnt = pData->pEccCtrl->nMainSectCnt;
    pData->nSectCnt = nMaxSectCnt < nSectCnt ? nMaxSectCnt : nSectCnt;
    pData->nPageCnt = nPageCnt;
    pData->pMain  = pMain;
    pData->pAdd  = pAdd;
}


T_BOOL Fwl_Nand_Read_ASAPage(T_U32 nChip, T_U32 nAbsPage, T_U8 *pMain)
{ 
    T_U32 nDevRet;
    T_NAND_DATA Data;
    T_NAND_ADDR Addr;
    T_U8 aAdd[32];
    //T_BOOL bRet = AK_TRUE;
    T_U32 nBlock;
    T_U32 nPage;
    

    Addr.nTargetAddr = 0;
    Addr.nSectAddr = 0;
    Addr.nLogAbsPageAddr = nAbsPage;

	config_device_data(&Data, pMain, \
	     aAdd, nRwPage, \
	    ALL_SECTION, AREA_ASA);
	

	if(!spi_nand_read(Addr.nLogAbsPageAddr, 0, Data.pMain, g_nPageSize))
	{
	    puts("spi_nand_read err\r\n");
	    return AK_FALSE;;
	}
	
	return AK_TRUE;
}



static T_BOOL Read_Flag(unsigned long nPage, unsigned char *pAdd, unsigned long nAddLen)
{
    unsigned long colnum = g_nOobpos + g_nPageSize;
    unsigned long i = 0;
    
   // pp_printf("Read_Flag colnum:%d\n",colnum);
    if (0 == nAddLen)
    {
        printf("Read_Flag nAddLen == 0\n");
        return AK_TRUE;
    } 
    
    if (!spi_nand_read(nPage, colnum, pAdd, nAddLen))
    {
        printf("spi_nand_read fail\n");
        return AK_FALSE;
    }

    return AK_TRUE;
}


static T_BOOL Read_Page(unsigned long nBlock, unsigned long nPage, unsigned char *pMain, unsigned char *pAdd, unsigned long nAddLen)
{
    unsigned long coladd;
    unsigned long rowadd;

    rowadd = nBlock*g_nPagePerBlock + nPage;
    coladd=0;

    if (!spi_nand_read(rowadd, coladd, pMain, g_nPageSize))
    {        
        printf("Read:block:%d,page:%d, abpage:%d\n",nBlock,nPage,rowadd);
        return AK_FALSE;
    }
    
    if (!Read_Flag(rowadd, pAdd, nAddLen))
    {
        printf("Read_Page fail\n");
        return AK_FALSE;
    }

    return AK_TRUE;
}



T_BOOL FHA_Nand_ReadPage(unsigned long nBlock, unsigned long page, unsigned char *pMain, unsigned char *pAdd, unsigned long nAddLen)
{
    unsigned long i = 0;
	
    
    if (!Read_Page(nBlock, page, pMain, pAdd, nAddLen))
    {   
        printf("FHA_Nand_ReadPage fail nblock:%d, nAbsPage:%d\n", nBlock, page);
        return AK_FALSE;
    }
    
    return AK_TRUE;
}

T_BOOL FHA_Nand_WritePage(unsigned long nBlock, unsigned long nPage,  unsigned char *data, unsigned char *oob, unsigned long ooblen)
{
    T_BOOL ret = AK_FALSE;
    unsigned long write_len = 0;
    unsigned long rowadd;
    unsigned long coladd, i = 0;

    rowadd = nBlock*g_nPagePerBlock + nPage;
    coladd = 0;
        
    memset(g_page_buf_temp, 0, g_nPageSize + g_oob_size);
    memset(g_oob_buf_temp, 0, g_oob_size);

    write_len = g_nPageSize;
    memcpy(g_page_buf_temp, data, write_len);

    if(oob != AK_NULL && ooblen != 0)
    {
         //冉卸spare拇小
         if(0 == spi_nand_read(rowadd, coladd + g_nPageSize, g_oob_buf_temp, g_oob_size))   
         {
            printf("spi_nand_read oob fail\r\n");
            return AK_FALSE;
         }
         memcpy(&g_oob_buf_temp[g_nOobpos], oob, ooblen);
         //spare
         memcpy(&g_page_buf_temp[write_len], g_oob_buf_temp, g_oob_size);
         write_len = write_len + g_oob_size;
         
    }

	//printf("spi_nand_write start\r\n");
    ret = spi_nand_write(rowadd, coladd, g_page_buf_temp, write_len);
	//printf("spi_nand_write end\r\n");
    return ret;
}



T_BOOL FHA_Nand_WritePage_burn(unsigned long nBlock, unsigned long nPage,  unsigned char *data)
{
	unsigned long rowadd;
    unsigned long coladd, i = 0;
	unsigned long write_len = g_nPageSize + g_oob_size;
	

	rowadd = nBlock*g_nPagePerBlock + nPage;
    coladd = 0;

	if(!spi_nand_write(rowadd, coladd, data, write_len))
	{
		printf("spi_nand_write fail\r\n");
		return AK_FALSE;
	}

	return AK_TRUE;
}


T_BOOL FHA_Nand_ReadPage_burn(unsigned long nBlock, unsigned long nPage,  unsigned char *data)
{
	unsigned long coladd;
    unsigned long rowadd;
	unsigned long read_len = g_nPageSize + g_oob_size;
	
    rowadd = nBlock*g_nPagePerBlock + nPage;
    coladd=0;
	

	if(!spi_nand_read(rowadd, coladd,  data, read_len))
	{
		printf("spi_nand_read fail\r\n");
		return AK_FALSE;
	}
	

	return AK_TRUE;
}





T_VOID print_bytes(T_U8* pBytes,T_U32 nLen,T_S8* pTag)
{
	T_U32 i;
	printf("\n-------%s------",pTag);
	for(i = 0; i < nLen; i++)
	{
	    if(0 == (i&31)) printf("\n");
	    printf("%.2x ",pBytes[i]);
	}
	printf("\n");
}


static T_BOOL set_partition_table(T_UPDATE_INFO update_info[])
{
	T_U32 j = 0, i = 0, nFileCnt= 0;
	T_U32 partition_cnt = (T_U32)m_partiton_buf[0];
	T_PARTITION_TABLE_INFO *partition;
	T_U32 partition_len_temp = 0;
	T_U8 partition_name[8] = {0};
	T_U8 partition_name_up[8] = {0};
	T_U8 partition_name_up_bak[8] = {0};
	T_U8 update_partition_flag = 0;
	T_U8 main_flag = 0, backup_flag = 0;
	
	//更新分区的有效数据长度

	nFileCnt = m_partiton_buf[0];	
	printf("nFileCnt:%d\r\n", nFileCnt);

	for(j = 0; j < PARTITION_CNT; j++)
	{
		//printf("update_info[%d].update_flag:%d\r\n", j, update_info[j].update_flag);
		if(update_info[j].update_flag == U_UPDATE_ING || update_info[j].update_flag == U_UPDATE_ENG)
		{
			if(update_info[j].update_flag == U_UPDATE_ING)//更新主分区到备份分区的长度
			{
				memset(partition_name_up, 0, 8);
				memcpy(partition_name_up, update_info[j].partition_name, 6);

				memset(partition_name_up_bak, 0, 8);
				memcpy(partition_name_up_bak, update_info[j].partition_name_bak, 6);
			}
			else//更新备份分区到主分区的长度
			{
				memset(partition_name_up, 0, 8);
				memcpy(partition_name_up, update_info[j].partition_name_bak, 6);

				memset(partition_name_up_bak, 0, 8);
				memcpy(partition_name_up_bak, update_info[j].partition_name, 6);
			}

			
		
			partition = (T_PARTITION_TABLE_INFO *)(&m_partiton_buf[1]);

			//先获取主分区的长度
			for (i = 0; i < nFileCnt; i++) 
			{
				memset(partition_name, 0, 8);
				memcpy(partition_name, partition->partition_info.name, 6);
				if(0 == cmp(partition_name_up, (T_U8 *)(partition_name), strlen(update_info[j].partition_name))
						&& strlen(partition_name_up) == strlen(partition_name))
				{
					partition_len_temp = partition->ex_partition_info.parameter1;
					printf("partition_name_up:%s, partition_len_temp:%d\r\n", partition_name_up, partition_len_temp);
					break;
				}
				partition++;
			}

			//获取备份分区的长度
			partition = (T_PARTITION_TABLE_INFO *)(&m_partiton_buf[1]);
			//先获取主分区的长度
			for (i = 0; i < nFileCnt; i++) 
			{
				memset(partition_name, 0, 8);
				memcpy(partition_name, partition->partition_info.name, 6);
				if(0 == cmp(partition_name_up_bak, (T_U8 *)(partition_name), strlen(update_info[j].partition_name_bak))
						&& strlen(partition_name_up_bak) == strlen(partition_name))
				{

					printf("partition_name_up_bak:%s, parameter1:%d\r\n", partition_name_up_bak, partition->ex_partition_info.parameter1);
					//再获取备份分区的长度,判断如果不一样的话,就需要更新
					if(partition_len_temp != partition->ex_partition_info.parameter1)
					{
						partition->ex_partition_info.parameter1 = partition_len_temp;
						printf("update %s data_len:%d\r\n", partition_name_up_bak, partition->ex_partition_info.parameter1);
						update_partition_flag = 1;
					}
					break;
				}
				partition++;
			}
		}
	}
	
	if(update_partition_flag == 1)
	{
		//写入分区表
		if(Prod_NandEraseBlock(g_startblock) == - 1){
			printf("set_partition_table erase fail, g_startblock:%d \r\n", g_startblock);
			main_flag = 1;
		}
		else{
			if(!FHA_Nand_WritePage(g_startblock, 0,  (T_U8 *)m_partiton_buf, AK_NULL, 0))
			{
				printf("set_partition_table FHA_Nand_WritePage fail,g_startblock:%d, now write bak table\r\n", g_startblock);
				main_flag = 1;
			}
		}

		//写备分分区表
		if(Prod_NandEraseBlock(g_startblock_backup) == - 1){
			printf("set_partition_table erase fail, g_startblock_backup:%d \r\n", g_startblock_backup);
			backup_flag = 1;

		}
		else
		{
			if(!FHA_Nand_WritePage(g_startblock_backup, 0, (T_U8 *)m_partiton_buf, AK_NULL, 0))
			{
				printf("set_partition_table FHA_Nand_WritePage bak table fail, g_startblock_backup:%d\r\n", g_startblock_backup);
				backup_flag = 1;
			}
		}

		if(backup_flag == 1 && main_flag == 1)
		{
			return AK_FALSE;
		}
	}

	return AK_TRUE;
}


static T_BOOL get_partition_table(T_UPDATE_INFO update_info[],T_U32 *p_nBiosAddress)
{
	T_PARTITION_TABLE_INFO *partition;
	T_U32 nFileCnt;
	T_U32 i = 0, j = 0;
    T_U32 offset = 0;
	T_U32 temp = 0;
	T_U32 partition_cnt = 0;
	T_U8 bios_flag = 0;
	T_U8 fs_flag[3] = {0};
	T_U8 fs_bak_flag[3] = {0};
	T_U8 partition_name[8] = {0};
	T_U8 reconvert_table_flag = 0;

	
	memset(m_partiton_buf, 0, sizeof(m_partiton_buf));
	if(!Fwl_Nand_Read_ASAPage(0, 0, (T_U8*)m_partiton_buf))
	{        
	    return AK_FALSE;
	}

	offset = SPINAND_PARAM_OFFSET;

    // unsigned char   row_cycle;//
    // unsigned char   delay_cnt;//
    //SPINAND
    //

	temp = offset + 17;
	g_startblock = ((T_U8*)m_partiton_buf)[temp];
	temp = offset + 18;
	g_startblock_backup =  ((T_U8*)m_partiton_buf)[temp];
	//printf("startblock:%d\n", startblock);
	//printf("startblock_backup:%d\n", startblock_backup);

	if (!Fwl_Nand_Read_ASAPage(0, g_startblock*g_nPagePerBlock, (T_U8*)m_partiton_buf))
	{
		printf("read the partition backup block:%d\r\n", g_startblock_backup);
		if (!Fwl_Nand_Read_ASAPage(0, g_startblock_backup*g_nPagePerBlock, (T_U8*)m_partiton_buf))
		{
			printf("read the partition backup block fail:%d\r\n", g_startblock_backup);
	    	return AK_FALSE;
		}
		reconvert_table_flag = 1;
	}
	
	nFileCnt = m_partiton_buf[0];	
	printf("nFileCnt:%d\r\n", nFileCnt);

	if(nFileCnt > g_nPageSize/sizeof(T_PARTITION_TABLE_INFO) || nFileCnt <= 0)
	{
		printf("nFileCnt %d error\r\n", nFileCnt);
		
		printf("read the partition backup block:%d\r\n", g_startblock_backup);
		if (!Fwl_Nand_Read_ASAPage(0, g_startblock_backup*g_nPagePerBlock, (T_U8*)m_partiton_buf))
		{
			printf("read the partition backup block 11 fail:%d\n",g_startblock_backup);
	    	return AK_FALSE;
		}
		nFileCnt = m_partiton_buf[0];	
		printf("nFileCnt:%d\r\n", nFileCnt);


		if(nFileCnt > g_nPageSize/sizeof(T_PARTITION_TABLE_INFO) || nFileCnt <= 0)
		{
			 printf("nFileCnt %d error\r\n", nFileCnt);
			 return AK_FALSE;
		}

		reconvert_table_flag = 1;
		
	}

	if(reconvert_table_flag == 1)
	{
		//恢复原分区表数据
		if(Prod_NandEraseBlock(g_startblock) == - 1){
			printf("revonvert partition table erase fail, g_startblock:%d \r\n", g_startblock);
		}
		if(!FHA_Nand_WritePage(g_startblock, 0,  (T_U8 *)m_partiton_buf, AK_NULL, 0))
		{
			printf(" revonvert partition table fail,g_startblock:%d\r\n", g_startblock);
		}
	}

	for(j = 0; j < PARTITION_CNT; j++)
	{
		//printf("update_info[%d].update_flag:%d\r\n", j, update_info[j].update_flag);
		if(update_info[j].update_flag == U_UPDATE_ING || update_info[j].update_flag == U_UPDATE_ENG || j == 0)
		{
			partition = (T_PARTITION_TABLE_INFO *)(&m_partiton_buf[1]);
			for (i = 0; i < nFileCnt; i++) 
			{
				//printf("name:%s, %s\r\n", partition->partition_info.name, update_info[j].partition_name);

				memset(partition_name, 0, 8);
				memcpy(partition_name, partition->partition_info.name, 6);
				
			    if (0 == cmp(update_info[j].partition_name, (T_U8 *)(partition_name), strlen(update_info[j].partition_name))
					&& strlen(update_info[j].partition_name) == strlen(partition_name))
				{
					update_info[j].start_page = partition->partition_info.start_pos/g_nPageSize;
					update_info[j].data_len = partition->ex_partition_info.parameter1;
					update_info[j].partitin_len = partition->partition_info.ksize*1024;
					update_info[j].partition_type = partition->partition_info.type;
					//printf("update_info[%d].start_page:%d\r\n", j, update_info[j].start_page);
					//printf("update_info[%d].data_len:%d\r\n", j, update_info[j].data_len);
					//printf("update_info[%d].partitin_len:%d\r\n", j, update_info[j].partitin_len);
					if(j == 0)
					{
						*p_nBiosAddress = partition->ex_partition_info.parameter2;
						//printf("*p_nBiosAddress:%d\n",*p_nBiosAddress);
						bios_flag = 1;
					}
					fs_flag[j] = 1;
			    }
				else if(0 == cmp(update_info[j].partition_name_bak, (T_U8 *)(partition_name), strlen(update_info[j].partition_name_bak))
					&& strlen(update_info[j].partition_name_bak) == strlen(partition_name))
				{

					update_info[j].start_page_bak = partition->partition_info.start_pos/g_nPageSize;
					update_info[j].data_len_bak = partition->ex_partition_info.parameter1;
					update_info[j].partitin_len_bak = partition->partition_info.ksize*1024;
					update_info[j].partition_type = partition->partition_info.type;
					//printf("update_info[%d].start_page_bak:%d\r\n", j, update_info[j].start_page_bak);
					//printf("update_info[%d].data_len_bak:%d\r\n", j, update_info[j].data_len_bak);
					//printf("update_info[%d].partitin_len_bak:%d\r\n", j, update_info[j].partitin_len_bak);
					
					fs_bak_flag[j] = 1;
					break;
				}
			    else
			    {
					//printf("%s:%s\n", m_aName_spinand, (T_U8 *)(partition->partition_info.name));
			    }
				partition++;
			}
		}
	}


	//printf("read partition table finish\r\n");

	//只需要判断kernel原数据区是是否存在,因为备份区有可能没有需要的
	if (bios_flag == 0)
    {
    	printf("no find %s\r\n", update_info[0].partition_name);
        return AK_FALSE;
    }


	for(j = 0; j < PARTITION_CNT; j++)
	{
		//printf("fs_flag[%d]:%d, fs_bak_flag[%d]:%d\r\n",j, fs_flag[j], j, fs_bak_flag[j]);
		
		if (update_info[j].update_flag == U_UPDATE_ING || update_info[j].update_flag == U_UPDATE_ENG)
	    {
	    	if(fs_flag[j] == 0)
	    	{
	    		printf("no find %s\r\n", update_info[j].partition_name);
				return AK_FALSE;
	    	}

			if(fs_bak_flag[j] == 0)
	    	{
	    		printf("no find %s\r\n", update_info[j].partition_name_bak);
				return AK_FALSE;
	    	}
	    	
	        
	    }
	}
	
	return AK_TRUE;
}

/**
 * @brief Read SPI Nand Block
 *
 * @author xie_wenzhong
 * @date 2015-6-18
 * @param[in] blk_pos: The Block will Be Read.
 * @param[in] page_cnt: remain pages.
 * @param[out] buf: read data.
 * @return T_BOOL
 */    
T_BOOL Fwl_Nand_Read_Block(T_U32 blk_pos, T_U32 page_cnt, T_U8* buf)
{
	T_U32 page;
	page_cnt = page_cnt<g_nPagePerBlock ? page_cnt : g_nPagePerBlock;
	
	for (page = 0; page < page_cnt; ++page){
        if (!Fwl_Nand_Read_ASAPage(0, blk_pos*g_nPagePerBlock + page, buf)) {
            return AK_FALSE;
        }
                
        buf += g_nPageSize;
        printf(".");
    }

	return AK_TRUE;
}

int asa_get_bad_block(T_U32 start_block, T_U8 pData[], T_U32 length)
{
    unsigned long page_index, page_offset;
    unsigned long byte_loc, byte_offset;
    unsigned long index_old = 0xFFFFFFFF;
    unsigned char pBuf[2048] = {0};
    unsigned long page_count;
    unsigned long i;//, idex = 0;
   
    
    //check parameter
    if(m_fha_asa_block.asa_head>= ASA_BLOCK_COUNT || AK_NULL == pData)
    {
        return -1;
    }

    page_count = (g_Blocknum - 1) / (g_nPageSize* 8) + 1;
	memset(pBuf, 0, 2048);

    //get all bad blocks
    for(i = 0; i < length; i++)
    {
        page_index = (start_block + i) / (g_nPageSize * 8);
        page_offset = (start_block + i) - page_index * (g_nPageSize * 8);
        
        byte_loc = page_offset / 8;
        byte_offset = 7 - page_offset % 8;

        //read page data if necessary
        if(index_old != page_index)
        {
            index_old = page_index;
            //尝试从安全区中所有块去读取信息
            //gPrtition.Printf("i:%d,%d, %d, %d\r\n", i, page_count, page_index, m_fha_asa_block.asa_count);
            if(-1 == asa_read_page(1+page_count+page_index, pBuf, m_fha_asa_block.asa_count))
            {
                return -1;
            }
            #if 0
            gPrtition.Printf("bad table:\r\n");    
            for(idex = 0; idex < 256; idex++)
            {
                if(idex %16 == 0)
                {
                    gPrtition.Printf("\r\n");
                }
                gPrtition.Printf("%02x ", pBuf[idex]);
            }
            #endif
        }

        //gPrtition.Printf("pBuf:%d, %d, %02x\r\n", byte_loc, byte_offset, pBuf[byte_loc]);
        //update bad block buffer
        if(pBuf[byte_loc] & (1 << byte_offset))
        {
            pData[i/8] |= 1<<(7-i%8);
        }
        else
        {
            pData[i/8] &= ~(1<<(7-i%8));
        }

    }
    #if 0
    gPrtition.Printf("pData bad table:\r\n");    
    for(idex = 0; idex < 256; idex++)
    {
        if(idex %16 == 0)
        {
            gPrtition.Printf("\r\n");
        }
        gPrtition.Printf("%02x ", pBuf[idex]);
    }
    #endif
    return 0;
}


/**
 * @BREIF    set a block to bad block
 * @AUTHOR  Liao Zhijun
 * @DATE     2009-11-23
 * @PARAM   block: which block u want to set
 * @RETURN   AK_TRUE, success, 
                    AK_FALSE, fail
 */
int asa_set_bad_block(unsigned long block)
{
    unsigned char pBuf[2048] = {0};
    unsigned long block_next = 0;
    unsigned long block_written = 0;
    unsigned long i, j;
    unsigned long bad_blocks[ASA_BLOCK_COUNT] = {0};
    unsigned char bb_count = 0;

    //check parameter
    if(m_fha_asa_block.asa_head >= ASA_BLOCK_COUNT)
    {
        return -1;
    }

    //alloc memory
 
    bad_blocks[0] = block;
    bb_count = 1;
    
    block_next = m_fha_asa_block.asa_head;
    for(j = 0; (j < m_fha_asa_block.asa_count) && (block_written < 2); j++)
    {
        unsigned long ret;
        unsigned long spare;

        spare = m_fha_asa_block.write_time;

        //get next block
        block_next = (block_next+1) % m_fha_asa_block.asa_count;
        if(0 == m_fha_asa_block.asa_blocks[block_next])
        {
            continue;
        }

        //避免擦除了安全区所有块
        if(block_next == m_fha_asa_block.asa_head)
        {
            printf("FHA: set bad blk err, asa get only one blk.\r\n");
            
            return -1;
        }

         //erase block
        if(-1 == Prod_NandEraseBlock(m_fha_asa_block.asa_blocks[block_next]))
        {
            goto WRITE_FAIL;
        }
        
        //move pages before the page
        for(i = 0; i < g_nPagePerBlock; i++)
        {
            //除了刚被擦除的块，从所有块中去读取数据
            memset(pBuf, 0, 2048);
            if(-1 == asa_read_page(i, pBuf, m_fha_asa_block.asa_count-1))
            {
                return -1;
            }

            asa_update_page_data(i, pBuf, bad_blocks, bb_count);
            ret = Prod_NandWritePage(m_fha_asa_block.asa_blocks[block_next], i, pBuf, (unsigned char*)&spare, 4);
            if(-1 == ret)
            {
                goto WRITE_FAIL;
            }
        }

        //update global variable
        m_fha_asa_block.asa_head = (unsigned char)block_next;
        
        m_fha_asa_block.write_time++;
        block_written++;

        continue;

WRITE_FAIL:
        Prod_NandEraseBlock(m_fha_asa_block.asa_blocks[block_next]);
        bad_blocks[bb_count++] = m_fha_asa_block.asa_blocks[block_next];
        m_fha_asa_block.asa_blocks[block_next] = 0;

        if (bb_count > 2)
        {
          	 printf("asar@#@");
            while(1);
        }
    }
    
    
    return (block_written > 0) ? 0 : -1;
}



//设置坏块
int  spinand_set_badblock(unsigned long block)
{
    unsigned long byte_loc, byte_offset;
    int ret = -1;
    
    ret = asa_set_bad_block(block);

    if(AK_NULL != m_pBuf_BadBlk)
    {
        byte_loc = block / 8;
        byte_offset = 7 - block % 8;
        m_pBuf_BadBlk[byte_loc] |= 1 << byte_offset;
    }

    return ret;
}



//此接口是有坏块的判断
int  spinand_is_badblock(unsigned long block)
{
    unsigned char  pData[4] = {0};

     if(1 == m_buf_stat && m_pBuf_BadBlk != AK_NULL)
    {
        //gPrtition.Printf("g_nand_phy_info.blk_num:%d\r\n",g_nand_phy_info.blk_num);
        asa_get_bad_block(0, m_pBuf_BadBlk, g_Blocknum);
        m_buf_stat = 2;
    }

    if(m_buf_stat > 1  && m_pBuf_BadBlk != AK_NULL)
    {
        unsigned long byte_loc, byte_offset;

        byte_loc = block / 8;
        byte_offset = 7 - block % 8;

        //gPrtition.Printf("byte_loc:%d, byte_offset:%d, %02x\r\n",byte_loc, byte_offset, m_pBuf_BadBlk[byte_loc]);
        if(m_pBuf_BadBlk[byte_loc] & (1 << byte_offset))
        {
            return 0;
        }
    }
    else
    {
        asa_get_bad_block(block, pData, 1);
        //gPrtition.Printf("block:%d, %02x\r\n",block, pData[0]);
        if ((pData[0] & (1 << 7)) != 0)
        {
            return 0;
        }
    }

    return -1;
}

static void asa_init_repair(unsigned long block_start, unsigned long block_end, unsigned char* pBuf)
{
    unsigned char bad_blocks[ASA_MAX_BLOCK_TRY];
    unsigned long i, j;
    unsigned long spare = m_fha_asa_block.write_time;

    //get bad blocks
    if(asa_get_bad_block(0, bad_blocks, ASA_MAX_BLOCK_TRY) == 0)
    {
        for(i = block_start; (i < block_end) && (m_fha_asa_block.asa_count < ASA_BLOCK_COUNT); i++)
        {
            for(j = 0; j < m_fha_asa_block.asa_count; j++)
            {
            	//printf("m_fha_asa_block.asa_blocks[%d]:%d, i:%d\r\n", j, m_fha_asa_block.asa_blocks[j], i);
                if(m_fha_asa_block.asa_blocks[j] == i)
                {
                    break;
                }
            }
            if(j < m_fha_asa_block.asa_count)
            {
                continue;
            }
            
            if((bad_blocks[i/8] & (1<<(7-(i%8)))) == 0)
            {
               // T_U32 j;
                if (Prod_NandEraseBlock(i) == -1)
                {
                    continue;
                }

                for (j=0; j<g_nPagePerBlock; j++)
                {
                    if (asa_read_page(j, pBuf, m_fha_asa_block.asa_count) == -1)
                    {    
                    	printf("asa_read_page fail, p:%d\n", j);
                        break;
                    }
                    else
                    {
                    	//printf("Prod_NandWritePage:%d, %02x,  %02x,  %02x, %02x, %02x, %02x, %02x, %02x, %d\r\n", i, pBuf[0], pBuf[1], pBuf[2], pBuf[3], pBuf[4], pBuf[5], pBuf[6], pBuf[7], spare);
                        if (Prod_NandWritePage(i, j, pBuf, (unsigned char *)&spare, 4) == -1)
                        {
                        	printf("Prod_NandWritePage fail, b:%d\n", i);
                            break;
                        }    
                    }    
                }
				//printf("repair:%d, %d\n", j, g_nPagePerBlock);
                if (j < g_nPagePerBlock)
                {
                    Prod_NandEraseBlock(i);
                    continue;
                }
                else
                {
                    printf("repair:%d\r\n", i);
                    m_fha_asa_block.asa_blocks[m_fha_asa_block.asa_count++] = (unsigned char)i;
					spare++;
                }    
            }
        }
    }
}    


T_BOOL  spinand_babblock_tbl_init(void)
{
    unsigned long wtime_primary;
    unsigned long block, spare;
    unsigned long block_try, block_get;
    unsigned long block_end = 0;
    unsigned long start_block = 0;
    unsigned char bLastInfo = 0;
    unsigned char bfirstInfo = 1;
	unsigned long m = 0;

    wtime_primary = 0;
    m_fha_asa_block.asa_head = 0;

    block_try = block_get = 0;
    block = 1;
    memset(m_fha_asa_block.asa_blocks, 0, ASA_BLOCK_COUNT);
	//memset(m_aBuffer, 0, 2048);

    block--;//涓轰笅闈?++
    //scan
    while(block_get <= ASA_BLOCK_COUNT && block_try < ASA_MAX_BLOCK_TRY)
    {
        T_ASA_HEAD *pHead = (T_ASA_HEAD *)m_aBuffer;

        block_try++;
        block++;
        m_fha_asa_block.asa_block_cnt = (unsigned char)block + 1;
        
       // printf("block_get:%d, block_try:%d, m_fha_asa_block.asa_block_cnt:%d\r\n", block_get, block_try, m_fha_asa_block.asa_block_cnt);
        //read page 0锛宎sa flag
        if(!FHA_Nand_ReadPage(block, 0, (unsigned char *)m_aBuffer, (unsigned char *)&spare, 4))
        {
            printf("asa_init read block:%d flag fail\r\n", block);
            continue;
        }

        //check head
        if(memcmp(pHead->head_str, m_hinfo_data, HEAD_SIZE) != 0)
        {
            //gFHAf.Printf("asa_init block:%d is not asa block\r\n", block);
            continue;
        }
      //  printf("read asa block:%d\r\n", block);
        //read time
        if(!FHA_Nand_ReadPage(block, g_nPagePerBlock- 1, (unsigned char *)m_aBuffer, (unsigned char *)&spare, 4))
        {
            printf("asa_init read times at block:%d fail\r\n", block);
            continue;
        }

		if(bfirstInfo == 1)
        {
            bfirstInfo = 0;
            start_block = block;
        }

        //spare
        if(0xFFFFFFFF == spare)
        {
            printf("asa_init times to max at block:%d, wtime_primary:%d\r\n", block, wtime_primary);
            continue;
        }

        
       // printf("asa block:%d, spare:%d, block_get:%d, block_try:%d\r\n", block, spare, block_get, block_try);
        block_end = block;
        if(0 == spare)
        {
            //spare
            printf("asa_init times is zero at block:%d, %d\r\n", block, m_fha_asa_block.asa_block_cnt);
            //m_fha_asa_block.asa_block_cnt--; 

            bLastInfo = 1;
            break;
        }

        //update spare
        if(spare > wtime_primary)
        {
            //
            wtime_primary = spare;
            m_fha_asa_block.asa_head = (unsigned char)block_get;
			//printf("init m_fha_asa_block.asa_head:%d\r\n", m_fha_asa_block.asa_head);
        }
        
        m_fha_asa_block.asa_blocks[block_get++] = (unsigned char)block;
        m_fha_asa_block.asa_count = (unsigned char)block_get;

        printf("asa_init block:%d, times:%d\r\n", block, spare);
    }

    //ASA_MAX_BLOCK_TRY
    if(ASA_MAX_BLOCK_TRY == block_try)
    {
        m_fha_asa_block.asa_block_cnt = ASA_MAX_BLOCK_TRY;
    }
    
    if(0 == m_fha_asa_block.asa_blocks[m_fha_asa_block.asa_head])
    {
        printf("asa_init cannot find a fit block\r\n");
        return AK_FALSE;   
    }
    else
    {
        m_fha_asa_block.write_time = wtime_primary+1;
		printf("asa_init primary block:%d, times:%d\r\n", m_fha_asa_block.asa_blocks[m_fha_asa_block.asa_head], m_fha_asa_block.write_time);

        //if asa blocks is not enough, fill with non-badblock
        if(m_fha_asa_block.asa_count < ASA_BLOCK_COUNT)
        {
            printf("m_fha_asa_block.asa_count:%d, start_block:%d, block_end:%d\r\n", m_fha_asa_block.asa_count, start_block, block_end);

        //if asa blocks is not enough, fill with non-badblock
            asa_init_repair(start_block, block_end, (unsigned char *)m_aBuffer);

        }

		m_buf_stat = 1;
		
		for (m=0; m<g_Blocknum; m++)
        {
            if (spinand_is_badblock(m) == 0)
            {
                printf("BB:%d ", m);
            }    
        } 
        printf("\r\n");
    }

       
    return AK_TRUE;
}

T_BOOL asa_read_page(unsigned long page, unsigned char data[], unsigned long count)
{
	T_BOOL ret = AK_FALSE;
    unsigned long spare;
    unsigned long i;
    unsigned long read_index = m_fha_asa_block.asa_head;
    
    //check parameter
    if(page >= g_nPagePerBlock|| m_fha_asa_block.asa_head>= ASA_BLOCK_COUNT || AK_NULL == data)
    {
    	printf("error: read fail, :%d, %d, %d\r\n", page, g_nPagePerBlock, m_fha_asa_block.asa_head);
        return AK_FALSE;
    }

    //read data
    for (i=0; i<count; i++)
    {
    	//gPrtition.Printf("count:%d, %d\r\n", count, i);
        //
        if (0 != m_fha_asa_block.asa_blocks[read_index])
        {
            //printf("read_index:%d, %d, %d\r\n", read_index, m_fha_asa_block.asa_blocks[read_index], page);
            ret = FHA_Nand_ReadPage(m_fha_asa_block.asa_blocks[read_index], page, data, (unsigned char*)&spare, 4);
        }
        
        if (ret == AK_TRUE)
        {
            break;
        }

		//gPrtition.Printf("asa_count:%d, %d\r\n", read_index, m_fha_asa_block.asa_count);
        //
        read_index = (read_index > 0) ? (read_index - 1) : (m_fha_asa_block.asa_count - 1);
    }    
    
    return AK_TRUE;
}


T_BOOL spinand_read_asa_data(unsigned char *data, unsigned long data_len)
{
	unsigned char *file_name = "UPNAME";
    T_ASA_HEAD *pHead = AK_NULL;
    T_ASA_FILE_INFO *pFileInfo = AK_NULL;
    unsigned long i = 0,j = 0;
    unsigned long file_num = 0;
    unsigned char bMatch = 0; //must false
    unsigned long ret= -1;
    unsigned long start_page = 0;
    unsigned long end_page = 0;
	
    unsigned short page_cnt = (unsigned short)(data_len-1)/g_nPageSize + 1;


	//gPrtition.Printf("spinand_read_asa_data, data_len:%d\r\n", data_len);
	//printf("m_fha_asa_block.asa_head:%d\r\n", m_fha_asa_block.asa_head);
	if(m_fha_asa_block.asa_blocks[m_fha_asa_block.asa_head] == 0)
	{
		printf("error: badblock tbl is not init or creat\r\n");
		return AK_FALSE;
	}

    if(AK_NULL == data || m_fha_asa_block.asa_head >= ASA_BLOCK_COUNT || data_len == 0)
    {
    	printf("error: NULL == data or data_len == %d, m_fha_asa_block.asa_head==%d\r\n", data_len, m_fha_asa_block.asa_head);
        return AK_FALSE;
    }

    //
    //read page0 to get file info
    if(!asa_read_page(0, (unsigned char *)m_aBuffer, 2))
    {
    	printf("error: asa_read_page fail\r\n");
        return AK_FALSE;
    }

    //head info
    pHead = (T_ASA_HEAD *)m_aBuffer;

    //asa file info
    pFileInfo = (T_ASA_FILE_INFO *)((unsigned char *)m_aBuffer + sizeof(T_ASA_HEAD) + 2 * sizeof(T_ASA_ITEM));

    file_num = pHead->item_num - 2;

    //search asa file name
    for(i=0; i<file_num; i++)
    {
        bMatch = 1;
        for(j=0; file_name[j] && (pFileInfo->file_name)[j]; j++)
        {
            if(file_name[j] != (pFileInfo->file_name)[j])
            {
                bMatch = 0;
                break;
            }    
        }

        if(file_name[j] != (pFileInfo->file_name)[j])
        {
            bMatch = 0;
        }

        //exist
        if(bMatch)
        {        
            break;
        }    
        else
        {
            //continue to search
            pFileInfo++;
        }
    }

    if(bMatch == 0)
    {
    	printf("no have update partition name \r\n");
        return AK_FALSE;
    }
    else
    {
        start_page = pFileInfo->start_page;
        end_page = pFileInfo->end_page;
		//gPrtition.Printf("start_page:%d, end_page:%d\r\n", start_page, end_page);
        if(page_cnt > (end_page - start_page))
        {
        	printf("error: read data page_cnt too long, %d, %d, %d \r\n", page_cnt, end_page, start_page);
           return AK_FALSE;
        }
        
        
        for(i=0; i < (data_len/g_nPageSize); i++)
        {
            //
            if(!asa_read_page(start_page+i, data+i*g_nPageSize, 2))
            {
            	printf("error: asa_read_page fail\r\n");
                return AK_FALSE;
            }    
        }

        if(0 != data_len%g_nPageSize)
        {
        	//gPrtition.Printf("start_page:%d, i:%d\r\n", start_page, i);
            if(!asa_read_page(start_page+i, (unsigned char *)m_aBuffer, 2))
            {
            	printf("error: asa_read_page2 fail\r\n");
                return AK_FALSE;
            }
            else
            {
                memcpy(data+i*g_nPageSize, (unsigned char *)m_aBuffer, data_len%g_nPageSize);
            }
        }    
    }

   return AK_TRUE;
            
}


static T_BOOL set_asa_update_flag(T_U8 *asa_name_buf)
{
	unsigned char data[2048] = {0};

	memset(data, 0, 2048);
	memcpy(data, asa_name_buf, sizeof(T_PARTITION_INFO));
	if(-1 == spinand_write_asa_data(data, sizeof(T_PARTITION_INFO)))
	{
		printf("update asa updater name error\r\n");
		return AK_FALSE;
	}

	return AK_TRUE;
}



static T_BOOL get_asa_update_flag(T_UPDATE_INFO update_info[])
{
	T_PARTITION_NAME_INFO *partition_info = AK_NULL;
	unsigned char data[64] = {0};
	T_U32 partition_cnt = 0, i = 0;
	
	//读取安全区的分区表名
	if(!spinand_read_asa_data(data, sizeof(T_PARTITION_INFO)))
	{
		//printf("spinand_read_asa_data fail\r\n");
		return AK_FALSE;
	}

	partition_cnt = ((T_U32 *)data)[0];
	//printf("update partition_cnt:%d\r\n", partition_cnt);
	if(PARTITION_CNT < partition_cnt || partition_cnt < 0)
	{
		printf("partition_cnt is error, :%d\r\n", partition_cnt);
		return AK_FALSE;
	}
	
	partition_info = (T_PARTITION_NAME_INFO *)(data + 4);

	for(i = 0; i < partition_cnt; i++)
	{
		if(i == 0)
		{
			memset(update_info[i].partition_name,0, 8);
			memset(update_info[i].partition_name_bak,0, 8);
			memcpy(update_info[i].partition_name, NAND_BOIS_NAME, strlen(NAND_BOIS_NAME));
			memcpy(update_info[i].partition_name_bak, NAND_BOIS_NAME_BAK, strlen(NAND_BOIS_NAME_BAK));
		}
		else if(i == 1)
		{
			memset(update_info[i].partition_name,0, 8);
			memset(update_info[i].partition_name_bak,0, 8);
			memcpy(update_info[i].partition_name, NAND_A_NAME, strlen(NAND_A_NAME));
			memcpy(update_info[i].partition_name_bak, NAND_A_NAME_BAK, strlen(NAND_A_NAME_BAK));
		}
		else if(i == 2)
		{
			memset(update_info[i].partition_name,0, 8);
			memset(update_info[i].partition_name_bak,0, 8);
			memcpy(update_info[i].partition_name, NAND_B_NAME, strlen(NAND_B_NAME));
			memcpy(update_info[i].partition_name_bak, NAND_B_NAME_BAK, strlen(NAND_B_NAME_BAK));
		}

		//printf("partition_info->update_flag:%s\r\n", partition_info->update_flag);
		if(memcmp(partition_info->update_flag, UPDATE_ING, strlen(UPDATE_ING)) == 0)
		{
			update_info[i].update_flag = U_UPDATE_ING;
		}
		else if(memcmp(partition_info->update_flag, UPDATE_END, strlen(UPDATE_END)) == 0)
		{
			update_info[i].update_flag = U_UPDATE_ENG;
		}
		else if(memcmp(partition_info->update_flag, UPDATE_ERR, strlen(UPDATE_ERR)) == 0)
		{
			update_info[i].update_flag = U_UPDATE_ERR;
		}
		else
		{
			update_info[i].update_flag = U_UPDATE_NULL;
		}
		partition_info++;
	}

	for(i = 0; i <partition_cnt; i++)
	{
		//printf("partition_name[%d]:%s,%s, %d\r\n", i , update_info[i].partition_name, update_info[i].partition_name_bak, update_info[i].update_flag);
	}
	
	return AK_TRUE;

}


static T_BOOL spinand_read_partition_data(T_U32 start_page, T_U32 date_len, T_U32 partition_len, T_U8 *pBuf, T_U8 type)
{
	T_U32 nPageCnt = 0, page = 0, i = 0;
	T_U32 page_idex = 0, partition_pagenum = 0, block_idex = 0;

	if(type == PART_FS_TYPE)
	{
		nPageCnt = 	(date_len + g_nPageSize + g_oob_size - 1)/(g_nPageSize + g_oob_size);  
	}
	else
	{
		nPageCnt = 	(date_len + g_nPageSize - 1)/g_nPageSize;  
	}
	
	printf("start_page:%d, nPageCnt:%d, type:%d\r\n", start_page, nPageCnt, type);
	page = 0;
	page_idex = 0;
	partition_pagenum = partition_len/g_nPageSize;
	printf("partition_pagenum:%d\r\n", partition_pagenum);
	//for(page = 0; page < nPageCnt; page++)
	while(1)
	{
		//read data len page num
		if(page_idex == nPageCnt)
		{
			break;
		}
				
		//printf("page:%d\r\n", page);
		//if read page cnt more than partition page num error
		if(page > partition_pagenum)
		{
			printf("error: read page more than partition page num\r\n");
			return AK_FALSE;
		}

		//判断此块是否坏块

		block_idex = (start_page+page)/g_nPagePerBlock;
		//printf("block:%d, %d, %d\r\n", block_idex, start_page, page);
		if(spinand_is_badblock(block_idex) == 0)
		{
			printf("the block %d is bad block\r\n", block_idex);
			page += g_nPagePerBlock; //指向下一个块
		}
		else
		{
			printf("r.");
			for(i = 0; i < g_nPagePerBlock; i++)
			{
				
				if (!FHA_Nand_ReadPage_burn(block_idex, i, pBuf))
				{
					printf("read partitin data fail\r\n");
		    		return AK_FALSE;
				}
				pBuf += g_nPageSize + g_oob_size;
				page ++;
				page_idex++;
				
				//read data len page num
				if(page_idex == nPageCnt)
				{
					break;
				}
			}
		}
	}

	return AK_TRUE;
}


static T_BOOL spinand_write_partition_data(T_U32 start_page, T_U32 date_len, T_U32 partition_len, T_U8 *pBuf, T_U8 type)
{
	T_U32 nPageCnt = 0, page = 0, i =0, buf_idex = 0, j = 0, block_cnt = 0;
	T_U32 page_idex = 0, partition_pagenum = 0, block_idex = 0;
	T_U8 *pPage_buf;
	T_U8 *pcompare_buf;
	T_BOOL check = AK_TRUE;
	

	if(type == PART_FS_TYPE)
	{
		nPageCnt = 	(date_len + g_nPageSize + g_oob_size - 1)/(g_nPageSize + g_oob_size);  
	}
	else
	{
		nPageCnt = 	(date_len + g_nPageSize - 1)/g_nPageSize;  
	}
	
	printf("partition_len:%d, nPageCnt:%d, type:%d\r\n",date_len, nPageCnt, type);	
	page = 0;
	page_idex = 0;
	partition_pagenum = partition_len/g_nPageSize;
	g_continu_badBlocknum = 0;
	printf("partition_len:%d, partition_pagenum:%d\r\n", partition_len, partition_pagenum);	
	//擦完分区的所有块
	while(1)
	{
		//if read page cnt more than partition page num error
		if(page == partition_pagenum)
		{
			break;
		}
		
		block_idex = (start_page+page)/g_nPagePerBlock;
		//printf("block:%d, %d, %d\r\n", block_idex, start_page, page);
		if(spinand_is_badblock(block_idex) == 0){
			printf("the block %d is bad block\r\n", block_idex);
		}
		else
		{
			printf("e.");
			//擦块
			if(Prod_NandEraseBlock(block_idex) == - 1){
				printf("Prod_NandEraseBlock fail block,and set badblock:%d \r\n", block_idex);
				//标志此块为坏块
				spinand_set_badblock(block_idex);
			}
			else{
				block_cnt++;
			}
		}
		page += g_nPagePerBlock; //指向下一个块
	}

	if(block_cnt == 0)
	{
		printf("erro: no any valib blcok\r\n");
		return AK_FALSE;
	}


	page = 0;
	page_idex = 0;
	//printf("partition_pagenum:%d\r\n", partition_pagenum);
	//for(page = 0; page < nPageCnt; page++)
	while(1)
	{
		//read data len page num
		if(page_idex == nPageCnt)
		{
			break;
		}

		//if read page cnt more than partition page num error
		if(page > partition_pagenum)
		{
			printf("error: read page more than partition page num\r\n");
			return AK_FALSE;
		}
		
		//printf("page:%d\r\n", page);

		//判断此块是否坏块

		block_idex = (start_page+page)/g_nPagePerBlock;
		//printf("block:%d, %d, %d\r\n", block_idex, bios_start_page, page);
		if(spinand_is_badblock(block_idex) == 0)
		{
			printf("the block %d is bad block\r\n", block_idex);
			page += g_nPagePerBlock; //指向下一个块
		}
		else
		{
			printf("w.");
			//写块
			buf_idex = page_idex*(g_nPageSize+ g_oob_size);
			pPage_buf = pBuf+buf_idex;
			for(i = 0; i < g_nPagePerBlock; i++)
			{
				if(page_idex == nPageCnt)
				{
					break;
				}
				
				if(!FHA_Nand_WritePage_burn(block_idex, i,  pPage_buf))
				{
					//标志此块为坏块
					spinand_set_badblock(block_idex);
					
					//
					page_idex -= i;
					page += g_nPagePerBlock; //指向下一个块
					printf("FHA_Nand_WritePage_burn fail block:%d, page_idex:%d, i:%d\r\n", block_idex, page_idex, i);
					break;	
				}
				if(check)
				{
					//比较读出来与写下去的数据
					pcompare_buf = (T_U8 *)m_aBuffer;
					if(!FHA_Nand_ReadPage_burn(block_idex, i,  pcompare_buf))
					{
						//标志此块为坏块
						spinand_set_badblock(block_idex);
						
						//
						page_idex -= i;
						page += g_nPagePerBlock; //指向下一个块
						printf("FHA_Nand_ReadPage_burn fail block:%d, page_idex:%d, i:%d\r\n", block_idex, page_idex, i);
						break;	
					}
					
					for(j = 0; j < g_nPageSize+ g_oob_size; j++)
					{
						if(pcompare_buf[j] != pPage_buf[j])
						{
							printf("update date compare fail, page:%d, idex:%d, dst:%02x, src:%02x\r\n", i, j, pcompare_buf[j] != pPage_buf[j]);
							break;
						}
					}

					if(j !=  g_nPageSize+ g_oob_size)
					{
						//标志此块为坏块
						spinand_set_badblock(block_idex);
						
						g_continu_badBlocknum++;
						page_idex -= i;
						page += g_nPagePerBlock; //指向下一个块
						printf("compare data fail  j:%d, g_nPageSize+ g_oob_size:%d\r\n", j, g_nPageSize+ g_oob_size);
						
						if(CONTINU_MAX_BD_NUM <= g_continu_badBlocknum)
						{
							printf("error: continue read data compare fail, set bad block %d more than %d\r\n", g_continu_badBlocknum, CONTINU_MAX_BD_NUM);
							return AK_FALSE;
						}
						
						break;	
					}
				}
				
				pPage_buf += g_nPageSize+ g_oob_size;
				page++;
				page_idex++;
			}
			
		}
	}


	printf("write data finish\r\n");

	return AK_TRUE;
}






static T_BOOL spinand_reconvert(T_UPDATE_INFO *part_info, T_U32* p_nBiosAddress, T_U8 *read_import_part_falg)
{
	T_U8 *pBuf;
	T_U32 r_start_page = 0;
	T_U32 r_date_len = 0; 
	T_U32 r_partition_len = 0;

	T_U32 w_start_page = 0;
	T_U32 w_date_len = 0; 
	T_U32 w_partition_len = 0;


	if(part_info->update_flag == U_UPDATE_ING)//升级标志为正在升级,需要把主分区数据恢复到备份分区上
	{
		r_start_page = part_info->start_page;
		r_date_len = part_info->data_len; 
		r_partition_len = part_info->partitin_len;

		part_info->data_len_bak = part_info->data_len;  //有效的数据长度应是主分区的

		w_start_page = part_info->start_page_bak;
		w_date_len = part_info->data_len_bak;
		w_partition_len = part_info->partitin_len_bak;
	}
	else//升级标志为已升级,需要把备份分区数据更新到主分区上
	{

		part_info->data_len = part_info->data_len_bak;//有效的数据长度应是备份分区的
		
		w_start_page = part_info->start_page;
		w_date_len = part_info->data_len; 
		w_partition_len = part_info->partitin_len;

		r_start_page = part_info->start_page_bak;
		r_date_len = part_info->data_len_bak; 
		r_partition_len = part_info->partitin_len_bak;
	}

	*read_import_part_falg = 0;
	pBuf = (T_U8*)(*p_nBiosAddress);
	//读取分区数据
	if(!spinand_read_partition_data(r_start_page, r_date_len, r_partition_len, pBuf, part_info->partition_type))
	{
		if(part_info->update_flag == U_UPDATE_ING)
		{
			*read_import_part_falg = 1;
		}
		return AK_FALSE;
	}

	pBuf = (T_U8*)(*p_nBiosAddress);
	//写入分区数据
	if(!spinand_write_partition_data(w_start_page, w_date_len, w_partition_len, pBuf, part_info->partition_type))
	{
		if(part_info->update_flag == U_UPDATE_ENG)
		{
			*read_import_part_falg = 1;
		}
		
		return AK_FALSE;
	}

	printf("spinand_reconvert finish\r\n");
	
	return AK_TRUE;
}




static T_BOOL spinand_load_kernel(T_U8 *pBuf, T_U32 start_page, T_U32 file_lenght, T_U32 partition_len)
{
	T_U32 nPageCnt = 0, page = 0, i = 0;
	T_U32 page_idex = 0, partition_pagenum = 0, block_idex = 0;
	
	nPageCnt = 	(file_lenght + g_nPageSize - 1)/g_nPageSize;  
	
	//printf("bios_start_page:%d, nPageCnt:%d\r\n", bios_start_page, nPageCnt);
	page = 0;
	page_idex = 0;
	partition_pagenum = partition_len/g_nPageSize;
	//printf("partition_pagenum:%d\r\n", partition_pagenum);
	//for(page = 0; page < nPageCnt; page++)
	while(1)
	{
		//read data len page num
		if(page_idex == nPageCnt)
		{
			break;
		}

		//if read page cnt more than partition page num error
		if(page > partition_pagenum)
		{
			printf("error: read page more than partition page num\r\n");
			return AK_FALSE;
		}
		
		//printf("page:%d\r\n", page);

		//判断此块是否坏块

		block_idex = (start_page+page)/g_nPagePerBlock;
		//printf("block:%d, %d, %d\r\n", block_idex, bios_start_page, page);
		if(spinand_is_badblock(block_idex) == 0)
		{
			printf("the block %d is bad block\r\n", block_idex);
			page += g_nPagePerBlock; //指向下一个块
		}
		else
		{
			if (!Fwl_Nand_Read_ASAPage(0, start_page+page, pBuf))
			{
				printf("read kernel data fail\r\n");
	    		return AK_FALSE;
			}
			pBuf += g_nPageSize;
			page ++;
			page_idex++;
		}
	}

	
	return AK_TRUE;
}



T_VOID fwl_spinand_load_kernel(T_U32* p_nBiosAddress)
{
	T_U8 *pBuf;
	T_U32 start_page = 0,  file_lenght = 0;
	T_U32 i = 0, partiton_cnt = PARTITION_CNT;
	T_U8 update_asa_flag = 0;
	T_U8 read_import_part_falg = 0;
	T_PARTITION_NAME_INFO *partition_info = AK_NULL;
	T_U8 asa_name_buf[64] = {0};
	T_UPDATE_INFO update_info[PARTITION_CNT] = {0};

	//初始化坏块表
	if(!spinand_babblock_tbl_init())
	{
		printf("error: bad block table not exist,pls check, while(1)\r\n");
		while(1);
	}

	memset(&update_info, 0, PARTITION_CNT*sizeof(T_UPDATE_INFO));
	//获取ASA区的3个分区的升级标志
	if(!get_asa_update_flag(update_info))
	{
		printf("error: read asa update flag fail\r\n");
		while(1);
	}


	//根据获取到的升级标志的分区个数,获取各个分区的信息
	if(!get_partition_table(update_info, p_nBiosAddress))
	{
		printf("error: get partition table fail\r\n");
		while(1);
	}

	memcpy(asa_name_buf, &partiton_cnt, sizeof(T_U32));
	partition_info = (T_PARTITION_NAME_INFO *)(asa_name_buf + 4);
	
	//根据升级标志进行恢复主分区或者备份分区
	for(i = 0; i < PARTITION_CNT; i++)
	{
		memset(partition_info[i].update_flag, 0, 6);
		memcpy(partition_info[i].update_flag, UPDATE_NULL, 1);
		if(update_info[i].update_flag == U_UPDATE_ING)//表示正在升级
		{
			printf("update %s data to %s ,pls wait...\r\n", update_info[i].partition_name, update_info[i].partition_name_bak);
			if(!spinand_reconvert(&update_info[i], p_nBiosAddress, &read_import_part_falg))
			{
				printf("error: spinand_reconvert update_flag =1 fail\r\n");
				memcpy(partition_info[i].update_flag, UPDATE_ERR, strlen(UPDATE_ERR));//
			}
			update_asa_flag = 1;
		}
		else if(update_info[i].update_flag == U_UPDATE_ENG)//表示已升级
		{
			printf("update %s data form %s ,pls wait...\r\n", update_info[i].partition_name, update_info[i].partition_name_bak);
			if(!spinand_reconvert(&update_info[i], p_nBiosAddress, &read_import_part_falg))
			{
				//由于是写主份区,
				if(read_import_part_falg == 1)
				{
					printf("error: write main partition data fail, partition:%d\r\n", i);
					while(1);
				}
				
				//读备份分区失败
				printf("error: spinand_reconvert update_flag =2 fail\r\n");
				memcpy(partition_info[i].update_flag, UPDATE_ERR, strlen(UPDATE_ERR));//
			}
			update_asa_flag = 1;
		}
		else
		{
			continue;//3表示无法再升级 和 0表示没有升级不需侨何处理直接从主分区启动
		}
	}

	
	if(update_asa_flag == 1)
	{
		//更新分区表,由于升级时,需要升级有效数据长度,所以需要更新分区表,是因为有坏块的原因
		if(!set_partition_table(update_info))
		{
			printf("error: set_partition_table fail\r\n");
			while(1);
		}
	
		//更新ASA的升级标志
		printf("update asa flag\r\n");
		if(!set_asa_update_flag(asa_name_buf))
		{
			printf("error: write asa update flag fail\r\n");
			while(1);
		}
	}

	//读取主分区内核,并进行加载
	pBuf = (T_U8*)(*p_nBiosAddress);

	printf("start load kernel....\r\n");
	if(!spinand_load_kernel(pBuf, update_info[0].start_page, update_info[0].data_len, update_info[0].partitin_len))
	{
		 printf("spinand get kernel data fail\r\n");
	  	 while(1);
	}
}


static T_BOOL set_feature(unsigned char addr, unsigned char val)
{
    unsigned char buf[3];

    buf[0] = SPI_NAND_SET_FEATURE;
    buf[1] = addr;
    buf[2] = val;

    if (!spi_master_write(SPI_ID0, buf, 3, AK_TRUE))
    {
        printf("set_feature write fail\r\n");
        return AK_FALSE;
    }
       
    return AK_TRUE;
}


T_BOOL spi_nand_init(T_eSPI_ID spi_id)
{
	T_NAND_PARAM spinand_param;

	T_U8 *p_data;
	T_U32 i =0;
	T_U8 Buffer[2048] = {0};
		
	//set clock to 25M, use mode0, coz some spi flash may have data error in mode3
	spi_ctrl_init(SPI_ID0, SPI_MODE0_V, SPI_MASTER_V, 20*1000*1000);

    if (!set_feature(ADDR_PROTECTION, 0))
    {
        printf("set_feature fail\r\n");
    }


	if(!Fwl_Nand_Read_ASAPage(0, 0, Buffer))
	{
		printf("read page 0 data fail, pls check\r\n");
	    while(1);
	}
	
	spinand_param = *((T_NAND_PARAM *)(&Buffer[512]));

    nRwPage = 1;
    nEBlock = 1; 
	g_nPagePerBlock = spinand_param.page_per_blk;
	g_nPageSize = spinand_param.page_size;
	g_Blocknum = spinand_param.blk_num;
	g_nOobpos = spinand_param.flag & 0xFF; 
	
	printf("g_nPageSize:%d\r\n", g_nPageSize);
	printf("g_nPagePerBlock:%d\r\n", g_nPagePerBlock);
	printf("g_Blocknum:%d\r\n", g_Blocknum);
	printf("g_nOobpos:%d\r\n", g_nOobpos);
}     


T_BOOL spi_nand_erase(unsigned long row)
{
    unsigned char buf1[4];
    T_BOOL ret = AK_FALSE;
    unsigned char status;
    
    if (!spi_IsInit())
    {
        printf("spi_nand_erase spinand not init\r\n");
        return AK_FALSE;
    }

    //write enable
    write_enable_spinand();

    //erase
    buf1[0] = SPI_NAND_BLOCK_ERASE;
    buf1[1] = (row >> 16) & 0xFF;
    buf1[2] = (row >> 8) & 0xFF;
    buf1[3] = (row >> 0) & 0xFF;

    //erase
    if (!spi_master_write(SPI_ID0, buf1, 4, AK_TRUE))
    {
        printf("spi_master_write fail\r\n");
        goto ERASE_EXIT;
    }
    //us_delay(10);
    //check status
    if (wait_cmd_finish(&status))
    {
    	//printf("status:%x\r\n", status);
        if(!(status & STATUS_E_FAIL))
        {
            ret = AK_TRUE;
        }
        else
        {
            printf("wait_cmd_finish fail status == STATUS_E_FAIL\r\n");
        }
    }
    
ERASE_EXIT:    
    
    return ret;
}


int Erase_NBlock(unsigned long nBlock, unsigned long nBlockCnt)
{
    unsigned long i = 0, j = 0;

    if (0 == nBlockCnt)
    {
        printf("Erase_NBlock  fail nBlockCnt == 0\n");
        return -1;
    }
    
    nBlockCnt = nBlockCnt + nBlock;
    for (i = nBlock; i < nBlockCnt; i++)
    {
       // pp_printf("Erase_NBlock:%d, %d\n",i, g_nPagePerBlock);
        if (!spi_nand_erase(i * g_nPagePerBlock))
        {
            printf("spi_nand_erase fail nBlock:%d\n",i);
            return -1;
        }
    }
	
    return 0;
}


int spinand_erase(unsigned long chip, unsigned long block)
{
	return Erase_NBlock(block, 1);
}

int Prod_NandEraseBlock(unsigned long block_index)
{
   // printf("block_index:%d\r\n",block_index);
    return spinand_erase(0, block_index);       
}



int Prod_NandWritePage(unsigned long block_index, unsigned long page, unsigned char data[], unsigned char spare[],  unsigned long spare_len)
{
    unsigned long page_addr;
    page_addr = block_index * g_nPagePerBlock + page;
   
    return FHA_Nand_WritePage(0, page_addr,  data, spare, spare_len);
}



static void asa_update_page_data(unsigned long page, unsigned char data[], unsigned long bad_blocks[], unsigned long bb_count)
{
    unsigned long page_count = 0;
    unsigned long page_index = 0, page_offset = 0;
    unsigned long byte_loc = 0, byte_offset = 0;
    unsigned char i = 0;
    
    page_count = (g_Blocknum- 1) / (g_nPageSize * 8) + 1;

    for(i = 0; i < bb_count; i++)
    {
        page_index = bad_blocks[i] / (g_nPageSize * 8);
        page_offset = bad_blocks[i] - page_index * (g_nPageSize * 8);

        if(page != 1 + page_count + page_index)//the previous pages(1 ~ page_count ) stores the bitmap of initial bad block
        {
            continue;
        }

        byte_loc = page_offset / 8;
        byte_offset = 7 - page_offset % 8;
        
        data[byte_loc] |= (1 << byte_offset);
    }
}



int spinand_write_block(T_ASA_HEAD* pHead, T_ASA_FILE_INFO *pFileInfo, unsigned long fileInfoOffset, const unsigned char* dataBuf)
{
    unsigned long block_next = 0;
    unsigned long block_written = 0;
    unsigned long i, j;
    unsigned long bad_blocks[ASA_BLOCK_COUNT] = {0};
    unsigned char bb_count = 0;
    unsigned char *pTmp = AK_NULL;
	unsigned char pTmpBuf[4096] ={0};
            
    
    block_next = m_fha_asa_block.asa_head;
 
    for(j= 0; j < m_fha_asa_block.asa_count; j++)
    {
        unsigned long ret;
        unsigned long spare;
        
        spare = m_fha_asa_block.write_time;

        //get next block
        block_next = (block_next+1) % m_fha_asa_block.asa_count;

		//printf("block_next:%d, %d, %d\r\n", block_next, m_fha_asa_block.asa_blocks[block_next], spare);
        if(0 == m_fha_asa_block.asa_blocks[block_next])
        {
            continue;
        }

		//printf("m_fha_asa_block.asa_head:%d\r\n", m_fha_asa_block.asa_head);
        //避免擦除了安全区所有块
        if(block_next == m_fha_asa_block.asa_head)
        {
        	printf("error: only one asa blcok\r\n");
            return -1;
        }

        //erase block
        if(-1 == Prod_NandEraseBlock(m_fha_asa_block.asa_blocks[block_next]))
        {
        	printf("Prod_NandEraseBlock %d fail\r\n", m_fha_asa_block.asa_blocks[block_next]);
            goto WRITE_FAIL;
        }


		//printf("start_page:%d, end_page:%d info_end:%d\r\n", pFileInfo->start_page, pFileInfo->end_page, pHead->info_end);
        //move page
        for(i = 0; i < g_nPagePerBlock; i++)
        {
        	//gPrtition.Printf("page:%d\r\n", i);
            //没有写过的页，不需要进行读写
            if(i > pHead->info_end && i < g_nPagePerBlock - 1)
            {
                continue;
            }

			//printf("i:%d, pFileInfo->start_page:%d, pFileInfo->end_page:%d\r\n",i, pFileInfo->start_page, pFileInfo->end_page);
            if(i<pFileInfo->start_page|| i>=pFileInfo->end_page)
            {
                //写安全区文件只尝试两个块的操作，防止破坏安全区
                if(-1 == asa_read_page(i, pTmpBuf, 2))
                {
                	printf("error: spinand_write_block asa_read_page fail\r\n");
                    return -1;
                }

                
                if(0 == i)
                {
                    memcpy(pTmpBuf, pHead, sizeof(T_ASA_HEAD));
                    memcpy(pTmpBuf + fileInfoOffset, pFileInfo, sizeof(T_ASA_FILE_INFO));
                }
                
                asa_update_page_data(i, pTmpBuf, bad_blocks, bb_count);

                pTmp = pTmpBuf;
            }
            else
            {
                pTmp = (unsigned char*)(dataBuf + (i - pFileInfo->start_page) * g_nPageSize);
            }

            ret = Prod_NandWritePage(m_fha_asa_block.asa_blocks[block_next], i, pTmp, (unsigned char *)&spare, 4);
            if(-1 == ret)
            {
            	printf("error: Prod_NandWritePage fail\r\n");
                goto WRITE_FAIL;
            }
        }

        //update global variable
        m_fha_asa_block.asa_head = (unsigned char)block_next;
        
        m_fha_asa_block.write_time++;
        block_written++;

        continue;

		//gPrtition.Printf("e block_next:%d, %d\r\n",block_next,  m_fha_asa_block.asa_blocks[block_next]);
WRITE_FAIL:
        Prod_NandEraseBlock(m_fha_asa_block.asa_blocks[block_next]);
        bad_blocks[bb_count++] = m_fha_asa_block.asa_blocks[block_next];
        //若操写失败，则把m_asa_blocks中对应块置为0，避免再去读
        m_fha_asa_block.asa_blocks[block_next] = 0;

		//gPrtition.Printf("e bb_count:%d\r\n",bb_count);

        if (bb_count > 1)
        {
            printf("error:spinand_write_block fail\r\n");
            return -1;
        }
    }
	
    return (block_written > 0) ? 0 : -1;

}


int spinand_write_asa_data(unsigned char *data, unsigned long data_len)
{
	unsigned char *file_name = "UPNAME";
    T_ASA_HEAD HeadInfo;
    T_ASA_FILE_INFO *pFileInfo = AK_NULL;
    T_ASA_FILE_INFO newFileInfo;
    unsigned long file_info_offset = 0;
    unsigned long i = 0,j = 0;
    unsigned long file_num = 0;
    unsigned char bMatch = 0; //must false
    unsigned char pBuf[4096] ={0};
    
    unsigned short page_cnt = (unsigned short)(data_len-1)/g_nPageSize + 1;


	//gPrtition.Printf("page_cnt:%d, data_len:%d\r\n", page_cnt, data_len);

	if(m_fha_asa_block.asa_blocks[m_fha_asa_block.asa_head] == 0)
	{
		printf("error: badblock tbl is not init or creat\r\n");
		return -1;
	}
    
    if(AK_NULL == data || m_fha_asa_block.asa_head >= ASA_BLOCK_COUNT || data_len == 0)
    {
    	printf("error: data = null or data_len == %d, m_fha_asa_block.asa_head = %d\r\n", data_len, m_fha_asa_block.asa_head);
        return -1;
    }
    
    //read page0 to get file info
    //安全区文件最初始时只写两个块，也只尝试去读两个块
    if(asa_read_page(0, pBuf, 2) == - 1)
    {
    	printf("error: asa_read_page fail\r\n");
        goto EXIT;
    }

    //head info
    memcpy(&HeadInfo, pBuf, sizeof(T_ASA_HEAD));

    file_info_offset = sizeof(T_ASA_HEAD) + 2 * sizeof(T_ASA_ITEM);

	printf("file_info_offset:%d\r\n", file_info_offset);

    //asa file info
    pFileInfo = (T_ASA_FILE_INFO *)(pBuf + file_info_offset);

    file_num = HeadInfo.item_num - 2;


	//printf("file_num:%d\r\n", file_num);


	//由于目前只需要记录升级名的一个数据暂时不需要区分哪个所以下面代码先注掉
	#if 1
    //search asa file name
    for(i=0; i<file_num; i++)
    {
        bMatch = 1;
        for(j=0; file_name[j] && (pFileInfo->file_name)[j]; j++)
        {
            if(file_name[j] != (pFileInfo->file_name)[j])
            {
                bMatch = 0;
                break;
            }    
        }

        if(file_name[j] != (pFileInfo->file_name)[j])
        {
            bMatch = 0;
        }

        //exist
        if(bMatch == 1)
        {
            //overflow page count
            if(page_cnt > (pFileInfo->end_page - pFileInfo->start_page))
            {
            	printf("error: page_cnt is more than src page num:%d, %d, %d \r\n",page_cnt, pFileInfo->end_page, pFileInfo->start_page);
                goto EXIT;
            }
            else
            {
                break;
            }
        }    
        else
        {
            //continue to search
            pFileInfo++;
            file_info_offset += sizeof(T_ASA_FILE_INFO);
        }
    }
	#endif

    if(bMatch == 0)
    {
        //new asa file
        pFileInfo->start_page = HeadInfo.info_end;
        pFileInfo->end_page = pFileInfo->start_page + page_cnt;
        memcpy(pFileInfo->file_name, file_name, 8);

        //whether excess block size
        if(pFileInfo->end_page > g_nPagePerBlock)
        {
        	printf("error: data page is more than PagePerBlock:%d, %d\r\n",pFileInfo->end_page, g_nPagePerBlock);
            goto EXIT;
        }

        //chenge head info if new file
        HeadInfo.item_num++;
        HeadInfo.info_end = pFileInfo->end_page;
    }
	
	pFileInfo->file_length = data_len;
    memcpy(&newFileInfo, pFileInfo, sizeof(T_ASA_FILE_INFO));


    return spinand_write_block(&HeadInfo, &newFileInfo, file_info_offset, data);

        
EXIT:

    return -1;

}


static T_BOOL get_test_program_info(T_U32 *start_page, T_U32 *file_lenght, T_U32 *partition_len,T_U32 *p_nBiosAddress)
{
	T_PARTITION_TABLE_INFO *partition;
	T_U32 nFileCnt;
	T_U32 i = 0, j = 0;
    T_U32 offset = 0;
	T_U32 temp = 0;
	T_U32 partition_cnt = 0;
	T_U8 bios_flag = 0;
	T_U8 fs_flag[3] = {0};
	T_U8 fs_bak_flag[3] = {0};
	T_U8 partition_name[8] = {0};
	T_U8 reconvert_table_flag = 0;

	
	memset(m_partiton_buf, 0, sizeof(m_partiton_buf));
	if(!Fwl_Nand_Read_ASAPage(0, 0, (T_U8*)m_partiton_buf))
	{        
	    return AK_FALSE;
	}

	offset = SPINAND_PARAM_OFFSET;

    // unsigned char   row_cycle;//
    // unsigned char   delay_cnt;//
    //SPINAND
    //

	temp = offset + 17;
	g_startblock = ((T_U8*)m_partiton_buf)[temp];
	temp = offset + 18;
	g_startblock_backup =  ((T_U8*)m_partiton_buf)[temp];
	//printf("startblock:%d\n", startblock);
	//printf("startblock_backup:%d\n", startblock_backup);

	if (!Fwl_Nand_Read_ASAPage(0, g_startblock*g_nPagePerBlock, (T_U8*)m_partiton_buf))
	{
		printf("read the partition backup block:%d\r\n", g_startblock_backup);
		if (!Fwl_Nand_Read_ASAPage(0, g_startblock_backup*g_nPagePerBlock, (T_U8*)m_partiton_buf))
		{
			printf("read the partition backup block fail:%d\r\n", g_startblock_backup);
	    	return AK_FALSE;
		}
		reconvert_table_flag = 1;
	}
	
	nFileCnt = m_partiton_buf[0];	
	printf("nFileCnt:%d\r\n", nFileCnt);

	if(nFileCnt > g_nPageSize/sizeof(T_PARTITION_TABLE_INFO) || nFileCnt <= 0)
	{
		printf("nFileCnt %d error\r\n", nFileCnt);
		
		printf("read the partition backup block:%d\r\n", g_startblock_backup);
		if (!Fwl_Nand_Read_ASAPage(0, g_startblock_backup*g_nPagePerBlock, (T_U8*)m_partiton_buf))
		{
			printf("read the partition backup block 11 fail:%d\n",g_startblock_backup);
	    	return AK_FALSE;
		}
		nFileCnt = m_partiton_buf[0];	
		printf("nFileCnt:%d\r\n", nFileCnt);


		if(nFileCnt > g_nPageSize/sizeof(T_PARTITION_TABLE_INFO) || nFileCnt <= 0)
		{
			 printf("nFileCnt %d error\r\n", nFileCnt);
			 return AK_FALSE;
		}

		reconvert_table_flag = 1;
		
	}

	if(reconvert_table_flag == 1)
	{
		//恢复原分区表数据
		if(Prod_NandEraseBlock(g_startblock) == - 1){
			printf("revonvert partition table erase fail, g_startblock:%d \r\n", g_startblock);
		}
		if(!FHA_Nand_WritePage(g_startblock, 0,  (T_U8 *)m_partiton_buf, AK_NULL, 0))
		{
			printf(" revonvert partition table fail,g_startblock:%d\r\n", g_startblock);
		}
	}


	partition = (T_PARTITION_TABLE_INFO *)(&m_partiton_buf[1]);
	for (i = 0; i < nFileCnt; i++) 
	{
		//printf("name:%s, %s\r\n", partition->partition_info.name, update_info[j].partition_name);

		memset(partition_name, 0, 8);
		memcpy(partition_name, partition->partition_info.name, 6);
		
	    if (0 == cmp(NAND_TEST_NAME, (T_U8 *)(partition_name), strlen(NAND_TEST_NAME))
			&& strlen(NAND_TEST_NAME) == strlen(partition_name))
		{
			*start_page = partition->partition_info.start_pos/g_nPageSize;
			*file_lenght = partition->ex_partition_info.parameter1;
			*partition_len = partition->partition_info.ksize*1024;
			*p_nBiosAddress = partition->ex_partition_info.parameter2;

			printf("partition_name:%s, start_page:%d, file_length:%d, partition_len:%d, p_nBiosAddress:%x\r\n", partition_name, *start_page, *file_lenght,*partition_len, *p_nBiosAddress);
			break;
	    }
	    else
	    {
			//printf("%s:%s\n", m_aName_spinand, (T_U8 *)(partition->partition_info.name));
	    }
		partition++;
	}

	//printf("read partition table finish\r\n");
	if (i == nFileCnt)
    {
    	printf("no find %s\r\n", NAND_TEST_NAME);
        return AK_FALSE;
    }
	
	return AK_TRUE;
}



T_VOID fwl_spinand_load_test_program(T_U32* p_nBiosAddress)
{
	T_U32 start_page = 0;
	T_U32 file_lenght = 0;
	T_U32 partition_len = 0;
	T_U8 *pBuf;

	//初始化坏块表
	if(!spinand_babblock_tbl_init())
	{
		printf("error: bad block table not exist,pls check, while(1)\r\n");
		while(1);
	}

	
	//读取
	if(!get_test_program_info(&start_page, &file_lenght, &partition_len, p_nBiosAddress))
	{
		printf("get %s info fail\r\n", NAND_TEST_NAME);
		while(1);
	}

	pBuf = (T_U8*)(*p_nBiosAddress);
	if(!spinand_load_kernel(pBuf, start_page, file_lenght, partition_len))
	{
		printf("spinand get %s data fail\r\n", NAND_TEST_NAME);
	  	while(1);
	}
	
}


  


