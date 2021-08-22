/**
 * @FILENAME ak_isp_char.h
 * @BRIEF    the  interface for isp
 * Copyright @ 2010 Anyka (Guangzhou) Software Technology Co., LTD
 * @AUTHOR   yang_mengxia
 * VERSION   1.0
 * @REF 
 */ 
#ifndef __AK_ISP_CHAR_H__
#define __AK_ISP_CHAR_H__

#include "ispdrv_modules_interface.h"
// IOCTL interface commands
#define AK_ISP_USER_CID_SET_ZOOM			   0x00010000 
#define AK_ISP_USER_CID_SET_SUB_CHANNEL		   0x00010001 
#define AK_ISP_USER_CID_SET_OCCLUSION		   0x00010002 
#define AK_ISP_USER_CID_SET_OCCLUSION_COLOR	   0x00010003 
#define AK_ISP_USER_CID_SET_GAMMA			   0x00010004 
#define AK_ISP_USER_CID_SET_SATURATION		   0x00010005 
#define AK_ISP_USER_CID_SET_BRIGHTNESS		   0x00010006 
#define AK_ISP_USER_CID_SET_CONTRAST		   0x00010007 
#define AK_ISP_USER_CID_SET_SHARPNESS		   0x00010008 

#define AK_ISP_USER_CID_SET_POWER_LINE_FREQUENCY 0x00010009 

#define AK_ISP_USER_CID_SET_OSD_COLOR_TABLE_ATTR 0x0001000a 
			
#define AK_ISP_USER_CID_SET_MAIN_CHANNEL_OSD_CONTEXT_ATTR 0x0001000b 
			
#define AK_ISP_USER_CID_SET_SUB_CHANNEL_OSD_CONTEXT_ATTR 0x0001000c 
			
#define AK_ISP_USER_CID_SET_MAIN_CHANNEL_OSD_MEM_ATTR 0x0001000d 
			
#define AK_ISP_USER_CID_SET_SUB_CHANNEL_OSD_MEM_ATTR 0x0001000e 	

#define  AK_ISP_VP_GET_BLC			   1
#define  AK_ISP_VP_SET_BLC			   2 
#define  AK_ISP_VP_GET_LSC      	   3 
#define  AK_ISP_VP_SET_LSC      	   4 
#define  AK_ISP_VP_GET_GB			   5 
#define  AK_ISP_VP_SET_GB			   6 
#define  AK_ISP_VP_GET_GB_LINKAGE	   7 
#define  AK_ISP_VP_SET_GB_LINKAGE	   8 
#define  AK_ISP_VP_GET_RAW_LUT		   9 
#define  AK_ISP_VP_SET_RAW_LUT		   10 
#define  AK_ISP_VP_GET_RAW_NR1		   11 
#define  AK_ISP_VP_SET_RAW_NR1		   12 
#define  AK_ISP_VP_GET_DEMO			   13 
#define  AK_ISP_VP_SET_DEMO			   14 
#define  AK_ISP_GET_DPC				   15 
#define  AK_ISP_SET_DPC				   16 
#define  AK_ISP_GET_CCM				   17 
#define  AK_ISP_SET_CCM				   18 
#define  AK_ISP_GET_CCM_EX			   19 
#define  AK_ISP_SET_CCM_EX			   20 
#define  AK_ISP_SET_WHITE_COLOR_S	   21 
#define  AK_ISP_GET_WHITE_COLOR_S	   22 
#define  AK_ISP_SET_CCM_FINE		   23 
#define  AK_ISP_GET_CCM_FINE		   24 
#define  AK_ISP_GET_RGB_GAMMA		   25 
#define  AK_ISP_SET_RGB_GAMMA		   26 
#define  AK_ISP_SET_WDR				   27 
#define  AK_ISP_GET_WDR				   28 
#define  AK_ISP_SET_WDR_EX			   29 
#define  AK_ISP_GET_WDR_EX			   30 
#define  AK_ISP_SET_EDGE			   31 
#define  AK_ISP_GET_EDGE			   32 
#define  AK_ISP_SET_EDGE_EX			   33 
#define  AK_ISP_GET_EDGE_EX			   34 
#define  AK_ISP_SET_EDGE_LINKAGE	   35 
#define  AK_ISP_GET_EDGE_LINKAGE	   36 
#define  AK_ISP_SET_SHARP			   37 
#define  AK_ISP_GET_SHARP			   38 
#define  AK_ISP_SET_SHARP_EX		   39 
#define  AK_ISP_GET_SHARP_EX		   40 
#define  AK_ISP_SET_SHARP_LINKAGE	   41 
#define  AK_ISP_GET_SHARP_LINKAGE	   42 
#define  AK_ISP_SET_Y_NR2			   43 
#define  AK_ISP_GET_Y_NR2			   44 
#define  AK_ISP_SET_Y_NR2_LINKAGE	   45 
#define  AK_ISP_GET_Y_NR2_LINKAGE	   46 
#define  AK_ISP_SET_3D_NR			   47 
#define  AK_ISP_GET_3D_NR			   48 
#define  AK_ISP_SET_3D_NR_EX		   49 
#define  AK_ISP_GET_3D_NR_EX		   50 
#define  AK_ISP_SET_3D_NR_LINKAGE	   51 
#define  AK_ISP_GET_3D_NR_LINKAGE	   52 
#define  AK_ISP_GET_FCS				   53 
#define  AK_ISP_SET_FCS				   54 
#define  AK_ISP_SET_FCS_LINKAGE		   55 
#define  AK_ISP_GET_FCS_LINKAGE		   56 
#define  AK_ISP_SET_CONTRAST		   57 
#define  AK_ISP_GET_CONTRAST		   58 
#define  AK_ISP_SET_SAT				   59 
#define  AK_ISP_GET_SAT				   60 
#define  AK_ISP_SET_SAT_LINKAGE		   61 
#define  AK_ISP_GET_SAT_LINKAGE		   62 
#define  AK_ISP_SET_RGB2YUV			   63 
#define  AK_ISP_GET_RGB2YUV			   64 
#define  AK_ISP_SET_YUV_EFFECT		   65 
#define  AK_ISP_GET_YUV_EFFECT		   66 
#define  AK_ISP_SET_RAW_HIST		   67 
#define  AK_ISP_GET_RAW_HIST		   68 
#define  AK_ISP_GET_RAW_HIST_STAT_INT	   69 
#define  AK_ISP_SET_RGB_HIST		   70 
#define  AK_ISP_GET_RGB_HIST		   71 
#define  AK_ISP_GET_RGB_HIST_STAT_INT	   72 
#define  AK_ISP_SET_Y_HIST			   73 
#define  AK_ISP_GET_Y_HIST			   74 
#define  AK_ISP_GET_Y_HIST_STAT_INT		   75 
#define  AK_ISP_SET_EXP_TYPE		   76 
#define  AK_ISP_GET_EXP_TYPE		   77 
#define  AK_ISP_SET_FRAME_RATE		   78 
#define  AK_ISP_GET_FRAME_RATE		   79 
#define  AK_ISP_SET_AE				   80 
#define  AK_ISP_GET_AE				   81 
#define  AK_ISP_GET_AE_RUN_INFO_INT		   82 
#define  AK_ISP_SET_WB_TYPE			   83 
#define  AK_ISP_GET_WB_TYPE			   84 
#define  AK_ISP_SET_AWB				   85 
#define  AK_ISP_GET_AWB				   86 
#define  AK_ISP_SET_AWB_DEFAULT		   87 
#define  AK_ISP_GET_AWB_DEFAULT		   88 
#define  AK_ISP_GET_AWB_STAT_INFO_INT	   89 


#define  AK_ISP_SET_MASK_COLOR		   92 
#define  AK_ISP_GET_MASK_COLOR	       93 
#define  AK_ISP_SET_WEIGHT			   94 
#define  AK_ISP_GET_WEIGHT	   		   95 
#define  AK_ISP_SET_AF				   96 
#define  AK_ISP_GET_AF	    		   97 
#define  AK_ISP_GET_AF_STAT_INT	    	   98 
#define  AK_ISP_SET_MWB				   99 
#define  AK_ISP_GET_MWB				   100

#define  AK_ISP_SET_MAIN_CHAN_MASK_AREA		   90 
#define  AK_ISP_GET_MAIN_CHAN_MASK_AREA	       91 
#define  AK_ISP_SET_SUB_CHAN_MASK_AREA		   101 
#define  AK_ISP_GET_SUB_CHAN_MASK_AREA	       102 

#define  AK_ISP_SET_3D_NR_REF		   103 
#define  AK_ISP_GET_3D_NR_REF	       104 

#define  AK_ISP_INIT_SENSOR_DEV		   105 
#define  AK_ISP_SET_3D_NR_PHYADDR	   106 
#define  AK_ISP_SET_SENSOR_REG		   107 
#define  AK_ISP_GET_SENSOR_REG		   108 
#define  AK_ISP_SET_USER_PARAMS		   109 
#define  AK_ISP_SET_MISC_ATTR		   110 
#define  AK_ISP_GET_MISC_ATTR		   111 

#define  AK_ISP_GET_3D_NR_STAT_INFO_INT	   112 
#define  AK_ISP_GET_SENSOR_ID		   113 

#define  AK_ISP_SET_ISP_CAPTURING	   114 

#define  AK_ISP_SET_AWB_EX			   115 
#define  AK_ISP_GET_AWB_EX			   116 

#define  AK_ISP_SET_Y_GAMMA			   117 
#define  AK_ISP_GET_Y_GAMMA			   118 

#define  AK_ISP_SET_HUE				   119 
#define  AK_ISP_GET_HUE				   120 

#define  AK_ISP_SET_FLIP_MIRROR		   121 
#define  AK_ISP_SET_SENSOR_FPS		   122 
#define  AK_ISP_GET_SENSOR_FPS		   123 
#define  AK_ISP_GET_WORK_SCENE		   124
#define  AK_ISP_SET_MAIN_CHANNEL_SCALE 125
#define  AK_ISP_SET_SUB_CHANNEL_SCALE 126
#define  AK_ISP_SET_CROP 127

/* Blow is the params that user can adjust in real time */
typedef struct {
	int id;
	unsigned char data[128];
} AK_ISP_USER_PARAM;

struct isp_zoom_info {
	int channel;
	int cut_xpos;
	int cut_ypos;
	int cut_width;
	int cut_height;
	int out_width;
	int out_height;
};

struct isp_channel2_info {
	int width;
	int height;
};

struct isp_mask_area {
	unsigned short start_xpos;
	unsigned short end_xpos;
	unsigned short start_ypos;
	unsigned short end_ypos;
	unsigned char enable;
};

struct isp_mask_area_info {
	struct isp_mask_area mask[4];
};

struct isp_mask_color_info {
	unsigned char color_type;
	unsigned char mk_alpha;
	unsigned char y_mk_color;
	unsigned char u_mk_color;
	unsigned char v_mk_color;
};

struct isp_gamma_info {
	int value;
};

struct isp_saturation_info {
	int value;
};

struct isp_brightness_info {
	int value;
};

struct isp_contrast_info {
	int value;
};

struct isp_sharp_info {
	int value;
};

struct isp_power_line_freq_info {
	int value;
};

struct isp_flip_mirror_info {
	int flip_en;
	int mirror_en;
};

struct isp_osd_color_table_attr {
	unsigned int color_table[16];
};

struct isp_osd_context_attr {
	unsigned char	*osd_context_addr;
	unsigned int	osd_width;
	unsigned int	osd_height;
	unsigned short	start_xpos;
	unsigned short	start_ypos;
	unsigned short	alpha;
	unsigned short	enable;
};

struct isp_osd_mem_attr {
	unsigned char	*dma_paddr;	
	unsigned int	size;
};

typedef struct ak_isp_channel_scale
{
	int width;
	int height;
}AK_ISP_CHANNEL_SCALE;

typedef struct ak_isp_crop
{
	int left;
	int top;
	int width;
	int height;
}AK_ISP_CROP;
long akisp_ioctl( unsigned int cmd, void* arg);
int isp_get_statinfo(enum isp_module_id module_id, void *buf, unsigned int *size);

int isp_get_hz(void);

/**  
 * brief: set power hz
 * @value[IN]: power hz, 50 or 60
 * return: 0 success, -1 failed
 * notes: 
 */
int isp_set_hz(int hz);


/**  
 * brief: malloc 3D NR memery
 * @width[IN]: main channel resulution of width
 * @height[IN]: main channel resulution of height 
 * return: memery address
 * notes: 
 */
void *isp_3D_NR_create(int width, int height);

#endif
