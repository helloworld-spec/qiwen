/************************************************************************
* pwm demo:
* -d:device_no(1-5),
1 means that gpio80 as pwm1,system time ,quit using;
2 means that gpio69 as pwm2,
3 means that gpio5 as pwm3,
4 means that gpio66 as pwm4,
5 means that gpio47 as pwm5.
other value is invalid


*-m:numerator of duty,if using percentage,m need larger 0 and lower 100,else range of m is 1 to 66535
*-n:denominator of duty,if n is 100 we use percentage,or n is 65536
*duty = m / n;such as 50%,m=50, n=100;for 10/65536,m=10,m=65536

************************************************************************/
#include <stdio.h>
#include <unistd.h>

#include "ak_common.h"
#include "ak_drv_pwm.h"

static void usage(const char *name)
{
	ak_print_normal("Usage: %s [options]\n\n"
		"options:\n"
		"-d device_no(1-5)\n"
		"-f frequence(92hz~6Mhz)\n"			
		"-m duty numerator (1-65535 or 1~99)\n"
		"-n duty denominator (65536 or 100)\n"
		"exp: %s -d 1 -f 1000 -m 10 -n 65536\n", name, name);
}

int main (int argc, char **argv)
{
	void *handle = NULL;
	int ret = 0;
    int ch = 0;
	int d_value = 0;
    int f_value = 0;
    int num_value = 0;
    int den_value = 0;
	int d_flag=0;
    int f_flag=0;
    int num_flag=0;
    int den_flag=0;
    int count = 100;

	ak_print_normal("pwm test start.\n");
	if (argc < 4){
		usage(argv[0]);  	
		return -1;
	}	
	
	while ((ch = getopt(argc,argv,"d:f:m:n:")) != EOF){
		switch(ch){
		case 'd':	
			sscanf(optarg,"%d",&d_value);
			ak_print_normal("d_value : %d\n", d_value);
			d_flag=1;
			break;
			
		case 'f':
			sscanf(optarg,"%d",&f_value);
			ak_print_normal("f_value : %d\n", f_value);
			f_flag=1;
			break;
			
		case 'm':
			sscanf(optarg,"%d",&num_value);
			ak_print_normal("num_value : %d\n", num_value);
			num_flag=1;
			break;
		case 'n':
			sscanf(optarg,"%d",&den_value);
			ak_print_normal("den_value : %d\n", den_value);
			den_flag=1;
			break;	
		default:
			usage(argv[0]);
			return 0;
			break;
		}
	}

    if( d_value > 4 || d_value <2) {
        ak_print_error("open pwm device should be 2-4.\n");
        return -1;
    }
        
    if((f_value < 92) || (f_value > 6*1000*1000)){
		ak_print_error("frequence should be 92~6*1000*1000.\n");
		return -1;
	}
    if( 65536 == den_value){
        if((num_value < 1) || (num_value > 65535)){
                ak_print_error("open duty cycle should be 1-65535.\n");
                return -1;
            }
    }else if(100 == den_value){
             if((num_value < 1) || (num_value > 99)){
                ak_print_error("open duty cycle should be 1-99.\n");
                return -1;
            }
    }else{
            ak_print_error("duty denominator should be 100 or 65536.\n");
            return -1;
    }
	
    while(count-- > 0){
	    if(d_flag && f_flag && num_flag && den_flag){
		    handle = ak_drv_pwm_open(d_value);
		    if(!handle){
			    ak_print_error("open pwm device fail.\n");
			    return -1;
		    }
		    ak_print_normal("pwm open succes\n");
		    ret = ak_drv_pwm_set(handle, f_value, num_value, den_value);
		    if(ret<0){
			    ak_print_error("set pwm frq duty cycle fail.\n");
			    return -1;
		    }
	    }else {
		    usage(argv[0]);
		    return -1;
	    }
	    ak_print_normal("pwm set succes\n");

	    sleep(10);

	    ak_drv_pwm_close(handle);
	    ak_print_normal("pwm test end.\n");
    }
    return 0;
}
