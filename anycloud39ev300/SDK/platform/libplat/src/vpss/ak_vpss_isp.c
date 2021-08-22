#include <string.h>
#include <stdlib.h>

#include "ak_common.h"
#include "ak_isp_sdk.h"
#include "ak_vpss.h"

#include "isp_basic.h"
#include "isp_main.h"

#define CHECK_TIME				3
#define AWB_NIGHT_ARRAY_NUM		5
#define AWB_DAY_ARRAY_NUM		10
/// ISP TOOL AWB 的 CNT 数组。
#define AWB_CNT_ARRAY_NUM       (10)

#define VPSS_ISP			"<plat_vpss_isp>"

struct soft_ir_save_info {
	int lum_factor;	// 
	int lum_avg;	// avarge lumi
	int lum_calc;	// calcurate lumi
	int awb_cnt[AWB_CNT_ARRAY_NUM];	// awb cnt array
	int night_lock;	// 1:lock in the night, 0 no lock
	//int lock_interval;	// lock time
	int init_flag;
	struct ak_timeval lock_time;	// lock in night time,ms
};

static struct soft_ir_save_info soft_ir_info = {0};
static struct ak_auto_day_night_threshold auto_day_night_threshold = {0};

static void set_auto_day_night_param(struct ak_auto_day_night_threshold *in_param,
							struct ak_auto_day_night_threshold *out_config)
{
	if (in_param) {
		memcpy(out_config, in_param, sizeof(struct ak_auto_day_night_threshold));
	} else {
		out_config->day_to_night_lum = DAY_TO_NIGHT_LUM_FACTOR;
		out_config->night_to_day_lum = NIGHT_TO_DAY_LUM_FACTOR;
		out_config->lock_time = LOCK_TIME;

		out_config->night_cnt[0] = TOTAL_CNT_NIGHT_0;	
		out_config->night_cnt[1] = TOTAL_CNT_NIGHT_1;
		out_config->night_cnt[2] = TOTAL_CNT_NIGHT_2;
		out_config->night_cnt[3] = TOTAL_CNT_NIGHT_3;
		out_config->night_cnt[4] = TOTAL_CNT_NIGHT_4;

		out_config->day_cnt[0] = TOTAL_CNT_DAY_0;
		out_config->day_cnt[1] = TOTAL_CNT_DAY_1;
		out_config->day_cnt[2] = TOTAL_CNT_DAY_2;
		out_config->day_cnt[3] = TOTAL_CNT_DAY_3;
		out_config->day_cnt[4] = TOTAL_CNT_DAY_4;
		out_config->day_cnt[5] = TOTAL_CNT_DAY_5;
		out_config->day_cnt[6] = TOTAL_CNT_DAY_6;
		out_config->day_cnt[7] = TOTAL_CNT_DAY_7;
		out_config->day_cnt[8] = TOTAL_CNT_DAY_8;
		out_config->day_cnt[9] = TOTAL_CNT_DAY_9;		
	}
		
	out_config->quick_switch_mode = 0;
		
	ak_print_normal_ex("(switch day_night) D_to_N=%d ,N_to_D=%d,lock_time=%d\n",
					out_config->day_to_night_lum,
					out_config->night_to_day_lum,
					out_config->lock_time);
	int i = 0;
	for (i = 0; i < NIGHT_ARRAY_NUM; i++)
		ak_print_normal_ex("%d--%d\n", i, out_config->night_cnt[i]);
	
	for (i = 0; i < DAY_ARRAY_NUM; i++)
		ak_print_normal_ex("%d--%d\n", i, out_config->day_cnt[i]);

}


static int day_mode_cmp_awb(void)
{
	unsigned int size = 0;
	struct vpss_isp_awb_stat_info awb_info_pre;	
	struct vpss_isp_awb_stat_info awb_info_cur;	
	unsigned long  total_cnt[DAY_ARRAY_NUM];
	
	if (isp_get_statinfo(ISP_AWBSTAT, (void *)(&awb_info_pre), &size)) {
		ak_print_error_ex(" get awb info failed \n");
		return AK_FAILED;
	}

	if (isp_get_statinfo(ISP_AWBSTAT, (void *)(&awb_info_cur), &size)) {
		ak_print_error_ex(" get awb info failed \n");
		return AK_FAILED;
	}

	int awb_day_night = 0;
	int i = 0;


	ak_print_normal_ex("\n");
	for (i = 0; i < DAY_ARRAY_NUM; i++) {
		total_cnt[i] = (awb_info_pre.total_cnt[i] + awb_info_cur.total_cnt[i]) / 2;
		ak_print_normal("--[%d]%ld--", i, total_cnt[i]);
	}
	ak_print_normal_ex("\n");
#if 0

	for (i = 0; i < AWB_DAY_ARRAY_NUM; i++) {
		ak_print_normal(VPSS_ISP "p--[%d]%d--", i, auto_day_night_threshold.day_cnt[i]);
	}
	ak_print_normal(VPSS_ISP "\n");
#endif
	
	for (i = 0; i < DAY_ARRAY_NUM; i++) {
		if (total_cnt[i] > auto_day_night_threshold.day_cnt[i]) {
			awb_day_night = 1;
			break;
		}
	}
	ak_print_normal("awb_day_night=%d\n", awb_day_night);
	return awb_day_night;
}

static int night_mode_cmp_awb(void)
{
	unsigned int size = 0;
	struct vpss_isp_awb_stat_info awb_info_pre;
	struct vpss_isp_awb_stat_info awb_info_cur;
	unsigned long  total_cnt[NIGHT_ARRAY_NUM];
	
	if (isp_get_statinfo(ISP_AWBSTAT, (void *)(&awb_info_pre), &size)) {
		ak_print_error_ex("get awb info failed \n");
		return AK_FAILED;
	}

	if (isp_get_statinfo(ISP_AWBSTAT, (void *)(&awb_info_cur), &size)) {
		ak_print_error_ex("get awb info failed \n");
		return AK_FAILED;
	}

	int awb_day_night = 0;
	int i = 0;

	ak_print_normal("\n");
	for (i = 0; i < NIGHT_ARRAY_NUM; i++) {
		total_cnt[i] = (awb_info_pre.total_cnt[i] + awb_info_cur.total_cnt[i]) / 2;
		ak_print_normal("--[%d]%ld--", i, total_cnt[i]);
	}
	ak_print_normal("\n");
#if 0

	for (i = 0; i < AWB_NIGHT_ARRAY_NUM; i++) {
		ak_print_normal("p--[%d]%d--", i, auto_day_night_threshold.night_cnt[i]);
	}
	ak_print_normal("\n");
#endif

	for (i = 0; i < NIGHT_ARRAY_NUM; i++) {
		if (total_cnt[i] > auto_day_night_threshold.night_cnt[i]) {
			awb_day_night = 1;// is day
			break;
		}
	}
	ak_print_normal("awb_day_night=%d\n", awb_day_night);
	return awb_day_night;
}

static int wait_move_stable(void)
{
	int stable_flag = 0;
	int cur_lum_factor = 0;
	int day_count = 0;
	int night_count =0;
	int no_change = 0;
	int day_to_night = auto_day_night_threshold.day_to_night_lum;
	int night_to_day = auto_day_night_threshold.night_to_day_lum;
	
	ak_sleep_ms(500); /* wait the light stable */

	while (!stable_flag) {
		cur_lum_factor = isp_get_cur_lum_factor();
		if (cur_lum_factor > day_to_night)
			night_count++;
		else if (cur_lum_factor < night_to_day)
			day_count++;
		else
			no_change++;

		if (day_count >= CHECK_TIME || night_count >= CHECK_TIME || 
						no_change >= CHECK_TIME) {
			if (day_count + night_count + no_change == CHECK_TIME)
				break;
			else {
				day_count = 0;
				night_count = 0;
				no_change = 0;
				ak_print_warning_ex(VPSS_ISP "not stable\n");
			}	
		}
		
		ak_sleep_ms(500);
	}
	
	return AK_SUCCESS;
}

int calc_cur_lumi(void)
{
	AK_ISP_AE_RUN_INFO run_info = {0};
	int avg_lumi = 40;
	
	if (AK_ISP_get_ae_run_info(&run_info)) {
		ak_print_error_ex("get lum avg error\n");
	} else {
		avg_lumi = run_info.current_calc_avg_lumi;
	}

	avg_lumi = (avg_lumi == 0 ? 40 : avg_lumi);

	int cur_lum_factor = isp_get_cur_lum_factor();
	int lum_factor = cur_lum_factor * 40 / avg_lumi;
	
	ak_print_info_ex("cur_lum =%d, lum_avg = %d, res_lum=%d\n", cur_lum_factor,
				avg_lumi, lum_factor);

	return lum_factor;
}

int save_soft_ir_info(void)
{		
	AK_ISP_AE_RUN_INFO run_info = {0};
	if (AK_ISP_get_ae_run_info(&run_info)) {
		ak_print_error_ex("get lum avg error\n");
		return AK_FAILED;
	} else {
		soft_ir_info.lum_avg = run_info.current_calc_avg_lumi;
	}

	soft_ir_info.lum_factor = isp_get_cur_lum_factor();
	soft_ir_info.lum_calc = calc_cur_lumi();

	/* save awb cnt */
	struct vpss_isp_awb_stat_info awb_info;	
	unsigned int size = 0;
	
	if (isp_get_statinfo(ISP_AWBSTAT, (void *)(&awb_info), &size)) {
		ak_print_error_ex(" get awb info failed \n");
		return AK_FAILED;
	}

	int i = 0;
	for (i = 0; i < DAY_ARRAY_NUM; i++) {
		soft_ir_info.awb_cnt[i] = awb_info.total_cnt[i];
	}

	//soft_ir_info.night_lock = 0;

	/* update lock time */
    ak_get_ostime(&(soft_ir_info.lock_time));

	return AK_SUCCESS;
	
}

int night_status_change(void)
{
	int cur_lum_calc = calc_cur_lumi();
	struct vpss_isp_awb_stat_info awb_info;	
	unsigned int size = 0;
	int cur_awb_cnt[DAY_ARRAY_NUM];
	
	if (isp_get_statinfo(ISP_AWBSTAT, (void *)(&awb_info), &size)) {
		ak_print_error_ex(" get awb info failed \n");
		return AK_FAILED;
	}

	int i = 0;
	for (i = 0; i < DAY_ARRAY_NUM; i++) {
		cur_awb_cnt[i] = awb_info.total_cnt[i];
	}

	int ret = AK_FALSE;

	if (soft_ir_info.lum_calc == 0)
		return ret;

	if (((abs(cur_lum_calc- soft_ir_info.lum_calc)) * 100) / soft_ir_info.lum_calc >= 30)
		return AK_TRUE;
	else {
		for (i = 0; i < DAY_ARRAY_NUM; i++) {
			if (((abs(cur_awb_cnt[i] - soft_ir_info.awb_cnt[i])) * 100) / (soft_ir_info.awb_cnt[i]+1) >= 30 &&
					(abs(cur_awb_cnt[i] - soft_ir_info.awb_cnt[i])) > 10000)
				return AK_TRUE;
		}
	}

	return ret;
}

/**
 * ak_vpss_isp_init_sdk: init isp sdk
 * @void
 * return: 0 success, -1 failed
 */
int ak_vpss_isp_init_sdk(void)
{
	return AK_ISP_sdk_init();
}

/**
 * ak_vpss_isp_get_ae_attr: get AE attr
 * @vi_handle[IN]: vi module handle
 * @ae_attr[OUT]: AE attr info
 * return: 0 success, -1 failed
 */
int ak_vpss_isp_get_ae_attr(const void *vi_handle, 
		struct vpss_isp_ae_attr *ae_attr)
{
	if (!vi_handle) {
		ak_print_error_ex(VPSS_ISP "vi_handle is NULL\n");
		return AK_FAILED;
	}
	if (!ae_attr) {
		ak_print_error_ex(VPSS_ISP "ae_attr is NULL\n");
		return AK_FAILED;
	}

	AK_ISP_AE_ATTR tmp = {0};
	int ret = AK_ISP_get_ae_attr(&tmp);
	if (!ret) {
		memcpy(ae_attr, &tmp, sizeof(struct vpss_isp_ae_attr));
	}
	
	return ret;
}

/**
 * ak_vpss_isp_set_ae_attr: set AE attr
 * @vi_handle[IN]: vi module handle
 * @ae_attr[IN]: AE attr info
 * return: 0 success, -1 failed
 */
int ak_vpss_isp_set_ae_attr(const void *vi_handle, 
							const struct vpss_isp_ae_attr *ae_attr)
{
	if (!vi_handle) {
		ak_print_error_ex(VPSS_ISP "vi_handle is NULL\n");
		return AK_FAILED;
	}
	if (!ae_attr) {
		ak_print_error_ex(VPSS_ISP "ae_attr is NULL\n");
		return AK_FAILED;
	}

	AK_ISP_AE_ATTR tmp = {0};
	memcpy(&tmp, ae_attr, sizeof(struct vpss_isp_ae_attr));

	return AK_ISP_set_ae_attr(&tmp);
}



/**
 * ak_vpss_isp_get_mae_attr: get MAE attr
 * @vi_handle[IN]: vi module handle
 * @mae_attr[OUT]: MAE attr info
 * return: 0 success, -1 failed
 */
int ak_vpss_isp_get_mae_attr(const void *vi_handle, 
		struct vpss_isp_mae_attr *mae_attr)
{
	if (!vi_handle) {
		ak_print_error_ex(VPSS_ISP "vi_handle is NULL\n");
		return AK_FAILED;
	}
	if (!mae_attr) {
		ak_print_error_ex(VPSS_ISP "mae_attr is NULL\n");
		return AK_FAILED;
	}

	AK_ISP_MAE_ATTR tmp = {0};
	int ret = AK_ISP_get_mae_attr(&tmp);
	if (!ret) {
		memcpy(mae_attr, &tmp, sizeof(struct vpss_isp_mae_attr));
	}
	
	return ret;
}

/**
 * ak_vpss_isp_set_mae_attr: set MAE attr
 * @vi_handle[IN]: vi module handle
 * @mae_attr[IN]: MAE attr info
 * return: 0 success, -1 failed
 */
int ak_vpss_isp_set_mae_attr(const void *vi_handle, 
							const struct vpss_isp_mae_attr *mae_attr)
{
	if (!vi_handle) {
		ak_print_error_ex(VPSS_ISP "vi_handle is NULL\n");
		return AK_FAILED;
	}
	if (!mae_attr) {
		ak_print_error_ex(VPSS_ISP "mae_attr is NULL\n");
		return AK_FAILED;
	}

	AK_ISP_MAE_ATTR tmp = {0};
	memcpy(&tmp, mae_attr, sizeof(struct vpss_isp_mae_attr));

	return AK_ISP_set_mae_attr(&tmp);
}

/**
 * ak_vpss_get_weight_attr: get weight attr
 * @vi_handle[IN]: vi module handle
 * @weight_attr[OUT]: weight_attr info
 * return: 0 success, -1 failed
 */
int ak_vpss_get_weight_attr(const void *vi_handle, 
		struct vpss_isp_weight_attr *weight_attr)
{
	if (!vi_handle) {
		ak_print_error_ex(VPSS_ISP "vi_handle is NULL\n");
		return AK_FAILED;
	}
	if (!weight_attr) {
		ak_print_error_ex(VPSS_ISP "weight_attr is NULL\n");
		return AK_FAILED;
	}

	AK_ISP_WEIGHT_ATTR tmp = {{{0}}};
	int ret = AK_ISP_get_weight_attr(&tmp);
	if (!ret) {
		memcpy(weight_attr, &tmp, sizeof(struct vpss_isp_weight_attr));
	}
	
	return ret;
}

/**
 * ak_vpss_set_weight_attr: set weight attr
 * @vi_handle[IN]: vi module handle
 * @weight_attr[IN]: weight_attr info
 * return: 0 success, -1 failed
 */
int ak_vpss_set_weight_attr(const void *vi_handle, 
		struct vpss_isp_weight_attr *weight_attr)
{
	if (!vi_handle) {
		ak_print_error_ex(VPSS_ISP "vi_handle is NULL\n");
		return AK_FAILED;
	}
	if (!weight_attr) {
		ak_print_error_ex(VPSS_ISP "weight_attr is NULL\n");
		return AK_FAILED;
	}

	AK_ISP_WEIGHT_ATTR tmp = {{{0}}};
	memcpy(&tmp, weight_attr, sizeof(struct vpss_isp_weight_attr));

	return AK_ISP_set_weight_attr(&tmp);
}


/**
 * ak_vpss_isp_set_ae_convergence_rate: set AE convergence rate
 * @vi_handle[IN]: vi module handle
 * @value[IN]: param of convergence rate
 * return: 0 success, -1 failed
 */
int ak_vpss_isp_set_ae_convergence_rate(const void *vi_handle, unsigned long value)
{
    if (!vi_handle) {
		ak_print_error_ex(VPSS_ISP "vi_handle is NULL\n");
		return AK_FAILED;
	}

	AK_ISP_AE_ATTR tmp = {0};
	int ret = AK_ISP_get_ae_attr(&tmp);

    if (AK_SUCCESS != ret) 
    {
		ak_print_error_ex("get ae fail\n");
        return AK_FAILED;
	}

	tmp.envi_gain_range[9][1] = value;
    ret = AK_ISP_set_ae_attr(&tmp);

    if (AK_SUCCESS != ret) 
    {
		ak_print_error_ex("set ae fail\n");
        return AK_FAILED;
	}

    return AK_SUCCESS;
}

/**
 * ak_vpss_isp_get_ae_convergence_rate: get AE convergence rate
 * @vi_handle[IN]: vi module handle
 * @value[OUT]: param of convergence rate
 * return: 0 success, -1 failed
 */
int ak_vpss_isp_get_ae_convergence_rate(const void *vi_handle, unsigned long *value)
{
    if (!vi_handle) {
		ak_print_error_ex(VPSS_ISP "vi_handle is NULL\n");
		return AK_FAILED;
	}

    if (!value) {
		ak_print_error_ex(VPSS_ISP "value is NULL\n");
		return AK_FAILED;
	}

	AK_ISP_AE_ATTR tmp = {0};
	int ret = AK_ISP_get_ae_attr(&tmp);

    if (AK_SUCCESS != ret) 
    {
		ak_print_error_ex("get ae fail\n");
        return AK_FAILED;
	}

	*value = tmp.envi_gain_range[9][1];

    return AK_SUCCESS;
}

/**
 * ak_vpss_isp_check_ae_stable: check ae is stable or not
 * @vi_handle[IN]: vi module handle
 * @stable[OUT]: 1 stable, 0 not stable
 * return: 0 success, -1 failed
 */
int ak_vpss_isp_check_ae_stable(const void *vi_handle, int *stable)
{
	if (!vi_handle) {
		ak_print_error_ex(VPSS_ISP "vi_handle is NULL\n");
		return AK_FAILED;
	}
	if (!stable) {
		ak_print_error_ex(VPSS_ISP "stable is NULL\n");
		return AK_FAILED;
	}

	AK_ISP_AE_ATTR ae = {0};
	int ret = AK_ISP_get_ae_attr(&ae);
	if (AK_SUCCESS != ret) {
		ak_print_error_ex("get ae attr fail\n");
        return AK_FAILED;
	}

    AK_ISP_AE_RUN_INFO run = {0};
	ret = AK_ISP_get_ae_run_info(&run);
	if (AK_SUCCESS != ret) {
		ak_print_error_ex("get ae run info fail\n");
        return AK_FAILED;
	}

    if ((run.current_calc_avg_compensation_lumi <= ae.target_lumiance + ae.exp_stable_range)
        && (run.current_calc_avg_compensation_lumi >= ae.target_lumiance - ae.exp_stable_range)){
        *stable = 1;
    } else {
        *stable = 0;
    }
	
	return ret;
}
/**
 * ak_vpss_isp_get_wb_type: get wb type
 * @vi_handle[IN]: vi module handle
 * @wb_type[OUT]: wb type
 * return: 0 success, -1 failed
 */
int ak_vpss_isp_get_wb_type(const void *vi_handle, 
							struct vpss_isp_wb_type_attr *wb_type)
{
	if (!vi_handle) {
		ak_print_error_ex(VPSS_ISP "vi_handle is NULL\n");
		return AK_FAILED;
	}
	if (!wb_type) {
		ak_print_error_ex(VPSS_ISP "wb_type is NULL\n");
		return AK_FAILED;
	}
	
	AK_ISP_WB_TYPE_ATTR	tmp = {0};
	int ret = AK_ISP_get_wb_type(&tmp);
	if (!ret) {
		memcpy(wb_type, &tmp, sizeof(struct vpss_isp_wb_type_attr));
	}

	return ret;
}

/**
 * ak_vpss_isp_set_wb_type: set wb type
 * @vi_handle[IN]: vi module handle
 * @wb_type[IN]: wb type
 * return: 0 success, -1 failed
 */
int ak_vpss_isp_set_wb_type(const void *vi_handle, 
							const struct vpss_isp_wb_type_attr *wb_type)
{
	if (!vi_handle) {
		ak_print_error_ex(VPSS_ISP "vi_handle is NULL\n");
		return AK_FAILED;
	}
	if (!wb_type) {
		ak_print_error_ex(VPSS_ISP "wb_type is NULL\n");
		return AK_FAILED;
	}

	AK_ISP_WB_TYPE_ATTR	tmp = {0};
	memcpy(&tmp, wb_type, sizeof(struct vpss_isp_wb_type_attr));

	return AK_ISP_set_wb_type(&tmp);
}

/**
 * ak_vpss_isp_get_3d_nr_attr: get 3D NR attr
 * @vi_handle[IN]: vi module handle
 * @nr_3d_attr[OUT]: 3D NR attr
 * return: 0 success, -1 failed
 */
int ak_vpss_isp_get_3d_nr_attr(const void *vi_handle, 
							struct vpss_isp_3d_nr_attr *nr_3d_attr)
{
	if (!vi_handle) {
		ak_print_error_ex(VPSS_ISP "vi_handle is NULL\n");
		return AK_FAILED;
	}
	if (!nr_3d_attr) {
		ak_print_error_ex(VPSS_ISP "3d_nr_attr is NULL\n");
		return AK_FAILED;
	}
	
	AK_ISP_3D_NR_ATTR tmp = {0};
	int ret = AK_ISP_get_3d_nr_attr(&tmp);
	if (!ret) {
		memcpy(nr_3d_attr, &tmp, sizeof(struct vpss_isp_3d_nr_attr));
	}

	return ret;
}

/**
 * ak_vpss_isp_set_3d_nr_attr: set 3D NR attr
 * @vi_handle[IN]: vi module handle
 * @nr_3d_attr[IN]: 3D NR attr
 * return: 0 success, -1 failed
 */
int ak_vpss_isp_set_3d_nr_attr(const void *vi_handle, 
							const struct vpss_isp_3d_nr_attr *nr_3d_attr)
{
	if (!vi_handle) {
		ak_print_error_ex(VPSS_ISP "vi_handle is NULL\n");
		return AK_FAILED;
	}
	if (!nr_3d_attr) {
		ak_print_error_ex(VPSS_ISP "3d_nr_attr is NULL\n");
		return AK_FAILED;
	}

	AK_ISP_3D_NR_ATTR tmp = {0};
	memcpy(&tmp, nr_3d_attr, sizeof(struct vpss_isp_3d_nr_attr));

	return AK_ISP_set_3d_nr_attr(&tmp);
}

/**
 * ak_vpss_isp_get_mwb_attr: get mwb attr
 * @vi_handle[IN]: vi module handle
 * @mwb_attr[OUT]: mwb attr
 * return: 0 success, -1 failed
 */
int ak_vpss_isp_get_mwb_attr(const void *vi_handle, 
							struct vpss_isp_mwb_attr *mwb_attr)
{
	if (!vi_handle) {
		ak_print_error_ex(VPSS_ISP "vi_handle is NULL\n");
		return AK_FAILED;
	}
	if (!mwb_attr) {
		ak_print_error_ex(VPSS_ISP "mwb_attr is NULL\n");
		return AK_FAILED;
	}
	
	AK_ISP_MWB_ATTR tmp = {0};
	int ret = AK_ISP_get_mwb_attr(&tmp);
	if (!ret) {
		memcpy(mwb_attr, &tmp, sizeof(struct vpss_isp_mwb_attr));
	}

	return ret;
}

/**
 * ak_vpss_isp_set_mwb_attr: set mwb attr
 * @vi_handle[IN]: vi module handle
 * @mwb_attr[IN]: mwb attr
 * return: 0 success, -1 failed
 */
int ak_vpss_isp_set_mwb_attr(const void *vi_handle, 
							const struct vpss_isp_mwb_attr *mwb_attr)
{
	if (!vi_handle) {
		ak_print_error_ex(VPSS_ISP "vi_handle is NULL\n");
		return AK_FAILED;
	}
	if (!mwb_attr) {
		ak_print_error_ex(VPSS_ISP "mwb_attr is NULL\n");
		return AK_FAILED;
	}
	
	AK_ISP_MWB_ATTR tmp = {0};
	memcpy(&tmp, mwb_attr, sizeof(struct vpss_isp_mwb_attr));
	
	return AK_ISP_set_mwb_attr(&tmp);
}

/**
 * ak_vpss_isp_get_awb_attr: get awb attr
 * @vi_handle[IN]: vi module handle
 * @awb_attr[OUT]: awb attr
 * return: 0 success, -1 failed
 */
int ak_vpss_isp_get_awb_attr(const void *vi_handle, 
							struct vpss_isp_awb_attr *awb_attr)
{
	if (!vi_handle) {
		ak_print_error_ex(VPSS_ISP "vi_handle is NULL\n");
		return AK_FAILED;
	}
	if (!awb_attr) {
		ak_print_error_ex(VPSS_ISP "awb_attr is NULL\n");
		return AK_FAILED;
	}

	AK_ISP_AWB_ATTR tmp = {{0}, 0};
	int ret = AK_ISP_get_awb_attr(&tmp);
	if (!ret) {
		memcpy(awb_attr, &tmp, sizeof(struct vpss_isp_awb_attr));
	}

	return ret;
}

/**
 * ak_vpss_isp_set_awb_attr: set awb attr
 * @vi_handle[IN]: vi module handle
 * @awb_attr[IN]: awb attr
 * return: 0 success, -1 failed
 */
int ak_vpss_isp_set_awb_attr(const void *vi_handle, 
							const struct vpss_isp_awb_attr *awb_attr)
{
	if (!vi_handle) {
		ak_print_error_ex(VPSS_ISP "vi_handle is NULL\n");
		return AK_FAILED;
	}
	if (!awb_attr) {
		ak_print_error_ex(VPSS_ISP "awb_attr is NULL\n");
		return AK_FAILED;
	}

	AK_ISP_AWB_ATTR tmp = {{0}, 0};
	memcpy(&tmp, awb_attr, sizeof(struct vpss_isp_awb_attr));
	
	return AK_ISP_set_awb_attr(&tmp);
}

/**
 * ak_vpss_isp_get_exp_type: get exp type
 * @vi_handle[IN]: vi module handle
 * @exp_type[IN]: exp type
 * return: 0 success, -1 failed
 */
int ak_vpss_isp_get_exp_type(const void *vi_handle, 
							struct vpss_isp_exp_type *exp_type)
{
	if (!vi_handle) {
		ak_print_error_ex(VPSS_ISP "vi_handle is NULL\n");
		return AK_FAILED;
	}
	if (!exp_type) {
		ak_print_error_ex(VPSS_ISP "exp_type is NULL\n");
		return AK_FAILED;
	}

	AK_ISP_EXP_TYPE tmp = {0};
	int ret = AK_ISP_get_exp_type(&tmp);
	if (!ret) {
		memcpy(exp_type, &tmp, sizeof(struct vpss_isp_exp_type));
	}

	return ret;
}

/**
 * ak_vpss_isp_set_exp_type: set exp type
 * @vi_handle[IN]: vi module handle
 * @exp_type[IN]: exp type
 * return: 0 success, -1 failed
 */
int ak_vpss_isp_set_exp_type(const void *vi_handle, 
							const struct vpss_isp_exp_type *exp_type)
{
	if (!vi_handle) {
		ak_print_error_ex(VPSS_ISP "vi_handle is NULL\n");
		return AK_FAILED;
	}
	if (!exp_type) {
		ak_print_error_ex(VPSS_ISP "exp_type is NULL\n");
		return AK_FAILED;
	}

	AK_ISP_EXP_TYPE tmp = {0};
	memcpy(&tmp, exp_type, sizeof(struct vpss_isp_exp_type));
	
	return AK_ISP_set_exp_type(&tmp);
}

/**
 * ak_vpss_isp_get_ae_run_info: get ae run info
 * @vi_handle[IN]: vi module handle
 * @ae_run_info[OUT]: ae run info
 * return: 0 success, -1 failed
 */
int ak_vpss_isp_get_ae_run_info(const void *vi_handle, 
							struct vpss_isp_ae_run_info *ae_run_info)
{
	if (!vi_handle) {
		ak_print_error_ex(VPSS_ISP "vi_handle is NULL\n");
		return AK_FAILED;
	}
	if (!ae_run_info) {
		ak_print_error_ex(VPSS_ISP "ae_run_info is NULL\n");
		return AK_FAILED;
	}

	AK_ISP_AE_RUN_INFO tmp = {0};
	int ret = AK_ISP_get_ae_run_info(&tmp);
	if (!ret) {
		memcpy(ae_run_info, &tmp, sizeof(struct vpss_isp_ae_run_info));
	}

	return ret;
}

/**
 * ak_vpss_isp_get_awb_stat_info: get awb stat info
 * @vi_handle[IN]: vi module handle
 * @awb_stat_info[OUT]: awb stat info
 * return: 0 success, -1 failed
 */
int ak_vpss_isp_get_awb_stat_info(const void *vi_handle, 
							struct vpss_isp_awb_stat_info *awb_stat_info)
{
	if (!vi_handle) {
		ak_print_error_ex(VPSS_ISP "vi_handle is NULL\n");
		return AK_FAILED;
	}
	if (!awb_stat_info) {
		ak_print_error_ex(VPSS_ISP "awb_stat_info is NULL\n");
		return AK_FAILED;
	}

	AK_ISP_AWB_STAT_INFO tmp = {{0}};
	int ret = Ak_ISP_get_awb_stat_info(&tmp);
	if (!ret) {
		memcpy(awb_stat_info, &tmp, sizeof(struct vpss_isp_awb_stat_info));
	}

	return ret;
}

/**
 * ak_vpss_isp_get_rgb_average: get rgb average value
 * @vi_handle[IN]: vi module handle
 * @r_avr[OUT]: r average value
 * @g_avr[OUT]: g average value
 * @b_avr[OUT]: b average value
 * return: 0 success, -1 failed
 */
int ak_vpss_isp_get_rgb_average(const void *vi_handle, 
        unsigned int *r_avr, unsigned int *g_avr, unsigned int *b_avr)
{
	if (!vi_handle) {
		ak_print_error_ex(VPSS_ISP "vi_handle is NULL\n");
		return AK_FAILED;
	}
	if (!r_avr || !g_avr || !b_avr) {
		ak_print_error_ex(VPSS_ISP "param is NULL\n");
		return AK_FAILED;
	}

	AK_ISP_AWB_STAT_INFO tmp = {{0}};
	int ret = Ak_ISP_get_awb_stat_info(&tmp);
	if (!ret) {
        if (tmp.total_cnt[9] > 0)
        {
		    *r_avr = tmp.total_R[9] / tmp.total_cnt[9];
            *g_avr = tmp.total_G[9] / tmp.total_cnt[9];
            *b_avr = tmp.total_B[9] / tmp.total_cnt[9];
        }
        else
        {
            *r_avr = 0;
            *g_avr = 0;
            *b_avr = 0;
        }
	}

	return ret;
}



/**
 * ak_vpss_isp_get_sensor_reg: get sensor register info
 * @vi_handle[IN]: vi module handle
 * @sensor_reg_info[IN/OUT]: sensor register info
 * return: 0 success, -1 failed
 */
int ak_vpss_isp_get_sensor_reg(const void *vi_handle, 
							struct vpss_isp_sensor_reg_info *sensor_reg_info)
{
	if (!vi_handle) {
		ak_print_error_ex(VPSS_ISP "vi_handle is NULL\n");
		return AK_FAILED;
	}
	if (!sensor_reg_info) {
		ak_print_error_ex(VPSS_ISP "ae_run_info is NULL\n");
		return AK_FAILED;
	}

	AK_ISP_SENSOR_REG_INFO tmp = {0};
	int ret = Ak_ISP_Sensor_Get_Reg(&tmp);
	if (!ret) {
		memcpy(sensor_reg_info, &tmp, sizeof(struct vpss_isp_sensor_reg_info));
	}

	return ret;
}

/**
 * ak_vpss_get_force_anti_flicker_flag: get force anti flicker flag
 * @vi_handle[IN]: vi module handle
 * return: force anti flicker flag
 */
int ak_vpss_get_force_anti_flicker_flag(const void *vi_handle)
{	
	if (!vi_handle) {
		ak_print_error_ex(VPSS_ISP "vi_handle is NULL\n");
		return 0;
	}

	return isp_get_force_anti_flicker_flag();
}

/**
 * ak_vpss_set_force_anti_flicker_flag: set force anti flicker flag
 * @vi_handle[IN]: vi module handle
 * @force_flag[IN]: force anti flicker flag
 * return: 0 success, -1 failed
 */
 int ak_vpss_set_force_anti_flicker_flag(const void *vi_handle, int force_flag)
{
	if (!vi_handle) {
		ak_print_error_ex(VPSS_ISP "vi_handle is NULL\n");
		return AK_FAILED;
	}
	
	return isp_set_force_anti_flicker_flag(force_flag);
}

/**
 * ak_vpss_isp_exit_sdk: exit isp sdk
 * @void
 * return: 0 success, -1 failed
 */
int ak_vpss_isp_exit_sdk(void)
{
	return AK_ISP_sdk_exit();
}

/**
 * ak_vpss_isp_set_auto_day_night_param: set auto day or night switch threshold
 * @param[IN]: input param threshold
 * return: NULL
 */
int ak_vpss_isp_set_auto_day_night_param(struct ak_auto_day_night_threshold *param)
{
	if (soft_ir_info.init_flag) {
		ak_print_warning_ex("auto_day_night param have set already\n");
		return AK_FAILED;
	}
	set_auto_day_night_param(param, &auto_day_night_threshold);
	soft_ir_info.init_flag = 1;

	return AK_SUCCESS;
}

/**
 * ak_vpss_isp_get_auto_day_night_level: get day or night 
 * @pre_ir_level[IN]: pre status, 0 night, 1 day
 * return: 0 night, 1 day, -1 failed
 */
int ak_vpss_isp_get_auto_day_night_level(int pre_ir_level)
{
	if (0 == soft_ir_info.init_flag) {
		ak_print_error_ex("not init,set ak_vpss_isp_set_auto_day_night_param first\n");
		return AK_FAILED;
	}

	if (0 > pre_ir_level || 1 < pre_ir_level) {
		ak_print_error_ex("wrong input parameter\n");	
		return AK_FAILED;
	}
	
	int day_night_level = pre_ir_level;	
	int check_time = CHECK_TIME;

	wait_move_stable();

	int day_count = 0;
	int night_count = 0;
	int i = 0;
	int cur_lum_factor = 0;

	ak_print_info_ex("lock time=%d, lock=%d\n", auto_day_night_threshold.lock_time,
					soft_ir_info.night_lock);

	//exception!!! STATE_DAY && night_lock
	//night_lock only valid at night state
	if(day_night_level == STATE_DAY && soft_ir_info.night_lock == 1) {
		ak_print_info_ex("1----lock= 0\n");
		soft_ir_info.night_lock = 0;
	}
	if (soft_ir_info.night_lock == 1) {
		struct ak_timeval cur_time;
		ak_get_ostime(&cur_time);
		long diff_time = ak_diff_ms_time(&cur_time, &(soft_ir_info.lock_time));
		
		if (diff_time > auto_day_night_threshold.lock_time 
			|| AK_TRUE == night_status_change()) {
			ak_print_info_ex("2----lock= 0, diff_time=%ld\n", diff_time);
			soft_ir_info.night_lock = 0;
		}
	} else {
		for (i = 0; i < check_time; i++) {
			cur_lum_factor = calc_cur_lumi();
			ak_print_info_ex(" pre lum=%s--%d\n", pre_ir_level==1?"Day":"night", 
							cur_lum_factor);
			switch (pre_ir_level) {
				case STATE_NIGHT:
					if (cur_lum_factor < auto_day_night_threshold.night_to_day_lum) {
						/* check WB */
						if (STATE_DAY == night_mode_cmp_awb()) {					
							/* day mode */
							day_night_level = STATE_DAY;
							day_count++;
						}
					}
					break;
				case STATE_DAY:
					if (cur_lum_factor > auto_day_night_threshold.day_to_night_lum) {
						/* night mode */	
						if (STATE_NIGHT == day_mode_cmp_awb()) {
							day_night_level = STATE_NIGHT;
							night_count++;
						}
					}
					break;
				default:
					break;
			}
			ak_sleep_ms(500);
		}

		if (0 < day_count && CHECK_TIME > day_count)
			day_night_level = pre_ir_level;
		else if (0 < night_count && CHECK_TIME > night_count)
			day_night_level = pre_ir_level;
		else if (STATE_DAY == day_night_level && STATE_NIGHT == pre_ir_level) {

			struct ak_timeval cur_time;
	    	ak_get_ostime(&cur_time);
	    	long use_time = ak_diff_ms_time(&cur_time, &(soft_ir_info.lock_time));
			save_soft_ir_info();
			if (use_time < 15000)
			{
				soft_ir_info.night_lock = 1;
				day_night_level = pre_ir_level;
			}
			else {
				ak_print_info_ex("3----lock= 0\n");
				soft_ir_info.night_lock = 0; // ???
			}
		}
		else if (STATE_DAY == day_night_level && STATE_DAY == pre_ir_level) {
			memset(&soft_ir_info.lock_time, 0, sizeof(struct ak_timeval));
			ak_print_info_ex("4----lock= 0\n");
			soft_ir_info.night_lock = 2;
		}

	}
	
	ak_print_info_ex("(switch day_night) end check-daycount=%d,nightcount=%d \n\n", 
				day_count, night_count);
	
	return day_night_level;
}

/**
 * ak_vpss_isp_clean_auto_day_night_param: clean auto day or night switch threshold
 * return: NULL
 */
void ak_vpss_isp_clean_auto_day_night_param(void)
{
	soft_ir_info.init_flag = 0;
}

/**
 * ak_vpss_isp_get_auto_day_night_level: get day or night 
 * @pre_ir_level[IN]: pre status, 0 night, 1 day
 * return: 0 night, 1 day, -1 failed
 * notice: not suggest to use
 */
int ak_vpss_isp_get_input_level(struct ak_ir_auto_check *param)
{
	if (!param) {
		ak_print_error_ex("input param is NULL\n");
		return AK_FAILED;
	}

	if (0 == soft_ir_info.init_flag) {	
		struct ak_auto_day_night_threshold in_threshold;
		int i = 0;

		/* set threshold */
		in_threshold.day_to_night_lum = param->day_to_night_lum;
		in_threshold.night_to_day_lum = param->night_to_day_lum;
		in_threshold.lock_time = param->lock_time;
		in_threshold.quick_switch_mode = 0;

		for (i = 0; i < NIGHT_ARRAY_NUM; i++) 
			in_threshold.night_cnt[i] = param->night_cnt[i];	

		for (i = 0; i < DAY_ARRAY_NUM; i++) 
			in_threshold.day_cnt[0] = param->day_cnt[i];	
		
		ak_vpss_isp_set_auto_day_night_param(&in_threshold);
	}
	
	/* get auto day night level, know now is day or night */
	int ir_val = ak_vpss_isp_get_auto_day_night_level(param->pre_ir_level);

	return ir_val;


}

/**
 * brief: get current lum factor 
 * return: lum factor
 * notes:
 */
int ak_vpss_isp_get_cur_lumi(void)
{
	int lum_factor = 0;

	lum_factor = isp_get_cur_lum_factor();

	return lum_factor;
}

/**
 * brief: get night lock 
 * return: lock status
 * notes:
 */
int ak_vpss_isp_get_ir_lock(void)
{
	return soft_ir_info.night_lock;
}

/**
 * brief: get night lock time
 * return: lock status
 * notes:
 */
int ak_vpss_isp_get_ir_lock_time(void)
{
	return auto_day_night_threshold.lock_time;
}



/**
 * ak_vpss_isp_set_manual_blc: set manual blc
 * @vi_handle[IN]: vi module handle
 * @mblc[IN]: manual blc info
 * return: 0 success, -1 failed
 */
int ak_vpss_isp_set_manual_blc(const void *vi_handle, 
							const struct ak_manual_blc *mblc)
{
	if (!vi_handle) {
		ak_print_error_ex(VPSS_ISP "vi_handle is NULL\n");
		return AK_FAILED;
	}
	if (!mblc) {
		ak_print_error_ex(VPSS_ISP "blc is NULL\n");
		return AK_FAILED;
	}

	AK_ISP_BLC_ATTR blc = {0};
	if (AK_SUCCESS != AK_ISP_get_blc_attr(&blc))
    {
        ak_print_error_ex(VPSS_ISP "get blc failed.\n");
		return AK_FAILED;
    }   

    blc.m_blc.bl_r_offset = mblc->bl_r_offset;
    blc.m_blc.bl_gr_offset = mblc->bl_gr_offset;
    blc.m_blc.bl_gb_offset = mblc->bl_gb_offset;
    blc.m_blc.bl_b_offset = mblc->bl_b_offset;

	return AK_ISP_set_blc_attr(&blc);
}



/**
 * ak_vpss_af_get_stat: get af stat info
 * @vi_handle[IN]: vi module handle
 * @af_stat[OUT]: af_stat info
 * return: 0 success, -1 failed
 * notes:
 */
int ak_vpss_af_get_stat(const void *vi_handle, struct vpss_af_stat_info *af_stat)
{
	if (NULL == vi_handle)
	{
		ak_print_error_ex(VPSS_ISP "vi_handle is NULL\n");
		return AK_FAILED;
	}
    
	if (NULL == af_stat) 
    {
		ak_print_error_ex(VPSS_ISP "af_stat is null\n");
		return AK_FAILED;
	}

    unsigned int size = 0;
	int ret = isp_get_statinfo(ISP_AFSTAT, af_stat, &size);
    
	if (AK_SUCCESS != ret) 
    {
		ak_print_error_ex("get af stat info fail\n");
        return AK_FAILED;
	}

	return AK_SUCCESS;
}

/**
 * ak_vpss_set_af_attr: set af attr
 * @vi_handle[IN]: vi module handle
 * @af_stat[IN]: af_attr
 * return: 0 success, -1 failed
 * notes:
 */
int ak_vpss_set_af_attr(const void *vi_handle, struct vpss_af_attr *af_attr)
{
    if (NULL == vi_handle)
	{
		ak_print_error_ex(VPSS_ISP "vi_handle is NULL\n");
		return AK_FAILED;
	}
    
	if (NULL == af_attr) 
    {
		ak_print_error_ex(VPSS_ISP "af_attr is null\n");
		return AK_FAILED;
	}

    AK_ISP_AF_ATTR af = {0};

	int ret = AK_ISP_get_af_attr(&af);
    
	if (AK_SUCCESS != ret) 
    {
		ak_print_error_ex("AK_ISP_get_af_attr fail\n");
        return AK_FAILED;
	}

    af.af_th = af_attr->af_th;
    af.af_win0_left = af_attr->af_win0_left;
    af.af_win0_right = af_attr->af_win0_right;
    af.af_win0_top = af_attr->af_win0_top;
    af.af_win0_bottom = af_attr->af_win0_bottom;
    
    af.af_win1_left = af_attr->af_win1_left;
    af.af_win1_right = af_attr->af_win1_right;
    af.af_win1_top = af_attr->af_win1_top;
    af.af_win1_bottom = af_attr->af_win1_bottom;
    
    af.af_win2_left = af_attr->af_win2_left;
    af.af_win2_right = af_attr->af_win2_right;
    af.af_win2_top = af_attr->af_win2_top;
    af.af_win2_bottom = af_attr->af_win2_bottom;

    ret = AK_ISP_set_af_attr(&af);
    
	if (AK_SUCCESS != ret) 
    {
		ak_print_error_ex("AK_ISP_set_af_attr fail\n");
        return AK_FAILED;
	}

    ret = AK_ISP_set_af_win34_attr((AK_ISP_AF_ATTR *)af_attr);
    
	if (AK_SUCCESS != ret) 
    {
		ak_print_error_ex("AK_ISP_set_af_win34_attr fail\n");
        return AK_FAILED;
	}

	return AK_SUCCESS;
}

/**
 * ak_vpss_get_af_attr: set af attr
 * @vi_handle[IN]: vi module handle
 * @af_stat[OUT]: af_attr
 * return: 0 success, -1 failed
 * notes:
 */
int ak_vpss_get_af_attr(const void *vi_handle, struct vpss_af_attr *af_attr)
{
    if (NULL == vi_handle)
	{
		ak_print_error_ex(VPSS_ISP "vi_handle is NULL\n");
		return AK_FAILED;
	}
    
	if (NULL == af_attr) 
    {
		ak_print_error_ex(VPSS_ISP "af_attr is null\n");
		return AK_FAILED;
	}

    AK_ISP_AF_ATTR af = {0};

	int ret = AK_ISP_get_af_attr(&af);
    
	if (AK_SUCCESS != ret) 
    {
		ak_print_error_ex("AK_ISP_get_af_attr fail\n");
        return AK_FAILED;
	}

    af_attr->af_th = af.af_th;
    af_attr->af_win0_left = af.af_win0_left;
    af_attr->af_win0_right = af.af_win0_right;
    af_attr->af_win0_top = af.af_win0_top;
    af_attr->af_win0_bottom = af.af_win0_bottom;

    af_attr->af_win1_left = af.af_win1_left;
    af_attr->af_win1_right = af.af_win1_right;
    af_attr->af_win1_top = af.af_win1_top;
    af_attr->af_win1_bottom = af.af_win1_bottom;

    af_attr->af_win2_left = af.af_win2_left;
    af_attr->af_win2_right = af.af_win2_right;
    af_attr->af_win2_top = af.af_win2_top;
    af_attr->af_win2_bottom = af.af_win2_bottom;

	return AK_SUCCESS;
}

/**
 * brief: open wdr
 * return: 0 success, -1 failed
 * notes:
 */
int ak_vpss_open_wdr(void)
{
    int ret = -1;
    
    if (MODE_DAY_OUTDOOR == isp_get_day_night_mode() || MODE_NIGHTTIME == isp_get_day_night_mode())
    {
        ak_print_normal_ex("wdr is already opened!\n");
		return AK_SUCCESS;  
    }

    if (MODE_CUSTOM_2 == isp_get_day_night_mode())
    {
        ret = isp_switch(MODE_DAY_OUTDOOR);
    }
    else
    {
        ret = isp_switch(MODE_NIGHTTIME);
    }

    return ret;
}

/**
 * brief: close wdr
 * return: 0 success, -1 failed
 * notes:
 */
int ak_vpss_close_wdr(void)
{
    int ret = -1;
    
    if (MODE_CUSTOM_2 == isp_get_day_night_mode() || MODE_CUSTOM_3 == isp_get_day_night_mode())
    {
        ak_print_normal_ex("wdr is already closed!\n");
		return AK_SUCCESS;  
    }

    if (MODE_CUSTOM_2 == isp_get_day_night_mode())
    {
        ret = isp_switch(MODE_CUSTOM_2);
    }
    else
    {
        ret = isp_switch(MODE_CUSTOM_3);
    }

    return ret;
}


/**
 * brief: get wdr attr
 * @p_wdr[OUT]: wdr attr
 * return: 0 success, -1 failed
 * notes:
 */
int ak_vpss_get_wdr_attr(struct vpss_wdr_attr *p_wdr)
{
    int ret = -1;

    if (NULL == p_wdr) 
    {
        ak_print_error_ex("p_wdr is null\n");
        return ret;
    }
    
	ret = AK_ISP_get_wdr_attr((AK_ISP_WDR_ATTR*)p_wdr);

    if (AK_SUCCESS != ret) 
    {
		ak_print_error_ex("get wdr attr failed.\n");
		return ret;
	}

    return AK_SUCCESS;

}

