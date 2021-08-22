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

#include "ak_vi.h"

/* use for Inter-Process Communication */
struct vi_ipc_t {
	char file[100];	//file include path
	int chn;		//channel, main or sub
	int dev_no;		//device number
	int num;		//save number
};

struct vi_chn_t {
	void *vi_handle;
	int save_yuv_fd[3];
	int save_yuv_nr[3];	//save yuv runtime flag, 1 -> save, 0 not
	int save_yuv_nr_total[3];	//save yuv total number
	char save_yuv_path[3][100];	//save yuv absolutely path
};

struct vi_ipcsrv_t {
	struct vi_chn_t *vi_chn[VIDEO_DEV_NUM];
	int registed_flag;
};

static struct vi_ipcsrv_t vi_ipcsrv = {{0}, 0};

/* check if it's needed to save yuv on each channel */
static int vi_need_save_yuv(struct vi_chn_t *handle, int chn)
{
	int ret = 0;
	if (!handle)
		ret = 0;
	else
		ret = (handle->save_yuv_nr[chn] > 0) ?  handle->save_yuv_nr[chn]-- : 0;
	return ret;
}

static void vi_parse_ipc_args(char *buf, unsigned int len,
		struct vi_ipc_t *ipc)
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
		{"chn",  1, NULL, 'c'},		//channel, main or sub
		{"devno",  1, NULL, 'd'},	//dev number
		{"num",  1, NULL, 'n'},		//number
		{"save", 1, NULL, 's'},		//save file(s)
		{0, 0, 0, 0}	//end
	};

	optind = 0;
	while ((opt = getopt_long(argc, argv, ":cdfns", longopts, NULL)) != -1) {
		switch (opt) {
			case ':':
				break;
			case 'c':	//to mach 'c', means channel
				if (optarg)
					ipc->chn = atoi(optarg);
				break;
			case 'd':	//to mach 'd', means device number
				if (optarg)
					ipc->dev_no = atoi(optarg);
				break;
			case 'n':	//to mach 'n', means number
				if (optarg)
					ipc->num = atoi(optarg);
				break;
			case 's':	//to mach 's', means save file name
				if (optarg)
					strncpy(ipc->file, optarg, 100);
				break;
			default:
				break;
		}
	}
	free(cmd);
}

/* open file for writing yuv*/
static int vi_ipcsrv_open_file(struct vi_chn_t *handle, int chn)
{
	char file_name[128] = {0};
	sprintf(file_name, "%s.%d.yuv", handle->save_yuv_path[chn],
			(handle->save_yuv_nr_total[chn] - handle->save_yuv_nr[chn]));

	/* if no exist then create it */
	int fd = open(file_name, O_CREAT | O_RDWR | 0644);
	if (fd == -1) {
		ak_print_error_ex("open %s fail, %s\n",
				handle->save_yuv_path[chn], strerror(errno));
		handle->save_yuv_nr[chn] = 0;
		return -1;
	} else if (fd == STDIN_FILENO || fd == STDOUT_FILENO || fd == STDERR_FILENO) {
		ak_print_notice_ex("current fd is same as system standard fd,"
				" avoid it, fd: %d\n", fd);
		/* if file-describ number same as standard-io number, duplicate it */
		fd = dup(fd);
		ak_print_notice_ex("new fd is %d\n", fd);
		/* 
		 * TODO:
		 * To avoid fd same as the std-io number absolutely, 
		 * should use do-while{}.
		 */
	}

	return fd;
}

/* calculate yuv buffer len */
static inline int vi_get_yuv_len(int w, int h)
{
	int len = (w * h * 3 / 2);
	return len;
}

static char *vi_get_write_buf_and_len(void *handle,
	   const char *start, int chn, int *len)
{
	char *save_start = (char *)start;
	int offset = 0;
	struct video_channel_attr attr;

	memset(&attr, 0, sizeof(struct video_channel_attr));
	ak_vi_get_channel_attr(handle, &attr);

	/* main channel*/
	switch (chn) {
		case 0:
			/* main channel yuv pointer */
			*len = vi_get_yuv_len(attr.res[VIDEO_CHN_MAIN].width,
					attr.res[VIDEO_CHN_MAIN].height);
			offset = 0;
			break;
		case 1:
			/* sub channel yuv pointer */
			*len = vi_get_yuv_len(attr.res[VIDEO_CHN_SUB].width,
					attr.res[VIDEO_CHN_SUB].height);
			offset = vi_get_yuv_len(attr.res[VIDEO_CHN_MAIN].width,
					attr.res[VIDEO_CHN_MAIN].height);
			break;
		case 2:
			/* third channel yuv pointer */
			*len = vi_get_yuv_len(attr.res[VIDEO_CHN_SUB].width/2,
					attr.res[VIDEO_CHN_SUB].height/2);
			offset = vi_get_yuv_len(attr.res[VIDEO_CHN_MAIN].width,
					attr.res[VIDEO_CHN_MAIN].height);
			offset += vi_get_yuv_len(attr.res[VIDEO_CHN_SUB].width,
					attr.res[VIDEO_CHN_SUB].height);
			break;
		default:
			*len = 0;
			offset = 0;
			break;
	}
	/* calculate offset, then return it to caller */
	save_start = (char *)start + offset;

	return save_start;
}

/*
 * vi_save_yuv_to_file - write yuv to file
 * dev_no[IN]: device number, same as calling ak_vi_open(dev_no);
 * buf[IN]: yuv data pointer
 * return: 0 on success, -1 failed
 */
int vi_save_yuv_to_file(int dev_no, unsigned char *buf)
{
	struct vi_chn_t *handle = vi_ipcsrv.vi_chn[dev_no];
	int i, failed_cnt = 0;

	for (i = 0; i < 3; i++) {
		if (vi_need_save_yuv(handle, i)) {
			/* create yuv file */
			if (handle->save_yuv_fd[i] == -1) {
				handle->save_yuv_fd[i] = vi_ipcsrv_open_file(handle, i);
				if (handle->save_yuv_fd[i] == -1) {
					failed_cnt++;
					continue; //to next channel
				}
			}
			int savelen, writen_len;
			/* get write buffer ptr */
			char *write_buf = vi_get_write_buf_and_len(handle->vi_handle,
					(const char *)buf, i, &savelen);

			/* write data */
			writen_len = write(handle->save_yuv_fd[i], write_buf, savelen);
			if (savelen != writen_len) {
				ak_print_error_ex("savelen:%d, should be: %d\n", savelen, writen_len);
				failed_cnt++;
			}

			close(handle->save_yuv_fd[i]);
			handle->save_yuv_fd[i] = -1;
			if (!handle->save_yuv_nr[i])
				ak_print_normal_ex("save yuv on chn: %d done\n", i);
		}
	} /* end of for() */

	if (failed_cnt >= 3)
		return AK_FAILED;

	return AK_SUCCESS;
}

/*
 * vi_get_status - get video encode module's status
 * status[OUT]: pointer to buffer to store status
 * len[IN]: indicate buffer's length
 * sock[IN]: use for response in need
 * return: 0 on success, -1 failed
 */
int vi_get_status(char *status, unsigned int len, int sock)
{
	int ret = AK_FAILED;
	char res[1024] = {0};
	struct vi_ipc_t ipc = {{0}, -1, -1, 0};

	/* parse arguments */
	vi_parse_ipc_args(status, len, &ipc);
	ak_print_info_ex("chn: %d\ndev_no: %d\n", ipc.chn, ipc.dev_no);

	if (ipc.dev_no < 0)
		ipc.dev_no = VIDEO_DEV0;

	/* get ak_vi internal handle */
	struct vi_chn_t *handle = vi_ipcsrv.vi_chn[ipc.dev_no];
	if (!handle) {
		snprintf(res, 1024, "unregister handle on dev number: %d\n", 
				ipc.dev_no);
		goto exit;	/* do nothing */
	}
	int fps = ak_vi_get_fps(handle->vi_handle);

	struct video_channel_attr attr;
	memset(&attr, 0, sizeof(struct video_channel_attr));
	ak_vi_get_channel_attr(handle->vi_handle, &attr);

	/* construct resolutution info */
	snprintf(res, 1024, "resolutions:\n"
			"main  chn [%d x %d]\n"
			"sub   chn [%d x %d]\n"
			"third chn [%d x %d]\n"
			"sensor capture fps: %d\n",
			attr.res[VIDEO_CHN_MAIN].width,
			attr.res[VIDEO_CHN_MAIN].height,
	 		attr.res[VIDEO_CHN_SUB].width,
			attr.res[VIDEO_CHN_SUB].height,
			attr.res[VIDEO_CHN_SUB].width/2,
			attr.res[VIDEO_CHN_SUB].height/2, fps);
	ret = AK_SUCCESS;
exit:
	/* if necessary, send result to client */
	if (sock)
		ak_cmd_result_response(res, strlen(res), sock);

	return ret;
}

/*
 * vi_set_status - set video encode module's status
 * status[IN]: pointer to buffer to indicate which scope
 *             will be set
 * len[IN]: indicate buffer's length
 * sock[IN]: use for response in need
 * return: 0 on success, -1 failed
 */
int vi_set_status(char *status, unsigned int len, int sock)
{
	int ret = AK_FAILED;
	char res[1024] = {0};
	struct vi_ipc_t ipc = {{0}};
	ipc.chn = -1;
	/* parse arguments */
	vi_parse_ipc_args(status, len, &ipc);

	/* channel check */
	if (ipc.chn == -1) {
		snprintf(res, 1024, "need a value to indicate channel\n");
		goto exit;
	} else if (ipc.chn > 2 || ipc.chn < 0) {
		snprintf(res, 1024, "unsupport channel %d, must be [0, 2]\n",
			   	ipc.chn);
		goto exit;	/* do nothing */
	}

	/* device number fix */
	if (ipc.dev_no < 0)
		ipc.dev_no = VIDEO_DEV0;

	struct vi_chn_t *handle = vi_ipcsrv.vi_chn[ipc.dev_no];
	if (!handle) {
		snprintf(res, 1024, "cann't get vi handle on dev number: %d\n", 
				ipc.dev_no);
		goto exit;	/* do nothing */
	}

	/*
	 * if there is no indicate save number of frames, error will report
	 * file path should set first, incase of error occur by thread-schdule
	 */
	if ((ipc.num > 0) && (strlen(ipc.file) > 0)) {
		strcpy(handle->save_yuv_path[ipc.chn], ipc.file);
		handle->save_yuv_nr[ipc.chn] = ipc.num;
		handle->save_yuv_nr_total[ipc.chn] = handle->save_yuv_nr[ipc.chn];
		snprintf(res, 1024, "start to save %d frames on chn %d to file: %s\n",
				handle->save_yuv_nr[ipc.chn], ipc.chn,
				handle->save_yuv_path[ipc.chn]);
	} else if (((ipc.num == 0) && (strlen(ipc.file) > 0))	/* argument invalid */
			|| ((ipc.num > 0) && (!strlen(ipc.file)))) {
		handle->save_yuv_nr[ipc.chn] = 0;
		handle->save_yuv_nr_total[ipc.chn] = handle->save_yuv_nr[ipc.chn];
		bzero(handle->save_yuv_path[ipc.chn],
				sizeof(handle->save_yuv_path[ipc.chn]));
		snprintf(res, 1024, "must indicate save numbers and filename\n"
				"e.g: --num 3 --save /mnt/save_yuv\n");
		goto exit;
	} else {
		snprintf(res, 1024, "unknow option, nothing to do\n");
		goto exit;
	}
	ret = AK_SUCCESS;
exit:
	/* if necessary, send result to client */
	if (sock)
		ak_cmd_result_response(res, strlen(res), sock);

	return ret;
}

/*
 * vi_usage - show ipc_srv usage
 * status[in]: pointer to buffer use for store result
 * len[IN]: indicate buffer's length
 * sock[IN]: use for response in need
 * return: 0 on success, -1 failed
 */
int vi_usage(char *status, unsigned int len, int sock)
{
	char res[1024] = {0};
	/* construct usage comment */
	snprintf(res, 1024,
			"module: vi\n"
			"\t[--chn CHN]    specific encode channel [0, 2]\n"
			"\t[--save FILE]  save file\n"
			"\t[--num NUM]    number of save frames\n"
			"\t[-g]           to get information\n\n"
			"notes:\n\tsave option must indicate FILE and NUM\n"
			"       it will stop automatic when save NUM frame(s) done\n"
			"       FILE suffix '.yuv' is not needed, we will add it\n"
			"       save multiple frames, format is 'test.0.yuv test.1.yuv' ...\n"
			"e.g:\t--chn 0 --num 10 --save /mnt/test\n");
	/* if necessary, send result to client */
	if (sock)
		ak_cmd_result_response(res, strlen(res), sock);

	return AK_SUCCESS;
}

/*
 * vi_get_sysipc_version - start to save yuv data
 * status[OUT]: pointer to buffer use for store result
 * len[IN]: indicate buffer's length
 * sock[IN]: use for response in need
 * return: 0 on success, -1 failed
 */
int vi_get_sysipc_version(char *status, unsigned int len, int sock)
{
	char res[50] = {0};
	snprintf(res, 50, "%s\n", ak_vi_get_version());

	/* if necessary, send result to client */
	if (sock)
		ak_cmd_result_response(res, strlen(res), sock);

	return AK_SUCCESS;
}

/* 
 * vi_sys_ipc_register - register vi module 
 * 						 system inter process communication function
 */
void vi_sys_ipc_register(void)
{
	/* use for build up IPC system, use for debug */
	struct ak_ipc_msg_t set, get, help;

	if (vi_ipcsrv.registed_flag)
		return;

	vi_ipcsrv.registed_flag = 1;

	int i, j;
	/* for all device */
	for (i = 0; i < VIDEO_DEV_NUM; i++) {
		vi_ipcsrv.vi_chn[i] = (struct vi_chn_t*)calloc(1,
				sizeof(struct vi_chn_t));
		/* init each channel fd */
		for (j = 0; j < 3; j ++)
			vi_ipcsrv.vi_chn[i]->save_yuv_fd[j] = -1;
	}

	/* register setting message and callback */
	set.cmd = "vi_set_status";
	set.msg_cb = vi_set_status;
	ak_cmd_register_msg_handle(ANYKA_IPC_PORT, &set);

	/* register getting message and callback */
	get.cmd = "vi_get_status";
	get.msg_cb = vi_get_status;
	ak_cmd_register_msg_handle(ANYKA_IPC_PORT, &get);

	/* register usage message and callback */
	help.cmd = "vi_usage";
	help.msg_cb = vi_usage;
	ak_cmd_register_msg_handle(ANYKA_IPC_PORT, &help);

	/* register version message and callback */
	help.cmd = "vi_version";
	help.msg_cb = vi_get_sysipc_version;
	ak_cmd_register_msg_handle(ANYKA_IPC_PORT, &help);

	ak_cmd_register_module(ANYKA_IPC_PORT, "vi");
}

/* 
 * vi_sys_ipc_unregister - unregister vi module 
 * 						 system inter process communication function
 */
void vi_sys_ipc_unregister(void)
{
	if (!vi_ipcsrv.registed_flag)
		return;

	/* unregister all message */
	ak_cmd_unregister_msg_handle(ANYKA_IPC_PORT,"vi_set_status");
	ak_cmd_unregister_msg_handle(ANYKA_IPC_PORT,"vi_get_status");
	ak_cmd_unregister_msg_handle(ANYKA_IPC_PORT, "vi_usage");

	/* unregister module from server */
	ak_cmd_unregister_module(ANYKA_IPC_PORT, "vi");

	int i;
	/* release all channel's resource */
	for (i = 0; i < VIDEO_DEV_NUM; i++) {
		if (vi_ipcsrv.vi_chn[i]) {
			free(vi_ipcsrv.vi_chn[i]);
			vi_ipcsrv.vi_chn[i] = NULL;
		}
	}

	vi_ipcsrv.registed_flag = 0;
}

/* 
 * vi_sysipc_bind_dev_handle - bind vi handle with device number
 * handle[IN]: vi handle, return by ak_vi_open();
 * dev_no[IN]: device number, same as the argument delivery to ak_vi_open();
 */
void vi_sysipc_bind_dev_handle(void *handle, int dev_no)
{
	struct vi_chn_t *dev_handle = vi_ipcsrv.vi_chn[dev_no];
	if (!dev_handle) {
		ak_print_warning_ex("video input device %d is not working\n", dev_no);
		return ;
	}
	/* store ak_vi encode handle to this module */
	dev_handle->vi_handle = handle;
}

/* 
 * vi_sysipc_unbind_dev_handle - unbind vi handle 
 * dev_no[IN]: device number, same as the argument delivery to ak_vi_open();
 */
void vi_sysipc_unbind_dev_handle(int dev_no)
{
	struct vi_chn_t *handle = vi_ipcsrv.vi_chn[dev_no];
	if (!handle) {
		ak_print_warning_ex("video input device %d is not working\n", dev_no);
		return ;
	}
	/* clear ak_vi encode handle from this module */
	handle->vi_handle = NULL;
}
