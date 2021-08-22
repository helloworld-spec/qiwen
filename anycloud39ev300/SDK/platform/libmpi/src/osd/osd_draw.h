#ifndef __AKFONT_DISPLAY_H__
#define __AKFONT_DISPLAY_H__

/** 
 * constant defined
 * include font size, osd max line, default channel. 
 */
#define FONT_SIZE_48X48 48
#define FONT_SIZE_16X16 16 

#define OSD_MAX_LINES				5		
#define OSD_UNICODE_CODES_NUM		100	
#define FONT_MAX           			OSD_UNICODE_CODES_NUM

#define DEFAULT_CHANNEL 			1
int osd_get_common_value(int *alpha, int *front_color, int *ground_color);
int osd_get_area_value(int channel, int osd_chn, int *xstart, int *ystart, 
	int *width, int *height, int *disp_font_size);

#endif
