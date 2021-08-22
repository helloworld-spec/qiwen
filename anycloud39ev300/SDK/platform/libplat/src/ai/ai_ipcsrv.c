#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <getopt.h>

#include "ak_global.h"
#include "ak_common.h"
#include "ak_ipc_srv.h"
#include "ai_ipcsrv.h"

#include "ak_ai.h"

#define AI_PATH_LEN 		100
#define ADC_MAX_VOLUME		12
#define AI_INFO_LEN			10
#define RES_LEN				1024

/* use for Inter-Process Communication */
struct ai_ipc_t {
	char file[AI_PATH_LEN];	// file include path
	int num;		// save number
	int sample_rate;		// encode fps
	int sample_bits;		// encode bitrate in k
	int channel_num;		// channel number
	int second;				// save pcm second
	int level;				// save file level
	char volume[AI_INFO_LEN];		// volume
	char source[AI_INFO_LEN];		// capture source
	char nr_max[AI_INFO_LEN];		// nr_max switch
	char aec_dump_file_switch[AI_INFO_LEN];		// nr_max switch
#if 0
	int nr_agc_enable;
	int aec_enable;
#endif
};

struct ai_save_file_info {
	FILE *save_pcm_fd;			// save pcm file fd
	int save_pcm_len;			// save stream runtime flag, 1 -> save, 0 not
	int pcm_total_len;			// pcm file total length
	char save_pcm_path[AI_PATH_LEN];	// save pcm absolutely path
};

struct ai_ipc_info_t {
	void *ai_handle;			// ai_handle
	int save_second;			// save file seconds
	int save_level;				// pcm file save level:0~3
	struct ai_save_file_info file_info[AI_MAX_LEVEL];// file information
};

static struct ai_ipc_info_t ai_ipcsrv;
static int ai_register = 0;// whether register already
static int ai_bind = 0;// whether ai bind already

/**
 * ai_need_save_pcm - whether ai need save pcm file  
 * @ipc_info[IN]: ipc information
 * @buf_len[IN]: data length
 * @save_level[IN]: ai data save level [0,3]
 * return: 0 success -1 failed
 * notes: 
 */
static int ai_need_save_pcm(struct ai_ipc_info_t *ipc_info,
	   	int buf_len, int save_level)
{
	int ret = 0;

	if (!ipc_info) {
		goto save_pcm_end;
	}
	if (!ipc_info->file_info[save_level].pcm_total_len) {
		ret = 0;
		goto save_pcm_end;
	}
	if (!ipc_info->file_info[save_level].save_pcm_len
			&& ipc_info->file_info[save_level].pcm_total_len) {
		ret = 1;
		goto save_pcm_end;
	}

	if (ipc_info->file_info[save_level].save_pcm_len + buf_len >
				ipc_info->file_info[save_level].pcm_total_len)
		ret = 0;
	else
		ret = 1;

save_pcm_end:
	return ret;
}

static void ai_parse_ipc_args(char *buf, unsigned int len,
							struct ai_ipc_t *ipc)
{
	char *cmd = (char *)calloc(1, len);
	if (!cmd)
		return;
	memcpy(cmd, buf, len); /* backup string */

	char *str, *saveptr, *token, *argv[50] = {cmd, 0};  /* to skip argv[0] */
	int argc = 1, opt;

	for (str = cmd; ; str = NULL) {
		token = strtok_r(str, DELIMIT, &saveptr);
		if (token == NULL)
			break;
		argv[argc++] = token;
	}

	/* arguments tabel */
	const struct option longopts[] = {
		{"aec_dump", 1, NULL, 'A'}, //aec
		{"source", 1, NULL, 'C'}, //source
		{"level", 1, NULL, 'l'}, //save file level
		{"sec",  1, NULL, 'S'},	 //seconds
		{"save", 1, NULL, 's'},	 //save files
		{"volume", 1, NULL, 'V'}, //volume
		{"nr_max", 1, NULL, 'N'}, //nr max switch
		{0, 0, 0, 0}			 //end
	};

	optind = 0;
	while ((opt = getopt_long(argc, argv, ":AClNSsV", longopts, NULL)) != -1) {
		switch (opt) {
			case ':':
				break;
			case 'A':
				if (optarg)
					strncpy(ipc->aec_dump_file_switch, optarg, AI_INFO_LEN);
				break;
			case 'l':
				if (optarg)
					ipc->level = atoi(optarg);
				break;
			case 'C':	//source
				if (optarg)
					strncpy(ipc->source, optarg, AI_INFO_LEN);
				break;
			case 'S':
				if (optarg)
					ipc->second = atoi(optarg);
				break;
			case 's':
				if (optarg)
					strncpy(ipc->file, optarg, AI_PATH_LEN);
				break;
			case 'V':	//volume
				if (optarg)
					strncpy(ipc->volume, optarg, AI_INFO_LEN);
				break;
			case 'N':	//nr_max switch
				if (optarg)
					strncpy(ipc->nr_max, optarg, AI_INFO_LEN);
				break;
			default:
				break;
		}
	}
	free(cmd);
}

static void ai_print_source_name(int source_type, char *source_name)
{
	switch (source_type) {
		case AI_SOURCE_MIC:// mic
			strcpy(source_name, "mic\n");
			break;
		case AI_SOURCE_LINEIN:// linein
			strcpy(source_name, "linein\n");
			break;
		default:
			strcpy(source_name, "unknow\n");
			break;
	}
}

/**
 * ai_set_file_name - set file name 
 * @origin_name[IN]: audio origin name
 * return: NULL
 * notes: 
 */
static void ai_set_file_name(const char *origin_name)
{
	if (!origin_name)
		return;

	char temp_path[AI_PATH_LEN];
	strncpy(temp_path, origin_name, AI_PATH_LEN);

	char *pstr = strstr(temp_path, ".pcm");
	if (pstr)
		memset(pstr, 0, 4); // remove .pcm

	sprintf(ai_ipcsrv.file_info[AI_ORIGIN_LEVEL].save_pcm_path,
			"%s%s", temp_path, "_origin.pcm");
	sprintf(ai_ipcsrv.file_info[AI_RESAMPLE_LEVEL].save_pcm_path,
		   	"%s%s", temp_path, "_resample.pcm");
	sprintf(ai_ipcsrv.file_info[AI_ASLC_LEVEL].save_pcm_path,
		   	"%s%s", temp_path, "_aslc.pcm");
}

static int ai_consert_volume(char *str_volume)
{
	int volume = -1;
	if (!str_volume)
		return volume;

	if (strstr(str_volume, "mute")) {
		volume = 0;
		return volume;
	}

	volume = atoi(str_volume);
	if (volume <= 0 || volume > ADC_MAX_VOLUME)
		volume = -1;// maybe not set volume, or set volume not right

	return volume;
}

static enum ai_source ai_consert_source(char *str_source)
{
	enum ai_source source = 0;
	if (!str_source)
		return source;

	if (strstr(str_source, "mic")) {
		source = AI_SOURCE_MIC;
	} else if (strstr(str_source, "linein"))
		source = AI_SOURCE_LINEIN;

	return source;
}

static int ai_consert_switch(char *str_switch)
{
	int enable = -1;
	if (!str_switch)
		return enable;

	if (strstr(str_switch, "enable"))
		enable = 1;
	else if (strstr(str_switch, "disable"))
		enable = 0;

	return enable;
}

/**
 * cb_ai_set_status - callbalk function,ai set status 
 * return: 0 success -1 failed
 */
int cb_ai_set_status(char *status, unsigned int len, int sock)
{
	int ret = AK_FAILED;
	char res[RES_LEN] = {0};
	struct ai_ipc_t ipc = {{0}, 0};

	void *handle = ai_ipcsrv.ai_handle;
	if (!handle) {
		snprintf(res, RES_LEN, "handle is NULL\n");
		goto exit;	/* do nothing */
	}

	/* parse arguments */
	ai_parse_ipc_args(status, len, &ipc);

	/* set volume */
	int volume = ai_consert_volume(ipc.volume);// get volume
	if (-1 != volume) {
		ak_print_normal_ex("ipc.volume=%s\n", ipc.volume);
		ak_ai_set_volume(handle, volume);
	}

	/* set source */
	enum ai_source source = ai_consert_source(ipc.source);// get source
	if (source) {
		ak_print_normal_ex("ipc.source=%s\n", ipc.source);
		ak_ai_set_source(handle, source);
	}

	/* set nr max */
	int nr_max_enable = ai_consert_switch(ipc.nr_max);// check flag
	if (-1 != nr_max_enable) {
		ak_print_normal_ex("ipc.nr_max=%s\n", ipc.nr_max);
		ak_ai_set_nr_max(handle, nr_max_enable);
	}

	/* set dump file */
	int aec_dump_file_enable = ai_consert_switch(ipc.aec_dump_file_switch);
	if (-1 != aec_dump_file_enable) {
		ak_print_normal_ex("ipc.aec_dump_file_switch=%s\n", 
							ipc.aec_dump_file_switch);
		ak_ai_save_aec_dump_file(handle, aec_dump_file_enable);
	}

	/* set save pcm file */
	struct ai_ipc_info_t *ipc_info = &ai_ipcsrv;
	if (!ipc_info) {
		snprintf(res, RES_LEN, "un-init module\n");
		goto exit;	/* do nothing */
	}

	if (ipc.level < 0 || ipc.level >= AI_MAX_LEVEL)
		goto exit;	/* do nothing */

	/*
	 * if there is no indicate save second of frames, 30 is default
	 * file path should set first, incase of error occur by thread-schdule
	 */
	if ((ipc.second > 0) && (strlen(ipc.file) > 0)) {
		ak_print_normal_ex("file: %s\nsecond: %d--level: %d\n",
					ipc.file, ipc.second, ipc.level);
		strncpy(ipc_info->file_info[AI_FINAL_LEVEL].save_pcm_path, ipc.file,
				AI_PATH_LEN);
		ai_set_file_name(ipc.file);
		ipc_info->save_second = ipc.second;

		struct ai_runtime_status other_status;
		ak_ai_get_runtime_status(ipc_info->ai_handle, &other_status);
		int actual_rate = other_status.actual_rate;

		struct pcm_param param;
		ak_ai_get_params(ipc_info->ai_handle, &param);
		int sample_rate = param.sample_rate;

		int i;
		for (i = 0; i < ipc.level + 1; i++) {
			struct ai_save_file_info *temp = &(ipc_info->file_info[i]);
			if (AI_ORIGIN_LEVEL == i)
				temp->pcm_total_len = actual_rate * 2 * ipc_info->save_second;
			else
				temp->pcm_total_len = sample_rate * 2 * ipc_info->save_second;
		}
		ipc_info->save_level = ipc.level;
		snprintf(res, RES_LEN, "%s start to save %d second pcm to file: %s\n",
				res, ipc_info->save_second,
				ipc_info->file_info[AI_FINAL_LEVEL].save_pcm_path);
	} else if (((ipc.second > 0) && (!strlen(ipc.file)))
			|| ((!ipc.second) && (strlen(ipc.file) > 0))) {
		ipc_info->save_second = 0;
		snprintf(res, RES_LEN, "must give filename and seconds to save file\n"
				"e.g: --sec 20 --save /mnt/test1.pcm \n");
		goto exit;
	} else {
		goto exit;
	}
	ret = AK_SUCCESS;
	
exit:
	if (sock)
		ak_cmd_result_response(res, strlen(res), sock);

	return ret;
}

/**
 * cb_ai_get_status - callbalk function,ai get status 
 * return: 0 success -1 failed
 */
int cb_ai_get_status(char *status, unsigned int len, int sock)
{
	char res[RES_LEN] = {0};
	int ret = AK_FAILED;

	/* get ak_venc internal handle */
	void *handle = ai_ipcsrv.ai_handle;
	if (!handle) {
		snprintf(res, RES_LEN, "handle is NULL\n");
		goto exit;	/* do nothing */
	}
	struct pcm_param param = {0};
	ak_ai_get_params(handle, &param);

	struct ai_runtime_status other_status = {0};
	ak_ai_get_runtime_status(handle, &other_status);
	int volume = ak_ai_get_volume(handle);
	int adc_volume = ak_ai_get_adc_volume(handle);
	int aslc_volume = ak_ai_get_aslc_volume(handle);
	
	int source = ak_ai_get_source(handle);
	char source_name[AI_INFO_LEN] = {0};
	ai_print_source_name(source, source_name);

	snprintf(res, RES_LEN,
			"actual sample rate: %d\n"
			"sample_rate: %d\n"
			"channel_num: %d\n"
			"sample_bits: %d\n"
			"nr_agc_enable: %d\n"
			"nr_max_enable: %d\n"
			"resample_enable: %d\n"
			"aec_enable: %d\n"
			"volume: %d, adc_volume: %d, aslc_volume: %d\n"
			"source: %s\n",
			other_status.actual_rate,
			param.sample_rate,
			param.channel_num,
			param.sample_bits,
			other_status.nr_agc_enable,
			other_status.nr_max_enable,
			other_status.resample_enable,
			other_status.aec_enable,
			volume, adc_volume, aslc_volume,
			source_name);
	ret = AK_SUCCESS;
exit:
	if (sock)
		ak_cmd_result_response(res, strlen(res), sock);
	return ret;
}

/**
 * cb_ai_usage - callbalk function,ai usage 
 * return: 0 success -1 failed
 */
int cb_ai_usage(char *status, unsigned int len, int sock)
{
	char res[RES_LEN] = {0};
	snprintf(res, RES_LEN, "module ai:\n"
			"\t[--save FILE]       save file\n"
			"\t[--sec  SEC]        save seconds\n"
			"\t[--level  LEVEL]    save level,value: [0,3],"
			"0:save final pcm,1:save origin pcm,"
			"2:save after resample pcm,3:save after aslc pcm\n"
			"\t[--volume  VOLUME]  set volume,value: mute or [1,12]\n"	
			"\t[--source  SOURCE]  set source,value: mic/linein\n"
			"\t[--nr_max  NRMAX]   set nr max config,value: enable/disable\n\n"
			"\te.g: set volume mute: --volume mute\n"		
			"\te.g: set volume 6: --volume 6\n"
			"\te.g: set source: --source mic\n"
			"\te.g: set nr_max: --nr_max enable\n"
			"\te.g: save 20s final pcm: --sec 20 --save /mnt/test1.pcm\n"
			"\te.g: save 20s final and origin pcm:"
			" --sec 20 --save /mnt/test1.pcm --level 1\n"
			"\te.g: save 20s final and origin and resample:"
			" --sec 20 --save /mnt/test1.pcm --level 2\n"
			"\te.g: save 20s final and origin and resample and aslc pcm:"
			" --sec 20 --save /mnt/test1.pcm --level 3\n");
			
	if (sock)
		ak_cmd_result_response(res, strlen(res), sock);

	return AK_SUCCESS;
}

/**
 * cb_ai_get_sysipc_version - callbalk function,get version 
 * return: 0 success -1 failed
 */
int cb_ai_get_sysipc_version(char *status, unsigned int len, int sock)
{
	char res[50] = {0};
	snprintf(res, 50, "%s\n", ak_ai_get_version());
	if (sock)
		ak_cmd_result_response(res, strlen(res), sock);

	return AK_SUCCESS;
}

/**
 * ai_save_stream_to_file - save pcm file  
 * @buf[IN]: ai data
 * @len[IN]: ai data length
 * @save_level[IN]: ai data save level [0,3]
 * return: 0 success -1 failed
 * notes: 
 */
int ai_save_stream_to_file(unsigned char *buf, int len,
						enum ai_level_index save_level)
{
	if (save_level > ai_ipcsrv.save_level)
		return AK_FAILED;	// this level not have to save
		
	if (!buf || !len) {
		ak_print_error_ex("no data\n");
		return AK_FAILED;
	}

	struct ai_save_file_info *save_info = &ai_ipcsrv.file_info[save_level];

	if (!save_info) {
		return AK_FAILED;
	}

	if (ai_need_save_pcm(&ai_ipcsrv, len, save_level)) {
		/* create stream file */
		if (!save_info->save_pcm_fd) {
			save_info->save_pcm_fd = fopen(
						save_info->save_pcm_path, "w+");
			if (!save_info->save_pcm_fd) {
				ak_print_error_ex("open %s fail, %s\n",
					   	save_info->save_pcm_path,
					   	strerror(errno));
				ai_ipcsrv.save_second = 0;
				save_info->pcm_total_len = 0;
				return AK_FAILED;
			}
		}
		/* write data entry */
		if (save_info->save_pcm_fd) {
			if (fwrite(buf, len, 1, save_info->save_pcm_fd) < 0) {
				ak_print_error_ex("write error len:%d,path:%s\n",
						len, save_info->save_pcm_path);
				return AK_FAILED;
			}

			save_info->save_pcm_len += len;
		}
	} else if (save_info->save_pcm_fd) {
		/* on write done, close file */
		fclose(save_info->save_pcm_fd);
		save_info->save_pcm_fd = NULL;
		save_info->save_pcm_len = 0;
		save_info->pcm_total_len = 0;
		ak_print_notice_ex("save pcm done\n");
	}
	return 0;
}

/**
 * ai_sys_ipc_register - register to ipc 
 * return: NULL
 * notes: 
 */
void ai_sys_ipc_register(void)
{
	if (ai_register) {
		ak_print_error_ex("ai has register already\n");
		return;	/* ai has register already */
	}
	memset(&ai_ipcsrv, 0, sizeof(struct ai_ipc_info_t));

	/* use for build up IPC system, use for debug */
	struct ak_ipc_msg_t set, get, help, version;

	set.cmd = "ai_set_status";
	set.msg_cb = cb_ai_set_status;
	ak_cmd_register_msg_handle(ANYKA_IPC_PORT, &set);

	get.cmd = "ai_get_status";
	get.msg_cb = cb_ai_get_status;
	ak_cmd_register_msg_handle(ANYKA_IPC_PORT, &get);

	help.cmd = "ai_usage";
	help.msg_cb = cb_ai_usage;
	ak_cmd_register_msg_handle(ANYKA_IPC_PORT, &help);

	version.cmd = "ai_version";
	version.msg_cb = cb_ai_get_sysipc_version;
	ak_cmd_register_msg_handle(ANYKA_IPC_PORT, &version);

	ak_cmd_register_module(ANYKA_IPC_PORT, "ai");
	ai_register = 1;
}

/**
 * ai_sys_ipc_unregister - unregister to ipc 
 * return: NULL
 * notes: 
 */
void ai_sys_ipc_unregister(void)
{
	if (!ai_register)
		return;

	ak_cmd_unregister_msg_handle(ANYKA_IPC_PORT,"ai_set_status");
	ak_cmd_unregister_msg_handle(ANYKA_IPC_PORT,"ai_get_status");
	ak_cmd_unregister_msg_handle(ANYKA_IPC_PORT,"ai_usage");
	ak_cmd_unregister_msg_handle(ANYKA_IPC_PORT,"ai_version");

	ak_cmd_unregister_module(ANYKA_IPC_PORT, "ai");
	ai_register = 0;
}

/**
 * ai_sysipc_bind_handle - bind ai handle 
 * @handle[IN]: audio in opened handle
 * return: NULL
 * notes: 
 */
void ai_sysipc_bind_handle(void *handle)
{
	if (!handle) {
		ak_print_warning_ex("ai handle is not working\n");
		return ;
	}
	if (ai_bind) {
		ak_print_warning_ex("ai handle has bind already, handle=%d\n",
							(int)ai_ipcsrv.ai_handle);
		return ;
	}
	/* store ak_venc encode handle to this module */
	ai_ipcsrv.ai_handle = handle;

	ai_bind = 1;
}

/**
 * ai_sysipc_unbind_handle - unbind ai handle
 * return: NULL
 * notes: 
 */
void ai_sysipc_unbind_handle(void)
{
	if (!ai_ipcsrv.ai_handle) {
		ak_print_warning_ex("ai handle is not working\n");
		return ;
	}

	/* clear ai handle from this module */
	ai_ipcsrv.ai_handle = NULL;
	ai_bind = 0;
}
