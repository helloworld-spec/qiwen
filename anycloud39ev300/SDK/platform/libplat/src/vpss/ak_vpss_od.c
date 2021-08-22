#include <string.h>

#include "isp_basic.h"

#include "ak_common.h"
#include "ak_vpss.h"

struct rgb_hist_orig {
	unsigned int rgb_total;
	unsigned int rgb_hist[VPSS_OD_RGB_HIST_MAX];
};

/**
 * brief: get occlusion detection params
 * @vi_handle[IN]: opened video input handle
 * @od[OUT]: od params 
 * return: 0 success, -1 failed
 * notes:
 */
int ak_vpss_od_get(const void *vi_handle, struct vpss_od_info *od)
{
	if (NULL == vi_handle) {
		ak_print_error_ex("vi_handle is NULL\n");
		return AK_FAILED;
	}

	unsigned int size = 0;
	struct rgb_hist_orig rgb_orig = {0};

	/*get AF stat info*/
	int ret = isp_get_statinfo(ISP_AFSTAT, (void *)od->af_statics, &size);
	if (0 != ret) {
		ak_print_error_ex("get af nr stat info fail\n");
		return AK_FAILED;
	}

	/*get AE sub rgbinfo*/
	ret = isp_get_statinfo(ISP_AESTAT_SUB_RGB_STAT, (void *)&rgb_orig, &size);
	if (0 != ret) {
		ak_print_error_ex("get 3d nr stat info fail\n");
		return AK_FAILED;
	}
	od->rgb_total = rgb_orig.rgb_total;
	memcpy(od->rgb_hist, rgb_orig.rgb_hist, sizeof(rgb_orig.rgb_hist));

	return AK_SUCCESS;
}
