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
#include "osd_draw.h"
#include "ak_osd.h"

/* use for Inter-Process Communication */
struct osd_ipc_t {	
	int chn;		//channel number
	int area;		//osd area number
	int disp_font_size;		//osd display font size			
};

static void osd_parse_ipc_args(char *buf, unsigned int len,
		struct osd_ipc_t *ipc_t)
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
		{"chn",  1, NULL, 'c'},	//channel number
		{"area",  1, NULL, 'a'},	//osd area number
		{"font_size",  1, NULL, 's'},	//osd display size			
		{0, 0, 0, 0}	//end
	};

	optind = 0;
	while ((opt = getopt_long(argc, argv, ":cas", longopts, NULL)) != -1) {
		switch (opt) {
			case ':':
				break;			
			case 'c':	//channel
				if (optarg)
					ipc_t->chn = atoi(optarg);
				break;
			case 'a':
				if (optarg)
					ipc_t->area = atoi(optarg);
				break;
			case 's':
				if (optarg)
					ipc_t->disp_font_size = atoi(optarg);
				break;				
			default:
				break;
		}
	}
	free(cmd);
}

/*
 * osd_get_status - get video encode module's status
 * status[OUT]: pointer to buffer to store status
 * len[IN]: indicate buffer's length
 * sock[IN]: use for response in need
 * return: 0 on success, -1 failed
 */
static int osd_get_status(char *status, unsigned int len, int sock)
{
	char res[1024] = {0};
	struct osd_ipc_t ipc_t = {0};

	ipc_t.chn = -1;
	/* parse arguments */
	osd_parse_ipc_args(status, len, &ipc_t);
	snprintf(res, 1024, "chn:%d area:%d \n", ipc_t.chn, ipc_t.area);
	
	/* get osd value */	
	if (-1 == ipc_t.chn){
		int alpha = 0;
		int front_color = 0;
		int ground_color = 0;
		osd_get_common_value(&alpha, &front_color, &ground_color);
		snprintf(res, 1024 - strlen(res), 
				"%salpha:%d\nfont_color:%d\nground_color:%d\n", 
				res, alpha, front_color, ground_color);
	} else {
		int xstart = 0;
		int ystart = 0; 
		int width = 0;
		int height = 0;
		int disp_font_size = 0;
		osd_get_area_value(ipc_t.chn, ipc_t.area, 
				&xstart, &ystart, &width, &height, &disp_font_size);
		snprintf(res, 1024 - strlen(res), 
				"%sxstart:%d\nystart:%d\nwidth:%d\nheight:%d\ndisp_font_size:%d\n", 
			res, xstart, ystart, width, height, disp_font_size);
	}
	if (sock)
		ak_cmd_result_response(res, strlen(res), sock);

	return AK_SUCCESS;
}

/*
 * osd_set_status - set video encode module's status
 * status[IN]: pointer to buffer to indicate which scope
 *             will be set
 * len[IN]: indicate buffer's length
 * sock[IN]: use for response in need
 * return: 0 on success, -1 failed
 */
static int osd_set_status(char *status, unsigned int len, int sock)
{
	char res[1024] = {0};
	struct osd_ipc_t ipc_t = {0};
	/* parse arguments */
	osd_parse_ipc_args(status, len, &ipc_t);

	snprintf(res, 1024, "chn: %d\narea: %d\nfont_size: %d\n", 
		ipc_t.chn, ipc_t.area, ipc_t.disp_font_size);
	if (ipc_t.disp_font_size > 0) {
		ak_osd_set_font_size(ipc_t.chn, ipc_t.disp_font_size);
	}
	if (sock)
		ak_cmd_result_response(res, strlen(res), sock);

	return AK_SUCCESS;
}

/*
 * osd_usage - show ipc_srv usage
 * status[OUT]: pointer to buffer use for store result
 * len[IN]: indicate buffer's length
 * sock[IN]: use for response in need
 * return: 0 on success, -1 failed
 */
static int osd_usage(char *status, unsigned int len, int sock)
{
	char res[1024] = {0};
	snprintf(res, 1024, "module osd\n"
			"\t[--chn CHN]        indicate osd channel\n"
			"\t[--area AREA]      indicate display area\n"
			"\t[--font_size]      indicate font size\n");
	if (sock)
		ak_cmd_result_response(res, strlen(res), sock);
	return AK_SUCCESS;
}

/*
 * osd_get_sysipc_version - get osd version
 * status[OUT]: pointer to buffer use for store result
 * len[IN]: indicate buffer's length
 * sock[IN]: use for response in need
 * return: 0 on success, -1 failed
 */
static int osd_get_sysipc_version(char *status, unsigned int len, int sock)
{
	char res[50] = {0};
	snprintf(res, 50, "%s\n", ak_osd_get_version());
	if (sock)
		ak_cmd_result_response(res, strlen(res), sock);
	return AK_SUCCESS;
}

void osd_sys_ipc_register(void)
{
	/* use for build up IPC system, use for debug */
	struct ak_ipc_msg_t set, get, help, version;

	set.cmd = "osd_set_status";
	set.msg_cb = osd_set_status;
	ak_cmd_register_msg_handle(ANYKA_IPC_PORT, &set);

	get.cmd = "osd_get_status";
	get.msg_cb = osd_get_status;
	ak_cmd_register_msg_handle(ANYKA_IPC_PORT, &get);

	help.cmd = "osd_usage";
	help.msg_cb = osd_usage;
	ak_cmd_register_msg_handle(ANYKA_IPC_PORT, &help);

	version.cmd = "osd_version";
	version.msg_cb = osd_get_sysipc_version;
	ak_cmd_register_msg_handle(ANYKA_IPC_PORT, &version);

	ak_cmd_register_module(ANYKA_IPC_PORT, "osd");
}

void osd_sys_ipc_unregister(void)
{
	ak_cmd_unregister_msg_handle(ANYKA_IPC_PORT,"osd_set_status");
	ak_cmd_unregister_msg_handle(ANYKA_IPC_PORT,"osd_get_status");	
	ak_cmd_unregister_msg_handle(ANYKA_IPC_PORT,"osd_usage");
	ak_cmd_unregister_msg_handle(ANYKA_IPC_PORT,"osd_version");

	ak_cmd_unregister_module(ANYKA_IPC_PORT, "osd");
}

