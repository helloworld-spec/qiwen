#include <linux/ioctl.h>

#include "ak_common.h"
#include "ak_isp_char.h"
//#include "ak_isp_sdk.h"
#include "ak_vpss.h"

static struct vpss_osd_param local_param[VPSS_OSD_CHANNELS_MAX];
static int osd_enable[VPSS_OSD_CHANNELS_MAX];

/**  
 * vpss_osd_param_set - set osd param to isp and then to driver
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
	case OSD_SET_MAIN_DMA_MEM_REQUST:
		osd_enable[0] = 1;
		param->id = AK_ISP_USER_CID_SET_MAIN_CHANNEL_OSD_MEM_ATTR;
		break;
	case OSD_SET_SUB_DMA_MEM_REQUST:
		osd_enable[1] = 1;
		param->id = AK_ISP_USER_CID_SET_SUB_CHANNEL_OSD_MEM_ATTR;
		break;
	default:
		break;
	
	}
	
	if (do_change)
		return akisp_ioctl(AK_ISP_SET_USER_PARAMS,(AK_ISP_USER_PARAM *)param);
	else
		return AK_SUCCESS;

}

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
	memset(param->data,0x0, sizeof(param->data));
	return akisp_ioctl(AK_ISP_SET_USER_PARAMS,(AK_ISP_USER_PARAM *)param);
}
