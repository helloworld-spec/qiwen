/**
 * @file drv_camera.h
 * @brief provide interfaces for low layer operation of Camera
 * Copyright (C) 2010 Anyka (Guangzhou) Microelectronics Technology Co., Ltd
 * @author xia_wenting
 * @date 2010-12-07
 * @version 1.0
 */

#ifndef __DRV_CAMERA_H__
#define __DRV_CAMERA_H__

#include "hal_camera.h"
#include "ak_isp_drv.h"

#ifndef BURNTOOL

/** @defgroup Drv_camera Hardware Abstract Layer of camera
 *  @ingroup Camera
 */
/*@{*/


typedef struct
{
	AK_ISP_SENSOR_CB 	*isp_sensor_cb;
	unsigned long		sensor_id;
	unsigned long		cam_width;
	unsigned long		cam_height;
    unsigned long		cam_mclk;
    void				(*cam_open_func)(void);
    bool				(*cam_close_func)(void);
    int		            (*cam_read_id_func)(void);
    bool				(*cam_init_func)(void);
    int					(*cam_set_framerate_func)(int framerate);
    int					(*cam_get_framerate_func)(void);
	enum sensor_bus_type	(*cam_get_bus_type_func)(void);
}T_CAMERA_FUNCTION_HANDLER;

typedef struct
{
    unsigned long DeviceID;
    T_CAMERA_FUNCTION_HANDLER *handler;
}T_CAMERA_INFO;

#define CAMERA_MAX_SUPPORT        20


/** 
 * @brief register a camera device, which will be probed at camera open
 * @author xia_wenting
 * @date 2010-12-07
 * @param[in] id  the camera chip id
 * @param[in] handler camera interface
 * @return bool
 * @retval true register successfully
 * @retval false register unsuccessfully
 */
bool camera_reg_dev(unsigned long id, T_CAMERA_FUNCTION_HANDLER *handler);

#endif
/*@}*/
#endif //__DRV_CAMERA_H__
