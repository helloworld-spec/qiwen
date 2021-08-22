/** 
 * @file hal_camera.c
 * @brief provide interfaces for high layer operation of Camera
 * Copyright (C) 2010 Anyka (Guangzhou) Microelectronics Technology Co., Ltd
 * @author xia_wenting
 * @date 2011-03-30
 * @version 1.0
 */
#include <string.h>
#include "anyka_types.h"
#include "anyka_cpu.h"
#include "drv_api.h"
#include "camera.h"
#include "interrupt.h"
#include "sysctl.h"
#include "hal_probe.h"
#include "drv_module.h"

#include <stdlib.h>
#include <stdarg.h>
#include "ak_isp_drv.h"
#include "ispdrv_modules_interface.h"

#ifndef BURNTOOL

//define camera buffer count
#define CAMERABUFFERCOUNT           4

//define camera message
#define CAMERA_MESSAGE              3

//suspend flag
#define CAMERA_SUSPEND              1
#define CAMERA_NO_SUSPEND           0

typedef enum
{
    BUFFER_EMPTY = 0,
    BUFFER_READY,
    BUFFER_CAPTURE,
    BUFFER_USE
}T_BUFFER_STATUS;

typedef struct 
{
//    T_CAMERA_BUFFER camera_buffer[CAMERABUFFERCOUNT]; 
	T_CAMERA_BUFFER ch1_camera_buffer[CAMERABUFFERCOUNT];
	T_CAMERA_BUFFER ch2_camera_buffer[CAMERABUFFERCOUNT];
    T_fCAMSTREAMCALLBACK DataNotifyCallbackFunc;    
    unsigned long CameraBufferCount; 
    unsigned char CaptureBufferIndex; 
    unsigned char ReadyBufferIndex;
    unsigned char UseBufferIndex;
    unsigned char suspend_flag;
    unsigned char interrupt_flag;
    T_CAMERA_FUNCTION_HANDLER *pCameraHandler; 
    bool bIsStream_change;   //indicate stream has been changed, used for abandoning the first frame
    enum isp_working_mode mode;
}T_HAL_PARAM;

enum CAM_ISP_FPS_STAT {
	LOW_FPS_STAT = 0,
	HIGH_FPS_STAT,
	NOTINIT_FPS_STAT
};

enum CAM_ISP_GAIN_STAT {
	GAIN_NO_CHANGE_STAT = 0,
	GAIN_LOW_STAT,
	GAIN_HIGH_STAT
};

struct cam_isp_fps_conf_info {
	enum CAM_ISP_FPS_STAT fps_stat;
	int cur_fps;
	int low_fps;
	int high_fps;
	int low_fps_exp_time;
	int high_fps_exp_time;
	int high_fps_to_low_fps_gain;
	int low_fps_to_high_fps_gain;
	int is_init;
};

struct cam_isp_fps_attr {
	unsigned int  init_frame_rate;             //????                -->a_gain
	unsigned int  init_frame_rate_exp_time;    //???????????  -->d_gain
	unsigned int  init_frame_rate_gain;        //????????        -->isp_d_gain
	unsigned int  reduce_frame_rate;           //????                -->exp_time
	unsigned int  reduce_frame_exp_time;       //???????????  -->a_gain_en
	unsigned int  reduce_frame_gain;           //????                -->d_gain_en
	unsigned int  other;                       //????               -->isp_d_gain_en
};

struct cam_isp_print_stat_info {
	char enable_print_awb_stat;
	char enable_print_ae_stat;
	char enable_print_af_stat;
	char enable_print_3dnr_stat;
};

struct cam_isp_data {
	struct cam_isp_fps_conf_info fps_conf_info;	//֡???л???Ϣ
	struct cam_isp_print_stat_info print_stat_info;
};

struct manual_switch_fps_attr {
	int dest_fps;
	int cur_fps;
};

static struct manual_switch_fps_attr manual_fps_attr;

static struct cam_isp_data isp_data;

static volatile T_HAL_PARAM m_hal_param = {0};

static unsigned long sensor_id = 0;

static int sensor_fps;

static char  is_auto_switch_fps = 1;

static void cam_start(void);
static void cam_interrupt_callback(void);
static void cam_message_callback(unsigned long *param, unsigned long len);
static bool cam_isp_set_fps_info(void);
static bool cam_isp_proc(void);

/**
 * @brief initialize the parameters of camera, should be done after reset and open camera to initialize   
 * @author xia_wenting
 * @date 2010-12-06
 * @return bool
 * @retval true if successed
 * @retval false if failed
 */
 
static bool cam_init(void)
{
    int i = 0;
    bool camera_open_flag = false;

  	sensor_id  = 0;
	sensor_fps = 0;
    m_hal_param.pCameraHandler = cam_probe();

    if (m_hal_param.pCameraHandler != NULL) {

		sensor_id = m_hal_param.pCameraHandler->sensor_id;
		//cam_init_ispdrv(m_hal_param.pCameraHandler->isp_sensor_cb);
		sensor_fps= m_hal_param.pCameraHandler->cam_get_framerate_func();//default fps
		camera_open_flag = camctrl_init_ispdrv(m_hal_param.pCameraHandler->isp_sensor_cb);
    }

    return camera_open_flag;
}

/**
 * @brief open camera, should be done the after reset camera to  initialize 
 * @author xia_wenting  
 * @date 2010-12-06
 * @return bool
 * @retval true if successed
 * @retval false if failed
 */    
bool cam_open(void)
{    
	is_auto_switch_fps = 1;
	manual_fps_attr.cur_fps = 0;
	manual_fps_attr.dest_fps = 0;

    camctrl_enable();

    if (cam_init() != false)
    {
        return true;
    }
    else
    {
        camctrl_disable();
        akprintf(C2, M_DRVSYS, "open camera failed\n");
        return false;
    }
}

/**
 * @brief close camera 
 * @author xia_wenting  
 * @date 2010-12-06
 * @return void
 */
void cam_close(void)
{    
	camctrl_deinit_ispdrv();
	sensor_id  = 0;
	sensor_fps = 0;
    if ((m_hal_param.pCameraHandler != NULL) && (m_hal_param.pCameraHandler->cam_close_func != NULL))
    {
        m_hal_param.pCameraHandler->cam_close_func();
        m_hal_param.pCameraHandler = NULL;
    }
    camctrl_disable();
}

/** 
 * @brief get sensor ID
 * @author kjp
 * @date 2017-01-18
 * @return  sensor id
 */
 unsigned long cam_get_sensor_id(void)
{
	return sensor_id;
}


/** 
 * @brief get sensor fps
 * @author kjp
 * @date 2017-01-18
 * @return  sensor fps
 */
int cam_get_sensor_fps(void)
{
	return sensor_fps;
}


/**
 * @brief set sensor output video framerate
 * @author ye_guohong   
 * @date 2016-10-19
 * @param[in] fps sensor output framerate
 * @return bool
 * @retval true if sucessed
 * @retval false if failed
 */
bool cam_set_framerate(int fps)
{
	int ret;
	
	if (!m_hal_param.pCameraHandler)
		return false;

	camctrl_pause_isp();
	ret = m_hal_param.pCameraHandler->cam_set_framerate_func(fps);
	if (!ret) {
		camctrl_resume_isp();
		sensor_fps = m_hal_param.pCameraHandler->cam_get_framerate_func();
		return true;
	}
	camctrl_resume_isp();
	printf("[%s] set fps:%d fail\n", __func__, fps);
	return false;
}

/** 
 * @brief set notify callback function
 * @author yi_ruoxiang
 * @date 2010-12-01
 * @param[in] callback_func callback function
 * @return void
*/
void camstream_set_callback(T_fCAMSTREAMCALLBACK callback_func)
{
    m_hal_param.DataNotifyCallbackFunc = callback_func;
}

/** 
 * @brief change camera configure
 * @author xia_wenting
 * @date 2011-03-30
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
                       T_CAMERA_BUFFER *YUV1, T_CAMERA_BUFFER *YUV2, T_CAMERA_BUFFER *YUV3)
{
    return false;
}

/**
 * @brief start isp to capture video
 * @author ye_guohong   
 * @date 2016-10-19
 * @param
 * @return void
 */
static void cam_start(void)
{
#if 0
    if (CAMERA_NO_SUSPEND == m_hal_param.suspend_flag)
    {
        m_hal_param.camera_buffer[m_hal_param.CaptureBufferIndex].status = BUFFER_CAPTURE;

        camctrl_capture_frame(m_hal_param.camera_buffer[m_hal_param.CaptureBufferIndex].dY,
                          m_hal_param.camera_buffer[m_hal_param.CaptureBufferIndex].dU,
                          m_hal_param.camera_buffer[m_hal_param.CaptureBufferIndex].dV);
    }
#endif
	enum sensor_bus_type type;

	type = m_hal_param.pCameraHandler->cam_get_bus_type_func();
	switch (type) {
		case BUS_TYPE_RAW:
		m_hal_param.mode = ISP_RGB_VIDEO_OUT;
		break;

		case BUS_TYPE_YUV:
		m_hal_param.mode = ISP_YUV_VIDEO_OUT;
		break;

		default:
		printf("%s type err\n", __func__);
		break;
	}

	camctrl_start_captuing(m_hal_param.mode);
}

/** 
 * @brief stop camera(interrupt mode)
 * @author yi_ruoxiang
 * @date 2010-12-01
 * @return void
 */
void camstream_stop(void)
{
	unsigned char i;
	
    camstream_suspend();
    
    m_hal_param.DataNotifyCallbackFunc = NULL;

    camctrl_set_interrupt_callback(NULL);  

    DrvModule_Terminate_Task(DRV_MODULE_CAMERA);

	camctrl_stop_captuing();
}

/** 
 * @brief a frame is ready
 * @author yi_ruoxiang
 * @date 2010-12-01
 * @return bool
 * @retval false if no data
 * @retval true if a frame is ready
 */
bool camstream_ready(void)
{
    INTR_DISABLE(IRQ_MASK_CAMERA_BIT);

    if(m_hal_param.ch1_camera_buffer[m_hal_param.ReadyBufferIndex].status == BUFFER_READY)
    {    
        INTR_ENABLE(IRQ_MASK_CAMERA_BIT);
        return true;
    }
    else
    {
        INTR_ENABLE(IRQ_MASK_CAMERA_BIT);
        
        return false;
    }
}

/** 
 * @brief get a frame data
 * @author yi_ruoxiang
 * @date 2010-09-01
 * @return T_CAMERA_BUFFER*
 * @retval pointer of current frame data
 */
T_CAMERA_BUFFER *camstream_get(void)
{
    T_CAMERA_BUFFER *ret = NULL;
	//cam_isp_proc();

    MMU_InvalidateDCache();

    INTR_DISABLE(IRQ_MASK_CAMERA_BIT);

    if (m_hal_param.ch1_camera_buffer[m_hal_param.ReadyBufferIndex].status \
			!= BUFFER_READY)
    {
        INTR_ENABLE(IRQ_MASK_CAMERA_BIT);
        printf("buffer is not ready!");
        return NULL;
    }
      
    ret = (T_CAMERA_BUFFER*)&m_hal_param.ch1_camera_buffer[m_hal_param.ReadyBufferIndex];
    
    m_hal_param.ch1_camera_buffer[m_hal_param.ReadyBufferIndex].status = BUFFER_USE;
    //m_hal_param.camera_buffer[m_hal_param.UseBufferIndex].status = BUFFER_EMPTY;
    m_hal_param.UseBufferIndex = m_hal_param.ReadyBufferIndex;

    m_hal_param.ReadyBufferIndex = (m_hal_param.ReadyBufferIndex + 1) % m_hal_param.CameraBufferCount; // read poiniter to next buffer

    if ((m_hal_param.ch1_camera_buffer[m_hal_param.CaptureBufferIndex].status == BUFFER_READY) || 
        (m_hal_param.ch1_camera_buffer[m_hal_param.CaptureBufferIndex].status == BUFFER_USE))
    {
        //m_hal_param.CaptureBufferIndex = (m_hal_param.CaptureBufferIndex + 1) % m_hal_param.CameraBufferCount;
        //printf("[CE-1]\n");
        //cam_start();
    }
    
    INTR_ENABLE(IRQ_MASK_CAMERA_BIT);
    
    return ret;
}

/**
 * @brief application give up using video buffer
 * @author ye_guohong   
 * @date 2016-10-19
 * @param[in] the application's buffer object
 * @return void
 */
void camstream_queue_buffer(int id)
{
	INTR_DISABLE(IRQ_MASK_CAMERA_BIT);

	m_hal_param.ch1_camera_buffer[id].status = BUFFER_EMPTY;
	camctrl_enable_buffer(id);
	//printf("%s id:%d,r00:0x%x\n",__func__, buffer->id, REG32(0x20000000));

	INTR_ENABLE(IRQ_MASK_CAMERA_BIT);
}

/**
 * @brief init video captu
 * @author ye_guohong   
 * @date 2016-10-19
 * @param
 * @return bool
 * @retval true if sucessed
 * @retval false if failed
 */
bool camstream_init(struct camstream_info *stream_info)
{
	unsigned long i;
	struct camera_info cam_info;
	
#if 0
    if ((m_hal_param.sensor_output_width > 4096) || (m_hal_param.sensor_output_height > 4096) 
        || (m_hal_param.sensor_output_width < 1) || (m_hal_param.sensor_output_height < 1)
        || (dstWidth > 4096) || (dstHeight > 4096) || (dstWidth < 1) || (dstHeight < 1)
        || ((dstWidth % 4) != 0 || (dstHeight % 2) != 0)
        || ((dstWidth * dstHeight % 128) != 0) //chip constrain
        || (m_hal_param.sensor_output_width < dstWidth) || (m_hal_param.sensor_output_height < dstHeight))
    {
        akprintf(C1, M_DRVSYS, "cam_set_info set param error\n");
        return false;
    } 
#endif
    if (!DrvModule_Create_Task(DRV_MODULE_CAMERA))
    {
        return false;
    }

    if (!DrvModule_Map_Message(DRV_MODULE_CAMERA, CAMERA_MESSAGE, cam_message_callback))
    {
        DrvModule_Terminate_Task(DRV_MODULE_CAMERA);
        return false;
    }

    //memset((unsigned char *)m_hal_param.camera_buffer, 0, sizeof(m_hal_param.camera_buffer));
    m_hal_param.CameraBufferCount = 0;

	for (i = 0; i < stream_info->buffer_num; i++) {
        m_hal_param.ch1_camera_buffer[m_hal_param.CameraBufferCount].dYUV = stream_info->ch1_YUV[i]->dYUV;//(&(ch1_YUV[i]))->dYUV;
        m_hal_param.ch1_camera_buffer[m_hal_param.CameraBufferCount].width = stream_info->ch1_dstWidth;
        m_hal_param.ch1_camera_buffer[m_hal_param.CameraBufferCount].height = stream_info->ch1_dstHeight;
        m_hal_param.ch1_camera_buffer[m_hal_param.CameraBufferCount].status = BUFFER_EMPTY;
		m_hal_param.ch1_camera_buffer[m_hal_param.CameraBufferCount].id= i;
		
        m_hal_param.ch2_camera_buffer[m_hal_param.CameraBufferCount].dYUV = stream_info->ch2_YUV[i]->dYUV;//(&(ch2_YUV[i]))->dYUV;
        m_hal_param.ch2_camera_buffer[m_hal_param.CameraBufferCount].width = stream_info->ch2_dstWidth;
        m_hal_param.ch2_camera_buffer[m_hal_param.CameraBufferCount].height = stream_info->ch2_dstHeight;
        m_hal_param.ch2_camera_buffer[m_hal_param.CameraBufferCount].status = BUFFER_EMPTY;
		m_hal_param.ch2_camera_buffer[m_hal_param.CameraBufferCount].id= i;

        m_hal_param.CameraBufferCount++;
	}
#if 0
    if (m_hal_param.CameraBufferCount < 4)
    {
        akprintf(C1, M_DRVSYS, "error, buffer count is %d, must > 4\n", m_hal_param.CameraBufferCount);
        return false;
    }
#endif
	//akprintf(C1, M_DRVSYS, "buffer count is %d\n", m_hal_param.CameraBufferCount);

    m_hal_param.CaptureBufferIndex = 0;
    m_hal_param.ReadyBufferIndex = 0;
    m_hal_param.UseBufferIndex = 0;
    m_hal_param.suspend_flag = CAMERA_NO_SUSPEND;
    m_hal_param.interrupt_flag = 0;

	cam_isp_set_fps_info();

    camctrl_set_interrupt_callback(cam_interrupt_callback);

	cam_info.cam_width		= m_hal_param.pCameraHandler->cam_width;
	cam_info.cam_height		= m_hal_param.pCameraHandler->cam_height;
	cam_info.ch1_dstWidth	= stream_info->ch1_dstWidth;
	cam_info.ch1_dstHeight	= stream_info->ch1_dstHeight;
	cam_info.ch2_dstWidth	= stream_info->ch2_dstWidth;
	cam_info.ch2_dstHeight	= stream_info->ch2_dstHeight;
	cam_info.ch1_dYUV0		= stream_info->ch1_YUV[0]->dYUV;
	cam_info.ch1_dYUV1		= stream_info->ch1_YUV[1]->dYUV;
	cam_info.ch1_dYUV2		= stream_info->ch1_YUV[2]->dYUV;
	cam_info.ch1_dYUV3		= stream_info->ch1_YUV[3]->dYUV;
	cam_info.ch2_dYUV0		= stream_info->ch2_YUV[0]->dYUV;
	cam_info.ch2_dYUV1		= stream_info->ch2_YUV[1]->dYUV;
	cam_info.ch2_dYUV2		= stream_info->ch2_YUV[2]->dYUV;
	cam_info.ch2_dYUV3		= stream_info->ch2_YUV[3]->dYUV;
	cam_info.crop_left		= stream_info->crop_left;
	cam_info.crop_top		= stream_info->crop_top;
	cam_info.crop_width		= stream_info->crop_width;
	cam_info.crop_height	= stream_info->crop_height;
	camctrl_set_info(&cam_info);

    cam_start();
  
    return true;    
}


//used to register callback function by DrvModule_Map_Message,
//AKOS used only
static void cam_message_callback(unsigned long *param, unsigned long len)
{
	camctrl_isp_works();
	cam_isp_proc();	/*白天、夜间、低照度的自动帧率切换*/

    if (m_hal_param.DataNotifyCallbackFunc != NULL)
        m_hal_param.DataNotifyCallbackFunc();
}

//used to register callback function by camctrl_set_interrupt_callback
//Chip interrupts used only
static void cam_interrupt_callback(void)
{
    unsigned char temp = 0;
	int id;
	signed char video_data_err = 0;
	static int  sensor_test = 0;  // 
	//camctrl_print_awb_stat();

    m_hal_param.interrupt_flag = 1;
    //printf("int r00:0x%x\n", REG32(0x20000000));
	DrvModule_Send_Message(DRV_MODULE_CAMERA, CAMERA_MESSAGE, NULL);
    if (camctrl_check_status())    //accept a good frame
    {
		ak_isp_irq_work();
		
        if(m_hal_param.bIsStream_change)            
        {
        	printf("stream_change\n");
            m_hal_param.bIsStream_change = false; //abandon the first frame
            //cam_start();
            return;            
        }

		if (ak_isp_is_continuous()) {
			id = ak_isp_vo_get_using_frame_buf_id();
			if(sensor_test < 6)
			{
				sensor_test++;
				id &= 0x7f;  // 2017.4.17	k			
			}
			if (id == -1) {
				printf("id:-1\n");
				goto end;
			} else if (id >= 80) {
				//should drop this frame
//				printf("id >= 80\n");
				video_data_err = 1;
			}

			id = id & 0xf;
			if (id != m_hal_param.CaptureBufferIndex) {
				printf("id:%d&%d\n", id, m_hal_param.CaptureBufferIndex);
				goto end;
			}

			temp = (m_hal_param.CaptureBufferIndex + 1 ) % m_hal_param.CameraBufferCount;
			if (m_hal_param.ch1_camera_buffer[temp].status != BUFFER_EMPTY) {
				printf("not enough free buffers\n");
				goto end;
			}
			
			m_hal_param.CaptureBufferIndex = \
				((m_hal_param.CaptureBufferIndex + 1) % m_hal_param.CameraBufferCount);
			
			//printf("id = %d\n",id);
			ak_isp_vo_disable_buffer(id);
			//m_hal_param.CaptureBufferIndex = id;
			m_hal_param.ch1_camera_buffer[id].status = BUFFER_READY;

			if (video_data_err)
				m_hal_param.ch1_camera_buffer[id].ts = 0;
			else
				m_hal_param.ch1_camera_buffer[id].ts = get_tick_count();

			//DrvModule_Send_Message(DRV_MODULE_CAMERA, CAMERA_MESSAGE, NULL);
end:
			ak_isp_vo_clear_irq_status(0xffff);
		} else {
			printf(" single mode\n");
		}

    }
    else                      //accept a error frame
    {
        //akprintf(C3, M_DRVSYS, "Accept a error frame, discard it\n");
        //cam_reset_ctl();              
        //cam_start();
    }
}


/** 
 * @brief suspend camera interface, only accept current frame, other not accept
 * @author yi_ruoxiang
 * @date 2010-09-01
 * @return bool
 * @retval false if suspend failed
 * @retval true if suspend successful
 */
bool camstream_suspend(void)
{
    unsigned long i = 0;
    bool ret = false;
    
    m_hal_param.suspend_flag = CAMERA_SUSPEND;
    m_hal_param.interrupt_flag = 0;

    while (1)
    {
        if (1 == m_hal_param.interrupt_flag)
        {
            m_hal_param.interrupt_flag = 0;

            ret = true;
            break;
        }

        i++;
        mini_delay(1);

        if (i > 500)
        {
            ret = false;
            break;
        }        
    }

    return ret;
}

/** 
 * @brief resume camera interface, start accept frame
 * @author yi_ruoxiang
 * @date 2010-09-01
 * @return void
 */
void camstream_resume(void)
{
    m_hal_param.suspend_flag = CAMERA_NO_SUSPEND;
    
    cam_start();
}





///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/**
 * @brief set fps information
 * @author ye_guohong   
 * @date 2016-10-19
 * @param[]
 * @return bool
 * @retval true set successfully
 * @retval false set unsuccessfully
 */
static bool cam_isp_set_fps_info(void)
{
	bool ret = true;
	AK_ISP_FRAME_RATE_ATTR fr;
	AK_ISP_FRAME_RATE_ATTR *p_fr = &fr;
	struct cam_isp_fps_attr *fps_attr = (struct cam_isp_fps_attr *)p_fr;
	struct cam_isp_fps_conf_info *p_fps_conf_info = &isp_data.fps_conf_info;

	ret = ispdrv_get_module(ISP_EXP_SUB_FRAME_RATE, (unsigned char *)&fr);
	if (ret == false) {
		printf("[%s] get ISP_EXP_SUB_ME fail\n", __func__);
		return ret;
	}

#if 0
	printf("[%s] get fps info: init_frame_rate:%d,init_frame_rate_exp_time=%d,\
			init_frame_rate_gain:%d, reduce_frame_rate=%d,\
			reduce_frame_exp_time=%d, reduce_frame_gain=%d\r\n", \
			__func__, fps_attr->init_frame_rate, fps_attr->init_frame_rate_exp_time,\
			fps_attr->init_frame_rate_gain, fps_attr->reduce_frame_rate,\
			fps_attr->reduce_frame_exp_time, fps_attr->reduce_frame_gain);
#endif

	p_fps_conf_info->fps_stat			= NOTINIT_FPS_STAT;
	p_fps_conf_info->cur_fps			= 0;
	p_fps_conf_info->low_fps			= fps_attr->reduce_frame_rate;
	p_fps_conf_info->high_fps			= fps_attr->init_frame_rate;
	p_fps_conf_info->low_fps_exp_time		= fps_attr->reduce_frame_exp_time;
	p_fps_conf_info->high_fps_exp_time		= fps_attr->init_frame_rate_exp_time;
	p_fps_conf_info->high_fps_to_low_fps_gain	= fps_attr->init_frame_rate_gain;
	p_fps_conf_info->low_fps_to_high_fps_gain	= fps_attr->reduce_frame_gain;
	p_fps_conf_info->is_init			= 1;

	return ret;
}

/**
 * @brief get ae status information
 * @author ye_guohong   
 * @date 2016-10-19
 * @param[out] stat ae status 
 * @return bool
 * @retval true get successfully
 * @retval false get unsuccessfully
 */
static bool cam_isp_get_aestat(AK_ISP_AE_RUN_INFO *stat)
{
	if (ispdrv_get_module(ISP_AESTAT_SUB_AE, (unsigned char *)stat))
		return false;

	return true;
}

/**
 * @brief get all gain change status
 * @author ye_guohong   
 * @date 2016-10-19
 * @param[out] res_gain current all gain
 * @return enum CAM_ISP_GAIN_STAT
 * @retval GAIN_HIGH_STAT if too higher all gain
 * @retval GAIN_LOW_STAT if too lower all gain
 * @retval GAIN_NO_CHANGE_STAT if all gain is no too higher and no too lower
 */
static enum CAM_ISP_GAIN_STAT cam_isp_get_gain_stat(int *res_gain)
{
	int gain;
	enum CAM_ISP_GAIN_STAT stat = GAIN_NO_CHANGE_STAT;
	struct cam_isp_fps_conf_info *p_fps_conf_info = &isp_data.fps_conf_info;
	AK_ISP_AE_RUN_INFO ae_stat;

	if (cam_isp_get_aestat(&ae_stat)) {
		stat = GAIN_NO_CHANGE_STAT;
		goto end;
	}

	//gain = (ae_stat.current_a_gain >> 8) * (ae_stat.current_d_gain >> 8) * (ae_stat.current_isp_d_gain >> 8);
	gain = ((ae_stat.current_a_gain * ae_stat.current_d_gain >> 8) * ae_stat.current_isp_d_gain) >> 8 >> 8;
	//anyka_debug("%s gain:%d, current_a_gain:%d, current_d_gain:%d, current_isp_d_gain:%d\n", __func__, gain, (int)ae_stat.current_a_gain, (int)ae_stat.current_d_gain, (int)ae_stat.current_isp_d_gain);


	if (gain > p_fps_conf_info->high_fps_to_low_fps_gain) {
		stat = GAIN_HIGH_STAT;
	} else if (gain < p_fps_conf_info->low_fps_to_high_fps_gain) {
		stat = GAIN_LOW_STAT;
	} else {
		stat = GAIN_NO_CHANGE_STAT;
	}

end:
	//printf("%s gain:%d stat:%d\n", __func__, gain, stat);
	*res_gain = gain;
	return stat;
}

/**
 * @brief change max exposal time
 * @author ye_guohong   
 * @date 2016-10-19
 * @param[in] new_max_exp new max exposal time
 * @return bool
 * @retval true change successfully
 * @retval false change unsuccessfully
 */
static bool hal_change_max_exp(int new_max_exp)
{
	AK_ISP_AE_ATTR ae;

	if (false == ispdrv_get_module(ISP_EXP_SUB_AE, (unsigned char *)&ae))
		return false;

	ae.exp_time_max = new_max_exp;
	if (false == ispdrv_set_module(ISP_EXP_SUB_AE, (unsigned char *)&ae))
		return false;

	//printf("[%s] set new_max_exp:%d ok\n", __func__, new_max_exp);
	return true;
}

static int hal_get_max_exp(int *exp)
{
	AK_ISP_AE_ATTR ae;

	if (false == ispdrv_get_module(ISP_EXP_SUB_AE, (unsigned char *)&ae))
		return -1;

	*exp = ae.exp_time_max;
	return 0;
}

static int cam_change_fps_exp(int old_fps, int new_fps)
{
	int ret = -1;
	int exp_time;
	int old_exp_time = 0;
	struct cam_isp_fps_conf_info *p_fps_conf_info = &isp_data.fps_conf_info;

	if (!p_fps_conf_info->is_init) {
		printf("%s not init\n", __func__);
		return -1;
	}

	if (new_fps <= 0) {
		printf("%s new_fps:%d err\n", __func__, new_fps);
		return -1;
	}

	if (new_fps == p_fps_conf_info->high_fps)
		exp_time = p_fps_conf_info->high_fps_exp_time;
	else if (new_fps == p_fps_conf_info->low_fps)
		exp_time = p_fps_conf_info->low_fps_exp_time;
	else
		exp_time = p_fps_conf_info->high_fps * p_fps_conf_info->high_fps_exp_time / new_fps;

	hal_get_max_exp(&old_exp_time);

	if (old_fps > new_fps) {
		if (true == cam_set_framerate(new_fps)) {
			hal_change_max_exp(exp_time);
			ret = 0;
		}
	} else {
		hal_change_max_exp(exp_time);
		if (true == cam_set_framerate(new_fps)) {
			ret = 0;
		} else {
			hal_change_max_exp(old_exp_time);
		}
	}

	if (!ret) {
		printf("%s set new fps:%d exp:%d ok\n", __func__, new_fps, exp_time);
	} else {
		printf("%s set new fps:%d exp:%d fail\n", __func__, new_fps, exp_time);
	}

	return ret;
}

/**
 * @brief change sensor output framerate
 * @author ye_guohong   
 * @date 2016-10-19
 * @param[in] set_fps_func_ptr set sensor framerate function
 * @return bool
 * @retval true change successfully
 * @retval false change unsuccessfully
 */
static bool cam_change_fps_auto(void)
{
	int gain;
	int new_fps;
	enum CAM_ISP_GAIN_STAT stat;
	struct cam_isp_fps_conf_info *p_fps_conf_info = &isp_data.fps_conf_info;

	if (!p_fps_conf_info->is_init) {
		printf("[%s] fps_conf_info is not init!!\n", __func__);
		return false;
	}

	stat = cam_isp_get_gain_stat(&gain);
	switch (stat) {
		case GAIN_NO_CHANGE_STAT:
		goto no_change;
		break;

		case GAIN_LOW_STAT:
		new_fps = p_fps_conf_info->high_fps;
		break;

		case GAIN_HIGH_STAT:
		new_fps = p_fps_conf_info->low_fps;
		break;

		default:
		break;
	}

	if (new_fps != p_fps_conf_info->cur_fps) {
		if (0 == cam_change_fps_exp(p_fps_conf_info->cur_fps, new_fps))
			p_fps_conf_info->cur_fps = new_fps;
		else
			return false;
	}

no_change:
	return true;
}

/**
 * @brief change sensor output framerate & isp exposal time
 * @author ye_guohong   
 * @date 2016-10-19
 * @param[]
 * @return bool
 * @retval true change successfully
 * @retval false change unsuccessfully
 */
static bool cam_change_fps(void)
{
	static unsigned long old_tick = 0;
	const int check_time = 2;//seconds
	unsigned long new_tick;
	
	if (is_auto_switch_fps) {
		new_tick = get_tick_count();
		if ((new_tick < old_tick) || \
				((new_tick - old_tick) >= check_time * 1000)) {
			old_tick = new_tick;
			cam_change_fps_auto();
		}
	} else {
		int dest_fps = manual_fps_attr.dest_fps;
		struct cam_isp_fps_conf_info *p_fps_conf_info = &isp_data.fps_conf_info;

		if ( dest_fps != manual_fps_attr.cur_fps) {
			if (0 == cam_change_fps_exp(manual_fps_attr.cur_fps, dest_fps)) {
				manual_fps_attr.cur_fps = dest_fps;
				p_fps_conf_info->cur_fps = dest_fps;
			}
		}
	}
}

/**
 * @brief set switch fps manually or automatically
 * @author ye_guohong   
 * @date 
 * @param[fps] >0: manual fps; <0: auto switch fps; ==0: disable auto switch fps.
 * @return bool
 * @retval true set successfully
 * @retval false set unsuccessfully
 */
bool cam_set_manual_switch_fps(int fps)
{
	if (fps < 0) {
		is_auto_switch_fps = 1;
		manual_fps_attr.dest_fps = 0;
		manual_fps_attr.cur_fps = 0;
	} else {
		manual_fps_attr.dest_fps = fps;
		manual_fps_attr.cur_fps = 0;
		is_auto_switch_fps = 0;
	}

	return true;
}

/**
 * @brief print current awb status
 * @author ye_guohong   
 * @date 2016-10-19
 * @param[]
 * @return bool
 * @retval true print successfully
 * @retval false print unsuccessfully
 */
static bool cam_isp_print_awb_stat(void)
{
	int i;
	AK_ISP_AWB_STAT_INFO awb_stat_info;

	if (false == ispdrv_get_module(ISP_AWBSTAT, (unsigned char *)&awb_stat_info)) {
		printf("[%s] get awb_stat_info failed\n", __func__);
		return false;
	}

	printf("\n");
	printf("RTotal,\tGTotal,\tBTotal,\tStable_cnt\n");
	for (i = 0; i < 10; i++) {
		printf("%d:%lu,\t%lu,\t%lu,\t%lu\n", i,\
			awb_stat_info.total_R[i],awb_stat_info.total_G[i],\
			awb_stat_info.total_B[i],awb_stat_info.total_cnt[i]);
	}

	printf("RGain:%u,GGain:%u,BGain:%u\n",awb_stat_info.r_gain,\
		awb_stat_info.g_gain,awb_stat_info.b_gain);
	printf("ROff:%d,GOff:%d,BOff:%d\n",awb_stat_info.r_offset,\
		awb_stat_info.g_offset,awb_stat_info.b_offset);
	printf("colortemp index:%u\n",awb_stat_info.current_colortemp_index);
	return true;
}

/**
 * @brief print current awb status intermittently
 * @author ye_guohong   
 * @date 2016-10-19
 * @param[]
 * @return bool
 * @retval true print successfully
 * @retval false print unsuccessfully
 */
static bool cam_print_awb_stat(void)
{
	static unsigned long old_tick = 0;
	const int check_time = 1;//seconds
	unsigned long new_tick;

	if (!isp_data.print_stat_info.enable_print_awb_stat)
		return false;

	new_tick = get_tick_count();
	if ((new_tick < old_tick) || \
		((new_tick - old_tick) >= check_time * 1000)) {
		old_tick = new_tick;
		cam_isp_print_awb_stat();
	}

	return true;
}

/**
 * @brief print current ae status
 * @author ye_guohong   
 * @date 2016-10-19
 * @param[]
 * @return bool
 * @retval true print successfully
 * @retval false print unsuccessfully
 */
static bool cam_isp_print_ae_stat(void)
{
	AK_ISP_AE_RUN_INFO ae_run_info;

	if (false == ispdrv_get_module(ISP_AESTAT_SUB_AE, (unsigned char *)&ae_run_info)) {
		printf("[%s] get ae_run_info failed\n", __func__);
		return false;
	}

	printf("\n");
	printf("AvgLumi:%d\tExpTime:%ld\nComLumi:%d\tAGainStep:%lu\nAGain:%ld\tDGainStep:%lu\n\
DGain:%ld\tIspDGainStep:%lu\nIspDGain:%ld\tExpStep:%lu\nDarkDayFlag:%d\n",\
		ae_run_info.current_calc_avg_lumi, ae_run_info.current_exp_time,\
		ae_run_info.current_calc_avg_compensation_lumi, ae_run_info.current_a_gain_step,\
		ae_run_info.current_a_gain, ae_run_info.current_d_gain_step,\
		ae_run_info.current_d_gain, ae_run_info.current_isp_d_gain_step,\
		ae_run_info.current_isp_d_gain, ae_run_info.current_exp_time_step,\
		ae_run_info.current_darked_flag);

	return true;
}

/**
 * @brief print current ae status intermittently
 * @author ye_guohong   
 * @date 2016-10-19
 * @param[]
 * @return bool
 * @retval true print successfully
 * @retval false print unsuccessfully
 */
static bool cam_print_ae_stat(void)
{
	static unsigned long old_tick = 0;
	const int check_time = 1;//seconds
	unsigned long new_tick;

	if (!isp_data.print_stat_info.enable_print_ae_stat)
		return false;

	new_tick = get_tick_count();
	if ((new_tick < old_tick) || \
		((new_tick - old_tick) >= check_time * 1000)) {
		old_tick = new_tick;
		cam_isp_print_ae_stat();
	}

	return true;
}

/**
 * @brief print current af status
 * @author ye_guohong   
 * @date 2016-10-19
 * @param[]
 * @return bool
 * @retval true print successfully
 * @retval false print unsuccessfully
 */
static bool cam_isp_print_af_stat(void)
{
	AK_ISP_AF_STAT_INFO af_stat;

	if (false == ispdrv_get_module(ISP_AFSTAT, (unsigned char *)&af_stat)) {
		printf("[%s] get af_stat failed\n", __func__);
		return false;
	}

	printf("\n");
	printf("AFStat:%u\n", (unsigned int)af_stat.af_statics);

	return true;
}

/**
 * @brief print current af status intermittently
 * @author ye_guohong   
 * @date 2016-10-19
 * @param[]
 * @return bool
 * @retval true print successfully
 * @retval false print unsuccessfully
 */
static bool cam_print_af_stat(void)
{
	static unsigned long old_tick = 0;
	const int check_time = 1;//seconds
	unsigned long new_tick;

	if (!isp_data.print_stat_info.enable_print_af_stat)
		return false;

	new_tick = get_tick_count();
	if ((new_tick < old_tick) || \
		((new_tick - old_tick) >= check_time * 1000)) {
		old_tick = new_tick;
		cam_isp_print_af_stat();
	}

	return true;
}

/**
 * @brief print current 3DNR status
 * @author ye_guohong   
 * @date 2016-10-19
 * @param[]
 * @return bool
 * @retval true print successfully
 * @retval false print unsuccessfully
 */
static bool cam_isp_print_3dnr_stat(void)
{
	int i, j;
	AK_ISP_3D_NR_STAT_INFO _3dnr_stat;

	if (false == ispdrv_get_module(ISP_3DSTAT, (unsigned char *)&_3dnr_stat)) {
		printf("[%s] get _3dnr_stat failed\n", __func__);
		return false;
	}
#if 0
	printf("\n");
	printf("YStat:%lu\tUvStat:%lu\tCnt:%lu\n",\
		_3dnr_stat.deltaY_stat, _3dnr_stat.deltaUV_stat, _3dnr_stat.MD_count);
	for (i = 0; i < 8; i++) {
		for (j = 0; j < 16; j++) {
			printf("%d\t", _3dnr_stat.MD_stat[i][j]);
		}
		printf("\n");
		if (i == 3)
			printf("\n");
	}
#endif
	return true;
}

/**
 * @brief print current 3DNR status intermittently
 * @author ye_guohong   
 * @date 2016-10-19
 * @param[]
 * @return bool
 * @retval true print successfully
 * @retval false print unsuccessfully
 */
static bool cam_print_3dnr_stat(void)
{
	static unsigned long old_tick = 0;
	const int check_time = 1;//seconds
	unsigned long new_tick;

	if (!isp_data.print_stat_info.enable_print_3dnr_stat)
		return false;

	new_tick = get_tick_count();
	if ((new_tick < old_tick) || \
		((new_tick - old_tick) >= check_time * 1000)) {
		old_tick = new_tick;
		cam_isp_print_3dnr_stat();
	}

	return true;
}

/**
 * @brief print NR1/NR2
 * @author ye_guohong   
 * @date 2016-10-19
 * @param[]
 * @return bool
 * @retval true print successfully
 * @retval false print unsuccessfully
 */
static bool cam_isp_print_denoise(void)
{
	int i, j;
	AK_ISP_NR1_ATTR nr1;
	AK_ISP_NR2_ATTR nr2;

	if (false == ispdrv_get_module(ISP_NR_SUB_NR1, (unsigned char *)&nr1)) {
		printf("[%s] get nr1 failed\n", __func__);
		return false;
	}

	if (false == ispdrv_get_module(ISP_NR_SUB_NR2, (unsigned char *)&nr2)) {
		printf("[%s] get nr2 failed\n", __func__);
		return false;
	}
	
	printf("\n");
	printf("0x070500, %d\n", nr1.nr1_mode);
	printf("0x070501, %d\n", nr2.nr2_mode);
	printf("0x070502, %d\n", nr1.manual_nr1.nr1_enable);
	printf("0x070503, %d\n", nr2.manual_nr2.nr2_enable);
	for (i = 0; i < 17; i++)
		printf("0x%06x, %d\n", i + 0x070000, nr1.manual_nr1.nr1_weight_rtbl[i]);
	printf("0x070011, %d\n", nr1.manual_nr1.nr1_calc_r_k);
	for (i = 0; i < 17; i++)
		printf("0x%06x, %d\n", i + 0x070100, nr1.manual_nr1.nr1_weight_gtbl[i]);
	printf("0x070111, %d\n", nr1.manual_nr1.nr1_calc_g_k);
	for (i = 0; i < 17; i++)
		printf("0x%06x, %d\n", i + 0x070200, nr1.manual_nr1.nr1_weight_btbl[i]);
	printf("0x070211, %d\n", nr1.manual_nr1.nr1_calc_b_k);

	printf("0x070212, %d\n", nr1.manual_nr1.nr1_k);
	for (i = 0; i < 17; i++)
		printf("0x%06x, %d\n", i + 0x070300, nr1.manual_nr1.nr1_lc_lut[i]);

	for (i = 0; i < 17; i++)
		printf("0x%06x, %d\n", i + 0x070400, nr2.manual_nr2.nr2_weight_tbl[i]);
	printf("0x070411, %d\n", nr2.manual_nr2.nr2_calc_y_k);
	printf("0x070412, %d\n", nr2.manual_nr2.nr2_k);

	for (i = 0; i < 9; i++) {
		printf("0x%06x, %d\n", i * 0x1000 + 0x071501, nr1.linkage_nr1[i].nr1_enable);
		printf("0x%06x, %d\n", i * 0x1000 + 0x071502, nr2.linkage_nr2[i].nr2_enable);
		for (j = 0; j < 17; j++) {
			printf("0x%06x, %d\n", i * 0x1000 + j + 0x071000, nr1.linkage_nr1[i].nr1_weight_rtbl[j]);
		}
		printf("0x%06x, %d\n", i * 0x1000 + j + 0x071000, nr1.linkage_nr1[i].nr1_calc_r_k);
		
		for (j = 0; j < 17; j++) {
			printf("0x%06x, %d\n", i * 0x1000 + j + 0x071100, nr1.linkage_nr1[i].nr1_weight_gtbl[j]);
		}
		printf("0x%06x, %d\n", i * 0x1000 + j + 0x071100, nr1.linkage_nr1[i].nr1_calc_g_k);

		for (j = 0; j < 17; j++) {
			printf("0x%06x, %d\n", i * 0x1000 + j + 0x071200, nr1.linkage_nr1[i].nr1_weight_btbl[j]);
		}
		printf("0x%06x, %d\n", i * 0x1000 + j + 0x071200, nr1.linkage_nr1[i].nr1_calc_b_k);

		j++;
		printf("0x%06x, %d\n", i * 0x1000 + j + 0x071200, nr1.linkage_nr1[i].nr1_k);

		for (j = 0; j < 17; j++) {
			printf("0x%06x, %d\n", i * 0x1000 + j + 0x071300, nr1.linkage_nr1[i].nr1_lc_lut[j]);
		}

		for (j = 0; j < 17; j++) {
			printf("0x%06x, %d\n", i * 0x1000 + j + 0x071400, nr2.linkage_nr2[i].nr2_weight_tbl[j]);
		}
		printf("0x%06x, %d\n", i * 0x1000 + j + 0x071400, nr2.linkage_nr2[i].nr2_calc_y_k);
		j++;
		printf("0x%06x, %d\n", i * 0x1000 + j + 0x071400, nr2.linkage_nr2[i].nr2_k);
	}
	
	return true;
}

/**
 * @brief print NR1/NR2 intermittently
 * @author ye_guohong   
 * @date 2016-10-19
 * @param[]
 * @return bool
 * @retval true print successfully
 * @retval false print unsuccessfully
 */
static bool cam_print_denoise(void)
{
	static unsigned long old_tick = 0;
	const int check_time = 1;//seconds
	unsigned long new_tick;
	static int total_cnt = 1;

	if (!total_cnt)
		return false;
	
	new_tick = get_tick_count();
	if ((new_tick < old_tick) || \
		((new_tick - old_tick) >= check_time * 1000)) {
		old_tick = new_tick;
		cam_isp_print_denoise();
		total_cnt--;
	}

	return true;
}

/**
 * @brief isp some process
 * @author ye_guohong   
 * @date 2016-10-19
 * @param[]
 * @return bool
 * @retval true if process successfully
 * @retval false if process unsuccessfully
 */
static bool cam_isp_proc(void)
{
	cam_change_fps();

	cam_print_awb_stat();
	cam_print_ae_stat();
	cam_print_af_stat();
	cam_print_3dnr_stat();
	//cam_print_denoise();

	return true;
}

/**
 * @brief set print isp status 
 * @author ye_guohong   
 * @date 2016-10-19
 * @param[]
 * @return bool
 * @retval true if  successfully
 * @retval false if  unsuccessfully
 */
bool cam_set_print_isp_stat(int en_awb_stat, int en_ae_stat,
			int en_af_stat, int en_3dnr_stat)
{
	isp_data.print_stat_info.enable_print_awb_stat = en_awb_stat;
	isp_data.print_stat_info.enable_print_ae_stat = en_ae_stat;
	isp_data.print_stat_info.enable_print_af_stat = en_af_stat;
	isp_data.print_stat_info.enable_print_3dnr_stat = en_3dnr_stat;

	return true;
}

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
bool cam_get_sensor_resolution(int *height, int *width)
{
	*height = m_hal_param.pCameraHandler->cam_height;
	*width = m_hal_param.pCameraHandler->cam_width;
	return true;
}

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
bool cam_sensor_write_reg(int reg, int data)
{
	if( true != m_hal_param.pCameraHandler->isp_sensor_cb->sensor_write_reg_func(reg, data))
		return false;

	return true;
}

/**
 * @brief set value to sensor resgister
 * @author yang_mengxia   
 * @date 2017-5-8
 * @param reg[in] the resgister addr of sensor
 * @return the value for set to param-reg
 */
int cam_sensor_read_reg(int reg)
{
	return m_hal_param.pCameraHandler->isp_sensor_cb->sensor_read_reg_func(reg);
}
/**
 * @brief 
 * @author 
 * @date 2017-03-07
 * @param height[out] get the height of sensor
 * @param width[out] get the width of sensor
 * @return bool
 * @retval true if  successfully
 * @retval false if  unsuccessfully
 */
void cam_set_sensor_ae_info(void *ae_info)
{
	AK_ISP_AE_INFO isp_ae_info;

	isp_ae_info.a_gain      = 256;
	isp_ae_info.d_gain      = 256;
	isp_ae_info.isp_d_gain  = 256;
	isp_ae_info.exp_time    = *(unsigned short *)ae_info;

	ak_isp_ae_info(&isp_ae_info);

}

#endif

