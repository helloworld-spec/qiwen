#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "list.h"
#include "ak_thread.h"
#include "ak_common.h"
#include "ak_ao.h"
#include "ak_adec.h"

#define FILE_NAME_LEN	(200)	//max file name length
#define BUF_LEN      	(4096)	//read file buffer length

/* voice play control struct */
struct voice_play_ctrls {
	int run_flag;	//run flag
	void *ao_handle;	//ao handle
	void *adec_handle;	//adec handle
	void *stream_handle; 		//adec stream handle

	ak_sem_t play_sem;			//play sem
	ak_pthread_t play_tid;		//play thread id
	ak_mutex_t play_mutex;		//lock
	struct list_head file_list;	//audio file queue
};

struct file_node {
	char file_name[FILE_NAME_LEN];	//include absolutely path
	unsigned int 		sample_rate;	//sample rate
	int 				channel_num;	//channel
	enum ak_audio_type 	type;			//audio type, mp3/aac and so on
	struct list_head 	list;		//hang to file_list
};

struct voice_play_ctrls voice_play_ctrl = {0};

/**
 * open_file - open file and set fd's close-on-exec flag
 * file_name[IN], file name which you want to open
 * return: a new file descriptor on success, -1 failed
 */
static int open_file(const char *file_name)
{
	int fd = open(file_name, O_RDONLY);
	if (fd < 0) {
		ak_print_error_ex("open file %s, %s\n", file_name, strerror(errno));
		return -1;
	}
	/* set close-on-exec flag */
	fcntl(fd, F_SETFD, FD_CLOEXEC);

	return fd;
}

/**
 * adec_init - init decode for decode mp3
 * return: 0 on success; -1 failed
 */
static int adec_init(struct file_node *file)
{
	struct audio_param pmedia = {0};

	pmedia.type = file->type;
	pmedia.channel_num = file->channel_num;
	pmedia.sample_bits = 16;
	pmedia.sample_rate = file->sample_rate;

	/* open decode */
	voice_play_ctrl.adec_handle = ak_adec_open(&pmedia);
	if (!voice_play_ctrl.adec_handle) {
		ak_print_error_ex("init audio decode failed\n");
		return -1;
	}

	return 0;
}

/*
 * play_voice_tips - do play
 * adec_handle[IN]: audio decode handle
 * data[IN]: data will be decode
 * read_len[IN]: read file length which will be decode
 */
static void play_voice_tips(void *adec_handle, unsigned char *data, int read_len)
{
	int send_len = 0;
	int dec_len = 0;

	while (read_len > 0) {
		dec_len = ak_adec_send_stream(adec_handle, &data[send_len], read_len, 0);
		if (dec_len < 0) {
			ak_print_error_ex("write pcm to DA error!\n");
			break;
		}

		read_len -= dec_len;
		send_len += dec_len;
		ak_sleep_ms(10);
	}
}

/**
 * decoding_and_play - encode data and then send to ao
 * fd[IN], file descriptor by open
 * return, void
 */
static void decoding_and_play(int fd)
{
	/* request streams */
	voice_play_ctrl.stream_handle = ak_adec_request_stream(
			voice_play_ctrl.ao_handle,
			voice_play_ctrl.adec_handle);
	if (!voice_play_ctrl.stream_handle) {
		ak_print_error_ex("fail to request adec stream\n");
		return;
	}

	/* read data */
	int read_len = 0;
	int total_len = 0;
	unsigned char data[BUF_LEN] = {0};

	while (voice_play_ctrl.run_flag) {
		/* read file stream */
		memset(data, 0, sizeof(data));
		read_len = read(fd, data, sizeof(data));
		if(read_len > 0) {
			total_len += read_len;
			/* decode */
			play_voice_tips(voice_play_ctrl.adec_handle, data, read_len);

			ak_sleep_ms(10);
		} else if(0 == read_len) {
			break;
		} else {
			ak_print_error("\nread, %s\n", strerror(errno));
			break;
		}
	}

	/* notify decode lib to output all buffer data */
	ak_adec_notice_stream_end(voice_play_ctrl.adec_handle);
	/* cancel decode, exit until decode end */
	ak_print_normal_ex("-------  stream_handle=%p\n", voice_play_ctrl.stream_handle);
	ak_adec_cancel_stream(voice_play_ctrl.stream_handle);
	
	voice_play_ctrl.stream_handle = NULL;
}

/**
 * voice play working thread, wait semaphore and then decode data
 * 		and send data to ao-module
 */
static void *voice_play_work_th(void *arg)
{
	int fd = -1;
	long int tid = ak_thread_get_tid();
	struct file_node *file = NULL;
	ak_print_normal_ex("thread id: %ld\n", tid);
	ak_thread_set_name("voice_play");

	while (voice_play_ctrl.run_flag) {
		ak_thread_sem_wait(&voice_play_ctrl.play_sem);
		ak_print_normal_ex("wakeup \n");

		/* it can only play one file at same time */
		ak_thread_mutex_lock(&voice_play_ctrl.play_mutex);
		file = list_first_entry_or_null(&voice_play_ctrl.file_list,
			   	struct file_node, list);
		if (!file) {
			ak_thread_mutex_unlock(&voice_play_ctrl.play_mutex);
			continue;
		}

		fd = open_file(file->file_name);
		if (fd < 0) {
			ak_print_error_ex("open file: %s failed\n", file->file_name);
		} else {
			if (adec_init(file)) {
				ak_print_error_ex("adec_init failed\n");
			} else {
				/* read and decode, then write DA */
				ak_print_normal_ex("open voice tips success\n");
				decoding_and_play(fd);
				ak_adec_close(voice_play_ctrl.adec_handle);
				voice_play_ctrl.adec_handle = NULL;
			}

			close(fd);
			fd = -1;
		}

		/* delete from list */
		list_del_init(&file->list);
		free(file);
		file = NULL;
		ak_thread_mutex_unlock(&voice_play_ctrl.play_mutex);
	}
	ak_print_normal_ex("exit ..., tid: %ld\n", tid);

	return 0;
}

/**
 * ak_misc_init_voice_tips - init voice tips play module
 * @ao_handle[IN]: opened ao handle
 * return: 0 on seccuss, -1 failed
 */
int ak_misc_init_voice_tips(void *ao_handle)
{
	if (!ao_handle) {
		return AK_FAILED;
	}
	if(voice_play_ctrl.run_flag) {
		ak_print_error_ex("it has been initialized.\n");
		return AK_FAILED;
	}

	memset(&voice_play_ctrl, 0, sizeof(voice_play_ctrl));
	voice_play_ctrl.ao_handle = ao_handle;
	INIT_LIST_HEAD(&voice_play_ctrl.file_list);
	ak_thread_sem_init(&voice_play_ctrl.play_sem, 0);
	ak_thread_mutex_init(&voice_play_ctrl.play_mutex, NULL);

	ak_thread_mutex_lock(&voice_play_ctrl.play_mutex);
	voice_play_ctrl.run_flag = AK_TRUE;
	ak_thread_create(&voice_play_ctrl.play_tid, voice_play_work_th, NULL,
		ANYKA_THREAD_MIN_STACK_SIZE, -1);
	ak_thread_mutex_unlock(&voice_play_ctrl.play_mutex);

	return AK_SUCCESS;
}

/*
 * ak_misc_add_voice_tips - add voice tips file to be played
 * file_name[IN]: file name include absolutely path
 * file_param[IN]: voice tips file param
 * return: 0 on success, -1 failed
 */
int ak_misc_add_voice_tips(const char *file_name, struct audio_param *file_param)
{
	if (!voice_play_ctrl.run_flag) {
		ak_print_warning_ex("uninitial\n");
		return AK_FAILED;
	}
	ak_print_normal_ex("adding file: %s\n", file_name);

	struct file_node *f = (struct file_node *)calloc(1, sizeof(struct file_node));
	if(!f) {
		ak_print_error_ex("no memory\n");
		return AK_FAILED;
	}

	if (strlen(file_name) > FILE_NAME_LEN)
		ak_print_warning_ex("%s's len big than %d\n", file_name, FILE_NAME_LEN);

	strncpy(f->file_name, file_name, FILE_NAME_LEN);
	f->type = file_param->type;
	f->sample_rate = file_param->sample_rate;
	f->channel_num = file_param->channel_num;

	/* get file info: sample_rate, file encode type and so on */
	ak_thread_mutex_lock(&voice_play_ctrl.play_mutex);
	list_add_tail(&f->list, &voice_play_ctrl.file_list);
	ak_thread_mutex_unlock(&voice_play_ctrl.play_mutex);
	ak_thread_sem_post(&voice_play_ctrl.play_sem);
	
	ak_print_notice_ex("add file: %s success\n", file_name);

	return AK_SUCCESS;
}

/*
 * ak_misc_exit_voice_tips - exit play voice tips
 * return: 0 on success, -1 failed
 */
int ak_misc_exit_voice_tips(void)
{
	ak_print_info_ex("entry ...\n");
	if (!voice_play_ctrl.run_flag) {
		ak_print_warning_ex("uninit\n");
		return -1;
	}

	voice_play_ctrl.run_flag = 0;
	ak_thread_sem_post(&voice_play_ctrl.play_sem);

	ak_thread_join(voice_play_ctrl.play_tid);
	ak_thread_sem_destroy(&voice_play_ctrl.play_sem);

	struct file_node *pos = NULL;
	struct file_node *n = NULL;

	ak_thread_mutex_lock(&voice_play_ctrl.play_mutex);
	list_for_each_entry_safe(pos, n, &voice_play_ctrl.file_list, list) {
		list_del(&pos->list);
		free(pos);
		pos = NULL;
	}
	ak_thread_mutex_unlock(&voice_play_ctrl.play_mutex);
	ak_thread_mutex_destroy(&voice_play_ctrl.play_mutex);

	ak_print_normal_ex("exit ...\n");

	return 0;
}
