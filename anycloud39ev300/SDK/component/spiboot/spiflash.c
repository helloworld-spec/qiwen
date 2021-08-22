/**
 * @file spi_flash.c
 * @brief spiflash driver source file
 * Copyright (C) 2004 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author LiaoZhijun
 * @date 2010-05-27
 * @version 1.0
 */
#include "anyka_types.h"
#include "anyka_cpu.h"
#include "spi.h"
#include "spiflash.h"

#define drv_print   printf
#define PDEBUG(fmt, args...) //printf(fmt, ##args) // cdh:dbug


#define SPIBOOT_PARAM_ADDR_BASE		(0x48000200)
#define SPIBOOT_PARAM_LEN		  	(512)

#define SPI_FLASH_COM_RDID        0x9f
#define SPI_FLASH_COM_READ        0x03
#define SPI_FLASH_COM_PROGRAM     0x02
#define SPI_FLASH_COM_WR_EN       0x06
#define SPI_FLASH_COM_ERASE       0xd8
#define SPI_FLASH_COM_RD_STUS1    0x05
#define SPI_FLASH_COM_RD_STUS2    0x35

#define SPI_FLASH_COM_WRSR1        0x01
#define SPI_FLASH_COM_WRSR2        0x31
#define SPI_FLASH_COM_WRSR3        0x11

#define SPI_FLASH_COM_AAI         0xad
#define SPI_FLASH_COM_WRDI        0x04

//2-wire mode
#define SPI_FLASH_COM_D_READ      0x3B
#define SPI_FLASH_COM_2IO_READ	  0xbb

//4-wire mode
#define SPI_FLASH_COM_Q_READ      0x6B
#define SPI_FLASH_COM_4IO_READ	  0xeb

#define SPI_FLASH_COM_Q_WRITE     0x32


#define SPI_FLASH_QUAD_ENABLE    (1 << 9)

#define SPI_SECTOR_SIZE           256

#define MAX_RD_SIZE              (32 * 1024)

T_BOOL spi_flash_read_status(T_U16 *status);
T_BOOL spi_flash_write_status(T_U16 status);

T_BOOL spi_flash_common_write_status(T_U16 status);
T_BOOL spi_flash_gd25q64c_write_status(T_U16 status);
T_BOOL spi_flash_gd25q128c_write_status(T_U16 status);
T_BOOL spi_flash_common_read_status(T_U16 *status);


static T_VOID write_enable(T_VOID);
static T_BOOL check_status_done(T_VOID);
static T_BOOL flash_read_standard(T_U32 page, T_U8 *buf, T_U32 page_cnt);
static T_BOOL flash_read_quad(T_U32 page, T_U8 *buf, T_U32 page_cnt);
static T_BOOL flash_read_dual(T_U32 page, T_U8 *buf, T_U32 page_cnt);
static T_BOOL flash_QE_set(T_BOOL enable);
static T_eSPI_BUS current_spi_bus_width(void);
static T_BOOL flash_read_2io(T_U32 page, T_U8 *buf, T_U32 page_cnt);
static T_BOOL flash_read_4io(T_U32 page, T_U8 *buf, T_U32 page_cnt);

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

/**
  *because of some spi flash is difference of status register difinition.
  *this structure use mapping the status reg function and corresponding.
*/
struct flash_status_reg
{
	T_U32	jedec_id;	

	unsigned b_wip:4;		/*write in progress*/
	unsigned b_wel:4;		/*wrute ebabke latch*/
	unsigned b_qe:4;		/*quad enable*/

	T_BOOL (*read_sr)(T_U16 *status);
	T_BOOL (*write_sr)(T_U16 status);
};



typedef struct
{
    T_BOOL bInit;

    T_eSPI_ID spi_id;

    T_U8 bus_wight;
#define BUS_SUPPORT_1WIRE	(1<<0)
#define BUS_SUPPORT_2WIRE	(1<<1)
#define BUS_SUPPORT_4WIRE	(1<<2)

    T_SFLASH_PARAM param;
	struct flash_status_reg stat_reg;
}
T_SPI_FLASH;

#ifndef USE_FOR_BURNTOOL_PARAM
static T_SFLASH_PARAM  flash_params[] = {
	{0x1640ef, 4194304,  256, 256, 65536, 25000000, 0xf8, 0x0, "w25q32"},
	{ 0x1640c8, 4194304,  256, 256, 65536, 25000000, 0x0, 0x0, "gd25q32"},
	{ 0x1530ef, 1048576,  256, 256, 65536, 25000000, 0x0, 0x0, "w25x16"},
	{ 0x1540c8, 1048576,  256, 256, 65536, 25000000, 0x0, 0x0, "gd25q16"},
	{ 0x1440ef, 1048576,  256, 256, 65536, 25000000, 0x0, 0x0, ""},
	{ 0x1630ef, 4194304,  256, 256, 65536, 25000000, 0x0, 0x0, "w25x32"},
	{ 0x1740ef, 8388608,  256, 256, 65536, 25000000, 0x0, 0x0, "w25q64"},
	{ 0x1740c8, 8388608,  256, 256, 65536, 25000000, 0xf8, 0x0, "gd25q64"},
	{ 0x1840c8, 16777216,  256, 256, 65536, 25000000, 0xf8, 0x0, "gd25q128"},
	{ 0x1720c2, 8388608,  256, 256, 65536, 25000000, 0x20, 0x0, "mx25l6405d"},
	{ 0x1820c2, 16777216,  256, 256, 65536, 25000000, 0x50, 0x0, "mx25l12805d"},
	{ 0x1840ef, 16777216,  256, 256, 65536, 25000000, 0xf8, 0x0, "w25q128"},
};
#endif

static 	struct flash_status_reg status_reg_list[] = {
		/*spiflash mx25l12805d*/
		{
			.jedec_id = 0x1820c2,
			.b_wip = 0,	.b_wel = 1,	.b_qe = 6,
			.write_sr = spi_flash_common_write_status,
			.read_sr = spi_flash_common_read_status,
		},

			/*spiflash gd25q64c*/
		{
			.jedec_id = 0x1740c853,
			.b_wip = 0, .b_wel = 1, .b_qe = 9,
			.write_sr = spi_flash_gd25q64c_write_status,
			.read_sr = spi_flash_common_read_status,
		},

		/*spiflash gd25q128c*/
		{
			.jedec_id = 0x1840c8,
			.b_wip = 0,	.b_wel = 1,	.b_qe = 9,
			.write_sr = spi_flash_gd25q128c_write_status,
			.read_sr = spi_flash_common_read_status,
		},

		/*normal status reg define*/
		{
			.jedec_id = 0,
			.b_wip = 0,	.b_wel = 1,	.b_qe = 9,
			.write_sr = spi_flash_common_write_status,
			.read_sr = spi_flash_common_read_status,
		},
};


static T_SPI_FLASH m_sflash = {0};


/**
 * strstr - Find the first substring in a %NUL terminated string
 * @s1: The string to be searched
 * @s2: The string to search for
 */
char *memstr(const char *s1, int l1, const char *s2, int l2)
{	
	while (l1 >= l2) {
		l1--;
		if (!memcmp(s1, s2, l2))
			return (char *)s1;
		s1++;
	}
	return 0;
}

static int ak_spiflash_init_stat_reg()
{
	int i;
	struct flash_status_reg *sr;
	T_SFLASH_PARAM *info = &m_sflash.param;
	T_U8 code[5] = {0x5a,0x00,0x00,0x00,0x00}; //OPCODE_VER;		
	T_U8 id[2];
	T_U8 id_ver_c;
	T_U32 tmp_id;	
	tmp_id = info->id;
	
	if(0x1740c8 == info->id){
		
			 spi_master_write(m_sflash.spi_id, code, 5, AK_FALSE);
			 if(!spi_master_read(m_sflash.spi_id, id, 1, AK_TRUE))
				 return AK_FALSE;
			 id_ver_c = id[0]; //0x53		
			 
			 if(0x53==id_ver_c){
				printf("This spiflash is GD25Q64C\r\n");		 	
			 	info->id = info->id << 8;	
				info->id |= id_ver_c;	  
				} 
			else{
					printf("This spiflash is GD25Q64B\r\n");					
				}
		}

	for(i=0, sr=status_reg_list; i<ARRAY_SIZE(status_reg_list); i++, sr++) {
		if (sr->jedec_id == info->id) {
			memcpy(&m_sflash.stat_reg, sr, sizeof(struct flash_status_reg));
			info->id = tmp_id ;
			return 0;
		}
	}
	
	m_sflash.stat_reg = status_reg_list[i-1];
	info->id = tmp_id ;
	return 0;
}

/**
 * @brief spi flash init
 * 
 * @author liao_zhijun
 * @date 2010-04-29
 * @return T_BOOL
 */
T_BOOL spi_flash_init(T_eSPI_ID spi_id)
{
	int ii;
	T_U32 dev_id;
	T_U8* p_data = AK_NULL;
	T_SFLASH_PARAM *info, sflash_param;

    //set clock to 25M, use mode0, coz some spi flash may have data error in mode3
    spi_ctrl_init(spi_id, SPI_MODE0_V, SPI_MASTER_V, 20*1000*1000);

    m_sflash.spi_id = spi_id;
    m_sflash.bus_wight = BUS_SUPPORT_1WIRE | BUS_SUPPORT_2WIRE | BUS_SUPPORT_4WIRE;
    m_sflash.bInit = AK_TRUE;

	dev_id = spi_flash_getid();

	p_data = (T_U8*)SPIBOOT_PARAM_ADDR_BASE;
	p_data = memstr((T_U8*)p_data, SPIBOOT_PARAM_LEN, "SPIP", 4);
	p_data += 4;
	
	PDEBUG("CDH:HOHO, get the spi param at %p.\r\n", p_data);
	//while(1);

#ifdef USE_FOR_BURNTOOL_PARAM
	sflash_param = *((T_SFLASH_PARAM*)p_data);
#else

	for (ii = 0, info = (T_SFLASH_PARAM*)flash_params;
			ii < ARRAY_SIZE(flash_params);
			ii++, info++) {
		if (info->id == dev_id) {
			sflash_param = *info;
			break;
		}
	}
#endif

	printf("spi param: id=%08x, total_size=%d, page_size=%d, program_size=%d.\r\n", 
				sflash_param.id, sflash_param.total_size, 
				sflash_param.page_size, sflash_param.program_size);
	printf("erase_size=%d, clock=%d, flag=%d, protect_mask=%d.\r\n", 
				sflash_param.erase_size, sflash_param.clock, 
				sflash_param.flag, sflash_param.protect_mask);

	spi_flash_set_param(&sflash_param);
    return AK_TRUE;
}

/**
 * @brief set param of serial flash
 * 
 * @author liao_zhijun
 * @date 2010-04-29
 * @param sflash_param [in]  serial flash param 
 * @return T_VOID
 */
T_VOID spi_flash_set_param(T_SFLASH_PARAM *sflash_param)
{
    T_U16 status;

    if(AK_NULL == sflash_param)
        return;

    //save param
    memcpy(&m_sflash.param, sflash_param, sizeof(T_SFLASH_PARAM));

    //setup clock
    if(sflash_param->clock > 0)
    {
        spi_ctrl_init(m_sflash.spi_id, SPI_MODE0_V, SPI_MASTER_V, sflash_param->clock);
    }

    //unmask protect
    if(sflash_param->flag & SFLASH_FLAG_UNDER_PROTECT)
    {
        spi_flash_read_status(&status);

        status &= ~sflash_param->protect_mask;

        spi_flash_write_status(status & 0xffff);
    }

	ak_spiflash_init_stat_reg();

    //enable quad
    if((sflash_param->flag & SFLASH_FLAG_4IO_READ) \
        || (sflash_param->flag & SFLASH_FLAG_QUAD_READ))
    {
        if (BUS_SUPPORT_4WIRE & m_sflash.bus_wight)
        {
            if (!flash_QE_set(AK_TRUE))
            {
                drv_print("warnning: enable QE fail, will set to one-wire mode\n", 0, AK_TRUE);
                m_sflash.bus_wight &= ~BUS_SUPPORT_4WIRE;
            }
        }
    }
}

/**
 * @brief get spiflash id
 * 
 * @author liao_zhijun
 * @date 2010-04-29
 * @return T_U32
 * @retval T_U32 spiflash id
 */
T_U32 spi_flash_getid(T_VOID)
{
    T_U8 buf1[1];
    T_U32 flash_id = 0;

    if(!m_sflash.bInit)
    {
        drv_print("spi flash not init\r\n", 0, AK_TRUE);
        return 0;
    }

    buf1[0] = SPI_FLASH_COM_RDID;

    spi_master_write(m_sflash.spi_id, buf1, 1, AK_FALSE);
    spi_master_read(m_sflash.spi_id, (T_U8 *)(&flash_id), 3, AK_TRUE);
    
    drv_print("the manufacture id is %08x\r\n", flash_id, AK_TRUE);

    return flash_id;
}

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
T_BOOL spi_flash_read(T_U32 page, T_U8 *buf, T_U32 page_cnt)
{
    T_BOOL ret = AK_FALSE;
    
    if (BUS_SUPPORT_4WIRE & m_sflash.bus_wight)		
    {
    	if(m_sflash.param.flag & SFLASH_FLAG_QUAD_READ)
        	return flash_read_quad(page, buf, page_cnt); 
		else if(m_sflash.param.flag & SFLASH_FLAG_4IO_READ)
			return flash_read_4io(page, buf, page_cnt); 
    }
	if (BUS_SUPPORT_2WIRE & m_sflash.bus_wight)
    {
    	if(m_sflash.param.flag & SFLASH_FLAG_DUAL_READ)
        	return flash_read_dual(page, buf, page_cnt);
		else if(m_sflash.param.flag & SFLASH_FLAG_2IO_READ)
			return flash_read_2io(page, buf, page_cnt); 
    }

    return flash_read_standard(page, buf, page_cnt);
}


T_BOOL spi_flash_common_read_status(T_U16 *status)
{
    T_U8 buf1[1];
    T_U8 status1, status2;

    buf1[0] = SPI_FLASH_COM_RD_STUS1;

    spi_master_write(m_sflash.spi_id, buf1, 1, AK_FALSE);

    if(!spi_master_read(m_sflash.spi_id, &status1, 1, AK_TRUE))
        return AK_FALSE;

    if (m_sflash.param.flag & SFLASH_FLAG_COM_STATUS2)
    {
        buf1[0] = SPI_FLASH_COM_RD_STUS2;
        
        spi_master_write(m_sflash.spi_id, buf1, 1, AK_FALSE);
        
        if(!spi_master_read(m_sflash.spi_id, &status2, 1, AK_TRUE))
            return AK_FALSE;
        
        *status = (status1 | (status2 << 8));
    }
    else
    {
        *status = status1 & 0xff;
    }

    return AK_TRUE;
}

T_BOOL spi_flash_gd25q64c_write_status(T_U16 status)
{
    T_U8 buf[3];

	check_status_done();
    write_enable();

    buf[0] = SPI_FLASH_COM_WRSR1;
    buf[1] = status & 0xff;
    
    if(!spi_master_write(m_sflash.spi_id, buf, 2, AK_TRUE))
        return AK_FALSE;

	check_status_done();
    write_enable();

    buf[0] = SPI_FLASH_COM_WRSR2;
    buf[1] = (status >> 8) & 0xff;

    if(!spi_master_write(m_sflash.spi_id, buf, 2, AK_TRUE))
        return AK_FALSE;

    return check_status_done();
}

T_BOOL spi_flash_gd25q128c_write_status(T_U16 status)
{
    T_U8 buf[3];

	check_status_done();
    write_enable();

    buf[0] = SPI_FLASH_COM_WRSR1;
    buf[1] = status & 0xff;
    
    if(!spi_master_write(m_sflash.spi_id, buf, 2, AK_TRUE))
        return AK_FALSE;

	check_status_done();
    write_enable();

    buf[0] = SPI_FLASH_COM_WRSR2;
    buf[1] = (status >> 8) & 0xff;

    if(!spi_master_write(m_sflash.spi_id, buf, 2, AK_TRUE))
        return AK_FALSE;

    return check_status_done();
}


T_BOOL spi_flash_common_write_status(T_U16 status)
{
    T_U8 buf[3];
    T_U32 write_cnt;

    write_enable();

    buf[0] = SPI_FLASH_COM_WRSR1;
    buf[1] = status & 0xff;
    buf[2] = (status >> 8) & 0xff;

    if (m_sflash.param.flag & SFLASH_FLAG_COM_STATUS2)
    {
        write_cnt = 3;
    }
    else
    {
        write_cnt = 2;
    }
    
    if(!spi_master_write(m_sflash.spi_id, buf, write_cnt, AK_TRUE))
        return AK_FALSE;

    return check_status_done();
}

T_BOOL spi_flash_read_status(T_U16 *status)
{
	if(m_sflash.stat_reg.read_sr)
		return m_sflash.stat_reg.read_sr(status);

	return AK_FALSE;
}


T_BOOL spi_flash_write_status(T_U16 status)
{
	if(m_sflash.stat_reg.write_sr)
		return m_sflash.stat_reg.write_sr(status);

	return AK_FALSE;
}


static T_BOOL flash_read_standard(T_U32 page, T_U8 *buf, T_U32 page_cnt)
{
    T_U8 buf1[4];
    T_U32 i, addr = page * m_sflash.param.page_size;
    T_BOOL bReleaseCS=AK_FALSE;
    T_U32 count, readlen;
    
    if((page > 0xffff) || (AK_NULL == buf))
    {
        drv_print("spi_flash_read: param error\r\n", 0, AK_TRUE);
        return AK_FALSE;
    }

    if(!m_sflash.bInit)
    {
        drv_print("spi flash not init\r\n", 0, AK_TRUE);
        return AK_FALSE;
    }
    PDEBUG("%s: addr is %08x, page count is %d\r\n", __func__, addr, page_cnt);
	
    buf1[0] = SPI_FLASH_COM_READ;
    buf1[1] = (addr >> 16) & 0xFF;
    buf1[2] = (addr >> 8) & 0xFF;
    buf1[3] = (addr >> 0) & 0xFF;

    spi_master_write(m_sflash.spi_id, buf1, 4, AK_FALSE);

    count = page_cnt * m_sflash.param.page_size;
    while(count>0)
    {
        readlen = (count > MAX_RD_SIZE)? MAX_RD_SIZE: count;
        count -= readlen;
        //only the last read, release CS
        bReleaseCS = (count>0)? AK_FALSE: AK_TRUE;
        if(!spi_master_read(m_sflash.spi_id, buf, readlen, bReleaseCS))
            return AK_FALSE;
        buf += readlen;        
    }
    
    return AK_TRUE;
}

static T_BOOL flash_read_dual(T_U32 page, T_U8 *buf, T_U32 page_cnt)
{
    T_U8 buf_cmd[5];
    T_U16 status;
    T_U32 addr = page * m_sflash.param.page_size;
    T_U32 count;
    T_U32 readlen;
    T_BOOL bReleaseCS=AK_FALSE;
    
    if((page > 0xffff) || (AK_NULL == buf))
    {
        drv_print("spi_flash_read: param error\r\n", 0, AK_TRUE);
        return AK_FALSE;
    }

    if(!m_sflash.bInit)
    {
        drv_print("spi flash not init\r\n", 0, AK_TRUE);
        return AK_FALSE;
    }
	
    PDEBUG("%s: addr is %08x, page count is %d\r\n", __func__, addr, page_cnt);

    buf_cmd[0] = SPI_FLASH_COM_D_READ;
    buf_cmd[1] = (T_U8)((addr >> 16) & 0xFF);
    buf_cmd[2] = (T_U8)((addr >> 8) & 0xFF);
    buf_cmd[3] = (T_U8)(addr & 0xFF);
    buf_cmd[4] = 0;

    // cmd
    if (!spi_master_write(m_sflash.spi_id, buf_cmd, 5, AK_FALSE))
    {
        drv_print("spi flash_read_quad: write cmd fail\r\n", 0, AK_TRUE);
        return AK_FALSE;
    }

    // set spi 2-wire
    spi_data_mode(m_sflash.spi_id, SPI_BUS2);

    // read
    count = page_cnt * m_sflash.param.page_size;
    while(count>0)
    {
        readlen = (count > MAX_RD_SIZE)? MAX_RD_SIZE: count;
        count -= readlen;
        //only the last read, release CS
        bReleaseCS = (count>0)? AK_FALSE: AK_TRUE;
        if(!spi_master_read(m_sflash.spi_id, buf, readlen, bReleaseCS))
        {
            // set spi 1-wire
            spi_data_mode(m_sflash.spi_id, SPI_BUS1);
            return AK_FALSE;
        }
        buf += readlen;        
    }
    
    // set spi 1-wire
    spi_data_mode(m_sflash.spi_id, SPI_BUS1);

    return AK_TRUE;
}

static T_BOOL flash_read_quad(T_U32 page, T_U8 *buf, T_U32 page_cnt)
{
    T_U8 buf_cmd[5];
    T_U16 status;
    T_U32 addr = page * m_sflash.param.page_size;
    T_U32 count;
    T_U32 readlen;
    T_BOOL bReleaseCS=AK_FALSE;
    T_BOOL ret = AK_TRUE;
    
    if((page > 0xffff) || (AK_NULL == buf))
    {
        drv_print("spi_flash_read: param error\r\n", 0, AK_TRUE);
        return AK_FALSE;
    }

    if(!m_sflash.bInit)
    {
        drv_print("spi flash not init\r\n", 0, AK_TRUE);
        return AK_FALSE;
    }
    
    PDEBUG("%s: addr is %08x, page count is %d\r\n", __func__, addr, page_cnt);
    // cmd
    buf_cmd[0] = SPI_FLASH_COM_Q_READ;
    buf_cmd[1] = (T_U8)((addr >> 16) & 0xFF);
    buf_cmd[2] = (T_U8)((addr >> 8) & 0xFF);
    buf_cmd[3] = (T_U8)(addr & 0xFF);
    buf_cmd[4] = 0;
    if (!spi_master_write(m_sflash.spi_id, buf_cmd, 5, AK_FALSE))
    {
        drv_print("spi flash_read_quad: write cmd fail\r\n", 0, AK_TRUE);
        goto EXIT;
    }

    // set spi 4-wire
    spi_data_mode(m_sflash.spi_id, SPI_BUS4);

    // read
    count = page_cnt * m_sflash.param.page_size;
    while(count>0)
    {
        readlen = (count > MAX_RD_SIZE)? MAX_RD_SIZE: count;
        count -= readlen;
        //only the last read, release CS
        bReleaseCS = (count>0)? AK_FALSE: AK_TRUE;
        if(!spi_master_read(m_sflash.spi_id, buf, readlen, bReleaseCS))
        {
            ret = AK_FALSE;
            goto EXIT;
        }
        buf += readlen;        
    }

EXIT:   
    // set spi 1-wire
    spi_data_mode(m_sflash.spi_id, SPI_BUS1);

    return ret;
}


static T_BOOL flash_read_2io(T_U32 page, T_U8 *buf, T_U32 page_cnt)
{
    T_U8 buf_cmd[5];
    T_U16 status;
    T_U32 addr = page * m_sflash.param.page_size;
    T_U32 count;
    T_U32 readlen;
    T_BOOL bReleaseCS=AK_FALSE;
    
    if((page > 0xffff) || (AK_NULL == buf))
    {
        drv_print("spi_flash_read: param error\r\n", 0, AK_TRUE);
        return AK_FALSE;
    }

    if(!m_sflash.bInit)
    {
        drv_print("spi flash not init\r\n", 0, AK_TRUE);
        return AK_FALSE;
    }
	
    PDEBUG("%s: addr is %08x, page count is %d\r\n", __func__, addr, page_cnt);

    buf_cmd[0] = SPI_FLASH_COM_2IO_READ;	
	
    buf_cmd[1] = (T_U8)((addr >> 16) & 0xFF);
    buf_cmd[2] = (T_U8)((addr >> 8) & 0xFF);
    buf_cmd[3] = (T_U8)(addr & 0xFF);
    buf_cmd[4] = 0;

    // cmd
    if (!spi_master_write(m_sflash.spi_id, buf_cmd, 1, AK_FALSE))
    {
        drv_print("spi flash_read_quad: write cmd fail\r\n", 0, AK_TRUE);
        return AK_FALSE;
    }

    // set spi 2-wire
    spi_data_mode(m_sflash.spi_id, SPI_BUS2);

    // cmd
    if (!spi_master_write(m_sflash.spi_id, &buf_cmd[1], 4, AK_FALSE))
    {
        drv_print("spi flash_read_quad: write addr fail\r\n", 0, AK_TRUE);
        return AK_FALSE;
    }

    // read
    count = page_cnt * m_sflash.param.page_size;
    while(count>0)
    {
        readlen = (count > MAX_RD_SIZE)? MAX_RD_SIZE: count;
        count -= readlen;
        //only the last read, release CS
        bReleaseCS = (count>0)? AK_FALSE: AK_TRUE;
        if(!spi_master_read(m_sflash.spi_id, buf, readlen, bReleaseCS))
        {
            // set spi 1-wire
            spi_data_mode(m_sflash.spi_id, SPI_BUS1);
            return AK_FALSE;
        }
        buf += readlen;        
    }
    
    // set spi 1-wire
    spi_data_mode(m_sflash.spi_id, SPI_BUS1);

    return AK_TRUE;
}

static T_BOOL flash_read_4io(T_U32 page, T_U8 *buf, T_U32 page_cnt)
{
    T_U8 buf_cmd[7];
    T_U16 status;
    T_U32 addr = page * m_sflash.param.page_size;
    T_U32 count;
    T_U32 readlen;
    T_BOOL bReleaseCS=AK_FALSE;
    T_BOOL ret = AK_TRUE;
    
    if((page > 0xffff) || (AK_NULL == buf))
    {
        drv_print("spi_flash_read: param error\r\n", 0, AK_TRUE);
        return AK_FALSE;
    }

    if(!m_sflash.bInit)
    {
        drv_print("spi flash not init\r\n", 0, AK_TRUE);
        return AK_FALSE;
    }
    
    PDEBUG("%s: addr is %08x, page count is %d\r\n", __func__, addr, page_cnt);
    // cmd
    buf_cmd[0] = SPI_FLASH_COM_4IO_READ;
	
    buf_cmd[1] = (T_U8)((addr >> 16) & 0xFF);
    buf_cmd[2] = (T_U8)((addr >> 8) & 0xFF);
    buf_cmd[3] = (T_U8)(addr & 0xFF);
    buf_cmd[4] = 0;	
    buf_cmd[5] = 0;
    buf_cmd[6] = 0;
	
    if (!spi_master_write(m_sflash.spi_id, buf_cmd, 1, AK_FALSE))
    {
        drv_print("spi flash_read_quad: write cmd fail\r\n", 0, AK_TRUE);
        goto EXIT;
    }

    // set spi 4-wire
    spi_data_mode(m_sflash.spi_id, SPI_BUS4);
	
    if (!spi_master_write(m_sflash.spi_id, &buf_cmd[1], 6, AK_FALSE))
    {
        drv_print("spi flash_read_quad: write addr fail\r\n", 0, AK_TRUE);
        goto EXIT;
    }

    // read
    count = page_cnt * m_sflash.param.page_size;
    while(count>0)
    {
        readlen = (count > MAX_RD_SIZE)? MAX_RD_SIZE: count;
        count -= readlen;
        //only the last read, release CS
        bReleaseCS = (count>0)? AK_FALSE: AK_TRUE;
        if(!spi_master_read(m_sflash.spi_id, buf, readlen, bReleaseCS))
        {
            ret = AK_FALSE;
            goto EXIT;
        }
        buf += readlen;        
    }

EXIT:   
    // set spi 1-wire
    spi_data_mode(m_sflash.spi_id, SPI_BUS1);

    return ret;
}

T_VOID write_enable_spinand(T_VOID)
{
    T_U8 buf1[1];
    
    //write enable
    buf1[0] = SPI_FLASH_COM_WR_EN;
    spi_master_write(SPI_ID0, buf1, 1, AK_TRUE);
}
static T_VOID write_enable(T_VOID)
{
    T_U8 buf1[1];
    
    //write enable
    buf1[0] = SPI_FLASH_COM_WR_EN;
    spi_master_write(m_sflash.spi_id, buf1, 1, AK_TRUE);
}

static T_BOOL check_status_done(T_VOID)
{
    T_U32 timeout = 0;
    T_U16 status = 0;

    while(1)
    {
        spi_flash_read_status(&status);

        if((status & (1 << m_sflash.stat_reg.b_wip)) == 0)
            break;
        if(timeout++ > 100000)
        {
            drv_print("spiflash check_status_done timeout\n", 0, AK_TRUE);
            return AK_FALSE;
        }
    }

    return AK_TRUE;
}


static T_BOOL flash_QE_set(T_BOOL enable)
{
    T_U16 status;
  
    if(check_status_done()==AK_FALSE)
		return AK_FALSE;	

	write_enable();	
    if (!spi_flash_read_status(&status))
    {
        drv_print("spi QE read status fail\r\n", 0, AK_TRUE);
        return AK_FALSE;
    }

    if (enable)
    {
        status |=(1<<m_sflash.stat_reg.b_qe);
    }
    else
    {
        status &=~(1<<m_sflash.stat_reg.b_qe);
    }
	printf("spiflash:set status reg:%08x\r\n", status);
    if (!spi_flash_write_status(status & 0xffff))
    {
        drv_print("spi QE: write status fail \r\n", 0, AK_TRUE);
        return AK_FALSE;
    }

    return AK_TRUE;
}


T_S32 Fwl_SPI_FileRead(T_PARTITION_TABLE_INFO *cfg, T_U8* buffer, T_U32 count)
{
	T_BIN_CONFIG binbuf;
	
	T_U8* pDATA = AK_NULL;
	T_S32 ret = -1;
	T_U32 page = 0;
	T_U32 page_cnt = 0;
	T_U32 addr = 0;
	T_U32 page_read_32k = 128;//one times read 32K
	T_U32 page_read_cnt;
	T_U32 start_add_dst=0;
	T_U32 start_add_src=0;
	
	if ((AK_NULL == buffer)||(AK_NULL == cfg))
	{
		return ret;
	}

	memcpy(&binbuf, &(cfg->ex_partition_info), sizeof(T_EX_PARTITION_CONFIG));
	pDATA = (T_U8*)0x81000000;
	if (pDATA == AK_NULL)
	{
		return ret;
	}
	
	if (count > binbuf.file_length)
	{
		count = binbuf.file_length;
	}
	
	page = (cfg->partition_info.start_pos)/SPI_FLASH_PAGE_SIZE;

	addr = count%SPI_FLASH_PAGE_SIZE;
		
	page_cnt = count/SPI_FLASH_PAGE_SIZE;
	if (0 == page_cnt)
	{
		page_cnt = 1;
	}
	else if (1 <= page_cnt)
	{
		if (0 != addr)
		{
			page_cnt++;
		}
	}

	if (1 == page_cnt)
	{
		spi_flash_read(page,pDATA,1);
		memcpy(&buffer,pDATA,count);
		start_add_dst += count;
		page_cnt --;
		page ++;
	}
	
	if ((0 != addr) && (1 <= page_cnt))
	{
		page_cnt--;
	}
	page_read_cnt = page_cnt/page_read_32k;
	while(page_read_cnt--)
	{
		spi_flash_read(page,&buffer[start_add_dst],page_read_32k);
		//memcpy(&buffer[start_add_dst],pDATA,buf_size);
		start_add_dst += SPI_FLASH_PAGE_SIZE*page_read_32k;
		page_cnt -= page_read_32k;
		page += page_read_32k;
	}
	page_read_cnt = page_cnt%page_read_32k;
	while(page_read_cnt--)
	{
		spi_flash_read(page,&buffer[start_add_dst],1);
		//memcpy(&buffer[start_add_dst],pDATA,SPI_PAGE_SIZE);
		start_add_dst += SPI_FLASH_PAGE_SIZE;
		page_cnt--;
		page ++;
	}
	
	if ((0 != addr) && (0 == page_cnt))
	{			
		spi_flash_read(page,pDATA,1);
		memcpy(&buffer[start_add_dst],pDATA,addr);
		start_add_dst += addr;
		page ++;
	}	
	
	//pDATA = FreeMem(pDATA);
	
	if (start_add_dst != count)
	{
		drv_print("ERR:LHS:SPI read %s fail..Page:%d,Count:%d\r\n",cfg->partition_info.name,page,count);
		start_add_dst = 0;
	}
	
	return start_add_dst;
}


