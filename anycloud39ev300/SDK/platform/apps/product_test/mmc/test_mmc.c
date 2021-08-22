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
#include <stdint.h>
#include <sys/vfs.h>
#include "ak_common.h"

#define SDP1_DEV_NAME       "/dev/mmcblk0p1"
#define SD_DEV_NAME         "/dev/mmcblk0"
#define SD_COMM_NAME		"mmcblk0"

#define MOUNT_SDP1			"mount -rw /dev/mmcblk0p1 /mnt" 
#define MOUNT_SD			"mount -rw /dev/mmcblk0 /mnt" 
#define UMOUNT_SD			"umount /mnt"


static uint64_t mmc_total_size = 0;

static int do_syscmd(char *cmd, char *result)
{
	char buf[1024];	
	FILE *filp;

	filp = popen(cmd, "r");
	if (NULL == filp){
		ak_print_error_ex(" popen fail!\n");
		return -2;
	}

	memset(buf, '\0', sizeof(buf));
	fread(buf, sizeof(char), sizeof(buf)-1, filp);

	sprintf(result, "%s", buf);

	pclose(filp);
	return strlen(result);	
}

/**
 * mount_sd - mount card.    
 * return:  void
 */
static void mount_sd(int flag)
{
	char cmd[128];

	if (flag)
		sprintf(cmd, "%s", MOUNT_SDP1);
	else
		sprintf(cmd, "%s", MOUNT_SD);

	system(cmd);
}

/**
 * have_mmc - check card exist.    
 * return:  0 or 1, card exist; -1, no card
 */
static int have_mmc(void)
{
	int ret = -1;

	if (access (SDP1_DEV_NAME, F_OK) == 0)
		ret = 1;//mount_sd(1);
	else if (access (SD_DEV_NAME, F_OK) == 0)
		ret = 0;//mount_sd(0);

	return ret;
}

/**
 * check_mount - get mount result.    
 * return:  0, not mounted; 1, mounted
 */
static int check_mount(void)
{
	char *p;
	char buf[1024];

	do_syscmd("mount", buf);
	p = strstr(buf, SD_COMM_NAME);
	if (!p)
		return 1;

	return 0;
}

/**
 * mount_mmc - mount card and get result.    
 * return:  0, not mounted; 1, mounted; -1, failed
 */
static int mount_mmc(int stat)
{
	int ret = -1;

	switch (stat) {
	case 0:
	case 1:
		mount_sd(stat);
		ret = check_mount();
		break;
	default:
		break;
	}

	return ret;
}

/**
 * umount_mmc - umount card.    
 * return:  0
 */
static int umount_mmc(void)
{
	system("sync");
	system(UMOUNT_SD);

	return 0;
}
#if 0
static void save_stat(char *file, int stat)
{
	char buf[32];

	sprintf(buf, "echo \"%d\" > %s", stat, file);
	system(buf);
	
}
#endif

/**
 * get_sd_size - get card size and save it to file.    
 * return:  void
 */
void get_sd_size(void)
{
	/*
	 * get sd card  size
	 */
	struct statfs disk_statfs;
	uint64_t total_size;

	bzero(&disk_statfs, sizeof(struct statfs));
	while (statfs("/mnt/", &disk_statfs) == -1) {
		if (errno != EINTR) {
			ak_print_error_ex("error occur on statfs /mnt, %s\n", strerror(errno));
			return;
		}
	}
	total_size = disk_statfs.f_bsize;
   	total_size = total_size * disk_statfs.f_blocks;

	mmc_total_size = total_size;

	ak_print_normal_ex("sd-card total size: %lld B\n",total_size);
	total_size = total_size >> 10;

	ak_print_normal_ex("sd-card total size: %lld KB\n",total_size);
	
}

uint64_t test_mmc_get_total_size(void)
{
	return mmc_total_size;
}


/**
 * test_mmc - test mmc function.    
 * return:  0, sucess; -1, failed
 */
int test_mmc( void)
{
	int stat;
	int count = 10;
	
    mmc_total_size = 0;

	while ( count-- > 0) {
		/* check if have card */
		stat = have_mmc();
		switch (stat) {
		case 0:
		case 1:
			/* mount card */
			if (mount_mmc(stat)) {
				ak_print_normal_ex("mount mmc card error.\n");				
				return -1;
			}
			/* get card size */
			get_sd_size();
			ak_print_normal_ex("mount mmc card ok.\n");
			/* unmount card */
			umount_mmc();
			
			return 0;
			break;
		default:
			break;
		}
        
		usleep(500);
	}

	ak_print_normal_ex("not have mmc card.\n");
	return -1;
}
