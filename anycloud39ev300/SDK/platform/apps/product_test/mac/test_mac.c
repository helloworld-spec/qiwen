#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <pthread.h>
#include <linux/netlink.h>
#include <linux/version.h>
#include <linux/input.h>
#include <syslog.h>
#include <stdarg.h>
#include <unistd.h>
#include "test_fha_char.h"
#include "ak_common.h"

#define MAC_FILE "/tmp/mac.txt"

/**
 * mac_set - set mac addr    
 * @new_mac[IN]: mac addr string    
 * return:   0 , success ;  -1 , failed;
 */
static int mac_set(char *new_mac)
{
	char buf[128];
	int len = strlen(new_mac);
	int ret = 0;
	int save_fd = -1;

	ak_print_normal_ex(" MAC=%s \n", new_mac);
	/**
	 * firstly, we write mac addr info to temp file,
	 * format in file: 4bytes (length of mac) + mac addr 
	 */
	save_fd  = open( MAC_FILE ,  O_RDWR | O_CREAT | O_TRUNC );
    if(save_fd >= 0) 
		lseek( save_fd, 0, SEEK_SET );
	else {
		ak_print_error_ex("open file fail.\n");
		return -1;
	}
	ret = write(save_fd, &len, 4);
	if(ret != 4){
		ak_print_error_ex("write file fail.\n");
		close(save_fd);
		return -1;
	}
	ret = write(save_fd, new_mac, len);
	if(ret != len){
		ak_print_error_ex("write file fail.\n");
		close(save_fd);
		return -1;
	}
	close(save_fd);
	memset(buf, 0, 128);
	sprintf(buf, "MAC=%s", MAC_FILE);
	/* call partition lib to write mac addr */
	test_update_mac(buf);

	return 0;
}

/**
 * test_mac - set mac addr    
 * @macstr[IN]: mac addr string    
 * return:   0 , success ;  -1 , failed;
 */
int test_mac(char *macstr)
{
	char *mac = macstr;	
	int ret = 0;	
	ret = mac_set(mac);
	
	return ret;
}

