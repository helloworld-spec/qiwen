#include <linux/ioctl.h>
#include <string.h>

#include "ak_isp_char.h"
#include "ak_isp_sdk.h"
#include "ak_common.h"
#include "ak_vpss.h"

static struct vpss_osd_param local_param[VPSS_OSD_CHANNELS_MAX];/*osd param ,main and sub channel*/
static int osd_enable[VPSS_OSD_CHANNELS_MAX];/*osd switch ,main and sub channel*/

/**  
 * ak_vpss_osd_set_param - set osd param to isp and then to driver
 * @vi_handle[IN]: vi module handle
 * @param[IN]: the osd param  
 * return: 0 success, otherwise failed
 */
int ak_vpss_osd_set_param(const void *vi_handle, struct vpss_osd_param *param)
{
	if (!vi_handle) {
		ak_print_error_ex("vi_handle is null\n");
		return AK_FAILED;
	}
	if (!param) {
		ak_print_error_ex("param is null\n");
		return AK_FAILED;
	}
	
	/*analysis with osd_enable , 
	if no change ,no need to set to isp and driver*/
	int do_change = 1;
	
	switch (param->id) {
	case OSD_SET_COLOR_TABLE:     
		param->id = AK_ISP_USER_CID_SET_OSD_COLOR_TABLE_ATTR;
		break;
	case OSD_SET_MAIN_CHANNEL_DATA:
		if (osd_enable[0]) {
			memcpy(&local_param[0], param, sizeof(struct vpss_osd_param));
			param->id = AK_ISP_USER_CID_SET_MAIN_CHANNEL_OSD_CONTEXT_ATTR;
		} else
			do_change = 0;
		break;
	case OSD_SET_SUB_CHANNEL_DATA:
		if (osd_enable[1]) {
			memcpy(&local_param[1], param, sizeof(struct vpss_osd_param));
			param->id = AK_ISP_USER_CID_SET_SUB_CHANNEL_OSD_CONTEXT_ATTR;
		} else
			do_change = 0;
		break;
	case OSD_SET_MAIN_DMA_MEM_REQUST:/*must call it first, otherwise can't display osd*/
		osd_enable[0] = 1;
		param->id = AK_ISP_USER_CID_SET_MAIN_CHANNEL_OSD_MEM_ATTR;
		break;
	case OSD_SET_SUB_DMA_MEM_REQUST:/*must call it first, otherwise can't display osd*/
		osd_enable[1] = 1;
		param->id = AK_ISP_USER_CID_SET_SUB_CHANNEL_OSD_MEM_ATTR;
		break;
	default:
		break;
	}

	if (do_change)
		return Ak_ISP_Set_User_Params((AK_ISP_USER_PARAM *)param);/*set to isp and then to driver*/
	else
		return AK_SUCCESS;
}

/**  
 * ak_vpss_osd_get_param - get osd param
 * @vi_handle[IN]: vi module handle
 * @param[OUT]: the osd param  
 * return: 0 success, otherwise failed
 */
int ak_vpss_osd_get_param(const void *vi_handle, struct vpss_osd_param *param)
{
	if (!vi_handle) {
		ak_print_error_ex("vi_handle is null\n");
		return AK_FAILED;
	}
	if (!param) {
		ak_print_error_ex("param is null\n");
		return AK_FAILED;
	}

	switch (param->id) {
	case OSD_SET_MAIN_CHANNEL_DATA:
		memcpy(param, &local_param[0], sizeof(struct vpss_osd_param));
		break;
	case OSD_SET_SUB_CHANNEL_DATA:
		memcpy(param, &local_param[1], sizeof(struct vpss_osd_param));
		break;
	default:
		break;
	}

	return AK_SUCCESS;
}


/**  
 * ak_vpss_osd_close - close osd to isp and then to driver
 * @vi_handle[IN]: vi module handle
 * @param[IN]: the osd param  
 * return: 0 success, otherwise failed
 */
int ak_vpss_osd_close(const void *vi_handle, struct vpss_osd_param *param)
{
	if (!vi_handle) {
		ak_print_error_ex("vi_handle is null\n");
		return AK_FAILED;
	}
	if (!param) {
		ak_print_error_ex("param is null\n");
		return AK_FAILED;
	}

	switch (param->id) {
	case OSD_SET_MAIN_CHANNEL_DATA:
		osd_enable[0] = 0;
		ak_print_notice_ex("disable main channel osd\n");
		param->id = AK_ISP_USER_CID_SET_MAIN_CHANNEL_OSD_CONTEXT_ATTR;
		break;
	case OSD_SET_SUB_CHANNEL_DATA:
		osd_enable[1] = 0;
		ak_print_notice_ex("disable sub channel osd\n");
		param->id = AK_ISP_USER_CID_SET_SUB_CHANNEL_OSD_CONTEXT_ATTR;
		break;
	default:
		ak_print_error_ex("unknow osd channel, id: %d\n", param->id);
		break;
	}
	bzero(param->data, sizeof(param->data));
	/*set to isp and then to driver*/
	return Ak_ISP_Set_User_Params((AK_ISP_USER_PARAM *)param);
}
