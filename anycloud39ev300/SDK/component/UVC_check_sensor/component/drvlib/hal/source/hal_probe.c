/**
 * @file hal_probe.c
 * @brief device probe framework
 * Copyright (C) 2010 Anyka (Guangzhou) Microelectronics Technology Co., Ltd
 * @author guoshaofeng 
 * @date 2010-12-07
 * @version 1.0
 * @ref
 */
#include "anyka_cpu.h"
#include "anyka_types.h"
#include "drv_api.h"
#include "hal_probe.h"

#define MAX_KEYPAD_SUPPORT  20  //max support keypad module number

#ifndef BURNTOOL

static T_CAMERA_INFO CAMERA_INFO_TABLE[CAMERA_MAX_SUPPORT] = {0};
static T_KEYPAD_TYPE_INFO REG_TABLE[MAX_KEYPAD_SUPPORT] = {0};


/**
 * @brief camera probe pointer
 * @author xia_wenting
 * @date 2010-12-07
 * @param
 * @return T_CAMERA_FUNCTION_HANDLER camera device pointer
 * @retval
 */
T_CAMERA_FUNCTION_HANDLER *cam_probe(void)
{
    unsigned long i, id, mclk;
		
    for (i = 0; i < CAMERA_MAX_SUPPORT; i++)
    {
        if (CAMERA_INFO_TABLE[i].handler != NULL)
        {
        	mclk = CAMERA_INFO_TABLE[i].handler->cam_mclk;
	        //wmj- for gcc compile
	        #ifndef BURNTOOL
	        camctrl_open(mclk);
            #endif
            CAMERA_INFO_TABLE[i].handler->cam_open_func();
            id = CAMERA_INFO_TABLE[i].handler->cam_read_id_func();
			
            if (id == CAMERA_INFO_TABLE[i].DeviceID)
            {
                akprintf(C3, M_DRVSYS, "match camera, id = 0x%x\n", id);
                return CAMERA_INFO_TABLE[i].handler;
            }
            else
            {
                CAMERA_INFO_TABLE[i].handler->cam_close_func();
            }
        }
    }
  
    return NULL;
}
        
bool camera_reg_dev(unsigned long id, T_CAMERA_FUNCTION_HANDLER *handler)
{
    unsigned long i;
    bool ret = false;
    
    for (i = 0; i < CAMERA_MAX_SUPPORT; i++)
    {
        // check device register or not 
        if (CAMERA_INFO_TABLE[i].DeviceID == id)
            break;
        // got an empty place for it 
        if (CAMERA_INFO_TABLE[i].DeviceID == 0 &&
            CAMERA_INFO_TABLE[i].handler == NULL)
        {
            akprintf(C3, M_DRVSYS, "camera register id = 0x%x, cnt = %d\n", id, i);
            CAMERA_INFO_TABLE[i].DeviceID = id;
            CAMERA_INFO_TABLE[i].handler = handler;
            ret = true;
            break;
        }
    }
    return ret;
}

T_KEYPAD_HANDLE *keypad_type_probe(unsigned long index)
{
    unsigned long i;
    
    for (i = 0; i < MAX_KEYPAD_SUPPORT; i++)
    {
        if (REG_TABLE[i].index == index)
        {
            return REG_TABLE[i].handler;
        }
    }
    
    return NULL;
}

bool keypad_reg_scanmode(unsigned long index, T_KEYPAD_HANDLE *handler)
{
    unsigned long i;
    bool ret = false;
    
    for (i = 0; i < MAX_KEYPAD_SUPPORT; i++)
    {
        if ((REG_TABLE[i].index == 0) && (REG_TABLE[i].handler == NULL))
        {
            REG_TABLE[i].index = index;
            REG_TABLE[i].handler = handler;
            ret = true;
            break;
        }
    }
    return ret;
}


#endif
