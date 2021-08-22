/**
 * @file 
 * @brief:
 *
 * This file provides 
 * Copyright (C) 2017 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author 
 * @date 2017-2-09
 * @version 1.0
 */

#include "anyka_types.h"

#include "platform_devices.h"
#include "device.h"

#ifdef __cplusplus
extern "C"{
#endif


extern int __dev_initcall_start, __dev_initcall_end;
static int *initcall_start=&__dev_initcall_start, *initcall_end=&__dev_initcall_end;



static T_PLATORM_DEV_INFO  *platform_dev_info_table[PLATFORM_DEV_MAX_NB]= {0,};


/** 
 * @brief  regoster  devcies. after the uart0 initializtion 
 *
 * @author KeJianping
 * @date 2017-2-09
 * @return  void
 */
void dev_do_register(void)
{
    dev_initcall_t *call;
    int *addr;

    for (addr = initcall_start; addr < initcall_end; addr++) 
    {
        call = (dev_initcall_t *)addr;
        (*call)();    
    }
}

/** 
 * @brief  get platform devices informaiton
 *
 * @author KeJianping
 * @date 2017-2-09
 * @param dev_name[in] devcie's name,
 * @return  void *
 * @retval  NULL:  failed
 * @retval  NOT NULL: successful
 */
void *platform_get_devices_info(char *dev_name)
{

	int i;
	void *devcie = NULL;
	
	for (i = 0; i < PLATFORM_DEV_MAX_NB; i++) 
	{
		
		if (NULL == platform_dev_info_table[i])
		{
			break;
		}
		
		if (0 == strcmp(platform_dev_info_table[i]->dev_name,dev_name))
		{
			devcie = platform_dev_info_table[i]->devcie;
			break;
		}
		
	}

	return devcie;

}


/** 
 * @brief  add platform devices informaiton
 *
 * @author KeJianping
 * @date 2017-2-09
 * @param platform_dev[in]  platform devcies 
 * @param num[in] the platform devcie count 
 * @return  void 
 */
void platform_add_devices_info(T_PLATORM_DEV_INFO **platform_dev, int num)
{
	int i;

	if(num > PLATFORM_DEV_MAX_NB)
	{
		while(1);
	}

	for (i = 0; i < num; i++) 
	{	
		platform_dev_info_table[i]  = platform_dev[i];
	}

	//
	for (; i < num; i++) 
	{	
		platform_dev_info_table[i]          = NULL;
	}

}



#ifdef __cplusplus
}
#endif



