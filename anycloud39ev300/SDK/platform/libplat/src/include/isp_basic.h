#ifndef _ISP_BASIC_H_
#define _ISP_BASIC_H_

#include "ak_isp_char.h"
#include "ak_vpss.h"

#define ISP_MODULE_MAX_SIZE		(12*1024)

enum isp_module_id{
	/***** ISP_BB to ISP_SENSOR must be same with isptool ********/
	ISP_BB = 0,   			//black balance
	ISP_LSC,				//lsc
	ISP_RAW_LUT,			//raw gamma
	ISP_NR,					//NR
	ISP_3DNR,				//3DNR

	ISP_GB,					//green balance
	ISP_DEMO,				//demosaic
	ISP_GAMMA,				//rgb gamma
	ISP_CCM,				//color correction
	ISP_FCS,				//FCS

	ISP_WDR,				//width dynamic range
	ISP_SHARP,				//sharp
	ISP_SATURATION,			//saturation
	ISP_CONSTRAST,			//contrast

	ISP_RGB2YUV,			//rgb to yuv
	ISP_YUVEFFECT,			//YUV effect
	ISP_DPC,				//dpc
	ISP_WEIGHT,				//zone weight
	ISP_AF,					//auto focus

	ISP_WB,					//white balance
	ISP_EXP,				//Expsoure
	ISP_MISC,				//misc
	ISP_Y_GAMMA,			//y gamma
	ISP_HUE,				//hue

	ISP_3DSTAT,				//3DNR stat info
	ISP_AESTAT,				//AE stat info
	ISP_AFSTAT,				//AF stat info
	ISP_AWBSTAT,			//AWB stat info

	ISP_SENSOR,				//sensor params table

	/********** add new items for new requirement ************/
	ISP_FPS_CTRL,			//frame rate control
	ISP_AESTAT_SUB_RGB_STAT,
	ISP_EV_TH,				//EV threshold

    ISP_MODULE_ID_NUM
};

/************************** for  isptuner **********************************/
/**  
 * brief: isp device (isp char) open
 * @void
 * return: 0 success, -1 failed
 * notes: 
 */ 
int isp_dev_open(void);

/**  
 * brief: isp device (isp char) close
 * @void
 * return: 0 success, -1 failed
 * notes: 
 */ 
int isp_dev_close(void);

/**  
 * brief: get isp module data
 * @module_id[IN]: id of the module in enum isp_module_id
 * @buf[OUT]: dest buf for data out
 * @size[OUT]: size of the attr
 * return: 0 success, -1 failed
 * notes: 
 */ 
int isp_get_attr(enum isp_module_id module_id, void *buf, unsigned int *size);

/**  
 * brief: set isp module data
 * @module_id[IN]: id of the module in enum isp_module_id
 * @buf[IN]: data buf to set
 * @size[OUT]: size of the attr
 * return: 0 success, -1 failed
 * notes: 
 */ 
int isp_set_attr(enum isp_module_id module_id, char *buf, unsigned int size);

/**  
 * brief: get isp stat info data
 * @module_id[IN]: id of the module in enum isp_module_id
 * @buf[OUT]: dest buf for data out
 * @size[OUT]: size of the attr
 * return: 0 success, -1 failed
 * notes: 
 */
int isp_get_statinfo(enum isp_module_id module_id, void *buf, unsigned int *size);

/**  
 * brief: get sensor register value
 * @addr[IN]:sensor register addr
 * @value[OUT]: value of the register addr
 * return: 0 success, -1 failed
 * notes: 
 */
int isp_get_sensor_value(unsigned short addr, unsigned short *value);

/**  
 * brief: set sensor register value
 * @addr[IN]:sensor register addr
 * @value[IN]: value to set
 * return: 0 success, -1 failed
 * notes: 
 */
int isp_set_sensor_value(unsigned short addr, unsigned short value);

/************************** for  internal other modules **********************************/
/**  
 * brief: get sensor id
 * @id[OUT]: sensor id
 * return: 0 success, -1 failed
 * notes: 
 */
int isp_get_sensor_id(int *id);

/**  
 * brief: isp module init
 * @cfgbuf[IN]:config data buf
 * @size[IN]: size of config data buf
 * return: 0 success, -1 failed
 * notes: 
 */
int isp_module_init(char *cfgbuf, unsigned int size);

/**  
 * brief: isp module deinit
 * @void
 * return: 0 success, -1 failed
 * notes: 
 */
int isp_module_deinit(void);

/**  
 * brief: day night switch
 * @cfgbuf[IN]:config data buf
 * @size[IN]: size of config data buf
 * return: 0 success, -1 failed
 * notes: 
 */
int isp_switch_mode(char *cfgbuf, unsigned int size);

/**  
 * brief: check config data
 * @cfgbuf[IN]:config data buf
 * @size[IN,OUT]: in ,size to check ; out, size that checked ok
 * return: 0 success, -1 failed
 * notes: 
 */
int isp_module_check_cfg(char *cfgbuf, unsigned int *size);

/**  
 * brief: get effect
 * @type[IN]:effect type
 * @value[OUT]: effect value
 * return: real value, -1 failed
 * notes: 
 */
int isp_get_effect(enum vpss_effect_type type, int *value);

/**
 * isp_set_effect - set effect
 * @type[IN]:effect type
 * @value[IN]: effect value, [-50, 50], 0 means use the value in ispxxx.conf
 * return: 0 success, -1 failed
 */
int isp_set_effect(enum vpss_effect_type type, int value);

int isp_get_hz(void);

/**  
 * brief: set power hz
 * @value[IN]: power hz, 50 or 60
 * return: 0 success, -1 failed
 * notes: 
 */
int isp_set_hz(int hz);

/**  
 * brief: get flip mirror flag
 * @value[OUT]: info, flip flag and mirror flag
 * return: 0 success, -1 failed
 * notes: 
 */
int isp_get_flip_mirror(struct isp_flip_mirror_info *info);

/**  
 * brief: set flip mirror
 * @value[IN]: info, flip flag and mirror flag
 * return: 0 success, -1 failed
 * notes: 
 */
int isp_set_flip_mirror(struct isp_flip_mirror_info *info);

/**  
 * brief: set sensor framerate 
 * @value[IN]: fps, sensor framerate
 * return: 0 success, -1 failed
 * notes: 
 */
int isp_set_sensor_fps(const int fps);

/**  
 * brief: get sensor current work scene
 * @void
 * return: current work scene
 */
int isp_get_work_scene(void);

/**  
 * brief: malloc 3D NR memery
 * @width[IN]: main channel resulution of width
 * @height[IN]: main channel resulution of height 
 * return: memery address
 * notes: 
 */
void *isp_3D_NR_create(int width, int height);

/**  
 * brief: create fps thread
 * @void
 * return: 0 success, -1 failed
 * notes: 
 */
int isp_fps_main(void);

/**  
 * brief: set main channel mask
 * @p_mask[IN]: main channel mask paramters
 * return: 0 success, -1 failed
 * notes: 
 */
int isp_set_main_mask_area(const struct isp_mask_area_info *p_mask);

/**  
 * brief: get main channel mask
 * @p_mask[OUT]: main channel mask paramters
 * return: 0 success, -1 failed
 * notes: 
 */
int isp_get_main_mask_area(struct isp_mask_area_info *p_mask);

/**  
 * brief: set sub channel mask
 * @p_mask[IN]: sub channel mask paramters
 * return: 0 success, -1 failed
 * notes: 
 */
int isp_set_sub_mask_area(const struct isp_mask_area_info *p_mask);

/**  
 * brief: get sub channel mask
 * @p_mask[OUT]: sub channel mask paramters
 * return: 0 success, -1 failed
 * notes: 
 */
int isp_get_sub_mask_area(struct isp_mask_area_info *p_mask);

/**  
 * brief: set mask color
 * @p_mask[IN]: mask color paramters
 * return: 0 success, -1 failed
 * notes: 
 */
int isp_set_mask_color(const struct isp_mask_color_info *p_mask);

/**  
 * brief: get mask color
 * @p_mask[OUT]: mask color paramters
 * return: 0 success, -1 failed
 * notes: 
 */
int isp_get_mask_color(struct isp_mask_color_info *p_mask);

int isp_get_sensor_fps(void);
int isp_set_switch_fps_enable(int enable);

int isp_get_force_anti_flicker_flag(void);

int isp_set_force_anti_flicker_flag(int force_flag);

/**
 * brief: get current lum factor 
 * return: lum factor
 * notes:
 */
int isp_get_cur_lum_factor(void);

#endif
