#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "ak_common.h"
#include "ak_drv_ir.h"

static inline void usage(const char *app)
{
	ak_print_normal("Usage: %s [options]\n\n"
		"options:\n"
		"-s value (set ircut mode.can be: 0 or 1)\n"
		"-g  (get ir value) \n"
		"-h   help\n",app);	
}

int main (int argc, char **argv)
{
	int ch,s_value = 0,value = 0,s_func = 0,g_func = 0;

	ak_print_normal("ir test start.\n");
	if (argc < 2){
		usage(argv[0]);  	
		return -1;
	}	
	while ((ch = getopt(argc,argv,"s:gh")) != EOF){
		switch(ch){
			case 's':
				s_value = atoi(optarg);
				s_func = 1;
				break;
			case 'g':
				g_func = 1;
				break;
			case 'h':
			case 'H':				
			default:
				usage(argv[0]);
				return 0;
				break;
		}
		//ak_print_normal("%s\n",optarg);
	}
	/*  s_value shoud be
	  IR_NIGHT = 0,  or
	  IR_DAYTIME,
	*/
	if(s_value < 0 || s_value > 1){
		usage(argv[0]);
		return 0;
	}
	
	ak_drv_ir_init();
	if(	g_func ){
		value = ak_drv_ir_get_input_level();
		ak_print_normal(" get ir value:%d.\n",value);
	}		
	if(	s_func ){
		ak_drv_ir_set_ircut( s_value) ;
		ak_print_normal(" set ircut value:%d.\n",s_value);
	}	    
	ak_print_normal("ircut test end.\n");
	
	return 0;
}
