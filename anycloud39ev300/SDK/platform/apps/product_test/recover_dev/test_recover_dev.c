
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <linux/input.h>
#include "ak_common.h"
#include "ak_drv_key.h"

int press_key = 0;
struct key_event key;

/**
 * test_recover_dev - test recover key function.  
 * @sec[IN]: time limit wait key pressed, second      
 * return:  0, sucess; -1, failed
 */
int test_recover_dev(int sec)
{
	void * handle = NULL;
	int ret = -1;
	int wait_time = sec;

	/* wait time shuld > 0 */
	if( wait_time <= 0) {
		ak_print_error("wait time should be over 0 second.\n");		
		return -1;
	}
    ak_print_normal("key test start.\n");
	/* open key driver and get drv key handle */
    handle = ak_drv_key_open();
    if(handle == NULL){
  	  ak_print_error_ex("ak_drv_key_open fail.\n");
	  return -1;
  	}		
	memset(&key,0,sizeof(key));
	/*
	  * wait time, second 
	  * should repeat to check
	  */
	for(int i = 0; i < wait_time; i++){
       ak_print_normal("ak_drv_key_get_event timeout 1000 ms.\n"); 
	   memset(&key,0,sizeof(key));
       ret = ak_drv_key_get_event(handle, &key,1000);
	   if(ret == 0){	   	
           ak_print_normal("get key:%d stat:%d(1==pressed,2==release).\n",key.code,key.stat); 
			/* if key is not recover key, check it again */
			if(!(key.code == KEY_0 || key.code == KEY_1))
				continue;
			/* if key is  recover key, break */
			if (key.stat == PRESS) {
				 ak_print_normal("key pressed.\n");
				press_key = 1;
				ret = 0;
				break;
			}
		}
	}
	/* close key driver */	
    ak_drv_key_close(handle);		
    ak_print_normal("key test end.\n");	
	
	return ret;
}

int test_get_press_key(void)
{
	return press_key;
}

void test_reset_press_key(void)
{
	press_key = 0;
}


