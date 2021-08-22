#include <stdlib.h>

#include "ak_common.h"
#include "isp_basic.h"
#include "ak_vpss.h"

/**  
 * brief: set main & sub channel mask area
 * @vi_handle[IN]: vi module handle
 * @p_mask[IN]: main & sub channel mask area paramters
 * return: 0 success, -1 failed
 * notes: 
 */
int ak_vpss_mask_set_area(const void *vi_handle, const struct vpss_mask_area_info *mask)
{
	if (NULL == vi_handle) {
		ak_print_error_ex("vi_handle is NULL\n");
		return -1;
	}

	int ret = isp_set_main_mask_area((void *)(mask->main_mask));
	if (ret) {
		ak_print_error_ex("set isp main mask failed\n");
	} else {
		ret = isp_set_sub_mask_area((void *)(mask->sub_mask));
		if (ret) {
			ak_print_error_ex("set isp sub mask failed\n");
		}
	}

	return ret;
}

/**  
 * brief: get main & sub channel mask area
 * @vi_handle[IN]: vi module handle
 * @p_mask[OUT]: main & sub channel mask area paramters
 * return: 0 success, -1 failed
 * notes: 
 */
int ak_vpss_mask_get_area(const void *vi_handle, struct vpss_mask_area_info *mask)
{
	if (NULL == vi_handle) {
		ak_print_error_ex("vi_handle is NULL\n");
		return -1;
	}

	int ret = isp_get_main_mask_area((void *)(mask->main_mask));
	if (ret) {
		ak_print_error_ex("set isp main mask failed\n");
	} else {
		ret = isp_get_sub_mask_area((void *)(mask->sub_mask));
		if (ret) {
			ak_print_error_ex("set isp sub mask failed\n");
		}
	}
	
	return ret;
}

/**  
 * brief: set main & sub channel mask color
 * @vi_handle[IN]: vi module handle
 * @color[IN]: main & sub channel mask color paramters
 * return: 0 success, -1 failed
 * notes: 
 */
int ak_vpss_mask_set_color(const void *vi_handle, const struct vpss_mask_color_info *color)
{
	if (NULL == vi_handle) {
		ak_print_error_ex("vi_handle is NULL\n");
		return -1;
	}

	int ret = isp_set_mask_color((void *)color);
	if (ret) {
		ak_print_error_ex("set mask color failed\n");
	}

	return ret;
}

/**  
 * brief: get main & sub channel mask color
 * @vi_handle[IN]: vi module handle
 * @color[OUT]: main & sub channel mask color paramters
 * return: 0 success, -1 failed
 * notes: 
 */
int ak_vpss_mask_get_color(const void *vi_handle, struct vpss_mask_color_info *color)
{
	if (NULL == vi_handle) {
		ak_print_error_ex("vi_handle is NULL\n");
		return -1;
	}

	int ret = isp_get_mask_color((void *)color);
	if (ret) {
		ak_print_error_ex("get mask color failed\n");
	}

	return ret;
}
