/**
 * @file spi_flash.c
 * @brief spiflash driver source file
 * Copyright (C) 2004 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author LiaoZhijun
 * @date 2010-05-27
 * @version 1.0
 */

 #if (DRV_SUPPORT_SPI_NAND > 0)
 
#include "anyka_types.h"
#include "drv_api.h"
#include "hal_spinand.h"


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

#define FEATURE_OTP_EN  (1<<6)
#define FEATURE_ECC_EN  (1<<4)  //(1<<5)

typedef struct
{
    bool bInit;

    T_eSPI_ID spi_id;

    T_eSPI_BUS bus_wight;

    T_SPI_NAND_PARAM param; 

    unsigned char QEFlag;
    unsigned char read_Flag;  //0表示先发地址，再发dummy, 1表示相反
}T_SPI_NAND;

static T_SPI_NAND m_spinand = {0};
static bool get_feature(unsigned char addr, unsigned char *val);
static bool set_feature(unsigned char addr, unsigned char val);
static bool spi_nand_set_QE(void);
static bool wait_cmd_finish(unsigned char *status);
static bool get_feature(unsigned char addr, unsigned char *val);
static bool set_feature(unsigned char addr, unsigned char val);
static bool spi_nand_set_QE(void);
static bool wait_cmd_finish(unsigned char *status);

/**
 * @brief spi nand init
 * 
 * @author liao_zhijun
 * @date 2014-1-2
 * @param spi_id [in]  spi id, can be spi0 or spi1
 * @param bus_wight [in] select bus width
 * @param clk [in] spi clock
 * @return T_BOOL
 */
bool spi_nand_init(T_eSPI_ID spi_id, T_eSPI_BUS bus_wight, unsigned long clk)
{
    //set clock to 25M, use mode0, coz some spi flash may have data error in mode3
    if(!spi_init(spi_id, SPI_MODE0, SPI_MASTER, clk))
    {
        akprintf(C1, M_DRVSYS, "spi_nand_init fail\r\n");
        return false;
    }
    
    m_spinand.spi_id = spi_id;
    m_spinand.bus_wight = bus_wight;
    m_spinand.bInit = true;
  
    //spi_nand_reset();

    us_delay(10);

    spi_set_protect(m_spinand.spi_id, SPI_BUS1);
    
//    spi_nand_print_reg();

    //clr protection
    if (!set_feature(ADDR_PROTECTION, 0))
    {
        akprintf(C1, M_DRVSYS, "set_feature fail\r\n");
    }
    
    if (m_spinand.QEFlag != 0)   //当QEFlag为0时即不需要设QE就可用SPI 4线；
    {
        if(SPI_BUS4 == bus_wight)
        {
            if (!spi_nand_set_QE())
            {
                akprintf(C1, M_DRVSYS, "spi_nand_set_QE fail\r\n");
            }
        }
    }
    
    spi_set_unprotect(m_spinand.spi_id, SPI_BUS1);

    return true;
}
/**
 * @brief get spi nand id
 * 
 * @author liao_zhijun
 * @date 2014-1-2
 * @return T_U32
 * @retval T_U32 spiflash id
 */
unsigned long spi_nand_getid(void)
{
    unsigned char buf[2];
    unsigned long flash_id = 0;

    if (!m_spinand.bInit)
    {
        akprintf(C1, M_DRVSYS, "spinand not init\r\n");
        return 0;
    }

    buf[0] = SPI_NAND_READ_ID;
    buf[1] = 0x0;

    spi_set_protect(m_spinand.spi_id, SPI_BUS1);
   
    if (!spi_master_write(m_spinand.spi_id, buf, 2, false))
    {
        akprintf(C1, M_DRVSYS, "spi_nand_getid write fail\r\n");
    }
    if (!spi_master_read(m_spinand.spi_id, (unsigned char *)(&flash_id), 4, true))
    {
        akprintf(C1, M_DRVSYS, "spi_nand_getid read fail\r\n");
    }

    spi_set_unprotect(m_spinand.spi_id, SPI_BUS1);

    return flash_id;
}

static bool spi_nand_set_QE(void)
{
    unsigned char status;

    if(!get_feature(ADDR_FEATURE, &status))
    {
        akprintf(C1, M_DRVSYS, "get_feature 00 fail\r\n");
        return false;
    }

    status |= (1 << 0);

    if(!set_feature(ADDR_FEATURE, status))
    {
        akprintf(C1, M_DRVSYS, "set_feature fail\r\n");
        return false;
    }

    //confirm
    status = 0;
    if (!get_feature(ADDR_FEATURE, &status))
    {
        akprintf(C1, M_DRVSYS, "get_feature 01 fail\r\n");
        return false;
    }
    if (status & 0x1)
    {
        return true;
    }
    else
    {
        akprintf(C1, M_DRVSYS, "spi_nand_set_QE fail\r\n");
        return false;
    }
}
bool spi_nand_set_param(T_SPI_NAND_PARAM *param)
{   
    unsigned long chip_id;
	
    m_spinand.spi_id = param->spi_id;
    m_spinand.bus_wight= param->SpiBusWidth;
    m_spinand.param.clock = param->clock;
    m_spinand.param.flag = param->flag;
    m_spinand.QEFlag = (param->flag>>8)&0x01;
    m_spinand.read_Flag = (param->flag>>9)&0x01;

    if (!spi_nand_init(m_spinand.spi_id, m_spinand.bus_wight, m_spinand.param.clock))
    {
        akprintf(C1, M_DRVSYS, "spi_nand_set_param spinand init fail\r\n");  
        return false;
    }

    return true;
}

bool ecc_ctl(bool bEnable)
{
    unsigned char feature;

    if(!get_feature(ADDR_FEATURE, &feature))
    {
        return false;
    }

    if(bEnable)
        feature |= FEATURE_ECC_EN;
    else
        feature &= ~FEATURE_ECC_EN;

    if(!set_feature(ADDR_FEATURE, feature))
    {
        return false;
    }

    return true;
}

bool spi_nand_read_noecc(unsigned long row, unsigned long column, unsigned char *data, unsigned long len)
{
    unsigned char buf[4];
    unsigned long count = len;
    unsigned char status;
    bool ret = false;
    
    if (!m_spinand.bInit)
    {
        akprintf(C1, M_DRVSYS, "spi_nand_read_noecc spinand not init\r\n");  
        return false;
    }

    spi_set_protect(m_spinand.spi_id, m_spinand.bus_wight);

    //set to no ecc
    if (!ecc_ctl(false))
    {
        akprintf(C1, M_DRVSYS, "ecc_ctl fail\r\n");  
        spi_set_unprotect(m_spinand.spi_id, m_spinand.bus_wight);
        return false;
    }

    //page read to cache
    buf[0] = SPI_NAND_PAGE_READ;
    buf[1] = (row >> 16) & 0xFF;
    buf[2] = (row >> 8) & 0xFF;
    buf[3] = (row >> 0) & 0xFF;

    if (!spi_master_write(m_spinand.spi_id, buf, 4, true))
    {
        akprintf(C1, M_DRVSYS, "spi_nand_read_noecc write fail\r\n");
        goto READ_EXIT;
    }

    //check status
    if (!wait_cmd_finish(&status))
    {
        akprintf(C1, M_DRVSYS, "wait_cmd_finish fail\r\n");
        goto READ_EXIT;
    }

    //read from cache

    buf[0] = (SPI_BUS4 == m_spinand.bus_wight) ? SPI_NAND_READ_CACHE_X4 : SPI_NAND_READ_CACHE; 
    if(m_spinand.read_Flag == 0)
    { 
        buf[1] = (column >> 8) & 0xF;
        buf[2] = column & 0xFF;
        buf[3] = 0x0;
    }
    else
    {
        buf[1] = 0x0;
        buf[2] = (column >> 8) & 0xF;
        buf[3] = column & 0xFF;  
    }

    if (!spi_master_write(m_spinand.spi_id, buf, 4, false))
    {
        akprintf(C1, M_DRVSYS, "spi_nand_read_noecc write 02 fail\r\n");
        goto READ_EXIT;
    }

    spi_data_mode(m_spinand.spi_id, m_spinand.bus_wight);

    if (spi_master_read(m_spinand.spi_id, data, count, true))
    {
        ret = true;
    }

    spi_data_mode(m_spinand.spi_id, SPI_BUS1);

READ_EXIT:

    if (!ecc_ctl(true))
    {
        akprintf(C1, M_DRVSYS, "spi_nand_read_noecc ecc_ctl fail\r\n");
        ret = false;
    }

    spi_set_unprotect(m_spinand.spi_id, m_spinand.bus_wight);
    return ret;
}
//读取第row页内容到cache中
bool spi_nand_readto_cache(void)
{
    unsigned char buf[4];
    unsigned char status;
    bool ret = true;
    unsigned long row=0; 
    if(!m_spinand.bInit)
    {
        akprintf(C1, M_DRVSYS, "spi_nand_readto_cache spinand not init\r\n");
        return false;
    }

    spi_set_protect(m_spinand.spi_id, m_spinand.bus_wight);

    //page read to cache
    buf[0] = SPI_NAND_PAGE_READ;
    buf[1] = (row >> 16) & 0xFF;
    buf[2] = (row >> 8) & 0xFF;
    buf[3] = (row >> 0) & 0xFF;

    if(!spi_master_write(m_spinand.spi_id, buf, 4, true))
    {
        ret = false;
        goto READ_EXIT;
    }

    //check status
    if(wait_cmd_finish(&status))
    {
        unsigned char ecc = (status >> 4) & 0x3;

        if(2 == ecc)
        {
            akprintf(C1, M_DRVSYS, "wait_cmd_finish ecc == 2\r\n");
            ret = false;
            goto READ_EXIT;
        }    
    }   
READ_EXIT:    
    spi_set_unprotect(m_spinand.spi_id, m_spinand.bus_wight);

    return ret;
}

static bool set_feature(unsigned char addr, unsigned char val)
{
    unsigned char buf[3];

    buf[0] = SPI_NAND_SET_FEATURE;
    buf[1] = addr;
    buf[2] = val;

    if (!spi_master_write(m_spinand.spi_id, buf, 3, true))
    {
        akprintf(C1, M_DRVSYS, "set_feature write fail\r\n");
        return false;
    }
       
    return true;
}
static void write_enable(void)
{
    unsigned char buf1[1];
    
    //write enable
    buf1[0] = SPI_NAND_WRITE_ENABLE;
    if (!spi_master_write(m_spinand.spi_id, buf1, 1, true))
    {
        akprintf(C1, M_DRVSYS, "write_enable write fail\r\n");
        return;
    }
}

//addr 
static bool get_feature(unsigned char addr, unsigned char *val)
{
    unsigned char buf[2];

    buf[0] = SPI_NAND_GET_FEATURE;
    buf[1] = addr;

    if (!spi_master_write(m_spinand.spi_id, buf, 2, false))
    {
        akprintf(C1, M_DRVSYS, "get_feature write fail\r\n");
        return false;
    }

    if (!spi_master_read(m_spinand.spi_id, val, 1, true))
    {
        akprintf(C1, M_DRVSYS, "get_feature read fail\r\n");
        return false;
    }
    return true;
}
static bool wait_cmd_finish(unsigned char *status)
{
    unsigned long timeout = 0;
    
    while(timeout++ < 1000000)
    {
        if(get_feature(ADDR_STATUS, status))
        {
           // akprintf(C1, M_DRVSYS, "status:%d, %d\r\n",  *status, STATUS_E_FAIL);
            if(!(*status&STATUS_OIP))
                return true;
        }
        
    }
    akprintf(C1, M_DRVSYS, "wait_cmd_finish fail\r\n");
    return false;
}

/**
 * @brief read data from one page of spi nand
 * 
 * @author liao_zhijun
 * @date 2014-1-2
 * @param row [in]  row address
 * @param column [in]  column address
 * @param data [in]  data buffer to be read
 * @param len [in]  data  length
 * @return T_BOOL
 */
bool spi_nand_read(unsigned long row, unsigned long column, unsigned char *data, unsigned long len)
{
    unsigned char buf[4];
    unsigned long count = len;
    unsigned long cnt_64, cnt_remain;
    unsigned char status;
    bool ret = false;
    
    if (!m_spinand.bInit || 0 == len)
    {
        akprintf(C1, M_DRVSYS, "spi_nand_read spinand not init or len == 0\r\n");
        return false;
    }

    spi_set_protect(m_spinand.spi_id, m_spinand.bus_wight);

    //page read to cache
    buf[0] = SPI_NAND_PAGE_READ;
    buf[1] = (row >> 16) & 0xFF;
    buf[2] = (row >> 8) & 0xFF;
    buf[3] = row & 0xFF;

    if (!spi_master_write(m_spinand.spi_id, buf, 4, true))
    {
        akprintf(C1, M_DRVSYS, "spi_nand_read write fail\r\n");
        goto READ_EXIT;
    }

    //check status
    if (wait_cmd_finish(&status))
    {
        unsigned char ecc = (status >> 4) & 0x3;
        //akprintf(C1, M_DRVSYS, "status:%d, ecc:%d\r\n", status, ecc);
        if(2 == ecc)    //bit error and not corrected
        {
            akprintf(C1, M_DRVSYS, "spi_nand_read ECC == 2 error \r\n");
            goto READ_EXIT;
        }
    }
    else
    {
        goto READ_EXIT;
    }

    //read from cache
    buf[0] = (SPI_BUS4 == m_spinand.bus_wight) ? SPI_NAND_READ_CACHE_X4 : SPI_NAND_READ_CACHE;   
    if(m_spinand.read_Flag == 0)
    {
        buf[1] = (column >> 8) & 0xF;
        buf[2] = column & 0xFF;
        buf[3] = 0x0;
    }
    else
    {
        buf[1] = 0x0;
        buf[2] = (column >> 8) & 0xF;
        buf[3] = column & 0xFF;  
    }
    

    if (!spi_master_write(m_spinand.spi_id, buf, 4, false))
    {
        akprintf(C1, M_DRVSYS, "spi_nand_read 01 write fail \r\n");
        goto READ_EXIT;
    }

    if (!spi_data_mode(m_spinand.spi_id, m_spinand.bus_wight))
    {
        akprintf(C1, M_DRVSYS, "spi_data_mode fail \r\n");
    }

    cnt_remain = count & 0x3F;
    cnt_64 = count - cnt_remain;
    //akprintf(C1, M_DRVSYS, "cnt_remain:%d, cnt_64:%d, count:%d \r\n", cnt_remain, cnt_64, count);
    if(cnt_remain > 0)
    {
        if (!spi_master_read(m_spinand.spi_id, data, cnt_64, false))
        {
            akprintf(C1, M_DRVSYS, "spi_master_read fail \r\n");
            goto READ_EXIT;
        }

        if (spi_master_read(m_spinand.spi_id, data+cnt_64, cnt_remain, true))
        {
            ret = true;
        }        
    }
    else
    {
        if (spi_master_read(m_spinand.spi_id, data, count, true))
        {
            ret = true;
        }
    }

    if (!spi_data_mode(m_spinand.spi_id, SPI_BUS1))
    {
        akprintf(C1, M_DRVSYS, "spi_data_mode fail \r\n");
    }

READ_EXIT:
    spi_set_unprotect(m_spinand.spi_id, m_spinand.bus_wight);

    return ret;
}


/**
 * @brief write data to one page of spi nand
 * 
 * @author liao_zhijun
 * @date 2014-1-2
 * @param row [in]  row address
 * @param column [in]  column address
 * @param data [in]  data buffer to be write
 * @param len [in]  data  length
 * @param oob [in]  oob buffer to be write
 * @param oobLen [in]  oob  length
 * @param g_nOobpos [in]  oob  offset
 * @return T_BOOL
 */

//此函数的修改点：
//1.修改SPI NAND的写接口函数，以支持5F1GQ4UBYIG，向下支持A版本的该款flash；
//2.修改驱动库版本号为：DrvLib V2.4.03。

//由于在调试过程中，此函数会把出对出厂坏块的标志进行修改，
//同时之前徐总那边查过一个问题需要data和spare区不能分区操作的问题，
//针对上面的问题，进行注掉此函数，同时创建另一个函数
#if 0
bool spi_nand_write_byoob(unsigned long row, unsigned long column, unsigned char *data, unsigned long len, unsigned char *oob, unsigned long ooblen, unsigned long oobpos)
{
    unsigned char buf[4];
    unsigned long count = len;
    unsigned char status;
    bool ret = false;
        
    if (!m_spinand.bInit || 0 == len)
    {
        akprintf(C1, M_DRVSYS, "spi_nand_write spinand not init\r\n");
        return false;
    }

    spi_set_protect(m_spinand.spi_id, m_spinand.bus_wight);

    //write enable
    write_enable();

    //program load
    buf[0] = (SPI_BUS4 == m_spinand.bus_wight) ? SPI_NAND_PROGRAM_LOAD_X4 : SPI_NAND_PROGRAM_LOAD;    
    buf[1] = (column >> 8) & 0x0F;
    buf[2] = column & 0xFF;
    
    if (!spi_master_write(m_spinand.spi_id, buf, 3, false))
    {
        akprintf(C1, M_DRVSYS, "spi_master_write 00 fail\r\n");
        goto WRITE_EXIT;
    }
 
    if (!spi_data_mode(m_spinand.spi_id, m_spinand.bus_wight))
    {
        akprintf(C1, M_DRVSYS, "spi_data_mode fail\r\n");
    }
    if(ooblen > 0 )
    {
        if (!spi_master_write(m_spinand.spi_id, data, count, false))
        {
            akprintf(C1, M_DRVSYS, "spi_master_write 01 fail\r\n");
            goto WRITE_EXIT;
        }
        if(oobpos > 0)                                                       //如果oob信息不是直接跟在页数据之后，填充任意数据
        {
            if (!spi_master_write(m_spinand.spi_id, data, oobpos, false))
            {
                akprintf(C1, M_DRVSYS, "spi_master_write 02 fail\r\n");
                goto WRITE_EXIT;
            }
        }
        if (!spi_master_write(m_spinand.spi_id, oob, ooblen, true))
        {
            akprintf(C1, M_DRVSYS, "spi_master_write 03 fail\r\n");
            goto WRITE_EXIT;
        }
    }
    else
    {
        if (!spi_master_write(m_spinand.spi_id, data, count, true))
        {
            akprintf(C1, M_DRVSYS, "spi_master_write 04 fail\r\n");
            goto WRITE_EXIT;
        }
    }
    if (!spi_data_mode(m_spinand.spi_id, SPI_BUS1))
    {
        akprintf(C1, M_DRVSYS, "spi_data_mode fail\r\n");
    }

    //program execute
    buf[0] = SPI_NAND_PROGRAM_EXECUTE;
    buf[1] = (row >> 16) & 0xFF;
    buf[2] = (row >> 8) & 0xFF;
    buf[3] = (row >> 0) & 0xFF;

    if (!spi_master_write(m_spinand.spi_id, buf, 4, true))
    {
        goto WRITE_EXIT;
    }

    //check status
    if (wait_cmd_finish(&status))
    {
        if(!(status & STATUS_P_FAIL))
        {
            ret = true;
        }
        else
        {
            akprintf(C1, M_DRVSYS, "fail status == STATUS_P_FAIL\r\n");
        }
    }

WRITE_EXIT:
    spi_set_unprotect(m_spinand.spi_id, m_spinand.bus_wight);

    return ret;

}
#endif

//OOB的数据与data一起写下去去
bool spi_nand_write(unsigned long row, unsigned long column, unsigned char *data, unsigned long len)
{
    unsigned char buf[4];
    unsigned long count = len;
    unsigned char status;
    bool ret = false;

    if (!m_spinand.bInit || 0 == len)
    {
        akprintf(C1, M_DRVSYS, "spi_nand_write spinand not init\r\n");
        return false;
    }
        
    spi_set_protect(m_spinand.spi_id, m_spinand.bus_wight);

    //write enable
    write_enable();

    //program load
    buf[0] = (SPI_BUS4 == m_spinand.bus_wight) ? SPI_NAND_PROGRAM_LOAD_X4 : SPI_NAND_PROGRAM_LOAD;    
    buf[1] = (column >> 8) & 0x0F;
    buf[2] = column & 0xFF;
    //buf[3] = data[0];
    
    if (!spi_master_write(m_spinand.spi_id, buf, 3, false))
    {
        akprintf(C1, M_DRVSYS, "spi_master_write 00 fail\r\n");
        goto WRITE_EXIT;
    }
 
    if (!spi_data_mode(m_spinand.spi_id, m_spinand.bus_wight))
    {
        akprintf(C1, M_DRVSYS, "spi_data_mode fail\r\n");
    }

    if (!spi_master_write(m_spinand.spi_id, data, count, true))
    {
        akprintf(C1, M_DRVSYS, "spi_master_write 04 fail\r\n");
        goto WRITE_EXIT;
    }
    if (!spi_data_mode(m_spinand.spi_id, SPI_BUS1))
    {
        akprintf(C1, M_DRVSYS, "spi_data_mode fail\r\n");
    }

    //program execute
    buf[0] = SPI_NAND_PROGRAM_EXECUTE;
    buf[1] = (row >> 16) & 0xFF;
    buf[2] = (row >> 8) & 0xFF;
    buf[3] = (row >> 0) & 0xFF;

    if (!spi_master_write(m_spinand.spi_id, buf, 4, true))
    {
        goto WRITE_EXIT;
    }

    //check status
    if (wait_cmd_finish(&status))
    {
        if(!(status & STATUS_P_FAIL))
        {
            ret = true;
        }
        else
        {
            akprintf(C1, M_DRVSYS, "fail status == STATUS_P_FAIL\r\n");
        }
    }

WRITE_EXIT:
    spi_set_unprotect(m_spinand.spi_id, m_spinand.bus_wight);

    return ret;

}

#if 0
bool spi_nand_write(unsigned long row, unsigned long column, unsigned char *data, unsigned long len, unsigned char *oob, unsigned long ooblen, unsigned long oobpos)
{
    bool ret = false;
    unsigned long write_len = 0;
    //由于目前spinand的大步都是2048，spare区的大小是64, 所以暂时先定这么大小
    unsigned long page_size = 2048;
    unsigned long oob_size = 64;
    unsigned char buftemp[2112] = {0};
    unsigned char oobbuftemp[64] = {0};
        
   // akprintf(C1, M_DRVSYS, " spi_nand_write *****************d\r\n");
    if (!m_spinand.bInit || 0 == len)
    {
        akprintf(C1, M_DRVSYS, "spi_nand_write spinand not init\r\n");
        return false;
    }
    memset(buftemp, 0, 2112);
    memset(oobbuftemp, 0, 64);
    
    write_len = len;
    memcpy(buftemp, data, write_len);

   // akprintf(C1, M_DRVSYS, " oob:%x, ooblen:%d\r\n", oob, ooblen);
    if(oob != NULL && ooblen != 0)
    {
         //先进行读出spare区的大小
         if(0 == spi_nand_read(row, column + page_size, oobbuftemp, oob_size))   
         {
            akprintf(C1, M_DRVSYS, "read oob fail\r\n");
            return false;
         }
         //akprintf(C1, M_DRVSYS, "oob: %02x, %02x, %02x, %02x\r\n", oobbuftemp[0], oobbuftemp[1], oobbuftemp[2], oobbuftemp[3]);
         
         memcpy(&oobbuftemp[oobpos], oob, ooblen);
         
         //填充spare区，
         memcpy(&buftemp[write_len], oobbuftemp, oob_size);
         write_len = write_len + oob_size;
    }

    //写data 和spare区
    ret = spi_nand_write_page(row, column, buftemp, write_len);

    return ret;
}

#endif

/**
 * @brief erase one block of spi nand
 * 
 * @author liao_zhijun
 * @date 2014-1-2
 * @param row [in]  row address of spi nand
 * @return T_BOOL
 */
bool spi_nand_erase(unsigned long row)
{
    unsigned char buf1[4];
    bool ret = false;
    unsigned char status;
    
    if (!m_spinand.bInit)
    {
        akprintf(C1, M_DRVSYS, "spi_nand_erase spinand not init\r\n");
        return false;
    }

    spi_set_protect(m_spinand.spi_id, SPI_BUS1);

    //write enable
    write_enable();

    //erase
    buf1[0] = SPI_NAND_BLOCK_ERASE;
    buf1[1] = (row >> 16) & 0xFF;
    buf1[2] = (row >> 8) & 0xFF;
    buf1[3] = (row >> 0) & 0xFF;

    //erase
    if (!spi_master_write(m_spinand.spi_id, buf1, 4, true))
    {
        akprintf(C1, M_DRVSYS, "spi_master_write fail\r\n");
        goto ERASE_EXIT;
    }
    us_delay(10);
    //check status
    if (wait_cmd_finish(&status))
    {
        if(!(status & STATUS_E_FAIL))
        {
            ret = true;
        }
        else
        {
            akprintf(C1, M_DRVSYS, "wait_cmd_finish fail status == STATUS_E_FAIL\r\n");
        }
    }
    
ERASE_EXIT:    
    spi_set_unprotect(m_spinand.spi_id, SPI_BUS1);
    
    return ret;
}

#if 0
bool spi_nand_reset(T_VOID)
{
    T_U8 buf[1];
    
    //write enable
    buf[0] = SPI_NAND_RESET;
    spi_master_write(m_spinand.spi_id, buf, 1, AK_TRUE);

    return AK_TRUE;
}
#endif

#endif

