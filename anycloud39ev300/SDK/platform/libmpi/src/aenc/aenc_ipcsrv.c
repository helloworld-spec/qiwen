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
#include "aenc_ipcsrv.h"

#include "ak_aenc.h"

#define AENC_PATH_LEN 		100		/* encode path length */
#define TYPE_LEN			12		/* encode type string length */
#define RES_LEN				1024	/* encode result string length */

/* encode type */
enum ad_encode_type {
    ENCODE_TYPE_AAC = 0x00,
    ENCODE_TYPE_G711A,
    ENCODE_TYPE_G711U,
    ENCODE_TYPE_AMR,
    ENCODE_TYPE_MP3,
    ENCODE_TYPE_RAW_PCM,
    ENCODE_TYPE_MAX
};

/* use for Inter-Process Communication */
struct aenc_ipc_t {
	char file[AENC_PATH_LEN];	/* file include path */
	char type[TYPE_LEN];		/* encode type */
	int sample_rate;			/* encode sample rate */
	int sample_bits;			/* encode bit */
	int channel_num;			/* channel number */
	int second;					/* save stream second */
	int level;					/* save file level */
};

struct save_file_info {
	FILE *save_stream_fd;					/* save pcm file fd	*/
	int save_second;						/* save file seconds */	
	struct ak_timeval *save_start_time;		/* save stream start time */
	char save_stream_path[AENC_PATH_LEN];	/* save pcm absolutely path */
};

struct encode_info_t {
	void *aenc_handle;					/* aenc handle */
	int encode_type;					/* encode type */
	int save_level; 					/* stream file save level:0~1 */
	struct save_file_info file_info[AENC_MAX_LEVEL];/* save stream path */
};

struct aenc_ipcsrv_t {
    unsigned char register_flag;				 /* whether aenc register */
	struct encode_info_t *group[ENCODE_TYPE_MAX];/* encode information group */
};

static struct aenc_ipcsrv_t aenc_ipcsrv;	/* encode ipc service */

static void print_encode_type(int origin_type)
{
	/* the origin_type see the enum ak_audio_type in ak_global.h */
	char enc_type_name[TYPE_LEN];
	memset(enc_type_name, 0, TYPE_LEN);

	/* check encode type */
	switch (origin_type) {
	case AK_AUDIO_TYPE_MP3:
		strncpy(enc_type_name, "mp3" , 3);
		break;
	case AK_AUDIO_TYPE_AMR:
		strncpy(enc_type_name, "amr" , 3);
		break;
	case AK_AUDIO_TYPE_AAC:
		strncpy(enc_type_name, "aac" , 3);
		break;
	case AK_AUDIO_TYPE_PCM_ALAW:
		strncpy(enc_type_name, "g711a" , 5);
		break;
	case AK_AUDIO_TYPE_PCM_ULAW:
		strncpy(enc_type_name, "g711u" , 5);
		break;
	case AK_AUDIO_TYPE_PCM:
		strncpy(enc_type_name, "pcm" , 3);
		break;
	default:
		strncpy(enc_type_name, "unknown" , 7);
		break;
	}

	ak_print_normal_ex("encode_type: %d, name: %s\n",
	    origin_type, enc_type_name);
}

/**
 * get_code_type_index - get encode type index 
 * @encode_type[IN]: audio encode type
 * return: 0 success, -1 failed
 * notes: 
 */
static int get_code_type_index(char *encode_type)
{
	int ret = 0;
	char type[TYPE_LEN] = {0};
	int type_len = strlen(encode_type);

	if (type_len > 0) {
		sprintf(type, "%s", encode_type);

		int i = 0;
		for (i=0; i<type_len; ++i) {
			type[i] = tolower(type[i]);
		}

		/* check the audio file type */
		if (0 == strcmp(type, "mp3")) {
			ret = ENCODE_TYPE_MP3;
		} else if (0 == strcmp(type, "amr")) {
			ret = ENCODE_TYPE_AMR;
		} else if (0 == strcmp(type, "aac")) {
			ret = ENCODE_TYPE_AAC;
		} else if (0 == strcmp(type, "g711a")) {
			ret = ENCODE_TYPE_G711A;
		} else if (0 == strcmp(type, "g711u")) {
			ret = ENCODE_TYPE_G711U;
		} else if (0 == strcmp(type, "pcm")) {
			ret = ENCODE_TYPE_RAW_PCM;
		} else {
			/* unknow type */
			ret = ENCODE_TYPE_MAX;
		}
	}

	return ret;
}

/**
 * get_cur_encode_type - set stream file name 
 * @origin_name[IN]: audio origin encode type
 * return: current encode type
 * notes: 
 */
static int get_cur_encode_type(int origin_type)
{
	int cur_encode_type = ENCODE_TYPE_MAX;

	switch (origin_type) {
	case AK_AUDIO_TYPE_MP3:
		cur_encode_type = ENCODE_TYPE_MP3;
		break;
	case AK_AUDIO_TYPE_AMR:
		cur_encode_type = ENCODE_TYPE_AMR;
		break;
	case AK_AUDIO_TYPE_AAC:
		cur_encode_type = ENCODE_TYPE_AAC;
		break;
	case AK_AUDIO_TYPE_PCM_ALAW:
		cur_encode_type = ENCODE_TYPE_G711A;
		break;
	case AK_AUDIO_TYPE_PCM_ULAW:
		cur_encode_type = ENCODE_TYPE_G711U;
		break;
	case AK_AUDIO_TYPE_PCM:
		cur_encode_type = ENCODE_TYPE_RAW_PCM;
		break;
	default:
		cur_encode_type = ENCODE_TYPE_MAX;
		break;
	}

	return cur_encode_type;
}

/**
 * set_pcm_file_name - set stream file name 
 * @origin_name[IN]: audio origin encode type
 * @type_index[IN]: audio type index
 * @save_level[IN]: audio file save level
 * return: NULL
 * notes: 
 */
static void set_pcm_file_name(const char *origin_name,
							int type_index, int save_level)
{
	if (save_level != BEFORE_ENC_LEVEL) 
		return;
		
	char temp_path[AENC_PATH_LEN];
	strncpy(temp_path, origin_name, AENC_PATH_LEN);

	int len = strlen(temp_path);
	if (len > AENC_PATH_LEN - 10) {
		ak_print_error_ex("path too long\n");
		return;
	}

	char *pstr = strstr(temp_path, ".");
	if (pstr)
		strncpy(pstr, "_" , 1); // 
		
	struct encode_info_t *ipc_info = aenc_ipcsrv.group[type_index];
	
	sprintf(ipc_info->file_info[save_level].save_stream_path,
			"%s%s", temp_path, "_aenc.pcm");
}

/**
 * aenc_need_save_stream - whether need to save stream  
 * @file_info[IN]: save file information
 * @save_level[IN]: save file level
 * return: 1 need to save file,0 no need to save file
 * notes: 
 */
static int aenc_need_save_stream(struct save_file_info *file_info,
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

static void aenc_parse_ipc_args(char *buf, unsigned int len,
							struct aenc_ipc_t *ipc)
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
		{"level", 1, NULL, 'l'},	//save file level
		{"sec",  1, NULL, 'S'},		//seconds
		{"save", 1, NULL, 's'},		//save file
		{"type", 1, NULL, 't'},		//encode type
		{0, 0, 0, 0}	//end
	};
	optind = 0;
	while ((opt = getopt_long(argc, argv, ":lSst", longopts, NULL)) != -1) {
		switch (opt) {
		case ':':
			break;
		case 'l':
			if (optarg)
				ipc->level = atoi(optarg);
			break;
		case 'S':
			if (optarg)
				ipc->second = atoi(optarg);
			break;
		case 's':
			if (optarg)
				strncpy(ipc->file, optarg, AENC_PATH_LEN);
			break;
		case 't':
			if (optarg)
				strncpy(ipc->type, optarg, TYPE_LEN);
			break;
		default:
			break;
		}
	}
	free(cmd);
}

/**
 * cb_aenc_set_status - callbalk function,aenc set status 
 * return: 0 success -1 failed
 */
int cb_aenc_set_status(char *status, unsigned int len, int sock)
{
	int ret = AK_FAILED;
	char res[RES_LEN] = {0};
	struct aenc_ipc_t ipc;
	memset(&ipc, 0, sizeof(struct aenc_ipc_t));

	/* parse arguments */
	aenc_parse_ipc_args(status, len, &ipc);

	int type_index = get_code_type_index(ipc.type);
	if (type_index == ENCODE_TYPE_MAX) {
		ak_print_error_ex("unknown type\n");
		goto exit;
	}
	
	if (!ipc.type) {
		snprintf(res, RES_LEN, "no type, please set encode type\n");
		goto exit;
	}
	
	struct encode_info_t *ipc_info = aenc_ipcsrv.group[type_index];
	if (!ipc_info) {
		snprintf(res, RES_LEN, "un-inited group %d\n", type_index);
		goto exit;
	}

	if (ipc.level < 0 || ipc.level >= AENC_MAX_LEVEL)
		goto exit;	/* do nothing */
		
	ipc_info->save_level = ipc.level;

	/* start to save file, file-path is necessary */
	if ((ipc.second > 0) && (strlen(ipc.file) > 0)) {			
		/*
		 * if there is no indicate save second of frames
		 * file path should set first, incase of error occur by thread-schdule
		 */
		strncpy(ipc_info->file_info[AFTER_ENC_LEVEL].save_stream_path,
				ipc.file, AENC_PATH_LEN);
		
		set_pcm_file_name(ipc.file, type_index, ipc_info->save_level);

		int i = 0;
		for (i = 0; i < ipc.level + 1; i++) {
			ipc_info->file_info[i].save_second = ipc.second;
			if (ipc_info->file_info[i].save_start_time) {
				free(ipc_info->file_info[i].save_start_time);
				ipc_info->file_info[i].save_start_time = NULL;
			}
		}

		snprintf(res, RES_LEN, "start to save %d second stream to file: %s,level=%d\n",
				ipc.second, ipc.file, ipc_info->save_level);
		if (ipc.level > AFTER_ENC_LEVEL) {
			snprintf(res, RES_LEN, "start to save %d second stream to file: %s\n",
					ipc.second, 
					ipc_info->file_info[AFTER_ENC_LEVEL].save_stream_path);
		}
		
	} else if (((ipc.second > 0) && (!strlen(ipc.file))) 
			|| ((!ipc.second) && (strlen(ipc.file) > 0))) {
		ipc_info->file_info[ipc.level].save_second = 0;
		bzero(ipc_info->file_info[ipc.level].save_stream_path, AENC_PATH_LEN);
		snprintf(res, RES_LEN, "save file must set FILE and SECODN\n"
				"e.g: --save /mnt/testfile --sec 10\n");
		goto exit;	/* do nothing */
	} else {
		snprintf(res, RES_LEN, " no option was given, nothing to be do\n");
		goto exit;	/* do nothing */
	}
exit:
	ret = AK_SUCCESS;
	if (sock)
		ak_cmd_result_response(res, strlen(res), sock);

	return ret;
}

/**
 * cb_adec_get_status - callbalk function,adec get status 
 * return: 0 success -1 failed
 */
int cb_aenc_get_status(char *status, unsigned int len, int sock)
{
	int ret = AK_FAILED;
	char res[RES_LEN] = {0};
	struct aenc_ipc_t ipc= {{0}, {0}, 0};

	/* parse arguments */
	aenc_parse_ipc_args(status, len, &ipc);

	int type_index = get_code_type_index(ipc.type);
	if (type_index == ENCODE_TYPE_MAX) {
		ak_print_error_ex("unknown type\n");
		goto exit;
	}
	
	/* get aenc internal handle */
	void *handle = aenc_ipcsrv.group[type_index]->aenc_handle;
	if (!handle) {
		snprintf(res, RES_LEN, "handle is NULL\n");
		goto exit;	/* do nothing */
	}
	struct audio_param param = {0};
	ak_aenc_get_params(handle, &param);

	snprintf(res, RES_LEN, " sample_rate:%d\n channel_num:%d\n sample_bits:%d\n",
			param.sample_rate, param.channel_num, param.sample_bits);
	ret = AK_SUCCESS;
exit:
	if (sock)
		ak_cmd_result_response(res, strlen(res), sock);

	return ret;
}

/**
 * cb_aenc_usage - callbalk function,aenc usage 
 * return: 0 success -1 failed
 */
int cb_aenc_usage(char *status, unsigned int len, int sock)
{
	char res[RES_LEN] = {0};
	snprintf(res, RES_LEN, "module aenc\n"
			"\t[--type TYPE]	   encode type\n"
			"\t[--save FILE]       save file\n"
			"\t[--sec  SEC]        save seconds\n"
			"\t[--level  LEVEL]    save level,value: [0,1],"			
			"0:save after encode audio data,1:save before encode pcm data\n\n"		
			"\te.g: save 10s after encode data to file: "
			"--sec 10 --save /mnt/save_aenc_file\n"
			"\te.g: save 20s after encode data and before encode pcm to file:"
			" --sec 20 --save /mnt/save_aenc_file --level 1\n");
	if (sock)
		ak_cmd_result_response(res, strlen(res), sock);

	return AK_SUCCESS;
}

/**
 * cb_aenc_get_sysipc_version - callbalk function,aenc get version 
 * return: 0 success -1 failed
 */
int cb_aenc_get_sysipc_version(char *status, unsigned int len, int sock)
{
	char res[50] = {0};
	snprintf(res, 50, "%s\n", ak_aenc_get_version());
	if (sock)
		ak_cmd_result_response(res, strlen(res), sock);

	return AK_SUCCESS;
}

/**
 * aenc_save_stream_to_file - save stream to file  
 * @origin_type[IN]: origin encode type
 * @buf[IN]: aenc data
 * @len[IN]: aenc data length
 * @save_level[IN]: aenc data save level [0,1]
 * return: 0 success -1 failed
 * notes: 
 */
int aenc_save_stream_to_file(int origin_type, unsigned char *buf, int len,
							enum aenc_level_index save_level)
{
	/* get cur_type,match enum ad_encode_type */
	int cur_type = get_cur_encode_type(origin_type);
	if (cur_type >= ENCODE_TYPE_MAX) {
		ak_print_error_ex("unknow type\n");
		return AK_FAILED;
	}

	struct encode_info_t *ipc_info = aenc_ipcsrv.group[cur_type];
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
		return AK_FAILED;
	}

	if (aenc_need_save_stream(file_info, save_level)) {
		/* create stream file */
		if (!file_info->save_stream_fd) {
			file_info->save_stream_fd = fopen(file_info->save_stream_path, 
												"w+");
			if (!file_info->save_stream_fd) {
				ak_print_error_ex("open %s fail,save_level=%d-- %s\n", 
							file_info->save_stream_path, 
							save_level, strerror(errno));
				file_info->save_second = 0;
				return AK_FAILED;
			}
			if (cur_type == ENCODE_TYPE_AMR && 
					save_level == AFTER_ENC_LEVEL) {
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
 * aenc_sys_ipc_register - register to ipc 
 * return: NULL
 * notes: 
 */
void aenc_sys_ipc_register(void)
{
	if (aenc_ipcsrv.register_flag) {
		ak_print_warning_ex("aenc ipcsrv has register already\n");
		return;
	}

	int i;
	for (i = 0; i < ENCODE_TYPE_MAX; i++) {
		aenc_ipcsrv.group[i] = (struct encode_info_t*)calloc(1,
				sizeof(struct encode_info_t));
		aenc_ipcsrv.group[i]->encode_type = i;
		aenc_ipcsrv.group[i]->save_level = AFTER_ENC_LEVEL;
		memset(&(aenc_ipcsrv.group[i]->file_info), 0, 
				sizeof(struct save_file_info) * AENC_MAX_LEVEL);
	}

	/* use for build up IPC system, use for debug */
	struct ak_ipc_msg_t set, get, help, version;

	set.cmd = "aenc_set_status";
	set.msg_cb = cb_aenc_set_status;
	ak_cmd_register_msg_handle(ANYKA_IPC_PORT, &set);

	get.cmd = "aenc_get_status";
	get.msg_cb = cb_aenc_get_status;
	ak_cmd_register_msg_handle(ANYKA_IPC_PORT, &get);

	help.cmd = "aenc_usage";
	help.msg_cb = cb_aenc_usage;
	ak_cmd_register_msg_handle(ANYKA_IPC_PORT, &help);

	version.cmd = "aenc_version";
	version.msg_cb = cb_aenc_get_sysipc_version;
	ak_cmd_register_msg_handle(ANYKA_IPC_PORT, &version);

	ak_cmd_register_module(ANYKA_IPC_PORT, "aenc");
	aenc_ipcsrv.register_flag = AK_TRUE;
}

/**
 * aenc_sys_ipc_unregister - unregister to ipc 
 * return: NULL
 * notes: 
 */
void aenc_sys_ipc_unregister(void)
{
	if (!aenc_ipcsrv.register_flag)
		return;
		
	int i;
	int handle_count = 0;
	for (i = 0; i < ENCODE_TYPE_MAX; i++) {
		if (aenc_ipcsrv.group[i]) {
			if (aenc_ipcsrv.group[i]->aenc_handle) {
				handle_count++;
			}
		}
	}

	if (handle_count)
		return;

	ak_cmd_unregister_msg_handle(ANYKA_IPC_PORT, "aenc_set_status");
	ak_cmd_unregister_msg_handle(ANYKA_IPC_PORT, "aenc_get_status");
	ak_cmd_unregister_msg_handle(ANYKA_IPC_PORT, "aenc_usage");
	ak_cmd_unregister_msg_handle(ANYKA_IPC_PORT, "aenc_version");

	for (i = 0; i < ENCODE_TYPE_MAX; i++) {
		if (aenc_ipcsrv.group[i]) {
			free(aenc_ipcsrv.group[i]);
			aenc_ipcsrv.group[i] = NULL;
		}
	}
	aenc_ipcsrv.register_flag = AK_FALSE;
	ak_cmd_unregister_module(ANYKA_IPC_PORT, "aenc");
}

/**
 * aenc_sysipc_bind_handle - bind aenc handle 
 * @handle[IN]: audio encode opened handle
 * return: NULL
 * notes: 
 */
void aenc_sysipc_bind_handle(void *handle)
{
	if (!handle) {
		ak_print_warning_ex("aenc handle is not working\n");
		return ;
	}

	struct audio_param param = {0};

	ak_aenc_get_params(handle, &param);
	print_encode_type(param.type);

	/* store aenc handle to this module */
	int encode_type = get_cur_encode_type(param.type);
	ak_print_info_ex("array index=%d\n", encode_type);
	if (encode_type >= ENCODE_TYPE_MAX)
		return;
		
	struct encode_info_t *match_handle = aenc_ipcsrv.group[encode_type];

	if (match_handle) {
		if (!match_handle->aenc_handle) {
			match_handle->aenc_handle = handle;
			match_handle->save_level = AFTER_ENC_LEVEL;
			memset(&(match_handle->file_info), 0, 
				sizeof(struct save_file_info) * AENC_MAX_LEVEL);
			ak_print_normal_ex("aenc handle bind handle=%p\n", handle);
		} else {		
			print_encode_type(param.type);
		}
	}
}

/**
 * aenc_sysipc_unbind_handle - unbind aenc handle 
 * @origin_type[IN]: audio aenc origin type
 * return: NULL
 * notes: origin_type is the type in global.h enum ak_audio_type
 */
void aenc_sysipc_unbind_handle(int origin_type)
{
	/* clear aenc handle from this module */
	int cur_type = get_cur_encode_type(origin_type);
	print_encode_type(origin_type);
	
	if (cur_type >= ENCODE_TYPE_MAX) {
		ak_print_error_ex("unknown type\n");
		return;
	}
		
	struct encode_info_t *match_handle = aenc_ipcsrv.group[cur_type];

	if (match_handle) {
		match_handle->aenc_handle = NULL;
		match_handle->save_level = AFTER_ENC_LEVEL;
		
		memset(&(match_handle->file_info), 0, 
			sizeof(struct save_file_info) * AENC_MAX_LEVEL);
	}
}

