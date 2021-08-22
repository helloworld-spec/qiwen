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

#include "ak_venc.h"

/* use for Inter-Process Communication */
struct venc_ipc_t {
	char file[100];	//file include path
	int chn;		//channel
	int num;		//save number
	int fps;		//encode fps
	int kbps;		//encode bitrate in k
};

struct venc_chn_t {
	enum encode_group_type chn;	//encode group
	void *venc_handle;          //venc handle
	int save_stream_fd;         //save stream file-describe
	int save_stream_nr;	        //save stream runtime flag, 1 -> save, 0 not
	char save_stream_path[100];	//save stream absolutely path
};

/* global channel structure */
struct venc_ipcsrv_t {
	struct venc_chn_t *venc_chn[ENCODE_GRP_NUM];
};

static struct venc_ipcsrv_t venc_ipcsrv = {{0}};

static void *venc_get_chn_handle(int chn)
{
	struct venc_chn_t *handle = venc_ipcsrv.venc_chn[chn];
	if (!handle) {
		ak_print_warning_ex("video encode channel %d is not working\n", chn);
		return NULL;
	}
	return handle->venc_handle;
}

/* 
 * venc_need_save_stream - check if this channel need to save
 * handle[IN]: channel handle
 * return: 1 need, 0 is not;
 */
static int venc_need_save_stream(struct venc_chn_t *handle)
{
	int ret = 0;

	if (!handle)
		ret = 0;
	else
		ret = (handle->save_stream_nr > 0) ? handle->save_stream_nr-- : 0;

	return ret;
}

/*
 * venc_parse_ipc_args - parse argument
 * buf[IN]: all context buffer
 * len[IN]: buffer len
 * ipc_t[OUT]: store parse result
 */
static void venc_parse_ipc_args(char *buf, unsigned int len,
		struct venc_ipc_t *ipc_t)
{
	char *cmd = (char *)calloc(1, len);
	if (!cmd)
		return;
	memcpy(cmd, buf, len); /* backup string */

	char *str, *saveptr, *token, *argv[50] = {cmd, 0};  /* to skip argv[0] */
	int argc = 1, opt;

	/* division argument by delimit symbol */
	for (str = cmd; ; str = NULL) {
		token = strtok_r(str, DELIMIT, &saveptr);
		if (token == NULL)
			break;
		argv[argc++] = token;
	}

	/* arguments tabel */
	const struct option longopts[] = {
		{"bps",  1, NULL, 'b'},	//bps
		{"chn",  1, NULL, 'c'},	//channel
		{"fps",  1, NULL, 'f'},	//fps
		{"num",  1, NULL, 'n'},	//number
		{"save", 1, NULL, 's'},	//save on-off switch
		{0, 0, 0, 0}	//end
	};

	optind = 0;
	while ((opt = getopt_long(argc, argv, ":bcfns", longopts, NULL)) != -1) {
		switch (opt) {
			case ':':
				ak_print_error("unsupport option: %d\n", opt);
				break;
			case 'c':	//channel
				if (optarg)
					ipc_t->chn = atoi(optarg);
				break;
			case 'n':	//number
				if (optarg)
					ipc_t->num = atoi(optarg);
				break;
			case 'f':	//fps
				if (optarg)
					ipc_t->fps = atoi(optarg);
				break;
			case 'b':	//bit rate
				if (optarg)
					ipc_t->kbps = atoi(optarg);
				break;
			case 's':	//save file path
				if (optarg)
					strncpy(ipc_t->file, optarg, 100);
				break;
			default:
				break;
		}
	}
	free(cmd);
}

/* 
 * venc_save_stream_to_file - save stream to file
 * chn[IN]: channel number
 * buf[IN]: pointer to buffer
 * len[IN]: indicate buffer's len
 */
int venc_save_stream_to_file(int chn, unsigned char *buf, int len)
{
	struct venc_chn_t *handle = venc_ipcsrv.venc_chn[chn];

	if (venc_need_save_stream(handle)) {
		/* create stream file */
		if (handle->save_stream_fd == -1) {
			handle->save_stream_fd = open(handle->save_stream_path,
					O_CREAT | O_RDWR | 0644);
			if (handle->save_stream_fd == -1) {
				ak_print_error_ex("open %s fail, %s\n",
					   	handle->save_stream_path, strerror(errno));
				handle->save_stream_nr = 0;
				return -1;
			}
		}

		/* write data entry */
		if (handle->save_stream_fd != -1) {
			int savelen;
			savelen = write(handle->save_stream_fd, buf, len);
			if (savelen != len) {
				ak_print_error_ex("savelen:%d, len:%d\n", savelen, len);
				return -1;
			}
		}
	} else if (handle->save_stream_fd != -1) {	/* on write done, close file */
		close(handle->save_stream_fd);
		handle->save_stream_fd = -1;
		ak_print_notice_ex("save stream on ch %d done\n", chn);
	}

	return 0;
}

/*
 * venc_get_status - get video encode module's status
 * status[OUT]: pointer to buffer to store status
 * len[IN]: indicate buffer's length
 * sock[IN]: use for response in need
 * return: 0 on success, -1 failed
 */
int venc_get_status(char *status, unsigned int len, int sock)
{
	int i;
	char res[1024] = {0};

	for (i = 0; i < ENCODE_GRP_NUM; i++) {
		/* get ak_venc internal handle */
		void *handle = venc_get_chn_handle(i);
		if (!handle)
			continue;	/* do nothing */
		int fps = ak_venc_get_fps(handle);
		int kbps = ak_venc_get_kbps(handle);
		snprintf(res, 1024 - strlen(res),
				"%schn %d, target fps: %d target kbps: %d\n",
				res, i, fps, kbps);
	}
	/* response result if needed */
	if (sock)
		ak_cmd_result_response(res, strlen(res), sock);

	return AK_SUCCESS;
}

/*
 * venc_set_status - set video encode module's status
 * status[IN]: pointer to buffer to indicate which scope
 *             will be set
 * len[IN]: indicate buffer's length
 * sock[IN]: use for response in need
 * return: 0 on success, -1 failed
 */
int venc_set_status(char *status, unsigned int len, int sock)
{
	int ret = AK_FAILED;
	char res[1024] = {0};
	struct venc_ipc_t ipc = {{0}, -1, 0};

	/* parse arguments */
	venc_parse_ipc_args(status, len, &ipc);

	ak_print_info("file:%s\nchn:%d\nnum:%d\nfps:%d\nkbps:%d\n",
			ipc.file, ipc.chn, ipc.num, ipc.fps, ipc.kbps);

	if (ipc.chn == -1) {
		snprintf(res, 1024, "unspeciffic channel\n");
		goto exit;	/* do nothing */
	}

	struct venc_chn_t *handle = venc_ipcsrv.venc_chn[ipc.chn];
	if (!handle) {
		snprintf(res, 1024, "cann't get venc handle on channel: %d\n", ipc.chn);
		goto exit;	/* do nothing */
	}

	if (ipc.fps > 0) {
		ak_venc_set_fps(handle->venc_handle, ipc.fps);
		snprintf(res, 1024 - strlen(res), "%sset channel %d fps %d\n",
			res, ipc.chn, ipc.fps);
	}

	if (ipc.kbps > 0) {
		ak_venc_set_rc(handle->venc_handle, ipc.kbps);
		snprintf(res, 1024 - strlen(res), "%sset channel %d bps %d\n",
			res, ipc.chn, ipc.kbps);
	}

	/*
	 * if there is no indicate save number of frames
	 * file path should set first, incase of error occur by thread-schdule
	 */
	if ((ipc.num > 0) && (strlen(ipc.file) > 0)) {
		strcpy(handle->save_stream_path, ipc.file);
		handle->save_stream_nr = ipc.num;
		snprintf(res, 1024 - strlen(res), 
				"%sstart to save %d frame stream on chn %d to file: %s\n",
				res, handle->save_stream_nr, handle->chn, handle->save_stream_path);
	} else if (((!ipc.num) && (strlen(ipc.file) > 0)) 
			|| ((ipc.num > 0) && (!strlen(ipc.file)))) {
		/* run to here, actually, channel was given */
		handle->save_stream_nr = 0;
		bzero(handle->save_stream_path, sizeof(handle->save_stream_path));
		snprintf(res, 1024 - strlen(res), 
				"%smust set filename and number and channel to save file\n"
				"e.g: --chn 1 --num 100 --save /mnt/save_stream.str\n", res);
		goto exit;
	} 
	ret = AK_SUCCESS;
exit:
	/* response result if needed */
	if (sock)
		ak_cmd_result_response(res, strlen(res), sock);

	return ret;
}

/*
 * venc_usage - show ipc_srv usage
 * status[OUT]: pointer to buffer use for store result
 * len[IN]: indicate buffer's length
 * sock[IN]: use for response in need
 * return: 0 on success, -1 failed
 */
int venc_usage(char *status, unsigned int len, int sock)
{
	char res[1024] = {0};
	snprintf(res, 1024, "module: venc\n"
			"\t[-g]             get module's active status, such as fps/bps\n"
			"\t[--chn CHN]      specific encode channel\n"
			"\t[--save FILE]    save file\n"
			"\t[--num NUM]      number of save streams\n"
			"\t[--bps N]        bitrates in K, integer\n"
			"\t[--fps N]        frame rate\n"
			"\te.g: --chn 1 --fps 25 --bps 2000\n"
			"\te.g: --chn 1 --num 100 --save /mnt/test.stream\n"
			"\te.g: -g\n");
	/* response result if needed */
	if (sock)
		ak_cmd_result_response(res, strlen(res), sock);

	return AK_SUCCESS;
}

/*
 * venc_get_sysipc_version - start to save stream data
 * status[OUT]: pointer to buffer use for store result
 * len[IN]: indicate buffer's length
 * sock[IN]: use for response in need
 * return: 0 on success, -1 failed
 */
int venc_get_sysipc_version(char *status, unsigned int len, int sock)
{
	char res[50] = {0};
	snprintf(res, 50, "%s\n", ak_venc_get_version());

	/* response result if needed */
	if (sock)
		ak_cmd_result_response(res, strlen(res), sock);

	return AK_SUCCESS;
}

/*
 * venc_sys_ipc_register - register message handle
 */
void venc_sys_ipc_register(void)
{
	/* use for build up IPC system, use for debug */
	struct ak_ipc_msg_t set, get, help, version;

	int i;
	for (i = 0; i < ENCODE_GRP_NUM; i++) {
		venc_ipcsrv.venc_chn[i] = (struct venc_chn_t*)calloc(1,
				sizeof(struct venc_chn_t));
		venc_ipcsrv.venc_chn[i]->chn = i;
		venc_ipcsrv.venc_chn[i]->save_stream_fd = -1;
	}

	/* register setting message handle */
	set.cmd = "venc_set_status";
	set.msg_cb = venc_set_status;
	ak_cmd_register_msg_handle(ANYKA_IPC_PORT, &set);

	/* register getting message handle */
	get.cmd = "venc_get_status";
	get.msg_cb = venc_get_status;
	ak_cmd_register_msg_handle(ANYKA_IPC_PORT, &get);

	/* register usage message handle */
	help.cmd = "venc_usage";
	help.msg_cb = venc_usage;
	ak_cmd_register_msg_handle(ANYKA_IPC_PORT, &help);

	/* register version message handle */
	version.cmd = "venc_version";
	version.msg_cb = venc_get_sysipc_version;
	ak_cmd_register_msg_handle(ANYKA_IPC_PORT, &version);

	/* register venc module to server */
	ak_cmd_register_module(ANYKA_IPC_PORT, "venc");
}

/*
 * venc_sys_ipc_unregister - unregister message
 */
void venc_sys_ipc_unregister(void)
{
	ak_cmd_unregister_msg_handle(ANYKA_IPC_PORT,"venc_set_status");
	ak_cmd_unregister_msg_handle(ANYKA_IPC_PORT,"venc_get_status");
	ak_cmd_unregister_msg_handle(ANYKA_IPC_PORT,"venc_usage");
	ak_cmd_unregister_msg_handle(ANYKA_IPC_PORT,"venc_version");

	int i;
	for (i = 0; i < ENCODE_GRP_NUM; i++) {
		if (venc_ipcsrv.venc_chn[i]) {
			free(venc_ipcsrv.venc_chn[i]);
			venc_ipcsrv.venc_chn[i] = NULL;
		}
	}

	ak_cmd_unregister_module(ANYKA_IPC_PORT, "venc");
}

/* 
 * venc_sysipc_bind_chn_handle - bind handle
 * handle[IN]: video encode handle, return by ak_venc_open();
 * chn[IN]: indicate handle belongs to which channel
 */
void venc_sysipc_bind_chn_handle(void *handle, int chn)
{
	struct venc_chn_t *chn_handle = venc_ipcsrv.venc_chn[chn];
	if (!chn_handle) {
		ak_print_warning_ex("video encode channel %d is not working\n", chn);
		return ;
	}
	/* store ak_venc encode handle to this module */
	chn_handle->venc_handle = handle;
}

/* 
 * venc_sysipc_unbind_chn_handle - unbind handle
 * chn[IN]: indicate handle belongs to which channel
 */
void venc_sysipc_unbind_chn_handle(int chn)
{
	struct venc_chn_t *handle = venc_ipcsrv.venc_chn[chn];
	if (!handle) {
		ak_print_warning_ex("video encode channel %d is not working\n", chn);
		return ;
	}

	/* clear ak_venc encode handle from this module */
	if (venc_get_chn_handle(chn))
		handle->venc_handle = NULL;
}
