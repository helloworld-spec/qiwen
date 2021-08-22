#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <unistd.h>
#include "ak_common.h"

static int do_syscmd(char *cmd, char *result)
{
	char buf[128];	
	FILE *filp;

	filp = popen(cmd, "r");
	if (NULL == filp){
		return -2;
	}

	memset(buf, '\0', sizeof(buf));
	fread(buf, sizeof(char), sizeof(buf)-1, filp);

	sprintf(result, "%s", buf);

	pclose(filp);
	return strlen(result);	
}

/**
 * test_pushid - set  uid  file
 * @uid[IN]: uid str 
 * @data[IN]: uid file data
 * @datalen[IN]: uid file data len
 * @dest_file[IN]: uid dest file path
 * return:   0 , success ;  -1 , failed;
 */
int test_pushid(char *uid, char *data, int datalen, char *dest_file)
{
	char res[128] = {0};
	char cmd[128] = {0};
	char config_des[128] = {0};
	int ret = -1;

	if (NULL == uid || NULL == data || NULL == dest_file || datalen <= 0) 
		return ret;

	ak_print_normal_ex("uid: %s, dest_file: %s\n", uid, dest_file);
	sprintf(config_des, "/etc/jffs2/%s", dest_file);

	/*
	  * if dest file exist, rm it 
	  * normal, it should not exist
	  */
	if(access(config_des, F_OK) == 0)
	{
		sprintf(cmd, "rm -f %s", config_des);
		do_syscmd(cmd, res);
		
		if(res[0] != 0)
		{
			ak_print_error_ex("do syscommond %s failed, return val : %s\n", cmd, res);
			goto ERR;
		}
		bzero(res, sizeof(res));
		bzero(cmd, sizeof(cmd));
	}
	
    /* update uid file */
	FILE *file = fopen(config_des, "w+");

	if(NULL == file)
	{
		ak_print_error_ex("%s open failed!\n", config_des);
		goto ERR;
	}
	
	fwrite(data, 1, datalen, file);
	fclose(file);


	/*
	 * store uid info to file
	 * when pc tool need it, platform upload info.
	 */
	do_syscmd("rm -f /etc/jffs2/custom_uid.txt", res);
	bzero(cmd, sizeof(cmd));
	sprintf(cmd, "echo %s > /etc/jffs2/custom_uid.txt", uid);
	do_syscmd(cmd, res);
	if(res[0] != 0)
	{
		ak_print_error_ex("do syscommond %s failed, return val : %s\n", cmd, res);
		goto ERR;
	}
	/* sync to disk */
	//system("sync");

	ret = 0;
	ak_print_normal_ex(" exit\n");
ERR:
	
	return ret;
}
