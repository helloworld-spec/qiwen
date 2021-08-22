/** 
 * brief :get Font data
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * date: 2016-10-26
 * version 1.0
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <fcntl.h>

#include "osd_draw.h"
#include "osd_font_data.h"

#include "ak_global.h"
#include "ak_common.h"
#include "ak_vpss.h"

#define UNICODE_START_CODE  0X4E00
#define UNICODE_END_CODE    0X9FA5

struct font_data_node {
	unsigned short unicode;
	unsigned char  *data;		
};

struct font_data_info {
	int num;    			/* font number */ 
	int fd;    				/* fd for font file */ 
	int byte;  				/* number byte of each font use */
	unsigned int size; 		/* font size in pixel */   
	struct font_data_node buf[FONT_MAX];    
};

static int init_flag = 0;
static struct font_data_info osd_font[VPSS_OSD_CHANNELS_MAX];
static char font_data_file[VPSS_OSD_CHANNELS_MAX][100];

/* 标准的显示汉字，包括"星期一二三四五六天年月日" */
static unsigned short basic_char[] = {
	'-', ':', '/', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
	'S', 'u', 'M', 'o', 'n','T', 'e', 's', 'W', 'd', 'h', 'r', 'F',  'i', 'S', 'a', 
	0x5e74, 0x6708, 0x65e5, 0x661f, 0x671f, 0x5929, 0x4e00, 0x4e8c, 
	0x4e09, 0x56db, 0x4e94, 0x516d
};

/** 
 * get_font_data_from_file: get one  font data of   char from  file . 
 * @channel[IN]: display channel [0,1]  
 * @code[IN]: unicode of char, the font of it is to be got from file
 * return: 0 - success; otherwise -1; 
 */
static int get_font_data_from_file( int channel,unsigned short code, 
				unsigned char *font_buffer)
{
	int offset = 0;

	if(code < 0x80) {
		offset = (code - 1);
	} else if (code >= UNICODE_START_CODE && code <= UNICODE_END_CODE) {
		offset = (127 + code - UNICODE_START_CODE);
	} else {
		/* no font for such code */
		offset = 0x1F;
		//ak_print_normal_ex("we don't include the font(0x%04x)\n", code);
	}
	
	if(osd_font[channel].fd < 0) {
		osd_font[channel].fd = open(font_data_file[channel], O_RDONLY);           
		if(osd_font[channel].fd >= 0)
		ak_print_normal_ex("fd:%d byte:%d\n",
			osd_font[channel].fd, osd_font[channel].byte);
	}
	
	if(osd_font[channel].fd < 0) {
		ak_print_normal_ex("we can't open the font data bin\n");
		return AK_FAILED;
	}
	
	lseek(osd_font[channel].fd, offset * osd_font[channel].byte, SEEK_SET);
	read(osd_font[channel].fd, font_buffer, osd_font[channel].byte);
	
	return AK_SUCCESS;
}

/** 
 * get_basic_font_data - get all font data of basic char from file 
 *			and reserve in buf. 
 * @channel[IN]: display channel [0,1]  
 * return: 0 - success; otherwise -1; 
 */
static int get_basic_font_data(int channel)
{
	int i = 0;
	int ret = AK_FAILED;
	int len = sizeof(basic_char) / sizeof(basic_char[1]);	

	osd_font[channel].num = len;  
	for(i=0; i<len; ++i) {
		osd_font[channel].buf[i].unicode = basic_char[i];
		osd_font[channel].buf[i].data = calloc(1, osd_font[channel].byte);
		if (!osd_font[channel].buf[i].data) {
			break;
		}
		
		if(get_font_data_from_file(channel, osd_font[channel].buf[i].unicode,
				osd_font[channel].buf[i].data)) {
			break;
		}
	}

	if (i >= len) {
		ret = AK_SUCCESS;
	} else {
		for(i=0; i<len; ++i) {
			if(osd_font[channel].buf[i].data) {
				free(osd_font[channel].buf[i].data);
				osd_font[channel].buf[i].data = NULL;
			}
		}
	}
	
	return ret;
}

static void init_font_data()
{
	if(0 == init_flag) {
		init_flag = 1;
		
		int i = 0;
		for (i=0; i<VPSS_OSD_CHANNELS_MAX; ++i) {
			memset(&osd_font[i], 0x00, sizeof(struct font_data_info));
			osd_font[i].fd = -1;
		}
	}
}

/** 
 * get_one_font_data - get one  font data from buf or  file. 
 * @channel[IN]: display channel [0,1] 
 * @code[IN]: unicode of char, the font of it is to be display
 * return: 0 - success; otherwise -1; 
 */
unsigned char* get_one_font_data(unsigned short code)
{
	int i = 0;
	int ret;
	int channel = DEFAULT_CHANNEL;
		
	for(i = 0; i < osd_font[channel].num; i++) {
		if(osd_font[channel].buf[i].unicode == code) {        
			return osd_font[channel].buf[i].data;            
		}
	}
	
	/*if the font of code is not found in osd_font, retry to find in font file.	*/
	if((i >= osd_font[channel].num) && (osd_font[channel].fd >= 0)) {
		osd_font[channel].buf[i].unicode = code; 		
		osd_font[channel].buf[i].data = calloc(1, osd_font[channel].byte);
		if (!osd_font[channel].buf[i].data) {
			return NULL;
		}
		
		ret = get_font_data_from_file(channel,
			osd_font[channel].buf[i].unicode, osd_font[channel].buf[i].data);
		if(ret) {
			free(osd_font[channel].buf[i].data);
			osd_font[channel].buf[i].data = NULL;
		} else {
			osd_font[channel].num++;
			return osd_font[channel].buf[i].data;
		}
	}
	
	return NULL;
}

int get_file_font_size(void)
{
	if(!init_flag){
		return 0;
	}

	return osd_font[DEFAULT_CHANNEL].size;
}

void free_font_data(void)
{	
	int channel = DEFAULT_CHANNEL;
		
	for(int i = 0; i < osd_font[channel].num; i++) {
		if(osd_font[channel].buf[i].data) {
			free(osd_font[channel].buf[i].data);
		}		
	}
	osd_font[channel].num = 0;
	if(osd_font[channel].fd >= 0){
		close(osd_font[channel].fd);
		osd_font[channel].fd = -1;
	}
}

/** 
 * ak_osd_set_font_file - set char font data file. 
 * @size[IN]:  size of font  in font data file 
 * @file_path[IN]: font data file path , can be read 
 * return: 0 - success; otherwise -1; 
 */
int ak_osd_set_font_file(int size, const char * file)
{ 
	int channel = DEFAULT_CHANNEL;
	
	init_font_data();
	strncpy(font_data_file[channel], file, 99);
	 
	osd_font[channel].byte = size * size / 8;
	osd_font[channel].size = size;
	ak_print_normal_ex("channel=%d, font size=%d\n", 
		channel, osd_font[channel].size);

	return get_basic_font_data(channel);
}
