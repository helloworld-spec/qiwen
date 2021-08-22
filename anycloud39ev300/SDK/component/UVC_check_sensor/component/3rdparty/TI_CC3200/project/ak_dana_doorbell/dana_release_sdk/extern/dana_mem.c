#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include "osi.h"

#include "dana_mem.h"
#include  "uart_if.h"


//大拿内存api接口， 厂商自行实现


extern void *dana_malloc(size_t size)
{	
    return  mem_Malloc(size);
}

extern void *dana_calloc(size_t nmemb, size_t size)
{
    char * _pt = mem_Malloc(nmemb * size); 
    if(_pt == NULL){
		return NULL;
    }

    mem_set(_pt,0,nmemb * size);
    return _pt;        
}

extern void dana_free(void *ptr)
{
    mem_Free(ptr);
}

extern void *dana_memcpy(void *dest, const void *src, size_t n)
{
    mem_copy(dest,(void *)src,n);
    return dest;
}

extern void *dana_memset(void *s, int c, size_t n)
{
     mem_set(s, c, n);
     return s;
}

extern uint32_t dana_strlen(const char *s)
{
    return strlen(s);
}

extern int32_t dana_atoi(const char *s)
{
    return atoi(s);
}

extern int32_t dana_vsnprintf(char *s, uint32_t size, const char *template, va_list ap)
{
    return vsnprintf(s,size,template, ap);
}


//#define dana_printf(...) Report(__VA_ARGS__)
//#define dana_printf Report
#if 1
extern void  dana_printf(char *pcFormat,...) 
{
 int iRet = 0;
#ifndef NOTERM

  char *pcBuff, *pcTemp;
  int iSize = 256;
 
  va_list list;
  pcBuff = (char*)malloc(iSize);
  if(pcBuff == NULL)
  {
      return ;
  }
  while(1)
  {
      va_start(list,pcFormat);
      iRet = vsnprintf(pcBuff,iSize,pcFormat,list);
      va_end(list);
      if(iRet > -1 && iRet < iSize)
      {
          break;
      }
      else
      {
          iSize*=2;
          if((pcTemp=realloc(pcBuff,iSize))==NULL)
          { 
              Message("Could not reallocate memory\n\r");
              iRet = -1;
              break;
          }
          else
          {
              pcBuff=pcTemp;
          }
          
      }
  }
  Message(pcBuff);
  free(pcBuff);
  
#endif
}
#endif



