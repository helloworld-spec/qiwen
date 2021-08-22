#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <string.h>

#include "ak_common.h"
#include "ak_global.h"
#include "ak_ai.h"
#include "ak_ao.h"
#include "ak_thread.h"

#define TEST_AI_SOURCE		0			/* test audio input source*/
#define AEC_EX_AI_VOLUME	5			/* aec ex demo ai volume */
#define AEC_EX_AO_VOLUME	3			/* aec ex demo ao volume */
#define SYNC_PCM_FRAME_TS	0			/* sync audio frame ts*/
#define SYNC_AUDIO_TIME		(10*1000)	/* sync per 10 seconds*/

/* ai data save path */
#define AEC_EX_SAVE_PATH	"/tmp/"
/* ai data save path */
#define AEC_EX_PLAY_PATH	"/tmp/talk.pcm"

#define AEC_EX_SAVE_TIME	(30*1000)	/* save ai data time, unit: second */
#define AEC_EX_SAMPLE_RATE	(8000)	    /* sample rate */

#define PCM_READ_LEN		4096		/* ao play buffer length */
#define ADC_MAX_VOLUME		12			/* including digital gain*/

struct aec_ex_param {
	int run_flag;			/* run flag */
	void *handle;           /* ai/ao handle */
	FILE *fp;			    /* pcm file file handle */
	ak_pthread_t tid;	    /* thread tid */
};

static struct aec_ex_param pcm_in = {0};	/* ai parameter */
static struct aec_ex_param pcm_out = {0};	/* ao parameter */

static int ai_save_time = AEC_EX_SAVE_TIME;
static int aec_pre_status = AK_FALSE;           /* 1-enable; 0-disable */
static int aec_cur_status = AK_FALSE;           /* 1-enable; 0-disable */
static int aec_status_switch = AK_FALSE;        /* 1-switch; 0-do not switch */

static void aec_ex_notice(void)
{
    printf("note 1: ai and ao sample rate must be 8000 \n");
	printf("note 2: ai save pcm file is 30s \n");
	printf("note 3: ai and ao volume is 6 \n");
	printf("note 4: ai pcm file is save to /tmp/ \n");
	printf("note 5: ao play pcm path is /tmp/talk.pcm \n");
	printf("\n");
}

static void aec_ex_usage(const char *app_name)
{
    printf("usage: %s [1-10] \n", app_name);
	printf(" 1: test ai with aec disable, no ao\n");
	printf(" 2: test ai with aec enable, no ao\n");
	printf(" 3: open ai -> open ao, aec disable\n");
	printf(" 4: open ai -> open ao, aec enable\n");
	printf(" 5: open ao -> open ai, aec disable\n");
	printf(" 6: open ao -> open ai, aec enable\n");
	printf(" 7: open ai -> open ao, aec disable and ao replay\n");
	printf(" 8: open ai -> open ao, aec enable and ao replay\n");
	printf(" 9: open ai -> open ao, aec dynamic, disable first\n");
	printf(" 10: open ai -> open ao, aec dynamic, enable first\n");
	printf(" 11: talk\n");
}

static inline void close_audio(void)
{
    if (pcm_out.run_flag) {
        ak_thread_join(pcm_out.tid);
    }
    if (pcm_in.run_flag) {
        ak_thread_join(pcm_in.tid);
    }

    if (pcm_in.handle) {
        ak_ai_set_aec(pcm_in.handle, AUDIO_FUNC_DISABLE);
    }

    /* ao close */
    if(pcm_out.fp) {
    	fclose(pcm_out.fp);
    	pcm_out.fp = NULL;
    }
    if (pcm_out.handle) {
        ak_ao_close(pcm_out.handle);
        pcm_out.handle = NULL;
    }

	/* ai close */
    if(pcm_in.fp) {
    	fclose(pcm_in.fp);
    	pcm_in.fp = NULL;
    }
    if (pcm_in.handle) {
        ak_ai_close(pcm_in.handle);
        pcm_in.handle = NULL;
    }
}

static int pcm_file_create(void)
{
	char time_str[20] = {0};
	char file_path[255] = {0};
	struct ak_date date;

	ak_get_localdate(&date);
	ak_date_to_string(&date, time_str);
    sprintf(file_path, "/tmp/%s.pcm", time_str);

	pcm_in.fp = fopen(file_path, "w+");
	if(!pcm_in.fp) {
		printf("open pcm file err\n");
		return AK_FAILED;
	}

	printf("open save file: %s OK\n", file_path);
	return AK_SUCCESS;
}

static void *read_ad_thread(void *arg)
{
	unsigned long long start_ts = 0;
	unsigned long long end_ts = 0;
	unsigned long pre_seq_no = 0;
	struct frame frame = {0};

	while (pcm_in.run_flag) {
		/* get the pcm data frame */
		if (ak_ai_get_frame(pcm_in.handle, &frame, 0) < 0) {
			ak_sleep_ms(10);
			continue;
		}

		if(fwrite(frame.data, frame.len, 1, pcm_in.fp) < 0) {
    		printf("write file err\n");
    		break;
    	}

		if (frame.seq_no != (pre_seq_no + 1)) {
			ak_print_normal_ex("audio: ts=%llu, len=%u, seq_no=%lu\n",
    			frame.ts, frame.len, frame.seq_no);
		}
		pre_seq_no = frame.seq_no;

		if(0 == start_ts) {
			start_ts = frame.ts;
			end_ts = frame.ts;
		}
		end_ts = frame.ts;
		ak_ai_release_frame(pcm_in.handle, &frame);

		if((end_ts - start_ts) >= ai_save_time) {
			break;
		}
		if (aec_cur_status != aec_pre_status) {
		    /* reset aec */
	        ak_ai_set_aec(pcm_in.handle, aec_cur_status);
	        aec_pre_status = aec_cur_status;
	        aec_status_switch = AK_TRUE;
		}
	}

	ak_print_normal("\t read_ad_pcm exit\n\n");
	ak_thread_exit();
	return NULL;
}

static void read_ad_run(void)
{
	/* set source */
	if (ak_ai_set_source(pcm_in.handle, AI_SOURCE_MIC) != 0)
		ak_print_error_ex("set ai source mic fail\n");
	else
		ak_print_normal_ex("set ai source mic success\n");

	ak_print_notice_ex("current aec enable=%d\n", aec_cur_status);
	aec_pre_status = aec_cur_status;
	/* set aec */
	ak_ai_set_aec(pcm_in.handle, aec_cur_status);

	/* enable nr&agc */
	ak_ai_set_nr_agc(pcm_in.handle, AUDIO_FUNC_ENABLE);

	/* set resample */
	ak_ai_set_resample(pcm_in.handle, AUDIO_FUNC_DISABLE);

	/* volume is from 0 to 12, volume 0 is mute */
	ak_ai_set_volume(pcm_in.handle, AEC_EX_AI_VOLUME);

	/* clear ai buffer */
	ak_ai_clear_frame_buffer(pcm_in.handle);

	/* start capture will begin to get frame */
	ak_ai_start_capture(pcm_in.handle);

	pcm_in.run_flag = AK_TRUE;

	/* capture thread begin */
	int ret = ak_thread_create(&(pcm_in.tid), read_ad_thread,
			NULL, ANYKA_THREAD_MIN_STACK_SIZE, -1);
	if (ret) {
		ak_print_error("create read_ad_thread thread FAILED, ret=%d\n", ret);
		return;
	}
}

static void process_signal(int sig)
{
	ak_print_notice("\t signal %d caught", sig);
	if((SIGTERM == sig) || (SIGINT == sig) || (SIGSEGV == sig)){
		pcm_in.run_flag = AK_FALSE;
		pcm_out.run_flag = AK_TRUE;
	}
}

static int register_signal(void)
{
	signal(SIGSEGV, process_signal);
	signal(SIGINT, process_signal);
	signal(SIGTERM, process_signal);
	signal(SIGCHLD, SIG_IGN);
	return 0;
}

static FILE* open_ao_file(const char *pcm_file)
{
	FILE *fp = fopen(pcm_file, "r");
	if(NULL == fp) {
		printf("open pcm file err\n");
		return NULL;
	}

	printf("open pcm file: %s OK\n", pcm_file);
	return fp;
}

static int init_aec_ai(void)
{
	if(!pcm_in.fp) {
		if(pcm_file_create()) {
			return AK_FAILED;
		}
	}

	/* init ai */
	struct pcm_param param = {0};

	/* sample bits only support 16 bit */
	param.sample_bits = 16;

	/* driver set to AUDIO_CHANNEL_MONO */
	param.channel_num = AUDIO_CHANNEL_MONO;

	/* set sample rate */
	param.sample_rate = AEC_EX_SAMPLE_RATE;

	/* ai open */
    pcm_in.handle = ak_ai_open(&param);
    if(!pcm_in.handle) {
        close_audio();
    	return AK_FAILED;
    }

    /* ak_ai_set_frame_interval must use before ak_ai_get_frame */
    int ret = ak_ai_set_frame_interval(pcm_in.handle, AMR_FRAME_INTERVAL);
	if (!ret) {
		ak_print_normal_ex("frame interval=%d\n", AMR_FRAME_INTERVAL);
	}

	return ret;
}

static int init_aec_ao(void)
{
	if (!pcm_out.fp) {
		pcm_out.fp = open_ao_file(AEC_EX_PLAY_PATH);
	}
	if(!pcm_out.fp) {
		printf("open %s failed!\n", AEC_EX_PLAY_PATH);
		return AK_FAILED;
	}

    int ret = AK_FAILED;
	struct pcm_param ao_param = {0};

	/* sample bits only support 16 bit */
	ao_param.sample_bits = 16;

	/* driver set to AUDIO_CHANNEL_MONO */
	ao_param.channel_num = AUDIO_CHANNEL_MONO;

	/* set sample rate */
	ao_param.sample_rate = AEC_EX_SAMPLE_RATE;

	/* ao open */
    pcm_out.handle = ak_ao_open(&ao_param);
    if(pcm_out.handle) {
    	ret = AK_SUCCESS;
    } else {
        close_audio();
    }

    return ret;
}

/**
 * copy_for_dual_channel - copy the same data for another channel
 * @src[IN]: audio pcm data
 * @len[IN]: audio pcm data len
 * @dest[IN]: audio pcm data new buffer
 * notes: we just copy the same pcm data for another channel
 */
static void copy_for_dual_channel(const unsigned char *src, int len,
							unsigned char *dest)
{
	int j = 0;
	int count = (len / 2);

	for (j=0; j<count; ++j) {
		dest[j * 4] = src[j * 2];
		dest[j * 4 + 1] = src[j * 2 + 1];
		dest[j * 4 + 2] = src[j * 2];
		dest[j * 4 + 3] = src[j * 2 + 1];
	}
}

/**
 * print_playing_dot - print . when playing every second
 * notes:
 */
static void print_playing_dot(void)
{
	static unsigned char first_flag = AK_TRUE;
	static struct ak_timeval cur_time;
	static struct ak_timeval print_time;

	if (first_flag) {
		first_flag = AK_FALSE;
		ak_get_ostime(&cur_time);
		ak_get_ostime(&print_time);
		ak_print_normal("\n.");
	}

	ak_get_ostime(&cur_time);
	if (ak_diff_ms_time(&cur_time, &print_time) >= 1000) {
		ak_get_ostime(&print_time);
		ak_print_normal(".");
	}
}

static void wait_play_finished(void *ao_handle)
{
    enum ao_play_status pre_status= AO_PLAY_STATUS_RESERVED;
	enum ao_play_status cur_status = AO_PLAY_STATUS_RESERVED;
    int total_time = 0;

    do {
    	/* get ao status */
    	cur_status = ak_ao_get_play_status(ao_handle);
    	if (pre_status != cur_status) {
    		pre_status = cur_status;
	    	switch(cur_status) {
	    	case AO_PLAY_STATUS_READY:
	    		ak_print_normal("play status=%d, ready to play\n", cur_status);
	    		break;
	    	case AO_PLAY_STATUS_PLAYING:
	    		ak_print_normal("play status=%d, playing, wait time: %d\n",
	    			cur_status, total_time);
	    		break;
	    	case AO_PLAY_STATUS_FINISHED:
	    		ak_print_normal("play status=%d, play finished\n", cur_status);
	    		break;
	    	default:
	    		ak_print_normal("play status=%d, reserved\n", cur_status);
	    		break;
	    	}
    	}

		/* wait the data play finish */
		if (AO_PLAY_STATUS_FINISHED != cur_status) {
			ak_sleep_ms(10);
			total_time += 10;
		}
	} while (AO_PLAY_STATUS_FINISHED != cur_status);
}

static void *write_da_thread(void *arg)
{
	int read_len = 0;
	int total_len = 0;
	unsigned char data[PCM_READ_LEN] = {0};
	unsigned char full_buf[PCM_READ_LEN * 2];

    fseek(pcm_out.fp, 0x00, SEEK_SET);
	while(pcm_out.run_flag) {
		/* read the pcm file data */
		memset(data, 0x00, sizeof(data));
		read_len = fread(data, sizeof(char), sizeof(data), pcm_out.fp);

		if(read_len > 0) {
			total_len += read_len;

			/* double channels */
			copy_for_dual_channel(data, read_len, full_buf);

			/* send frame and play */
			if (ak_ao_send_frame(pcm_out.handle, full_buf, read_len*2, 0) < 0) {
				ak_print_error_ex("write pcm to DA error!\n");
				break;
			}
			print_playing_dot();
		} else if(0 == read_len) {
		    ak_ao_notice_frame_end(pcm_out.handle);
			ak_print_normal("\n\t read to the end of file\n");
			break;
		} else {
			ak_print_error("read, %s\n", strerror(errno));
			break;
		}
		ak_sleep_ms(10);
	}

	/* wait the driver play end */
	if (0 == read_len) {
		wait_play_finished(pcm_out.handle);
	}
	ak_sleep_ms(10);

	ak_print_normal("\t write_da_pcm exit\n\n");
	ak_thread_exit();

	/* disable apeaker */
	ak_ao_enable_speaker(pcm_out.handle, AUDIO_FUNC_DISABLE);
	return NULL;
}

static void write_da_run(int run_count)
{
	/* enable speaker must set before ak_ao_send_frame */
	ak_ao_enable_speaker(pcm_out.handle, AUDIO_FUNC_ENABLE);

	/* volume is from 0 to 12, volume 0 is mute */
	ak_ao_set_volume(pcm_out.handle, AEC_EX_AO_VOLUME);

	/* ak_ao_set_resample have to use before ak_ao_send_frame */
	ak_ao_set_resample(pcm_out.handle, AUDIO_FUNC_DISABLE);

    if (0 == run_count) {
        /* before ak_ao_send_frame,must clear frame buffer */
        ak_ao_clear_frame_buffer(pcm_out.handle);
    }

    pcm_out.run_flag = AK_TRUE;
	/* ao play thread begin */
	int ret = ak_thread_create(&(pcm_out.tid), write_da_thread,
			NULL, ANYKA_THREAD_MIN_STACK_SIZE, -1);
	if (ret) {
		ak_print_error("create write_da_thread thread FAILED, ret=%d\n", ret);
		return;
	}
}

/**
 * open ai, do not open ao, test aec enable/disable
 */
static void test_ai_alone(void)
{
	if (!init_aec_ai()) {
		/* ai run */
        read_ad_run();

        close_audio();
	}
}

/**
 * open_ai_first - open ai first then open ao
 */
static void open_ai_first(void)
{
	if (init_aec_ai()) {
		ak_print_error_ex("init aec ai failed\n");
		goto ai_first_end;
	}

	/* ai run */
    read_ad_run();

	/* init ao */
	if (init_aec_ao()) {
		ak_print_error_ex("init aec ao failed\n");
		goto ai_first_end;
	}

	/* ao run */
    write_da_run(0);

ai_first_end:
    close_audio();
}

/**
 * open_ao_first - open ao first then open ai
 */
static void open_ao_first(void)
{
	/* init ao */
	if (init_aec_ao()) {
		ak_print_error_ex("init aec ao failed\n");
		goto ao_first_end;
	}

	/* ao run */
    write_da_run(0);

	if (init_aec_ai()) {
		ak_print_error_ex("init aec ai failed\n");
		goto ao_first_end;
	}

	/* ai run */
    read_ad_run();

ao_first_end:
    close_audio();
}

static void aec_ao_replay(void)
{
	if (init_aec_ai()) {
		ak_print_error_ex("init aec ai failed\n");
		goto ao_replay_end;
	}

	/* init ao */
	if (init_aec_ao()) {
		ak_print_error_ex("init aec ao failed\n");
		goto ao_replay_end;
	}

    /* ai run */
    ai_save_time = (AEC_EX_SAVE_TIME * 3);
    read_ad_run();

    int count = 0;
    while (AK_TRUE) {
    	/* ao run */
        write_da_run(count);
        ak_thread_join(pcm_out.tid);
        ++count;
        if (count >= 2) {
            break;
        }

        ak_sleep_ms(30*1000);
    }

ao_replay_end:
    close_audio();
}

/*
 * 1. open ai and disable aec first, and then open ao;
 * 2. after ao play finished, just keep ai for a time;
 * 3. enable aec, and then start playing again after sleep time.
 */
static void aec_dynamic_use(void)
{
	if (init_aec_ai()) {
		ak_print_error_ex("init aec ai failed\n");
		return;
	}

	/* init ao */
	if (init_aec_ao()) {
		ak_print_error_ex("init aec ao failed\n");
		return;
	}

    /* ai run */
    ai_save_time = (AEC_EX_SAVE_TIME * 3);
    read_ad_run();

    int count = 0;
    while (AK_TRUE) {
    	/* ao run */
        write_da_run(count);
        ak_thread_join(pcm_out.tid);

        ++count;
        if (count >= 2) {
            break;
        }

        ak_sleep_ms(30*1000);
        aec_status_switch = AK_FALSE;
        aec_cur_status = !aec_cur_status;
        while (!aec_status_switch) {
            ak_sleep_ms(10);
        }
    }

    close_audio();
}

static void aec_talk(void)
{
	if (init_aec_ai()) {
		ak_print_error_ex("init aec ai failed\n");
		return;
	}

	/* init ao */
	if (init_aec_ao()) {
		ak_print_error_ex("init aec ao failed\n");
		return;
	}

    /* ai run */
    ai_save_time = (AEC_EX_SAVE_TIME * 3);
    read_ad_run();

    int count = 0;
    while (AK_TRUE) {
    	/* ao run */
        write_da_run(count);
        ak_thread_join(pcm_out.tid);

        ++count;
        if (count >= 2) {
            break;
        }

        aec_status_switch = AK_FALSE;
        aec_cur_status = !aec_cur_status;
        while (!aec_status_switch) {
            ak_sleep_ms(10);
        }
        ak_sleep_ms(10*1000);

        ak_sleep_ms(10*1000);
        aec_status_switch = AK_FALSE;
        aec_cur_status = !aec_cur_status;
        while (!aec_status_switch) {
            ak_sleep_ms(10);
        }
    }

    close_audio();
}

/**
 * argv[0]: ak_aec_ex_demo
 * argv[1]: input use no.
 */
int main(int argc, char **argv)
{
	if (argc != 2) {
        aec_ex_notice();
        aec_ex_usage(argv[0]);
		return AK_FAILED;
	}

    register_signal();

    int test_num = atoi(argv[1]);
	switch (test_num) {
	case 1: //disable aec
	    aec_cur_status = AK_FALSE;
		test_ai_alone();
		break;
	case 2: //enable aec
	    aec_cur_status = AK_TRUE;
		test_ai_alone();
		break;
	case 3: //disable aec
	    aec_cur_status = AK_FALSE;
		open_ai_first();
		break;
	case 4: //enable aec
	    aec_cur_status = AK_TRUE;
		open_ai_first();
		break;
	case 5: //disable aec
	    aec_cur_status = AK_FALSE;
		open_ao_first();
		break;
	case 6: //enable aec
	    aec_cur_status = AK_TRUE;
		open_ao_first();
		break;
	case 7: //disable aec
	    aec_cur_status = AK_FALSE;
        aec_ao_replay();
		break;
	case 8: //enable aec
	    aec_cur_status = AK_TRUE;
        aec_ao_replay();
		break;
	case 9: //disable aec first then enable
	    aec_cur_status = AK_FALSE;
	    aec_dynamic_use();
	    break;
	case 10://enable aec first then disable
	    aec_cur_status = AK_TRUE;
	    aec_dynamic_use();
	    break;
	case 11://test talk
	    aec_cur_status = AK_TRUE;
	    aec_talk();
	    break;
	default:
		break;
	}

    printf("----- %s exit -----\n", argv[0]);
	return 0;
}
