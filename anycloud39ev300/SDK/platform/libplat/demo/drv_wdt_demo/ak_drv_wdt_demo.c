#include <stdio.h>
#include <unistd.h>

#include "ak_common.h"
#include "ak_drv_wdt.h"

static inline void usage(const char *app)
{
	ak_print_normal("Usage: %s [options]\n\n"
		"options:\n"
		"-n   test normal\n"
		"-r   test reboot(dog hungry) \n"
		"-h   help\n",app);	
}

int main (int argc, char **argv)
{
	int ch,dogtime = 1;
	
	ak_print_normal("wdt test start.\n");
 	if (argc != 2){
		usage(argv[0]);  	
		return -1;
	}
	
	while ((ch = getopt(argc,argv,"nrh")) != EOF){
		switch(ch){			
			case 'n':
				/*
				 * feed dog time is 2 and hungry time is 5.
				 * watchdog do not dead.
				 */
				dogtime = 2;
				break;
			case 'r':
				/*
				 * feed dog time is 6 and hungry time is 5.
				 * watchdog dead.
				 */
				dogtime = 6;
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

	/* dog hungry time is 5. */
	ak_drv_wdt_open(5);
	for(int i = 0; i < 4; i++){
		ak_print_normal("feed dog\n");
		ak_drv_wdt_feed();
		sleep(dogtime);
	}
	/* close watch dog */
	ak_drv_wdt_close();
	ak_print_normal("wdt test end.\n");	
	
	return 0;
}
