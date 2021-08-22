#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "ak_common.h"
#include "ak_global.h"
#include "ak_ao.h"
#include "ak_ipc_srv.h"

#define PCM_READ_LEN		 	4096
#define AO_DAC_MAX_VOLUME		12		/* max volume according to ao */
#define TEST_AO_VOLUME			0		/* test audio volume */

#if TEST_AO_VOLUME
	struct ak_timeval volume_time; 		/* current time */
	int cur_volume;						/* current volume level */
	int count = 0;				/* count the times change volume from 1~12 */
#endif

#if TEST_AO_VOLUME
static void test_ao_volume_adjust(void *ao_handle)
{
	struct ak_timeval cur_time;

	if (0 > cur_volume)
		cur_volume = 0;

	ak_get_ostime(&cur_time);
	long diff_time = ak_diff_ms_time(&cur_time, &(volume_time));

	/* change volume every 10s */
	if(diff_time >= 10*1000) {
		/* volume change from 0~12 */
		++cur_volume;
		if (cur_volume > AO_DAC_MAX_VOLUME) {
			cur_volume = 0;
			count++;
		}

		ak_print_notice_ex("cur_volume=%d\n", cur_volume);
		ak_get_ostime(&(volume_time));
		ak_ao_set_volume(ao_handle, cur_volume);
	}
}
#endif

static FILE* open_pcm_file(const char *pcm_file)
{
	FILE *fp = fopen(pcm_file, "r");
	if(NULL == fp) {
		printf("open pcm file err\n");
		return NULL;
	}

	printf("open pcm file: %s OK\n", pcm_file);
	return fp;
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

static void write_da_pcm(void *ao_handle, FILE *fp, unsigned int channel_num,
                unsigned int volume)
{
	int read_len = 0;
	int send_len = 0;
	int total_len = 0;
	unsigned char data[PCM_READ_LEN] = {0};
	unsigned char full_buf[PCM_READ_LEN * 2];

	/* enable speaker must set before ak_ao_send_frame */
	ak_ao_enable_speaker(ao_handle, AUDIO_FUNC_ENABLE);

	/* volume is from 0 to 12,volume 0 is mute */
	ak_ao_set_dac_volume(ao_handle, 4);

	/* ak_ao_set_resample have to use before ak_ao_send_frame */
	ak_ao_set_resample(ao_handle, AUDIO_FUNC_DISABLE);

	/* before ak_ao_send_frame,must clear frame buffer */
	ak_ao_clear_frame_buffer(ao_handle);

#if TEST_AO_VOLUME
	/* get current time */
	ak_get_ostime(&(volume_time));
#endif

	while(AK_TRUE) {
		/* read the pcm file data */
		memset(data, 0x00, sizeof(data));
		/* read pcm data */
		read_len = fread(data, sizeof(char), sizeof(data), fp);

		if(read_len > 0) {
			total_len += read_len;
			switch (channel_num) {
			case AUDIO_CHANNEL_MONO:
			    copy_for_dual_channel(data, read_len, full_buf);
			    send_len = (read_len << 1);
			    break;
			case AUDIO_CHANNEL_STEREO:
                memcpy(full_buf, data, read_len);
                send_len = read_len;
			    break;
			default:
			    return;
			}

			/* send frame and play */
			if (ak_ao_send_frame(ao_handle, full_buf, send_len, 0) < 0) {
				ak_print_error_ex("write pcm to DA error!\n");
				break;
			}

#if TEST_AO_VOLUME
			/* change volume */
			test_ao_volume_adjust(ao_handle);
#endif

			print_playing_dot();
			ak_sleep_ms(10);
		} else if(0 == read_len) {
		    ak_ao_notice_frame_end(ao_handle);
			/* read to the end of file */
			ak_print_normal("\n\t read to the end of file\n");
#if TEST_AO_VOLUME
			fseek(fp, 0, SEEK_SET);
#else
			break;
#endif
		} else {
			ak_print_error("read, %s\n", strerror(errno));
			break;
		}

#if TEST_AO_VOLUME
		/* change the volume from 0~12 for 3 times */
		if (3 == count)
			break;
#endif
	}

	/* wait the driver play end */
	if (0 == read_len) {
		wait_play_finished(ao_handle);
	}

	ak_sleep_ms(100);

	/* disable speaker */
	ak_ao_enable_speaker(ao_handle, AUDIO_FUNC_DISABLE);
	ak_print_normal("%s exit\n", __func__);
}

/**
 * argv[0]: ak_ao_demo
 * argv[1]: sample rate
 * argv[2]: appointed pcm file, absolute path
 * argv[3]: volume
 * note: read the appointed pcm file, and then output to DA
 */
int main(int argc, char **argv)
{
	if (argc != 5) {
		printf("usage: %s [sample rate] [channel num] [pcm_file] [volume]\n",
		    argv[0]);
		printf("eg.: %s 8000 1 /mnt/20161123-153020.pcm 6\n", argv[0]);
		return AK_FAILED;
	}

	printf("----- %s -----\n", argv[0]);
	printf("version: %s\n\n", ak_ao_get_version());

	/* open pcm fle */
	FILE *fp = open_pcm_file(argv[3]);
	if(NULL == fp) {
		return AK_FAILED;
	}

    /* get channel number */
	int channel_num = atoi(argv[2]);
	if ((channel_num > AUDIO_CHANNEL_STEREO)
	    || (channel_num < AUDIO_CHANNEL_MONO)) {
		printf("channel number is [1, 2]\n");
		return AK_FAILED;
	}

	/* get volume */
	int volume = atoi(argv[4]);
	if (volume > AO_DAC_MAX_VOLUME || volume < 0) {
		printf("volume is 0~%d\n", AO_DAC_MAX_VOLUME);
		return AK_FAILED;
	}

	int ret = AK_SUCCESS;
	ak_cmd_server_register(ANYKA_IPC_PORT, "ao_demo7000");

	struct pcm_param ao_param = {0};
	/* sample bits only support 16 bit */
	ao_param.sample_bits = 16;
	/* driver set to AUDIO_CHANNEL_STEREO */
	ao_param.channel_num = channel_num;
	/* get sample rate */
	ao_param.sample_rate = atoi(argv[1]);

	/* open ao */
    void *ao_handle = ak_ao_open(&ao_param);
    if(NULL == ao_handle) {
    	ret = AK_FAILED;
    	goto main_end;
    }
	struct ak_audio_eq_attr m_eq;

	m_eq.pre_gain = (signed short)(0*(1<<10));
	m_eq.aslc_ena = 0;
	m_eq.aslc_level_max = 28000;
	m_eq.bands = 2;
	
	m_eq.bandfreqs[0] = 1200;
	m_eq.bandgains[0] = (signed short)(-12*(1<<10));
	m_eq.bandQ[0] = (unsigned short)(0.9*(1<<10));
	m_eq.band_types[0] = TYPE_LSF;

	m_eq.bandfreqs[1] = 400;
	m_eq.bandgains[1] = 0;
	m_eq.bandQ[1] = (unsigned short)(0.8*(1<<10));
	m_eq.band_types[1] = TYPE_HPF;
	ak_ao_set_eq_attr(ao_handle, &m_eq);

	/* get pcm data,send to play */
    write_da_pcm(ao_handle, fp, channel_num, volume);

	/* play finish,close pcm file */
    if(NULL != fp) {
    	fclose(fp);
    	fp = NULL;
    }

    /* close ao */
    ak_ao_close(ao_handle);
    ao_handle = NULL;

main_end:
	ak_cmd_server_unregister(ANYKA_IPC_PORT);
    printf("----- %s exit -----\n", argv[0]);

	return ret;
}
