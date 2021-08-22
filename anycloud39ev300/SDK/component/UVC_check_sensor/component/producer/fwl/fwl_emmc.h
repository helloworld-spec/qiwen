#ifndef __FWL_EMMC_H__
#define __FWL_EMMC_H__

#include "anyka_types.h"
#include "mtdlib.h"

T_PMEDIUM SDDisk_Initial(void);

unsigned long FHA_SD_Erase(unsigned long nChip,  unsigned long nPage);
unsigned long FHA_SD_Read(unsigned long nChip,  unsigned long nPage, unsigned char *pData, unsigned long nDataLen,  unsigned char *pOob, unsigned long nOobLen , unsigned long eDataType);
unsigned long FHA_SD_Write(unsigned long nChip, unsigned long nPage, const unsigned char *pData, unsigned long nDataLen,  unsigned char *pOob, unsigned long nOobLen, unsigned long eDataType);

#endif
