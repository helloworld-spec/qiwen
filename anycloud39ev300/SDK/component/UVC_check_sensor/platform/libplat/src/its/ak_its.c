#define ITS_IMPLEMENT			1

#if	ITS_IMPLEMENT

#include "lwip/sockets.h"
#include "lwip/ipv4/ip.h"
#include "lwip/ipv4/ip_addr.h"

#include "ak_common.h"
#include "ak_thread.h"
#include "ak_vi.h"

#include "libc_mem.h"
#include "ispdrv_modules_interface.h"
#include "isp_conf.h"
#include "list.h"
#include "kernel.h"

#include "isp_struct.h"
#include "hal_camera.h"
#include "ak_isp_char.h"

#define ISP_PORT				8000		/** tcp port **/
#define ISP_ATTR_ID_SIZE		2
#define ISP_CMD_TYPE_SIZE		2

#define ISP_MODULE_ID_SIZE		2
#define ISP_MODULE_LEN_SIZE		2

#define ISP_PACKET_HEAD_SIZE	(ISP_ATTR_ID_SIZE+ISP_CMD_TYPE_SIZE)
#define ISP_PARM_LEN_SIZE		4
#define ISP_RET_SIZE			4
#define ISP_PARM_CNT_SIZE		4

#define HEARTBEAT_CHECKTIME		(15)

#define ISP_CFG_MAX_SIZE		(1024*200)
#define ISP_CONF_PATH "ISPCFG"

#define YUV_SAVE_PATH	"a:/"

typedef enum {
	CMD_GET = 0,
	CMD_REPLY,
	CMD_SET,
	CMD_RET,
	CMD_GET_TXT,
	CMD_REPLY_TXT,

	CMD_TYPE_NUM
} T_CMD_TYPE;

typedef enum {
	HEAD_ERR = 0,
	HEAD_NO_DATA,
	HEAD_HAVE_DATA,

	HEAD_TYPE_NUM
} T_HEAD_TYPE;

typedef enum {

	ISP_PARM_CODE = ISP_SENSOR + 1,	//按参数编码
	ISP_REGISTER,						//寄存器参数

	ISP_RAW_IMG,						//一帧raw图像数据
	ISP_YUV_IMG,						//一帧yuv图像数据
	ISP_ENCODE_IMG,						//一帧encode图像数据

	ISP_CFG_DATA,						//cfg data
	ISP_HEARTBEAT,						//heartbeat

	ISP_ATTR_TYPE_NUM
} T_ATTR_TYPE;

typedef struct _ctl_handle
{
	int		sock;
	ak_sem_t	isp_sem;
	struct list_head queue;
	ak_mutex_t lock;
}CTL_HANDLE, *PCTL_HANDLE;

typedef struct _cmd_head
{
	unsigned short attr_id;
	unsigned short cmd_type;
}CMD_HEAD;

typedef struct _trans_data
{
	CMD_HEAD	head;
	unsigned long		datalen;
	unsigned char* 		data;
}TRANS_DATA, *PTRANS_DATA;

struct cmd_node {
	unsigned char *send_buf;
	struct list_head list;
};

unsigned short isp_struct_len[ISP_HUE + 1] = {
	sizeof(struct ak_isp_init_blc),
	sizeof(struct ak_isp_init_lsc),
	sizeof(struct ak_isp_init_raw_lut),
	sizeof(struct ak_isp_init_nr),
	sizeof(struct ak_isp_init_3dnr),
	sizeof(struct ak_isp_init_gb),
	sizeof(struct ak_isp_init_demo),
	sizeof(struct ak_isp_init_gamma),
	sizeof(struct ak_isp_init_ccm),
	sizeof(struct ak_isp_init_fcs),
	sizeof(struct ak_isp_init_wdr),
	sizeof(struct ak_isp_init_sharp),
	sizeof(struct ak_isp_init_saturation),
	sizeof(struct ak_isp_init_contrast),
	sizeof(struct ak_isp_init_rgb2yuv),
	sizeof(struct ak_isp_init_effect),
	sizeof(struct ak_isp_init_dpc),
	sizeof(struct ak_isp_init_weight),
	sizeof(struct ak_isp_af),
	sizeof(struct ak_isp_init_wb),
	sizeof(struct ak_isp_init_exp),
	sizeof(struct ak_isp_init_misc),
	sizeof(struct ak_isp_init_y_gamma),
	sizeof(struct ak_isp_init_hue)
};

static unsigned char its_run_flag = AK_FALSE;
static int cfd = -1;	/** socket file descriptor **/
static const char its_version[] = "libplat_its V2.0.00";
static int sfd = -1;	/** server socket file descriptor **/

static PCTL_HANDLE pctrl_start = NULL;	/** define a isp handle **/
static ak_pthread_t isp_svr_tid = NULL;
static ak_pthread_t respons_tid = NULL;	/** response data thread id **/
static void heartbeat_timeout(int msg)
{
/*
	switch (msg){
		case CMD_SIGALRM:
			printf("heartbeat timeout!\n");

			if (-1 != cfd) {
				close(cfd);
				cfd = -1;
			}
			//set_check_timer(0);
			break;

		default:
			break;
	}
	return ;
*/
}

static T_HEAD_TYPE check_cmd_head(PTRANS_DATA cmd)
{
	if (ISP_HEARTBEAT != cmd->head.attr_id)
		ak_print_info("attr_id : %d, cmd_type : %d\n",
				cmd->head.attr_id, cmd->head.cmd_type);

	if ((cmd->head.attr_id >= ISP_ATTR_TYPE_NUM)
			|| ((cmd->head.cmd_type != CMD_GET)
				&& (cmd->head.cmd_type != CMD_SET)
				&& (cmd->head.cmd_type != CMD_GET_TXT))) {
		ak_print_warning_ex("recv attr id or cmd type error:%d, %d!\n",
				cmd->head.attr_id, cmd->head.cmd_type);
		return HEAD_ERR;
	}
	
	// ak_print_notice("ymx: cmd_type:%d, id:%d\n",cmd->head.cmd_type,cmd->head.attr_id);
	if ((CMD_SET == cmd->head.cmd_type)
			|| ((ISP_PARM_CODE == cmd->head.attr_id)
				|| (ISP_REGISTER == cmd->head.attr_id))) {
		return HEAD_HAVE_DATA;
	}

	return HEAD_NO_DATA;
}

/*
 * get yuv data from anyka_ipc
 *
 * success, buf pointer to the file name
 * 			size, is the length of file name
 * fail, buf is NULL, size is zero
 */
static void Get_YUV_Img(unsigned char* buf, unsigned int* size)
{
	ak_print_notice_ex("entry\n");


	void *vi_handle = ak_vi_get_handle(VIDEO_DEV0);
	if (!vi_handle) {
		ak_print_warning_ex("get vi handle failed\n");
		return;
	}

	struct video_input_frame *frame = (struct video_input_frame *)calloc(1,
			sizeof(struct video_input_frame));
	if (!frame) {
		ak_print_warning_ex("calloc video_input_frame failed\n");
		return;
	}

	int ret = ak_vi_get_frame(vi_handle, frame);
	if (!ret) {
		ak_print_notice_ex("get frame ok\n");

		*size = frame->vi_frame[VIDEO_CHN_MAIN].len;
		memcpy(buf, frame->vi_frame[VIDEO_CHN_MAIN].data, *size);
		ak_vi_release_frame(vi_handle, frame);
	
	}

	free(frame);

}

static int ImgData_Get(unsigned short module_id, unsigned char* buf,
		unsigned int* size)
{
	if (module_id > ISP_ENCODE_IMG || module_id < ISP_RAW_IMG
			|| NULL == buf || NULL == size) {
		printf("Isp_ImgData_Get param err\n");
		return 0;
	}

	switch (module_id) {
		case ISP_RAW_IMG:
			printf("get RAW IMG!\n");
			break;
		case ISP_YUV_IMG:
			printf("get YUV IMG!\n");
			Get_YUV_Img(buf, size);
			break;
		case ISP_ENCODE_IMG:
			printf("get Encoded IMG!\n");
			break;
		default:
			break;
	}

	return 0;
}

/**  
 * isp_conf_read: close isp capture
 * @file_name[in]:the path of isp config file
 * @p_data[out]:the data of isp config file
 * @p_data_len[out]:the data len of isp config file
 * return: 0 success, -1 failed
 * notes:
 */
static int isp_conf_read(const char* file_name, char**  p_data, char**  p_sensor_data, unsigned long* p_data_len)
{
	void *handle = NULL;
	int pr_ret=0, ret = 0, i=0;
	unsigned long all_count = 0;

	CFGFILE_HEADINFO *p_isp_conf_header;
	unsigned long tmp_len = 0, read_len = 0;
	unsigned long sensor_id;
	
	handle = (void *)ak_partition_open(file_name); 
	
	if(handle == NULL)
	{
		printf("ak_partition_open err!\r\n");
		ret = -1;
		goto ERR;
	}

	all_count = ak_partition_get_dat_size(handle);
	printf("ymx : dat_size=%d\r\n", all_count);
	if(0 == all_count)
	{
		printf("ISPCFG  partition get data fail\r\n");
		ret = -1;
		goto ERR;
	}

	*p_data = malloc(all_count);
	if(NULL == *p_data)
	{
		printf("isp conf buffer malloc fail\n");
		ret = -4;
		goto ERR1;
	}	
		
	pr_ret = ak_partition_read(handle, *p_data, all_count);

	
	if((pr_ret < 0) || (pr_ret != (all_count)))
	{
		printf("ak_partition_read err! read=%d,real=%d\r\n",all_count,pr_ret);
		ret = -1;
		goto ERR;
	}
	printf("ak_partition_read ! read=%d,real=%d\r\n",pr_ret,all_count);

	int *p_version =  (int *)*p_data;
	printf("version=%d\r\n",*p_version);

	/* get one sensor config file */
	p_isp_conf_header = (CFGFILE_HEADINFO *)(*p_data);
	
	sensor_id = cam_get_sensor_id();

	for(i = 0; i < all_count; i = tmp_len)
	{

		printf("ID %x,size:%d\r\n",sensor_id,p_isp_conf_header->subFileLen);

		if(sensor_id == p_isp_conf_header->sensorId)
		{
			(*p_sensor_data) = (char *)p_isp_conf_header;// get sensor start point of the config file
			
			read_len =  p_isp_conf_header->subFileLen;
			//(*p_data_len) = read_len;// get day isp config data len
			
			p_isp_conf_header = (CFGFILE_HEADINFO *)((*p_data) + tmp_len +read_len);
			read_len +=  p_isp_conf_header->subFileLen; 
			(*p_data_len) = read_len;// get day and night isp config data len	

			goto ERR;
		}

		/* skip one sensor config file  */
		tmp_len +=  p_isp_conf_header->subFileLen;
		p_isp_conf_header = (CFGFILE_HEADINFO *)((*p_data) + tmp_len);
		
		tmp_len +=  p_isp_conf_header->subFileLen; //白天和黑夜配置长度有可能不一样
		p_isp_conf_header = (CFGFILE_HEADINFO *)((*p_data) + tmp_len);

	}
	
	printf("error : no sensor config found.");
	goto ERR;
ERR1:
	free(*p_data);
	
ERR:	
	ak_partition_close(handle);
	return ret;	

}

static int isp_cfg_file_load(char *p_data, unsigned int* size)
{
	unsigned long out_data_size;
	char *p_out_data = NULL,*p_sensor_out_data = NULL;
	memset(p_data, 0, *size);
	if(isp_conf_read(ISP_CONF_PATH, &p_out_data, &p_sensor_out_data, &out_data_size))
	{
		
		ak_print_error_ex("read isp config error\n");
		return -1;
	}
	if(*size < out_data_size)
	{
		ak_print_error_ex("malloc size less than isp config file size.\n");
		free(p_out_data);
		return -1;
	}
	*size = out_data_size;

	int *tmp_version = (int *)p_sensor_out_data;
	ak_print_error_ex("tmp_version=%d .%d\n",*tmp_version,out_data_size);
	
	memcpy(p_data, p_sensor_out_data, out_data_size);

	int *tmp_version2 = (int *)p_data;
	ak_print_error_ex("tmp_version=%d .%d\n",*tmp_version2,out_data_size);
	
	free(p_out_data);
	return 0;
}

/**
 * brief: set isp module data
 * @module_id[IN]: id of the module in enum isp_module_id
 * @buf[IN]: data buf to set
 * @size[OUT]: size of the attr
 * return: 0 success, -1 failed
 * notes:
 */
static int isp_set_attr(enum isp_module_id module_id, char *buf, unsigned int size)
{
	if (!buf) {
		ak_print_error_ex("param err\n");
		return -1;
	}

	if ((module_id <= ISP_HUE) && (size != isp_struct_len[module_id])) {
		ak_print_error_ex("size err\n");
		return -1;
	}

	int ret = -1;
	switch (module_id) {
	case ISP_BB: {
			struct  ak_isp_init_blc *p = (struct  ak_isp_init_blc *)buf;
			ret = ak_isp_vp_set_blc_attr(&p->p_blc);//ak_isp_set_blc_attr(&p->blc);
		}
		break;
	case ISP_LSC: {
			struct  ak_isp_init_lsc *p = (struct  ak_isp_init_lsc *)buf;
			ret = ak_isp_vp_set_lsc_attr(&p->lsc);
		}
		break;
	case ISP_RAW_LUT: {
			struct ak_isp_init_raw_lut *p = (struct ak_isp_init_raw_lut *)buf;
			ret = ak_isp_vp_set_raw_lut_attr(&p->raw_lut_p);
		}
		break;
	case ISP_NR: {
			struct ak_isp_init_nr *p = (struct ak_isp_init_nr *)buf;
			ret = ak_isp_vp_set_nr1_attr(&p->p_nr1);
			ret |= ak_isp_vp_set_nr2_attr(&p->p_nr2);
		}
		break;
	case ISP_3DNR: {
			struct ak_isp_init_3dnr *p = (struct ak_isp_init_3dnr *)buf;
			ret = ak_isp_vp_set_3d_nr_attr(&p->p_3d_nr);
		}
		break;
	case ISP_GB: {
			struct ak_isp_init_gb *p = (struct ak_isp_init_gb *)buf;
			ret = ak_isp_vp_set_gb_attr(&p->p_gb);
		}
		break;
	case ISP_DEMO: {
			struct ak_isp_init_demo *p = (struct ak_isp_init_demo *)buf;
			ret = ak_isp_vp_set_demo_attr(&p->p_demo_attr);
		}
		break;
	case ISP_GAMMA: {
			struct ak_isp_init_gamma *p = (struct ak_isp_init_gamma *)buf;
			ret = ak_isp_vp_set_rgb_gamma_attr(&p->p_gamma_attr);
		}
		break;
	case ISP_CCM: {
			struct  ak_isp_init_ccm *p = (struct  ak_isp_init_ccm *)buf;
			ret = ak_isp_vp_set_ccm_attr(&p->p_ccm);
		}
		break;
	case ISP_FCS: {
			struct  ak_isp_init_fcs *p = (struct  ak_isp_init_fcs *)buf;
			ret = ak_isp_vp_set_fcs_attr(&p->p_fcs);
		}
		break;
	case ISP_WDR: {
			struct ak_isp_init_wdr *p = (struct ak_isp_init_wdr *)buf;
			ret = ak_isp_vp_set_wdr_attr(&p->p_wdr_attr);
		}
		break;
	case ISP_SHARP: {
			struct ak_isp_init_sharp *p = (struct ak_isp_init_sharp *)buf;
			ret = ak_isp_vp_set_sharp_attr(&p->p_sharp_attr);
			ret |= ak_isp_vp_set_sharp_ex_attr(&p->p_sharp_ex_attr);
		}
		break;
	case ISP_SATURATION: {
			struct ak_isp_init_saturation *p = (struct ak_isp_init_saturation *)buf;
			ret = ak_isp_vp_set_saturation_attr(&p->p_se_attr);
		}
		break;
	case ISP_CONSTRAST: {
			struct  ak_isp_init_contrast *p = (struct  ak_isp_init_contrast *)buf;
			ret = ak_isp_vp_set_contrast_attr(&p->p_contrast);
		}
		break;
	case ISP_YUVEFFECT: {
			struct ak_isp_init_effect *p = (struct ak_isp_init_effect *)buf;
			ret = ak_isp_vp_set_effect_attr(&p->p_isp_effect);
		}
		break;
	case ISP_RGB2YUV: {
			struct ak_isp_init_rgb2yuv *p = (struct ak_isp_init_rgb2yuv *)buf;
			ret = ak_isp_vp_set_rgb2yuv_attr(&p->p_rgb2yuv);
		}
		break;
	case ISP_DPC: {
			struct ak_isp_init_dpc *p = (struct ak_isp_init_dpc *)buf;
			ret = ak_isp_vp_set_dpc_attr(&p->p_ddpc);
			ret |= ak_isp_vp_set_sdpc_attr(&p->p_sdpc);
		}
		break;
	case ISP_WEIGHT: {
			struct ak_isp_init_weight *p = (struct ak_isp_init_weight *)buf;
			ret = ak_isp_vp_set_zone_weight(&p->p_weight);
		}
		break;
	case ISP_AF: {
			struct ak_isp_af *p = (struct ak_isp_af *)buf;
			ret = ak_isp_vp_set_af_attr(&p->p_af_attr);
		}
		break;
	case ISP_WB: {
			struct ak_isp_init_wb *p = (struct ak_isp_init_wb *)buf;
			ret = ak_isp_vp_set_wb_type(&p->wb_type);
			ret |= ak_isp_vp_set_mwb_attr(&p->p_mwb);
			ret |= ak_isp_vp_set_awb_attr(&p->p_awb);
			ret |= ak_isp_vp_set_awb_ex_attr(&p->p_awb_ex);
		}
		break;
	case ISP_EXP: {
			struct ak_isp_init_exp *p = (struct ak_isp_init_exp *)buf;
			ret = ak_isp_vp_set_raw_hist_attr(&p->p_raw_hist);
			ret |= ak_isp_vp_set_rgb_hist_attr(&p->p_rgb_hist);
			ret |= ak_isp_vp_set_yuv_hist_attr(&p->p_yuv_hist);
			ret |= ak_isp_vp_set_exp_type(&p->p_exp_type);
			ret |= ak_isp_vp_set_frame_rate(&p->p_frame_rate);
			ret |= ak_isp_vp_set_ae_attr(&p->p_ae);
		}
		break;
	case ISP_MISC: {
			struct ak_isp_init_misc *p = (struct ak_isp_init_misc *)buf;
			ret = ak_isp_vo_set_misc_attr(&p->p_misc);
		}
		break;

	case ISP_Y_GAMMA: {
			struct ak_isp_init_y_gamma *p = (struct ak_isp_init_y_gamma*)buf;
			ret = ak_isp_vp_set_y_gamma_attr(&p->p_gamma_attr);
		}
		break;
	case ISP_HUE: {
			struct ak_isp_init_hue *p = (struct ak_isp_init_hue *)buf;
			ret = ak_isp_vp_set_hue_attr(&p->p_hue);
		}
		break;
	case ISP_FPS_CTRL: {
			AK_ISP_FRAME_RATE_ATTR *p = (AK_ISP_FRAME_RATE_ATTR *)buf;
			ak_isp_vp_set_frame_rate(p);
		}
		break;
	default:
		ak_print_error_ex("param err\n");
		return -1;
		break;
	}

	return ret;
}


/**
 * brief: get isp module data
 * @module_id[IN]: id of the module in enum isp_module_id
 * @buf[OUT]: dest buf for data out
 * @size[OUT]: size of the attr
 * return: 0 success, -1 failed
 * notes:
 */
int isp_get_attr(enum isp_module_id module_id, void *buf, unsigned int *size)
{
	if (NULL == buf || NULL == size) {
		ak_print_error_ex("param err\n");
		return -1;
	}

	int ret = -1;

	switch (module_id) {
	case ISP_BB: {
			struct  ak_isp_init_blc* p = (struct  ak_isp_init_blc*)buf;

			ret = ak_isp_vp_get_blc_attr(&p->p_blc);
			*size = sizeof(struct  ak_isp_init_blc);
		}
		break;
	case ISP_LSC: {
			struct  ak_isp_init_lsc* p = (struct  ak_isp_init_lsc*)buf;

			ret = ak_isp_vp_get_lsc_attr(&p->lsc);
			*size = sizeof(struct  ak_isp_init_lsc);
		}
		break;
	case ISP_RAW_LUT: {
			struct ak_isp_init_raw_lut *p = (struct ak_isp_init_raw_lut*)buf;

			ret = ak_isp_vp_get_raw_lut_attr(&p->raw_lut_p);
			*size = sizeof(struct ak_isp_init_raw_lut);
		}
		break;
	case ISP_NR: {
			struct ak_isp_init_nr *p = (struct ak_isp_init_nr*)buf;

			ret = ak_isp_vp_get_nr1_attr(&p->p_nr1);
			ret |= ak_isp_vp_get_nr2_attr(&p->p_nr2);
			*size = sizeof(struct ak_isp_init_nr);
		}
		break;
	case ISP_3DNR: {
			struct ak_isp_init_3dnr * p = (struct ak_isp_init_3dnr*)buf;

			ret = ak_isp_vp_get_3d_nr_attr(&p->p_3d_nr);
			*size = sizeof(struct ak_isp_init_3dnr);
		}
		break;
	case ISP_GB: {
			struct ak_isp_init_gb *p = (struct ak_isp_init_gb*)buf;

			ret = ak_isp_vp_get_gb_attr(&p->p_gb);
			*size = sizeof(struct ak_isp_init_gb);
		}
		break;
	case ISP_DEMO: {
			struct ak_isp_init_demo *p = (struct ak_isp_init_demo*)buf;

			ret = ak_isp_vp_get_demo_attr(&p->p_demo_attr);
			*size = sizeof(struct ak_isp_init_demo);
		}
		break;
	case ISP_GAMMA: {
			struct ak_isp_init_gamma *p = (struct ak_isp_init_gamma*)buf;

			ret = ak_isp_vp_get_rgb_gamma_attr(&p->p_gamma_attr);
			*size = sizeof(struct ak_isp_init_gamma);
		}
		break;
	case ISP_CCM: {
			struct  ak_isp_init_ccm *p = (struct  ak_isp_init_ccm*)buf;

			ret = ak_isp_vp_get_ccm_attr(&p->p_ccm);
			*size = sizeof(struct  ak_isp_init_ccm);
		}
		break;
	case ISP_FCS: {
			struct  ak_isp_init_fcs *p = (struct  ak_isp_init_fcs*)buf;

			ret = ak_isp_vp_get_fcs_attr(&p->p_fcs);
			*size = sizeof(struct  ak_isp_init_fcs);
		}
		break;
	case ISP_WDR: {
			struct ak_isp_init_wdr *p = (struct ak_isp_init_wdr*)buf;

			ret = ak_isp_vp_get_wdr_attr(&p->p_wdr_attr);
			*size = sizeof(struct ak_isp_init_wdr);
		}
		break;
	case ISP_SHARP: {
			struct ak_isp_init_sharp *p = (struct ak_isp_init_sharp*)buf;

			ret = ak_isp_vp_get_sharp_attr(&p->p_sharp_attr);
			ret |= ak_isp_vp_get_sharp_ex_attr(&p->p_sharp_ex_attr);
			*size = sizeof(struct ak_isp_init_sharp);
		}
		break;
	case ISP_SATURATION: {
			struct ak_isp_init_saturation *p = (struct ak_isp_init_saturation*)buf;

			ret = ak_isp_vp_get_saturation_attr(&p->p_se_attr);
			*size = sizeof(struct ak_isp_init_saturation);
		}
		break;
	case ISP_CONSTRAST: {
			struct  ak_isp_init_contrast *p = (struct  ak_isp_init_contrast*)buf;

			ret = ak_isp_vp_get_contrast_attr(&p->p_contrast);
			*size = sizeof(struct  ak_isp_init_contrast);
		}
		break;
	case ISP_RGB2YUV: {
			struct ak_isp_init_rgb2yuv *p = (struct ak_isp_init_rgb2yuv*)buf;

			ret = ak_isp_vp_get_rgb2yuv_attr(&p->p_rgb2yuv);
			*size = sizeof(struct ak_isp_init_rgb2yuv);
		}
		break;
	case ISP_YUVEFFECT: {
			struct ak_isp_init_effect *p = (struct ak_isp_init_effect*)buf;

			ret = ak_isp_vp_get_effect_attr(&p->p_isp_effect);
			*size = sizeof(struct ak_isp_init_effect);
		}
		break;
	case ISP_DPC: {
			struct ak_isp_init_dpc *p = (struct ak_isp_init_dpc*)buf;

			ret = ak_isp_vp_get_dpc_attr(&p->p_ddpc);
			ret |= ak_isp_vp_get_sdpc_attr(&p->p_sdpc);
			*size = sizeof(struct ak_isp_init_dpc);
		}
		break;
	case ISP_WEIGHT: {
			struct ak_isp_init_weight *p = (struct ak_isp_init_weight*)buf;

			ret = ak_isp_vp_get_zone_weight(&p->p_weight);
			*size = sizeof(struct ak_isp_init_weight);
		}
		break;
	case ISP_AF: {
			struct ak_isp_af *p = (struct ak_isp_af*)buf;

			ret = ak_isp_vp_get_af_attr(&p->p_af_attr);
			*size = sizeof(struct ak_isp_af);
		}
		break;
	case ISP_WB: {
			struct ak_isp_init_wb *p = (struct ak_isp_init_wb*)buf;

			ret = ak_isp_vp_get_wb_type(&p->wb_type);
			ret |= ak_isp_vp_get_mwb_attr(&p->p_mwb);
			ret |= ak_isp_vp_get_awb_attr(&p->p_awb);
			ret |= ak_isp_vp_get_awb_ex_attr(&p->p_awb_ex);
			*size = sizeof(struct ak_isp_init_wb);
		}
		break;
	case ISP_EXP: {
			struct ak_isp_init_exp *p = (struct ak_isp_init_exp*)buf;

			ret = ak_isp_vp_get_raw_hist_attr(&p->p_raw_hist);
			ret |= ak_isp_vp_get_rgb_hist_attr(&p->p_rgb_hist);
			ret |= ak_isp_vp_get_yuv_hist_attr(&p->p_yuv_hist);
			ret |= ak_isp_vp_get_exp_type(&p->p_exp_type);
			ret |= ak_isp_vp_get_frame_rate(&p->p_frame_rate);
			ret |= ak_isp_vp_get_ae_attr(&p->p_ae);
			*size = sizeof(struct ak_isp_init_exp);
		}
		break;
	case ISP_MISC: {
			struct ak_isp_init_misc *p = (struct ak_isp_init_misc*)buf;

			ret = ak_isp_vo_get_misc_attr(&p->p_misc);
			*size = sizeof(struct ak_isp_init_misc);
		}
		break;
	case ISP_Y_GAMMA: {
			struct ak_isp_init_y_gamma *p = (struct ak_isp_init_y_gamma*)buf;

			ret =  ak_isp_vp_get_y_gamma_attr(&p->p_gamma_attr);
			*size = sizeof(struct ak_isp_init_y_gamma);
		}
		break;
	case ISP_HUE: {
			struct ak_isp_init_hue *p = (struct ak_isp_init_hue*)buf;

			ret =  ak_isp_vp_get_hue_attr(&p->p_hue);
			*size = sizeof(struct ak_isp_init_hue);
		}
		break;
	case ISP_FPS_CTRL: {
			AK_ISP_FRAME_RATE_ATTR *p = (AK_ISP_FRAME_RATE_ATTR *)buf;
			ret =  ak_isp_vp_get_frame_rate(p);
			*size = sizeof(AK_ISP_FRAME_RATE_ATTR);
		}
		break;
	default:
		ak_print_error_ex("param err\n");
		return -1;
		break;
	}

	/* isp basic setting need to set,other no need to set */
	if (module_id <= ISP_MISC) {
		memcpy(buf, &module_id, ISP_MODULE_ID_SIZE);
		memcpy(buf+ISP_MODULE_ID_SIZE, size, ISP_MODULE_LEN_SIZE);
	}
	return ret;
}

static int get_cmd(PTRANS_DATA cmd, unsigned char* buf)
{
	unsigned int size = 0;

	if (NULL == cmd || NULL == buf) {
		printf("get_cmd param err\n");
		return 0;
	}
	//ak_print_notice_ex("get_cmd attr_id=%d\n", cmd->head.attr_id);
	if (cmd->head.attr_id <= ISP_HUE) {
		isp_get_attr(cmd->head.attr_id, buf + ISP_PARM_LEN_SIZE, &size);
		memcpy(buf, &size, ISP_PARM_LEN_SIZE);
	} else if (cmd->head.attr_id >= ISP_3DSTAT
			&& cmd->head.attr_id <= ISP_AWBSTAT) {
		isp_get_statinfo(cmd->head.attr_id, buf + ISP_PARM_LEN_SIZE, &size);
		memcpy(buf, &size, ISP_PARM_LEN_SIZE);
	} else if (cmd->head.attr_id >= ISP_RAW_IMG
			&& cmd->head.attr_id <= ISP_ENCODE_IMG) {
		ImgData_Get(cmd->head.attr_id, buf + ISP_PARM_LEN_SIZE, &size);
		memcpy(buf, &size, ISP_PARM_LEN_SIZE);
	} else if (cmd->head.attr_id >= ISP_PARM_CODE
			&& cmd->head.attr_id <= ISP_REGISTER) {
		unsigned long cnt = 0;
		unsigned long i = 0;
		unsigned short addr = 0;
		unsigned char  addrlen = 0;
		unsigned short value = 0;
		unsigned char  valuelen = 2;
		unsigned char* p = NULL;
		unsigned char* pdst = buf + ISP_PARM_LEN_SIZE;

		memcpy(&cnt, cmd->data, ISP_PARM_CNT_SIZE);
		printf("cnt = %lu\n", cnt);
		memcpy(pdst, &cnt, ISP_PARM_CNT_SIZE);
		pdst += ISP_PARM_CNT_SIZE;
		size += ISP_PARM_CNT_SIZE;

		p = cmd->data+ ISP_PARM_CNT_SIZE;

		for (i=0; i<cnt; i++) {
			memcpy(&addrlen, p, 1);
			p += 1;
			memcpy(&addr, p, addrlen);
			p += addrlen;

			printf("addr = 0x%x\n", addr);

			//get
			//isp_get_sensor_value(addr, &value);
			AK_ISP_SENSOR_REG_INFO sensor_reg;

			sensor_reg.reg_addr = addr;
			sensor_reg.value = value;
			akisp_ioctl(AK_ISP_GET_SENSOR_REG, &sensor_reg);
			value = sensor_reg.value;
			
			memcpy(pdst, &addrlen, 1);
			pdst += 1;
			size += 1;

			memcpy(pdst, &addr, addrlen);
			pdst += addrlen;
			size += addrlen;

			memcpy(pdst, &valuelen, 1);
			pdst += 1;
			size += 1;

			memcpy(pdst, &value, valuelen);
			pdst += valuelen;
			size += valuelen;
		}

		memcpy(buf, &size, ISP_PARM_LEN_SIZE);
	} else if (ISP_CFG_DATA == cmd->head.attr_id) {

		size = ISP_CFG_MAX_SIZE - ISP_PACKET_HEAD_SIZE - ISP_PARM_LEN_SIZE;
		
		isp_cfg_file_load((char *)(buf + ISP_PARM_LEN_SIZE),&size);

		
		memcpy(buf, &size, ISP_PARM_LEN_SIZE);
	} else if (ISP_HEARTBEAT == cmd->head.attr_id) {
		char str[] = "isp heartbeat!";

		size = strlen(str);

		memcpy(buf+ISP_PARM_LEN_SIZE, str, size);
		memcpy(buf, &size, ISP_PARM_LEN_SIZE);

		//set_check_timer(HEARTBEAT_CHECKTIME);
	}

	return 0;
}

static int write_ispcfg_partition(char *p_data, unsigned long p_data_len)
{
	int ret=0, w_ret=0;
	void *handle = NULL;
	unsigned long partition_size;

	ak_print_notice_ex("write_ispcfg_partition:len=%d\n", p_data_len);
	
	// OPEN partition
	handle = (void *)ak_partition_open(ISP_CONF_PATH);
	if(handle == NULL)
	{
		printf("ak_partition_open err!\r\n");
		ret = -1;
		goto ERR;
	}

	// get partition size 
	partition_size = ak_partition_get_size(handle);

	if(partition_size < p_data_len )
	{
		ak_print_error_ex("new ispcfg file more than isp conf partition size.\n");
		ret = -1;
		goto ERR1;
	}

	//write data to partition
	w_ret = ak_partition_write(handle, p_data, p_data_len);
	if(w_ret < p_data_len)
	{
		ak_print_error_ex("new ispcfg file more than isp conf partition size.\n");
		ret = -1;
		goto ERR1;
	}
	ak_print_normal("new ispcfg write to partition success.\n");
ERR1:

	// close partition
	ak_partition_close(handle);
	handle==NULL;
ERR:	
	return ret;
}
static int isp_cfg_file_store(char *cmd_data, unsigned long cmd_datalen)
{
	void *handle = NULL;
	int pr_ret=0, ret = 0, i=0;
	unsigned long all_count = 0,all_partition_count = 0, new_all_count=0;

	CFGFILE_HEADINFO *p_isp_conf_header;
	unsigned long tmp_len = 0, read_len = 0;
	unsigned long sensor_id;
	char *p_data=NULL, *p_parti_data=NULL;

	ak_print_notice_ex("=====version=%d\n", *((int*)cmd_data));
	// OPEN partition
	handle = (void *)ak_partition_open(ISP_CONF_PATH); 
	
	if(handle == NULL)
	{
		printf("ak_partition_open err!\r\n");
		return -1;
	}

	// get partition size and all data size
	all_count = ak_partition_get_dat_size(handle);
	all_partition_count = ak_partition_get_size(handle);
	printf("ymx : dat_size=%d,all_partition_count=%d\r\n", all_count, all_partition_count);
	if( 0 == all_count || 0 == all_partition_count )
	{
		printf("ISPCFG  partition get data fail\r\n");
		ret = -1;
		goto ERR1;
	}

	p_data = malloc(all_count);
	p_parti_data = malloc(all_partition_count);
	if(NULL == p_data)
	{
		printf("isp conf buffer malloc fail\n");
		ret = -1;
		goto ERR1;
	}
	if(NULL == p_parti_data)
	{
		free(p_data);
		p_data = NULL;
		
		printf("isp conf parti buffer malloc fail\n");
		ret = -1;
		goto ERR1;
	}
	
	//read all isp config from partition
	pr_ret = ak_partition_read(handle, p_data, all_count);

	
	if((pr_ret < 0) || (pr_ret != (all_count)))
	{
		printf("ak_partition_read err! read=%d,real=%d\r\n",all_count,pr_ret);
		ret = -1;
		goto ERR2;
	}
	printf("ak_partition_read ! read=%d,real=%d\r\n",pr_ret,all_count);

	int *p_version =  (int *)p_data;
	printf("version=%d\r\n",*p_version);

	/* get one sensor config file */
	p_isp_conf_header = (CFGFILE_HEADINFO *)p_data;
	
	sensor_id = cam_get_sensor_id();

	for(i = 0; i < all_count; i = tmp_len)
	{

		printf("ID %x,size:%d\r\n",sensor_id,p_isp_conf_header->subFileLen);

		
		if(sensor_id == p_isp_conf_header->sensorId)
		{
			CFGFILE_HEADINFO *tmp_header = (CFGFILE_HEADINFO *)cmd_data;
			tmp_header->file_version[5]='\0';
			
			ak_print_notice_ex("version:%s\n", *((int*)tmp_header));
			ak_print_notice_ex("file_version:%s\n", tmp_header->file_version);
			memcpy(p_parti_data+ tmp_len, cmd_data, cmd_datalen);

			ak_print_notice_ex("new_all_count=%d + %d,sensorId:%d\n", new_all_count, p_isp_conf_header->sensorId);
			new_all_count += cmd_datalen;
			/* skip one sensor config file  */
			tmp_len +=  p_isp_conf_header->subFileLen;
			p_isp_conf_header = (CFGFILE_HEADINFO *)(p_data + tmp_len);
			
			tmp_len +=  p_isp_conf_header->subFileLen; //白天和黑夜配置长度有可能不一样
			p_isp_conf_header = (CFGFILE_HEADINFO *)(p_data + tmp_len);

		}else{
			char * tmp = (char *)p_isp_conf_header;

			read_len =  p_isp_conf_header->subFileLen;
			//tmp_len += read_len;
			p_isp_conf_header = (CFGFILE_HEADINFO *)(p_data + tmp_len);
			
			read_len +=  p_isp_conf_header->subFileLen; 
			//tmp_len += read_len;
		
			//copy old data and new sensor data from isptool to p_parti_data
			memcpy(p_parti_data+ tmp_len, tmp, read_len);

			ak_print_notice_ex("2new_all_count=%d + %d\n", new_all_count, read_len);
			new_all_count += read_len;//read_len is one sensor config size.
			tmp_len +=  read_len;
			p_isp_conf_header = (CFGFILE_HEADINFO *)(p_data + tmp_len);
		}
		ak_print_notice_ex("tmplen=%d\n",tmp_len);

	}
	goto ERR1;
ERR2:	
	free(p_parti_data);
	p_parti_data = NULL;

	free(p_data);
	p_data = NULL;
	
ERR1:	
	ak_partition_close(handle);
	handle = NULL;

	if(ret == 0 )
		// write new sensor data to partition
		ret = write_ispcfg_partition(p_parti_data, new_all_count);
	else
		goto FUN_END;
ERR0:
	
	free(p_parti_data);
	p_parti_data = NULL;
ERR:
	free(p_data);
	p_data = NULL;
FUN_END:
	return ret;
}



static int set_cmd(PTRANS_DATA cmd)
{
	int ret = 0;

	if (NULL == cmd) {
		printf("set_cmd param err\n");
		return -1;
	}

	if (cmd->head.attr_id <= ISP_HUE) {
		ret = isp_set_attr(cmd->head.attr_id, (char *)cmd->data, cmd->datalen);
	}
	else if (cmd->head.attr_id >= ISP_PARM_CODE && cmd->head.attr_id <= ISP_REGISTER) {
		unsigned long cnt = 0;
		unsigned long i = 0;
		unsigned short addr = 0;
		unsigned char  addrlen = 0;
		unsigned short value = 0;
		unsigned char  valuelen = 0;
		unsigned char* p = NULL;

		memcpy(&cnt, cmd->data, ISP_PARM_CNT_SIZE);
		printf("cnt = %lu\n", cnt);

		p = cmd->data + ISP_PARM_CNT_SIZE;

		for (i=0; i<cnt; i++) {
			memcpy(&addrlen, p, 1);
			p += 1;
			memcpy(&addr, p, addrlen);
			p += addrlen;

			memcpy(&valuelen, p, 1);
			p += 1;
			memcpy(&value, p, valuelen);
			p += valuelen;

			printf("addr = 0x%x, value = %d\n", addr, value);
			//set
			//ret |= isp_set_sensor_value(addr, value);
			AK_ISP_SENSOR_REG_INFO sensor_reg;

			sensor_reg.reg_addr = addr;
			sensor_reg.value = value;

			//Ak_ISP_Sensor_Set_Reg(&sensor_reg);
			ret = akisp_ioctl(AK_ISP_SET_SENSOR_REG, &sensor_reg);
			
			//ak_print_notice_ex("ymx err,go here\n");
		}
	} else if (ISP_CFG_DATA == cmd->head.attr_id) {

		//ak_print_notice_ex("store file version=%d,data_len=%ld\n",*((unsigned int*)cmd->data), cmd->datalen);

		//ak_print_notice_ex("-----version=%d\n", *((int*)cmd_data));
		ret = isp_cfg_file_store((char *)(cmd->data), cmd->datalen);
	}

	return ret;
}

static void send_cmd(PCTL_HANDLE pctrl, struct cmd_node *cmd)
{
	ak_thread_mutex_lock(&pctrl->lock);
	list_add_tail(&cmd->list, &pctrl->queue);
	ak_thread_mutex_unlock(&pctrl->lock);

	ak_thread_sem_post(&pctrl->isp_sem);
}

static int exec_cmd(PCTL_HANDLE pctrl, PTRANS_DATA cmd)
{
	int cmd_type = cmd->head.cmd_type;
	int ret = 0;
	//unsigned char* send_buf = NULL;
	unsigned long send_size = 0;
	struct video_resolution res = {0};
	void *vi_handle = NULL;

	struct cmd_node *cmd_t = (struct cmd_node *)calloc(1,
			sizeof(struct cmd_node));
	if (!cmd_t) {
		ak_print_warning_ex("calloc failed\n");
		return -1;
	}
	// ak_print_notice_ex("ymx:cmd_type:%d\n",cmd_type);
	switch (cmd_type) {
		case CMD_GET:
		case CMD_GET_TXT:
			// ak_print_notice("ymx : isp get cmd, attr id : %d\n", cmd->head.attr_id);
			/* to get system arguments */
			vi_handle = ak_vi_get_handle(VIDEO_DEV0);
			ak_vi_get_sensor_resolution(vi_handle, &res);
			if ((cmd->head.attr_id >= ISP_RAW_IMG) && (cmd->head.attr_id <= ISP_ENCODE_IMG))
				cmd_t->send_buf = (unsigned char*)calloc(1, res.width*res.height*3/2 + ISP_PACKET_HEAD_SIZE + ISP_PARM_LEN_SIZE);
			else
				cmd_t->send_buf = (unsigned char*)calloc(1, ISP_CFG_MAX_SIZE);
			
			if (NULL == cmd_t->send_buf) {
				// mencius exit_err("send_buf");
				ak_print_error_ex("send_buf\n");
				return 0;
			}

			memcpy(cmd_t->send_buf, &cmd->head.attr_id, ISP_ATTR_ID_SIZE);
			if (CMD_GET == cmd_type) {
				cmd_t->send_buf[2] = CMD_REPLY;
			} else {
				cmd_t->send_buf[2] = CMD_REPLY_TXT;
			}

			get_cmd(cmd, cmd_t->send_buf + ISP_PACKET_HEAD_SIZE);
			send_cmd(pctrl, cmd_t);
			break;

		case CMD_SET:
			printf("ymx:isp set cmd, attr id : %d\n", cmd->head.attr_id);

			send_size = ISP_PACKET_HEAD_SIZE + ISP_PARM_LEN_SIZE + ISP_RET_SIZE;
			cmd_t->send_buf = (unsigned char*)calloc(1, send_size);

			if (NULL == cmd_t->send_buf) {
				//exit_err("send_buf");
				ak_print_error_ex("send_buf\n");
				return 0;
			}

			//memset(send_buf, 0, send_size);
			memcpy(cmd_t->send_buf, &cmd->head.attr_id, ISP_ATTR_ID_SIZE);
			cmd_t->send_buf[2] = CMD_RET;
			cmd_t->send_buf[4] = ISP_RET_SIZE;

			ret = set_cmd(cmd);
			memcpy(cmd_t->send_buf + ISP_PACKET_HEAD_SIZE + ISP_PARM_LEN_SIZE,
					&ret, ISP_RET_SIZE);
			send_cmd(pctrl, cmd_t);
			break;

		default:
			printf("cmd unrecognized\n");
			break;
	}

	return 0;

}

/*
 * isp_send_thread
 */
static void *isp_send_thread(void *arg)
{
	int ret;
	unsigned long size = 0;
	struct cmd_node *cmd = NULL;
	PCTL_HANDLE pctrl = (PCTL_HANDLE)arg;

	ak_print_normal_ex("isp cmd thread id: %ld\n", ak_thread_get_tid());
	while (its_run_flag) {

		ak_thread_sem_wait(&pctrl->isp_sem);
		if (!list_empty(&pctrl->queue)) {
			cmd = list_first_entry(&pctrl->queue, struct cmd_node, list);

			if (cmd->send_buf) {
				memcpy(&size, cmd->send_buf + ISP_PACKET_HEAD_SIZE, ISP_PARM_LEN_SIZE);
				size += ISP_PACKET_HEAD_SIZE + ISP_PARM_LEN_SIZE;

				ret = send(pctrl->sock, cmd->send_buf, size, 0);
				if (ret <= 0) {
					//exit_err("send");
					ak_print_error_ex("send error!\n");
				}

				ak_thread_mutex_lock(&pctrl->lock);
				free(cmd->send_buf);
				list_del(&cmd->list);
				free(cmd);
				ak_thread_mutex_unlock(&pctrl->lock);
			}
		}
	}

	ak_print_normal_ex("### thread id: %ld exit ###\n", ak_thread_get_tid());
	ak_thread_exit();
	return NULL;
}

static int isp_set_net_srv(void)
{
	/** create socket **/
	int sfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sfd < 0) {
		//exit_err("sfdet");
		ak_print_error_ex("sfdet\n");
	}

	/** initialize and fill in the message to the sfdet addr infor struct **/
	struct sockaddr_in server_addr;	/** server socket addr information struct **/
	memset(&server_addr, 0, sizeof(struct sockaddr));

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(ISP_PORT);
	server_addr.sin_addr.s_addr =INADDR_ANY;

	/** set socket attribute **/
	int sinsize = 1;//SOF_REUSEADDR;//1;
	int ret = setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &sinsize, sizeof(int));
	if (ret != 0) {//ret < 0
		close(sfd);
		//exit_err("setsockopt");
		ak_print_error_ex("setsockopt error.\n");
	}

	/** set close-on-exec flag **/
//	ret = fcntl(sfd, F_SETFD, FD_CLOEXEC);
//	if (ret < 0) {
//		close(sfd);
		//exit_err("fcntl");
//		ak_print_error_ex("fcntl\n");
//	}

	/** bind sfdet with this program **/
	ret = bind(sfd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_in));
	if (ret != 0) {
		close(sfd);
		//exit_err("bind");
		ak_print_error_ex("bind\n");
	}

	return sfd;
}
static void close_for_connect(void)
{
	ak_thread_mutex_lock(&pctrl_start->lock);
	struct cmd_node *cmd_t = NULL, *tmp = NULL;
	list_for_each_entry_safe(cmd_t, tmp, &pctrl_start->queue, list) {
		if (cmd_t->send_buf)
			free(cmd_t->send_buf);
		list_del(&cmd_t->list);
		free(cmd_t);
	}
	ak_thread_mutex_unlock(&pctrl_start->lock);
	ak_thread_mutex_destroy(&pctrl_start->lock);

	its_run_flag = AK_FALSE;
	ak_thread_sem_post(&pctrl_start->isp_sem);
	ak_thread_sem_destroy(&pctrl_start->isp_sem);

	free(pctrl_start);
	if (-1 != cfd) {
		close(cfd);
		cfd = -1;
	}
	ak_thread_join(respons_tid);
	respons_tid = NULL;
}
static int isp_srv_start(void)
{
	int ret; 	/** call function return val **/
	int sinsize; 	/** use for set socket option **/
	TRANS_DATA cmd;
	int recvlen = 0;
	int size = 0;
	T_HEAD_TYPE headret = HEAD_ERR;
	int is_tool_close=0;

	//mencius signal(SIGALRM, heartbeat_timeout);
	//register signal
    //cmd_signal(CMD_SIGALRM, heartbeat_timeout);

	sfd = isp_set_net_srv();
	if (sfd < 0) {
		ak_print_warning_ex("set net srv failed\n");
		return -1;
	}

	/** listen, create wait_queue **/
	ret = listen(sfd, 1);	/* only one client can connect to this server */
	if (ret < 0) {
		close(sfd);
		//exit_err("listen");
		ak_print_error_ex("listen error\n");
		return -1;
	}

	struct sockaddr_in peer_addr;		/** server socket addr information struct **/
	memset(&peer_addr, 0, sizeof(struct sockaddr));
	//accept
	while (1) {
		ak_print_notice_ex("waiting for connect ...\n");

		sinsize = sizeof(struct sockaddr);
		/** accept the client to connect **/
		cfd = accept(sfd, (struct sockaddr *)&peer_addr, (socklen_t *)&sinsize);
		if (cfd < 0) {
			close(sfd);
			//exit_err("accept");
			ak_print_error_ex("accept error\n");

			return -1;
		}
		printf("Client connected, socket cfd = %d\n", cfd);

		/** init the handle and others **/
		pctrl_start= calloc(1, sizeof(CTL_HANDLE));
		pctrl_start->sock = cfd;
		ak_thread_sem_init(&pctrl_start->isp_sem, 0);
		INIT_LIST_HEAD(&pctrl_start->queue);
		ak_thread_mutex_init(&pctrl_start->lock);

		/** command parse thread **/
		if (ak_thread_create(&respons_tid, isp_send_thread, pctrl_start, ANYKA_THREAD_MIN_STACK_SIZE, 85))
		{
			//ak_thread_detach(respons_tid);
		}
		//set_check_timer(HEARTBEAT_CHECKTIME);

		/** receive data **/
		while (its_run_flag) {
			size = ISP_PACKET_HEAD_SIZE;
			memset(&cmd, 0, sizeof(TRANS_DATA));

			ak_print_notice_ex("waiting for recv head...\n");

			ret = recv(cfd, &cmd.head, size, 0);
			// ak_print_notice_ex("ymx: recv:%d\n",ret);

			if(ret <= 0)
			{	
				if(ret == 0)
				{
					is_tool_close = 1;
				}
				//ak_print_notice_ex("break\n");
				break;
			}
			headret = check_cmd_head(&cmd);

			// ak_print_notice_ex("ymx:headret %d\n",headret);
			if (HEAD_ERR == headret) {
				continue;
			} else if (HEAD_HAVE_DATA == headret) {
				size = ISP_PARM_LEN_SIZE;
				ret = recv(cfd, &cmd.datalen, size, 0);
				// ak_print_notice_ex("ymx:recv2 %d\n",ret);
				if(ret <= 0) {
					break;
				}
				ak_print_notice_ex("cmd.datalen = %lu\n", cmd.datalen);

				recvlen = 0;
				size = cmd.datalen;
				cmd.data = (unsigned char *)calloc(1, size);
				if (NULL == cmd.data) {
					break;
				}

				memset(cmd.data, 0, size);
				while (recvlen < cmd.datalen) {
					ret = recv(cfd, cmd.data + recvlen, size, 0);
					if(ret <= 0) {
						goto RECV_ERROR;
					}
					recvlen += ret;
					size -= ret;
					printf("recvlen = %d\n", recvlen);
				}
			}

			exec_cmd(pctrl_start, &cmd);
			if (NULL != cmd.data) {
				free(cmd.data);
				cmd.data = NULL;
			}
		}
		

RECV_ERROR:
		ak_print_notice_ex("recv error, ret = %d\n", ret);
	
		close_for_connect();
		
		if (NULL != cmd.data) {
			free(cmd.data);
			cmd.data = NULL;
		}
		// ak_print_normal("ymx:is_close:%d\n",is_tool_close);
		if(is_tool_close)
		{
			its_run_flag = AK_TRUE;
			is_tool_close = 0;
		}
		//set_check_timer(0);
	}

	if(sfd != -1)
	{	
		close(sfd);
		sfd = -1;
	}
	ak_print_normal_ex("isp server exit\n");
	return 0;
}

static void *isp_tool_server(void *arg)
{
	long int tid = ak_thread_get_tid();
	ak_print_normal_ex("thread tid: %ld\n", tid);

	isp_srv_start();

	ak_print_normal_ex("exit thread tid: %ld\n", tid);
	ak_thread_exit();
	return NULL;
}

/**  
 * ak_its_start - start anyka ISP tool server
 * notes: If you wan't use ISP tool to debug, do not start this server.
 */
void ak_its_start(void)
{
	if (!its_run_flag) {
		
		its_run_flag = AK_TRUE;
		ak_thread_create(&isp_svr_tid, isp_tool_server, NULL, 
			ANYKA_THREAD_MIN_STACK_SIZE, -1);

		ak_print_notice_ex("start to its,thread id=%d\n",isp_svr_tid);
	}
}


/**  
 * ak_its_stop - stop anyka ISP tool server
 * notes:
 */
void ak_its_stop(void)
{
	ak_print_notice_ex("its_run_flag=%d\n",its_run_flag);
	if(its_run_flag)
	{	
		its_run_flag = AK_FALSE;
		ak_thread_cancel(isp_svr_tid);
		
	}

	ak_thread_join(isp_svr_tid);

	if(respons_tid != NULL)
	{
		close_for_connect();
	}
	if(sfd != -1)
	{	
		close(sfd);
		sfd = -1;
	}
	if (-1 != cfd)
	{
		close(cfd);
		cfd = -1;
	}
	
}

#else

void ak_its_start(void)
{
}
void ak_its_stop(void)
{
}
#endif
