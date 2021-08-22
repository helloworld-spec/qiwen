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
#include "ao_ipcsrv.h"

#include "ak_ao.h"

#define AO_PATH_LEN 		100
#define DAC_MAX_VOLUME		12
#define AO_INFO_LEN			10
#define RES_LEN				1024

/* use for Inter-Process Communication */
struct ao_ipc_t {
	char file[AO_PATH_LEN];	// file include path
	int num;		// save number
	int sample_rate;		// encode fps
	int sample_bits;		// encode bitrate in k
	int channel_num;		// channel number
	int second;				// save pcm second
	int level;				// save file level
	char volume[AO_INFO_LEN];		// volume
#if 0
	int resample;
#endif
};

struct ao_save_file_info {
	FILE *save_pcm_fd;			// save pcm file fd
	int save_pcm_len;			// save stream runtime flag, 1 -> save, 0 not
	int pcm_total_len;			// pcm file total length
	char save_pcm_path[AO_PATH_LEN];	// save pcm absolutely path
};

struct ao_ipc_info_t {
	void *ao_handle;			// ao_handle
	int save_second;			// save file seconds
	int save_level;				// pcm file save level:0~3
	struct ao_save_file_info file_info[AO_MAX_LEVEL];
};

static struct ao_ipc_info_t ao_ipcsrv = {0};
static int ao_register = 0;
static int ao_bind = 0;

static int ao_need_save_pcm(struct ao_ipc_info_t *ipc_info,
				int buf_len, int save_level)
{
	int ret = 0;
	if (!ipc_info) {
		goto save_pcm_end;
	}
	if (!ipc_info->file_info[save_level].pcm_total_len) {
		ret = 0;
		ak_print_info_ex("ipc_info->file_info[%d].pcm_total_len = 0\n", save_level);
		goto save_pcm_end;
	}
	if (!ipc_info->file_info[save_level].save_pcm_len
			&& ipc_info->file_info[save_level].pcm_total_len) {
		ret = 1;
		goto save_pcm_end;
	}
	if (ipc_info->file_info[save_level].save_pcm_len + buf_len >
				ipc_info->file_info[save_level].pcm_total_len) {
		ak_print_info_ex("save_pcm_len + buf_len > pcm_total_len\n");
		ret = 0;
	} else {
		ret = 1;
	}
save_pcm_end:
	return ret;
}

static void ao_parse_ipc_args(char *buf, unsigned int len,
							struct ao_ipc_t *ipc_t)
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
		{"sec",  1, NULL, 'S'},	//seconds
		{"save", 1, NULL, 's'},	//save on-off switch
		{"level", 1, NULL, 'l'}, //save file level
		{"volume", 1, NULL, 'V'}, //volume
		{0, 0, 0, 0}	//end
	};

	optind = 0;
	while ((opt = getopt_long(argc, argv, ":lSsV", longopts, NULL)) != -1) {
		switch (opt) {
			case ':':
				break;
			case 'S':
				if (optarg)
					ipc_t->second = atoi(optarg);
				break;
			case 's':
				if (optarg)
					strncpy(ipc_t->file, optarg, AO_PATH_LEN);
				break;
			case 'l':
				if (optarg)
					ipc_t->level = atoi(optarg);
				break;
			case 'V':	//volume
				if (optarg)
					strncpy(ipc_t->volume, optarg, AO_INFO_LEN);
				break;
			default:
				break;
		}
	}
	free(cmd);
}

/**
 * ao_set_file_name - set file name 
 * @origin_name[IN]: audio origin name
 * return: NULL
 * notes: 
 */
static void ao_set_file_name(const char *origin_name)
{
	if (!origin_name)
		return;

	char temp_path[AO_PATH_LEN];
	strncpy(temp_path, origin_name, AO_PATH_LEN);

	char *pstr = strstr(temp_path, ".pcm");
	if (pstr)
		memset(pstr, 0, 4); // remove .pcm

	sprintf(ao_ipcsrv.file_info[AO_ORIGIN_LEVEL].save_pcm_path,
			"%s%s", temp_path, "_origin1.pcm");
	sprintf(ao_ipcsrv.file_info[AO_RESAMPLE_LEVEL].save_pcm_path,
		   	"%s%s", temp_path, "_resample2.pcm");
	sprintf(ao_ipcsrv.file_info[AO_ASLC_LEVEL].save_pcm_path,
		   	"%s%s", temp_path, "_aslc4.pcm");
	sprintf(ao_ipcsrv.file_info[AO_EQ_LEVEL].save_pcm_path,
		   	"%s%s", temp_path, "_eq3.pcm");
}

static int ao_consert_volume(char *str_volume)
{
	int volume = -1;
	if (!str_volume)
		return volume;

	if (strstr(str_volume, "mute")) {
		volume = 0;
		return volume;
	}

	volume = atoi(str_volume);
	if (volume <= 0 || volume > DAC_MAX_VOLUME)
		volume = -1;// maybe not set volume, or set volume not right

	return volume;
}

/**
 * cb_ao_set_status - callbalk function,ao set status 
 * return: 0 success -1 failed
 */
int cb_ao_set_status(char *status, unsigned int len, int sock)
{
	int ret = AK_FAILED;
	char res[RES_LEN] = {0};
	struct ao_ipc_t ipc = {{0}, 0};

	/* parse arguments */
	ao_parse_ipc_args(status, len, &ipc);

	void *handle = ao_ipcsrv.ao_handle;
	if (!handle) {
		snprintf(res, RES_LEN, "handle is NULL\n");
		goto exit;	/* do nothing */
	}

	struct ao_ipc_info_t *ipc_info = &ao_ipcsrv;
	if (!ipc_info) {
		snprintf(res, RES_LEN, "un-init module\n");
		goto exit;	/* do nothing */
	}

	int volume = ao_consert_volume(ipc.volume);
	if (-1 != volume) {
		ak_print_normal_ex("ipc.volume=%s\n", ipc.volume);
		ak_ao_set_volume(handle, volume);
	}

	if (ipc.level < 0 || ipc.level >= AO_MAX_LEVEL)
		goto exit;	/* do nothing */

	/*
	 * if there is no indicate save second of frames
	 * file path should set first, incase of error occur by thread-schdule
	 */
	if ((ipc.second > 0) && (strlen(ipc.file) > 0)) {
		ak_print_normal_ex("file: %s\nsecond: %dlevel: %d\n",
					ipc.file, ipc.second, ipc.level);
		strcpy(ipc_info->file_info[AO_FINAL_LEVEL].save_pcm_path, ipc.file);
		ao_set_file_name(ipc.file);
		ipc_info->save_second = ipc.second;
		snprintf(res, RES_LEN, "%s start to save %d second pcm to file: %s\n",
				res, ipc_info->save_second,
				ipc_info->file_info[AO_FINAL_LEVEL].save_pcm_path);

		struct ao_runtime_status other_status;
		ak_ao_get_runtime_status(ipc_info->ao_handle, &other_status);
		int actual_rate = other_status.actual_rate;

		struct pcm_param param;
		ak_ao_get_params(ipc_info->ao_handle, &param);
		int sample_rate = param.sample_rate;

		int i;
		for (i = 0; i < AO_MAX_LEVEL; i++) {
			struct ao_save_file_info *temp = &(ipc_info->file_info[i]);
			if (AO_ORIGIN_LEVEL == i)
				temp->pcm_total_len = sample_rate * 2 * ipc_info->save_second;
			else
				temp->pcm_total_len = actual_rate * 2 * ipc_info->save_second;
			snprintf(res, RES_LEN - strlen(res),
					"%spcm_total_len: %d\n", res, temp->pcm_total_len);
		}
		ipc_info->save_level = AO_MAX_LEVEL -1;//ipc.level;						
	} else if (((ipc.second > 0) && (!strlen(ipc.file))) 
			|| ((!ipc.second) && (strlen(ipc.file) > 0))) {
		ipc_info->save_second = 0;
		snprintf(res, RES_LEN, "must give filename and seconds to save file\n"
				"e.g: --sec 20 --save /mnt/test1.pcm \n");
		goto exit;
	} else {
		goto exit;	/* do nothing */
	}
	ret = AK_SUCCESS;
exit:
	if (sock)
		ak_cmd_result_response(res, strlen(res), sock);

	return ret;
}

/**
 * cb_ao_get_status - callbalk function,ao get status 
 * return: 0 success -1 failed
 */
int cb_ao_get_status(char *status, unsigned int len, int sock)
{
	char res[RES_LEN] = {0};
	int ret = AK_FAILED;
	
	/* get ak_venc internal handle */
	void *handle = ao_ipcsrv.ao_handle;
	if (!handle) {
		snprintf(res, RES_LEN, "handle is NULL\n");
		goto exit;	/* do nothing */
	}
	struct pcm_param param = {0};
	ak_ao_get_params(handle, &param);
	
	struct ao_runtime_status other_status = {0};
	ak_ao_get_runtime_status(handle, &other_status);
	int volume = ak_ao_get_volume(handle);	
	int dac_volume = ak_ao_get_dac_volume(handle);
	int aslc_volume = ak_ao_get_aslc_volume(handle);

	snprintf(res, RES_LEN, 
			"actual sample rate: %d\n"
			"sample_rate: %d\n"
			"channel_num:%d\n"
			"sample_bits: %d\n"
			"resample_enable: %d\n"
			"volume: %d, dac_volume: %d, aslc_volume: %d\n",
			other_status.actual_rate,
			param.sample_rate, 
			param.channel_num, 
			param.sample_bits,
			other_status.resample_enable,
			volume, dac_volume, aslc_volume);
	ret = AK_SUCCESS;
	
exit:
	if (sock)
		ak_cmd_result_response(res, strlen(res), sock);
	return ret;
}

/**
 * cb_ao_usage - callbalk function,ao usage 
 * return: 0 success -1 failed
 */
int cb_ao_usage(char *status, unsigned int len, int sock)
{
	char res[RES_LEN] = {0};
	snprintf(res, RES_LEN, "module ao:\n"		
			"\t[--save FILE]       save file\n"
			"\t[--sec  SEC]        save seconds\n"
			"\t[--level  LEVEL]    save level,value: [0,3]"		
			"0:save final pcm,1:save origin pcm,"
			"2:save after resample pcm,3:save after aslc pcm\n"
			"\t[--volume  VOLUME]  set volume,value: mute or [1,12]\n\n"
			"\te.g: set volume mute: --volume mute\n"		
			"\te.g: set volume: --volume 6\n"
			"\te.g: save 20s final pcm: --sec 20 --save /mnt/test1.pcm\n"
			"\te.g: save 20s final and origin pcm:"
			" --sec 20 --save /mnt/test1.pcm --level 1\n"
			"\te.g: save 20s final and origin and resample pcm:"
			" --sec 20 --save /mnt/test1.pcm --level 2\n"
			"\te.g: save 20s final and origin and resample and aslc pcm:"
			" --sec 20 --save /mnt/test1.pcm --level 3\n");
			
	if (sock)
		ak_cmd_result_response(res, strlen(res), sock);

	return AK_SUCCESS;
}

/**
 * cb_ao_get_sysipc_version - callbalk function,ao get version 
 * return: 0 success -1 failed
 */
int cb_ao_get_sysipc_version(char *status, unsigned int len, int sock)
{
	char res[50] = {0};
	snprintf(res, 50, "%s\n", ak_ao_get_version());
	if (sock)
		ak_cmd_result_response(res, strlen(res), sock);

	return AK_SUCCESS;
}

/**
 * ao_save_stream_to_file - save pcm file  
 * @buf[IN]: ai data
 * @len[IN]: ai data length
 * @save_level[IN]: ai data save level [0,3]
 * return: 0 success -1 failed
 * notes: 
 */
int ao_save_stream_to_file(unsigned char *buf, int len,
						enum ao_level_index save_level)
{
	//if (save_level > ao_ipcsrv.save_level)
	//	return AK_FAILED;	// this level not have to save

	if (!buf || !len) {
		ak_print_error_ex("no data\n");
		return -1;
	}

	struct ao_save_file_info *save_info = &ao_ipcsrv.file_info[save_level];

	if (!save_info) {
		return AK_FAILED;
	}

	if (ao_need_save_pcm(&ao_ipcsrv, len, save_level)) {
		/* create stream file */
		if (!save_info->save_pcm_fd) {
			save_info->save_pcm_fd = fopen(
						save_info->save_pcm_path, "w+");
			if (!save_info->save_pcm_fd) {
				ak_print_error_ex("open %s fail, %s\n",
					   	save_info->save_pcm_path,
					   	strerror(errno));
				ao_ipcsrv.save_second = 0;
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
	} else if (save_info->save_pcm_fd) {	/* on write done, close file */
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
void ao_sys_ipc_register(void)
{
	if (ao_register) {
		ak_print_error_ex("ao has register already\n");
		return;	/* ao has register already */
	}
	memset(&ao_ipcsrv, 0, sizeof(struct ao_ipc_info_t));

	/* use for build up IPC system, use for debug */
	struct ak_ipc_msg_t set, get, help, version;

	set.cmd = "ao_set_status";
	set.msg_cb = cb_ao_set_status;
	ak_cmd_register_msg_handle(ANYKA_IPC_PORT, &set);

	get.cmd = "ao_get_status";
	get.msg_cb = cb_ao_get_status;
	ak_cmd_register_msg_handle(ANYKA_IPC_PORT, &get);

	help.cmd = "ao_usage";
	help.msg_cb = cb_ao_usage;
	ak_cmd_register_msg_handle(ANYKA_IPC_PORT, &help);

	version.cmd = "ao_version";
	version.msg_cb = cb_ao_get_sysipc_version;
	ak_cmd_register_msg_handle(ANYKA_IPC_PORT, &version);

	ak_cmd_register_module(ANYKA_IPC_PORT, "ao");
	ao_register = 1;
}

/**
 * ao_sys_ipc_unregister - unregister to ipc 
 * return: NULL
 * notes: 
 */
void ao_sys_ipc_unregister(void)
{
	if (!ao_register)
		return;
		
	ak_cmd_unregister_msg_handle(ANYKA_IPC_PORT,"ao_set_status");
	ak_cmd_unregister_msg_handle(ANYKA_IPC_PORT,"ao_get_status");
	ak_cmd_unregister_msg_handle(ANYKA_IPC_PORT,"ao_usage");
	ak_cmd_unregister_msg_handle(ANYKA_IPC_PORT,"ao_version");

	ak_cmd_unregister_module(ANYKA_IPC_PORT, "ao");
	ao_register = 0;
}

/**
 * ao_sysipc_bind_handle - bind ao handle 
 * @handle[IN]: audio out opened handle
 * return: NULL
 * notes: 
 */
void ao_sysipc_bind_handle(void *handle)
{
	if (!handle) {
		ak_print_warning_ex("ao handle is not working\n");
		return ;
	}
	if (ao_bind) {
		ak_print_warning_ex("ao handle has bind already, handle=%d\n",
							(int)ao_ipcsrv.ao_handle);
		return ;
	}
	/* store ak_venc encode handle to this module */
	ao_ipcsrv.ao_handle = handle;
	ao_bind = 1;
}

/**
 * ao_sysipc_unbind_handle - unbind ao handle
 * return: NULL
 * notes: 
 */
void ao_sysipc_unbind_handle(void)
{
	if (!ao_ipcsrv.ao_handle) {
		ak_print_warning_ex("ao handle is not working\n");
		return ;
	}

	/* clear ao handle from this module */
	ao_ipcsrv.ao_handle = NULL;
	ao_bind = 0;
}
