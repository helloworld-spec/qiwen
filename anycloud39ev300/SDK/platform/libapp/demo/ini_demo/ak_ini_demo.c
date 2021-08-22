#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "ak_common.h"
#include "ak_ini.h"

static void usage(const char *name)
{
	ak_print_normal("Usage: %s [options]\n\n"
		"options:\n"
		"-f configure file \n"
		"-h   help\n",name);
}

int main (int argc, char **argv)
{
	void *cfg_handle = NULL;
	char cfg_file[100];
	char str[100];
	int ch;

	ak_print_normal("ini_demo test start.\n");
	ak_print_normal("ini demo compile:%s %s %s\n",__DATE__,__TIME__,ak_ini_get_version());

	/* parse argument */
	while ((ch = getopt(argc,argv,"f:h")) != EOF){
		switch(ch){
			case 'f':
			case 'F':
				strncpy(cfg_file,optarg,100);
				break;
			case 'h':
			case 'H':
			default:
				usage(argv[0]);
				return 0;
				break;
		}

	}

	ak_print_normal("%s\n",cfg_file);
	/* init cfg_file and get handle */
	if((cfg_handle = ak_ini_init(cfg_file)) == NULL){
		ak_print_error("ak_ini_open fail.\n");
		return -1;
	}
	/* test get item function and item is exist */
	if(ak_ini_get_item_value(cfg_handle, "ethernet", "ipaddr", str) != 0){
		ak_print_error("ak_cfg_get_item ethernet->ipaddr fail.\n");
	}else
	   ak_print_normal("get ethernet->ipaddr:%s\n",str);
	/* test set item function  and item  not exist */
	if(ak_ini_set_item_value(cfg_handle, "net", "ipaddr", "192.168.1.18") != 0){
		ak_print_error("ak_ini_set_item_value net->ipaddr fail.\n");

	}else
		ak_print_normal("ak_ini_set_item_value net->ipaddr ok.\n");
	/* test get item function and item is set before */
	if(ak_ini_get_item_value(cfg_handle, "net", "ipaddr", str) != 0){
		ak_print_error("ak_cfg_get_item net->ipaddr fail.\n");
	}else
		ak_print_normal("get net->ipaddr:%s\n",str);
	/* test set item function and item is exist */
	if(ak_ini_set_item_value(cfg_handle, "net", "ipaddr", "168.168.68.168") != 0){
		ak_print_error("ini_set_item_value net->ipaddr fail.\n");
	}else{
		ak_print_normal("ini_set_item_value net->ipaddr ok.\n");
	}
	/* test get item function and item is exist */
	if(ak_ini_get_item_value(cfg_handle, "net", "ipaddr", str) != 0){
		ak_print_error("ak_cfg_get_item net->ipaddr fail.\n");
	}else{
	   ak_print_normal("get net->ipaddr:%s\n",str);
	}

	/* free ini handle resource */
	ak_ini_destroy(cfg_handle);
	ak_print_normal("ini_demo test end.\n");

	return 0;
}
