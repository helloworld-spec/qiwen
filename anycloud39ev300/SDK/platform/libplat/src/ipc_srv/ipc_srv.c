#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "list.h"
#include "ak_common.h"
#include "ak_thread.h"
#include "ak_ipc_srv.h"

#define PRO_NAME_SZ		(16)
#define WAIT_QUEUE_MAX	(100)
#define MODULE_NAME_SZ 	(20)

static const char *ipcsrv_version = "libplat_ipcsrv V1.0.00";

/* single server structure */
struct ipc_srv_t {
	int run_flg;				//module run flag
	int server_sock;			//server socket fd
	int cmd_nr;					//current command number
	int port;					//server's tcp-port
	ak_pthread_t handle_tid;	//server thread, do accept and handle
	char pro_name[PRO_NAME_SZ];	//programe name
	struct list_head list;		//hang to global list module
	struct list_head cmd_list;	//manage program's cmd
	struct list_head mod_list;	//manage program's module list
	ak_mutex_t  	 srv_lock;	//server option mutex lock
	ak_sem_t		 sem;		//server work semaphore
};

/* all servers set structure */
struct servers_set_t {
	int server_nr;					//current server number
	struct list_head server_set;	//server set list head
	ak_mutex_t mutex;				//servers operate mutex lock
};

/* message/cmd structure */
struct srv_cmd_t {
	struct list_head list;		//list node, hang to server's cmd list
	struct ak_ipc_msg_t *msg;	//pointer to message
};

/* client handle structure */
struct client_handle_arg {
	struct ipc_srv_t *srv;	//pointer to its server
	int client_sockfd;		//client socket fd
};

/* module node, each programe will create ones */
struct module_node {
	char module_name[MODULE_NAME_SZ];	//module name in the program
	struct list_head list;				//list, hang to servers set
};

static struct servers_set_t all_servers = {0};

/* 
 * find_server_by_port - find server by port
 * port[IN]: port number, same as ak_cmd_register_module() argument 1;
 * return: pointer to struct ipc_srv_t on found, NULL on not
 */
static struct ipc_srv_t *find_server_by_port(unsigned int port)
{
	struct ipc_srv_t *pos;

	ak_thread_mutex_lock(&all_servers.mutex);
	/* travel the list */
	list_for_each_entry(pos, &all_servers.server_set, list) {
		/* compare port */
		if (pos->port == port) {
			ak_thread_mutex_unlock(&all_servers.mutex);
			return pos;
		}
	}
	ak_thread_mutex_unlock(&all_servers.mutex);

	return NULL;
}

/* 
 * alloc_srv_cmd - allocate cmd structure memory
 * return: pointer to struct srv_cmd_t on success, NULL on fail
 */
static struct srv_cmd_t *alloc_srv_cmd(void)
{
	struct srv_cmd_t *cmd = (struct srv_cmd_t *)calloc(1,
			sizeof(struct srv_cmd_t));
	if (cmd) {
		cmd->msg = (struct ak_ipc_msg_t *)calloc(1,
				sizeof(struct ak_ipc_msg_t));
		if (cmd->msg)
			return cmd;
		else {
			free(cmd);
			cmd = NULL;
		}
	} else
		cmd = NULL;
	return cmd;
}

/*
 * free_srv_cmd - free struct srv_cmd_t memory
 * cmd[IN]: pointer to struct srv_cmd_t 
 */
static void free_srv_cmd(struct srv_cmd_t *cmd)
{
	if (cmd) {
		free(cmd->msg);
		free(cmd);
		cmd = NULL;
	}
}

/* 
 * remove_all_srv_cmd - remove all cmd under server 
 * srv[IN]: pointer to server
 */
static void remove_all_srv_cmd(struct ipc_srv_t *srv)
{
	struct srv_cmd_t *pos, *n;

	ak_thread_mutex_lock(&srv->srv_lock);
	/* travel the server's cmd list */
	list_for_each_entry_safe(pos, n, &srv->cmd_list, list) {
		list_del(&pos->list);
		free_srv_cmd(pos);
		srv->cmd_nr--;
	}
	ak_thread_mutex_unlock(&srv->srv_lock);
}

/*
 * create_socket - create socket as TCP socket
 * return: new socket fd on success, -1 on failed
 */
static int create_socket(void)
{
	int sock = -1;

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1) {
		ak_print_error_ex("fail to create tcp socket.\n");
		return sock;
	}

	/* set reuse flag and close-on-exec flag */
	int sinsize = 1;
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR,
				&sinsize, sizeof(int)) != 0) {
		ak_print_error_ex("setsockopt, %s\n", strerror(errno));
		goto fail;
	}
	if (fcntl(sock, F_SETFD, FD_CLOEXEC) == -1) {
		ak_print_error_ex("fcntl: %s\n", strerror(errno));
		goto fail;
	}
	return sock;

fail:
	close(sock);
	sock = -1;
	return sock;
}

/*
 * bind_with_port - bind socket and port
 * sock[IN]: tcp socket, return by create_socket();
 * port[IN]: communicate port, not TCP port
 * return: 0 on success, -1 failed
 */
static int bind_with_port(int sock, unsigned int port)
{
	/* do bind */
	struct sockaddr_in server_address;

	memset(&server_address, 0, sizeof(struct sockaddr_in));
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(port);
	server_address.sin_addr.s_addr = inet_addr("127.0.0.1");

	/*** bind ***/
	if (bind(sock, (struct sockaddr *)&server_address,
				sizeof(struct sockaddr)) == -1) {
		ak_print_error_ex("bind: %s\n", strerror(errno));
		return AK_FAILED;
	}

	return AK_SUCCESS;
}

/*
 * create_wait_queue - create wait queue on socket
 * sock[IN]: socket fd, return by create_socket();
 * backlog[IN]: wait queue max, see 'man listen' for more detail
 * return: 0 on success, -1 failed
 */
static int create_wait_queue(int sock, int backlog)
{
	/**** listen ****/
	if (listen(sock, backlog) == -1) {
		ak_print_error_ex("listen: %s\n", strerror(errno));
		return AK_FAILED;
	}
	ak_print_normal_ex("Waiting for connect......\n");
	return AK_SUCCESS;
}

/*
 * do_connect - to do connect
 * sock[IN]: socket fd, return by create_socket();
 * port[IN]: communicate port, not TCP port
 * return: 0 on success, -1 failed
 */
static int do_connect(int sock, unsigned int port)
{
	struct sockaddr_in server_address;
	int sinsize = sizeof(struct sockaddr_in);

	/* do connect */
	memset(&server_address, 0, sizeof(struct sockaddr_in));
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(port);
	server_address.sin_addr.s_addr = inet_addr("127.0.0.1");

	int ret = connect(sock,
			(struct sockaddr *)&server_address,
			(socklen_t)sinsize);
	if (ret)
		ak_print_error_ex("connect: %s\n", strerror(errno));
	return ret;
}

/*
 * send_cmd - to send command
 * sock[IN]: socket fd, retur by create_socket();
 * cmd[IN]: pointer to command buffer
 * len[IN]: indicate buffer's len
 * return: 0 on success, -1 failed
 */
static int send_cmd(int sock,
		const char *cmd, unsigned int len)
{
	return send(sock, cmd, len, 0);
}

/*
 * get_cmd_result - get command execute result
 * sock[IN]: socket fd, retur by create_socket();
 * result[OUT]: pointer to store result's buffer
 * res_len[IN]: indicate result buffer len
 * tv_out[IN/OUT]: pointer to time_out value, NULL means block;
 * 	   			   on select succeed, 
 * 	   			   tv_out was modified to reflect the amount of time not slept.
 * return: on succeed, receive message len will return;
 *         on failed, -1 is return.
 */
static int get_cmd_result(int sock, char *result,
		unsigned int res_len, long int *tv_out)
{
	struct timeval *tv = NULL;

	/* set time-out value */
	if (tv_out && *tv_out > 0) {
		tv = (struct timeval *)calloc(1,
			sizeof(struct timeval));
		tv->tv_sec = (*tv_out) / 1000;
		tv->tv_usec = ((*tv_out) % 1000) * 1000;
	} else {
		/* block */
		tv = NULL;
	}

	fd_set read_fds;
	FD_ZERO(&read_fds);
	FD_SET(sock, &read_fds);

	int ret = select(sock + 1, &read_fds, NULL, NULL, tv);
	int recv_len = 0;

	switch (ret) {
		case 0:	/* timeout */
			ak_print_normal_ex("timeout\n");
			break;
		case 1: /* get result success */
			if (FD_ISSET(sock, &read_fds)) {
				if (tv) {
					*tv_out = tv->tv_sec * 1000;
					*tv_out += tv->tv_usec / 1000;
				}
				recv_len = recv(sock, result, res_len, 0);
			}
			break;
		default:
			ak_print_error_ex("select error\n");
			break;
	}

	/* if set time out, free memory */
	if (tv)
		free(tv);

	return recv_len;
}

/* 
 * parse_cmd - parse command, to get sub level argument which will
 *             be transmission to next level to do parse.
 * recv[IN]: pointer to receive data which will be parse
 * arg_offset[OUT]: store first argument offset, 
 *                  skip first command and delimit symbol
 * return: pointer to first command on parse succeed,
 *         NULL is returned on failed
 */
static char *parse_cmd(char *recv, int *arg_offset)
{
	char *cmd = NULL;

	cmd = strtok(recv, DELIMIT);
	if (cmd) {
		*arg_offset = strlen(cmd) + strlen(DELIMIT);
	}

	return cmd;
}

/* 
 * do_recv_and_parse - do receive and parse command, if any message matched,
 *                     call corresponding call-back function 
 * srv[IN]:	pointer to server
 * cli_sock[IN]: current client socket fd
 */
static void do_recv_and_parse(struct ipc_srv_t *srv, int cli_sock)
{
	/* receive data and parse data */
	char recvbuf[2048] = {0};
	int recvsz = 0;

	/* recv data */
	recvsz = recv(cli_sock, recvbuf, sizeof(recvbuf) - 1, 0);
	if (recvsz <= 0) {
		ak_print_error_ex("recv error, %s\n", strerror(errno));
		return;
	}
	ak_print_info_ex("recvbuf: %s\n", recvbuf);

	/* parse cmd */
	char *cmd = NULL;
	int arg_offset = 0;

	/* NOTICE: after next invocate, recvbuf will be modify */
	cmd = parse_cmd(recvbuf, &arg_offset);
	if (!cmd) {
		ak_print_error_ex("no match cmd\n");
		return;
	}

	/* traverse list, match cmd, invoking callbacks */
	struct srv_cmd_t *pos, *match_msg = NULL;

	ak_thread_mutex_lock(&srv->srv_lock);
	list_for_each_entry(pos, &srv->cmd_list, list) {
		if (!strcmp(pos->msg->cmd, cmd)) {
			match_msg = pos;
			break;
		}
	}
	ak_thread_mutex_unlock(&srv->srv_lock);

	/* if find this message was register, invoking its callback */
	if (match_msg) {
		match_msg->msg->msg_cb(recvbuf + arg_offset,
				recvsz - arg_offset, cli_sock);
	} else	/* Notice: if it is not match any message, show all modules */
		ak_cmd_show_working_modules(cli_sock);
}

/*
 * client_handle_srv_th - client handle thread
 * 						  to handle a new connection
 */
static void *client_handle_srv_th(void *arg)
{
	struct client_handle_arg *srv_arg = (struct client_handle_arg *)arg;
	ak_thread_sem_post(&(srv_arg->srv->sem));

	pthread_detach(pthread_self());
	ak_print_info_ex("tid: %ld\n", ak_thread_get_tid());

	do_recv_and_parse(srv_arg->srv, srv_arg->client_sockfd);
	close(srv_arg->client_sockfd);

	ak_print_info_ex("thread exit, %ld\n", ak_thread_get_tid());
	free(arg);
	ak_thread_exit();
	return NULL;
}

/* 
 * handle_thread - server's handler thread, to handle new connection 
 * 				   each server create on handle thread
 */
static void *handle_thread(void *arg)
{
	struct ipc_srv_t *srv = (struct ipc_srv_t *)arg;
	long int tid = ak_thread_get_tid();
	struct sockaddr_in client_address;
	int sinsize;
	int srv_sock = srv->server_sock;
	int	client_sockfd = -1;

	ak_print_normal_ex("thread id: %ld\n", tid);
	ak_thread_set_name(srv->pro_name);

	memset(&client_address, 0, sizeof(struct sockaddr_in));

	while (srv->run_flg) {
		/**** accept ****/
		sinsize = sizeof(struct sockaddr);
		client_sockfd = accept(srv_sock,
				(struct sockaddr *)&client_address,
			   	(socklen_t *)&sinsize);
		
		ak_pthread_t tid;
		struct client_handle_arg *arg = (struct client_handle_arg *)calloc(1, 
				sizeof(struct client_handle_arg));
		arg->srv = srv;
		arg->client_sockfd = client_sockfd;

		ak_thread_create(&tid, client_handle_srv_th, arg,
			   	ANYKA_THREAD_MIN_STACK_SIZE, -1);
		/* make sure child-thread run first */
		ak_thread_sem_wait(&srv->sem);
	}

	ak_print_normal_ex("exit thread: %ld\n", tid);
	ak_thread_exit();
	return NULL;
}

/*
 * create_handle_thread - create handle thread
 */
static int create_handle_thread(struct ipc_srv_t *srv)
{
	int ret = AK_FAILED;
	if (!srv) {
		ak_print_error_ex("invalid argument\n");
		return ret;
	}

	/* init new server's resource */
	srv->run_flg = AK_TRUE;
	srv->cmd_nr = 0;
	INIT_LIST_HEAD(&srv->cmd_list);
	INIT_LIST_HEAD(&srv->mod_list);
	ak_thread_mutex_init(&srv->srv_lock, NULL);
	ak_thread_sem_init(&srv->sem, 0);

	ret = ak_thread_create(&srv->handle_tid,
		   	handle_thread, srv,
			ANYKA_THREAD_MIN_STACK_SIZE, -1);

	if (ret == AK_FAILED) {
		ak_print_error_ex("thread create fail\n");
		return ret;
	}

	return ret;
}

/********************************************************/
/* 
 * ak_cmd_server_register - server register, bind port and name 
 * port[IN]: server's port number
 * name[IN]: server's name
 * return: 0 on success, -1 failed
 */
int ak_cmd_server_register(unsigned int port, char *name)
{
	int server_sockfd = 0;
	int ret = AK_FAILED;

	/* 1. create socket server */
	server_sockfd = create_socket();

	/* 2. bind socket with port */
	ret = bind_with_port(server_sockfd, port);
	if (ret == AK_FAILED) {
		close(server_sockfd);
		return ret;
	}

	/* 3. create socket wait queue */
	create_wait_queue(server_sockfd, WAIT_QUEUE_MAX);

	/* 4. init server node */
	struct ipc_srv_t *new_srv = (struct ipc_srv_t *)calloc(1,
			sizeof(struct ipc_srv_t));
	if (!new_srv) {
		ak_print_error_ex("calloc fail\n");
		close(server_sockfd);
		return AK_FAILED;
	}
	strncpy(new_srv->pro_name, name, PRO_NAME_SZ - 1);
	new_srv->server_sock = server_sockfd;
	new_srv->port = port;

	/* 5. create server's message handle thread */
	ret = create_handle_thread(new_srv);
	if (ret == AK_FAILED) {
		free(new_srv);
		close(server_sockfd);
		return ret;
	}

	/* 6. init global server's list */
	if (!all_servers.server_nr) {
		INIT_LIST_HEAD(&all_servers.server_set);
		ak_thread_mutex_init(&all_servers.mutex, NULL);
	}

	/* 7. add current server to global server set */
	ak_thread_mutex_lock(&all_servers.mutex);
	list_add_tail(&new_srv->list, &all_servers.server_set);
	all_servers.server_nr++;
	ak_thread_mutex_unlock(&all_servers.mutex);

	return ret;
}

/* 
 * ak_cmd_server_unregister - server unregister, remove server from set
 * port[IN]: server's port number
 * return: 0 on success, -1 failed
 */
int ak_cmd_server_unregister(unsigned int port)
{
	struct ipc_srv_t *pos, *n;

	if (!all_servers.server_nr) {
		ak_print_error_ex("there is no server was register\n");
		return AK_FAILED;
	}
	ak_print_info_ex("entry ...\n");

	ak_thread_mutex_lock(&all_servers.mutex);
	list_for_each_entry_safe(pos, n, &all_servers.server_set, list) {
		if (pos->port == port) {
			/* stop handle_thread */
			pos->run_flg = AK_FALSE;
			ak_thread_cancel(pos->handle_tid);
			ak_thread_join(pos->handle_tid);

			/* close socket */
			close(pos->server_sock);
			pos->server_sock = -1;
			/* remove all message resource */
			remove_all_srv_cmd(pos);

			/* remove from set list */
			list_del(&pos->list);
			ak_thread_mutex_destroy(&pos->srv_lock);
			ak_thread_sem_destroy(&pos->sem);
			free(pos);
			all_servers.server_nr--;
			break;
		}
	}
	ak_thread_mutex_unlock(&all_servers.mutex);

	/* remove global resource */
	if (!all_servers.server_nr) {
		ak_thread_mutex_destroy(&all_servers.mutex);
	}
	ak_print_info_ex("leave ...\n");

	return AK_SUCCESS;
}

/* 
 * ak_cmd_send - send commamd/message
 * port[IN]: server's port number
 * cmd[IN]: pointer to command buffer
 * cmd_len[IN]: indicate command buffer len
 * result[OUT]: store cmd execute result
 * res_len[IN]: indicate result buffer len
 * tv_out[IN/OUT]: pointer to time_out value, NULL means block;
 * 	   			   on select succeed, 
 * 	   			   tv_out was modified to reflect the amount of time not slept.
 * return: 0 on success, -1 failed
 */
int ak_cmd_send(unsigned int port,
		const char *cmd, unsigned int cmd_len,
	   	char *result, unsigned int res_len,
		long int *tv_out)
{
	int ret = AK_FAILED;
	int sock = -1;

	/* 1. */
	sock = create_socket();
	if (sock == -1) {
		ak_print_error_ex("create socket fail\n");
		return AK_FAILED;
	}

	/* 2. */
	ret = do_connect(sock, port);
	if (ret) {
		ak_print_error_ex("connect fail\n");
		close(sock);
		return AK_FAILED;
	}

	/* 3. */
	ret = send_cmd(sock, cmd, cmd_len);
	if (ret != cmd_len) {
		ak_print_error_ex("cmd_len:%d ret:%d\n", cmd_len, ret);
	}

	/* 4. */
	if (result && (res_len > 0)) {
		ret = get_cmd_result(sock, result, res_len, tv_out);
	}
	close(sock);

	return ret;
}

/* 
 * ak_cmd_register_msg_handle - register message handle
 * port[IN]: server's port number
 * msg[IN]: pointer to message structure
 * return: 0 on success, -1 failed
 */
int ak_cmd_register_msg_handle(unsigned int port,
	   	struct ak_ipc_msg_t *msg)
{
	if (!all_servers.server_nr) {
		ak_print_info_ex("no server was register, can not register message\n");
		return AK_FAILED;
	}
	struct ipc_srv_t *srv = NULL;

	/* find server */
	srv = find_server_by_port(port);
	if (!srv) {
		ak_print_error_ex("this port has no server bind\n");
		return AK_FAILED;
	}

	/* new a cmd node */
	struct srv_cmd_t *new_cmd = alloc_srv_cmd();
	if (!new_cmd) {
		ak_print_error_ex("calloc fail\n");
		return AK_FAILED;
	}
	memcpy(new_cmd->msg, msg, sizeof(struct ak_ipc_msg_t));

	/* insert new node to server's cmd list */
	ak_thread_mutex_lock(&srv->srv_lock);
	list_add_tail(&new_cmd->list, &srv->cmd_list);
	srv->cmd_nr++;
	ak_thread_mutex_unlock(&srv->srv_lock);

	return AK_SUCCESS;
}

/* 
 * ak_cmd_unregister_msg_handle - unregister message handle
 * port[IN]: server's port number
 * msg[IN]: pointer to message structure
 * return: 0 on success, -1 failed
 */
int ak_cmd_unregister_msg_handle(unsigned int port,
		char *msg)
{
	if (!all_servers.server_nr) {
		ak_print_error_ex("no server was register, can not register message\n");
		return AK_FAILED;
	}

	struct ipc_srv_t *srv = NULL;

	/* find server */
	srv = find_server_by_port(port);
	if (!srv) {
		ak_print_error_ex("this port has no server bind\n");
		return AK_FAILED;
	}

	int ret = AK_FAILED;
	struct srv_cmd_t *pos, *n;

	/* 
	 * travel the list to match cmd
	 * if there has matched node, remove it
	 */
	ak_thread_mutex_lock(&srv->srv_lock);
	list_for_each_entry_safe(pos, n, &srv->cmd_list, list) {
		if (!strcmp(pos->msg->cmd, msg)) {
			list_del(&pos->list);
			free_srv_cmd(pos);
			srv->cmd_nr--;
			ret = AK_SUCCESS;
			break;
		}
	}
	ak_thread_mutex_unlock(&srv->srv_lock);

	return ret;
}

/* 
 * ak_cmd_result_response - cmd response
 *                          send the executed result of cmd to client
 * port[IN]: server's port number
 * msg[IN]: pointer to message structure
 */
void ak_cmd_result_response(char *result, int len, int sock)
{
	send_cmd(sock, result, len);
}

/* 
 * ak_cmd_register_module - register module
 * port[IN]: server's port number
 * name[IN]: pointer to module name
 */
void ak_cmd_register_module(unsigned int port, const char *name)
{
	if (!all_servers.server_nr) {
		ak_print_error_ex("no server was register, can not register module\n");
		return ;
	}

	struct ipc_srv_t *srv = NULL;

	srv = find_server_by_port(port);
	if (!srv) {
		ak_print_error_ex("this port has no server bind\n");
		return ;
	}

	struct module_node *new_module = (struct module_node *)calloc(1,
		   	sizeof(struct module_node));
	/* init module and all to server's module list */
	strncpy(new_module->module_name, name, MODULE_NAME_SZ - 1);
	ak_thread_mutex_lock(&srv->srv_lock);
	list_add_tail(&new_module->list, &srv->mod_list);
	ak_thread_mutex_unlock(&srv->srv_lock);
}

/* 
 * ak_cmd_unregister_module - unregister module
 * port[IN]: server's port number
 * name[IN]: pointer to module name
 */
void ak_cmd_unregister_module(unsigned int port, const char *name)
{
	if (!all_servers.server_nr) {
		ak_print_error_ex("no server was register, can not unregister module\n");
		return ;
	}

	struct ipc_srv_t *srv = NULL;

	srv = find_server_by_port(port);
	if (!srv) {
		ak_print_error_ex("this port has no server bind\n");
		return ;
	}

	struct module_node *pos, *n;

	ak_thread_mutex_lock(&srv->srv_lock);
	list_for_each_entry_safe(pos, n, &srv->mod_list, list) {
		if (!strcmp(name, pos->module_name)) {
			list_del(&pos->list);
			free(pos);
			pos = NULL;
			break;	
		}
	}
	ak_thread_mutex_unlock(&srv->srv_lock);
}

/* 
 * ak_cmd_show_working_modules - show all registed module
 * cli_sock[IN]: use to send result back to client
 */
void ak_cmd_show_working_modules(int cli_sock)
{
	if (!all_servers.server_nr) {
		ak_print_error_ex("no module here\n");
		return;
	}
	
	/* traverse all server and list all server's modules */
	char mod_name[1024] = {0};
	struct ipc_srv_t *pos, *n;

	ak_thread_mutex_lock(&all_servers.mutex);
	list_for_each_entry_safe(pos, n, &all_servers.server_set, list) {
		struct module_node *mod;

		ak_thread_mutex_lock(&pos->srv_lock);
		list_for_each_entry(mod, &pos->mod_list, list)
			sprintf(mod_name, "%s\t[%s]\n", mod_name, mod->module_name);
		ak_thread_mutex_unlock(&pos->srv_lock);
	}
	ak_thread_mutex_unlock(&all_servers.mutex);

	ak_cmd_result_response(mod_name, strlen(mod_name), cli_sock);
}

/* 
 * ak_ipcsrv_get_version - get module version
 * return: pointer to version string
 */
const char *ak_ipcsrv_get_version(void)
{
	return ipcsrv_version;
}
