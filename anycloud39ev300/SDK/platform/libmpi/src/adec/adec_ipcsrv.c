#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <getopt.h>
#include <ctype.h>

#include "ak_global.h"
#include "ak_common.h"
#include "ak_ipc_srv.h"
#include "adec_ipcsrv.h"

#include "ak_adec.h"

#define ADEC_PATH_LEN 		100		/* decode path length */
#define TYPE_LEN			12		/* decode type string length */
#define RES_LEN				1024	/* decode result string length */

/* decode type */
enum da_decode_type {
    DECODE_TYPE_AAC = 0x00,
    DECODE_TYPE_G711A,
    DECODE_TYPE_G711U,
    DECODE_TYPE_AMR,
    DECODE_TYPE_MP3,
    DECODE_TYPE_RAW_PCM,
    DECODE_TYPE_MAX
};

/* use for Inter-Process Communication */
struct adec_ipc_t {
	char file[ADEC_PATH_LEN];	/* file include path */
	char type[TYPE_LEN];		/* decode type */
	int num;					/* save number */
	int sample_rate;			/* sample rate */
	int sample_bits;			/* sample bit */
	int channel_num;			/* channel number */
	int handle;					/* save stream second */
	int second;					/* save file times(second) */
	int level;					/* save file level */
};

struct save_file_info {
	FILE *save_stream_fd;					/* save pcm file fd	*/
	int save_second;						/* save file seconds */	
	struct ak_timeval *save_start_time;		/* save stream start time */
	char save_stream_path[ADEC_PATH_LEN];	/* save stream path */
};

struct decode_info_t {
	void *adec_handle;						/* adec handle */
	int decode_type;						/* decode type */
	int save_level; 						/* stream file save level:0~1 */
	struct save_file_info file_info[ADEC_MAX_LEVEL]; /* save file information */
};

struct adec_ipcsrv_t {
	int adec_register;							 /* whether adec register */
	struct decode_info_t *group[DECODE_TYPE_MAX];/* decode information group */
};

static struct adec_ipcsrv_t adec_ipcsrv;/* decode ipc service */

static void print_decode_type(int origin_type)
{
	/* the origin_type see the enum ak_audio_type in ak_global.h */
	char dec_type_name[TYPE_LEN];
	memset(dec_type_name, 0, TYPE_LEN);

	/* check decode type */
	switch (origin_type) {
	case AK_AUDIO_TYPE_MP3:
		strncpy(dec_type_name, "mp3" , 3);
		break;
	case AK_AUDIO_TYPE_AMR:
		strncpy(dec_type_name, "amr" , 3);
		break;
	case AK_AUDIO_TYPE_AAC:
		strncpy(dec_type_name, "aac" , 3);
		break;
	case AK_AUDIO_TYPE_PCM_ALAW:
		strncpy(dec_type_name, "g711a" , 5);
		break;
	case AK_AUDIO_TYPE_PCM_ULAW:
		strncpy(dec_type_name, "g711u" , 5);
		break;
	case AK_AUDIO_TYPE_PCM:
		strncpy(dec_type_name, "pcm" , 3);
		break;
	default:
		strncpy(dec_type_name, "unknown" , 7);
		break;
	}

	ak_print_normal_ex("decode_type: %d, name: %s\n",
	    origin_type, dec_type_name);
}

/**
 * get_code_type_index - get decode type index 
 * @decode_type[IN]: audio decode type
 * return: 0 success, -1 failed
 * notes: 
 */
static int get_code_type_index(char *decode_type)
{
	int ret = 0;
	char type[TYPE_LEN] = {0};
	int type_len = strlen(decode_type);

	if (type_len > 0) {
		sprintf(type, "%s", decode_type);

		int i = 0;
		for (i=0; i<type_len; ++i) {
			type[i] = tolower(type[i]);
		}

		ak_print_notice_ex("audio decode type: %s\n", type);

		/* check the audio file type */
		if (0 == strcmp(type, "mp3")) {
			ret = DECODE_TYPE_MP3;
		} else if (0 == strcmp(type, "amr")) {
			ret = DECODE_TYPE_AMR;
		} else if (0 == strcmp(type, "aac")) {
			ret = DECODE_TYPE_AAC;
		} else if (0 == strcmp(type, "g711a")) {
			ret = DECODE_TYPE_G711A;
		} else if (0 == strcmp(type, "g711u")) {
			ret = DECODE_TYPE_G711U;
		} else if (0 == strcmp(type, "pcm")) {
			ret = DECODE_TYPE_RAW_PCM;
		} else {
			/* unknow type */
			ret = DECODE_TYPE_MAX;
		}
	}
	return ret;
}

/**
 * set_pcm_file_name - set stream file name 
 * @origin_name[IN]: audio origin decode type
 * @type_index[IN]: audio type index
 * @save_level[IN]: audio file save level
 * return: NULL
 * notes: 
 */
static void set_pcm_file_name(const char *origin_name,
							int type_index, int save_level)
{
	if (save_level != BEFORE_DEC_LEVEL) 
		return;
	char temp_path[ADEC_PATH_LEN];
	strncpy(temp_path, origin_name, ADEC_PATH_LEN);
	int len = strlen(temp_path);

	if (len > ADEC_PATH_LEN - 10) {
		ak_print_error_ex("path too long\n");
		return;
	}

	char *pstr = strstr(temp_path, ".");
	if (pstr)
		strncpy(pstr, "_" , 1); 
		
	struct decode_info_t *ipc_info = adec_ipcsrv.group[type_index];
	
	sprintf(ipc_info->file_info[save_level].save_stream_path,
			"%s%s", temp_path, "_aenc");
}

/**
 * set_pcm_file_name - set pcm file name 
 * @origin_name[IN]: audio origin decode name
 * return: current decode type
 * notes: 
 */
static int get_cur_decode_type(int origin_type)
{
	int cur_decode_type = DECODE_TYPE_MAX;

	/* get decoe type from origin type */
	switch (origin_type) {
		case AK_AUDIO_TYPE_MP3:
			cur_decode_type = DECODE_TYPE_MP3;
			break;
		case AK_AUDIO_TYPE_AMR:
			cur_decode_type = DECODE_TYPE_AMR;
			break;
		case AK_AUDIO_TYPE_AAC:
			cur_decode_type = DECODE_TYPE_AAC;
			break;
		case AK_AUDIO_TYPE_PCM_ALAW:
			cur_decode_type = DECODE_TYPE_G711A;
			break;
		case AK_AUDIO_TYPE_PCM_ULAW:
			cur_decode_type = DECODE_TYPE_G711U;
			break;
		case AK_AUDIO_TYPE_PCM:
			cur_decode_type = DECODE_TYPE_RAW_PCM;
			break;
		default:
			cur_decode_type = DECODE_TYPE_MAX;
			break;
	}
	return cur_decode_type;
}

/**
 * adec_need_save_stream - whether need to save stream  
 * @file_info[IN]: save file information
 * @save_level[IN]: save file level
 * return: 1 need to save file,0 no need to save file
 * notes: 
 */
static int adec_need_save_stream(struct save_file_info *file_info,
								int save_level)
{
	int ret = 0;
	struct ak_timeval *start_time = NULL;
	start_time = file_info->save_start_time;

	if (!start_time) {
		ret = 1;
		goto save_stream_end;
	}

	struct ak_timeval cur_time;
	ak_get_ostime(&cur_time);
	long diff_ms = ak_diff_ms_time(&cur_time, start_time);

	if (diff_ms >= (file_info->save_second * 1000))
		ret = 0;
	else
		ret = 1;
save_stream_end:
	return ret;
}

static void adec_parse_ipc_args(char *buf, unsigned int len,
							struct adec_ipc_t *ipc_t)
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
		{"level", 1, NULL, 'l'},//save file level
		{"sec",  1, NULL, 'S'},	//seconds
		{"save", 1, NULL, 's'},	//save on-off switch
		{"type", 1, NULL, 't'},	//save type
		{0, 0, 0, 0}	//end
	};

	optind = 0;
	while ((opt = getopt_long(argc, argv, ":lSst", longopts, NULL)) != -1) {
		switch (opt) {
			case ':':
				break;
			case 'l':
				if (optarg)
					ipc_t->level = atoi(optarg);
				break;
			case 'S':
				if (optarg)
					ipc_t->second = atoi(optarg);
				break;
			case 's':
				if (optarg)
					strncpy(ipc_t->file, optarg, ADEC_PATH_LEN);
				break;
			case 't':
				if (optarg)
					strncpy(ipc_t->type, optarg, TYPE_LEN);
				break;
			default:
				break;
		}
	}
	free(cmd);
}

/**
 * cb_adec_set_status - callbalk function,adec set status 
 * return: 0 success -1 failed
 */
int cb_adec_set_status(char *status, unsigned int len, int sock)
{
	int ret = AK_FAILED;
	char res[RES_LEN] = {0};
	struct adec_ipc_t ipc_t;
	memset(&ipc_t, 0, sizeof(struct adec_ipc_t));

	/* parse arguments */
	adec_parse_ipc_args(status, len, &ipc_t);

	int type_index = get_code_type_index(ipc_t.type);
	if (type_index == DECODE_TYPE_MAX) {
		ak_print_error_ex("unknown type\n");
		goto exit;
	}
	
	if (!ipc_t.type) {
		snprintf(res, RES_LEN, "no type,please set decode type\n");
		goto exit;
	}
	struct decode_info_t *ipc_info = adec_ipcsrv.group[type_index];
	if (!ipc_info) {
		snprintf(res, RES_LEN, "un-inited group %d\n", type_index);
		goto exit;
	}
	if (ipc_t.level < 0 || ipc_t.level >= ADEC_MAX_LEVEL)
		goto exit;	/* do nothing */
		
	ipc_info->save_level = ipc_t.level;

	/* start to save file, file-path is necessary */
	if ((ipc_t.second > 0) && (strlen(ipc_t.file) > 0)) {
		/*
		 * if there is no indicate save second of frames, 30 is default
		 * file path should set first, incase of error occur by thread-schdule
		 */
		strncpy(ipc_info->file_info[AFTER_DEC_LEVEL].save_stream_path,
				ipc_t.file, ADEC_PATH_LEN);
		set_pcm_file_name(ipc_t.file, type_index, ipc_info->save_level);

		int i = 0;
		for (i = 0; i < ipc_t.level + 1; i++) {
			ipc_info->file_info[i].save_second = ipc_t.second;
			if (ipc_info->file_info[i].save_start_time) {
				free(ipc_info->file_info[i].save_start_time);
				ipc_info->file_info[i].save_start_time = NULL;
			}
		}

		snprintf(res, RES_LEN, "start to save %d second stream to file: %s,level=%d\n",
				ipc_t.second, ipc_t.file, ipc_info->save_level);
		if (ipc_t.level > AFTER_DEC_LEVEL) {
			snprintf(res, RES_LEN, "start to save %d second stream to file: %s\n",
					ipc_t.second, 
					ipc_info->file_info[AFTER_DEC_LEVEL].save_stream_path);
		}
		
	} else if (((ipc_t.second > 0) && (!strlen(ipc_t.file))) 
			|| ((!ipc_t.second) && (strlen(ipc_t.file) > 0))) {
		ipc_info->file_info[ipc_t.level].save_second = 0;
		bzero(ipc_info->file_info[ipc_t.level].save_stream_path, ADEC_PATH_LEN);

		snprintf(res, RES_LEN, "must give filename and second to save file\n"
				"e.g: --sec 10 --save /mnt/decode_audio\n");
		goto exit;
	} else {
		snprintf(res, RES_LEN, "no option match, nothing to be do\n");
		goto exit;
	}
	ret = AK_SUCCESS;
exit:
	if (sock)
		ak_cmd_result_response(res, strlen(res), sock);
	return ret;
}

/**
 * cb_adec_get_status - callbalk function,adec get status 
 * return: 0 success -1 failed
 */
int cb_adec_get_status(char *status, unsigned int len, int sock)
{
	int ret = AK_FAILED;
	char res[RES_LEN] = {0};
	struct adec_ipc_t ipc_t = {{0}, {0}, 0};

	/* parse arguments */
	adec_parse_ipc_args(status, len, &ipc_t);

	int type_index = get_code_type_index(ipc_t.type);
	if (type_index == DECODE_TYPE_MAX) {
		ak_print_error_ex("unknown type\n");
		goto exit;
	}
	
	/* get adec internal handle */
	void *handle = adec_ipcsrv.group[type_index]->adec_handle;
	if (!handle) {
		snprintf(res, RES_LEN, "handle is NULL\n");
		goto exit;	/* do nothing */
	}
	struct audio_param param = {0};
	ak_adec_get_params(handle, &param);

	snprintf(res, RES_LEN, "sample_rate:%d\nchannel_num:%d\nsample_bits:%d\n",
			param.sample_rate, param.channel_num, param.sample_bits);
	ret = AK_SUCCESS;
exit:
	if (sock)
		ak_cmd_result_response(res, RES_LEN, sock);
	return ret;
}

/**
 * cb_adec_usage - callbalk function,adec usage 
 * return: 0 success -1 failed
 */
int cb_adec_usage(char *status, unsigned int len, int sock)
{
	char res[RES_LEN] = {0};
	snprintf(res, RES_LEN, "module aenc\n"
			"\t[--save FILE]       save file\n"
			"\t[--sec  SEC]        save seconds\n"
			"\t[--type TYPE]       decode type\n"	
			"\t[--level  LEVEL]    save level,value: [0,1],"			
			"0:save after decode pcm data,1:save before decode audio data\n\n"		
			"\te.g: save 10s after decode pcm to file: "
			"--sec 10 --save /mnt/save_aenc_file\n"
			"\te.g: save 20s after decode pcm and before decode data to file:"
			" --sec 20 --save /mnt/save_aenc_file --level 1\n");
	if (sock)
		ak_cmd_result_response(res, strlen(res), sock);

	return AK_SUCCESS;
}

/**
 * cb_adec_get_sysipc_version - callbalk function,adec get version 
 * return: 0 success -1 failed
 */
int cb_adec_get_sysipc_version(char *status, unsigned int len, int sock)
{
	char res[50] = {0};
	snprintf(res, 50, "%s\n", ak_adec_get_version());
	if (sock)
		ak_cmd_result_response(res, strlen(res), sock);

	return AK_SUCCESS;
}

/**
 * adec_save_stream_to_file - save stream to file  
 * @origin_type[IN]: origin decode type
 * @buf[IN]: adec data
 * @len[IN]: adec data length
 * @save_level[IN]: adec data save level [0,1]
 * return: 0 success -1 failed
 * notes: 
 */
int adec_save_stream_to_file(int origin_type, const unsigned char *buf, int len,
							enum adec_level_index save_level)
{
	/* get cur_type,match enum da_encode_type */
	int cur_type = get_cur_decode_type(origin_type);
	if (cur_type >= DECODE_TYPE_MAX) {
		ak_print_error_ex("unknow type\n");
		return AK_FAILED;
	}
	
	struct decode_info_t *ipc_info = adec_ipcsrv.group[cur_type];
	if (!ipc_info) {
		return AK_FAILED;
	}

	if (save_level > ipc_info->save_level)
		return AK_FAILED;	// this level not have to save

	struct save_file_info *file_info = &(ipc_info->file_info[save_level]);
	if (!file_info || !file_info->save_second) {
		return AK_FAILED;
	}

	if (!buf || !len) {
		ak_print_error_ex("no data\n");
		return -1;
	}

	if (adec_need_save_stream(file_info, save_level)) {
		/* create stream file */
		if (!file_info->save_stream_fd) {
			file_info->save_stream_fd = fopen(file_info->save_stream_path, 
												"w+");
			if (!file_info->save_stream_fd) {
				ak_print_error_ex("open %s fail, %s\n", 
							file_info->save_stream_path, strerror(errno));
				file_info->save_second = 0;
				return AK_FAILED;
			}
			if (cur_type == DECODE_TYPE_AMR && 
					save_level == BEFORE_DEC_LEVEL) {
				const unsigned char amrheader[]= "#!AMR\n";
				fwrite(amrheader, sizeof(amrheader) - 1, 1,
							file_info->save_stream_fd);
			}
		}

		/* write data entry */
		if (file_info->save_stream_fd) {
			if (!file_info->save_start_time) {
				file_info->save_start_time = (struct ak_timeval *)calloc(1,
								sizeof(struct ak_timeval));
				ak_get_ostime(file_info->save_start_time);
				ak_print_normal_ex("save_start_time %ld\n",
									file_info->save_start_time->sec);
			}
			
			if (fwrite(buf, len, 1, file_info->save_stream_fd) <= 0) {
				ak_print_error_ex("write error len:%d,path:%s\n",
						len, file_info->save_stream_path);
				return AK_FAILED;
			}
		}
	} else if (file_info->save_stream_fd) {	/* on write done, close file */
		fclose(file_info->save_stream_fd);
		file_info->save_stream_fd = NULL;
		file_info->save_second = 0;
		if (file_info->save_start_time) {
			free(file_info->save_start_time);
			file_info->save_start_time = NULL;
		}
		ak_print_notice_ex("save stream done\n");
	}

	return 0;
}

/**
 * adec_sys_ipc_register - register to ipc 
 * return: NULL
 * notes: 
 */
void adec_sys_ipc_register(void)
{
	if (adec_ipcsrv.adec_register) {
		ak_print_warning_ex("adec has register already\n");
		return;	/* adec has register already */
	}

	int i;
	for (i = 0; i < DECODE_TYPE_MAX; i++) {
		adec_ipcsrv.group[i] = (struct decode_info_t*)calloc(1,
				sizeof(struct decode_info_t));
		adec_ipcsrv.group[i]->decode_type = i;
		adec_ipcsrv.group[i]->save_level = AFTER_DEC_LEVEL;
		memset(&(adec_ipcsrv.group[i]->file_info), 0, 
				sizeof(struct save_file_info) * ADEC_MAX_LEVEL);
	}

	/* use for build up IPC system, use for debug */
	struct ak_ipc_msg_t set, get, help, version;

	set.cmd = "adec_set_status";
	set.msg_cb = cb_adec_set_status;
	ak_cmd_register_msg_handle(ANYKA_IPC_PORT, &set);

	get.cmd = "adec_get_status";
	get.msg_cb = cb_adec_get_status;
	ak_cmd_register_msg_handle(ANYKA_IPC_PORT, &get);

	help.cmd = "adec_usage";
	help.msg_cb = cb_adec_usage;
	ak_cmd_register_msg_handle(ANYKA_IPC_PORT, &help);

	version.cmd = "adec_version";
	version.msg_cb = cb_adec_get_sysipc_version;
	ak_cmd_register_msg_handle(ANYKA_IPC_PORT, &version);

	ak_cmd_register_module(ANYKA_IPC_PORT, "adec");
	adec_ipcsrv.adec_register = 1;
}

/**
 * adec_sys_ipc_unregister - unregister to ipc 
 * return: NULL
 * notes: 
 */
void adec_sys_ipc_unregister(void)
{
	int i;
	int handle_count = 0;

	for (i = 0; i < DECODE_TYPE_MAX; i++) {
		if (adec_ipcsrv.group[i]) {
			if (adec_ipcsrv.group[i]->adec_handle) {
				handle_count++;
			}
		}
	}
	if (handle_count)
		return;

	ak_cmd_unregister_msg_handle(ANYKA_IPC_PORT,"adec_set_status");
	ak_cmd_unregister_msg_handle(ANYKA_IPC_PORT,"adec_get_status");
	ak_cmd_unregister_msg_handle(ANYKA_IPC_PORT,"adec_usage");
	ak_cmd_unregister_msg_handle(ANYKA_IPC_PORT,"adec_version");

	for (i = 0; i < DECODE_TYPE_MAX; i++) {
		if (adec_ipcsrv.group[i]) {
			free(adec_ipcsrv.group[i]);
			adec_ipcsrv.group[i] = NULL;
		}
	}
	ak_cmd_unregister_module(ANYKA_IPC_PORT, "adec");
	adec_ipcsrv.adec_register = 0;
}

/**
 * adec_sysipc_bind_handle - bind adec handle 
 * @handle[IN]: audio decode opened handle
 * return: NULL
 * notes: 
 */
void adec_sysipc_bind_handle(void *handle)
{
	if (!handle) {
		ak_print_warning_ex("adec handle is not working\n");
		return ;
	}

	struct audio_param param = {0};
	ak_adec_get_params(handle, &param);
	print_decode_type(param.type);

	/* store adec handle to this module */
	int decode_type = get_cur_decode_type(param.type);
	ak_print_normal_ex("array index=%d\n", decode_type);
	if (decode_type >= DECODE_TYPE_MAX) 
		return;
		
	struct decode_info_t *match_handle = adec_ipcsrv.group[decode_type];

	if (match_handle) {
		if (!match_handle->adec_handle) {
			match_handle->adec_handle = handle;
			match_handle->save_level = AFTER_DEC_LEVEL;
			memset(&(match_handle->file_info), 0, 
				sizeof(struct save_file_info) * ADEC_MAX_LEVEL);
			ak_print_normal_ex("aenc handle bind handle=%p\n", handle);
		} else {
			print_decode_type(param.type);
		}
	}
}

/**
 * adec_sysipc_unbind_handle - unbind adec handle 
 * @origin_type[IN]: audio decode origin type
 * return: NULL
 * notes: 
 */
void adec_sysipc_unbind_handle(int origin_type)
{
	/* clear adec handle from this module */
	int decode_type = get_cur_decode_type(origin_type);	
	if (decode_type >= DECODE_TYPE_MAX) {
		ak_print_error_ex("unknown type\n");
	}
	
	struct decode_info_t *match_handle = adec_ipcsrv.group[decode_type];
	print_decode_type(origin_type);	
	
	if (match_handle) {
		match_handle->adec_handle = NULL;
		match_handle->save_level = AFTER_DEC_LEVEL;
		
		memset(&(match_handle->file_info), 0, 
			sizeof(struct save_file_info) * ADEC_MAX_LEVEL);
	}
}

