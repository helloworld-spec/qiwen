#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "record_common.h"
#include "ak_common.h"
#include "ak_cmd_exec.h"

#ifndef _LARGEFILE64_SOURCE
#define _LARGEFILE64_SOURCE
#endif
#include <time.h>
#include <fcntl.h>
#include <sys/statfs.h>

#include "anyka_types.h"
#include "mtdlib.h"
#include "file.h"
#include "driver.h"
#include "globallib.h"
#include "global.h"

#define DEV_MMCBLK0   "/dev/mmcblk0"
#define DEV_MMCBLK0P1 "/dev/mmcblk0p1"
#define LEN_PATH_FILE 255
#define FAT_FIX_TEMPLATE "A:%s"
#define MSEC2NSEC      1000000
#define CMD_MMC_FS_FIX      "mmc_test -F"                                                       //用于检查和修复tf卡文件系统
#define LEN_CMD       100
#define LEN_RES       2048

enum fixstatus {
	FIX_STATUS_ERROR   = -1 ,
	FIX_STATUS_NONEED  = 0 ,
	FIX_STATUS_SUCCESS = 1 ,
};

#define PATH_MOUNT_POINT        "/mnt"

/**
 * ak_dvr_repair_ro_disk - try to repair readonly SD card
 * @bad_file[IN]: bad file full path, including file name
 * return: 0 success, -1 failed
 * notes:
 */
int ak_dvr_repair_ro_disk( )                                                    //重新修复T卡
{
	char cmd[LEN_CMD] = {0};
	char *pc_dev = NULL ;
	char res[LEN_CMD];

	if ( ak_is_dev_file( DEV_MMCBLK0P1 ) == AK_TRUE ) {                         //get tf card dev name. dev name should be /dev/mmcblk0 or /dev/mmcblk0p1
		pc_dev = DEV_MMCBLK0P1 ;
	}
	else if ( ak_is_dev_file( DEV_MMCBLK0 ) == AK_TRUE ) {
		pc_dev = DEV_MMCBLK0 ;
	}
	
	//ak_print_normal_ex("%s\n", res);

	if ( pc_dev != NULL ) {
		snprintf(cmd, LEN_CMD, "umount -lf %s", PATH_MOUNT_POINT);
		ak_cmd_exec(cmd, res, LEN_CMD);
		sync();		
		system( CMD_MMC_FS_FIX );
		sync();
		snprintf(cmd, LEN_CMD, "mount %s %s", pc_dev , PATH_MOUNT_POINT);       //重新mount文件系统
		ak_cmd_exec(cmd, res, LEN_CMD);
		return AK_SUCCESS;
	}
	else {
		return AK_FAILED;
	}
}
