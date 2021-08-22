#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>

#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/vfs.h>

#include <linux/netlink.h>
#include <linux/version.h>
#include <linux/input.h>

#include "ak_thread.h"
#include "ak_common.h"

#define CMD_SERVERD_TCP_PORT (8782)
#define CMD_MAX_BUF_LEN		(1024)

/* cmd type define */
enum cmd_flag {
	CMD_F_RESERVE = 0,
	CMD_F_NEED_RESULT,	//need resutl
	CMD_F_JUST_EXEC,	//just exec
};

/* cmd head define */
typedef struct cmd_header_t {
	unsigned int seq_no;	//cmd sequence number
	int flag;				//cmd flag, assignment from enum cmd_flag
}cmd_header;

static unsigned int global_seq_no = 0;

/********************** Function *************************/

/*
 * ak_cmd_exec - execute command, if result is not NULL, store
 * executed result to it.
 * cmd[IN], command which you want to execute
 * result[OUT], store result
 * res_len[OUT], indicate the lenght of result buf
 * return: on success 0, -1 fail
 */
int ak_cmd_exec(const char *cmd, char *result, int res_len)
{
	int ret_val = AK_FAILED;
	int client_sockfd = 0;
	int sinsize = sizeof(struct sockaddr_in);
	struct sockaddr_in server_address;

	if (!cmd) {
		ak_print_error_ex("invalid cmd\n");
		return ret_val;
	}

	/* create socket server */
	client_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (client_sockfd == -1) {
		ak_print_error_ex("fail to create TCP socket.\n");
		return ret_val;
	}
	ak_print_info_ex("cmd: %s\n", cmd);

	/* do connect */
	memset(&server_address, 0, sizeof(struct sockaddr_in));
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(CMD_SERVERD_TCP_PORT);
	server_address.sin_addr.s_addr = inet_addr("127.0.0.1");

	int ret = connect(client_sockfd, (struct sockaddr *)&server_address,
			(socklen_t)sinsize);
	if (ret) {
		ak_print_error_ex("connect: %s\n", strerror(errno));
		goto exit;
	}

	/* init cmd header, seq_no just for debug */
	cmd_header header;
	header.seq_no = global_seq_no++;
	/* need result and just exec */
	if (result && res_len > 0)
		header.flag = CMD_F_NEED_RESULT;
	else
		header.flag = CMD_F_JUST_EXEC;

	/* construct cmd */
	int header_len = sizeof(cmd_header);
	int cmd_len = strlen(cmd);
	int cmdbuf_len = header_len + cmd_len + 1; /* plus 1 store '\0' */

	if (cmdbuf_len > CMD_MAX_BUF_LEN) {
		ak_print_warning_ex("cmd len is greate than %d\n",
				CMD_MAX_BUF_LEN - 1 - header_len);
		cmdbuf_len = CMD_MAX_BUF_LEN;
		cmd_len = cmdbuf_len - header_len - 1;
	}
	char cmdbuf[CMD_MAX_BUF_LEN] = {0};

	/* set cmd header */
	memcpy(cmdbuf, &header, header_len);
	/* set cmd context */
	memcpy(cmdbuf + header_len, cmd, cmd_len);

	/* send to cmd_serverd to execute command */
	send(client_sockfd, cmdbuf, cmdbuf_len, 0);

	if (header.flag == CMD_F_NEED_RESULT) {
		/* res_len -1 means save one bit to store '\0' */
		int recvsz = recv(client_sockfd, result, res_len - 1, 0);
		result[recvsz] = '\0';
		ak_print_info("cmd: %s\n", cmd);
		ak_print_info("result len=%d, context:\n\t%s\n", recvsz, result);
	}
	ret_val = AK_SUCCESS;
exit:
	if (client_sockfd > 0)
		close(client_sockfd);

	return ret_val;
}
