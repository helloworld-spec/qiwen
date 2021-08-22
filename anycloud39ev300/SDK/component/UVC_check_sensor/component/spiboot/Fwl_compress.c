
#ifdef COMPRESS

#include "Fwl_compress.h"
#include "Zlib.h"
#include "memapi.h"


//#define DBG_COM
#define    DECOM_CFG_ADDRESS               (0x30500500)
#ifdef DEBUG_OUTPUT
#define COMPRESS_INFO  printf
#else
#define COMPRESS_INFO
#endif

T_pVOID  malloc(T_U32 size)
{
#ifdef DBG_COM
	COMPRESS_INFO("malloc %d\n",size);
#endif
	return AllocMem(size);
}

T_pVOID free(T_pVOID pVal)
{
#ifdef DBG_COM
	COMPRESS_INFO("free 0x%x\n",pVal);
#endif
	FreeMem(pVal);
	return AK_NULL;
}

static T_BOOL Fwl_InitCompress(T_VOID)
{
	zlibmemfunset((cballoc_func)AllocMem,(cbfree_func)FreeMem);
	
#ifdef DBG_COM
	COMPRESS_INFO("[CMPRS]ZlibVersion  %s Malloc[0x%x]Free[0x%x]\n",
		zlibVersion(),AllocMem,FreeMem);
#endif
	return AK_TRUE;
}

T_U32  Fwl_DeComImg(T_pVOID srcAddr,T_U32 srcLen,T_pVOID destAddr,T_U32 destLen)
{
	T_S32  retval = 0;
	T_BOOL ret = AK_FALSE;
	T_U32 decomLen = 0;
	
#ifdef DBG_COM
	if (srcLen > destLen)
	{
		COMPRESS_INFO("[CMPRS]SrcBufLen = %d DeComBufLen = %d\n",srcLen,destLen);
		return AK_FALSE;
	}
#endif
	decomLen = destLen;
	Fwl_InitCompress();

#ifdef DBG_COM
	COMPRESS_INFO("[CMPRS]Src[Addr = 0x%x Len = %d] Des[Addr = 0x%x Len = %d]\n",srcAddr,srcLen,destAddr,decomLen);
#endif
	

	retval = decompress_spring((Bytef*)srcAddr,(uLong)srcLen,\
		(Bytef*)destAddr,(uLongf*)&decomLen);
	
	ret = ((Z_OK == retval) && (decomLen > 0));
	
#ifdef DBG_COM
	COMPRESS_INFO("[CMPRS]decom ret = %d decomLen = %d\n",\
		retval,decomLen);
#endif

#ifdef DBG_COM
	if (!ret)
	{
		COMPRESS_INFO("[CMPRS]decom Er\n");
	}
	else
	{
		Hex_Dump("DeComFile",destAddr,decomLen);
	}
#else
	COMPRESS_INFO("decom ret=%d,len=%d\n",ret,decomLen);
#endif
	
	//*((T_U32 *)DECOM_CFG_ADDRESS)= decomLen;
	return decomLen;
}


#ifdef DEBUG_API_TEST
T_VOID Hex_Dump(T_U8* tips, T_U8*data,T_U32 Len)
{
#ifdef DEBUG_OUTPUT
#define DUMP_PRINT  printf
#endif
#define FS_TEST_DUMP_COL 16
	T_U32 i= 0;
    DUMP_PRINT("%s:Begin...[%d]Bytes <Ver: %s>\n",tips,Len,TEST_VER_NO);
	//Len = 1024;
	if (AK_NULL != data)
	{
		for (i=0;i<Len;i++)
		{
			if (i%FS_TEST_DUMP_COL == 0)
			{
				DUMP_PRINT("%08xh: ",i);
			}
		    DUMP_PRINT(" %02X",data[i]);
			if ((i+1)%FS_TEST_DUMP_COL == 0)
			{
		    DUMP_PRINT("\n");
			}
		}
	}
    DUMP_PRINT("%s End\n",tips);
}
#endif
#endif

