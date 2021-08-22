#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <string.h>

#include "ak_common.h"
#include "ak_global.h"
#include "ak_ai.h"
#include "ak_ao.h"
#include "ak_thread.h"

#define TEST_AI_SOURCE		0			/* test audio input source */
#define TEST_AI_VOLUME		0			/* test audio volume */
#define SYNC_PCM_FRAME_TS	0			/* sync audio frame ts */
#define SYNC_AUDIO_TIME		(10*1000)	/* sync per 10 seconds */

#define PCM_READ_LEN		4096		/* ao play buffer length */
#define ADC_MAX_VOLUME		12			/* including digital gain */

struct aec_ai_param {
	int run_flag;			/* capture run flag */
	int save_time;			/* pcm file save time */
	char *save_path;		/* pcm file save path */
	FILE *fp_ai;			/* pcm file file handle */
	ak_pthread_t pcm_tid;	/* capture thread tid */
};

struct aec_ao_param {
	char *play_path;		/* ao play file path */
	FILE *fp_ao;			/* ao play file handle */
	ak_pthread_t pcm_tid;	/* playback thread tid */
};

static struct aec_ai_param pcm_in = {0};	/* ai parameter */
static struct aec_ao_param pcm_out = {0};	/* ao parameter */
static void *ai_handle = NULL;				/* ai handle */
static void *ao_handle = NULL;				/* ao handle */

static int pcm_file_create(void)
{
	char time_str[20] = {0};
	char file_path[255] = {0};
	struct ak_date date;

	ak_get_localdate(&date);
	ak_date_to_string(&date, time_str);
    sprintf(file_path, "%s%s.pcm", pcm_in.save_path, time_str);

	pcm_in.fp_ai = fopen(file_path, "w+");
	if(NULL == pcm_in.fp_ai) {
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

	while(pcm_in.run_flag) {
		/* get the pcm data frame */
		if (ak_ai_get_frame(ai_handle, &frame, 0) < 0) {
			ak_sleep_ms(10);
			continue;
		}

		if(fwrite(frame.data, frame.len, 1, pcm_in.fp_ai) < 0) {
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
		ak_ai_release_frame(ai_handle, &frame);

		if((end_ts - start_ts) >= pcm_in.save_time) {
			break;
		}
	}

	ak_print_normal("\t read_ad_pcm exit\n\n");
	ak_thread_exit();
	return NULL;
}

static void read_ad_run(void *ai_handle, int volume)
{
	/* set source */
	if (ak_ai_set_source(ai_handle, AI_SOURCE_MIC) != 0)
		ak_print_error_ex("set ai source mic fail\n");
	else
		ak_print_normal_ex("set ai source mic success\n");

	/* enable aec,aec only support 8K sample */
	ak_ai_set_aec(ai_handle, AUDIO_FUNC_ENABLE);

	/* set nr and agc,nr&agc only support 8K sample */
	ak_ai_set_nr_agc(ai_handle, AUDIO_FUNC_ENABLE);

	/* set resample */
	ak_ai_set_resample(ai_handle, AUDIO_FUNC_DISABLE);

	/* volume is from 0 to 12,volume 0 is mute */
	ak_ai_set_volume(ai_handle, volume);

	/* clear ai buffer */
	ak_ai_clear_frame_buffer(ai_handle);

    //ak_ai_save_aec_dump_file(ai_handle, AUDIO_FUNC_ENABLE);

	ak_ai_start_capture(ai_handle);

	pcm_in.run_flag = AK_TRUE;
	/* capture thread begin */
	int ret = ak_thread_create(&(pcm_in.pcm_tid), read_ad_thread,
			NULL, ANYKA_THREAD_MIN_STACK_SIZE, -1);
	if (ret) {
		ak_print_error("create read_ad_thread thread FAILED, ret=%d\n", ret);
		return;
	}
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

static void *init_aec_ai(unsigned int sample_rate)
{
	if(NULL == pcm_in.fp_ai) {
		if(AK_FAILED == pcm_file_create()) {
			return NULL;
		}
	}

	/* init ai */
	struct pcm_param param = {0};

	/* sample bits only support 16 bit */
	param.sample_bits = 16;

	/* driver set to AUDIO_CHANNEL_MONO */
	param.channel_num = AUDIO_CHANNEL_MONO;

	/* set sample rate */
	param.sample_rate = sample_rate;

	/* ai open */
    ai_handle = ak_ai_open(&param);
    if(NULL == ai_handle) {
    	return NULL;
    }

	struct ak_audio_aec_attr aec_attr;
	aec_attr.audio_in_digi_gain = 512;
	aec_attr.audio_out_digi_gain = 1024;
	aec_attr.audio_out_threshold = 9830;
	ak_ai_set_aec_attr(ai_handle, &aec_attr);
	
	//ak_ai_enable_eq(ai_handle, 1);
	
	struct ak_audio_eq_attr eq_attr;
	memset(&eq_attr, 0, sizeof(struct ak_audio_eq_attr));
	eq_attr.bands = 1;		 
	eq_attr.bandfreqs[0] = 2000;
	eq_attr.bandgains[0] = (signed short)(-18*(1<<10));
	eq_attr.bandQ[0] = (unsigned short)(1*(1<<10));
	eq_attr.band_types[0] = TYPE_LPF;
	ak_ai_set_eq_attr(ai_handle, &eq_attr);

	/* ak_ai_set_frame_interval must use before ak_ai_get_frame */
	if(AK_SUCCESS == ak_ai_set_frame_interval(ai_handle, AMR_FRAME_INTERVAL)) {
		ak_print_normal_ex("frame interval=%d\n", AMR_FRAME_INTERVAL);
	}

	return ai_handle;
}

static void *init_aec_ao(unsigned int sample_rate)
{
	if (NULL == pcm_out.fp_ao) {
		pcm_out.fp_ao = open_ao_file(pcm_out.play_path);
	}
	if(NULL == pcm_out.fp_ao) {
		return NULL;
	}

	struct pcm_param ao_param = {0};

	/* sample bits only support 16 bit */
	ao_param.sample_bits = 16;

	/* driver set to AUDIO_CHANNEL_MONO */
	ao_param.channel_num = AUDIO_CHANNEL_MONO;

	/* set sample rate */
	ao_param.sample_rate = sample_rate;

	/* ao open */
    ao_handle = ak_ao_open(&ao_param);
    if(NULL == ao_handle) {
    	return NULL;
    }

	struct ak_audio_eq_attr m_eq;

	m_eq.pre_gain = (signed short)(0*(1<<10));
	m_eq.aslc_ena = 0;
	m_eq.aslc_level_max = 28000;
	m_eq.bands = 1;
	
	m_eq.bandfreqs[0] = 800;
	m_eq.bandgains[0] = (signed short)(-6*(1<<10));
	m_eq.bandQ[0] = (unsigned short)(1*(1<<10));
	m_eq.band_types[0] = TYPE_HPF;
	ak_ao_set_eq_attr(ao_handle, &m_eq);
	return ao_handle;
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

static void *write_da_thread(void *arg)
{
	int read_len = 0;
	int total_len = 0;
	unsigned char data[PCM_READ_LEN] = {0};		/* read play pcm file buffer */
	unsigned char full_buf[PCM_READ_LEN * 2];
	FILE *fp = arg;

	while(AK_TRUE) {
		/* read the pcm file data */
		memset(data, 0x00, sizeof(data));
		read_len = fread(data, sizeof(char), sizeof(data), fp);

		if(read_len > 0) {
			total_len += read_len;

			/* double channels */
			copy_for_dual_channel(data, read_len, full_buf);

			/* send frame and play */
			if (ak_ao_send_frame(ao_handle, full_buf, read_len*2, 0) < 0) {
				ak_print_error_ex("write pcm to DA error!\n");
				break;
			}
			print_playing_dot();
		} else if(0 == read_len) {
			ak_print_normal("\n\t read to the end of file\n");
			break;
		} else {
			ak_print_error("read, %s\n", strerror(errno));
			break;
		}
		ak_sleep_ms(10);
	}

	ak_print_normal("\t write_da_pcm exit\n\n");
	ak_thread_exit();
	ak_sleep_ms(20);

	/* disable apeaker */
	ak_ao_enable_speaker(ao_handle, AUDIO_FUNC_DISABLE);
	return NULL;
}

static void write_da_run(void *ao_handle, FILE *fp, int volume)
{
	/* enable speaker must set before ak_ao_send_frame */
	ak_ao_enable_speaker(ao_handle, AUDIO_FUNC_ENABLE);

	/* volume is from 0 to 12,volume 0 is mute */
	ak_ao_set_volume(ao_handle, volume);

	/* ak_ao_set_resample have to use before ak_ao_send_frame */
	ak_ao_set_resample(ao_handle, AUDIO_FUNC_DISABLE);

	/* before ak_ao_send_frame,must clear frame buffer */
	ak_ao_clear_frame_buffer(ao_handle);

	void *arg = (void *)fp;
	/* ao play thread begin */
	int ret = ak_thread_create(&(pcm_out.pcm_tid), write_da_thread,
			arg, ANYKA_THREAD_MIN_STACK_SIZE, -1);
	if (ret) {
		ak_print_error("create write_da_thread thread FAILED, ret=%d\n", ret);
	}
}

/**
 * argv[0]: ak_aec_demo
 * argv[1]: sample rate
 * argv[2]: pcm file save path
 * argv[3]: read AD time, unit: second
 * argv[4]: volume
 * argv[5]: play pcm file path
 * note: We will generate a pcm file in the appointed file path
 * 		such as 20161123-153020.pcm
 */
int main(int argc, char **argv)
{
	if (argc != 7) {
		printf("usage: %s <sample rate> <save path> <time(secs)> <ad_volume> <da_volume> <play path>\n",
				argv[0]);
		printf("eg: %s 8000 /mnt/ 120 4 3 /mnt/xxx.pcm\n", argv[0]);
		printf("note: max sample rate 48000Hz\n");
		printf("note: max volume %d, [0-%d]\n", ADC_MAX_VOLUME, ADC_MAX_VOLUME);
		return AK_FAILED;
	}

	/* get volume */
	int ad_volume = atoi(argv[4]);
	int da_volume = atoi(argv[5]);

    if (ad_volume < 0 || ad_volume > ADC_MAX_VOLUME) {
		printf("volume is 0~%d\n", ADC_MAX_VOLUME);
		return AK_FAILED;
	}

	/* get sample rete */
	unsigned int sample_rate = atoi(argv[1]);

	/* init ai */
	pcm_in.save_time = atoi(argv[3]) * 1000;
	pcm_in.save_path = argv[2];

	ai_handle = init_aec_ai(sample_rate);
	if (NULL == ai_handle) {
		ak_print_error_ex("*** ai_handle is NULL \n");
		return AK_FAILED;
	}

	/* init ao */
	pcm_out.play_path = argv[6];
	ao_handle = init_aec_ao(sample_rate);
	if (NULL == ao_handle) {
		ak_print_error_ex("--- ao_handle is NULL \n");
		return AK_FAILED;
	}

	/* ai run */
    read_ad_run(ai_handle, ad_volume);

	/* ao run */
    write_da_run(ao_handle, pcm_out.fp_ao, da_volume);

    ak_thread_join(pcm_in.pcm_tid);
    ak_thread_join(pcm_out.pcm_tid);

	/* ai close */
    if(NULL != pcm_in.fp_ai) {
    	fclose(pcm_in.fp_ai);
    	pcm_in.fp_ai= NULL;
    }

    ak_ai_close(ai_handle);

	/* ao close */
    if(NULL != pcm_out.fp_ao) {
    	fclose(pcm_out.fp_ao);
    	pcm_out.fp_ao = NULL;
    }
    ak_ao_close(ao_handle);
    printf("----- %s exit -----\n", argv[0]);

	return 0;
}
