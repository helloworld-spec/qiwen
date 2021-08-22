#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "ak_common.h"
#include "ak_drv_ptz.h"

static void usage(const char *name)
{
	ak_print_normal("Usage: %s [options]\n\n"
		"options:\n"
		"-m mode number1 number2 \n"
		"if mode==0, number1 means  direct,number2 means  degree .\n"
		"if mode==1, number1 means horizontal pos of object,number2 means vertical pos of object.\n"
		"-h   help\n",name);
}

static void check_ptz_status(void)
{
    enum ptz_status status[PTZ_DEV_NUM] = {PTZ_WAIT_INIT};

    do{
    	ak_sleep_ms(1000);
    	ak_drv_ptz_get_status(PTZ_DEV_H, &status[PTZ_DEV_H]);
    	ak_drv_ptz_get_status(PTZ_DEV_V, &status[PTZ_DEV_V]);
    } while((status[PTZ_DEV_H] != PTZ_INIT_OK)
        || (status[PTZ_DEV_V] != PTZ_INIT_OK));
}

int main (int argc, char **argv)
{
	int turn_mode = 0,turn_direct = 0,degree = 0;
	int horizontal_pos = 0, vertical_pos = 0;
	int ch;

	ak_print_normal_ex(" compile:%s %s version:%s\n",
			__DATE__,__TIME__,ak_drv_ptz_get_version());

	while ((ch = getopt(argc,argv,"m:h")) != EOF){
		switch(ch){
			case 'm':
				turn_mode = atoi(optarg);
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
	/*for(int i=0; i< argc; i++)
		ak_print_normal_ex("%d %s \n",i,argv[i]);
	*/
	if(argc != 5){
		usage(argv[0]);
		return 0;
	}
	if(0 == turn_mode ) {
		turn_direct = atoi(argv[3]);
		degree= atoi(argv[4]);
	}
	if(1 == turn_mode ) {
		horizontal_pos = atoi(argv[3]);
		vertical_pos = atoi(argv[4]);
	}
    ak_print_normal_ex("ptz test start.\n");
    if(-1 == ak_drv_ptz_open()){
		ak_print_error("ak_drv_ptz_open fail.\n");
		return -1;
    }

	ak_drv_ptz_set_angle_rate(24/24.0, 21/21.0);
	ak_drv_ptz_set_degree(350, 130);
	ak_drv_ptz_check_self(PTZ_FEEDBACK_PIN_NONE);

    check_ptz_status();

	if( 0 == turn_mode )
    	ak_drv_ptz_turn(turn_direct, degree);

    if( 1 == turn_mode )
		ak_drv_ptz_turn_to_pos(horizontal_pos, vertical_pos);

	check_ptz_status();

    ak_drv_ptz_close();
    ak_print_normal("ptz test end.\n");

	return 0;
}
