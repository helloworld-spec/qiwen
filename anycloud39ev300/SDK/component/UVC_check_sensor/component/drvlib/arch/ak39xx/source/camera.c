/**
 * @file camera.c
 * @brief camera function file
 * This file provides camera APIs: open, capture photo
 * Copyright (C) 2010 Anyka (Guangzhou) Microelectronics Technology Co., Ltd
 * @author xia_wenting 
 * @date 2011-03-30
 * @version 1.0
 * @note ref AK37xx technical manual.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "anyka_cpu.h"
#include "anyka_types.h"
#include "drv_api.h"
#include "camera.h"
#include "hal_probe.h"
#include "interrupt.h"
#include "sysctl.h"
#include "drv_gpio.h"
#include "ispdrv_modules_interface.h"


#ifndef BURNTOOL

#define CLOCK_GATE_CTRL1		(0x0800001C)

#define AK_SHAREPIN_CON2		(0x08000078)
#define AK_SRESET_CAMERA		(19)

#undef	MHz
#define	MHz					(1000000)


static T_fCamera_Interrupt_CALLBACK m_CameraInterruptCallback = NULL;

static bool camctrl_interrupt_handler(void);

 /**
  * @brief reset camera 
  * @author xia_wenting  
  * @date 2010-12-06
  * @return void
  */
void camctrl_enable(void)
{
	sysctl_clock(CLOCK_CAMERA_ENABLE);
	/*
	注释下面这句的原因: 因为camera 的data0~data12在系统上电时就是这个配置，
	所以为了兼顾部分data引脚用作gpio的情况才注释掉。
	*/
	//gpio_pin_group_cfg(ePIN_AS_CAMERA);
	REG32(AK_SHAREPIN_CON2) &= ~(0xf);
	
	//reset camera interface
	sysctl_reset(AK_SRESET_CAMERA);
	
	//enbale PLL2
	//camctrl_open(24); 	  
}

 /**
 * @brief open camera, should be done the after reset camera to initialize 
 * @author xia_wenting  
 * @date 2010-12-06
 * @param[in] mclk send to camera mclk 
 * @return bool
 * @retval true if successed
 * @retval false if failed
 */
bool camctrl_open(unsigned long mclk)
{
	unsigned long regval;
	unsigned long mclk_div;

	unsigned long peri_pll = get_peri_pll()/1000000;

	//printf("peri_pll:%ld\n",peri_pll);
	//set mclk, the sensor present working 24MHz
	mclk_div = peri_pll/mclk - 1;
	
	regval = REG32(PERI_CLOCK_DIV_REG_2);
	regval &= ~(0x3f << 10);
	regval |= (mclk_div << 10);
	REG32(PERI_CLOCK_DIV_REG_2) = (1 << 19)|(1 << 18)|regval;	//not the same as PG

	// enable isp clock, pclk
	REG32(CLOCK_GATE_CTRL1) &= ~(1 << 19);

	REG32(PERI_CLOCK_DIV_REG_1) &= ~(1 << 25);

#if 0
	//alloc memory
	if (!isp.ispdma_addr)
	{
		isp.bytes = 1024;
	    isp.ispdma_addr = drv_malloc(isp.bytes);
	    if(NULL == isp.ispdma_addr)
	    {
	        akprintf(C1, M_DRVSYS, "malloc fail in isp dma\r\n");
	        return false;
	    }
		memset(isp.ispdma_addr, 0, isp.bytes);
	}

	init_isp_mode(&isp);
#endif
	return true;
}

/**
 * @brief close camera 
 * @author xia_wenting  
 * @date 2010-12-06
 * @return void
 */
void camctrl_disable(void)
{
	akprintf(C3, M_DRVSYS, "...camctrl_disable...\n");
		
    INTR_DISABLE(IRQ_MASK_CAMERA_BIT);
#if 0    
    //disable PLL2    
    REG32(MUL_FUN_CTL_REG) |= (1UL << 31); 
    mini_delay(1);
    if (lcd_tvout_clk_get() == TVOUT_CLK_EXTERNAL)
        REG32(MUL_FUN_CTL_REG) |= (1 << 30);
    mini_delay(1);
#endif        
    sysctl_clock(~CLOCK_CAMERA_ENABLE);
#if 0

	if (isp.ispdma_addr) {
		drv_free(isp.ispdma_addr);
	}
#endif
}

/**
 * @brief set interrupt callback function
 * @author xia_wenting  
 * @date 2010-12-01
 * @param[in] callback_func callbak function
 * @return 
 * @retval 
 */
void camctrl_set_interrupt_callback(T_fCamera_Interrupt_CALLBACK callback_func)
{
    m_CameraInterruptCallback = callback_func;
    if (callback_func)
    {
        int_register_irq(INT_VECTOR_CAMERA, camctrl_interrupt_handler);
    }
    else
    {
        INTR_DISABLE(IRQ_MASK_CAMERA_BIT);
    }

}

/**
 * @brief libispdrv's callball for print message
 * @author ye_guohong
 * @date 2016-10-19
 * @param[in] fmt message for output
 * @return 
 * @retval 
 */
static void camctrl_cb_printf (char * fmt, ... )
{
    va_list args;
    int n;
    int i;
    int slen;
    
    char printbuffer[1024];
    va_start ( args, fmt );
    n = vsprintf ( printbuffer, fmt, args );
    va_end ( args );        

    slen = strlen(printbuffer);
    for(i = 0; i < slen; i++)
    {
        putch(printbuffer[i]);
    }
}

/**
 * @brief libispdrv's callball for dma memory malloc
 * @author ye_guohong
 * @date 2016-10-19
 * @param[in] sz size to malloc
 * @param[out] handle physical address for dma
 * @return void * virtual dma address
 * @retval NULL if malloc dma memory failed
 * @retval others if malloc dma memory successed
 */
static void *camctrl_cb_dmamalloc(unsigned long sz, void *handle)
{
	void *p;

	p = (void *)drv_malloc(sz);
	if (NULL == p) {
		printf("%s malloc fail\n", __func__);
		return NULL;
	}

	memset(p, 0, sz);
	*((unsigned long *)handle) = (unsigned long)p;
	return p;
}

/**
 * @brief libispdrv's callball for dma memory free
 * @author ye_guohong
 * @date 2016-10-19
 * @param[in] ptr virtual dma address
 * @param[in] sz size to malloc
 * @param[in] handle physical address for dma
 * @return void
 */
static void camctrl_cb_dmafree(void *ptr, unsigned long sz, unsigned long handle)
{
	drv_free((void *)ptr);
}

/**
 * @brief libispdrv's callball for micro second delay
 * @author ye_guohong
 * @date 2016-10-19
 * @param[in] ms micro senconds for delay
 * @return void
 */
static void camctrl_cb_msleep(int ms)
{
	int ticks;
	
	ticks = ms / 5;
	if (!ticks)
		ticks = 1;
	AK_Sleep(ticks);
}

/**
 * @brief libispdrv's callball for normal memory malloc
 * @author ye_guohong
 * @date 2016-10-19
 * @param[in] sz size memory to malloc
 * @return void * address to memory
 * @retval NULL if malloc memory failed
 * @retval others if malloc memory successed
 */
static void *camctrl_cb_malloc(unsigned long sz)
{
	void * ret;
	
	ret = drv_malloc(sz);
	if (ret == NULL)
		return NULL;
	memset(ret, 0, sz);
	return (void *)ret;
}

/**
 * @brief init libispdrv
 * @author ye_guohong   
 * @date 2016-10-19
 * @param[in] isp_sensor_cb sensor' callback function registered to libispdrv
 * @return bool
 * @retval true if init sucessed
 * @retval false if init failed
 */
bool camctrl_init_ispdrv(AK_ISP_SENSOR_CB *isp_sensor_cb)
{
	signed long ret;
	unsigned long isp_reg_base = 0x20000000;
	AK_ISP_FUNC_CB func_cb;

	func_cb.cb_printk		= camctrl_cb_printf;
	func_cb.cb_memcpy		= (ISPDRV_CB_MEMCPY)memcpy;
	func_cb.cb_memset		= (ISPDRV_CB_MEMSET)memset;
	func_cb.cb_malloc		= (ISPDRV_CB_MALLOC)camctrl_cb_malloc;
	func_cb.cb_free		= (ISPDRV_CB_FREE)drv_free;
	func_cb.cb_dmamalloc	= camctrl_cb_dmamalloc;
	func_cb.cb_dmafree		= camctrl_cb_dmafree;
	func_cb.cb_msleep		= camctrl_cb_msleep;
	func_cb.cb_cache_invalid = MMU_InvalidateDCache;//MMU_Clean_Invalidate_Dcache;

	ret = isp2_module_init(&func_cb, isp_sensor_cb, (void *)isp_reg_base);
	if (ret) {
		printf("%s init ispdrv fail\n", __func__);
		return false;
	}

	//printf("%s init ispdrv finish\n", __func__);
	return true;
}

/**
 * @brief deinit libispdrv
 * @author ye_guohong   
 * @date 2016-10-19
 * @param[in] isp_sensor_cb sensor' callback function registered to libispdrv
 * @return bool
 * @retval true if init sucessed
 * @retval false if init failed
 */
bool camctrl_deinit_ispdrv(void)
{
	isp2_module_fini();
	return true;
}

/**
 * @brief low level function to set pclk polar
 * @author ye_guohong   
 * @date 2016-10-19
 * @param[in] is_rising is pclk polar is rising
 * @return void
 */
static void __camctrl_set_pclk_polar(int is_rising)
{
    unsigned long regval;
    
    regval = REG32(PERI_CLOCK_DIV_REG_2);
    if (is_rising)
        regval |= (0x1 << 20);
    else
        regval &= ~(0x1 << 20);
    REG32(PERI_CLOCK_DIV_REG_2) = regval;
}

/**
 * @brief high level function to set pclk polar
 * @author ye_guohong   
 * @date 2016-10-19
 * @param[]
 * @return void
 */
static void camctrl_set_pclk_polar(void)
{
	signed long pclk_polar;

	pclk_polar = ak_isp_get_pclk_polar();
	switch (pclk_polar) {
	case POLAR_RISING:
		__camctrl_set_pclk_polar(1);
		break;
	case POLAR_FALLING:
		__camctrl_set_pclk_polar(0);
		break;
	default:
		printf("pclk polar wrong: %ld\n", pclk_polar);
		break;
	}
}

/**
 * @brief set sensor & ch1 & ch2 resolution, buffers infomation
 * @author ye_guohong   
 * @date 2016-10-19
 * @param[in] cam_info set cam information
 * @return bool
 * @retval true if set sucessed
 * @retval false if set failed
 */
bool camctrl_set_info(struct camera_info *cam_info)
{

	camctrl_set_pclk_polar();
	//ak_isp_vi_set_crop(0, 0, cam_info->cam_width, cam_info->cam_height);
	ak_isp_vi_set_crop(cam_info->crop_left, cam_info->crop_top, cam_info->crop_width, cam_info->crop_height);
	ak_isp_vo_set_main_channel_scale(cam_info->ch1_dstWidth, cam_info->ch1_dstHeight);
	ak_isp_vo_set_sub_channel_scale(cam_info->ch2_dstWidth, cam_info->ch2_dstHeight);

	ak_isp_vo_set_buffer_addr(BUFFER_ONE, (unsigned long)cam_info->ch1_dYUV0, (unsigned long)cam_info->ch2_dYUV0);
	ak_isp_vo_enable_buffer(BUFFER_ONE);
	ak_isp_vo_set_buffer_addr(BUFFER_TWO, (unsigned long)cam_info->ch1_dYUV1, (unsigned long)cam_info->ch2_dYUV1);
	ak_isp_vo_enable_buffer(BUFFER_TWO);
	ak_isp_vo_set_buffer_addr(BUFFER_THREE, (unsigned long)cam_info->ch1_dYUV2, (unsigned long)cam_info->ch2_dYUV2);
	ak_isp_vo_enable_buffer(BUFFER_THREE);
	ak_isp_vo_set_buffer_addr(BUFFER_FOUR, (unsigned long)cam_info->ch1_dYUV3, (unsigned long)cam_info->ch2_dYUV3);
	ak_isp_vo_enable_buffer(BUFFER_FOUR);

	return true;
}

/**
 * @brief set isp to start capture video
 * @author ye_guohong   
 * @date 2016-10-19
 * @param[in] isp_working_mode isp working mode for capturing
 * @return void
 */
void camctrl_start_captuing(enum isp_working_mode mode)
{
	ak_isp_vi_apply_mode(mode);
	ak_isp_vo_enable_irq_status(1);
	ak_isp_vi_start_capturing();
}

/**
 * @brief set isp to stop capture video
 * @author ye_guohong   
 * @date 2016-10-19
 * @param
 * @return void
 */
void camctrl_stop_captuing(void)
{
	int i;
	
	for (i = 0; i < 4; i++)
		ak_isp_vo_disable_buffer(BUFFER_ONE + i);

	ak_isp_vi_stop_capturing();
	ak_isp_vo_enable_irq_status(0);
}

/**
 * @brief read camera controller's register, and check the frame finished or occur errorred
 * @author xia_wenting   
 * @date 2010-12-06
 * @param
 * @return bool
 * @retval true the frame finished
 * @retval false the frame not finished or occur errorred
 */
 bool camctrl_check_status(void)
{
	unsigned long status;

	status = ak_isp_vo_check_irq_status();
	if (!(status & 0x01)) {
		ak_isp_vo_clear_irq_status(0xfffe);
		printf("%s %d status:0x%lx\n", __func__, __LINE__, status);
		return false;
	}

	return true;
}

/**
 * @brief enable isp buffer to work
 * @author ye_guohong   
 * @date 2016-10-19
 * @param[in] id buffer's index
 * @return void
 */
void camctrl_enable_buffer(unsigned char id)
{
	ak_isp_vo_enable_buffer(BUFFER_ONE + id);
}

/**
 * @brief isp's frequent works 
 * @author ye_guohong   
 * @date 2016-10-19
 * @param
 * @return void
 */
void camctrl_isp_works(void)
{
	ak_isp_awb_work();
	ak_isp_ae_work();
}

/**
 * @brief pause isp not to capture video
 * @author ye_guohong   
 * @date 2016-10-19
 * @param
 * @return bool
 * @retval true if pause sucessed
 * @retval false if pause failed
 */
bool camctrl_pause_isp(void)
{
	ispdrv_isp_pause();
	return true;
}

/**
 * @brief resume isp not to capture video
 * @author ye_guohong   
 * @date 2016-10-19
 * @param
 * @return bool
 * @retval true if resume sucessed
 * @retval false if resume failed
 */
bool camctrl_resume_isp(void)
{
	ispdrv_isp_resume();
	return true;
}

/**
 * @brief low level function for camera interrupt
 * @author ye_guohong   
 * @date 2016-10-19
 * @param
 * @return bool
 * @retval true if sucessed
 * @retval false if failed
 */
static bool camctrl_interrupt_handler(void)
{    
    if (m_CameraInterruptCallback != NULL)
    {
        m_CameraInterruptCallback();
        //akprintf(C3, M_DRVSYS, "camctrl_interrupt_handler!\n");
    }
    else
    {
        camctrl_check_status();
    }

    return true;
}

#endif

