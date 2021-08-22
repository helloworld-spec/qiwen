#include "ak_common.h"
#include "ak_sd_card.h"


/**
 * ak_sd_get_status - get system sd status
 * @void
 * return: the sd card status, may multi status combination
 */
int ak_sd_get_status(void)
{
    return SD_STATUS_MOUNTED |SD_STATUS_CARD_INSERT;
}
/**
 * sd_check_insert_status - check SD card insert status
 * @void
 * @return: SD card insert status
 * 		SD_STATUS_CARD_INSERT or SD_STATUS_CARD_REMOVED
 */
int ak_sd_check_insert_status(void)
{
	
    return SD_STATUS_CARD_INSERT;
}

/**
 * ak_sd_check_mount_status - check SD card mount status
 * @void
 * @return: 0 sd mounted; -1 sd unmount
 */
int ak_sd_check_mount_status(void)
{
   	
	return 0;
}


