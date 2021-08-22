#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <getopt.h>
#include <arpa/inet.h>

#include "ak_common.h"
#include "ak_ipc_srv.h"

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#define NONOPTION(x) (argv[(x)][0] != '-' || argv[(x)][1] == '\0')

#define GET_OPT         (0x01)	//for get option
#define SET_OPT         (0x02)	//for set option
#define MOD_OPT         (0x04)	//for specific module
#define VER_OPT			(0x08) 	//for get version

static const char *ccli_version = "ccli_version: V1.0.0";
static char ipc_mod[10] = {0};

static void get_support_module(char *module, int size)
{
	char cmd[50] = {0}, result[1024] = {0};
	sprintf(cmd, "get all support modules");
	int len = strlen(cmd) + 1;
	ak_cmd_send(ANYKA_IPC_PORT, cmd, len, result, 1024, NULL);

	strncpy(module, result, size);
}

static void get_all_mods_version(void)
{
	char all[1000] = {0}, cur[10] = {0}, cmd[256] = {0}, res[2048] = {0};
	char *p = NULL, *start = all;
	int ret = 0;

	get_support_module(all, 1000);
	while ((p = strchr(start, '['))) {
		if ((ret = sscanf(p, "[%s", cur)) == 1) {
			char *tail = strchr(cur, ']');
			if (tail)
				*tail = '\0';	//drop last ']'
			
			sprintf(cmd, "%s_version", cur);
			int len = strlen(cmd) + 1;
			ak_cmd_send(ANYKA_IPC_PORT, cmd, len, res, 2048, NULL);
			printf("%s", res);
			bzero(cmd, 256);
			bzero(res, 2048);
			start = p+1;
		}
	}
}

/*
 * usage: show this program usage
 */
static void usage(void)
{
	char result[1024] = {0};
	char usage_buf[2048] = {0};

	get_support_module(result, 1024);
	sprintf(usage_buf, "%s\nUsage: %s [module] [-g] [-v] [--opt val]\n"
			"\tjust specific [module] can get module's usage\n"
			"\t[-g] to get information of module\n"
			"\t[-v] to get module's version, "
			"not select module cat get all module's version\n"
			"\t[--opt val] to set option for module\n\n"
			"\tnow supported module:\n%s", 
			ccli_version, program_invocation_name, result);
	puts(usage_buf);

	exit(EXIT_SUCCESS);
}

static int check_modules(const char *module)
{
	if (!module) {
		return -1;
	} 

	char all[1000] = {0};
	get_support_module(all, 1000);

	char cur[10] = {0};
	char *p = NULL, *start = all;
	int ret = 0;

	while ((p = strchr(start, '['))) {
		if ((ret = sscanf(p, "[%s", cur)) == 1) {
			char *tail = strchr(cur, ']');
			if (tail)
				*tail = '\0';	//drop last ']'

			if (!strcmp(module, cur)) {
				//strncpy(ipc_mod, cur, 9);
				return 0;
			}
			start = p+1;
		}
	}
	return -1;
}

/*
 * get_others_args - constructure commond
 * argc[IN]: arguments counter
 * argv[IN]: arguments array
 * others[OUT]: store result
 * return: none
 */
static void get_others_args(int argc, char **argv, char *others)
{
	int i;
	for (i = 0; i < argc; i++) {
		if (!strcmp(argv[i], ipc_mod))
			continue;
		sprintf(others, "%s#%s", others, argv[i]);
	}
}

static void command_structure_and_send(char **argv, int option)
{
	if (argv == NULL) {
		printf("less argv\n");	
		return;
	}

	/* notify module to start or stop save data */
	char cmd[255] = {0};
	char others_arg[1000] = {0};
	char res[2048] = {0};
	int argc = 0;	//skip argv[0], in this function argv[0] is module

	while (argv[argc])
		argc++;

	get_others_args(argc, &argv[0], others_arg);

	/* construct command and send out */
	if (option & GET_OPT)					//function: get 
		sprintf(cmd, "%s_get_status%s", ipc_mod, others_arg);
	else if (option & SET_OPT)				//function: set 
		sprintf(cmd, "%s_set_status%s", ipc_mod, others_arg);
	else if (option & VER_OPT)				//get version
		sprintf(cmd, "%s_version", ipc_mod);
	else if (option & MOD_OPT) 				//get help
		sprintf(cmd, "%s_usage", ipc_mod);
	else {
		printf("Unknow option, please get modules usage\n");
		return;
	}

	int len = strlen(cmd) + 1;
	//printf("cmd_send: %s, len: %d\n", cmd, len);
	ak_cmd_send(ANYKA_IPC_PORT, cmd, len, res, 2048, NULL);
	printf("%s", res);
}

int main(int argc, char **argv)
{
	if (!argv[1])
		usage();
	/* 
	 * In first case, argv[1] is -v, get all module's version.
	 * On others case, argv[1] must be module name, otherwise will not 
	 * communicate to any modules
	 */
	if ((!strcmp(argv[1], "-v")) || (!strcmp(argv[1], "--version"))) {
		get_all_mods_version();
		return 0;
	}

	/* store module name when check ok */
	if (!check_modules(argv[1]))
		strncpy(ipc_mod, argv[1], 9);

	int get_opt = 0;
	int version = 0;
	int i = 1;	/* skip argv[0] of the programe name */

	/* 
	 * here we don't use getopt(), because it will permute the argv 
	 * so we cann't get orignal argv sequence
	 */
	while (i < argc) {
		if (NONOPTION(i)) {
			i++;
			continue;
		}
		if (argv[i][1] == 'g') {
			get_opt = 1; 	/* match -g */
			break;
		}
		if (argv[i][1] == 'v') {
			version = 1;    /* match -v */
			break;
		}
		if (argv[i][1] == 'h') {
			usage();       /* match -h */
			break;
		}
		i++;
	}

	int option = 0;
	/* get and set cann't send at same times */
	if (get_opt) {			//get info
		option |= GET_OPT;
	} else if (version) {
		option |= VER_OPT;	//to get module's version
	} else if (ipc_mod[0]) {
		option |= MOD_OPT;	//set module name
		if (argc > 2)
			option |= SET_OPT;	//set option for module
	} else
		usage();			//get usage and supported module

	/* send to speciffic module */
	command_structure_and_send(&argv[1], option);

	return 0;
}
