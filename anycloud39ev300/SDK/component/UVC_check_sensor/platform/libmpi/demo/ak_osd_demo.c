/******************************************************
 * @brief  osd demo
 
 * @Copyright (C), 2016-2017, Anyka Tech. Co., Ltd.
 
 * @Modification  2017-5-17 
*******************************************************/
#include "ak_common.h"
#include "ak_vi.h"
#include "ak_osd.h"
#include "command.h"
#include "drv_api.h"
#include "libc_mem.h"
#include "kernel.h"

/******************************************************
  *                    Constant         
  ******************************************************/
static char *help[]={
	"osd module demo",
	"usage:osddemo <position> <font_color> <background_color>\n"
	"      position: osd show position,1 <= position <= 5,\n"
	"          1-full(defalut),2-leftup,3-leftdown,4-rightup,5-rightdown\n"
	"      font_color:1 <=font_color <=16, defualt 16(black)\n"
	"      background_color: 1 <=background_color <=16 default 2(white)\n",
	"      sample : osddemo 1\n"
	
};

/* include chinese character "星期一二三四五六天年月日" */
static unsigned short basic_char[] = {
	'-', ':', '/', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
	'S', 'u', 'M', 'o', 'n','T', 'e', 's', 'W', 'd', 'h', 'r', 'F',  'i', 'S', 'a', 
	'A','y','k',
	0x5e74, 0x6708, 0x65e5, 0x661f, 0x671f, 0x5929, 0x4e00, 0x4e8c, 
	0x4e09, 0x56db, 0x4e94, 0x516d
};

/*str1 is : 2016-11-10 18:18:18 anyka 星期一 */
static unsigned short osd_str1[] =
{ '2', '0', '1', '6', '-', '1', '1', '-','1', '0',' ',
  '1', '8', ':','1', '8', ':','1', '8', ' ',
  'A','n','y','k','a',0x661f,0x671f ,0x56db};

/*str2 is : 16年10月18日*/
static unsigned short osd_str2[] =
{
	'1', '6',0x5e74, '1', '0',0x6708,'1', '8', 0x65e5,
};

char * osd_pos_str[5] = {"full","leftup","leftdown",
								"rightup","rightdown"};

/******************************************************
  *                    Macro         
  ******************************************************/
#define CH0_WIDTH 1280
#define CH0_HEIGHT 720
#define CH1_WIDTH 640
#define CH1_HEIGHT 480

#define FONT_BIN "FONT"
#define OSD_UNICODE_CODES_NUM		100	
#define FONT_MAX           			OSD_UNICODE_CODES_NUM

/* tf card the least free size */
#define MIN_DISK_SIZE_FOR_OSD ((CH0_WIDTH*CH0_HEIGHT*3+CH1_WIDTH * CH1_HEIGHT*3)/1024)
/******************************************************
  *                    Type Definitions         
  ******************************************************/
struct font_data_node {
	unsigned short unicode;
	unsigned char  *data;		
};

struct font_data_info {
	int num;    			/* font number */ 
	void* fd;    				/* fd for font file */ 
	int byte;  				/* number byte of each font use */
	unsigned int size; 		/* font size in pixel */   
	struct font_data_node buf[FONT_MAX];    
};

/******************************************************
  *                    Global Variables         
  ******************************************************/
static void * vi_handle = NULL;
static int init_flag = 0;

/*  the osd font background color*/
static int background_color = 0;

/* the osd font color*/
static int font_color = 0;

/*the buf reserve all font data of basic char from partition */ 
static struct font_data_info osd_font;

/* find all basic_char in partition */
static int find_word_count = 0;

int osd_pos_str_index = 0;

/******************************************************
*               Function prototype                           
******************************************************/
static void free_basic_font_data(void);

/******************************************************
*               Function Declarations
******************************************************/

/**
 *ai_get_disk_free_size - get disk free size, appoint the dir path
 * @path[IN]: disk dir full path
 * return: disk free size in KB, -1 failed
 */

static unsigned long osd_get_disk_free_size(const char *path)
{
	struct statfs disk_statfs;
    unsigned long free_size;
	
	bzero( &disk_statfs, sizeof(struct statfs));

	if( statfs( path, &disk_statfs ) == -1 ) 
	{		
		ak_print_error("fs_get_disk_free_size error!\n"); 
		return -1;
	}
	
    free_size = disk_statfs.f_bsize;
    free_size = free_size / 512;
    free_size = free_size * disk_statfs.f_bavail / 2;
   
	return free_size;
}
/** 
 * @brief  malloc memery for all font data of basic char from partition. 
 * @size[IN]: the font size[0,1]  
 * return: 0 - success; otherwise -1; 
 */
static int init_basic_font_data(int size)
{
	int i = 0;
	int ret = AK_FAILED;
	int len = sizeof(basic_char) / sizeof(basic_char[0]);	

	if(1 == init_flag) {
		return AK_SUCCESS;
	}

	init_flag = 1;
	memset(&osd_font, 0x00, sizeof(struct font_data_info));

	osd_font.byte = size * size / 8;
	osd_font.size = size;

	ak_print_normal_ex("font size=%d\n",osd_font.size);
	
	/* init font data */
	osd_font.num = len;  
	for(i=0; i<len; ++i) {
		osd_font.buf[i].unicode = basic_char[i];

		/* osd_font.byte*4:use ak_osd_draw_matrix(), font buf convert to matrix */
		osd_font.buf[i].data= (unsigned char *)calloc(1, osd_font.byte*4);

		if (!osd_font.buf[i].data) {
			break;
		}
	}
	
	if (i >= len) {
		ret = AK_SUCCESS;
	} else {
		
		ak_print_error_ex("calloc font data error.\n");
		free_basic_font_data();
	}
	return ret;
}


/** 
 * @brief - free the buf for all font data of basic char from 
 *			partition. 
 * return: 0 - success; otherwise -1; 
 */
static void free_basic_font_data(void)
{
	int len = osd_font.num;	

	if(1 == init_flag) {
		init_flag = 0;
	
		for(int i=0; i<len; ++i) {
			if(osd_font.buf[i].data) {
				free(osd_font.buf[i].data);
				osd_font.buf[i].data = NULL;
			}
		}

		memset(&osd_font, 0x00, sizeof(struct font_data_info));
	}
}

/** 
 * @brief - change a font data to matrix.
 *	 
 * @data[OUT]: the font matrix buf  
 * @p_font[IN]: the font data from partition  
 * @font_byte[IN]: the font size in partition  (unit:byte) 
 *
 */
static void change_font_to_matrix(unsigned char* data,
		unsigned char *p_font, int font_byte)
{
	int i = 0, j=0;

	/* def_color_tables's index */
	char color_index[2]={background_color,font_color};
	int dot_bit_value = 0;
	int matrix_data_out = 0;
	char dot_value_char;
	
	for(i=0; i<font_byte; i++)
	{
		/* clean matrix_data_out */
		matrix_data_out = 0;
		
		dot_value_char = *(p_font +i);
		/* chang 1 byte to 4 bytes use new color info*/
		for(j=0; j < 8; j++)
		{
			dot_bit_value = (dot_value_char >> j) & 0x1 ;
			if(j == 0)
				matrix_data_out = matrix_data_out | color_index[dot_bit_value];
			else
				matrix_data_out = (matrix_data_out << 4) | color_index[dot_bit_value];
		}

		memcpy((data+(i*4)),&matrix_data_out, sizeof(matrix_data_out));

	}

}
/** 
 * @brief - get all font data of basic char from file 
 *			and reserve in buf. 
 * @code[IN]: the unicode of the font  
 * @p_font[OUT]:  the font data in font buf
 */
static void get_basic_font_data(unsigned short code, 
		unsigned char * p_font)
{
	int i = 0;
	int len = osd_font.num;
	find_word_count = len;
	
	for( i=0; i<len; i++) {

		if(basic_char[i] == code)
		{
			change_font_to_matrix(osd_font.buf[i].data,p_font, osd_font.byte);

			find_word_count--;
			break;
		}

	}

}

/** 
 * @brief - find font data from font partition in basic_char
 * 				to reserve buf
 * return: 0 - success; otherwise -1; 
 */
static int osd_set_font_partition(void)
{

	/** step1: Open font partition */
	void* font_fd = (void *)ak_partition_open(FONT_BIN);
	if(font_fd == NULL) {
		ak_print_error("we can't open the font data bin\n");
		return AK_FAILED;
	}

	int font_len = osd_font.byte;

	/* font_len + unicode len */
	int osd_font_len = font_len+sizeof(unsigned short);
	
	/*
	  * read 128 words once ,
	  * read_count_once%font_len == 0
	  * read_count_once%flash_page_count == 0 need to know page size, now page_size is 256
	  */
	int words_read_count = 128;
	int read_count_once = osd_font_len*words_read_count;
	unsigned char * p_read_part_once = calloc(1, read_count_once);
	
	unsigned short  *p_word_tmp = NULL;

	int ret  =  AK_SUCCESS, i = 0;
start_loop:
	/** step2: Read font partition */
	ret = ak_partition_read(font_fd, p_read_part_once, read_count_once);
	if (ret == -1)
	{

		ak_print_error_ex("the end of the font file.\r\n");
		if (find_word_count > 0)
		{
			ak_print_error_ex("has words not in the font file,plean support a new font file.\r\n");
			ret =  AK_FAILED;
		}
		goto end_loop;
	}

	for(i=0; i<words_read_count ; i++)
	{
		p_word_tmp = (unsigned short *)(p_read_part_once + (i * osd_font_len));

		/* 0x0 and 0xffff is font partition over */
		if(0x0 == *p_word_tmp || 0xFFFF == *p_word_tmp)
			goto end_loop;

		/*
		  *compare unicode with basic font 
		  *p_word_tmp+1 is offset 2 bytes,skip unicode
		  */
		get_basic_font_data((*p_word_tmp), (unsigned char*)(p_word_tmp+1));

		if(find_word_count == 0)
			goto end_loop;
	}

	if(i == words_read_count)
		goto start_loop;

end_loop:	
	free(p_read_part_once);

	/** step3:  Close partition*/
	if(font_fd)
	{
		ak_partition_close(font_fd);
		font_fd = NULL;
	}

	return ret;
}

/** 
 * @brief - set char font data file. 
 * @size[IN]:  size of font  in font data file 
 * return: 0 - success; otherwise -1; 
 */
static int osd_set_font_file(int size)
{ 

	unsigned long start_time, end_time;
	struct ak_timeval start_tv, end_tv;
	// init font buf
	int ret = init_basic_font_data(size);
	if(ret == AK_FAILED)
		return ret;

	ak_get_ostime(&start_tv);

	/* reserve font to buf from partition */
	ret = osd_set_font_partition();
	if( ret == AK_FAILED )
		ak_print_error_ex("get font from partition failed.");

	ak_get_ostime(&end_tv);
	
	ak_print_notice_ex("load fond time %ld us.", end_tv.usec- start_tv.usec);
	return ret;
}

/** 
 * @brief - find font data from font buf. 
 * @code[IN]:  the unicode of the font 
 * return: char* - success; otherwise NULL; 
 */
char* find_one_font_data(unsigned short code)
{
	int i = 0;
	int len = osd_font.num;
	
	for(i = 0 ; i < len; i++)
	{
		if(osd_font.buf[i].unicode == code)
			return osd_font.buf[i].data;
	}

	return NULL;
}

/** 
 * @brief  init sensor. 
 * @handle[IN]:  the handler of the sensor 
 * return: 0 - success; otherwise -1; 
 */
static int osd_init_video_input(void * handle)
{
	struct video_resolution video_res;
	struct video_channel_attr ch_attr ;

	/* get sensor resolution */
	memset(&video_res, 0, sizeof(struct video_resolution));
	if (ak_vi_get_sensor_resolution(handle, &video_res))
		ak_print_error_ex("ak_mpi_vi_get_sensor_res fail!\n");
	else
		ak_print_normal("ak_mpi_vi_get_sensor_res ok! w:%d, h:%d\n",
			video_res.width, video_res.height);

	/* set video resolution and  crop information */
	memset(&ch_attr, 0, sizeof(struct video_channel_attr));
	ch_attr.res[VIDEO_CHN_MAIN].width = CH0_WIDTH;
	ch_attr.res[VIDEO_CHN_MAIN].height = CH0_HEIGHT;
	ch_attr.res[VIDEO_CHN_SUB].width = CH1_WIDTH;
	ch_attr.res[VIDEO_CHN_SUB].height = CH1_HEIGHT;
	ch_attr.crop.width = video_res.width;
	ch_attr.crop.height = video_res.height;

	if (ak_vi_set_channel_attr(handle, &ch_attr)) {
		ak_print_error("ak_vi_set_channel_attr fail!\n");
		return -1;
	} else {
		ak_print_normal("ak_vi_set_channel_attr ok!\n");
	}

	memset(&ch_attr, 0, sizeof(struct video_channel_attr));
	if (ak_vi_get_channel_attr(handle, &ch_attr)) {
		ak_print_error("ak_vi_get_channel_attr fail!\n");
	}

	return 0;
}

/*****************************************
 * @brief  save screen data. yuv format
 * @param handle[in]  vi handler
 * @param pos_id[in]  osd position index
 * @return on success return 0, fail return -1
 *****************************************/
static int get_yuv_file(void * handle, int pos_id)
{
	struct video_input_frame frame;
	FILE *fd = NULL;
	char name[128] = {0};
	struct ak_date systime;
	int i=0, channel = 0;

	memset(&frame, 0, sizeof(struct video_input_frame));
	/* clear frame buf which is old data */
	while((ak_vi_get_frame(handle, &frame) == 0)){
				ak_vi_release_frame(handle, &frame);
		memset(&frame, 0, sizeof(struct video_input_frame));
		if( i++ > 2) break;
	}

	/* i is save photo count.*/
	for ( i = 0; i < 2; i++) {
		memset(&frame, 0, sizeof(struct video_input_frame));
		if (ak_vi_get_frame(handle, &frame)) {
			ak_print_error("ak_video_input_get_frame fail!\n");
			ak_sleep_ms(2*1000);
			continue;
		} else {
			ak_print_normal("ak_video_input_get_frame ok!\n");

			/* save the YUV frame data */
			ak_get_localdate(&systime);
			for(channel = 0; channel < 2; channel++){

				memset(name, 0, 32);	    
				sprintf(name, "a:/CH%1d%s_%d%02d%02d%02d%02d%02d_%1d.yuv",
						channel,
						osd_pos_str[pos_id],
						systime.year, 
						systime.month, systime.day, systime.hour, systime.minute, 
						systime.second,
						i);
				ak_print_normal("filename:%s\n",name);
				fd = fopen(name,"w");
				if (fd != NULL) {
					if (frame.vi_frame[channel].len == fwrite(
								frame.vi_frame[channel].data, 1,
								frame.vi_frame[channel].len, fd)) {
						ak_print_normal_ex(" save YUV file ok!!--i=%d\n",i);
					}
					else {
						ak_print_error_ex(" save YUV file fail!!--i=%d\n",i);
					}
					fclose(fd);
				} else {
					ak_print_error("open YUV file fail!!\n");
				}
			}
			/* the frame has used,release the frame data */
			ak_vi_release_frame(handle, &frame);
			ak_sleep_ms(1000);
		}
	}

	return 0;
}

/*****************************************
 * @brief start to process osd
 * @param [void]  
 * @return on success return 0, fail return -1
 *****************************************/
static int test_osd(void)
{
	find_word_count = 0;

	/** step1: init font lib to memery */
	if (osd_set_font_file(24) < 0)	{
		ak_print_error("ak_osd_set_font_file fail!\n");
		return -1;
	}
	ak_print_normal("ak_osd_set_font_file ok!\n");

	/** step2: init osd font and channel config */
	if (ak_osd_init(vi_handle) < 0) {
		ak_print_error("ak_osd_init fail!\n");
		return -1;
	}
	ak_print_normal("ak_osd_init ok!\n");

	/** step3: get channel  resolution */
	/* set osd buff */
	int channel;
	for (channel = 0; channel < 2; channel++) {

		int width = (channel == 0) ? CH0_WIDTH : CH1_WIDTH;
		int height = (channel == 0) ?  CH0_HEIGHT: CH1_HEIGHT;
		int max_w, max_h;

		if (ak_osd_get_max_rect(channel, &max_w, &max_h) < 0) {
			ak_print_error_ex("chn:%d ak_osd_get_max_rect fail!\n", channel);
			return -1;
		}
		ak_print_normal_ex("chn:%d ak_osd_get_max_rect ok, max_w: %d max_h: %d\n",
				channel, max_w, max_h);

		width = (width > max_w) ? max_w : width;
		height = (height > max_h) ? max_h : height;

		if (ak_osd_set_rect(vi_handle, channel, 0, 0, width, height) < 0) {
			ak_print_error("chn: %d ak_osd_set_rect fail!\n", channel);
			return -1;
		}
		ak_print_normal("chn: %d ak_osd_set_rect ok!\n", channel);
	}

	/** step4: draw osd */
	int font_w = osd_font.size,font_h = osd_font.size;
	int osd_str1_count = (sizeof(osd_str1)/sizeof(osd_str1[0]));
	
	int main_startx[4] = { 0,0,1280,1280};
	int main_starty[4] = { 0,720,0,720};
	/* process 4 position */
	main_starty[1] -= font_h;
	main_starty[3] = main_starty[1];

	ak_print_normal_ex("ch0(%d,%d),(%d,%d),(%d,%d),(%d,%d)\n",
			main_startx[0],main_starty[0],
			main_startx[1],main_starty[1],
			main_startx[2],main_starty[2],
			main_startx[3],main_starty[3]);

	int sub_startx[4] = { 0,0,640,640};
	int sub_starty[4] = { 0,480,0,480};

	int osd_str2_len = (sizeof(osd_str2)/sizeof(osd_str2[0]));

	sub_starty[1] -= font_h;
	sub_startx[2] -= osd_str2_len * font_w;

	sub_startx[3] = sub_startx[2];
	sub_starty[3] = sub_starty[1];

	ak_print_normal_ex("ch1(%d,%d),(%d,%d),(%d,%d),(%d,%d)\n",
		sub_startx[0],sub_starty[0],
		sub_startx[1],sub_starty[1],
		sub_startx[2],sub_starty[2],
		sub_startx[3],sub_starty[3]);
	
	int osd_pos_id = osd_pos_str_index;
	int pos_show = 4, is_full = 1;
	int all_osd_font_len = 0;
	int osd_str1_index = 0;
	if ( 0 != osd_pos_id )
		{
		pos_show = osd_pos_id +1;
		is_full = 0;
		}
	/* calculate string length */
	for( osd_str1_index = 0; osd_str1_index < osd_str1_count; osd_str1_index++)
	{
		if(osd_str1_index != 0 && osd_str1[osd_str1_index-1] < 0xff)
				all_osd_font_len += font_w/2;
			else if(osd_str1_index != 0 )
				all_osd_font_len += font_w;
			
			if(osd_str1[osd_str1_index] == ' ')
			{
				continue;
			}

			if(osd_str1_index == osd_str1_count - 1)
			{	
				all_osd_font_len += (osd_str1[osd_str1_index] < 0xff ? font_w/2 : font_w);

				main_startx[2] -= all_osd_font_len;

				main_startx[3] = main_startx[2];
			}
	}

	/* display osd on 4 position: "leftup","leftdown","rightup","rightdown" */
	for (; osd_pos_id < pos_show; osd_pos_id++) {
		
		/* draw channel 0 osd */
		channel = 0;
		
		int offsetx=main_startx[is_full?osd_pos_id:osd_pos_id-1];
		char *osd_char;
		
		/* one bit change to color index */
		int matrix_size = osd_font.byte*4;
		
	
		for( osd_str1_index = 0; osd_str1_index < osd_str1_count; osd_str1_index++)
		{
			if(osd_str1_index != 0 && osd_str1[osd_str1_index-1] < 0xff)
				offsetx += font_w/2;
			else if(osd_str1_index != 0 )
				offsetx += font_w;
			
			if(osd_str1[osd_str1_index] == ' ')
			{
				continue;
			}

			osd_char = find_one_font_data(osd_str1[osd_str1_index]);

			if( ak_osd_draw_matrix(0, 
				offsetx, main_starty[is_full?osd_pos_id:osd_pos_id-1], 
				font_w, font_w , osd_char, matrix_size) < 0 )
			{ 
				ak_print_error_ex("ak_osd_draw_matrix fail!\n");
				return -1;
			}
		}
		ak_print_normal_ex("ch0 ak_osd_draw_matrix ok!\n");

		/* draw channel 1 osd */
		channel = 1;
		int osd_str2_index = 0;
		
		for( ; osd_str2_index < osd_str2_len; osd_str2_index++)
		{

			osd_char = find_one_font_data(osd_str2[osd_str2_index]);
			if( ak_osd_draw_matrix(1, 
				sub_startx[is_full?osd_pos_id:osd_pos_id-1] + osd_str2_index*font_w,
				sub_starty[is_full?osd_pos_id:osd_pos_id-1], 
				font_w, font_w , osd_char, matrix_size) < 0 )
			{ 
				ak_print_error_ex("ak_osd_draw_matrix fail!\n");
				return -1;
			}
		}
		ak_print_normal_ex("ch1 ak_osd_draw_matrix ok!\n");
		
	}

	/* draw matrix to osd
	{
		int size = 48;
		int matrix_size = 96 * 48 / 2;
		unsigned char *osd_char = calloc(1, matrix_size);
		memset(osd_char, 0x12, matrix_size);
		ak_osd_draw_matrix(0, size * 2, 144, size, size * 2, osd_char, matrix_size);
		ak_osd_draw_matrix(0, size * 6, 144, size * 2, size, osd_char, matrix_size);
		free(osd_char);
	}
	*/

	/** step5: save frames to show osd */
	int ret = ak_mount_fs(DEV_MMCBLOCK, 0, "");	
	if (ret < 0)
	{
		ak_print_error("mount sd fail!\n");
		goto finish_free;
	}
	

	char *path = "a:/";
	int free_size = osd_get_disk_free_size(path);
	
	if ( free_size < MIN_DISK_SIZE_FOR_OSD)
	{
		ak_print_error_ex("the card memery isn't enough!(%lu,%d)\n",
			    free_size, MIN_DISK_SIZE_FOR_OSD);
		goto ERR;
	}

	/* get screen data and save it in file. yuv format */
	if (get_yuv_file(vi_handle, osd_pos_str_index) < 0) {
		ak_print_error_ex("get_yuv_file fail!\n");
	}

	/* free font buf memery */
	free_basic_font_data();

ERR:
	ak_unmount_fs(DEV_MMCBLOCK, 0, "");
finish_free:
	ak_osd_destroy();

	return 0;
}

/*****************************************
 * @brief match sensor
 * @param [void]  
 * @return on success return 0, fail return -1
 *****************************************/
static int match_sensor(void)
{
	char *main_addr ="ISPCFG";
	if (ak_vi_match_sensor(main_addr)) {
		ak_print_error(" match sensor first_cfg fail!\n");
		return -1;
	} else {
		ak_print_normal(" ak_vi_match_sensor ok!\n");
	}

	return 0;
}

/*****************************************
 * @brief init params
 * @param argc[in]  the count of command param
 * @param args[in]  the command param
 * @return void
 *****************************************/
static int init_params(int argc, char **args)
{
	int i = 0;
	if (argc > 3)
	{
		return -1;
	}

	background_color = 0x1 ;/* background is white*/
	font_color = 0xf;/* font is black*/
	osd_pos_str_index = 0;
	for (i = 0; i < argc; i++) {
		switch (i) {
			case 0:
				osd_pos_str_index= atoi(args[i]);
				if ( osd_pos_str_index <1 || osd_pos_str_index > 5 
					|| strlen(args[i]) > 1 ) 
				{
					return -1;
				}
				osd_pos_str_index -= 1;
				break;
			case 1:
				font_color= atoi(args[i]);
				if (font_color <1 || font_color > 16 
					|| (font_color < 10 && strlen(args[i]) > 1)
					|| (font_color >= 10 && strlen(args[i]) > 2)
					) 
				{
					return -1;
				}

				font_color -= 1;
				break;
			case 2:
				background_color= atoi(args[i]);
				if (background_color <1 || background_color > 16
					|| (font_color < 10 && strlen(args[i]) > 1)
					|| (font_color >= 10 && strlen(args[i]) > 2)
					) 
				{
					return -1;
				}

				background_color -= 1;
				break;

			default:
				ak_print_error_ex("i err:%d\n", i);
		}
	}
	return 0;
}

/*****************************************
 * @brief start function for command
 * @param argc[in]  the count of command param
 * @param args[in]  the command param
 * @return void
 *****************************************/
void cmd_osd_test(int argc, char **argv)
{
	
	ak_print_notice("*** osd demo begin compile:%s %s %s ***\n",
			__DATE__, __TIME__, ak_osd_get_version());

	/* init params*/
	if(init_params(argc,argv))
	{
		ak_print_error("%s",help[1]);
		goto ERR;
	}

	/** step1: match sensor*/
	if (match_sensor()) {
		ak_print_error("match sensor fail!\n");
		goto ERR;
	}

	/** step2: open sensor*/
	vi_handle = ak_vi_open(VIDEO_DEV0);
	if (NULL == vi_handle) {
		ak_print_error("ak_vi_open fail!\n");
		goto ERR;
	} else {
		ak_print_normal("ak_vi_open ok!\n");
	}

	/** step3: init sensor*/
	if(osd_init_video_input(vi_handle) == 0){

		/** step4: process osd*/
		test_osd();
	}

	ak_vi_close(vi_handle);

	ak_print_notice("********** osd demo finish ************\n");

	ERR:
		return;
		
}

/*****************************************
 * @brief register osddemo command
 * @param [void]  
 * @return 0
 *****************************************/
static int cmd_osd_reg(void)
{
    cmd_register("osddemo", cmd_osd_test, help);
    return 0;
}

cmd_module_init(cmd_osd_reg)
