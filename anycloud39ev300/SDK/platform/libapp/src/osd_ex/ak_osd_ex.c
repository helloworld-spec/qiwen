#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <iconv.h>
#include "ak_thread.h"
#include "ak_common.h"
#include "ak_vi.h"
#include "ak_venc.h"
#include "ak_osd.h"
#include "ak_osd_ex.h"

#define CRYSTAL_CLEAR_COLOR_TABLE_INDEX	0		/*full transparentcolor table */
#define WHITE_COLOR_TABLE_INDEX 		1		/* white color table */
#define BLACK_COLOR_TABLE_INDEX 		2		/* black color table  */
#define OSD_FILE_FONT_SIZE	16
#define OSD_STAT_RATE	2	//per 2 seconds
#define OSD_STAT_LEN		50
#if 1
#define OSD_EX_FONT_BIN		"/usr/local/ak_font_16.bin"
#else
#define OSD_EX_FONT_BIN		"/tmp/ak_font_16.bin"
#endif
struct osd_rect_info {
	int width;
	int height;
};
struct ak_video_stat {
    int bps;
    int fps;
    int gop_len;

	char stat_str[OSD_STAT_LEN];
};

struct ak_osd_control {
	ak_pthread_t tid;	//osd thread id
	void *vi_handle;    // vi handle
	int run_flag;		//osd thread run flag
	int osd_switch;		//osd module on/off flag
	int video_stat_update[OSD_EX_VIDEO_CH];
	int osd_name_update;
	int osd_on_off[OSD_EX_VIDEO_CH]; //osd update on/off flag
	int init_flag;
};

static struct osd_rect_info rect_info[VIDEO_CHN_NUM][OSD_MAX_RECT];
static struct osd_ex_info osd_info;
static struct ak_video_stat osd_stat[VIDEO_CHN_NUM] = {{0}, {0}};
static struct ak_osd_control osd_contrl = {0};
static const char osd_ex_version[] = "libapp_osd_ex V1.2.03";

/**
 * code_convert - str code convert
 * from_charset[IN]: in str code format
 * to_charset[IN]: out str code format
 * inbuf[IN]: in str buf
 * inlen[IN]: in str len
 * outbuf[OUT]: out str buf
 * outlen[OUT]: out str len
 * return: 0 success, -1 failed
 */
static int code_convert(char *from_charset, char *to_charset,
		char *inbuf, size_t inlen, char *outbuf, size_t outlen)
{
	iconv_t cd;
	char **pin = &inbuf;
	char **pout = &outbuf;

	cd = iconv_open(to_charset, from_charset);
	if (cd == (iconv_t)-1) {
		perror("iconv_open");
		return -1;
	}
	memset(outbuf, 0, outlen);
	if (iconv(cd, pin, &inlen, pout, &outlen) == (size_t)-1) {
		perror("iconv");
		return -1;
	}
	iconv_close(cd);
	*pout = '\0';

	return 0;
}

static int asc_to_short(unsigned short *dest, char *src)
{
 	int count = 0;
    while (*src) {
		if (*src < 0x80){
	        *dest = *src;
	        dest++;
	        src++;
			count++;
		} else if (*src < 0xA0){
			src++;
		} else if (*(src + 1) < 0xA0) {
			src++;
		} else {
			*dest = ((unsigned short)(*(src + 1) << 8)) | *src;
	        dest++;
	        src += 2;
			count++;
		}
    }

	return count;
}

static int get_asc_sum(unsigned short *src, int total)
{
	unsigned short *p = src;
	int asc_sum = 0;

	/* width of asc is half of height */
	for (int i = 0; i < total; i++){
        if (*(p + i) < 0x80)
          	asc_sum ++;
    }

	return asc_sum;
}

/**
 * format_osd_date_time - format osd date and time info
 * osd_time[IN]: current osd time
 * osd_ex[IN]: osd config info
 * osd_str[OUT]: formated osd string
 * return: osd real len, -1 failed
 */
static int format_osd_date_time(const time_t *osd_time, struct osd_ex_info *osd_ex,
								unsigned short *osd_str)
{
  	int osd_len = 0;
	unsigned short year[4], month[2], day[2], hour[2], min[2], second[2];
/* 标准的显示汉字，包括"星期一二三四五六天年月日" */
/* gb2312 , 汉字    区位码  编码:
	星4839	0xd0c7 期3858  0xc6da
	一5027 0xd2bb 二2294 0xb6fe 三4093 0xc8fd 四4336 0xcbc4
	五4669 0xcee5 六3389 0xc1f9 天4476 0xccec
	年3674 0xc4ea 月5234 0xd4c2 日4053 0xc8d5

	example: 3674 -> 0x244A -> 0x244A + 0xa0a0 -> 0xc4ea
*/
	unsigned short week_ch[3]= {0xc7d0, 0xdac6, 0};
	unsigned short year_gb2312 = 0xeac4, mon_gb2312 = 0xc2d4, day_gb2312 = 0xd5c8;
	unsigned short week_day[] = {0xeccc, 0xbbd2, 0xfeb6, 0xfdc8, 0xc4cb, 0xe5ce, 0xf9c1};
	unsigned short week_day_en[7][4] = {
    	{'S', 'u', 'n', 'n'},
        {'M', 'o', 'n', ' '},
        {'T', 'u', 'e', 's'},
        {'W', 'e', 'd', ' '},
        {'T', 'h', 'u', 'r'},
        {'F', 'r', 'i', ' '},
        {'S', 'a', 't', ' '},
    };
	char tmp[50];

	struct tm tt;
	/* localtime_r is safe for thread */
	localtime_r(osd_time, &tt);

	sprintf(tmp, "%d", tt.tm_year+1900);
	asc_to_short(year, tmp);
	sprintf(tmp, "%02d", tt.tm_mon+ 1);
	asc_to_short(month, tmp);
	sprintf(tmp, "%02d", tt.tm_mday);
	asc_to_short(day, tmp);
	sprintf(tmp, "%02d", tt.tm_hour);
	asc_to_short(hour, tmp);
	sprintf(tmp, "%02d", tt.tm_min);
	asc_to_short(min, tmp);
	sprintf(tmp, "%02d", tt.tm_sec);
	asc_to_short(second, tmp);

	switch (osd_ex->date_format)
	{
	case 1:
		//YYYY-MM-DD
	        memcpy(osd_str + osd_len, year, sizeof(year));
	        osd_len += 4;
	        osd_str[osd_len] ='-';
	        osd_len ++;
	        memcpy(osd_str + osd_len, month, sizeof(month));
	        osd_len += 2;
	        osd_str[osd_len] ='-';
	        osd_len ++;
	        memcpy(osd_str + osd_len, day, sizeof(day));
	        osd_len += 2;
	        osd_str[osd_len] =' ';
	        osd_len ++;
		break;
	case 2:
		//MM-DD-YYYY
	        memcpy(osd_str + osd_len, month, sizeof(month));
	        osd_len += 2;
	        osd_str[osd_len] ='-';
	        osd_len ++;
	        memcpy(osd_str + osd_len, day, sizeof(day));
	        osd_len += 2;
	        osd_str[osd_len] ='-';
	        osd_len ++;
	        memcpy(osd_str + osd_len, year, sizeof(year));
	        osd_len += 4;
	        osd_str[osd_len] =' ';
	        osd_len ++;
		break;
	case 3:
		//YYYY MM DD
	        memcpy(osd_str + osd_len, year, sizeof(year));
	        osd_len += 4;
	        osd_str[osd_len] = year_gb2312;
	        osd_len ++;
	        memcpy(osd_str + osd_len, month, sizeof(month));
	        osd_len += 2;
	        osd_str[osd_len] = mon_gb2312;
	        osd_len ++;
	        memcpy(osd_str + osd_len, day, sizeof(day));
	        osd_len += 2;
	        osd_str[osd_len] =day_gb2312;
	        osd_len ++;
	        osd_str[osd_len] =' ';
	        osd_len ++;
		break;
	case 4:
		//MM DD YYYY
	        memcpy(osd_str + osd_len, month, sizeof(month));
	        osd_len += 2;
	        osd_str[osd_len] = mon_gb2312;
	        osd_len ++;

		memcpy(osd_str + osd_len, day, sizeof(day));
	   	osd_len += 2;
		osd_str[osd_len] =day_gb2312;
	   	osd_len ++;

        memcpy(osd_str + osd_len, year, sizeof(year));
	        osd_len += 4;
	        osd_str[osd_len] = year_gb2312;
		osd_str[osd_len] =' ';
	   	osd_len ++;
		break;
	case 5:
		//DD-MM-YYYY
	        memcpy(osd_str + osd_len, day, sizeof(day));
	        osd_len += 2;
	        osd_str[osd_len] ='-';
	        osd_len ++;
	        memcpy(osd_str + osd_len, month, sizeof(month));
	        osd_len += 2;
	        osd_str[osd_len] ='-';
	        osd_len ++;
	        memcpy(osd_str + osd_len, year, sizeof(year));
	        osd_len += 4;
	        osd_str[osd_len] =' ';
	        osd_len ++;
		break;
	case 6:
		//DD MM YYYY
	        memcpy(osd_str + osd_len, day, sizeof(day));
	        osd_len += 2;
		 memcpy(osd_str + osd_len, (char *)&day_gb2312, sizeof(day_gb2312));
	        osd_len ++;
	        memcpy(osd_str + osd_len, month, sizeof(month));
	        osd_len += 2;
	        memcpy(osd_str + osd_len,  (char *)&mon_gb2312, sizeof(mon_gb2312));
	        osd_len ++;
	        memcpy(osd_str + osd_len, year, sizeof(year));
	        osd_len += 4;
	        memcpy(osd_str + osd_len,  (char *)&year_gb2312, sizeof(year_gb2312));
	        osd_len ++;
	        osd_str[osd_len] = ' ';
	        osd_len ++;
		break;
	default:
		break;
	}

	/* hour:min:second */
	if (osd_ex->hour_format) {
		memcpy(osd_str + osd_len, hour, sizeof(hour));
		osd_len += 2;
		osd_str[osd_len] =':';
		osd_len ++;
		memcpy(osd_str + osd_len, min, sizeof(min));
		osd_len += 2;
		osd_str[osd_len] =':';
		osd_len ++;
		memcpy(osd_str + osd_len, second, sizeof(second));
		osd_len += 2;
		osd_str[osd_len] =' ';
		osd_len ++;
	}

    if (osd_ex->week_format) {
        if(osd_ex->week_format == 1) {
            //chinese
            week_ch[2] = week_day[tt.tm_wday];
            memcpy(osd_str + osd_len, week_ch, sizeof(week_ch));
            osd_len += 3;
        } else {
            //english
            memcpy(osd_str + osd_len, week_day_en[tt.tm_wday], 8);
            osd_len += 4;
        }
    }

    return osd_len;
}

/**
 * osd_disp_stat - display real time fps, bps, goplen 
 * channel[IN]: 0 main , 1 sub
 * osd_ex[IN]: osd config info
 * return: 0 success, -1 failed
 */
static int osd_disp_stat(int channel, struct osd_ex_info *osd_info)
{
	static unsigned short osd_len_bak = 0;
	unsigned short osd_gb2312code_name[OSD_STAT_LEN] = {0};
	unsigned short osd_len = 0;
	int x_adjust = 0;
	int x_start = 0;

	osd_len = strlen(osd_stat[channel].stat_str);
	asc_to_short(osd_gb2312code_name,osd_stat[channel].stat_str);
	if (osd_len <= 0) {
		ak_print_notice_ex("stat_str empty!\n");
		return AK_FAILED;
	}

	if(osd_info->on_right_side[OSD_STAT_RECT]){
		x_adjust =	osd_info->osd_disp_size[channel] * osd_len / 2;
		x_start = osd_info->x_width[channel] - x_adjust;
	}
	else
		x_start = osd_info->osd_disp_size[channel];
	/* we need to clean the old string*/
	if (osd_len_bak > osd_len){
		//ak_osd_clean_str(channel, OSD_STAT_RECT, 0, 0, rect_info[channel][OSD_STAT_RECT].width,
			//rect_info[channel][OSD_STAT_RECT].height);

		int num = osd_len_bak - osd_len;
		for (int i=0; i<num; i++) {
			osd_gb2312code_name[osd_len + i] = 0x20;	//add space to clean the old string
		}
		osd_len = osd_len_bak;
		if(osd_info->on_right_side[OSD_STAT_RECT]){
			x_adjust =	osd_info->osd_disp_size[channel] * osd_len / 2;
			x_start = osd_info->x_width[channel] - x_adjust;
		}
	}
	if (ak_osd_draw_str(channel, OSD_STAT_RECT, x_start, 0,
		osd_gb2312code_name, osd_len) < 0) {
		ak_print_error_ex("draw_osd_str fail!\n");
		return AK_FAILED;
	}
	osd_len_bak = osd_len;

	return AK_SUCCESS;
}

/**
 * osd_disp_name - display osd name
 * osd_ex[IN]: osd config info
 * return: 0 success, -1 failed
 */
static int osd_disp_name(struct osd_ex_info *osd_info)
{
	char osd_gb2312code_name[100] = {0};
	unsigned short osd_str[100]={0};
	int osd_len = 0, asc_sum = 0;
	int ret = 0;
	int x_adjust = 0;
	int x_start = 0;
	int  max_w = 0, max_h = 0, i = 0;

	ret = ak_osd_ex_u2g(osd_info->name, strlen(osd_info->name), (char *)osd_gb2312code_name, 100);
	i = strlen(osd_gb2312code_name);

	if ((ret < 0) || (i <= 0)) {
		ak_print_error_ex("utf8_to_gb2312code fail!\n");
		return AK_FAILED;
	}
	osd_len = asc_to_short(osd_str, osd_gb2312code_name);
	/*asc char only half width of chinese char*/
	asc_sum = get_asc_sum(osd_str, osd_len);
	for (int channel = 0; channel < OSD_EX_VIDEO_CH; channel++){
		if(osd_info->on_right_side[OSD_NAME_RECT]){
			x_adjust =   osd_info->osd_disp_size[channel] * (2 * osd_len - asc_sum) / 2;
			x_start = osd_info->x_width[channel] - x_adjust;
		}
		else
			x_start = osd_info->osd_disp_size[channel];

		ret	= ak_osd_get_max_rect(channel, &max_w, &max_h);
		ak_osd_clean_str(channel, OSD_NAME_RECT, 0, 0, max_w, osd_info->osd_disp_size[channel]);

		if (ak_osd_draw_str(channel, OSD_NAME_RECT, x_start, 0,
			(unsigned short *)osd_str, osd_len) < 0) {
			ak_print_error_ex("draw_osd_str fail!\n");
			return AK_FAILED;
		}
	}

	return AK_SUCCESS;
}

/**
 * osd_disp_date_time - display osd date and time
 * osd_ex[IN]: osd config info
 * return: 0 success, -1 failed
 */
static int osd_disp_date_time(struct osd_ex_info *osd_info)
{
	unsigned short osd_str[100];
	time_t osd_time;
	int osd_len = 0, asc_sum = 0;
	int x_adjust = 0;
	int x_start = 0;

	if (0 == osd_info->position[OSD_TIME_RECT]){
		ak_print_info_ex("osd time off\n");
		return AK_SUCCESS;
	}
	/*
	 * the date and times will change frequently,
	 * so we update it by a specified interval time.
	 */
	time(&osd_time);
	if(osd_time == osd_info->time_bak){
		return AK_SUCCESS;
	}else{
		osd_info->time_bak = osd_time;
	}
	memset(osd_str, 0, sizeof(osd_str));
	osd_len = format_osd_date_time(&osd_time, osd_info, osd_str);
	/*asc char only half width of chinese char*/
	asc_sum = get_asc_sum(osd_str, osd_len);
	/* draw string osd on channel 0 && channel 1 */
	for (int channel = 0; channel < OSD_EX_VIDEO_CH; channel++){
		if(osd_info->on_right_side[OSD_TIME_RECT]){
			x_adjust =  osd_info->osd_disp_size[channel] * (2 * osd_len - asc_sum) / 2;
			x_start = osd_info->x_width[channel] - x_adjust;
		}
		else
			x_start = osd_info->osd_disp_size[channel];

		if (osd_contrl.osd_on_off[channel] && (ak_osd_draw_str(channel, OSD_TIME_RECT, x_start,
			0, osd_str, osd_len) < 0)) {
			ak_print_error_ex("draw_osd_str fail!\n");
		}
	}
	//ak_print_error_ex("%s\n",osd_str);

	return AK_SUCCESS;
}

/**
 * set_osd_buffer - set osd buff
 * @vi_handle[IN]: opened vi handle
 * osd_info[IN]: osd struct 
 * return: 0, success; -1, failed.
 */
static int set_osd_buffer(void *vi_handle,struct osd_ex_info *osd_info)
{
	int channel = 0;

	for (channel = 0; channel < OSD_EX_VIDEO_CH; channel++) {
		int width = 0;
		int height = 0;
		int y_start = 0;
		int x_start = 0;
		int *p_width = NULL;
		int max_w, max_h;

		if (ak_osd_get_max_rect(channel, &max_w, &max_h) < 0) {
			ak_print_error_ex("chn=%d, ak_osd_get_max_rect failed\n", channel);
			return AK_FAILED;
		}
		ak_print_info_ex("chn=%d, ak_osd_get_max_rect OK, max_w: %d max_h: %d\n",
			channel, max_w, max_h);

		height =  osd_info->osd_disp_size[channel];

		/*notice to display resolution and osd rect limit*/
		p_width =  &(osd_info->x_width[channel]);
		//ak_print_normal_ex("x_width: %d\n", *p_width);

		for (int i= 0; i < OSD_MAX_RECT; i++){
			y_start = height * i;
			x_start = 0;
			switch (osd_info->position[i]) {
				case 1:	//leftdown
					y_start = max_h - height * (OSD_MAX_RECT - i);
				case 2:	//leftup
					break;

				case 4: //rightdown
					y_start = max_h - height * (OSD_MAX_RECT - i);
				case 3:	//rightup
					osd_info->on_right_side[i]	= 1;
					if (*p_width > max_w){
						x_start = *p_width - max_w;
					}
					break;
				default:
					break;
			}

			width = (*p_width > max_w)? max_w: *p_width;

			if (ak_osd_set_rect(vi_handle, channel, i, x_start, y_start, width, height) < 0) {
				ak_print_error_ex("chn=%d,rect:%d ak_osd_set_rect failed\n", channel, i);
				return AK_FAILED;
			}
			rect_info[channel][i].width = width;
			rect_info[channel][i].height = height;
		}
		if (*p_width > max_w){
			*p_width = max_w;
		}
		//ak_print_normal("chn=%d, ak_osd_set_rect OK\n", channel);
	}

	return AK_SUCCESS;
}

/**
 * init_osd - init osd
 * vi[IN]: opened vi handle
 * osd_info[IN]: osd struct 
 * return: 0 success, -1 failed
 */
static int init_osd(void * vi, struct osd_ex_info *osd_info)
{
	void *vi_handle = vi;

	if (ak_osd_init(vi_handle) < 0) {
		ak_print_error("ak_osd_init failed\n");
		return -1;
	}
	ak_osd_set_color(WHITE_COLOR_TABLE_INDEX, CRYSTAL_CLEAR_COLOR_TABLE_INDEX);
	ak_osd_set_edge_color(BLACK_COLOR_TABLE_INDEX);
	ak_osd_set_font_size(0, osd_info->osd_disp_size[0]);
	ak_osd_set_font_size(1, osd_info->osd_disp_size[1]);
	/* set osd rect buff for two channel */
	if (set_osd_buffer(vi_handle, osd_info) < 0) {
		ak_print_error("set_osd_buffer failed\n");
		ak_osd_destroy();
		return -1;
	}

	return 0;
}
/* osd_thread -  osd thread, draw osd */
static void *osd_thread(void *arg)
{
	void *vi_handle = arg;

	ak_thread_set_name("ak_osd");

	ak_print_normal_ex("thread id: %ld\n", ak_thread_get_tid());

   /* init osd file */
	ak_osd_set_font_file(OSD_FILE_FONT_SIZE, OSD_EX_FONT_BIN);

	if (init_osd(vi_handle, &osd_info) < 0){
		ak_print_error_ex("init_osd failed!\n");
		goto AK_OSD_END;
	}

	ak_print_info_ex("name_switch:%d!\n",osd_info.position[OSD_NAME_RECT]);
	if (osd_info.position[OSD_NAME_RECT] > 0) {
		ak_print_info_ex("osd_disp_name!\n");
		osd_disp_name(&osd_info);
	}

	while (osd_contrl.run_flag) {
		if (osd_disp_date_time(&osd_info) < 0)
			break;
		if (osd_contrl.osd_name_update) {
			osd_disp_name(&osd_info);
			osd_contrl.osd_name_update = 0;
		}
		for (int i=0; i < 2; i++){
			if (osd_contrl.video_stat_update[i]) {
				osd_disp_stat(i, &osd_info);
				osd_contrl.video_stat_update[i] = 0;
			}
		}
		ak_sleep_ms(100);
	}

	/* resources release */
	ak_osd_destroy();

AK_OSD_END:
	ak_print_normal_ex("thread tid: %ld exit\n", ak_thread_get_tid());
	ak_thread_exit();

	return NULL;
}

/**
 * ak_osd_ex_get_version - get osd ex version
 * return: version string
 */
const char* ak_osd_ex_get_version(void)
{
	return osd_ex_version;
}

/**
 * ak_osd_ex_u2g - utf8 str convert to gbk str
 * inbuf[IN]: utf8 str buf
 * inlen[IN]: utf8 str len
 * outbuf[OUT]: gbk str buf
 * outlen[OUT]: gbk str len
 * return: 0 success, -1 failed
 */
int ak_osd_ex_u2g(char *inbuf, size_t inlen, char *outbuf, size_t outlen)
{
	return code_convert("utf-8", "gbk", inbuf, inlen, outbuf, outlen);
}

/**
 * ak_osd_ex_g2u - gbk str convert to utf8 str
 * inbuf[IN]: gbk str buf
 * inlen[IN]: gbk str len
 * outbuf[OUT]: utf8 str buf
 * outlen[OUT]: utf8 str len
 * return: 0 success, -1 failed
 */
int ak_osd_ex_g2u(char *inbuf, size_t inlen, char *outbuf, size_t outlen)
{
	return code_convert("gbk", "utf-8", inbuf, inlen, outbuf, outlen);
}

/**
 * ak_osd_ex_stat_video - display video stat info via osd
 * channel[IN]: channel no.
 * stream_handle[IN]: stream handle
 * return: 0 success, -1 failed
 */
int ak_osd_ex_stat_video(int channel, void *stream_handle)
{
	if (!stream_handle) {
		ak_print_error_ex("param failed\n");
		return AK_FAILED;
	}
	if (!osd_contrl.init_flag) {
		ak_print_info_ex("not init osd\n");
		return AK_SUCCESS;
	}
	if (0 == osd_contrl.osd_on_off[channel]){
		ak_print_info_ex("channel %d osd off\n", channel);
		return AK_SUCCESS;
	}
	if (0 == osd_info.position[OSD_STAT_RECT]){
		ak_print_info_ex("osd stat off\n");
		return AK_SUCCESS;
	}

    struct venc_rate_stat stat = {0};

    ak_venc_get_rate_stat(stream_handle, &stat);

	if ((stat.bps < 0) || (stat.bps > 99999))
		stat.bps = 0;

	if ((stat.gop < 0) || (stat.gop > 999))
		stat.gop = 0;

	if ((stat.fps < 0) || (stat.fps > 99))
		stat.fps = 0;
	
    if ((stat.bps != osd_stat[channel].bps)
        || (stat.fps != osd_stat[channel].fps)
        || (stat.gop != osd_stat[channel].gop_len)) {
		struct video_channel_attr cur_attr;
		memset(&cur_attr, 0x00, sizeof(cur_attr));
		if (ak_vi_get_channel_attr(osd_contrl.vi_handle, &cur_attr)) {
			ak_print_normal("ak_vi_get_channel_attr failed!\n");
		}

		snprintf(osd_stat[channel].stat_str, 49, "CH%d %d*%d BR:%4d GOP:%d FPS:%3.1f",
			channel,cur_attr.res[channel].width, cur_attr.res[channel].height,
			stat.bps, stat.gop, stat.fps);
    	osd_contrl.video_stat_update[channel] = 1;

		/* incase of gop changed, so we get gop again */
		osd_stat[channel].bps = stat.bps;
        osd_stat[channel].fps = stat.fps;
        osd_stat[channel].gop_len = stat.gop;
	}

	return AK_SUCCESS;
}

/**
 * ak_osd_ex_update_name - update osd name on screen
 * @osd_name[IN]: osd name to be update
 * return: 0 - success, fail return -1.
 */
int ak_osd_ex_update_name(char *osd_name)
{
	if (!osd_name || (strlen(osd_name) < 1)) {
		ak_print_error_ex("para failed\n");
		return AK_FAILED;
	}
	if (!osd_contrl.init_flag) {
		ak_print_info_ex("not init osd\n");
		return AK_SUCCESS;
	}
	if (0 == osd_info.position[OSD_NAME_RECT]){
		ak_print_info_ex("osd name off\n");
		return AK_SUCCESS;
	}
	memset(osd_info.name, 0, sizeof(osd_info.name));
	strncpy(osd_info.name, osd_name, 49);
	osd_contrl.osd_name_update = 1;

	return AK_SUCCESS;
}

/**
 * ak_osd_ex_turn_on - turn on to draw osd on screen
 * @channel[IN]: video channel
 * return: 0 - success, fail return -1.
 */
int ak_osd_ex_turn_on(int channel)
{
	if ((channel < 0) || (channel > 1)) {
		ak_print_error_ex("para failed\n");
		return AK_FAILED;
	}
	osd_contrl.osd_on_off[channel] = 1;

	return AK_SUCCESS;
}

/**
 * ak_osd_ex_turn_off - turn off to draw osd on screen
 * @channel[IN]: video channel
 * return: 0 - success, fail return -1.
 */
int ak_osd_ex_turn_off(int channel)
{
	if ((channel < 0) || (channel > 1)) {
		ak_print_error_ex("para failed\n");
		return AK_FAILED;
	}
	osd_contrl.osd_on_off[channel] = 0;

	return AK_SUCCESS;
}

/**
 * ak_osd_ex_init - create thread to draw osd to screen
 * @vi[IN]: vi handle
 * return: 0 - success, fail return -1.
 */
int ak_osd_ex_init(void *vi, struct osd_ex_info *info)
{
	/* init  osd if needed */
	if (osd_contrl.init_flag) {
		return 0;
	}
	memset(rect_info, 0, sizeof(struct osd_rect_info) * VIDEO_CHN_NUM * OSD_MAX_RECT );
	osd_contrl.vi_handle = vi;
	/* get camera information */
	memcpy(&osd_info, info, sizeof(struct osd_ex_info));
	ak_print_info_ex("osd size.main:%d sub:%d!\n", osd_info.osd_disp_size[0],
			osd_info.osd_disp_size[1]);

	osd_contrl.init_flag = 1;
	osd_contrl.run_flag = 1;
	/* create osd thread to update osd */
	ak_thread_create(&osd_contrl.tid, osd_thread,
		vi, 100*1024, -1);

	return 0;
}

/**
 * ak_osd_ex_exit - exit osd thread
 * return: void.
 */
void ak_osd_ex_exit(void)
{
	if (osd_contrl.init_flag){
		osd_contrl.run_flag = 0;
		ak_thread_join(osd_contrl.tid);
		osd_contrl.init_flag = 0;
	}
}

