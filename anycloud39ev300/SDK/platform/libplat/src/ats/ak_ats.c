#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <getopt.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <stdbool.h>

#include "ak_common.h"
#include "ak_thread.h"

#include "list.h"
#include "ak_ai.h"
#include "ak_ao.h"

#define AUDIO_TOOL_ATTR_ID_SIZE		2	//sub file number
#define AUDIO_TOOL_CMD_TYPE_SIZE		2

#define AUDIO_TOOL_PACKET_HEAD_SIZE	(AUDIO_TOOL_ATTR_ID_SIZE + AUDIO_TOOL_CMD_TYPE_SIZE)	//head size
#define AUDIO_TOOL_PARM_LEN_SIZE		4
#define AUDIO_TOOL_RET_SIZE			4

#define AUDIO_ATTR_ID_SIZE		2
#define AUDIO_CMD_TYPE_SIZE		2

#define HEARTBEAT_CHECKTIME		(15)
#define PCM_READ_LEN		4096		/* ao play buffer length */
#define SAMPLE_RATE         8000
#define VRSION_STR_LEN      50


#define exit_err(msg) \
	do{ perror(msg); return 0; } while(0);


/* command head define */
typedef struct _cmd_head
{
	unsigned short attr_id;
	unsigned short cmd_type;
}CMD_HEAD;

/* head state define */
typedef enum {
	HEAD_ERR = 0,
	HEAD_NO_DATA,
	HEAD_HAVE_DATA,
	HEAD_TYPE_NUM
} T_HEAD_TYPE;

/* global handle */
typedef struct _ctl_handle
{
	int		sock;				//socket handle
	sem_t	isp_sem;			//operate semaphore
	struct list_head queue;		//command queue
	ak_mutex_t lock;			//mutex lock
}CTL_HANDLE, *PCTL_HANDLE;

/* transform data node */
typedef struct _trans_data
{
	CMD_HEAD	head;
	unsigned long		datalen;
	unsigned char* 		data;
}TRANS_DATA, *PTRANS_DATA;

struct ak_audio_volume_attr 
{
    int adc_gain;  //0~8
    int adc_volume;   //-20~20
    int dac_gain;  //0~6
    int dac_volume; //-20~20 
    struct ak_audio_aslc_attr m_adc_aslc;
	struct ak_audio_aslc_attr m_dac_aslc;
};


struct ak_audio_adc_dac_nr_attr
{
	struct ak_audio_nr_attr adc_nr_attr;

    struct ak_audio_nr_attr dac_nr_attr;
};

struct ak_audio_vqe_attr 
{
	struct ak_audio_aec_attr aec_attr;
    struct ak_audio_agc_attr agc_attr;
    struct ak_audio_adc_dac_nr_attr nr_attr;
};


struct ak_audio_aec_file_attr 
{
	unsigned char aec_enable;// 新增的数据结构必须放在	T_U8 aec_enable的前面
};

//enable aec銆乤gc銆乶r
struct ak_audio_enable
{
	int aec_enable;
	int agc_enable;
	int nr_enable;
};
	
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
	AUDIO_AEC = 0,   			
	AUDIO_AGC,				
	AUDIO_EQ_ADC,
	AUDIO_EQ_DAC,
	AUDIO_VOLUME,	
	AUDIO_HEARTBEAT,
	AUDIO_FILE,
	AUDIO_NR,
	AUDIO_DEBUG_INFO,
	AUDIO_DATA_SAVE,
	AUDIO_VERSION,
	AUDIO_VQE,
	AUDIO_GET_PARAMS,
	AUDIO_CHIP_TYPE,
	AUDIO_ATTR_TYPE_NUM
} T_ATTR_TYPE;

/* command node define */
struct cmd_node {
	unsigned char *send_buf;
	struct list_head list;
};


struct audio_tool_data 
{
	unsigned char aec_enable;
    unsigned char* true_data;
};

typedef enum {
    AK_V200 = 0x200,
    AK_V300 = 0x300,
    AK_DEFAULT = 0x500,
    AK_CHIP_TOP
}
T_CHIP_TYPE;

struct audio_version {
    char ai_version[VRSION_STR_LEN];
    char ao_version[VRSION_STR_LEN];
    char aenc_version[VRSION_STR_LEN];
    char adec_version[VRSION_STR_LEN];
    char audio_filter_version[VRSION_STR_LEN];
    char audio_code_version[VRSION_STR_LEN];
};


static int cfd = -1;	/** socket file descriptor **/
static int sfd = -1;	/** server socket file descriptor **/
static unsigned char ats_run_flag = AK_FALSE;	//global run flag, audio tool run flag
static unsigned int port_id = 8012;
static ak_pthread_t isp_svr_tid = 0;
static void *ai_handle = NULL;
static void *ao_handle = NULL;
static const char audio_tool_version[] = "libapp_ats V1.0.09";

/**
 * brief: set check timer, to check heartbeat timeout
 * @sec[IN]:second
 * return: void
 * notes:
 */
static void set_check_timer(int sec)
{
	struct itimerval s_time;

	/* just use secord scope */
	s_time.it_value.tv_sec  = sec;
	s_time.it_value.tv_usec = 0;
	s_time.it_interval.tv_sec  = 0;
	s_time.it_interval.tv_usec = 0;

	setitimer(ITIMER_REAL, &s_time, NULL);

	return ;
}

/**
 * brief: check cmd head info
 * @cmd[IN]:cmd info
 * return: enum T_HEAD_TYPE
 * notes:
 */
static T_HEAD_TYPE check_cmd_head(PTRANS_DATA cmd)
{    
	/* first check attr id */
	if (AUDIO_HEARTBEAT != cmd->head.attr_id)
		ak_print_info( "attr_id : %d, cmd_type : %d\n",
				cmd->head.attr_id, cmd->head.cmd_type);
	/* attr id error */
	if ((cmd->head.attr_id >= AUDIO_ATTR_TYPE_NUM)
			|| ((cmd->head.cmd_type != CMD_GET)
				&& (cmd->head.cmd_type != CMD_SET)
				&& (cmd->head.cmd_type != CMD_GET_TXT))) {
		ak_print_warning_ex( "recv attr id or cmd type error:%d, %d!\n",
				cmd->head.attr_id, cmd->head.cmd_type);
		return HEAD_ERR;
	}

    	/* check whether it is data */
	if (CMD_SET == cmd->head.cmd_type) {
		return HEAD_HAVE_DATA;
	}
	return HEAD_NO_DATA;
}

/**
 * brief: get cmd deal
 * @cmd[IN]:cmd info
 * @buf[OUT]:buf to load data
 * return: 
 * notes:
 */
static int get_cmd(PTRANS_DATA cmd, unsigned char* buf)
{
	unsigned int size = 0;

	if (NULL == cmd || NULL == buf) {
		printf("get_cmd param err\n");
		return 0;
	}
    
    char str[] = "audio tool heartbeat!";
    struct ak_audio_aec_attr aec_attr;
    struct ak_audio_agc_attr agc_attr;
    struct ak_audio_eq_attr eq_attr;
    struct ak_audio_volume_attr volume_attr;
    struct ak_audio_nr_attr nr_attr;

    struct ak_audio_vqe_attr vqe_attr;
    struct pcm_param data_attr;
    int chip_id = 0;
    switch (cmd->head.attr_id) {
    case AUDIO_HEARTBEAT:
        /*reply heartbeat cmd*/   
		size = strlen(str);
		memcpy(buf+AUDIO_TOOL_PARM_LEN_SIZE, str, size);
		memcpy(buf, &size, AUDIO_TOOL_PARM_LEN_SIZE);
		set_check_timer(HEARTBEAT_CHECKTIME);
        break;

    case AUDIO_AEC:     
        ak_print_normal_ex( "get aec.......\n");
        memset(&aec_attr, 0, sizeof(struct ak_audio_aec_attr));
        if (NULL == ai_handle)
            ak_ai_get_handle(0, &ai_handle);
        
        ak_ai_get_aec_attr(ai_handle, &aec_attr);
    	ak_print_normal_ex("audio_in_digi_gain=%d, audio_out_digi_gain=%d,tail=%d,enable=%d\n",
        aec_attr.audio_in_digi_gain,aec_attr.audio_out_digi_gain,aec_attr.tail,aec_attr.enable);
        size = sizeof(struct ak_audio_aec_attr);

        memcpy(buf+AUDIO_TOOL_PARM_LEN_SIZE, &aec_attr, sizeof(struct ak_audio_aec_attr));
        memcpy(buf, &size, AUDIO_TOOL_PARM_LEN_SIZE);
        break;

    case AUDIO_AGC:
        ak_print_normal_ex( "get agc.......\n");
        memset(&agc_attr, 0, sizeof(struct ak_audio_agc_attr));
        if (NULL == ai_handle)
            ak_ai_get_handle(0, &ai_handle);

        ak_ai_get_agc_attr(ai_handle, &agc_attr);
        ak_print_notice_ex("agc_level=%ld, agc_max_gain=%d, agc_min_gain=%d, near_sensitivity=%d\n",
        agc_attr.agc_level, agc_attr.agc_max_gain, agc_attr.agc_min_gain, agc_attr.near_sensitivity);
        size = sizeof(struct ak_audio_agc_attr);       

        memcpy(buf+AUDIO_TOOL_PARM_LEN_SIZE, &agc_attr, sizeof(struct ak_audio_agc_attr));
        memcpy(buf, &size, AUDIO_TOOL_PARM_LEN_SIZE);
        break;

    case AUDIO_NR:
        ak_print_normal_ex("get nr.......\n");
        memset(&nr_attr, 0, sizeof(struct ak_audio_nr_attr));
        if (NULL == ai_handle)
            ak_ai_get_handle(0, &ai_handle);

        ak_ai_get_nr_attr(ai_handle, &nr_attr);

        ak_print_notice_ex("noise_suppress_db=%d\n", nr_attr.noise_suppress_db);
        size = sizeof(struct ak_audio_nr_attr);

        memcpy(buf+AUDIO_TOOL_PARM_LEN_SIZE, &nr_attr, sizeof(struct ak_audio_nr_attr));
        memcpy(buf, &size, AUDIO_TOOL_PARM_LEN_SIZE);
        break;

    case AUDIO_EQ_ADC:
        ak_print_normal_ex( "get adc eq.......\n");
        memset(&eq_attr, 0, sizeof(struct ak_audio_eq_attr));
        if (NULL == ai_handle)
            ak_ai_get_handle(0, &ai_handle);

        ak_ai_get_eq_attr(ai_handle, &eq_attr);
        size = sizeof(struct ak_audio_eq_attr);

        ak_print_notice_ex( "pre_gain=%d,bands=%ld,smoothEna=%d,smoothTime =%d, dcRmEna=%d,dcfb=%ld,aslc_ena=%d,aslc_level_max=%d\n",
             eq_attr.pre_gain, eq_attr.bands, eq_attr.smoothEna, eq_attr.smoothTime,
             eq_attr.dcRmEna, eq_attr.dcfb, eq_attr.aslc_ena, eq_attr.aslc_level_max);
        for (int i = 0; i < EQ_MAX_BANDS; i++)
        {
            ak_print_notice_ex( "bandfreqs[%d]=%ld,bandgains[%d] =%d, bandQ[%d]=%d,band_types[%d]=%d\n",
             i,eq_attr.bandfreqs[i], i,eq_attr.bandgains[i], i,
             eq_attr.bandQ[i], i, eq_attr.band_types[i]);
        }

        memcpy(buf+AUDIO_TOOL_PARM_LEN_SIZE, &eq_attr, sizeof(struct ak_audio_eq_attr));
        memcpy(buf, &size, AUDIO_TOOL_PARM_LEN_SIZE);
        break;
        
    case AUDIO_EQ_DAC:
        ak_print_normal_ex( "get adc eq.......\n");
        memset(&eq_attr, 0, sizeof(struct ak_audio_eq_attr));
        if (NULL == ao_handle)
            ak_ao_get_handle(0, &ao_handle);

        ak_ao_get_eq_attr(ao_handle, &eq_attr);
        size = sizeof(struct ak_audio_eq_attr);

        ak_print_notice_ex( "pre_gain=%d,bands=%ld,smoothEna=%d,smoothTime =%d, dcRmEna=%d,dcfb=%ld,aslc_ena=%d,aslc_level_max=%d\n",
             eq_attr.pre_gain, eq_attr.bands, eq_attr.smoothEna, eq_attr.smoothTime,
             eq_attr.dcRmEna, eq_attr.dcfb, eq_attr.aslc_ena, eq_attr.aslc_level_max);
        for (int i = 0; i < EQ_MAX_BANDS; i++)
        {
            ak_print_notice_ex( "bandfreqs[%d]=%ld,bandgains[%d] =%d, bandQ[%d]=%d,band_types[%d]=%d\n",
             i,eq_attr.bandfreqs[i], i,eq_attr.bandgains[i], i,
             eq_attr.bandQ[i], i, eq_attr.band_types[i]);
        }

        memcpy(buf+AUDIO_TOOL_PARM_LEN_SIZE, &eq_attr, sizeof(struct ak_audio_eq_attr));
        memcpy(buf, &size, AUDIO_TOOL_PARM_LEN_SIZE);
        break;

    case AUDIO_VOLUME:
        ak_print_normal_ex( "get volume.......\n");
        memset(&volume_attr, 0, sizeof(struct ak_audio_volume_attr));
        if (NULL == ai_handle)
            ak_ai_get_handle(0, &ai_handle);
        
        if (NULL == ao_handle)
            ak_ao_get_handle(0, &ao_handle);

        volume_attr.adc_gain = ak_ai_get_adc_volume(ai_handle);
        volume_attr.adc_volume = ak_ai_get_aslc_volume(ai_handle);
        volume_attr.dac_gain = ak_ao_get_dac_volume(ao_handle);
        volume_attr.dac_volume = ak_ao_get_aslc_volume(ao_handle);
        size = sizeof(struct ak_audio_volume_attr);
        ak_print_warning_ex("adc_gain=%d, adc_volume=%d, dac_gain=%d, dac_volume=%d, adc_limit=%ld, adc_aslc_db=%ld,dac_limit=%ld, dac_aslc_db=%ld\n", 
            volume_attr.adc_gain, volume_attr.adc_volume,
            volume_attr.dac_gain, volume_attr.dac_volume,volume_attr.m_adc_aslc.limit,volume_attr.m_adc_aslc.aslc_db,volume_attr.m_dac_aslc.limit,volume_attr.m_dac_aslc.aslc_db);
        
       
        memcpy(buf+AUDIO_TOOL_PARM_LEN_SIZE, &volume_attr, sizeof(struct ak_audio_volume_attr));
        memcpy(buf, &size, AUDIO_TOOL_PARM_LEN_SIZE);
        ak_print_normal_ex( "--size=%d\n", size);
        break;

    case AUDIO_FILE:
        ak_print_normal_ex( "get aec file.......\n");
        break;

    case AUDIO_VQE:
		ak_print_normal_ex("get aec.......\n");
        memset(&vqe_attr.aec_attr, 0, sizeof(struct ak_audio_aec_attr));
        if (NULL == ai_handle)
            ak_ai_get_handle(0, &ai_handle);
		if (NULL == ao_handle)
			ak_ao_get_handle(0, &ao_handle);
		//aec
        ak_ai_get_aec_attr(ai_handle, &vqe_attr.aec_attr);
    	ak_print_normal_ex("audio_in_digi_gain=%d, audio_out_digi_gain=%d,tail=%d,enable=%d\n",
        vqe_attr.aec_attr.audio_in_digi_gain,vqe_attr.aec_attr.audio_out_digi_gain,vqe_attr.aec_attr.tail,vqe_attr.aec_attr.enable);
        
        //agc
		ak_print_normal_ex("get agc.......\n");
        memset(&vqe_attr.agc_attr, 0, sizeof(struct ak_audio_agc_attr));
        
        ak_ai_get_agc_attr(ai_handle, &vqe_attr.agc_attr);
        
    	ak_print_normal_ex("agc_level=%ld, agc_max_gain=%d, agc_min_gain=%0.1f, near_sensitivity=%d,enable=%d\n",
       vqe_attr.agc_attr.agc_level, vqe_attr.agc_attr.agc_max_gain, (float)vqe_attr.agc_attr.agc_min_gain/1024, vqe_attr.agc_attr.near_sensitivity,vqe_attr.agc_attr.enable);
   
		//nr
		
		ak_print_normal_ex("get nr.......\n");
		memset(&vqe_attr.nr_attr, 0, sizeof(struct ak_audio_adc_dac_nr_attr));

        ak_ai_get_nr_attr(ai_handle, &(vqe_attr.nr_attr.adc_nr_attr));
        
    	ak_print_normal_ex("adc noise_suppress_db=%d,adc enable=%d,dac noise_suppress_db=%d,dac enable=%d\n", 
                vqe_attr.nr_attr.adc_nr_attr.noise_suppress_db,vqe_attr.nr_attr.adc_nr_attr.enable,vqe_attr.nr_attr.dac_nr_attr.noise_suppress_db,vqe_attr.nr_attr.dac_nr_attr.enable);                           
       
		size =sizeof(struct ak_audio_vqe_attr);
		memcpy(buf+AUDIO_TOOL_PARM_LEN_SIZE, &vqe_attr,sizeof(struct ak_audio_vqe_attr)); 	
		memcpy(buf, &size, AUDIO_TOOL_PARM_LEN_SIZE);

		break;

    case AUDIO_GET_PARAMS:
        memset(&data_attr, 0, sizeof(struct pcm_param));
        if (NULL == ai_handle) {
            ak_ai_get_handle(0, &ai_handle);
        }
        
        ak_ai_get_params(ai_handle, &data_attr);
        size =sizeof(struct pcm_param);
	    memcpy(buf+AUDIO_TOOL_PARM_LEN_SIZE, &data_attr,sizeof(struct pcm_param)); 	
	    memcpy(buf, &size, AUDIO_TOOL_PARM_LEN_SIZE);
        break;

    case AUDIO_CHIP_TYPE:
        chip_id = AK_V300;
        size = sizeof(int);
	    memcpy(buf + AUDIO_TOOL_PARM_LEN_SIZE, &chip_id, sizeof(int)); 	
	    memcpy(buf, &size, AUDIO_TOOL_PARM_LEN_SIZE);
        break;         
    default:

        break;
    }
	return 0;
}

/**
 * brief: set cmd deal
 * @cmd[IN]:cmd info
 * return: 
 * notes:
 */
static int set_cmd(PTRANS_DATA cmd)
{
	int ret = 0;

	if (NULL == cmd) {
		ak_print_error_ex("set_cmd param err\n");
		return -1;
	}
    ak_print_warning_ex( "isp set cmd, attr id : %d\n", 
                        cmd->head.attr_id);

    struct ak_audio_aec_attr aec_attr;
    struct ak_audio_agc_attr agc_attr;
    struct ak_audio_eq_attr eq_attr;
    struct ak_audio_volume_attr volume_attr;
    unsigned char enable;
    struct ak_audio_nr_attr nr_attr;
 
    switch (cmd->head.attr_id)
    {
    case AUDIO_HEARTBEAT:
        break;
    
    case AUDIO_AEC:    
        ak_print_normal_ex("set aec...datalen=%ld\n", cmd->datalen);
        if (NULL == ai_handle)
            ak_ai_get_handle(0, &ai_handle);

        if (1 != cmd->datalen)
        {
            memcpy(&aec_attr, cmd->data, sizeof(struct ak_audio_aec_attr));
            ak_print_normal_ex( "audio_in_digi_gain=%d,audio_out_digi_gain=%d, audio_out_threshold= %ld\n",
                    aec_attr.audio_in_digi_gain, aec_attr.audio_out_digi_gain, aec_attr.audio_out_threshold);
            ret = ak_ai_set_aec_attr(ai_handle, &aec_attr);
        }
        else
        {
            memcpy(&enable, cmd->data, sizeof(unsigned char));
            ak_print_normal_ex( "set aec...enable=%d\n", enable);
            ret = ak_ai_set_aec(ai_handle, enable);
        }
        break;

    case AUDIO_AGC:
        ak_print_normal_ex( "set aec...datalen=%ld\n", cmd->datalen);
        if (NULL == ai_handle)
            ak_ai_get_handle(0, &ai_handle);

        if (1 != cmd->datalen)
        {
            memcpy(&agc_attr, cmd->data, sizeof(struct ak_audio_agc_attr));
            ak_print_normal_ex( "agc_level=%ld,agc_max_gain=%d\n",
                 agc_attr.agc_level, agc_attr.agc_max_gain);
            ret = ak_ai_set_agc_attr(ai_handle, &agc_attr);
        }
        else
        {
            memcpy(&enable, cmd->data, sizeof(unsigned char));
            ak_print_normal_ex( "set agc...enable=%d\n", enable);
            ret = ak_ai_enable_agc(ai_handle, enable);
        }
        break;

    case AUDIO_NR:
        ak_print_normal_ex("set nr...datalen=%ld\n", cmd->datalen);
        if (NULL == ai_handle)
            ak_ai_get_handle(0, &ai_handle);

        if (1 != cmd->datalen)
        {
            memcpy(&nr_attr, cmd->data, sizeof(struct ak_audio_nr_attr));
            ak_print_normal_ex("noise_suppress_db=%d\n",
                 nr_attr.noise_suppress_db);
            ret = ak_ai_set_nr_attr(ai_handle, &nr_attr);
        }
        else
        {
            memcpy(&enable, cmd->data, sizeof(unsigned char));
            ak_print_normal_ex("set nr...enable=%d\n", enable);
            ret = ak_ai_set_nr(ai_handle, enable);
        }
        break;
        
    case AUDIO_EQ_ADC:
        ak_print_normal_ex( "set eq...datalen=%ld\n", cmd->datalen);
        if (NULL == ai_handle)
            ak_ai_get_handle(0, &ai_handle);

        if (1 != cmd->datalen)
        {
            memcpy(&eq_attr, cmd->data, sizeof(struct ak_audio_eq_attr));

            ak_print_normal_ex( "pre_gain=%d,bands=%ld,smoothEna=%d,smoothTime =%d, dcRmEna=%d,dcfb=%ld,aslc_ena=%d,aslc_level_max=%d\n",
                 eq_attr.pre_gain, eq_attr.bands, eq_attr.smoothEna, eq_attr.smoothTime,
                 eq_attr.dcRmEna, eq_attr.dcfb, eq_attr.aslc_ena, eq_attr.aslc_level_max);
            for (int i = 0; i < EQ_MAX_BANDS; i++)
            {
                ak_print_normal_ex( "bandfreqs[%d]=%ld,bandgains[%d] =%d, bandQ[%d]=%d,band_types[%d]=%d\n",
                 i,eq_attr.bandfreqs[i], i,eq_attr.bandgains[i], i,
                 eq_attr.bandQ[i], i, eq_attr.band_types[i]);
            }

            ret = ak_ai_set_eq_attr(ai_handle, &eq_attr);
        }
        else
        {
            memcpy(&enable, cmd->data, sizeof(unsigned char));
            ak_print_normal_ex( "eq enable=%d\n", enable);
            ret = ak_ai_enable_eq(ai_handle, enable);
        }
        break;

    case AUDIO_EQ_DAC:
        ak_print_normal_ex( "set eq...datalen=%ld\n", cmd->datalen);
        if (NULL == ao_handle)
            ak_ao_get_handle(0, &ao_handle);

        if (1 != cmd->datalen)
        {
            memcpy(&eq_attr, cmd->data, sizeof(struct ak_audio_eq_attr));

            ak_print_normal_ex( "pre_gain=%d,bands=%ld,smoothEna=%d,smoothTime =%d, dcRmEna=%d,dcfb=%ld,aslc_ena=%d,aslc_level_max=%d\n",
                 eq_attr.pre_gain, eq_attr.bands, eq_attr.smoothEna, eq_attr.smoothTime,
                 eq_attr.dcRmEna, eq_attr.dcfb, eq_attr.aslc_ena, eq_attr.aslc_level_max);
            for (int i = 0; i < EQ_MAX_BANDS; i++)
            {
                ak_print_normal_ex( "bandfreqs[%d]=%ld,bandgains[%d] =%d, bandQ[%d]=%d,band_types[%d]=%d\n",
                 i,eq_attr.bandfreqs[i], i,eq_attr.bandgains[i], i,
                 eq_attr.bandQ[i], i, eq_attr.band_types[i]);
            }
            ret = ak_ao_set_eq_attr(ao_handle, &eq_attr);
        }
        else
        {
            memcpy(&enable, cmd->data, sizeof(unsigned char));
            ak_print_normal_ex( "eq agc...enable=%d\n", enable);
            ret = ak_ao_enable_eq(ao_handle, enable);
        }
        break;

    case AUDIO_VOLUME:
        ak_print_normal_ex( "set volume...datalen=%ld\n", cmd->datalen);
        memcpy(&volume_attr, cmd->data, sizeof(struct ak_audio_volume_attr));
        if (NULL == ai_handle)
            ak_ai_get_handle(0, &ai_handle);
        
        if (NULL == ao_handle)
            ak_ao_get_handle(0, &ao_handle);

        ak_print_normal_ex( "adc_gain=%d,adc_volume=%d\n",
             volume_attr.adc_gain, volume_attr.adc_volume);

        if (ak_ai_set_adc_volume(ai_handle, volume_attr.adc_gain)) {
            ak_print_error_ex( "ak_ai_set_adc_volume error gain =%d\n",
                            volume_attr.adc_gain);
        }
        
        if (ak_ai_set_aslc_volume(ai_handle, volume_attr.adc_volume)) {
            ak_print_error_ex( "ak_ai_set_aslc_volume error gain =%d\n",
                            volume_attr.adc_volume);  
        }
        if (ak_ao_set_dac_volume(ao_handle, volume_attr.dac_gain)) {
            ak_print_error_ex( "ak_ao_set_dac_volume error gain =%d\n",
                            volume_attr.dac_gain);  
        }
        if (ak_ao_set_aslc_volume(ao_handle, volume_attr.dac_volume)) {
            ak_print_error_ex( "ak_ao_set_aslc_volume error gain =%d\n",
                            volume_attr.dac_volume);
        }
        break;
        
    case AUDIO_FILE:
        ak_print_normal_ex( "set AUDIO_FILE...datalen=%ld\n",
                                        cmd->datalen);
        if (1 == cmd->datalen)
        {
            memcpy(&enable, cmd->data, sizeof(unsigned char));
            ak_print_normal_ex( "enable=%d\n", enable);
            ret = ak_ai_save_aec_dump_file(ai_handle, enable);
        }
        break;

	case AUDIO_DEBUG_INFO:
		ak_print_normal_ex("set AUDIO_DEBUG_INFO...datalen=%ld\n",  cmd->datalen);
        if (1 == cmd->datalen) {
            //ak_ai_print_runtime_status(ai_handle_id);
        }
        if(2 == cmd->datalen) {
            ak_print_notice_ex("print ai data thread start...datalen=%ld\n", cmd->datalen);  
        }
        if(3 == cmd->datalen) {
            ak_print_notice_ex("print ai data thread end...datalen=%ld\n", cmd->datalen);  
        }
        break;
	        
	case AUDIO_VERSION:
        ak_print_normal_ex("set enable---------%ld\n", cmd->datalen);   
        //ai version
        ak_print_normal_ex("ai_version :%s\n",  ak_ai_get_version());  
        //ao version
        ak_print_normal_ex("ao_version :%s\n", ak_ao_get_version());     
        break;
			  
    default:

        break;
    }
	return ret;
}


/**
 * brief: add reply data to send list
 * @pctrl[IN]:pctrl handle
 * @cmd[IN]:cmd info
 * return: 
 * notes:
 */
static void send_cmd(PCTL_HANDLE pctrl, struct cmd_node *cmd)
{
	ak_thread_mutex_lock(&pctrl->lock);
	list_add_tail(&cmd->list, &pctrl->queue);
	ak_thread_mutex_unlock(&pctrl->lock);

	sem_post(&pctrl->isp_sem);
}


/**
 * brief: recv cmd execute
 * @pctrl[IN]:pctrl handle
 * @cmd[IN]:cmd info
 * return: 
 * notes:
 */
static int exec_cmd(PCTL_HANDLE pctrl, PTRANS_DATA cmd)
{
	int cmd_type = cmd->head.cmd_type;
	int ret = 0;
	unsigned long send_size = 0;

	struct cmd_node *cmd_t = (struct cmd_node *)calloc(1,
			sizeof(struct cmd_node));
	if (!cmd_t) {
		ak_print_warning_ex( "calloc failed\n");
		return -1;
	}
	
	/*analysis com info and execute*/
	switch (cmd_type) {
		case CMD_GET:
		case CMD_GET_TXT:
			cmd_t->send_buf = (unsigned char*)calloc(1, 80*1024);
            
			if (NULL == cmd_t->send_buf) {
				exit_err("send_buf");
			}

			memcpy(cmd_t->send_buf, &cmd->head.attr_id, AUDIO_TOOL_ATTR_ID_SIZE);
			if (CMD_GET == cmd_type) {
				cmd_t->send_buf[2] = CMD_REPLY;
			} else {
				cmd_t->send_buf[2] = CMD_REPLY_TXT;
			}

			/*get cmd deal*/
			get_cmd(cmd, cmd_t->send_buf + AUDIO_TOOL_PACKET_HEAD_SIZE);

			/*add reply data to send list*/
			send_cmd(pctrl, cmd_t);
			break;

		case CMD_SET:

			send_size = AUDIO_TOOL_PACKET_HEAD_SIZE + AUDIO_TOOL_PARM_LEN_SIZE + AUDIO_TOOL_RET_SIZE;
			cmd_t->send_buf = (unsigned char*)calloc(1, send_size);

			ak_print_notice_ex( "cmd_t->send_buf= %p\n", cmd_t->send_buf);

			if (NULL == cmd_t->send_buf) {
				exit_err("send_buf");
			}

			memcpy(cmd_t->send_buf, &cmd->head.attr_id, AUDIO_TOOL_ATTR_ID_SIZE);
			cmd_t->send_buf[2] = CMD_RET;
			cmd_t->send_buf[4] = AUDIO_TOOL_RET_SIZE;

			/*set cmd deal*/
			ret = set_cmd(cmd);
            ak_print_notice_ex( "ret =%d\n", ret);
			memcpy(cmd_t->send_buf + AUDIO_TOOL_PACKET_HEAD_SIZE + AUDIO_TOOL_PARM_LEN_SIZE,
					&ret, AUDIO_TOOL_RET_SIZE);
			
			/*add reply data to send list*/
			send_cmd(pctrl, cmd_t);
			break;

		default:
			ak_print_error_ex("cmd unrecognized\n");
			break;
	}

	return 0;
}


/**  
 * isp_send_thread - thread for send reply data
 * @arg[IN]:arg
 * notes: 
 */
static void *audio_tool_send_thread(void *arg)
{
	int ret;
	unsigned long size = 0;
	struct cmd_node *cmd = NULL;
	PCTL_HANDLE pctrl = (PCTL_HANDLE)arg;

	ak_print_normal_ex( "isp cmd thread id: %ld\n", ak_thread_get_tid());
	while (ats_run_flag) {
		sem_wait(&pctrl->isp_sem);

		if (!list_empty(&pctrl->queue)) {
			cmd = list_first_entry(&pctrl->queue, struct cmd_node, list);

			if (cmd->send_buf) {
				memcpy(&size, cmd->send_buf + AUDIO_TOOL_PACKET_HEAD_SIZE, AUDIO_TOOL_PARM_LEN_SIZE);
				size += AUDIO_TOOL_PACKET_HEAD_SIZE + AUDIO_TOOL_PARM_LEN_SIZE;

				ret = send(pctrl->sock, cmd->send_buf, size, 0);
				if (ret <= 0) {
					exit_err("send");
				}

				ak_thread_mutex_lock(&pctrl->lock);
				free(cmd->send_buf);
				list_del(&cmd->list);
				free(cmd);
				ak_thread_mutex_unlock(&pctrl->lock);
			}
		}
	}

	ak_print_normal_ex( "### thread id: %ld exit ###\n", ak_thread_get_tid());
	ak_thread_exit();
	return NULL;
}


/**
 * brief: alarm signal callback 
 * @msg[IN]:message
 * return: void
 * notes:
 */
static void audio_tool_heartbeat_timeout(int msg)
{
	switch (msg){
		case SIGALRM:
			ak_print_normal_ex( "heartbeat timeout!\n");
			if (-1 != cfd) {
				close(cfd);
				cfd = -1;
			}
			set_check_timer(0);
			break;

		default:
			break;
	}
	return ;
}

/**  
 * isp_set_net_srv - tcp sever socket and bind
 * notes: 
 */
static int audio_tool_set_net_srv(unsigned int port)
{
	/** create socket **/
	int sfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sfd < 0) {
		exit_err("sfdet");
	}

	/** initialize and fill in the message to the sfdet addr infor struct **/
	struct sockaddr_in server_addr;	/** server socket addr information struct **/
	memset(&server_addr, 0, sizeof(struct sockaddr));

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = INADDR_ANY;

	/** set socket attribute **/
	int sinsize = 1;
	int ret = setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &sinsize, sizeof(int));
	if (ret < 0) {
		close(sfd);
		exit_err("setsockopt");
	}

	/** bind sfdet with this program **/
	ret = bind(sfd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr));
	if (ret < 0) {
		close(sfd);
		exit_err("bind");
	}

	return sfd;
}

/**
 * audio_tool_server_start - audio tool server start
 * @port[IN]: server port id
 * return: 0 success, -1 failed
 * notes:
 */
static int audio_tool_server_start(unsigned int port)
{
	int ret; 	/** call function return val **/
	int sinsize; 	/** use for set socket option **/
	pthread_t respons_tid;	/** response data thread id **/
	PCTL_HANDLE pctrl = NULL;	/** define a isp handle **/
	TRANS_DATA cmd;
	int recvlen = 0;
	int size = 0;
	T_HEAD_TYPE headret = HEAD_ERR;
    
	/* set heart beat timeout */
	signal(SIGALRM, audio_tool_heartbeat_timeout);

	/* start net server to receive connection */
	sfd = audio_tool_set_net_srv(port);
	if (sfd <= 0) {
		ak_print_warning_ex( "set net srv failed\n");
		return -1;
	}

	/** listen, create wait_queue **/
	ret = listen(sfd, 1);	/* only one client can connect to this server */
	if (ret < 0) {        
		ak_print_error_ex( "listen failed\n");
		close(sfd);
		exit_err("listen");
	}
    ats_run_flag = AK_TRUE;

	struct sockaddr_in peer_addr;		/** server socket addr information struct **/
	memset(&peer_addr, 0, sizeof(struct sockaddr));
	//accept
	while (ats_run_flag) {
		ak_print_notice_ex( "waiting for connect ...\n");

		sinsize = sizeof(struct sockaddr);
		/** accept the client to connect **/
		cfd = accept(sfd, (struct sockaddr *)&peer_addr, (socklen_t *)&sinsize);
		if (cfd < 0) {
			close(sfd);
			exit_err("accept");
		}
		ak_print_notice_ex( "Client connected, socket cfd = %d\n", cfd);

		/** init the handle and others **/
		pctrl = calloc(1, sizeof(CTL_HANDLE));
		pctrl->sock = cfd;
		sem_init(&pctrl->isp_sem, 0, 0);
		INIT_LIST_HEAD(&pctrl->queue);
		ak_thread_mutex_init(&pctrl->lock, NULL);

		/** command parse thread **/
		if (pthread_create(&respons_tid, NULL, audio_tool_send_thread, pctrl))
			pthread_detach(respons_tid);

		/* heart bead sending */
		set_check_timer(HEARTBEAT_CHECKTIME);

		/** receive data **/
		while (ats_run_flag) 
        {
			size = AUDIO_TOOL_PACKET_HEAD_SIZE;
			memset(&cmd, 0, sizeof(TRANS_DATA));
			ret = recv(cfd, &cmd.head, size, 0);
			if(ret <= 0)
			{
				break;
			}

			/* head check */
			headret = check_cmd_head(&cmd);

			/* drop invalid packet */
			if (HEAD_ERR == headret) 
            {
				continue;
			} 
            else if (HEAD_HAVE_DATA == headret) 
			{
				size = AUDIO_TOOL_PARM_LEN_SIZE;
				ret = recv(cfd, &cmd.datalen, size, 0);

				if(ret <= 0) 
                {
					break;
				}
				ak_print_notice_ex( "cmd.datalen = %lu\n", 
                            cmd.datalen);

				recvlen = 0;
				size = cmd.datalen;
				cmd.data = (unsigned char *)calloc(1, size);
				if (NULL == cmd.data) 
                {
					break;
				}

				memset(cmd.data, 0, size);
				while (recvlen < cmd.datalen) 
                {
					ret = recv(cfd, cmd.data + recvlen, size, 0);
					if(ret <= 0) 
                    {
						goto RECV_ERROR;
					}
					recvlen += ret;
					size -= ret;
					ak_print_notice_ex( "recvlen = %d\n", recvlen);
				}
			}

			exec_cmd(pctrl, &cmd);
			if (NULL != cmd.data) 
            {
				free(cmd.data);
				cmd.data = NULL;
			}
		}

RECV_ERROR:
		ak_print_notice_ex( "recv error, ret = %d\n", ret);

		ak_thread_mutex_lock(&pctrl->lock);
		struct cmd_node *cmd_t = NULL, *tmp = NULL;
		list_for_each_entry_safe(cmd_t, tmp, &pctrl->queue, list) {
			if (cmd_t->send_buf)
				free(cmd_t->send_buf);
			list_del(&cmd_t->list);
			free(cmd_t);
		}
		ak_thread_mutex_unlock(&pctrl->lock);
		ak_thread_mutex_destroy(&pctrl->lock);

		sem_destroy(&pctrl->isp_sem);
		free(pctrl);

		if (-1 != cfd) {
			close(cfd);
			cfd = -1;
		}

		pthread_cancel(respons_tid);
		if (NULL != cmd.data) {
			free(cmd.data);
			cmd.data = NULL;
		}

		set_check_timer(0);
	}

	if (-1 != sfd)
	{
		close(sfd);
		sfd = -1;
	}
	ak_print_normal_ex( "isp server exit\n");

	return 0;
}

/**  
 * audio_tool_server - audio_tool thread function
 * @arg[IN]:arg
 * notes: 
 */
static void *audio_tool_server(void *arg)
{
	ak_print_normal_ex( "audio_tool_server start\n");
	long int tid = ak_thread_get_tid();
	ak_print_normal_ex( "thread tid: %ld\n", tid);
	ak_thread_set_name("isp_tool_server");
    ak_thread_detach(isp_svr_tid);

	/* start isp tool server */
	audio_tool_server_start(port_id);

	ak_print_normal_ex( "exit thread tid: %ld\n", tid);
	ak_thread_exit();
	return NULL;
}

/**
 * ak_ats_get_version - get audio tool server in version
 * return: version string
 * notes:
 */
const char* ak_ats_get_version(void)
{
	return audio_tool_version;
}

/**
 * ak_ats_start - audio tool server start
 * @port[IN]: server port id
 * return: 0 success, -1 failed
 * notes:
 */
int ak_ats_start(unsigned int port)
{
    ak_ai_get_handle(0, &ai_handle);
    ak_ao_get_handle(0, &ao_handle);
    
    port_id = port;
	ak_print_normal_ex( "begin ----------%s\n", audio_tool_version);
	if (!ats_run_flag) 
    {

        /* create server thread */
        ak_thread_create(&isp_svr_tid, audio_tool_server, NULL, 
            ANYKA_THREAD_MIN_STACK_SIZE, -1);
        ats_run_flag = AK_TRUE;
    }
    return AK_SUCCESS;
}

/**
 * ak_ats_stop - audio tool server stop
 * return: 0 success, -1 failed
 * notes:
 */
void ak_ats_stop(void)
{
	if (ats_run_flag) {
		ats_run_flag = AK_FALSE;
		shutdown(sfd, 2);
		close(sfd);
		sfd = -1;
	}
}

