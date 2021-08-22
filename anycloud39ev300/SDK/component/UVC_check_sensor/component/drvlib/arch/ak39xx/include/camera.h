/* @file camera.h
 * @brief Define structures and functions of camera driver
 * This file provide APIs of Camera, such as open, close, capture image. etc.
 * Copyright (C) 2010 Anyka (Guangzhou) Microelectronics Technology Co., Ltd
 * @author xia_wenting 
 * @date 2011-03-30
 * @version 1.0
 */

#ifndef __CAMERA_H__
#define __CAMERA_H__

#include "ak_isp_drv.h"

#ifndef BURNTOOL

typedef void (*T_fCamera_Interrupt_CALLBACK)(void);

struct camera_info {
	unsigned long cam_width;
	unsigned long cam_height;
	unsigned long ch1_dstWidth;
	unsigned long ch1_dstHeight;
	unsigned long ch2_dstWidth;
	unsigned long ch2_dstHeight;
	unsigned char *ch1_dYUV0;
	unsigned char *ch1_dYUV1;
	unsigned char *ch1_dYUV2;
	unsigned char *ch1_dYUV3;
	unsigned char *ch2_dYUV0;
	unsigned char *ch2_dYUV1;
	unsigned char *ch2_dYUV2;
	unsigned char *ch2_dYUV3;
	int crop_left;
	int crop_top;
	int crop_width;
	int crop_height;
};
 
 /**
  * @brief reset camera 
  * @author xia_wenting  
  * @date 2010-12-06
  * @return void
  */
 void camctrl_enable(void);

 /**
 * @brief open camera, should be done the after reset camera to initialize 
 * @author xia_wenting  
 * @date 2010-12-06
 * @param[in] mclk send to camera mclk 
 * @return bool
 * @retval true if successed
 * @retval false if failed
 */
bool camctrl_open(unsigned long mclk);

/**
 * @brief close camera 
 * @author xia_wenting  
 * @date 2010-12-06
 * @return void
 */
void camctrl_disable(void);


/**
 * @brief set interrupt callback function
 * @author xia_wenting  
 * @date 2010-12-01
 * @param[in] callback_func callbak function
 * @return 
 * @retval 
 */
void camctrl_set_interrupt_callback(T_fCamera_Interrupt_CALLBACK callback_func);

/**
 * @brief read camera controller's register, and check the frame finished or occur errorred
 * @author xia_wenting   
 * @date 2010-12-06
 * @param
 * @return bool
 * @retval true the frame finished
 * @retval false the frame not finished or occur errorred
 */
bool camctrl_check_status(void);

/**
 * @brief init libispdrv
 * @author ye_guohong   
 * @date 2016-10-19
 * @param[in] isp_sensor_cb sensor' callback function registered to libispdrv
 * @return bool
 * @retval true if init sucessed
 * @retval false if init failed
 */
bool camctrl_init_ispdrv(AK_ISP_SENSOR_CB *isp_sensor_cb);

/**
 * @brief deinit libispdrv
 * @author ye_guohong   
 * @date 2016-10-19
 * @param[in] isp_sensor_cb sensor' callback function registered to libispdrv
 * @return bool
 * @retval true if init sucessed
 * @retval false if init failed
 */
bool camctrl_deinit_ispdrv(void);

/**
 * @brief set sensor & ch1 & ch2 resolution, buffers infomation
 * @author ye_guohong   
 * @date 2016-10-19
 * @param[in] cam_info set cam information
 * @return bool
 * @retval true if set sucessed
 * @retval false if set failed
 */
bool camctrl_set_info(struct camera_info *cam_info);

/**
 * @brief set isp to start capture video
 * @author ye_guohong   
 * @date 2016-10-19
 * @param[in] isp_working_mode isp working mode for capturing
 * @return void
 */
void camctrl_start_captuing(enum isp_working_mode mode);

/**
 * @brief set isp to stop capture video
 * @author ye_guohong   
 * @date 2016-10-19
 * @param
 * @return void
 */
void camctrl_stop_captuing(void);

/**
 * @brief enable isp buffer to work
 * @author ye_guohong   
 * @date 2016-10-19
 * @param[in] id buffer's index
 * @return void
 */
void camctrl_enable_buffer(unsigned char id);

/**
 * @brief isp's frequent works 
 * @author ye_guohong   
 * @date 2016-10-19
 * @param
 * @return void
 */
void camctrl_isp_works(void);

/**
 * @brief pause isp not to capture video
 * @author ye_guohong   
 * @date 2016-10-19
 * @param
 * @return bool
 * @retval true if pause sucessed
 * @retval false if pause failed
 */
bool camctrl_pause_isp(void);

/**
 * @brief resume isp not to capture video
 * @author ye_guohong   
 * @date 2016-10-19
 * @param
 * @return bool
 * @retval true if resume sucessed
 * @retval false if resume failed
 */
bool camctrl_resume_isp(void);

#endif

/*@}*/

#endif //__ARCH_CAMERA_H__


