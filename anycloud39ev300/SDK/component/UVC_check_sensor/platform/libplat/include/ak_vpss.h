#ifndef __AK_VPSS_H__
#define __AK_VPSS_H__

/********************** osd *******************************/
#define VPSS_OSD_CHANNELS_MAX 2

/********************** md *******************************/
#define VPSS_MD_DIMENSION_H_MAX 32
#define VPSS_MD_DIMENSION_V_MAX 16

/********************** mask *******************************/
#define VPSS_MASK_AREA_MAX	4

/********************** od *******************************/
#define VPSS_OD_AF_STATICS_MAX	5
#define VPSS_OD_RGB_HIST_MAX	256

/********************** osd *******************************/
enum vpss_osd_set_id {
	OSD_SET_COLOR_TABLE = 1,	
	OSD_SET_MAIN_CHANNEL_DATA,
	OSD_SET_SUB_CHANNEL_DATA,	
	OSD_SET_MAIN_DMA_MEM_REQUST,
	OSD_SET_SUB_DMA_MEM_REQUST,
};

struct vpss_osd_param {
	enum vpss_osd_set_id id;
	unsigned char data[128];
};

/********************** md *******************************/
struct vpss_md_info {
	unsigned short stat_max;
	unsigned short stat[VPSS_MD_DIMENSION_V_MAX][VPSS_MD_DIMENSION_H_MAX];
	unsigned int level;
};

/********************** mask *******************************/
enum vpss_mask_color_type {
	VPSS_MASK_ORIGINAL_VIDEO = 0,	/* masked with original video data*/
	VPSS_MASK_MOSAIC_VIDEO,			/* masked with mosaic video data*/
	VPSS_MASK_MOSAIC_WHITE_VIDEO,	/* masked with mosaic & white 50% video data*/
	VPSS_MASK_MOSAIC_BLACK_VIDEO,	/* masked with mosaic & black 50% video data*/
};

struct vpss_mask_color_info {
	/* defined by enum vpss_mask_color_type*/
	unsigned char color_type;

	/* range [0, 0xff]. if 0 mask video opaquely, then can not see image */
	unsigned char mk_alpha;

	/* range [0, 0xff] */
	unsigned char y_mk_color;
	/* range [0, 0xff] */
	unsigned char u_mk_color;
	/* range [0, 0xff] */
	unsigned char v_mk_color;
};

struct vpss_mask_area {
	unsigned short start_xpos;
	unsigned short end_xpos;
	unsigned short start_ypos;
	unsigned short end_ypos;
	unsigned char enable;
};

struct vpss_mask_area_info {
	struct vpss_mask_area main_mask[VPSS_MASK_AREA_MAX];
	struct vpss_mask_area sub_mask[VPSS_MASK_AREA_MAX];
};

/********************** od *******************************/
struct vpss_od_info {
	unsigned int af_statics[VPSS_OD_AF_STATICS_MAX];
	unsigned int rgb_total;
	unsigned int rgb_hist[VPSS_OD_RGB_HIST_MAX];
};

/********************** effect *******************************/
enum vpss_effect_type {
	/* HUE to SHARP, value: [-50, 50], 0 means use the value in ISP config file */
	VPSS_EFFECT_HUE = 0x00,
	VPSS_EFFECT_BRIGHTNESS,
	VPSS_EFFECT_SATURATION,
	VPSS_EFFECT_CONTRAST,
	VPSS_EFFECT_SHARP,
	
	VPSS_STYLE_ID,		//[0, 2]
	VPSS_POWER_HZ		//50 or 60
};

/********************** ISP *******************************/
struct vpss_isp_ae_attr {
	unsigned long exp_time_max;			//曝光时间的最大值
    unsigned long exp_time_min;			//曝光时间的最小值
    unsigned long d_gain_max;      		//数字增益的最大值
    unsigned long d_gain_min;     		//数字增益的最小值
    unsigned long isp_d_gain_min;  		//isp数字增益的最小
    unsigned long isp_d_gain_max;  		//isp数字增益的最大值
    unsigned long a_gain_max;     		//模拟增益的最大值
    unsigned long a_gain_min;      		//模拟增益的最小值
    unsigned long exp_step;            	//用户曝光调整步长
    unsigned long exp_stable_range;     //稳定范围
    unsigned long target_lumiance;     	//目标亮度
    unsigned long envi_gain_range[10][2];
    unsigned long hist_weight[16];
    unsigned long OE_suppress_en;
    unsigned long OE_detect_scope;		//[0,255]
    unsigned long OE_rate_max;			//[0, 255]
    unsigned long OE_rate_min;			//[0, 255]
};

struct vpss_isp_wb_type_attr {
    unsigned short wb_type;
};

struct vpss_isp_3d_nr {
	unsigned short uv_min_enable;
	unsigned short tnr_y_enable;
	unsigned short tnr_uv_enable;
	unsigned short updata_ref_y;		//更新Y
	unsigned short updata_ref_uv;		//更新uv
	unsigned short tnr_refFrame_format;	//参考帧的格式
	unsigned short y_2dnr_enable;
	unsigned short uv_2dnr_enable;

	unsigned short uvnr_k; 				//[0, 15]
	unsigned short uvlp_k; 				//[0, 15]
	unsigned short t_uv_k; 				//[0, 127]
	unsigned short t_uv_minstep;		//[0,31]
	unsigned short t_uv_mf_th1;			//[0, 8191]
	unsigned short t_uv_mf_th2;			//[0, 8191]
	unsigned short t_uv_diffth_k1; 		//[0, 255]
	unsigned short t_uv_diffth_k2; 		//[0, 255]
	unsigned short t_uv_diffth_slop;	//[0, 255]
	unsigned short t_uv_mc_k;			//[0-31]
	unsigned short t_uv_ac_th; 			//[0, 1023]

	unsigned short ynr_weight_tbl[17];
	unsigned short ynr_calc_k;
	unsigned short ynr_k;				//[0, 15]
	unsigned short ynr_diff_shift; 		//[0,1]
	unsigned short ylp_k;				//[0, 15]
	unsigned short t_y_th1;				//[0, 255]
	unsigned short t_y_k1; 				//[0, 127]
	unsigned short t_y_k2; 				//[0, 127]
	unsigned short t_y_kslop;			//[0, 127]
	unsigned short t_y_minstep;			//[0-31]
	unsigned short t_y_mf_th1; 			//[0, 8191]
	unsigned short t_y_mf_th2; 			//[0, 8191]
	unsigned short t_y_diffth_k1;		//[0, 255]
	unsigned short t_y_diffth_k2;		//[0, 255]
	unsigned short t_y_diffth_slop;		//[0, 255]
	unsigned short t_y_mc_k;			//[0-31]
	unsigned short t_y_ac_th;	    	//[0, 1023]

	unsigned long md_th;				//[0, 65535]  运动检测阈值 [0-127]
};

struct vpss_isp_3d_nr_attr {
    unsigned short        _3d_nr_mode;
    struct vpss_isp_3d_nr manual_3d_nr;
    struct vpss_isp_3d_nr linkage_3d_nr[9];
};

struct vpss_isp_mwb_attr {
    unsigned short r_gain;
    unsigned short g_gain;
    unsigned short b_gain;
    short r_offset;
    short g_offset;
    short b_offset;
};

struct vpss_isp_awb_attr {
    unsigned short g_weight[16];
    unsigned short y_low;				//y_low<=y_high
    unsigned short y_high;
	unsigned short err_est;
    unsigned short gr_low[10];			//gr_low[i]<=gr_high[i]
    unsigned short gb_low[10]; 			//gb_low[i]<=gb_high[i]
    unsigned short gr_high[10];
    unsigned short gb_high[10];
    unsigned short rb_low[10];			//rb_low[i]<=rb_high[i]
    unsigned short rb_high[10];

    //awb软件部分需要设置的参数
    unsigned short auto_wb_step;                //白平衡步长计算
    unsigned short total_cnt_thresh;            //像素个数阈值
    unsigned short colortemp_stable_cnt_thresh; //稳定帧数，多少帧一样认为环境色温改变
    unsigned short colortemp_envi[10];
};

struct vpss_isp_exp_type {
    unsigned short  exp_type;
};

struct vpss_isp_ae_run_info {
    unsigned char current_calc_avg_lumi;         		//现在的计算出的亮度值
    unsigned char current_calc_avg_compensation_lumi;	//经过曝光补偿后的亮度值

    unsigned char current_darked_flag;                 	//白天黑夜的标记

    long  current_a_gain;				//模拟增益的值
    long  current_d_gain;				//数字增益的值
    long  current_isp_d_gain;			//isp数字增益的值
    long  current_exp_time;				//曝光时间的值

    unsigned long  current_a_gain_step;	//现在的模拟增益的步长
    unsigned long  current_d_gain_step; //数字增益的步长
    unsigned long  current_isp_d_gain_step;	//isp数字增益的步长
    unsigned long  current_exp_time_step;	//曝光时间的步长
};

struct vpss_isp_sensor_reg_info {
    unsigned short reg_addr;
    unsigned short value;
};

/********************** public *******************************/

/**
 * ak_vpss_get_version - get vpss version
 * return: version string
 */
const char *ak_vpss_get_version(void);

/********************** osd *******************************/
/**  
 * ak_vpss_osd_set_param : set osd param to isp and then to driver
 * @vi_handle[IN]: vi module handle
 * @param[IN]: the osd param  
 * return: 0 success, otherwise failed
 * notes: 
 */
int ak_vpss_osd_set_param(const void *vi_handle, struct vpss_osd_param *param);

/**  
 * ak_vpss_osd_get_param: get osd param to isp and then to driver
 * @vi_handle[IN]: vi module handle
 * @param[OUT]: the osd param  
 * return: 0 success, otherwise failed
 * notes: 
 */
int ak_vpss_osd_get_param(const void *vi_handle, struct vpss_osd_param *param);

/**  
 * ak_vpss_osd_close: close specifically osd channel
 * @vi_handle[IN]: vi module handle
 * @param[IN]: the osd param  
 * return: 0 success, otherwise failed
 * notes: 
 */
int ak_vpss_osd_close(const void *vi_handle, struct vpss_osd_param *param);

/********************** md *******************************/
/**
 * ak_vpss_md_get_stat: get motion detection stat params
 * @vi_handle[IN]: vi module handle
 * @md[OUT]: md params 
 * return: 0 success, -1 failed
 * notes:
 */
int ak_vpss_md_get_stat(const void *vi_handle, struct vpss_md_info *md);

/********************** mask *******************************/
/**  
 * brief: set main & sub channel mask area
 * @vi_handle[IN]: vi module handle
 * @p_mask[IN]: main & sub channel mask area paramters
 * return: 0 success, -1 failed
 * notes: 
 */
int ak_vpss_mask_set_area(const void *vi_handle, 
		const struct vpss_mask_area_info *mask);

/**  
 * brief: get main & sub channel mask area
 * @vi_handle[IN]: vi module handle
 * @p_mask[OUT]: main & sub channel mask area paramters
 * return: 0 success, -1 failed
 * notes: 
 */
int ak_vpss_mask_get_area(const void *vi_handle, 
		struct vpss_mask_area_info *mask);

/**  
 * brief: set main & sub channel mask color
 * @vi_handle[IN]: vi module handle
 * @color[IN]: main & sub channel mask color paramters
 * return: 0 success, -1 failed
 * notes: 
 */
int ak_vpss_mask_set_color(const void *vi_handle, 
		const struct vpss_mask_color_info *color);

/**  
 * brief: get main & sub channel mask color
 * @vi_handle[IN]: vi module handle
 * @color[OUT]: main & sub channel mask color paramters
 * return: 0 success, -1 failed
 * notes: 
 */
int ak_vpss_mask_get_color(const void *vi_handle, 
		struct vpss_mask_color_info *color);

/********************** od *******************************/
/**
 * brief:  get occlusion detection params
 * @vi_handle[IN]: opened video input handle
 * @od[OUT]: od params 
 * return: 0 success, -1 failed
 * notes:
 */
int ak_vpss_od_get(const void *vi_handle, struct vpss_od_info *od);

/********************** effect *******************************/
/**  
 * ak_vpss_effect_get - get isp effect param. 
 * @vi_handle[IN]: vi module handle 
 * @type[IN]:   effect type name 
 * @effect[OUT]: effect value(s) 
 * return: 0 - success; otherwise -1;  
 */
int ak_vpss_effect_get(void *vi_handle, enum vpss_effect_type type, int *effect);

/** 
 * ak_vpss_effect_set - set isp effect param. 
 * @vi_handle[IN]: vi module handle
 * @type[IN]:   effect type name
 * @effect[IN]: effect value
 * return: 0 - success; otherwise -1; 
 */
int ak_vpss_effect_set(void *vi_handle, enum vpss_effect_type type, 
				const int effect);

/********************** ISP *******************************/
/**
 * ak_vpss_isp_init_sdk: init isp sdk
 * @void
 * return: 0 success, -1 failed
 */
int ak_vpss_isp_init_sdk(void);

/**
 * ak_vpss_isp_exit_sdk: exit isp sdk
 * @void
 * return: 0 success, -1 failed
 */
int ak_vpss_isp_exit_sdk(void);

/**
 * ak_vpss_isp_get_ae_attr: get AE attr
 * @vi_handle[IN]: vi module handle
 * @ae_attr[OUT]: AE attr info
 * return: 0 success, -1 failed
 */
int ak_vpss_isp_get_ae_attr(const void *vi_handle,
							struct vpss_isp_ae_attr *ae_attr);

/**
 * ak_vpss_isp_set_ae_attr: set AE attr
 * @vi_handle[IN]: vi module handle
 * @ae_attr[IN]: AE attr info
 * return: 0 success, -1 failed
 */
int ak_vpss_isp_set_ae_attr(const void *vi_handle,
							const struct vpss_isp_ae_attr *ae_attr);

/**
 * ak_vpss_isp_get_wb_type: get wb type
 * @vi_handle[IN]: vi module handle
 * @wb_type[OUT]: wb type
 * return: 0 success, -1 failed
 */
int ak_vpss_isp_get_wb_type(const void *vi_handle,
							struct vpss_isp_wb_type_attr *wb_type);

/**
 * ak_vpss_isp_set_wb_type: set wb type
 * @vi_handle[IN]: vi module handle
 * @wb_type[IN]: wb type
 * return: 0 success, -1 failed
 */
int ak_vpss_isp_set_wb_type(const void *vi_handle,
							const struct vpss_isp_wb_type_attr *wb_type);

/**
 * ak_vpss_isp_get_3d_nr_attr: get 3D NR attr
 * @vi_handle[IN]: vi module handle
 * @nr_3d_attr[OUT]: 3D NR attr
 * return: 0 success, -1 failed
 */
int ak_vpss_isp_get_3d_nr_attr(const void *vi_handle,
							struct vpss_isp_3d_nr_attr *nr_3d_attr);

/**
 * ak_vpss_isp_set_3d_nr_attr: set 3D NR attr
 * @vi_handle[IN]: vi module handle
 * @nr_3d_attr[IN]: 3D NR attr
 * return: 0 success, -1 failed
 */
int ak_vpss_isp_set_3d_nr_attr(const void *vi_handle,
							const struct vpss_isp_3d_nr_attr *nr_3d_attr);

/**
 * ak_vpss_isp_get_mwb_attr: get mwb attr
 * @vi_handle[IN]: vi module handle
 * @mwb_attr[OUT]: mwb attr
 * return: 0 success, -1 failed
 */
int ak_vpss_isp_get_mwb_attr(const void *vi_handle,
							struct vpss_isp_mwb_attr *mwb_attr);

/**
 * ak_vpss_isp_set_mwb_attr: set mwb attr
 * @vi_handle[IN]: vi module handle
 * @mwb_attr[IN]: mwb attr
 * return: 0 success, -1 failed
 */
int ak_vpss_isp_set_mwb_attr(const void *vi_handle,
							const struct vpss_isp_mwb_attr *mwb_attr);

/**
 * ak_vpss_isp_get_awb_attr: get awb attr
 * @vi_handle[IN]: vi module handle
 * @awb_attr[OUT]: awb attr
 * return: 0 success, -1 failed
 */
int ak_vpss_isp_get_awb_attr(const void *vi_handle,
							struct vpss_isp_awb_attr *awb_attr);
/**
 * ak_vpss_isp_set_awb_attr: set awb attr
 * @vi_handle[IN]: vi module handle
 * @awb_attr[IN]: awb attr
 * return: 0 success, -1 failed
 */
int ak_vpss_isp_set_awb_attr(const void *vi_handle,
							const struct vpss_isp_awb_attr *awb_attr);

/**
 * ak_vpss_isp_get_exp_type: get exp type
 * @vi_handle[IN]: vi module handle
 * @exp_type[IN]: exp type
 * return: 0 success, -1 failed
 */
int ak_vpss_isp_get_exp_type(const void *vi_handle,
							struct vpss_isp_exp_type *exp_type);

/**
 * ak_vpss_isp_set_exp_type: set exp type
 * @vi_handle[IN]: vi module handle
 * @exp_type[IN]: exp type
 * return: 0 success, -1 failed
 */
int ak_vpss_isp_set_exp_type(const void *vi_handle,
							const struct vpss_isp_exp_type *exp_type);

/**
 * ak_vpss_isp_get_ae_run_info: get ae run info
 * @vi_handle[IN]: vi module handle
 * @ae_run_info[OUT]: ae run info
 * return: 0 success, -1 failed
 */
int ak_vpss_isp_get_ae_run_info(const void *vi_handle,
							struct vpss_isp_ae_run_info *ae_run_info);

/**
 * ak_vpss_isp_get_sensor_reg: get sensor register info
 * @vi_handle[IN]: vi module handle
 * @sensor_reg_info[IN/OUT]: sensor register info
 * return: 0 success, -1 failed
 */
int ak_vpss_isp_get_sensor_reg(const void *vi_handle,
							struct vpss_isp_sensor_reg_info *sensor_reg_info);

#endif
