#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "ak_thread.h"
#include "ak_common.h"

#define CMD_SERVERD_TCP_PORT (8782)	//server's tcp port
#define CMD_SERVERD_MAX_WAITNO (100)	//max wait queue number

/* cmd type define */
enum cmd_flag {
	CMD_F_RESERVE = 0,
	CMD_F_NEED_RESULT,	//need result
	CMD_F_JUST_EXEC,	//just execute
};

/* cmd head define */
typedef struct cmd_header_t {
	unsigned int seq_no;	//cmd sequence number
	int flag;				//cmd flag, value assignment from enum cmd_flag
}cmd_header;

/********************** Function *************************/
/*
* @daemon_init, make current process to be a daemon process
* @return: 0 on success, -1 on faield
*/
static int daemon_init(void)
{
	int a, max_fd, i;

	/** 1 **/
	signal(SIGHUP, SIG_IGN);
	signal(SIGCHLD, SIG_IGN);

	/*** 2 ***/
	a = fork();
	if (a > 0)
		exit(EXIT_SUCCESS);
	/*** 3 ***/
	setsid();
	/*** 4 ***/
	a = fork();
	if (a > 0)
		exit(EXIT_SUCCESS);
	/*** 5 ***/
	setpgrp();
	/*** 6 ***/
	max_fd = sysconf(_SC_OPEN_MAX);
	for (i = 3; i < max_fd; i++)
		close(i);
	/*** 7 ***/
	umask(0);
	/*** 8 ***/
	chdir("/");

	return 0;
}

/*
 * cmd_exec - execute cmd
 * cmd[IN]: cmd will be execute
 * result[OUT]: store execute result if its not null
 * len[IN]: indicate result buf length
 * return: 0 on success, -1 failed
 */
static int cmd_exec(char *cmd, char *result, int len)
{
	char buf[1024];
	FILE *filp;

	filp = popen(cmd, "r");
	if (!filp) {
		ak_print_error_ex("popen %s, cmd: %s\n", strerror(errno), cmd);
		return -1;
	}
	memset(buf, '\0', sizeof(buf));
	fread(buf, sizeof(char), sizeof(buf) - 1, filp);
	pclose(filp);

	return snprintf(result, len, "%s", buf);
}

/*
 * cmd_parse_need_result - test whether this flag set need result flag
 * flag[IN]: flag need to be test
 * return: 1 is set, 0 is not
 */
static int cmd_parse_need_result(int flag)
{
	return (flag & CMD_F_NEED_RESULT);
}

/*
 * do_cmd_parse_and_exec - parse and execute cmd
 * sockfd[IN]: socket use to receive data
 */
static void do_cmd_parse_and_exec(int sockfd)
{
	char cmdbuf[1000] = {0};
	char recvbuf[1024] = {0};
	char resultbuf[1024] = {0};
	int recvsz = 0, ret;

	/* recv data */
	recvsz = recv(sockfd, recvbuf, sizeof(recvbuf) - 1, 0);
	if (recvsz <= 0) {
		ak_print_error_ex("recv error\n");
		return ;
	}

	/* get cmd_header */
	cmd_header header;
	int header_len = sizeof(cmd_header);
	memcpy(&header, recvbuf, header_len);
	ak_print_info_ex("header: seq=%u, flag=%d, recvsz=%d, header_len=%d\n",
			header.seq_no, header.flag, recvsz, header_len);

	/* get cmd */
	memcpy(cmdbuf, recvbuf + header_len, (recvsz - header_len));

	/* need result, use popen(). otherwise use system() */
	if (cmd_parse_need_result(header.flag)) {
		ret = cmd_exec(cmdbuf, resultbuf, sizeof(resultbuf) - 1);
		if (ret > 0) {
			/* execute success and send result to client */
			ak_print_info_ex("send result, len: %d\n", ret);
			send(sockfd, resultbuf, ret, 0);
		}
	} else { /* else means just exec */
		system(cmdbuf);
	}
}

/*
 * process_signal - signal handle callback
 */
static void process_signal(unsigned int sig, siginfo_t *si, void *ptr)
{
	ak_backtrace(sig, si, ptr);
	if(sig == SIGINT || sig == SIGSEGV) {

	}

	if(sig == SIGTERM) {
	}
	exit(1);
}

/*
 * register_signal - register signal handle callback
 */
static int register_signal(void)
{
	struct sigaction s;

	s.sa_flags = SA_SIGINFO;
	s.sa_sigaction = (void *)process_signal;

	sigaction(SIGSEGV, &s, NULL);
	sigaction(SIGINT, &s, NULL);
	sigaction(SIGTERM, &s, NULL);
	signal(SIGPIPE, SIG_IGN);

	return 0;
}

/*
 * main program has became a daemon program before daemon_init operate
 * 'cmd_serverd' means commond server daemon process
 */
int main (int argc, char **argv)
{
	/* make current process to be a daemon process */
	daemon_init();

	register_signal();

	ak_print_normal("\n***************************************\n");
	ak_print_normal("******** cmd server has running! ******\n");
	ak_print_normal("***************************************\n");

	int server_sockfd = 0, client_sockfd;
	int sinsize;
	struct sockaddr_in server_address, client_address;

	/* create socket server */
	server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(server_sockfd == -1) {
		ak_print_error_ex("fail to create TCP socket.\n");
		exit(EXIT_FAILURE);
	}
	ak_print_normal_ex("Success to create TCP socket.\n");

	/* set reuse flag and close-on-exec flag */
	sinsize = 1;
	if (setsockopt(server_sockfd, SOL_SOCKET, SO_REUSEADDR,
				&sinsize, sizeof(int)) != 0) {
		ak_print_error_ex("setsockopt, %s\n", strerror(errno));
		goto exit;
	}
	if (fcntl(server_sockfd, F_SETFD, FD_CLOEXEC) == -1) {
		ak_print_error_ex("fcntl: %s\n", strerror(errno));
		goto exit;
	}

	/* do bind */
	memset(&server_address, 0, sizeof(struct sockaddr_in));
	memset(&client_address, 0, sizeof(struct sockaddr_in));

	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(CMD_SERVERD_TCP_PORT);
	server_address.sin_addr.s_addr = inet_addr("127.0.0.1");

	/*** bind ***/
	if (bind(server_sockfd, (struct sockaddr *)&server_address,
				sizeof(struct sockaddr)) == -1) {
		ak_print_error_ex("bind: %s\n", strerror(errno));
		goto exit;
	}

	/**** listen ****/
	if(listen(server_sockfd, CMD_SERVERD_MAX_WAITNO) == -1) {
		ak_print_error_ex("listen: %s\n", strerror(errno));
		goto exit;
	}
	ak_print_normal_ex("Waiting for connect......\n");

	while (1) {
		/**** accept ****/
		sinsize = sizeof(struct sockaddr);
		client_sockfd = accept(server_sockfd,
				(struct sockaddr *)&client_address, (socklen_t *)&sinsize);

		pid_t pid = fork();

		/* child */
		if (pid == 0) {
			do_cmd_parse_and_exec(client_sockfd);
			close(client_sockfd);
			exit(EXIT_SUCCESS);
		} else if (pid > 0) { /* parent */
			close(client_sockfd);
		} else {
			ak_print_error_ex("Error, fork: %s\n", strerror(errno));
			ak_sleep_ms(100);
			continue;
		}
	}

exit:
	if (server_sockfd > 0)
		close(server_sockfd);

	return 0;
}
