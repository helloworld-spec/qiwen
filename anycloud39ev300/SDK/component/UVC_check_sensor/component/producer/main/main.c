#include "anyka_types.h"
#include "anyka_bsp.h"
#include "drv_api.h"
#include "Fwl_osMalloc.h"
#include "version.h"
#include "stdarg.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

  
#define DBG_TRACE_LOGLEN        2048

static unsigned char pp_printf_buf[DBG_TRACE_LOGLEN];


signed long pp_printf(unsigned char * s, ...)
{
    unsigned long slen = 0, i = 0;
    va_list     args;

    va_start(args, s);
   // vsprintf(pp_printf_buf, DBG_TRACE_LOGLEN, (const char*)s, args);  
    vsprintf(pp_printf_buf, (const char*)s, args);   
   // puts(pp_printf_buf);

    slen = strlen(pp_printf_buf);
    for(i = 0; i < slen; i++)
    {
        if (pp_printf_buf[i] == '\n')
            putch('\r');
        putch(pp_printf_buf[i]);
    }
    va_end(args); 

    return 0;
}

void CMain()
{
    T_DRIVE_INITINFO drv_info;
    unsigned long asic_freq;

    Fwl_MallocInit();

    drv_info.chip = CHIP_3918E; 

    drv_info.fRamAlloc = (void *)Fwl_MallocAndTrace;
    drv_info.fRamFree = (void *)Fwl_FreeAndTrace;
    drv_init(&drv_info);

    //timer_init();
    MMU_Init(_MMUTT_STARTADDRESS);

    console_init(uiUART0, CONSOLE_UART, 115200);

    pp_printf("chip type:AK39XX\r\n");

    
    pp_printf("producer version:%d.%d.%02d\r\n", MAIN_VERSION, SUB1_VERSION, SUB2_VERSION);

    asic_freq = get_asic_freq();

    pp_printf("asic: %d\n", asic_freq);
    

    pp_printf("pll: %dMhz\n", get_peri_pll());

    //event loop
    pp_printf("\r\nEnter transc handle ...... \r\n");
    
    Prod_Transc_Main();
	
}

//for drvlib, otherwise cannot compile
void lcdbl_set_brightness()
{
}

