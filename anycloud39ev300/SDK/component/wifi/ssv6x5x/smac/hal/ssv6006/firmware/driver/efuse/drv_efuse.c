#include <config.h>
#include <log.h>
//#include <./../drv_comm.h>
#include <regs.h>
#include "drv_efuse.h"

void read_efuse_identify(u32 *chip_identity)
{
#if 0
    // For Cabrio-E ADR_PAD20 is necessary to be set before accessing efuse.
    #if 0 //Liam PAD20 not defined
    volatile u32 *pointer = (u32 *)(ADR_PAD20);

    //Set GIO
    //SMAC_REG_WRITE(sh,0xC0000328,0x11);
    *pointer = 0x11;
    #endif

    //Enable block 0 read procedure
    //ADR_EFUSE_SPI_RD0_EN
    //#define SSV_EFUSE_ID_READ_SWITCH   0xC2000128
    //SMAC_REG_WRITE(sh, SSV_EFUSE_ID_READ_SWITCH, 0x1);
    SET_EFS_SPI_RD0_EN(0x1);

    //Read block 0
    //ADR_EFUSE_SPI_RDATA_0
    //#define SSV_EFUSE_ID_RAW_DATA_BASE 0xC200014C
    //SMAC_REG_READ(sh, SSV_EFUSE_ID_RAW_DATA_BASE, &val);
    *chip_identity = GET_EFS_SPI_RDATA_0;

    #if 0
    //Set GIO
    //SMAC_REG_WRITE(sh,0xC0000328,0x1800000a);
    *pointer = 0x1800000a;
    #endif//Liam PAD20 not defined
#else // for chip_id to 0x0 after release_20161123063141
    *chip_identity = 0;
#endif
}

