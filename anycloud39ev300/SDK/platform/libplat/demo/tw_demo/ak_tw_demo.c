#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include "ak_global.h"
#include "ak_common.h"
#include "ak_ai.h"
#include "ak_tw.h"

#define TW_TEST_SAMPLE_RATE     48000

static void td_usage(char *pro_name)
{
    printf("Usage: \n"
        "1. %s\n"
        "\tdetect tone wave;\n"
        "2. %s ssid password save_file\n"
        "\tgenerate tone wave file.\n",
        pro_name, pro_name);
}

static void process_sig(int sig)
{
	printf("tone catch signal: %d\n", sig);
	if((SIGTERM == sig) || (SIGINT == sig) || (SIGSEGV == sig)){
		exit(EXIT_FAILURE);
	}
}

static void init_sig(void)
{
	signal(SIGSEGV, process_sig);
	signal(SIGINT, process_sig);
	signal(SIGTERM, process_sig);
}

static void* start_ai(void)
{
    struct pcm_param param = {0};

	/* sample bits only support 16 bit */
	param.sample_bits = 16;
	/* channel number only support AUDIO_CHANNEL_MONO */
	param.channel_num = AUDIO_CHANNEL_MONO;
	/* get sample rate */
	param.sample_rate = TW_TEST_SAMPLE_RATE;

    /* ai open */
    void *ai_handle = ak_ai_open(&param);
    if(NULL == ai_handle) {
    	return NULL;
    }

	/* ak_ai_set_frame_interval must use before ak_ai_get_frame */
	if(AK_SUCCESS == ak_ai_set_frame_interval(ai_handle, AUDIO_DEFAULT_INTERVAL)) {
		ak_print_normal_ex("frame interval=%d\n", AUDIO_DEFAULT_INTERVAL);
	}

	/* set source */
	if (ak_ai_set_source(ai_handle, AI_SOURCE_MIC))
		ak_print_error_ex("set ai source mic fail\n");
	else
		ak_print_normal_ex("set ai source mic success\n");

	/* set aec,aec only support 8K sample */
	ak_ai_set_aec(ai_handle, AUDIO_FUNC_DISABLE);

	/* set nr and agc,nr&agc only support 8K sample */
	ak_ai_set_nr_agc(ai_handle, AUDIO_FUNC_DISABLE);

	/* set resample */
	ak_ai_set_resample(ai_handle, AUDIO_FUNC_DISABLE);

	/* volume is from 0 to 12,volume 0 is mute */
	ak_ai_set_volume(ai_handle, 6);

	/* clear ai buffer */
	ak_ai_clear_frame_buffer(ai_handle);

	/* start capture will begin to get frame */
	ak_ai_start_capture(ai_handle);

	return ai_handle;
}

static int detect_tone_wave(void)
{
    int ret = AK_FAILED;
    void *ai_handle = start_ai();
    if (ai_handle) {
        struct tw_detect detect = {0};

        detect.ai_handle = ai_handle;
        detect.detect_time = 60*1000;
        detect.parse_str = NULL;

        detect.debug_file_flag = 0;
        if (detect.debug_file_flag) {
            detect.debug_file = "/tmp/tone_detect.wav";
        } else {
            detect.debug_file = NULL;
        }

        ret = ak_tw_get_detect(&detect);
        if (!ret) {
            ak_print_notice_ex("parse: %s\n", detect.parse_str);
            ak_tw_release_detect(&detect);
        }

        ak_ai_close(ai_handle);
    }

    return ret;
}

static int write_tone_wave(unsigned char *data, int data_len)
{
    char time_str[20] = {0};
	char file_path[255] = {0};
	struct ak_date date;

	/* get the file path */
	ak_get_localdate(&date);
	ak_date_to_string(&date, time_str);
    sprintf(file_path, "/tmp/tone_%s.wav", time_str);

	/* open file */
	FILE *wav_fp = fopen(file_path, "w+");
	if(!wav_fp) {
		printf("open tone wave file failed\n");
		return AK_FAILED;
	}

	printf("open save file: %s OK\n", file_path);

	fwrite(data, data_len, 1, wav_fp);
    fclose(wav_fp);

    return AK_SUCCESS;
}

static int generate_tone_wave(const char *ssid, const char *passwd, char *wav_file)
{
    char str[100] = {0};
    struct tw_generate gen;

    sprintf(str, "%s@%s\n", ssid, passwd);
    gen.gen_str = str;
    gen.sample_rate = TW_TEST_SAMPLE_RATE;

    gen.data_len = 0;
    gen.data = NULL;

    gen.write_file_flag = 0;
    if (gen.write_file_flag) {
        gen.wav_file = wav_file;
    } else {
        gen.wav_file = NULL;
    }

    int ret = ak_tw_get_generate(&gen);
    if (!ret) {
        if (gen.data) {
            /* the "gen.data" is the same as in "wav_file". */
            write_tone_wave(gen.data, gen.data_len);
        }
        ak_tw_release_generate(&gen);
    }

    return ret;
}

int main(int argc, char **argv)
{
	init_sig();

    switch (argc) {
    case 1: /* recorgnize */
        detect_tone_wave();
        break;
    case 4: /* generate tone */
        ak_print_normal("input info:\n\tssid: %s"
            "\n\tpswd: %s"
            "\n\twav_save_path: %s\n\n",
            argv[1], argv[2], argv[3]);
        generate_tone_wave(argv[1], argv[2], argv[3]);
	    break;
    default:
        td_usage(argv[0]);
        break;
    }

    return 0;
}
