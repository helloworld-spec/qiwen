#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "ak_common.h"
#include "ak_drv_key.h"

struct key_event key;
static void usage(const char *name)
{
	ak_print_normal("Usage: %s [options]\n\n"
		"options:\n"
		"-t number (time to wait key input, second) \n"			
		"-h   help\n",name);
}

int main (int argc, char **argv)
{
    void * handle = NULL;
	int ret = 0;
	int wait_time = 0;//second
	int ch;
	
	ak_print_normal_ex(" compile:%s %s version:%s\n",
			__DATE__,__TIME__,ak_drv_key_get_version());
		
	while ((ch = getopt(argc,argv,"t:h")) != EOF){
		switch(ch){
			case 't':
				wait_time = atoi(optarg);
				break;
								
			case 'h':
			case 'H':				
			default:
				usage(argv[0]);
				return 0;
				break;
		}
		/* ak_print_normal("%s\n",optarg); */
	}
	/* for(int i=0; i< argc; i++)
	  *	ak_print_normal_ex("%d %s \n",i,argv[i]);
	  */
	
	if( wait_time <= 0) {
		ak_print_error("wait time should be over 0 second.\n");
		usage(argv[0]);
		return 0;
	}
    ak_print_normal("key test start.\n");
	/* open key driver and get key handle  */	
    handle = ak_drv_key_open();
    if(handle == NULL){
  	  ak_print_error_ex("ak_drv_key_open fail.\n");
	  return -1;
  	}
		
	memset(&key,0,sizeof(key));
    
	for(int i = 0; i < wait_time; i++){
       ak_print_normal("ak_drv_key_get_event timeout 1000 ms.\n"); 
	   memset(&key,0,sizeof(key));
       ret = ak_drv_key_get_event(handle, &key,1000);
	   /* get key, output */
	   if(ret == 0)
           ak_print_normal("get key:%d stat:%d(1==pressed,2==release).\n",key.code,key.stat); 
	}
	/* close key handle  */	
    ak_drv_key_close(handle);
    ak_print_normal("key test end.\n");	
    
    return 0;
}
