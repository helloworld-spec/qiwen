#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>

#include "ak_common.h"
#include "ak_ipc_srv.h"
#include "ak_vi.h"
#include "ak_misc.h"
#include "ak_its.h"
#include "ak_ats.h"
#include "ak_config.h"

#define FILE_PATH_LEN	(200)	//max file name length

struct ak_misc_ipc_t {
	char file_name[FILE_PATH_LEN];	//include absolutely path
	int its;
	int ircut;
	int photosensitive;	//photosensitive
	int update_end;
    int ats;
};

static void misc_parse_ipc_args(char *buf, unsigned int len,
							struct ak_misc_ipc_t *ipc_t)
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
		{"its",  1, NULL, 'i'},				//isp test tools
		{"photosensitive", 1, NULL, 'p'},	//photosensitive
		{"ircut",  1, NULL, 'r'},			//isp test tools
		{"tips", 1, NULL, 't'},				//play tips with file
		{"updateend", 1, NULL, 'u'},		//update end	
		{"ats", 1, NULL, 'a'},		        //audio tools
		{0, 0, 0, 0}						//end
	};

	optind = 0;
	while ((opt = getopt_long(argc, argv, ":iprtu", longopts, NULL)) != -1) {
		switch (opt) {
			case ':':
				break;
			case 't':	//tips, store file name
				if (optarg)
					strncpy(ipc_t->file_name, optarg, FILE_PATH_LEN - 1);
				break;
			case 'i':	//its
				if (optarg)
					ipc_t->its = atoi(optarg);
				break;
			case 'r':	//ircut
				if (optarg)
					ipc_t->ircut = atoi(optarg);
				break;
			case 'p':	//photosensitive
				if (optarg)
					ipc_t->photosensitive = atoi(optarg);
				break;
			case 'u':	//update end notify
				if (optarg)
					ipc_t->update_end = atoi(optarg);
				break;
            case 'a':	//ats
				if (optarg)
					ipc_t->ats = atoi(optarg);
				break;
			default:
				break;
		}
	}
	free(cmd);
}

static void misc_cb_play_tips(char *file_name, int sock)
{
	if (strlen(file_name) > 0) {
		char res[200] = {0};
		snprintf(res, 200, "play tips, file: %s\n", file_name);
		struct audio_param file_param = {
			AK_AUDIO_TYPE_MP3, 8000, 16, 1};
		ak_misc_add_voice_tips((const char *)file_name, &file_param);
		if (sock)
			ak_cmd_result_response(res, strlen(res), sock);
	}
}

static void misc_cb_set_its(int its, int sock)
{
	if (its != -1) {
		char res[100] = {0};
		snprintf(res, 100, "turn %s isp test tool\n", its == 1 ? "on" : "off");
		if (its)
			ak_its_start();
		else
			ak_its_stop();

		if (sock)
			ak_cmd_result_response(res, strlen(res), sock);
	}
}

static void misc_cb_set_ats(int ats, int sock)
{
	if (ats != -1) {
		char res[100] = {0};
		snprintf(res, 100, "turn %s ats test tool\n", ats == 1 ? "on" : "off");
		if (ats)
			ak_ats_start(8012);
		else
			ak_ats_stop();

		if (sock)
			ak_cmd_result_response(res, strlen(res), sock);
	}
}

static void misc_cb_set_ircut(int ircut, int sock)
{
	if (ircut != -1) {
		char res[100] = {0};
		snprintf(res, 100, "switch ircut, %s\n", ircut ? "day": "night");
		void *vi_handle = ak_vi_get_handle(VIDEO_DEV0);

		struct camera_disp_config *camera = ak_config_get_camera_info();
		ak_misc_set_video_day_night(vi_handle, ircut, camera->day_ctrl);

		if (sock)
			ak_cmd_result_response(res, strlen(res), sock);
	}
}

static void misc_cb_set_photosensitive(int photosensitive, int sock)
{
	if (photosensitive != -1) {
		char res[100] = {0};
		snprintf(res, 100, "turn %s photosensitive\n", 
				photosensitive == 1 ? "on" : "off");

		ak_misc_switch_photosensitive_ircut(photosensitive);

		if (sock)
			ak_cmd_result_response(res, strlen(res), sock);
	}
}

static int cb_misc_set_status(char *status, unsigned int len, int sock)
{
	struct ak_misc_ipc_t ipc = {{0}, -1, -1, -1, 0};
	misc_parse_ipc_args(status, len, &ipc);

	/* tips */
	misc_cb_play_tips(ipc.file_name, sock);
	/* its */
	misc_cb_set_its(ipc.its, sock);
    /* ats */
	misc_cb_set_ats(ipc.ats, sock);
	/* set ircut */
	misc_cb_set_ircut(ipc.ircut, sock);
	/* set photosensitive */
	misc_cb_set_photosensitive(ipc.photosensitive, sock);

	return AK_TRUE;
}

static int cb_misc_get_status(char *status, unsigned int len, int sock)
{
	char res[50] = {0};
	snprintf(res, 50, "nothing to be get on this module now\n");
	if (sock)
		ak_cmd_result_response(res, strlen(res), sock);
	return AK_SUCCESS;
}

static int cb_misc_get_sysipc_version(char *status, unsigned int len, int sock)
{
	char res[50] = {0};
	snprintf(res, 50, "%s\n", ak_misc_get_version());
	if (sock)
		ak_cmd_result_response(res, strlen(res), sock);

	return AK_SUCCESS;
}

static int cb_misc_usage(char *status, unsigned int len, int sock)
{
	char res[1024] = {0};
	snprintf(res, 1024, "module: misc\n"
			"\t[--tips FILE]            play voice tips with specifically file\n"
			"\t[--ircut 0/1]            ircut day/night\n"
			"\t[--its EN]               isp test tool connect enable/disable\n"
			"\t[--update 1]             notify onvif update finish\n"
			"\t[--photosensitive 0/1]   photosensitive on/off\n"			
            "\t[--ats EN]               audio tool connect enable/disable\n\n"
	        "\te.g: --ircut 1\n"
	        "\te.g: --tips camera_start.mp3\n");
	if (sock)
		ak_cmd_result_response(res, strlen(res), sock);
	return AK_SUCCESS;
}

void ak_misc_sys_ipc_register(void)
{
	/* use for build up IPC system, use for debug */
	struct ak_ipc_msg_t set, get, help, version;

	set.cmd = "misc_set_status";
	set.msg_cb = cb_misc_set_status;
	ak_cmd_register_msg_handle(ANYKA_IPC_PORT, &set);

	get.cmd = "misc_get_status";
	get.msg_cb = cb_misc_get_status;
	ak_cmd_register_msg_handle(ANYKA_IPC_PORT, &get);

	help.cmd = "misc_usage";
	help.msg_cb = cb_misc_usage;
	ak_cmd_register_msg_handle(ANYKA_IPC_PORT, &help);

	version.cmd = "misc_version";
	version.msg_cb = cb_misc_get_sysipc_version;
	ak_cmd_register_msg_handle(ANYKA_IPC_PORT, &version);

	ak_cmd_register_module(ANYKA_IPC_PORT, "misc");
	ak_print_normal_ex("misc register ipcserver done\n");
}

void ak_misc_sys_ipc_unregister(void)
{
	ak_print_info_ex("entry ...\n");
	ak_cmd_unregister_msg_handle(ANYKA_IPC_PORT,"misc_set_status");
	ak_cmd_unregister_msg_handle(ANYKA_IPC_PORT,"misc_usage");
	ak_cmd_unregister_msg_handle(ANYKA_IPC_PORT,"misc_version");
	ak_print_normal_ex("misc unregister ipcserver done\n");

	ak_cmd_unregister_module(ANYKA_IPC_PORT, "misc");
	ak_print_info_ex("leave ...\n");
}
