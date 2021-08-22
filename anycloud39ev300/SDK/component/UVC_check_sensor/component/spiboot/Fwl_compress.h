
#ifdef COMPRESS

#ifndef _FWL_COMPRESS_H__
#define _FWL_COMPRESS_H__

#include "anyka_types.h"

//#define DEBUG_API_TEST

#ifdef DEBUG_API_TEST
#define TEST_VER_NO   "WX0.0.1"
T_VOID Hex_Dump(T_U8* tips, T_U8*data,T_U32 Len);
#endif

T_U32  Fwl_DeComImg(T_pVOID srcAddr,T_U32 srcLen,T_pVOID destAddr,T_U32 destLen);
#endif
#endif

