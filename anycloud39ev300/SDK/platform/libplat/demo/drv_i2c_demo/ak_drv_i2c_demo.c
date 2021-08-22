#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "ak_common.h"
#include "ak_drv_i2c.h"

static void usage(const char *name)
{
	ak_print_normal("Usage: %s [options]\n\n"
		"options:\n"
		"-d device_addr(select i2c device)\n"			
		"-r register(read device register)\n"
		"-w register -v value(write device register)\n"
		"exp: %s -d 0x60 -r 0x3107 \n", name, name);
}

int main (int argc, char **argv)
{
	void *handle = NULL;
	int ret, ch;
	int d_value, v_value, r_value, w_value;
	int d_flag=0, r_flag=0, w_flag=0, v_flag=0;

	unsigned char read_value=0;

	ak_print_normal("i2c test start.\n");
	if (argc < 3){
		usage(argv[0]);  	
		return -1;
	}	
	
	while ((ch = getopt(argc,argv,"d:r:w:v:h")) != EOF){
		switch(ch){
		case 'd':	
			sscanf(optarg,"%x",&d_value);
			d_flag=1;
			break;
			
		case 'r':
			sscanf(optarg,"%x",&r_value);
			r_flag=1;
			break;
			
		case 'w':
			sscanf(optarg,"%x",&w_value);
			w_flag=1;
			break;
			
		case 'v':
			sscanf(optarg,"%x",&v_value);
			v_flag=1;
			break;

		case 'h':				
		default:
			usage(argv[0]);
			return 0;
			break;
		}
	}

	if(d_flag && r_flag){
		handle = ak_drv_i2c_open(((char)d_value&0xff)>>1);
		if(!handle){
			ak_print_error("open i2c device fail.\n");
			return -1;
		}
		
		ret = ak_drv_i2c_read(handle, r_value, &read_value, 1);
		if(ret<0){
			ak_print_error("read i2c device addr: 0x%x fail.\n", r_value);
			return -1;
		}
		ak_print_normal("read 0x%x reg : 0x%02x\n", r_value, read_value);
		
	} else if(d_flag && w_flag && v_flag){
		handle = ak_drv_i2c_open(((char)d_value&0xff)>>1);
		if(!handle){
			ak_print_error("open i2c device fail.\n");
			return -1;
		}
		ret = ak_drv_i2c_write(handle, w_value, (unsigned char*)&v_value, 1);
		if(ret<0){
			ak_print_error("write i2c device addr: 0x%x fail.\n", w_value);
			return -1;
		}
		ak_print_normal("write data success!\n");
	}
	
    return 0;
}
