/**
 * @file hal_camera.h
 * @brief provide interfaces for high layer operation of Camera
 * Copyright (C) 2010 Anyka (Guangzhou) Microelectronics Technology Co., Ltd
 * @author xia_wenting
 * @date 2010-12-07
 * @version 1.0
 * @note this an example to use the API of camera driver
 *
 *            cam_open();        //open it for init
 *
 *            
 *            //set camera mode parameter if needed
 *            cam_set_to_prev(srcWidth, srcHeight);
 *
 *            //set camera feature, such as AWB, effect, brightness.etc.
 *            cam_set_feature(CAM_FEATURE_EFFECT, CAMERA_EFFECT_NEGATIVE); 
 *            ......
 *
 *            //capture a photo in YUV mode
 *            cam_capture_YUV(dy, du, dv, dstw, dsth, timeout);
 *        
 *            //then display on LCD or save it
 *            ......
 */


#ifndef __HAL_CAMERA_H__
#define __HAL_CAMERA_H__

/** @defgroup Hal_camera Hardware Abstract Layer of camera
 *    @ingroup Camera
 */
/*@{*/

#include "anyka_types.h"

/******************************************************************************************
 *    the following define the camera device register interface *      
******************************************************************************************/

typedef struct
{
    unsigned long   width;
    unsigned long   height;
	unsigned long	ts;
	unsigned char	*dYUV;
    unsigned char    status;
	unsigned char 	id;
}T_CAMERA_BUFFER;

struct camstream_info {
	unsigned char  ch1_enable;    //主通道使能标志，0为不使能，其他值为使能。
	unsigned char  ch2_enable;    //次通道使能标志，功能同ch1_enable.
	unsigned char  single_mode;   //视频模式。0为连续帧模式，1为单帧模式。注意：当为连续帧模式时buffer_num必须为4
	unsigned char  buffer_num;    //表示使用前几个ch1_YUV、ch2_YUV的buffer进行采集。注意：当为连续帧模式时buffer_num必须为4
	unsigned long ch1_dstWidth;
	unsigned long ch1_dstHeight;
	unsigned long ch2_dstWidth;
	unsigned long ch2_dstHeight;
	T_CAMERA_BUFFER *ch1_YUV[4];
	T_CAMERA_BUFFER *ch2_YUV[4];
	int crop_left;
	int crop_top;
	int crop_width;
	int crop_height;
};

/**
 * @brief open camera, should be done the after reset camera to  initialize 
 * @author xia_wenting  
 * @date 2010-12-06
 * @return bool
 * @retval true if successed
 * @retval false if failed
 */
bool cam_open(void);

/**
 * @brief close camera 
 * @author xia_wenting  
 * @date 2010-12-06
 * @return void
 */
void cam_close(void);

/******************************************************************************************
 *    the following define the camera stream recording interface *      
******************************************************************************************/
/** 
 * @brief init camera interrupt mode
 * @author ye_guohong
 * @date 2016-12-19
 * @param[in] stream_info  information for initiation
 * @return bool
 * @retval true init successfully
 * @retval false init unsuccessfully
 */
bool camstream_init(struct camstream_info *stream_info);


/** 
 * @brief notify app when data ready
 * @author yi_ruoxiang
 * @date 2010-09-01
 * @param[in] void 
 * @return void
 */
typedef void (*T_fCAMSTREAMCALLBACK)(void);

/** 
 * @brief set notify callback function
 * @author yi_ruoxiang
 * @date 2010-09-01
 * @param[in] callback_func callback function
 * @return void
*/
void camstream_set_callback(T_fCAMSTREAMCALLBACK callback_func);

/** 
 * @brief change camera configure
 * @author xia_wenting
 * @date 2010-12-07
 * @param[in] dstWidth  camera dest width
 * @param[in] dstHeight camera dest height
 * @param[in] YUV1     store a frame YUV buffer, can be NULL
 * @param[in] YUV2     store a frame YUV buffer, can be NULL
 * @param[in] YUV3     store a frame YUV buffer, can be NULL
 * @return bool
 * @retval true change successfully
 * @retval false change unsuccessfully
 */
bool camstream_change(unsigned long dstWidth, unsigned long dstHeight,
                       T_CAMERA_BUFFER *YUV1, T_CAMERA_BUFFER *YUV2, T_CAMERA_BUFFER *YUV3);

/** 
 * @brief stop camera(interrupt mode)
 * @author yi_ruoxiang
 * @date 2010-09-01
 * @return void
 */
void camstream_stop(void);

/** 
 * @brief a frame is ready
 * @author yi_ruoxiang
 * @date 2010-09-01
 * @return bool
 * @retval false: no data
 * @retval true: a frame is ready
 */
bool camstream_ready(void);

/** 
 * @brief get a frame data
 * @author yi_ruoxiang
 * @date 2010-09-01
 * @return T_CAMERA_BUFFER*
 * @retval pointer of current frame data
 */
T_CAMERA_BUFFER *camstream_get(void);

/** 
 * @brief release camera buffer
 * @author ye_guohong
 * @date 2016-12-15
 * @param[in] id: buffer id
 * @return void
 * @retval 
 */
void camstream_queue_buffer(int id);

/** 
 * @brief suspend camera interface, only accept current frame, other not accept
 * @author yi_ruoxiang
 * @date 2010-09-01
 * @return bool
 * @retval false: suspend failed
 * @retval true: suspend successful
 */
bool camstream_suspend(void);

/** 
 * @brief resume camera interface, start accept frame
 * @author yi_ruoxiang
 * @date 2010-09-01
 * @return void
 */
void camstream_resume(void);


/** 
 * @brief get sensor ID
 * @author kjp
 * @date 2017-01-18
 * @return  sensor id
 */
 unsigned long cam_get_sensor_id(void);


/** 
 * @brief get sensor fps
 * @author kjp
 * @date 2017-01-18
 * @return  sensor fps
 */
int cam_get_sensor_fps(void);

/**
 * @brief set switch fps manually or automatically
 * @author ye_guohong   
 * @date 
 * @param[fps] >0: manual fps; <=0: auto switch fps
 * @return bool
 * @retval true set successfully
 * @retval false set unsuccessfully
 */
bool cam_set_manual_switch_fps(int fps);

/**
 * @brief get sensor max  resolution
 * @author yang_mengxia   
 * @date 2017-03-07
 * @param height[out] get the height of sensor
 * @param width[out] get the width of sensor
 * @return bool
 * @retval true if  successfully
 * @retval false if  unsuccessfully
 */
bool cam_get_sensor_resolution(int *height, int *width);

/**
 * @brief set value to sensor resgister
 * @author yang_mengxia   
 * @date 2017-5-5
 * @param reg[in] the resgister addr of sensor
 * @param data[in] the value for set to param-reg
 * @return bool
 * @retval true if  successfully
 * @retval false if  unsuccessfully
 */
bool cam_sensor_write_reg(int reg, int data);

/**
 * @brief set value to sensor resgister
 * @author yang_mengxia   
 * @date 2017-5-8
 * @param reg[in] the resgister addr of sensor
 * @return the value for set to param-reg
 */
int cam_sensor_read_reg(int reg);
/*@}*/
#endif
