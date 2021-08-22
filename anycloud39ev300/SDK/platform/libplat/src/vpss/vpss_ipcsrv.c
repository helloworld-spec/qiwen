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
#include "ak_vpss.h"
#include "isp_cfg_file.h"

#define LOG_TAG_LEN			(20)

/* use for Inter-Process Communication */
struct vpss_ipc_t {
	int dev_no;		//device number
	int power_freq;
	int log_switch;	//switch on or off module print
	char log_tag[LOG_TAG_LEN];	//log tag buffer
};

struct vpss_ipcsrv_t {
	void *vi_handle[VIDEO_DEV_NUM];
	int registed_flag;
};

static struct vpss_ipcsrv_t vpss_ipcsrv = {{0},0};

static void vpss_parse_ipc_args(char *buf, unsigned int len,
		struct vpss_ipc_t *ipc)
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
		{"devno",  1, NULL, 'd'},	//dev number
		{"power_freq",  1, NULL, 'p'},	//power freq
		{"log_switch",  1, NULL, 'l'},	//log print switch
		{"tag_name",  1, NULL, 't'},	//log tag name
		{0, 0, 0, 0}	//end
	};

	optind = 0;
	while ((opt = getopt_long(argc, argv, ":dplt", longopts, NULL)) != -1) {
		switch (opt) {
			case ':':
				ak_print_error("unsupport option: %d\n", opt);
				break;
			case 'd':
				if (optarg)
					ipc->dev_no = atoi(optarg);
				break;
			case 'p':
				if (optarg)
					ipc->power_freq = atoi(optarg);
				break;
			case 'l':
				if (optarg)
					ipc->log_switch = atoi(optarg);
				break;
			case 't':
				if (optarg)
					strncpy(ipc->log_tag, optarg, LOG_TAG_LEN);
				break;
			default:
				break;
		}
	}
	free(cmd);
}

/*
 * vpss_get_status - get video encode module's status
 * status[OUT]: pointer to buffer to store status
 * len[IN]: indicate buffer's length
 * sock[IN]: use for response in need
 * return: 0 on success, -1 failed
 */
int vpss_get_status(char *status, unsigned int len, int sock)
{
	char res[2048] = {0};
	struct vpss_ipc_t ipc = {-1, -1};
	struct cfg_file_head cfg_head = {0};
	struct vpss_isp_ae_run_info ae_run_info = {0};
	int i = 0;
		
	/* parse arguments */
	vpss_parse_ipc_args(status, len, &ipc);

	if (ipc.dev_no == -1) {
		ipc.dev_no = VIDEO_DEV0;
	}

	/* get power freq info */
	ak_vpss_effect_get(vpss_ipcsrv.vi_handle[ipc.dev_no], VPSS_POWER_HZ, &ipc.power_freq);
	snprintf(res, 2048, "power_freq:%d\n\n", ipc.power_freq);

	/* get isp cfg file headinfo */
	snprintf(res + strlen(res), 2048 - strlen(res), "isp cfg info:\n");
	for (i=0; i<2; i++) {
		isp_cfg_file_get_headinfo(i, &cfg_head);

		snprintf(res + strlen(res), 2048 - strlen(res), 
			"subfile:%d, version:%s, sensor id:%x, style id:%d\nnotes:\n%s\n\n", 
				i, cfg_head.file_version, cfg_head.sensor_id, cfg_head.style_id, cfg_head.notes);
	}

	/* get ae run info */
	ak_vpss_isp_get_ae_run_info(vpss_ipcsrv.vi_handle[ipc.dev_no], &ae_run_info);
	snprintf(res + strlen(res), 2048 - strlen(res), "ae run info:\n");
	snprintf(res + strlen(res), 2048 - strlen(res), "a_gain:%ld, d_gain:%ld, isp_d_gain:%ld, exp_time:%ld\n\n", 
		ae_run_info.current_a_gain, ae_run_info.current_d_gain, 
		ae_run_info.current_isp_d_gain, ae_run_info.current_exp_time);

	if (sock)
		ak_cmd_result_response(res, strlen(res), sock);

	return AK_SUCCESS;
}

/*
 * vpss_set_status - set video encode module's status
 * status[IN]: pointer to buffer to indicate which scope
 *             will be set
 * len[IN]: indicate buffer's length
 * sock[IN]: use for response in need
 * return: 0 on success, -1 failed
 */
int vpss_set_status(char *status, unsigned int len, int sock)
{
	char res[1024] = {0};
	struct vpss_ipc_t ipc = {-1, -1, -1};
	/* parse arguments */
	vpss_parse_ipc_args(status, len, &ipc);

	if (ipc.dev_no == -1) {
		ipc.dev_no = VIDEO_DEV0;
	}

	if (ipc.power_freq > 0) {
		ak_vpss_effect_set(vpss_ipcsrv.vi_handle[ipc.dev_no], VPSS_POWER_HZ, ipc.power_freq);
		snprintf(res, 1024, "set power_freq:%d\n", ipc.power_freq);
	}

	if (ipc.log_switch != -1 && (strlen(ipc.log_tag) > 0)) {
		unsigned char enable = (unsigned char)ipc.log_switch;
		ak_set_module_print(ipc.log_tag, enable);
	}

	if (sock)
		ak_cmd_result_response(res, strlen(res), sock);

	return AK_SUCCESS;
}

/*
 * vpss_usage - show ipc_srv usage
 * status[OUT]: pointer to buffer use for store result
 * len[IN]: indicate buffer's length
 * sock[IN]: use for response in need
 * return: 0 on success, -1 failed
 */
int vpss_usage(char *status, unsigned int len, int sock)
{
	char res[1024] = {0};

	snprintf(res, 1024, "module: vpss\n"
		"\t[--power_freq] --> set power freq 50 or 60\n"
		"\t[-g] --> get module's active status, such as power freq/isp cfg info/ae run info\n"
		"-----------------------------------------------\n"
		"e.g:\t-g\n"
		"e.g:\t--power_freq 50\n"
		"-----------------------------------------------\n\n");

	if (sock)
		ak_cmd_result_response(res, strlen(res), sock);

	return AK_SUCCESS;
}

/*
 * vpss_get_sysipc_version - get vpss version
 * status[OUT]: pointer to buffer use for store result
 * len[IN]: indicate buffer's length
 * sock[IN]: use for response in need
 * return: 0 on success, -1 failed
 */
int vpss_get_sysipc_version(char *status, unsigned int len, int sock)
{
	char res[50] = {0};
	snprintf(res, 50, "%s\n", ak_vpss_get_version());

	if (sock)
		ak_cmd_result_response(res, strlen(res), sock);

	return AK_SUCCESS;
}

/*
 * vpss_sys_ipc_register - register vpss module
 * return: void
 */
void vpss_sys_ipc_register(void)
{
	/* use for build up IPC system, use for debug */
	struct ak_ipc_msg_t set, get, help, version;

	if (vpss_ipcsrv.registed_flag)
		return;

	vpss_ipcsrv.registed_flag = 1;

	set.cmd = "vpss_set_status";
	set.msg_cb = vpss_set_status;
	ak_cmd_register_msg_handle(ANYKA_IPC_PORT, &set);

	get.cmd = "vpss_get_status";
	get.msg_cb = vpss_get_status;
	ak_cmd_register_msg_handle(ANYKA_IPC_PORT, &get);

	help.cmd = "vpss_usage";
	help.msg_cb = vpss_usage;
	ak_cmd_register_msg_handle(ANYKA_IPC_PORT, &help);

	version.cmd = "vpss_version";
	version.msg_cb = vpss_get_sysipc_version;
	ak_cmd_register_msg_handle(ANYKA_IPC_PORT, &version);

	ak_cmd_register_module(ANYKA_IPC_PORT, "vpss");
}

/*
 * vpss_sys_ipc_unregister - unregister vpss module
 * return: void
 */
void vpss_sys_ipc_unregister(void)
{
	if (!vpss_ipcsrv.registed_flag)
		return;

	ak_cmd_unregister_msg_handle(ANYKA_IPC_PORT,"vpss_set_status");
	ak_cmd_unregister_msg_handle(ANYKA_IPC_PORT,"vpss_get_status");
	ak_cmd_unregister_msg_handle(ANYKA_IPC_PORT,"vpss_usage");
	ak_cmd_unregister_msg_handle(ANYKA_IPC_PORT,"vpss_version");

	vpss_ipcsrv.registed_flag = 0;

	ak_cmd_unregister_module(ANYKA_IPC_PORT, "vpss");
}

/*
 * vpss_sysipc_bind_dev_handle - bind vi handle
 * handle[IN]: vi handle
 * dev_no[IN]: dev no
 * return: void
 */
void vpss_sysipc_bind_dev_handle(void *handle, int dev_no)
{
	if (!handle) {
		ak_print_warning_ex("video input device %d is not working\n", dev_no);
		return ;
	}
	/* store ak_vi encode handle to this module */
	vpss_ipcsrv.vi_handle[dev_no] = handle;
}

/*
 * vpss_sysipc_unbind_dev_handle - unbind vi handle
 * dev_no[IN]: dev no
 * return: void
 */
void vpss_sysipc_unbind_dev_handle(int dev_no)
{
	/* clear ak_vi encode handle from this module */
	vpss_ipcsrv.vi_handle[dev_no] = NULL;
}
