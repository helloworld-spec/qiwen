/* system heads */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/* project heads */
#include <ak_common.h>
#include <ak_thread.h>

/* local communication fifo path */
#define FIFO_LOCATION "/tmp/daemon_fifo"

/* mini thread, 16k is minimal stack size */
#define MONITOR_THREAD_STACK_SZ (16*1024)

static int monitor_runflag = 1;

/*
 * monitor thread
 * read fifo data and check whether anyka_ipc is running
 */
static void *monitor(void *arg)
{
	char i = 0;
	char *msg = "being alives";
	char msg_buf[20];
	int snd_fd, wt_ret;
	long int tid = ak_thread_get_tid();
	
	ak_thread_set_name("monitor");
	ak_print_normal_ex("this thread id: %ld\n", tid);
	/* detach thread, and ignore pipe signal */
	pthread_detach(pthread_self());
	signal(SIGPIPE, SIG_IGN);   //register signal handler

	/********** make fifo **********/
	if(access(FIFO_LOCATION, F_OK) < 0) //check the fifo
		mkfifo(FIFO_LOCATION, 0777);    //not exist, create it
	else
		ak_print_info_ex("the daemon fifo has created\n");

	/* open the fifo and set fd's close-on-exec flag */
	snd_fd = open(FIFO_LOCATION, O_WRONLY);
	fcntl(snd_fd, F_SETFD, FD_CLOEXEC);

	/** send heart beat package loop **/
	while (monitor_runflag) {
        /* write specific val */
		sprintf(msg_buf, "%s%d", msg, i++);
		wt_ret = write(snd_fd, msg_buf, strlen(msg_buf));
		if (wt_ret < 0) {
			/* error handler */
			ak_print_warning_ex("write data failed,%s\n", strerror(errno));
			break;
		}
		/* value reset */
		if(i > 128)
			i = 0;
       /* make a interval */
		sleep(2);
		//ak_print_normal_ex("testting: %s\n", msg_buf);
	}
	close(snd_fd);  /* close the fifo when exit */
	ak_print_normal_ex("this thread id: %ld exit\n", tid);
	ak_print_notice_ex("exit anyka_ipc\n");
	exit(EXIT_SUCCESS);

	return NULL;
}
/* 
 * start monitor
 * create thread to run as monitor
 */
void start_monitor_th(void)
{
	ak_print_normal_ex("start monitor thread\n");
	ak_pthread_t monitor_tid;
	ak_thread_create(&monitor_tid, monitor, NULL, MONITOR_THREAD_STACK_SZ, -1);
	ak_print_normal_ex("start monitor thread ok\n");
}
