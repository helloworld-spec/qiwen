#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ak_common.h"
#include "ak_sd_card.h"

#define SD_DEV 			"/dev/mmcblk0"
#define SD_DEV_NODE 	"/dev/mmcblk0p1"

static unsigned int sd_status = SD_STATUS_CARD_REMOVED;

/**
 * sd_check_insert_status - check SD card insert status
 * @void
 * @return: SD card insert status
 * 		SD_STATUS_CARD_INSERT or SD_STATUS_CARD_REMOVED
 */
int ak_sd_check_insert_status(void)
{
	enum sd_card_status status = SD_STATUS_CARD_REMOVED;

	if(access(SD_DEV_NODE, F_OK) == 0) {
		status = SD_STATUS_CARD_INSERT;
	} else if (access(SD_DEV, F_OK) == 0) {
		status = SD_STATUS_CARD_INSERT;
	}

	if(SD_STATUS_CARD_INSERT == status) {
		sd_status &= (~SD_STATUS_CARD_REMOVED);
		sd_status |= status;
	} else {
		sd_status = SD_STATUS_CARD_REMOVED;
	}

	return status;
}

/**
 * ak_sd_check_mount_status - check SD card mount status
 * @void
 * @return: 0 sd mounted; -1 sd unmount
 */
int ak_sd_check_mount_status(void)
{
	FILE *fp = fopen("/proc/mounts", "r");
	if (NULL == fp) {
		perror("/proc/mounts");
		return AK_FAILED;
	}

	int result = AK_FAILED;
	char *p = NULL;
	char buf[256] = {0};

	while (NULL != fgets(buf, sizeof(buf), fp)) {
		/** check mmcblk0p1 **/
		p = strstr(buf, SD_DEV_NODE);
		if (NULL != p) {
			result = AK_SUCCESS;
			break;
		}

		/** check mmcblk0 **/
		p = strstr(buf, SD_DEV);
		if (NULL != p) {
			result = AK_SUCCESS;
			break;
		}
		memset(buf, 0x00, sizeof(buf));
	}

	if (NULL != fp) {
		fclose(fp);
		fp = NULL;
	}

	if (AK_SUCCESS == result) {
		sd_status &= (~SD_STATUS_UNMOUNT);
		sd_status |= SD_STATUS_MOUNTED;
	} else {
		sd_status &= (~SD_STATUS_MOUNTED);
		sd_status |= SD_STATUS_UNMOUNT;
	}

	return result;
}

/**
 * ak_sd_mount - mount sd card to appointed point
 * @mount_point[IN]: mount point, such as /mnt
 * @return: 0 mount success; -1 mount failed
 */
int ak_sd_mount(const char *mount_point)
{
	char cmd[255] = {0};

	if(access(SD_DEV_NODE, R_OK) == 0) {
		sprintf(cmd, "mount %s %s", SD_DEV_NODE, mount_point);
		system(cmd);
		ak_print_normal_ex("*** mount %s on %s ***\n", SD_DEV_NODE, mount_point);
	} else {
		sprintf(cmd, "mount %s %s", SD_DEV, mount_point);
		system(cmd);
		ak_print_normal_ex("*** mount %s on %s ***\n", SD_DEV, mount_point);
	}

	int ret = ak_sd_check_mount_status();
	if(AK_SUCCESS == ret) {
		ak_print_normal_ex("*** mount sd card OK ***\n");
	} else {
		ak_print_normal_ex("*** mount sd card FAILED ***\n");
	}

	return ret;
}

/**
 * ak_sd_mount - umount the sd card from appointed point
 * @mount_point[IN]: mount point, such as /mnt
 * @return: 0 umount success; -1 umount failed
 */
int ak_sd_umount(const char *mount_point)
{
	char cmd[255] = {0};

	sprintf(cmd, "umount -l %s", mount_point);
	system(cmd);

	int ret = AK_FAILED;
	if(AK_SUCCESS == ak_sd_check_mount_status()) {
		ak_print_normal_ex("*** umount the sd card FAILED ***\n");
	} else {
		ret = AK_SUCCESS;
		ak_print_normal_ex("*** umount the sd card OK ***\n");
	}

	return ret;
}

/**
 * ak_sd_set_status - set system sd status
 * @status[IN]: new sd card status
 * @mount_point[IN]: mount point, such as /mnt
 * @return: 0 success; -1 failed
 */
int ak_sd_set_status(enum sd_card_status status, const char *mount_point)
{
	int ret = AK_FAILED;

	if (mount_point != NULL) { //set status and mount
		switch(status) {
		case SD_STATUS_MOUNTED:
			ret = ak_sd_mount(mount_point);
			break;
		case SD_STATUS_UNMOUNT:
			ret = ak_sd_umount(mount_point);
			break;
		default:
			break;
		}
	} else { //only set status
		sd_status = status;
	}
	ak_print_normal_ex("*** set sd status=%d, real status=%d ***\n",
		status, sd_status);

	return ret;
}

/**
 * ak_sd_get_status - get system sd status
 * @void
 * return: the sd card status, may multi status combination
 */
int ak_sd_get_status(void)
{
	return sd_status;
}

/**
 * ak_sd_init_status - init system sd status
 * @void
 * return: the sd card status, may multi status combination
 */
void ak_sd_init_status(void)
{
	ak_sd_check_insert_status();
	if(SD_STATUS_CARD_INSERT & sd_status) {
		for (int i=0; i<5; i++) {
			if (AK_SUCCESS == ak_sd_check_mount_status()) {
				break;
			}
			ak_sleep_ms(1000);
		}
	}

	ak_print_normal_ex("sdcard status: %d\n", sd_status);
}
