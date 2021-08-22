#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>

#include "list.h"
#include "isp_main.h"
#include "isp_basic.h"
#include "isp_cfg_file.h"
#include "isp_vi.h"
#include "internal_error.h"
#include "akuio.h"

#include "ak_common.h"
#include "ak_thread.h"
#include "ak_vi.h"
#include "vi_ipcsrv.h"

#define PLAT_VI                     "<plat_vi>"

/* use for calculate main channel frame size */
#define CH_MAIN_SIZE(x,y)           (((x) * (y)) * 3 / 2)
/* use for calculate sub channel frame size */
#define CH_SUB_SIZE(x,y)            (((x) * (y)) * 3 / 2)
/* use for calculate sub channel frame size */
#define CH_CH3_SIZE(x,y)            (((x) * (y)) * 3 / 2)
/* move count info array size: 24*32*2 */
#define MOVE_CNT_INFO_SZ            (VPSS_MD_DIMENSION_H_MAX*VPSS_MD_DIMENSION_V_MAX*2)

#define BUFFER_RESERVE_BYTES		(128)

/* use for dealwith multiple user */
#define VI_DEV_MAX_USER	            (sizeof(int) * 8)
/* use for debug get frame time interval */
#define CALC_VI_GET_FRAME_TIME	    (0)
/* user space can get max frame(s) at same time */
#define USER_SPACE_MAX_FRAMES       (2)
/* use for debug move count info */
#define DEBUG_MD_INFO               (0)

/* chip supported max and min resolution of main channel */
/* current main channel max width */
//#define CHIP_MAIN_CHN_MAX_WIDTH 	(1920)
/* current main channel max height */
//#define CHIP_MAIN_CHN_MAX_HEIGHT 	(1080)
/* current sub channel max width */
#define CHIP_MAIN_CHN_MIN_WIDTH 	(640)
/* current sub channel max height */
#define CHIP_MAIN_CHN_MIN_HEIGHT 	(480)

/* chip supported max and min resolution of sub channel */
/* sub channel max width subject to chip */
//#define CHIP_SUB_CHN_MAX_WIDTH 		(640)
/* sub channel max height subject to chip */
//#define CHIP_SUB_CHN_MAX_HEIGHT 	(480)

/* ch3 channel max width subject to chip */
#define CHIP_CH3_CHN_MAX_WIDTH 		(320)
/* ch3 channel max height subject to chip */
#define CHIP_CH3_CHN_MAX_HEIGHT 	(240)

/* sub channel min width subject to chip */
#define CHIP_SUB_CHN_MIN_WIDTH 		(18)
/* sub channel min width subject to chip */
#define CHIP_SUB_CHN_MIN_HEIGHT 	(18)

#define ALIGN(x,a)      __ALIGN_MASK(x,(__typeof__(x))(a)-1)
#define __ALIGN_MASK(x,mask)    (((x)+(mask))&~(mask))

/* 
 * the video encoder IP require that 
 * the width and height of video resolution must align as follow:
 */
#define VENCODER_WIDTH_ALIGN_REQ	(32)
#define VENCODER_HEIGHT_ALIGN_REQ	(8)

struct ak_frame {
	int ref;		    	//reference counter of this struct
	int bit_map;			//marks which user use it
	void *private_data; 	//v4l2 buffer begin address
	unsigned char *buf;     //YUV data buffer begin address
	unsigned int main_len;  //main channel buffer len in bytes
	unsigned int sub_len;   //sub channel buffer len in bytes
	unsigned int ch3_len;   //ch3 channel buffer len in bytes
	unsigned long long ts;  //timestamp(ms)
	unsigned long seq_no;	//current frame sequence no.
	struct ak_timeval get_time;	//get frame time
	struct list_head list;	//frames list
};

struct user_t {
	int dev_num; 			//video input device number
	int user_flg; 			//video input device number
	struct list_head list;	//list head, hang to device's user_list
	struct vi_device *pdev;
};


struct vi_device {
	int user_ref;		//device user reference
	int user_map;		//open user bit map
	int capture_on;		//capture on/off flag
	int frame_count;	//current user space frame counter
	int dev_num;		//current device number
	void *isp_mem;		//user for 3D

	struct video_resolution *chn_main;	//main channel resource
	struct video_resolution *chn_sub;	//sub channel resource
	struct crop_info *crop_info;		//crop information
	struct list_head frame_list;  //store frames
	struct list_head dev;	      //hang to video_input_handle's dev_list
	struct list_head user_list;	  //if each dev has only 1 user, it is no use

	struct ak_timeval pre_get_time;	//previous get frame time
	ak_mutex_t vi_lock;		//this module's mutex lock
	ak_mutex_t frame_lock;	//frame list operate mutex lock

	struct box_info {

		video_rect box_attr;
	    unsigned char luma;
		unsigned char chroma_u;
		unsigned char chroma_v;

	} box_info[VIDEO_CHN_NUM];

};

/* module global control handle */
struct video_input_handle {
	unsigned char dma_flag;	//dma inited flag
	int dev_count;			//current module load device couter number
	ak_mutex_t lock;		//device add or delete or global resource operate lock
	struct list_head dev_list;	//device list
};

/*box of motion rect*/
struct _box
{
  int left;
  int top;
  int width;
  int height;
};

static struct _box box_rect = {0};
static ak_mutex_t box_lock = PTHREAD_MUTEX_INITIALIZER;


static const char vi_version[] = "libplat_vi V1.0.05";

/* define global control handle instance */
static struct video_input_handle vi_ctrl = {
	.dma_flag = 0,
	.dev_count = 0,
	.lock = PTHREAD_MUTEX_INITIALIZER,
	.dev_list = LIST_HEAD_INIT(vi_ctrl.dev_list)
};


/**
 * draw_box - draw a rectangle box on the frame
 * @frame[IN]: frames
 * @luma[IN]: luma of the drawing lines
 * @vid_height: video height
 * @vid_width: video width
 * @box_x: the horizontal position of left top point of the drawing box
 * @box_y: the vertical position of left top point of the drawing box
 * @box_height: the height of drawing box
 * @box_width: the width of drawing box
 * @line_size: the size of drawing lines
 * return: 0 success, -1 failed
 * notes:
 */
static int draw_box(struct video_input_frame *frame, int channel,unsigned char luma,unsigned char chroma_u,unsigned char chroma_v,
	int vid_height, int vid_width, int box_x, int box_y, int box_height, int box_width, int line_size)
{
	int i, j; // code_updating	
    
	if ((vid_height != 0) && (vid_width != 0) && (box_height != 0) && (box_width != 0)
		&& (line_size != 0))		
	{	
	    if(box_x + box_width + line_size > vid_width)
	    {
	        box_width = vid_width - box_x - line_size;
	    }

	    if(box_y + box_height + line_size > vid_height)
	    {
	        box_height = vid_height - box_y - line_size;
	    }
	    
	    if(channel == VIDEO_CHN_MAIN)
	    {
            for(i = box_x; i <box_x + box_width; i++)
            {
                for(j=box_y; j<box_y+line_size; j++) 
                {
                    frame->vi_frame[VIDEO_CHN_MAIN].data[i+j*vid_width] = luma;
                    frame->vi_frame[VIDEO_CHN_MAIN].data[(i/2*2)+j/2*vid_width+vid_height*vid_width] = chroma_u;
                    frame->vi_frame[VIDEO_CHN_MAIN].data[(i/2*2+1)+j/2*vid_width+vid_height*vid_width] = chroma_v;
                }
            }

            for(i = box_x; i <box_x + box_width; i++)
            {
                for(j=box_y+box_height; j<box_y+box_height+line_size; j++) 
                {
                    frame->vi_frame[VIDEO_CHN_MAIN].data[i+j*vid_width] = luma;
                    frame->vi_frame[VIDEO_CHN_MAIN].data[(i/2*2)+j/2*vid_width+vid_height*vid_width] = chroma_u;
                    frame->vi_frame[VIDEO_CHN_MAIN].data[(i/2*2+1)+j/2*vid_width+vid_height*vid_width] = chroma_v;
                }
            }


            for (i = box_x; i < box_x + line_size; i++)  // code_updating           
            {                   
                for (j = box_y; j < box_y + box_height; j++)                
                {                   
                    frame->vi_frame[VIDEO_CHN_MAIN].data[j * vid_width + i] = luma;
                    frame->vi_frame[VIDEO_CHN_MAIN].data[(i/2*2)+j/2*vid_width+vid_height*vid_width] = chroma_u;
                    frame->vi_frame[VIDEO_CHN_MAIN].data[(i/2*2+1)+j/2*vid_width+vid_height*vid_width] = chroma_v;
                }               
            }   

            for (i = box_x + box_width - line_size; i < box_x + box_width; i++)  // code_updating       
            {                   
                for (j = box_y; j < box_y + box_height ; j++)       
                {           
                    frame->vi_frame[VIDEO_CHN_MAIN].data[j * vid_width + i] = luma;
                    frame->vi_frame[VIDEO_CHN_MAIN].data[(i/2*2)+j/2*vid_width+vid_height*vid_width] = chroma_u;
                    frame->vi_frame[VIDEO_CHN_MAIN].data[(i/2*2+1)+j/2*vid_width+vid_height*vid_width] = chroma_v;
                }               
            }   

        }else if(channel == VIDEO_CHN_SUB){

            for(i = box_x; i <box_x + box_width; i++)
            {
                for(j=box_y; j<box_y+line_size; j++) 
                {
                    frame->vi_frame[VIDEO_CHN_SUB].data[i+j*vid_width] = luma;
                    frame->vi_frame[VIDEO_CHN_SUB].data[(i/2*2)+j/2*vid_width+vid_height*vid_width] = chroma_u;
                    frame->vi_frame[VIDEO_CHN_SUB].data[(i/2*2+1)+j/2*vid_width+vid_height*vid_width] = chroma_v;
                }
            }
            
            for(i = box_x; i <box_x + box_width; i++)
            {
                for(j=box_y+box_height; j<box_y+box_height+line_size; j++) 
                {
                    frame->vi_frame[VIDEO_CHN_SUB].data[i+j*vid_width] = luma;
                     frame->vi_frame[VIDEO_CHN_SUB].data[(i/2*2)+j/2*vid_width+vid_height*vid_width] = chroma_u;
                    frame->vi_frame[VIDEO_CHN_SUB].data[(i/2*2+1)+j/2*vid_width+vid_height*vid_width] = chroma_v;
                }
            }

		
    		for (i = box_x; i < box_x + line_size; i++)  // code_updating			
    		{					
    			for (j = box_y; j < box_y + box_height; j++)				
    			{					
    				frame->vi_frame[VIDEO_CHN_SUB].data[j * vid_width + i] = luma;
    				frame->vi_frame[VIDEO_CHN_SUB].data[(i/2*2)+j/2*vid_width+vid_height*vid_width] = chroma_u;
                    frame->vi_frame[VIDEO_CHN_SUB].data[(i/2*2+1)+j/2*vid_width+vid_height*vid_width] = chroma_v;
    			}				
    		}	
    		
    		for (i = box_x + box_width - line_size; i < box_x + box_width; i++)  // code_updating		
    		{					
    			for (j = box_y; j < box_y + box_height ; j++)		
    			{			
    				frame->vi_frame[VIDEO_CHN_SUB].data[j * vid_width + i] = luma;
    				frame->vi_frame[VIDEO_CHN_SUB].data[(i/2*2)+j/2*vid_width+vid_height*vid_width] = chroma_u;
                    frame->vi_frame[VIDEO_CHN_SUB].data[(i/2*2+1)+j/2*vid_width+vid_height*vid_width] = chroma_v;
    			}				
    		}	
    	}
		return 0;
	}
	else
	{
		return -1;
	}
}

/*
 * do_match: do acctually match things
 * config_file[IN]:  config file's absolutely path
 * return: if mathc ok, 0; if fail , -1
 */
static int do_match(const char *config_file)
{
	int ret = AK_FAILED;
	if (ak_check_file_exist(config_file)) {
		ak_print_error_ex(PLAT_VI "file: %s not exist\n", config_file);
		return ret;
	}

	/* do true match */
	ak_thread_mutex_lock(&vi_ctrl.lock);
	ret = isp_main_match_sensor_cfgfile(config_file);
	ak_thread_mutex_unlock(&vi_ctrl.lock);

	return ret;
}

/*
 * match_all_config: get all config to match
 * path[IN]: config store path
 * return: if mathc ok, 0; if fail , -1
 */
static int match_all_config(const char *path)
{
	int match_ok = AK_FAILED;
	struct dirent *dir_ent = NULL;
	char tmpstr[8] = {0};
	int dvp_flag = 0;
	char *tmp = NULL;
	char *isp_cfg = NULL;
	char sensor_if[8] = {0};
	
	DIR *dir = opendir(path);
	if (!dir) {
		ak_print_error_ex(PLAT_VI "opendir: %s\n", strerror(errno));
		return match_ok;
	}

	/* last slash / deal with */
	char dir_name[100] = {0};
	if ((path + strlen(path) - 1) != strrchr(path, '/'))
		sprintf(dir_name, "%s/", path);
	else
		sprintf(dir_name, "%s", path);

	FILE *fp = fopen(SENSOE_IF_PATH, "r");
	if ((!fp) && (errno == ENOENT)) {
		ak_print_error_ex(PLAT_VI "cannot get sensor if, check your camera driver\n");
		closedir(dir);
		return match_ok;
	}

	fread(sensor_if, sizeof(sensor_if), 1, fp);
	fclose(fp);

	if (strstr(sensor_if, "mipi1"))
		strcpy(tmpstr, "mipi_1");
	else if (strstr(sensor_if, "mipi2"))
		strcpy(tmpstr, "mipi_2");
	else 
		dvp_flag = 1;

	ak_print_normal_ex(PLAT_VI "sensor_if:%s\n", sensor_if);

	/* traversal */
	while (NULL != (dir_ent = readdir(dir))) {
		if (NULL == dir_ent->d_name)
			continue;
		/* fine next when we get dir */
		if ((dir_ent->d_type & DT_DIR))
			continue;

		/* make sure use isp_*.conf file to match */
		tmp = strstr(dir_ent->d_name, "isp_");
		if (!tmp) {
			continue;
		}

		if ((!dvp_flag && !strstr(tmp, tmpstr))
			|| (dvp_flag && strstr(tmp, "mipi")))
			continue;
			

		/* we match the subfix */
		isp_cfg = strstr(tmp, ".conf");
		if (isp_cfg) {
			char absolutely_path[100] = {0};
			sprintf(absolutely_path, "%s%s", dir_name, dir_ent->d_name);

			if (do_match(absolutely_path) == AK_SUCCESS) {
				ak_print_notice_ex(PLAT_VI "%s match ok\n", absolutely_path);
				match_ok = AK_SUCCESS;
				break;
			}
		}
	}
	closedir(dir);

	return match_ok;
}

/**
 * vi_format_frame: format frame so user can use it
 * @camera_buf[IN]: the frame buffer address
 * return: 0 success, otherwise failed
 * notes:
 */
static void vi_format_frame(struct vi_device *pdev,
		const struct isp_frame *from,
		struct ak_frame *to)
{
	/* transform different format */
	to->seq_no = from->seq_no;
	to->private_data = from->private_data;
	to->buf = from->buf;
	to->ts = from->ts;
	to->get_time.sec = from->get_time.sec;
	to->get_time.usec = from->get_time.usec;

	/* channel lenth is width*height*1.5 */
	to->main_len = CH_MAIN_SIZE(pdev->chn_main->width, pdev->chn_main->height);
	to->sub_len = CH_SUB_SIZE(pdev->chn_sub->width, pdev->chn_sub->height);
	to->ch3_len = 0;//CH_CH3_SIZE(pdev->chn_sub->width >> 1, pdev->chn_sub->height >> 1);
}

/**
 * vi_to_channel2: get stream info for ch1 and ch2
 * @frame[OUT]: channel main information
 * @entry[IN]: the info for YUV frame
 * return: 0 success, otherwise failed
 * notes:
 */
static int vi_to_channel2(struct video_input_frame *frame,
		const struct ak_frame *entry)
{
	/* internal arguments check */
	if (NULL == entry || NULL == frame) {
		return AK_FAILED;
	}

	/* drop error frame */
	if ((0 == entry->ts) || (0 == entry->main_len) || (0 == entry->sub_len)) {
		ak_print_warning_ex(PLAT_VI "entry: %p ts=%llu main_len=%u sub_len=%u seq=%lu\n",
			entry, entry->ts, entry->main_len, entry->sub_len, entry->seq_no);
	}

	/* main channal data assignment */
	frame->vi_frame[VIDEO_CHN_MAIN].data = entry->buf;
	frame->vi_frame[VIDEO_CHN_MAIN].len = entry->main_len;
	frame->vi_frame[VIDEO_CHN_MAIN].ts  = entry->ts;
	frame->vi_frame[VIDEO_CHN_MAIN].seq_no  = entry->seq_no;

	/* sub channal data assignment */
	frame->vi_frame[VIDEO_CHN_SUB].data = entry->buf + entry->main_len;
	frame->vi_frame[VIDEO_CHN_SUB].len = entry->sub_len;
	frame->vi_frame[VIDEO_CHN_SUB].ts  = entry->ts;
	frame->vi_frame[VIDEO_CHN_SUB].seq_no  = entry->seq_no;

	/* times rollback check */
	static unsigned long long pre_ts =  0;
	if ((pre_ts > 0) && (pre_ts > entry->ts)) {
		ak_print_error_ex(PLAT_VI "frame rollback, pre_ts=%llu, entry->ts=%llu, diff=%llu\n",
				pre_ts, entry->ts, (pre_ts - entry->ts));
	}
	pre_ts = entry->ts;

	/* set mdinfo pointer */
	frame->mdinfo = entry->buf + entry->main_len + entry->sub_len + entry->ch3_len;

#if DEBUG_MD_INFO
	/* to show md data */
	static int test_cnt;
	int md_cnt = 0;
	unsigned short info[24][32];
	/* show each 5 frame to decrease print */
	if (test_cnt % 5 == 0) {
		memcpy(info, frame->mdinfo, 24*32*2);
		int i, j;
		for (i = 0; i < 24; i++) {
			for (j = 0; j < 32; j++) {
				if (info[i][j] >= 5000)	//this condition can modify if you want
					md_cnt++;
			}
		}
		ak_print_normal_ex(PLAT_VI "# md_cnt %d\n", md_cnt);
	}
	test_cnt++;
#endif

	return AK_SUCCESS;
}

/*
 * transform data to fill in to frame structure
 */
static int vi_data_to_frame(struct user_t *user, struct ak_frame *pos,
				struct video_input_frame *frame)
{
	int ret = -1;
	
	ak_print_debug(PLAT_VI "user_flg=%d, seq_no=%lu, ts=%llu\n",
		user->user_flg, pos->seq_no, pos->ts);
	/* get frame success, decrease frame's user reference */
	//clear_bit(&(pos->bit_map), user->user_flg);

	ret = vi_to_channel2(frame, pos);

	if (0 == ret)
	{		
		ak_thread_mutex_lock(&box_lock);

		draw_box(frame, VIDEO_CHN_MAIN,user->pdev->box_info[VIDEO_CHN_MAIN].luma,user->pdev->box_info[VIDEO_CHN_MAIN].chroma_u,user->pdev->box_info[VIDEO_CHN_MAIN].chroma_v, user->pdev->chn_main->height, user->pdev->chn_main->width, 
				    user->pdev->box_info[VIDEO_CHN_MAIN].box_attr.left, user->pdev->box_info[VIDEO_CHN_MAIN].box_attr.top, 
				    user->pdev->box_info[VIDEO_CHN_MAIN].box_attr.height, user->pdev->box_info[VIDEO_CHN_MAIN].box_attr.width, 4);
		  
		draw_box(frame, VIDEO_CHN_SUB,user->pdev->box_info[VIDEO_CHN_SUB].luma,user->pdev->box_info[VIDEO_CHN_SUB].chroma_u,user->pdev->box_info[VIDEO_CHN_SUB].chroma_v, user->pdev->chn_sub->height, user->pdev->chn_sub->width, 
				    user->pdev->box_info[VIDEO_CHN_SUB].box_attr.left, user->pdev->box_info[VIDEO_CHN_SUB].box_attr.top, 
				    user->pdev->box_info[VIDEO_CHN_SUB].box_attr.height, user->pdev->box_info[VIDEO_CHN_SUB].box_attr.width, 2);
				    
		draw_box(frame, VIDEO_CHN_MAIN,0, 0, 0, user->pdev->chn_main->height, user->pdev->chn_main->width, 
			box_rect.left, box_rect.top, 
			box_rect.height, box_rect.width, 4);

		box_rect.height=0;
		box_rect.width=0;
		box_rect.top=0;
		box_rect.left=0;

		ak_thread_mutex_unlock(&box_lock);
	}

	return ret;	
}

static inline void right_align_value(int *value, int align)
{
	*value = ALIGN(*value, align);
}

static int vi_check_main_chn_res_range(int width, int height, int max_width, int max_height)
{
	if (width <= 0 || height <= 0
			|| width > max_width
			|| height > max_height
			|| width < CHIP_MAIN_CHN_MIN_WIDTH
			|| height < CHIP_MAIN_CHN_MIN_HEIGHT)
		return AK_FAILED;
	return AK_SUCCESS;
}

static int vi_check_sub_chn_res_range(int width, int height, int max_width, int max_height)
{
	if (width <= 0 || height <= 0
			|| width > max_width
			|| height > max_height
			|| width < CHIP_SUB_CHN_MIN_WIDTH
			|| height < CHIP_SUB_CHN_MIN_HEIGHT)
		return AK_FAILED;
	return AK_SUCCESS;
}

static int vi_check_channel_attr_align(struct video_channel_attr *attr)
{
	/* check and align width and height */
	right_align_value(&attr->res[VIDEO_CHN_MAIN].width, 
			VENCODER_WIDTH_ALIGN_REQ);
	right_align_value(&attr->res[VIDEO_CHN_MAIN].height, 
			VENCODER_HEIGHT_ALIGN_REQ);
	right_align_value(&attr->res[VIDEO_CHN_SUB].width, 
			VENCODER_WIDTH_ALIGN_REQ);
	right_align_value(&attr->res[VIDEO_CHN_SUB].height, 
			VENCODER_HEIGHT_ALIGN_REQ);

	right_align_value(&attr->res[VIDEO_CHN_MAIN].max_width, 
			VENCODER_WIDTH_ALIGN_REQ);
	right_align_value(&attr->res[VIDEO_CHN_MAIN].max_height, 
			VENCODER_HEIGHT_ALIGN_REQ);
	right_align_value(&attr->res[VIDEO_CHN_SUB].max_width, 
			VENCODER_WIDTH_ALIGN_REQ);
	right_align_value(&attr->res[VIDEO_CHN_SUB].max_height, 
			VENCODER_HEIGHT_ALIGN_REQ);

	/* check main chn */
	if (vi_check_main_chn_res_range(attr->res[VIDEO_CHN_MAIN].width,
				attr->res[VIDEO_CHN_MAIN].height,
				attr->res[VIDEO_CHN_MAIN].max_width,
				attr->res[VIDEO_CHN_MAIN].max_height)) {
		ak_print_error_ex(PLAT_VI "main channel argument error, w: %d, h: %d\n",
			   attr->res[VIDEO_CHN_MAIN].width, attr->res[VIDEO_CHN_MAIN].height);
		return AK_FAILED;
	}
	/* check sub chn */
	if (vi_check_sub_chn_res_range(attr->res[VIDEO_CHN_SUB].width,
				attr->res[VIDEO_CHN_SUB].height, 
				attr->res[VIDEO_CHN_SUB].max_width,
				attr->res[VIDEO_CHN_SUB].max_height)) {
		ak_print_error_ex(PLAT_VI "sub channel argument error, w: %d, h: %d\n",
			   attr->res[VIDEO_CHN_SUB].width, attr->res[VIDEO_CHN_SUB].height);
		return AK_FAILED;
	}
	/* check main and sub chn */
	if (attr->res[VIDEO_CHN_SUB].width > attr->res[VIDEO_CHN_MAIN].width
			|| attr->res[VIDEO_CHN_SUB].height > attr->res[VIDEO_CHN_MAIN].height) {
		ak_print_error_ex(PLAT_VI "main channel resolution must bigger than sub's,"
				"main w: %d, h: %d, sub w: %d, h: %d\n",
				attr->res[VIDEO_CHN_MAIN].width, attr->res[VIDEO_CHN_MAIN].height,
				attr->res[VIDEO_CHN_SUB].width, attr->res[VIDEO_CHN_SUB].height);
		return AK_FAILED;
	}

	right_align_value(&attr->crop.width, VENCODER_WIDTH_ALIGN_REQ);
	right_align_value(&attr->crop.height, VENCODER_HEIGHT_ALIGN_REQ);
	if (attr->crop.width <= 0 || attr->crop.height <= 0) {
		ak_print_error_ex(PLAT_VI "crop argument error, w: %d, h: %d\n",
				attr->crop.width, attr->crop.height);
		return AK_FAILED;
	}

	return AK_SUCCESS;
}

static void vi_init_dma(void)
{
	if (!vi_ctrl.dma_flag) {
		akuio_pmem_init();
		vi_ctrl.dma_flag = 1;
	}
}

static void vi_release_dma(void)
{
	/*
	 * we still have problem at pmem finished.(TODO)
	 * we can delete it to support open and close many times.
	 */
#if 0
	if (vi_ctrl.dma_flag) {
		akuio_pmem_fini();
		vi_ctrl.dma_flag = 0;
	}
#endif
}

/* releas all frames */
static void vi_release_all_frames(struct vi_device *pdev)
{
	struct ak_frame *pos = NULL;
	struct ak_frame *n = NULL;

	list_for_each_entry_safe(pos, n, &pdev->frame_list, list) {
		/* qbuf */
		ak_print_normal_ex(PLAT_VI "########## release vi ##########\n");
		isp_vi_release_frame(pos->private_data);
		list_del(&pos->list);
		free(pos);
	}
}

static void vi_release_channel_resource(struct vi_device *pdev)
{
	if (pdev->chn_main) {
		free(pdev->chn_main);
		pdev->chn_main = NULL;
	}
	if (pdev->chn_sub) {
		free(pdev->chn_sub);
		pdev->chn_sub = NULL;
	}
	if (pdev->crop_info) {
		free(pdev->crop_info);
		pdev->crop_info = NULL;
	}
}

/**
 * vi_get_device - match device by device number
 * @dev[IN]: appointed video input device number
 * return: pointer to device on success, fail on NULL
 */
static void *vi_get_device(int dev_num)
{
	struct vi_device *pdev = NULL;

	list_for_each_entry(pdev, &vi_ctrl.dev_list, dev) {
		if (pdev->dev_num == dev_num)
			return pdev;
	}

	return NULL;
}

static void *vi_new_user(void *dev, int dev_num)
{
	struct user_t *user = (struct user_t *)calloc(1, sizeof(struct user_t));
	if (!user) {
		ak_print_error_ex(PLAT_VI "No memory\n");
		return NULL;
	}

	int i = 0;
	struct vi_device *pdev = (struct vi_device *)dev;

	for (i=0; i<VI_DEV_MAX_USER; ++i) {
		if (!test_bit(i, &(pdev->user_map))) {
			user->user_flg = i;
			break;
		}
	}
	if (i >= VI_DEV_MAX_USER) {
		free(user);
		user = NULL;
		return NULL;
	}

	ak_thread_mutex_lock(&pdev->vi_lock);
	user->pdev = pdev;
	user->dev_num = dev_num;
	set_bit(&(pdev->user_map), user->user_flg);
	add_ref(&(pdev->user_ref), 1);
	list_add_tail(&(user->list), &(pdev->user_list));
	ak_thread_mutex_unlock(&pdev->vi_lock);

	ak_print_notice_ex(PLAT_VI "now user_flg=%d, user_ref=%d\n",
		user->user_flg, pdev->user_ref);

	return user;
}

#if CALC_VI_GET_FRAME_TIME
static void vi_calc_frame_time(struct vi_device *pdev,
							struct ak_frame *new_frame,
							struct ak_timeval *ptv_start,
							struct ak_timeval *ptv_end)
{
	static unsigned long long pre_ts = 0;
	int cur_fps = isp_get_sensor_fps();
	int interval = (1000 / cur_fps);

	if (1000 % cur_fps) {
		interval += (10 - (interval % 10));
	}

	if (pdev->pre_get_time.sec > 0) {
		long diff_time = ak_diff_ms_time(ptv_end, &(pdev->pre_get_time));
		if (diff_time >= (interval + 40)) {
			ak_print_normal_ex(PLAT_VI "### get frame diff time: %ld(ms)\n", diff_time);
		}
	}
	ak_get_ostime(&(pdev->pre_get_time));

	if ((pre_ts > 0) && ((new_frame->ts - pre_ts) >= (interval * 2))) {
		ak_print_info_ex(PLAT_VI "seq_no=%lu, capture ts=%llu, diff ts=%lld\n",
			new_frame->seq_no, new_frame->ts, (new_frame->ts - pre_ts));
		ak_print_info_ex(PLAT_VI "pre_ts=%llu, cur_fps=%d, interval=%d\n\n",
			pre_ts, cur_fps, interval);
	}
	pre_ts = new_frame->ts;

	long diff_time = ak_diff_ms_time(ptv_end, ptv_start);
	if (diff_time > (interval * 2)) {
		ak_print_info_ex(PLAT_VI "### isp-get-frame over time: %ld(ms)\n", diff_time);
	} else {
		ak_print_debug_ex(PLAT_VI "### isp-get-frame use time: %ld(ms)\n\n", diff_time);
	}
}
#endif

static void* vi_get_one_frame(void *dev, struct user_t *user)
{
#if CALC_VI_GET_FRAME_TIME
	struct ak_timeval tv_start;
	struct ak_timeval tv_end;
	ak_get_ostime(&tv_start);
#endif

	struct ak_frame *new_frame = NULL;
	struct isp_frame frame = {0};

	if (isp_vi_get_frame(&frame)) {
		return NULL;
	}

#if CALC_VI_GET_FRAME_TIME
	ak_get_ostime(&tv_end);
#endif

	new_frame = (struct ak_frame *)calloc(1, sizeof(struct ak_frame));
	if (!new_frame) {
		ak_print_error_ex(PLAT_VI "calloc struct ak_frame failed\n");
		if (frame.private_data) {
			isp_vi_release_frame(frame.private_data);
			frame.private_data = NULL;
		}
		return NULL;
	}

	struct vi_device *pdev = (struct vi_device *)dev;
	vi_format_frame(pdev, &frame, new_frame);

	//ak_thread_mutex_lock(&pdev->frame_lock);
	pdev->frame_count++;
	new_frame->ref = pdev->user_ref;
	new_frame->bit_map = pdev->user_map;
	clear_bit(&(new_frame->bit_map), user->user_flg);
	list_add_tail(&(new_frame->list), &(pdev->frame_list));
	//ak_thread_mutex_unlock(&pdev->frame_lock);

#if CALC_VI_GET_FRAME_TIME
	vi_calc_frame_time(pdev, new_frame, &tv_start, &tv_end);
#endif
	/* debug: save yuv entry */
	vi_save_yuv_to_file(pdev->dev_num, new_frame->buf);

	return new_frame;
}

/**
 * vi_unregister_device -
 * @dev[IN]: pointer to device
 * return: 0 success, -1 failed
 * notes:
 */
static int vi_unregister_device(void *dev)
{
	struct vi_device *pdev = (struct vi_device *)dev;

	/* capture stop */
	ak_thread_mutex_lock(&pdev->vi_lock);

	vi_sys_ipc_unregister();

	/* releas all frames */
	vi_release_all_frames(pdev);

	vi_release_channel_resource(pdev);

	/* close devices(isp and camera) */
	isp_module_deinit();
	isp_vi_reset_drop_frame();

	/* isp device close and mutex resources release */
	isp_device_close();

	/* release 3D memory */
	if (pdev->isp_mem) {
		akuio_free_pmem(pdev->isp_mem);
		pdev->isp_mem = NULL;
	}

	ak_thread_mutex_unlock(&pdev->vi_lock);
	ak_thread_mutex_destroy(&pdev->vi_lock);
	ak_thread_mutex_destroy(&(pdev->frame_lock));
	/* unregister dma, all user modules can not use dma longer */
	vi_release_dma();
	vi_ctrl.dev_count--;
	list_del_init(&pdev->dev);
	free(pdev);
	pdev = NULL;
	ak_print_normal_ex(PLAT_VI "unregister done\n");

	return AK_SUCCESS;
}

/* register device to this module */
static void *vi_register_device(int dev_num)
{
	struct vi_device *pdev;

	/* if this is not the first device, create */
	if (!list_empty(&vi_ctrl.dev_list)) {
		pdev = vi_get_device(dev_num);
		if (pdev) {
			ak_print_normal(PLAT_VI "old device\n");
			return pdev;
		}
	}

	/* is a new device */
	ak_print_normal(PLAT_VI "new device\n");
	pdev = (struct vi_device *)calloc(1, sizeof(struct vi_device));
	if (!pdev) {
		ak_print_error_ex(PLAT_VI "No memory\n");
		return NULL;
	}
	ak_print_normal_ex(PLAT_VI "new dev=%p\n", pdev);
	/* init dma module, after this all user module can use dma */
	vi_init_dma();

	/* device open */
	if (isp_device_open()) {
		free(pdev);
		vi_release_dma();
		ak_print_error_ex(PLAT_VI "isp open deviece failed\n");
		return NULL;
	}

	/* isp config init */
	int ret = isp_init();
	if (ret) {
		ak_print_error_ex(PLAT_VI "isp init failed, video cannot work\n");
		isp_device_close();
		vi_release_dma();
		free(pdev);
		return NULL;
	}

	/* init device's resource */
	pdev->dev_num = dev_num;
	pdev->frame_count = 0;
	INIT_LIST_HEAD(&(pdev->frame_list));
	INIT_LIST_HEAD(&(pdev->user_list));
	ak_thread_mutex_init(&(pdev->vi_lock), NULL);
	ak_thread_mutex_init(&(pdev->frame_lock), NULL);

	/* add device to global device list */
	list_add_tail(&(pdev->dev), &vi_ctrl.dev_list);
	++vi_ctrl.dev_count;

	/* register vi debug callbacks */
	vi_sys_ipc_register();

	ak_print_normal(PLAT_VI "register vi device ok, dev_count=%d\n", vi_ctrl.dev_count);

	return pdev;
}




static int vi_set_capture_on(void *handle, enum isp_vi_status status)
{
	struct user_t *user = (struct user_t *)handle;
	struct vi_device *pdev = user->pdev;

	ak_print_info_ex(PLAT_VI "enter..., capture_on=%d\n", pdev->capture_on);
	if (pdev->capture_on)
		return 0;

	int main_size = CH_MAIN_SIZE(pdev->chn_main->max_width, pdev->chn_main->max_height);
	int sub_size = CH_SUB_SIZE(pdev->chn_sub->max_width, pdev->chn_sub->max_height);
	/* ch3_size must be half of sub_size */
	/* disable ch3, no need to alloc mem for ch3*/
	int ch3_size = 0;//CH_CH3_SIZE(pdev->chn_sub->max_width>>1, pdev->chn_sub->max_height>>1);
	int yuv_total_len = main_size + sub_size + ch3_size
		+ MOVE_CNT_INFO_SZ + BUFFER_RESERVE_BYTES;

	ak_thread_mutex_lock(&vi_ctrl.lock);
	int ret = isp_vi_capture_on(status, yuv_total_len);
	ak_thread_mutex_unlock(&vi_ctrl.lock);
	if (!ret) {
		pdev->capture_on = 1;
	}

	ak_print_info_ex(PLAT_VI "leave..., ret=%d\n", ret);
	return ret;
}

static int vi_set_capture_off(void *handle, enum isp_vi_status status)
{
	ak_print_info_ex(PLAT_VI "enter...\n");

	struct user_t *user = (struct user_t *)handle;
	struct vi_device *pdev = user->pdev;

	if (!pdev) {
		set_error_no(ERROR_TYPE_INVALID_USER);
		return AK_FAILED;
	}

    int ret = AK_SUCCESS;

    if (pdev->capture_on) {
        ak_print_notice_ex(PLAT_VI "capture off\n");
    	pdev->capture_on = 0;
    	ret = isp_vi_capture_off(status);
	}
	ak_print_info_ex(PLAT_VI "leave..., ret=%d\n", ret);

	return ret;
}

static int vi_set_channel_attr(struct vi_device *pdev, 
		const struct video_channel_attr *attr)
{
	int ret = AK_FAILED;

	ret = isp_vi_set_crop_attr(attr->crop.left,
			attr->crop.top, attr->crop.width, attr->crop.height);
	ret += isp_vi_set_main_attr(attr->res[VIDEO_CHN_MAIN].width,
			attr->res[VIDEO_CHN_MAIN].height);
	ret += isp_vi_set_sub_attr(attr->res[VIDEO_CHN_SUB].width,
			attr->res[VIDEO_CHN_SUB].height);

	if (!pdev->chn_main) {
		pdev->chn_main = (struct video_resolution *)calloc(1,
				sizeof(struct video_resolution ));
		pdev->chn_sub = (struct video_resolution *)calloc(1,
				sizeof(struct video_resolution ));
		pdev->crop_info = (struct crop_info *)calloc(1,
				sizeof(struct crop_info));
	}
	pdev->chn_main->width = attr->res[VIDEO_CHN_MAIN].width;
	pdev->chn_main->height = attr->res[VIDEO_CHN_MAIN].height;
	pdev->chn_sub->width = attr->res[VIDEO_CHN_SUB].width;
	pdev->chn_sub->height = attr->res[VIDEO_CHN_SUB].height;

	pdev->chn_main->max_width = attr->res[VIDEO_CHN_MAIN].max_width;
	pdev->chn_main->max_height = attr->res[VIDEO_CHN_MAIN].max_height;
	pdev->chn_sub->max_width = attr->res[VIDEO_CHN_SUB].max_width;
	pdev->chn_sub->max_height = attr->res[VIDEO_CHN_SUB].max_height;

	pdev->crop_info->left = attr->crop.left;
	pdev->crop_info->top = attr->crop.top;
	pdev->crop_info->width = attr->crop.width;
	pdev->crop_info->height = attr->crop.height;
    
	ak_print_notice_ex(PLAT_VI "main change resolution to: w: %d, h: %d\n",
		   	attr->res[VIDEO_CHN_MAIN].width, attr->res[VIDEO_CHN_MAIN].height);
	ak_print_notice_ex(PLAT_VI "sub change resolution to: w: %d, h: %d\n",
		   	attr->res[VIDEO_CHN_SUB].width, attr->res[VIDEO_CHN_SUB].height);

	return ret;
}

const char* ak_vi_get_version(void)
{
	return vi_version;
}

/**
 * ak_vi_match_sensor: match sensor according to appointed config file.
 * @config_file[IN]: it can be config file absolutely path, or a directory name.
 * If it is a directory name, it will search config file which format is
 * correct to match.
 * return: 0 success, -1 failed
 * notes:
 */
int ak_vi_match_sensor(const char *config_file)
{
	int ret = AK_FAILED;

	ak_print_normal_ex(PLAT_VI "config_file: %s\n", config_file);
	if (!config_file) {
		ak_print_error_ex(PLAT_VI "invalid argument\n");
		set_error_no(ERROR_TYPE_POINTER_NULL);
		return ret;
	}

	/*
	 * if config_file type is equal dir,
	 * iterate dir to find a suitable config file;
	 * otherwise, just check specifically file pointer by 'config_file'
	 */
	struct stat statebuf;
	stat(config_file, &statebuf);
	if (S_ISDIR(statebuf.st_mode)) {
		ret = match_all_config(config_file);
	} else {
		ret = do_match(config_file);
	}

	return ret;
}

/**
 * ak_vi_open: open video input device
 * @dev[IN]: video input device ID
 * return: 0 success, -1 failed
 * notes:
 */
void* ak_vi_open(enum video_dev_type dev)
{
	/*
	 * module init,
	 * if device not register, create it
	 */
	if (dev >= VIDEO_DEV_NUM) {
		ak_print_normal_ex(PLAT_VI "Unsupportted dev type: %d\n", dev);
		set_error_no(ERROR_TYPE_INVALID_ARG);
		return NULL;
	}

	ak_print_notice_ex(PLAT_VI "register device, dev type: %d\n", dev);
	ak_thread_mutex_lock(&vi_ctrl.lock);
	struct vi_device *pdev = vi_register_device(dev);
	if (!pdev) {
		ak_print_error_ex(PLAT_VI "register video device failed\n");
		ak_thread_mutex_unlock(&vi_ctrl.lock);
		set_error_no(ERROR_TYPE_DEV_OPEN_FAILED);
		return NULL;
	}
	memset(pdev->box_info,0,sizeof(pdev->box_info));
	ak_thread_mutex_unlock(&vi_ctrl.lock);
	ak_print_notice_ex(PLAT_VI "register device OK, pdev: %p\n", pdev);

	/* check if we need to add user */
	struct user_t *user = vi_new_user(pdev, dev);

	/* bind handle */
	vi_sysipc_bind_dev_handle(user, dev);

	return user;
}

/**
 * ak_vi_get_channel_attr: get channel attribution
 * @handle[IN]: opened vi handle
 * @attr[OUT]: channel attribution
 * return: 0 success, -1 failed
 * notes:
 */
int ak_vi_get_channel_attr(void *handle, struct video_channel_attr *attr)
{
	if (handle == NULL) {
		ak_print_error_ex(PLAT_VI "Invalid handle\n");
		set_error_no(ERROR_TYPE_POINTER_NULL);
		return -1;
	}
	if (attr == NULL) {
		ak_print_error_ex(PLAT_VI "Invalid arguments\n");
		set_error_no(ERROR_TYPE_INVALID_ARG);
		return -1;
	}

	struct user_t *user = (struct user_t *)handle;
	struct vi_device *pdev = user->pdev;

	if (!pdev) {
		ak_print_error_ex(PLAT_VI "invalid user\n");
		set_error_no(ERROR_TYPE_INVALID_USER);
		return AK_FAILED;
	}
	attr->res[VIDEO_CHN_MAIN].width = pdev->chn_main->width;
	attr->res[VIDEO_CHN_MAIN].height = pdev->chn_main->height;
	attr->res[VIDEO_CHN_MAIN].max_width = pdev->chn_main->max_width;
	attr->res[VIDEO_CHN_MAIN].max_height = pdev->chn_main->max_height;

	attr->res[VIDEO_CHN_SUB].width = pdev->chn_sub->width;
	attr->res[VIDEO_CHN_SUB].height = pdev->chn_sub->height;
	attr->res[VIDEO_CHN_SUB].max_width = pdev->chn_sub->max_width;
	attr->res[VIDEO_CHN_SUB].max_height = pdev->chn_sub->max_height;

	attr->crop.left = pdev->crop_info->left;
	attr->crop.top = pdev->crop_info->top;
	attr->crop.width= pdev->crop_info->width;
	attr->crop.height = pdev->crop_info->height;

	return 0;
}

/**
 * ak_vi_set_channel_attr: set channel attribution
 * @handle[IN]: opened vi handle
 * @attr[IN]: channel attribution
 * return: 0 success, -1 failed
 * notes:
 */
int ak_vi_set_channel_attr(void *handle,
		const struct video_channel_attr *attr)
{
	if (handle == NULL) {
		ak_print_error_ex(PLAT_VI "Invalid handle\n");
		set_error_no(ERROR_TYPE_POINTER_NULL);
		return -1;
	}

	if (attr == NULL) {
		ak_print_error_ex(PLAT_VI "Invalid arguments\n");
		set_error_no(ERROR_TYPE_INVALID_ARG);
		return -1;
	}

	/* 
	 * make a copy, because we will make sure the 
	 * value of height or width is corresponding 
	 */
	struct video_channel_attr check_attr;
	memcpy(&check_attr, attr, sizeof(struct video_channel_attr));
	/* check resolution and align it in need */
	if (vi_check_channel_attr_align(&check_attr)) {
		ak_print_error_ex(PLAT_VI "check attribute argument failed\n");
		return AK_FAILED;
	}

	int ret;
	struct user_t *user = (struct user_t *)handle;
	struct vi_device *pdev = user->pdev;
	if (!pdev) {
		ak_print_error_ex(PLAT_VI "invalid user\n");
		set_error_no(ERROR_TYPE_INVALID_USER);
		return AK_FAILED;
	}

	ak_thread_mutex_lock(&pdev->vi_lock);
	/* allocate 3d nr dma memory */
	if (!pdev->isp_mem) {
		pdev->isp_mem = isp_3D_NR_create(check_attr.res[VIDEO_CHN_MAIN].max_width,
				check_attr.res[VIDEO_CHN_MAIN].max_height);
		isp_fps_main();
	}
	/* set channel attribute */
	ret = vi_set_channel_attr(pdev, &check_attr);
	ak_thread_mutex_unlock(&pdev->vi_lock);

	return ret;
}

/**
 * ak_vi_change_channel_attr - change channel attribution
 * @handle[IN]: opened vi handle
 * @attr[IN]: channel attribution
 * return: 0 success, -1 failed
 * notes: IMPORTANT-you can change channel attribution in real time.
 */
int ak_vi_change_channel_attr(const void *handle,
		const struct video_channel_attr *attr)
{
	if (!handle) {
		ak_print_error_ex(PLAT_VI "invalid handle: %p\n", handle);
		set_error_no(ERROR_TYPE_POINTER_NULL);
		return AK_FAILED;
	}

	if (!attr) {
		ak_print_error_ex(PLAT_VI "invalid argument: %p\n", attr);
		set_error_no(ERROR_TYPE_POINTER_NULL);
		return AK_FAILED;
	}

	int ret;
	struct user_t *user = (struct user_t *)handle;
	struct vi_device *pdev = user->pdev;

	int i;
	const int lost_frame = 3;
	struct isp_frame frame;

	if (!pdev) {
		ak_print_error_ex(PLAT_VI "invalid user\n");
		set_error_no(ERROR_TYPE_INVALID_USER);
		return AK_FAILED;
	}

	/* 
	 * make a copy, because we will make sure the 
	 * value of height or width is corresponding 
	 */
	struct video_channel_attr check_attr;
	memcpy(&check_attr, attr, sizeof(struct video_channel_attr));
	/* check resolution and align it in need */
	if (vi_check_channel_attr_align(&check_attr)) {
		ak_print_error_ex(PLAT_VI "attribute argument is uncorrect\n");
		return AK_FAILED;
	}

	ak_thread_mutex_lock(&pdev->vi_lock);
	/*
	 * If video resolution switch from high to low, and video flip is enable,
	 * and osd rectangle size equal to old video resolution, we should
	 * close osd first, and then switch video resolution. After that, user
	 * should call osd set rect function to change to a suitable rectangle and
	 * call draw osd to enable osd out put.
	 */
	/* get flip */
	int flip, mirror;
   	ak_vi_get_flip_mirror(user, &flip, &mirror);

	/* when flip = 1, and resolution is high to low, close osd */
	if (flip == 1) {
		ak_print_notice_ex(PLAT_VI "##### close osd #####\n");

		int origin_main_w, origin_main_h, origin_sub_w, origin_sub_h;
		struct isp_osd_context_attr *osd_context_attr;
		struct vpss_osd_param param = {0};

		/* main channel check */
		param.id = OSD_SET_MAIN_CHANNEL_DATA;
		ak_vpss_osd_get_param(user, &param);
		osd_context_attr = (struct isp_osd_context_attr *)param.data;
		origin_main_w = osd_context_attr->osd_width;
		origin_main_h = osd_context_attr->osd_height;

		if (origin_main_w > attr->res[VIDEO_CHN_MAIN].width
				|| origin_main_h > attr->res[VIDEO_CHN_MAIN].height) {
			param.id = OSD_SET_MAIN_CHANNEL_DATA;
			ak_vpss_osd_close(user, &param);
		}

		/* sub channel check */
		memset(&param, 0, sizeof(struct vpss_osd_param));
		param.id = OSD_SET_SUB_CHANNEL_DATA;
		ak_vpss_osd_get_param(user, &param);
		osd_context_attr = (struct isp_osd_context_attr *)param.data;
		origin_sub_w = osd_context_attr->osd_width;
		origin_sub_h = osd_context_attr->osd_height;

		if (origin_sub_w > attr->res[VIDEO_CHN_SUB].width
				|| origin_sub_h > attr->res[VIDEO_CHN_SUB].height) {
			param.id = OSD_SET_SUB_CHANNEL_DATA;
			ak_vpss_osd_close(user, &param);
		}
	}

	/* capture off, to make sure change resulotion is safe */
	ret = isp_vi_stream_ctrl_off();
	if (AK_SUCCESS != ret) {
		ak_print_error_ex(PLAT_VI "stream ctrl off error\n");
		ak_thread_mutex_unlock(&pdev->vi_lock);
		set_error_no(ERROR_TYPE_IOCTL_FAILED);
		return AK_FAILED;
	}

	/* do change video resolution things */
	ret = vi_set_channel_attr(pdev, &check_attr);

	ret = isp_vi_stream_ctrl_on();
	if (AK_SUCCESS != ret) {
		ak_print_error_ex(PLAT_VI "stream ctrl on error\n");
		ak_thread_mutex_unlock(&pdev->vi_lock);
		set_error_no(ERROR_TYPE_IOCTL_FAILED);
		return AK_FAILED;
	}

	for (i = 0; i < lost_frame;) {
		ret = isp_vi_get_frame(&frame);
		if (AK_SUCCESS == ret) {
			i++;
			ak_print_normal_ex(PLAT_VI "########## release vi ##########\n");
			isp_vi_release_frame(frame.private_data);
		}
	}
	ak_thread_mutex_unlock(&pdev->vi_lock);

	return AK_SUCCESS;
}

int ak_vi_get_flip_mirror(void *handle, int *flip_enable, int *mirror_enable)
{
	if (!handle) {
		ak_print_error_ex(PLAT_VI "invalid handle: %p\n", handle);
		set_error_no(ERROR_TYPE_POINTER_NULL);
		return AK_FAILED;
	}

	struct isp_flip_mirror_info info = {0};
	int ret = isp_get_flip_mirror(&info);

	if (flip_enable) {
		*flip_enable = info.flip_en;
	}
	if (mirror_enable) {
		*mirror_enable = info.mirror_en;
	}

	return ret;
}

int ak_vi_set_flip_mirror(void *handle, int flip_enable, int mirror_enable)
{
	if (!handle) {
		ak_print_error_ex(PLAT_VI "invalid handle: %p\n", handle);
		set_error_no(ERROR_TYPE_POINTER_NULL);
		return AK_FAILED;
	}

	struct isp_flip_mirror_info value = {0};

	value.flip_en = flip_enable;
	value.mirror_en = mirror_enable;

	return isp_set_flip_mirror(&value);
}

/**
 * ak_vi_capture_on - open isp capture
 * @handle[IN], return by open
 * return: 0 success, -1 failed
 * notes:
 */
int ak_vi_capture_on(void *handle)
{
	if (!handle) {
		ak_print_error_ex(PLAT_VI "invalid handle: %p\n", handle);
		set_error_no(ERROR_TYPE_POINTER_NULL);
		return AK_FAILED;
	}

	return vi_set_capture_on(handle, ISP_VI_STATUS_OPEN);
}

/*
 * ak_vi_capture_off - close isp capture
 * @handle[IN], return by open
 * return: 0 success, -1 failed
 * notes:
 */
int ak_vi_capture_off(void *handle)
{
	if (!handle) {
		ak_print_error_ex(PLAT_VI "invalid handle: %p\n", handle);
		set_error_no(ERROR_TYPE_POINTER_NULL);
		return AK_FAILED;
	}

	return vi_set_capture_off(handle, ISP_VI_STATUS_CLOSE);
}

/**
 * ak_vi_reset - reset vi device
 * @handle[IN]: return by ak vi open
 * return: 0 success, -1 failed
 */
int ak_vi_reset(void *handle)
{
	ak_print_info_ex(PLAT_VI "enter...\n");
	if (!handle) {
		ak_print_error_ex(PLAT_VI "handle is NULL\n");
		return AK_FAILED;
	}

	/* capture stop */
	if (vi_set_capture_off(handle, ISP_VI_STATUS_RESET)) {
		return AK_FAILED;
	}

	struct user_t *user = (struct user_t *)handle;
	struct vi_device *pdev = user->pdev;

 	ak_thread_mutex_lock(&pdev->vi_lock);
	vi_release_all_frames(pdev);

	/* close devices(isp and camera) */
	isp_module_deinit();
	if (isp_device_close()) {
		ak_thread_mutex_unlock(&pdev->vi_lock);
		ak_print_error_ex(PLAT_VI "isp device close failed\n");
		return AK_FAILED;
	}
	ak_thread_mutex_unlock(&pdev->vi_lock);
	ak_sleep_ms(10);

	if (isp_device_open()) {
		ak_print_error_ex(PLAT_VI "isp device open failed\n");
		return AK_FAILED;
	}

	/* isp config init */
	if (isp_init()) {
		ak_print_error_ex(PLAT_VI "isp init failed, video cannot work\n");
		isp_device_close();
		return AK_FAILED;
	}

	/* set crop information */
 	struct video_channel_attr attr;

	memset(&attr, 0x00, sizeof(attr));
	attr.res[VIDEO_CHN_MAIN].width = pdev->chn_main->width;
	attr.res[VIDEO_CHN_MAIN].height = pdev->chn_main->height;
	attr.res[VIDEO_CHN_SUB].width = pdev->chn_sub->width;
	attr.res[VIDEO_CHN_SUB].height = pdev->chn_sub->height;

	attr.res[VIDEO_CHN_MAIN].max_width = pdev->chn_main->max_width;
	attr.res[VIDEO_CHN_MAIN].max_height = pdev->chn_main->max_height;
	attr.res[VIDEO_CHN_SUB].max_width = pdev->chn_sub->max_width;
	attr.res[VIDEO_CHN_SUB].max_height = pdev->chn_sub->max_height;

	attr.crop.left = pdev->crop_info->left;
	attr.crop.top = pdev->crop_info->top;
	attr.crop.width = pdev->crop_info->width;
	attr.crop.height = pdev->crop_info->height;

	if (ak_vi_set_channel_attr(handle, &attr)) {
		ak_print_error_ex(PLAT_VI "ak_vi_set_channel_attr failed\n");
	}

	isp_vi_reset_drop_frame();
	int ret = vi_set_capture_on(handle, ISP_VI_STATUS_RESET);
	ak_print_info_ex(PLAT_VI "leave..., ret=%d\n", ret);
	return ret;
}

/**
 * ak_vi_get_frame: get frame
 * @handle[IN]: return by open
 * @frame[OUT]: store frames
 * return: 0 success, otherwise failed
 */
int ak_vi_get_frame(void *handle, struct video_input_frame *frame)
{
	/* internal arguments check */
	if (NULL == handle) {
		ak_print_error_ex(PLAT_VI "Invalid handle\n");
		set_error_no(ERROR_TYPE_POINTER_NULL);
		return AK_FAILED;
	}
	if (NULL == frame) {
		ak_print_error_ex(PLAT_VI "Invalid arguments\n");
		set_error_no(ERROR_TYPE_INVALID_ARG);
		return AK_FAILED;
	}

	int ret = AK_FAILED;
	struct ak_frame *pos = NULL;
	struct user_t *user = (struct user_t *)handle;
	struct vi_device *pdev = user->pdev;

	/* filter invalid user */
	if (!pdev) {
		ak_print_error_ex(PLAT_VI "un register handle: %p\n", user);
		set_error_no(ERROR_TYPE_INVALID_USER);
		return AK_FAILED;
	}

	/* do nothing if capture off */
	if (!pdev->capture_on) {
		ak_print_error_ex(PLAT_VI "must call after capture on\n");
		set_error_no(ERROR_TYPE_NOT_INIT);
		return AK_FAILED;
	}


	ak_thread_mutex_lock(&(pdev->frame_lock));

	/* block because app level have only 2 frame data */
	if ((!list_empty(&(pdev->frame_list))) &&
			(pdev->frame_count >= USER_SPACE_MAX_FRAMES)) {

		goto get_frame_end;
	}

	/* find in current frame list */
	list_for_each_entry(pos, &(pdev->frame_list), list) {
		/* multiple user use same frame */
		if (test_bit(user->user_flg, &(pos->bit_map))) {
			ret = vi_data_to_frame(user, pos, frame);
			clear_bit(&(pos->bit_map), user->user_flg);
			goto get_frame_end;
		}
	}

	/* get new frame */
	pos = vi_get_one_frame(pdev, user);
	if (pos) {
		ret = vi_data_to_frame(user, pos, frame);
	} else {
		ak_print_error_ex(PLAT_VI "get frame failed\n");
		set_error_no(ERROR_TYPE_NO_DATA);
	}



get_frame_end:

	ak_thread_mutex_unlock(&(pdev->frame_lock));

	return ret;
}

/**
 * ak_vi_release_frame - release frame buffer after used
 * @pbuf[IN]: stream data buffer begin address
 * return: 0 success, otherwise failed
 * notes:
 */
int ak_vi_release_frame(void *handle, struct video_input_frame *frame)
{
	/* internal arguments check */
	if (NULL == handle) {
		ak_print_error_ex(PLAT_VI "Invalid handle\n");
		set_error_no(ERROR_TYPE_POINTER_NULL);
		return -1;
	}
	if (NULL == frame) {
		ak_print_error_ex(PLAT_VI "Invalid arguments\n");
		set_error_no(ERROR_TYPE_INVALID_ARG);
		return -1;
	}

	int ret = AK_FAILED;
	unsigned char found = AK_FALSE;
	struct user_t *user = (struct user_t *)handle;
	struct vi_device *pdev = user->pdev;
	struct ak_frame *pos = NULL;
	struct ak_frame *n = NULL;

	/* check frame by compare its timestamp */
	ak_thread_mutex_lock(&(pdev->frame_lock));
	list_for_each_entry(pos, &(pdev->frame_list), list) {
		if (pos->ts == frame->vi_frame[VIDEO_CHN_MAIN].ts) {
			ak_print_debug(PLAT_VI "release user_flg=%d, seq_no=%ld\n",
				user->user_flg, pos->seq_no);
			found = AK_TRUE;
			ret = AK_SUCCESS;
			break;
		}
	}
	ak_thread_mutex_unlock(&(pdev->frame_lock));

	if (found) {
		/* set current frame state and reduce reference */
		ak_thread_mutex_lock(&(pdev->frame_lock));
		del_ref(&(pos->ref), 1);

		/* release it methodical by traversal frames */
		list_for_each_entry_safe(pos, n, &pdev->frame_list, list) {
			/*
			 * Now the pos is pointing to first frame, so we only need
			 * to check whether the first frame uses OK.
			 * If it uses OK, than release it, and find next frame.
			 * If not, return or blocking a ticks time just like 10ms.
			 */
			if (pos->ref <= 0) {
				/* all time code use for debug */
				struct ak_timeval cur_time;
				long diff_time = 0;

				ak_get_ostime(&cur_time);
				diff_time = ak_diff_ms_time(&cur_time, &(pos->get_time));
				ak_print_debug(PLAT_VI "free user_flg=%d, seq_no=%ld, diff_time=%ld\n\n",
					user->user_flg, pos->seq_no, diff_time);

				/* true release operate */
				isp_vi_release_frame(pos->private_data);
				pdev->frame_count--;
				list_del(&pos->list);
				free(pos);
			} else {
				break;
			}
		}
		ak_thread_mutex_unlock(&(pdev->frame_lock));
	} else
		set_error_no(ERROR_TYPE_INVALID_ARG);

	return ret;
}

/**
 * ak_vi_get_fps - get current capture fps
 * @handle[IN]: video opened handle
 * return: fps value
 * notes:
 */
int ak_vi_get_fps(void *handle)
{
	if (!handle) {
		ak_print_error_ex(PLAT_VI "invalid handle: %p\n", handle);
		set_error_no(ERROR_TYPE_POINTER_NULL);
		return AK_FAILED;
	}
	/*
	 * this is current sensor capture fps
	 * not the ability of sensor max capture
	 */
	return isp_get_sensor_fps();
}

/**
 * ak_vi_set_fps - set capture fps
 * @handle[IN]: video opened handle
 * @fps[IN]: the fps value to be set
 * return: 0 success, -1 failed
 * notes:
 */
int ak_vi_set_fps(void *handle, int fps)
{
	if (!handle) {
		ak_print_error_ex(PLAT_VI "invalid handle: %p\n", handle);
		set_error_no(ERROR_TYPE_POINTER_NULL);
		return AK_FAILED;
	}

	/* set current sensor capture fps, can not bigger than driver supports */
	return isp_set_sensor_fps(fps);
}

/**
 * ak_vi_switch_mode - switch day/night mode
 * @handle[IN]: video opened handle
 * @mode[IN]:
 * return: 0 success, -1 failed
 * notes:
 */
int ak_vi_switch_mode(void *handle, enum video_daynight_mode mode)
{
	int ret = AK_FAILED;

	if (!handle) {
		ak_print_error_ex(PLAT_VI "invalid handle: %p\n", handle);
		return AK_FAILED;
	}

	ret = isp_switch(mode);
	return ret;
}

/**
 * ak_vi_get_sensor_resolution - get sensor max resolution supported
 * @handle[IN]: video opened handle
 * @res[OUT]:
 * return: 0 success, -1 failed
 * notes:
 */
int ak_vi_get_sensor_resolution(void *handle, struct video_resolution *res)
{
	if (handle == NULL) {
		set_error_no(ERROR_TYPE_POINTER_NULL);
		return AK_FAILED;
	}
	if (NULL == res) {
		set_error_no(ERROR_TYPE_INVALID_ARG);
		return AK_FAILED;
	}

	return isp_vi_get_sensor_attr(&res->width, &res->height);
}

int ak_vi_set_switch_fps_enable(void *handle, int enable)
{
	if (!handle) {
		ak_print_error_ex(PLAT_VI "invalid handle: %p\n", handle);
		set_error_no(ERROR_TYPE_POINTER_NULL);
		return AK_FAILED;
	}
	return isp_set_switch_fps_enable(enable);
}

void* ak_vi_get_handle(enum video_dev_type dev)
{
	/* 1. get device */
	struct vi_device *pdev = vi_get_device(dev);
	if (!pdev) {
		set_error_no(ERROR_TYPE_INVALID_ARG);
		ak_print_normal(PLAT_VI "the dev=%d had not opened\n", dev);
		return NULL;
	}

	/* 2. get handle by device */
	void *handle = NULL;
	struct user_t *user = NULL;

	list_for_each_entry(user, &pdev->user_list, list) {
		if (user->dev_num == dev) {
			handle = (void *)user;
			break;
		}
	}

	return handle;
}

enum video_work_scene ak_vi_get_work_scene(enum video_dev_type dev)
{
	struct vi_device *pdev = vi_get_device(dev);
	if (!pdev) {
		ak_print_normal(PLAT_VI "the dev=%d had not opened\n", dev);
		return VIDEO_SCENE_UNKNOWN;
	}

	int scene = isp_get_work_scene();
	if ((scene < VIDEO_SCENE_INDOOR) || (scene > VIDEO_SCENE_OUTDOOR)) {
		scene = VIDEO_SCENE_INDOOR;
	}

	return scene;
}


/**
 * ak_vi_clear_buffer - clear vi capture buffer
 * @handle[IN]: return by ak vi open
 * return: 0 success, -1 failed
 * notes: if stopped encoding at all
 *		you can call this function before encoding again
 */
int ak_vi_clear_buffer(void *handle)
{
	if (!handle) {
		set_error_no(ERROR_TYPE_POINTER_NULL);
		ak_print_error_ex(PLAT_VI "invalid arguments\n");
		return AK_FAILED;
	}

	isp_vi_reset_drop_frame();

	return AK_SUCCESS;
}

/**
 * ak_vi_close -
 * @handle[IN]:return by ak vi open
 * return: 0 success, otherwise failed
 * notes:
 */
int ak_vi_close(void *handle)
{
	ak_print_notice_ex(PLAT_VI "enter close vi\n");

	if (handle == NULL) {
		set_error_no(ERROR_TYPE_POINTER_NULL);
		ak_print_error_ex(PLAT_VI "invalid arguments\n");
		return AK_FAILED;
	}

	int ret = AK_SUCCESS;
	struct user_t *user = (struct user_t *)handle;
	struct vi_device *pdev = user->pdev;

	/* incase of invalid user */
	if (!pdev) {
		set_error_no(ERROR_TYPE_INVALID_USER);
		ak_print_error_ex(PLAT_VI "un init user: %p\n", user);
		return AK_FAILED;
	}

	vi_sysipc_unbind_dev_handle(pdev->dev_num);

	/* release vi device data */
	ak_thread_mutex_lock(&vi_ctrl.lock);
	del_ref(&(pdev->user_ref), 1);
	ak_print_notice_ex(PLAT_VI "user_ref=%d\n", pdev->user_ref);
	/* unregister device while no more user */
	if (pdev->user_ref <= 0) {
	    vi_set_capture_off(handle, ISP_VI_STATUS_CLOSE);
		ret = vi_unregister_device(pdev);
	}

	/* release vi user data */
	list_del_init(&user->list);
	free(user);
	user = NULL;
	ak_thread_mutex_unlock(&vi_ctrl.lock);
	ak_print_normal_ex(PLAT_VI "leave\n");

	return ret;
}

/**
 * ak_vi_put_rect -
 * @handle[IN]:return by ak vi open
 *@box_info[IN]:want to draw rect info ,if NULL is clear rect
 *@rgb24[IN]:rgb24 color
 * return: 0 success, otherwise failed
 * notes:
 */

int ak_vi_put_rect(void *handle, enum video_channel channel,video_rect *box_info,unsigned int rgb24)
{
    unsigned char r=0,g=0,b=0;
    if (handle == NULL || channel > VIDEO_CHN_NUM) {
		set_error_no(ERROR_TYPE_POINTER_NULL);
		ak_print_error_ex(PLAT_VI "put rect \n");
		return AK_FAILED;
	}
    struct user_t *user = (struct user_t *)handle;
    struct vi_device *pdev = user->pdev;
	ak_thread_mutex_lock(&box_lock);
	if(box_info == NULL)
    {
        pdev->box_info[channel].box_attr.height = 0;
        pdev->box_info[channel].box_attr.left = 0;
        pdev->box_info[channel].box_attr.top = 0;
        pdev->box_info[channel].box_attr.width = 0;
    }else{

        memcpy(&pdev->box_info[channel].box_attr, box_info, sizeof(video_rect));

		r = (rgb24 & 0xff0000) >> 16;
		g = (rgb24 & 0xff00) >> 8;
		b = (rgb24 & 0xff);
		pdev->box_info[channel].luma = (unsigned char)((66*r + 129*g + 25*b + 128) >> 8) +16;
		pdev->box_info[channel].chroma_u = (unsigned char)((-38*r -74*g + 112*b +128) >> 8) + 128;
		pdev->box_info[channel].chroma_v = (unsigned char)((112*r -94*g - 18*b + 128) >> 8) +128;
		if(pdev->box_info[channel].chroma_u > 255)
			pdev->box_info[channel].chroma_u = 255;
		if(pdev->box_info[channel].chroma_u <0)
			pdev->box_info[channel].chroma_u = 0;
		if(pdev->box_info[channel].chroma_v > 255)
			pdev->box_info[channel].chroma_v = 255;
		if(pdev->box_info[channel].chroma_v <0)
			pdev->box_info[channel].chroma_v = 0;

    }
    
	ak_thread_mutex_unlock(&box_lock);
    return 0;
}

/**
 * ak_vi_set_box_rect - set box rect info
 * @x[IN]: left of rect
 * @y[IN]: top of rect
 * @width[IN]: width of rect
 * @height[IN]: height of rect
 * return: 0 success, -1 failed
 * notes:
 */
int ak_vi_set_box_rect(int x, int y, int width, int height)
{
	ak_thread_mutex_lock(&box_lock);
	box_rect.left = x;
	box_rect.top = y;
	box_rect.width = width;
	box_rect.height = height;
	ak_thread_mutex_unlock(&box_lock);

	return 0;
}
