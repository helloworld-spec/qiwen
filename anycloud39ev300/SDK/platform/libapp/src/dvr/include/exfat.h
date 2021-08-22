#ifndef _EX_FAT_H_
#define _EX_FAT_H_
#define EXFAT_BITMAP_SCLUS  2
#include "driverlib.h"
#include "attr.h"

T_PFAT Fat_InitialEX(T_PDRIVER driver, T_U8 *buf);
T_VOID Fat_InitFatBufEX(T_PDRIVER driver, F_GetDriverCallback pGet_Griver);
T_BOOL Fat_DriverWriteFatMap(T_PDRIVER driver, T_U32 Addr, T_U8 *buf);
T_BOOL Fat_DriverReadFatMap(T_PDRIVER driver, T_U32 Addr, T_U8 *buf);
T_BOOL Fat_FlushFatMapEX(T_PATTR attr, T_U8 AllFlag);
T_U16 Fat_GetUnicodeUpdateCase(T_U16 UniCode);
T_BOOL Fat_FormatEX(T_PDRIVER driver);
T_BOOL Fat_QuickFormatEX(T_PDRIVER driver);
#endif

