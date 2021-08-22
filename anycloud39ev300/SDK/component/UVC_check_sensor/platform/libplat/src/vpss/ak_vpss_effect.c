//#include "isp_basic.h"
//#include "isp_cfg_file.h"

#include "ak_common.h"
#include "ak_vpss.h"

/**  
 * ak_vpss_effect_get - get isp effect param. 
 * @vi_handle[IN]: vi module handle 
 * @type[IN]:   effect type name 
 * @effect[OUT]: effect value(s) 
 * return: 0 - success; otherwise -1;  
 */
int ak_vpss_effect_get(void *vi_handle, enum vpss_effect_type type, int *effect)
{
	if (!vi_handle) {
		ak_print_error_ex("invalid handle: %p\n", vi_handle);
		return AK_FAILED;
	}
	if (!effect) {
		ak_print_error_ex("invalid effect: %p\n", effect);
		return AK_FAILED;
	}
	
	if(type <= VPSS_EFFECT_SHARP){
		//return isp_get_effect(type, effect);
	}

	int ret = AK_SUCCESS;

	switch (type) {
	case VPSS_STYLE_ID:
		//*effect = isp_cfg_file_get_style_id();
		break;
	case VPSS_POWER_HZ:
		*effect = isp_get_hz();
		break;
	default:
		ak_print_error_ex("error type: %d\n", type);
		ret = AK_FAILED;
		break;
	}

	return ret;
}

/** 
 * ak_vpss_effect_set - set isp effect param. 
 * @vi_handle[IN]: vi module handle
 * @type[IN]:   effect type name
 * @effect[IN]: effect value
 * return: 0 - success; otherwise -1; 
 */
int ak_vpss_effect_set(void *vi_handle, enum vpss_effect_type type, 
				const int effect)
{
	if (!vi_handle) {
		ak_print_error_ex("invalid handle: %p\n", vi_handle);
		return AK_FAILED;
	}
	
	if(type <= VPSS_EFFECT_SHARP){
		//return isp_set_effect(type, effect);
	}

	int ret = AK_SUCCESS;
	switch (type){
	case VPSS_STYLE_ID:
		//ret = isp_cfg_file_set_style_id(effect);
		break;
	case VPSS_POWER_HZ:
		if ((effect != 50) && (effect != 60))
			break;

		ret = isp_set_hz(effect);
		break;
	default:
		ak_print_error_ex("error type: %d\n", type);
		ret = AK_FAILED;
		break;
	}

	return ret;
}
