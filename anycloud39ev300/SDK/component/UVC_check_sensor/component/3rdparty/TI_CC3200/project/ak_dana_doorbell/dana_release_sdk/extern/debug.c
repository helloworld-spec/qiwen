

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <time.h>
#include <errno.h>
#include <string.h>

#include "uart_if.h"
#include "debug.h"

#include "dana_mem.h"
#include "dana_time.h"

static bool dana_debug = false;

void dbg_on() 
{
    dana_debug = true;
    return;
}

void dbg_off()
{
    dana_debug = false;
    return;
}

void dbg(const char *msg, ...)
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
		  va_start(list,msg);
		  iRet = vsnprintf(pcBuff,iSize,msg,list);
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
          Message("\r\n");
	  free(pcBuff);
	  
#endif
	  return ;
}


