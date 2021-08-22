#include <stdlib.h>

#include "ak_common.h"
//#include "isp_basic.h"
#include "ak_vpss.h"
//#include "ak_isp_char.h"
#include "ispdrv_modules_interface.h"

/**
 * ak_vpss_hw_md_get_stat: get motion detection stat params
 * @vi_handle[IN]: vi module handle
 * @md[OUT]: md params 
 * return: 0 success, -1 failed
 * notes:
 */
int ak_vpss_md_get_stat(const void *vi_handle, struct vpss_md_info *md)
{
	if (NULL == vi_handle) {
		ak_print_error_ex("vi_handle is NULL\n");
		return -1;
	}

	unsigned int size = 0;
	int ret = isp_get_statinfo(ISP_3DSTAT, (void *)md, &size);
	if (0 != ret) {
		ak_print_error_ex("get 3d nr stat info fail\n");
	}

	return ret;
}
