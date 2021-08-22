#include "ak_ai.h"
#include "ak_tw.h"

#include "pcm.h"
#include "tw_common.h"

#define RESULT_BUF_SIZE         128
#define FILTER_INBUF_SIZE       512

static const char *tw_version = "libplat_tw V1.0.01";

static FILE *detect_fp = NULL;

/**
 * calc_frame_size - calculate the size of pcm
 * @user[IN]: pointer point to adc parameters. 
 * return: pcm size
 */
static int calc_frame_size(pcm_user_t *user)
{
	/* size = sample rete* channel num * time(ms) *sample bit /8 / 1000 */
	/* 8 is bit to byte */
	/* 1000 is second to ms */
	/* user->interval is ms */
	unsigned int sample_bytes = (user->param.sample_bits >> 3);
	int size = (user->param.rate * user->param.channels
			* user->interval * sample_bytes ) / 1000;
	if (size & 1) {
		size++;
	}

	return size;
}

/**
 * parse_capture_pcm - put data to detect tone wave 
 * @detect[IN/OUT]: tone wave detection control and data
 * @filter[IN]: filter handle
 * @pcm_buf[IN]: pcm buffer
 * return: 0 success, -1 failed
 * notes: 
 */
static int parse_capture_pcm(struct tw_detect *detect, void *filter,
                unsigned char *pcm_buf)
{
    int ret = AK_FAILED;
    char result[RESULT_BUF_SIZE] = {0};
    T_AUDIO_FILTER_BUF_STRC fbuf_strc;

	fbuf_strc.buf_in = pcm_buf;
	fbuf_strc.len_in = FILTER_INBUF_SIZE;
	fbuf_strc.buf_out = result;
	fbuf_strc.len_out = sizeof(result);
	ret = _SD_Filter_Control(filter, &fbuf_strc);
	if (ret > 0) {
		detect->parse_str = (char *)calloc(1, strlen(result));
		if (detect->parse_str) {
			/* get the result */
		    strcpy(detect->parse_str, result);
		}
		ret = AK_SUCCESS;
	} else {
	    ret = AK_FAILED;
	}

	return ret;
}

/**
 * detect_tone - detect tone wave 
 * @detect[IN/OUT]: tone wave detection control and data
 * @filter[IN]: filter handle
 * return: 0 success, -1 failed
 * notes: 
 */
static int detect_tone(struct tw_detect *detect, T_VOID *filter)
{
    int write_len = 0;
    struct wave_header whead;
	unsigned char *pcm_buf = NULL;
	pcm_user_t *user = (pcm_user_t *)detect->ai_handle;

    if (detect->debug_file_flag) {
    	detect_fp = open_wave_file(detect->debug_file);
    	write_wave_header(detect_fp, &whead, user->actual_rate);
    }

    int ret = AK_FAILED;
    struct frame frame = {0};
    int pcm_off = 0;
    struct ak_timeval cur_time;
    struct ak_timeval hold_time;
    int frame_size = calc_frame_size(user);

    pcm_buf = (unsigned char *)calloc(1, (frame_size << 1));
    if (!pcm_buf) {
        ak_print_error_ex("calloc frame size failed\n");
	    return AK_FAILED;
    }

	ak_get_ostime(&cur_time);
	ak_get_ostime(&hold_time);

	while (ret) {
		/* check time */
        ak_get_ostime(&cur_time);
	    if (ak_diff_ms_time(&cur_time, &hold_time) >= detect->detect_time) {
	        ak_print_info_ex("timeout...\n");
	        break;
	    }

        /* get the pcm data frame */
		if (ak_ai_get_frame(detect->ai_handle, &frame, 0) < 0) {
			ak_sleep_ms(10);
			continue;
		}

        /* write debug wave file */
		if (detect->debug_file_flag && detect_fp) {
    		fwrite(frame.data, 1, frame.len, detect_fp);
    		write_len += frame.len;
		}

        memcpy(&pcm_buf[pcm_off], frame.data, frame.len);
		pcm_off += frame.len;
		ak_ai_release_frame(detect->ai_handle, &frame);

        while (pcm_off >= FILTER_INBUF_SIZE) {
            if (parse_capture_pcm(detect, filter, pcm_buf)) {
                pcm_off -= FILTER_INBUF_SIZE;
                if (pcm_off > 0) {
                    memmove(pcm_buf, &pcm_buf[FILTER_INBUF_SIZE], pcm_off);
                }
            } else {
                ak_print_warning_ex("tone wave parser OK\n");
                ret = AK_SUCCESS;
                break;
            }
        }
	}

	if (pcm_buf) {
        free(pcm_buf);
        pcm_buf = NULL;
    }

    if (detect->debug_file_flag && detect_fp) {
        update_head_len(detect_fp, &whead, write_len);
    	fclose(detect_fp);
        detect_fp = NULL;
	}

	return ret;
}

/**
 * ak_tw_detect - detect tone wave from ai to analyze ssid and password.
 * @detect[IN/OUT]: tone wave detection control and data
 * return: 0 success, -1 failed
 * notes: 1. open ai with sample rate 44.1K/48K, 16 bits per sample, mono channel.
 *      2. set "debug_file_flag=1" if you want to write a debug file.
 *      3. parsed string in "parse_str" when detect success.
 */
int ak_tw_get_detect(struct tw_detect *detect)
{
    ak_print_info_ex("enter...\n");
    if (!detect) {
        return AK_FAILED;
    }
    if (!detect->ai_handle) {
        return AK_FAILED;
    }

	pcm_user_t *user = (pcm_user_t *)detect->ai_handle;

	/* init tone detection filter */
    struct pcm_param pcm_attr_in;
    pcm_attr_in.sample_rate = user->param.rate;
    pcm_attr_in.sample_bits = user->param.sample_bits;
    pcm_attr_in.channel_num = user->param.channels;
	T_VOID *filter = sdfilter_open(&pcm_attr_in, _SD_FILTER_TONE_DETECTION, user->actual_rate);
	if (!filter) {
	    ak_print_error_ex("open tone detect filter failed\n");
	    return AK_FAILED;;
	}

	int ret = detect_tone(detect, filter);

    /* close tone detection filter */
    if (filter) {
        sdfilter_close(filter);
        filter = NULL;
    }
    ak_print_info_ex("leave..., ret=%d\n", ret);

	return ret;
}

/**
 * ak_tw_release_detect - release tone wave detection info,
 *      especially the out parsed buffer.
 * @detect[IN/OUT]: tone wave detection control and data
 * return: 0 success, -1 failed
 */
int ak_tw_release_detect(struct tw_detect *detect)
{
    if(!detect) {
        ak_print_error_ex("arguments error\n");
        return AK_FAILED;
    }
    if(detect->parse_str) {
        free(detect->parse_str);
        detect->parse_str = NULL;
    }

    return AK_SUCCESS;
}

/**
 * ak_tw_get_version - get tw version
 * return: pointer to tw version string
 */
const char *ak_tw_get_version(void)
{
	return tw_version;
}
