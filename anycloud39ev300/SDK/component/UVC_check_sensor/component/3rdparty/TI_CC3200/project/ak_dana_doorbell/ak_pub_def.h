
#ifndef __AK_PUB_DEF_H__
#define __AK_PUB_DEF_H__



#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hw_types.h"
#include "pin.h"
#include "common.h"
#include "udma.h"

#include "hw_memmap.h"
#include "hw_common_reg.h"
#include "hw_ints.h"
#include "rom.h"
#include "rom_map.h"



/* Peripheral interface includes. */
//#include "udma_if.h"
#include "uart.h"
#include "prcm.h"


/* free_rtos/ti-rtos includes */ 
#include "osi.h"
#ifdef USE_FREERTOS
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#endif

/* Simplelink includes */
#include "SimpleLink.h"


/* common interface includes */
#ifndef NOTERM
#include "uart_if.h"
#endif


// 实测 1 有效  但0无效  
#define SLEEP_MS(d_ms) osi_Sleep((d_ms == 0) ? 1 : d_ms)


/* task priority */
#define TASK_MAIN_TASK_PRIORITY       1
#define TASK_NETRECV_PRIORITY         2
#define TASK_NETSEND_PRIORITY         4 // 3
#define TASK_SPI_FR_AK39E_PRIORITY    3 // 4
#define TASK_UART1_FR_AK39E_PRIORITY  5
#define TASK_DISCAST_PRIORITY         6

#define TASK_UDPTASK_PRIORITY         7


#define TASK_SL_SPAWN_PRIORITY        9

#define AK_POWER_OFF				  0
#define AK_POWER_ON                   1

#define AK_CMD_POWER_ON               "akon"
#define AK_CMD_POWER_OFF              "akoff"
#define AK_CMD_START_VIDEO            "start"
#define AK_CMD_STOP_VIDEO             "stop"
#define AK_CMD_WIFI_TX_TEST			  "wifitx"
#define AK_CMD_CONFIG                 "config"
#define AK_CMD_CONFIG_SHOW            "showcfg"
#define AK_CMD_SET_SSID				  "ssid="
#define AK_CMD_SET_KEY				  "key="
#define AK_CMD_SET_SERVER			  "server="
#define AK_CMD_SET_PORT			      "port="
#define AK_CMD_CLEAR_CFG    	      "clearcfg"
#define AK_CMD_REBOOT       	      "reboot"

#endif  //#ifndef __AK_PUB_DEF_H__

