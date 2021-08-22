/**
* Copyright (C) 2018 Anyka(Guangzhou) Microelectronics Technology CO.,LTD.
* File Name: ak_ai_demo.c
* Description: This is a simple example to show how the AI module working.
* Notes:
* History: V1.0.1
*/
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "ak_common.h"
#include "ak_ai.h"


/* 
 * check_dir: check whether the 'path' was exist.
 * path[IN]: pointer to the path which will be checking.
 * return: 1 on exist, 0 is not.
 */
static int check_dir(const char *path)
{
	struct stat buf = {0};

	if (NULL == path)
		return 0;

	stat(path, &buf);
	if (S_ISDIR(buf.st_mode)) {
		return AK_TRUE;
	} else {
		return AK_FALSE;
	}
}

/* 
 * create_pcm_file_name: create pcm file by path+date.
 * path[IN]: pointer to the path which will be checking.
 * file_path[OUT]: pointer to the full path of pcm file.
 * return: void.
 */
static void create_pcm_file_name(const char *path, char *file_path)
{
	if (AK_FALSE == check_dir(path)) {
		return;
	}
	
	char time_str[20] = {0};
	struct ak_date date;

	/* get the file path */
	ak_get_localdate(&date);
	ak_date_to_string(&date, time_str);
    sprintf(file_path, "%s%s.pcm", path, time_str);
}

/* 
 * open_pcm_file: open pcm file.
 * path[IN]: pointer to the path which will be checking.
 * fp[OUT]: pointer of opened pcm file.
 * return: void.
 */
static void open_pcm_file(const char *path, FILE **fp)
{
	/* create the pcm file name */
	char file_path[255];
	create_pcm_file_name(path, file_path);
	
	/* open file */
	*fp = fopen(file_path, "w+b");
	if (NULL == *fp) {
		ak_print_normal("open pcm file: %s error\n", file_path);
	} else {
		ak_print_normal("open pcm file: %s OK\n", file_path);
	}
}

/* 
 * close_pcm_file: close pcm file.
 * fp[IN]: pointer of opened pcm file.
 * return: void.
 */
static void close_pcm_file(FILE *fp)
{
    if (NULL != fp) {
    	fclose(fp);
    	fp = NULL;
    }
}

/*
 * ai_capture_loop: loop to get and release pcm data, between get and release,
 *                  here we just save the frame to file, on your platform,
 *                  you can rewrite the save_function with your code.
 * ai_handle[IN]: pointer to ai handle, return by ak_ai_open()
 * path[IN]: save directory path, if NULL, will not save anymore.
 * save_time[IN]: captured time of pcm data, unit is second.
 */
static void ai_capture_loop(void *ai_handle, const char *path, int save_time)
{
	unsigned long long start_ts = 0;// use to save capture start time
	unsigned long long end_ts = 0;	// the last frame time
	struct frame frame = {0};

	/* open pcm file */
	FILE *fp = NULL;
	open_pcm_file(path, &fp);

	/*
	 * To get frame by loop
	 */
	ak_print_normal("*** capture start ***\n");
	
	while (AK_TRUE) {
		/* get the pcm data frame */
		if (ak_ai_get_frame(ai_handle, &frame, 0) < 0) {
			ak_sleep_ms(10);
			continue;
		}

		/* 
		* Here, you can implement your code to use this frame.
		* Notice, do not occupy it too long.
		*/

		/* 
		 * TODO: Write your code here to replace save the data to the file 
		 * to implement your things.
		 */
		if (NULL != fp) {
			/* write frame to file */
			if(fwrite(frame.data, frame.len, 1, fp) < 0) {
	    		ak_print_normal("write file error.\n");
	    		break;
	    	}
	    }

		/* save the begin time */
		if (0 == start_ts) {
			start_ts = frame.ts;
			end_ts = frame.ts;
		}
		end_ts = frame.ts;

		/* 
		* in this context, this frame was useless,
		* release frame data
		*/
		ak_ai_release_frame(ai_handle, &frame);

		/* time is up */
		if ((end_ts - start_ts) >= save_time) {
			break;
		}
	}

	/* close file handle */
	close_pcm_file(fp);

	ak_print_normal("*** capture finish ***\n\n");
}


/**
 * Preconditions:
 * 1��TF card is already mounted
 * 2��your main audio progress must stop
 */
int main(int argc, char **argv)
{
    ak_print_normal("*****************************************\n");
	ak_print_normal("** ai demo version: %s **\n", ak_ai_get_version());
    ak_print_normal("*****************************************\n");
	
	/* 
	 * step 0: global value initialize
	 */
	int ret = AK_FAILED;
	int volume = 7;					// set volume,volume is from 0~12
	const int save_time = 20000;		// set save time(ms)
	const char *save_path = "/mnt/";	// set save path

	struct pcm_param param;
	memset(&param, 0, sizeof(struct pcm_param));

	param.sample_rate = 8000;				// set sample rate 
	param.sample_bits = 16;					// sample bits only support 16 bit 
	param.channel_num = AUDIO_CHANNEL_MONO;	// channel number 


	/* 
	 * step 1: open ai
	 */
    void *ai_handle = ak_ai_open(&param);
    if (NULL == ai_handle) {
    	ak_print_normal("*** ak_ai_open failed. ***\n");
    	goto exit;
    }


    /* 
	 * step 2: set ai working configuration,
	 * configuration include:
	 * frame interval, source, volume, 
	 * enable noise reduction and automatic gain control,
	 * resample, clear frame buffer
	 */
	ret = ak_ai_set_frame_interval(ai_handle, AUDIO_DEFAULT_INTERVAL);
	if (ret) {
		ak_print_normal("*** set ak_ai_set_frame_interval failed. ***\n");
		ak_ai_close(ai_handle);
		goto exit;
	}

	/* set source, source include mic and linein */
	ret = ak_ai_set_source(ai_handle, AI_SOURCE_MIC);
	if (ret) {
		ak_print_normal("*** set ak_ai_open failed. ***\n");
		ak_ai_close(ai_handle);
		goto exit;
	}

	/* volume is from 0 to 12,volume 0 is mute */
	ak_ai_set_volume(ai_handle, volume);

	/* enable noise reduction and automatic gain control ,
	 * nr&agc only support 8K sample 
	 */
	ret = ak_ai_set_nr_agc(ai_handle, AUDIO_FUNC_ENABLE);
	if (ret) {
		ak_print_normal("*** set ak_ai_set_nr_agc failed. ***\n");
		ak_ai_close(ai_handle);
		goto exit;
	}

	/* set resample */
	ret = ak_ai_set_resample(ai_handle, AUDIO_FUNC_DISABLE);
	if (ret) {
		ak_print_normal("*** set ak_ai_set_resample  failed. ***\n");
		ak_ai_close(ai_handle);
		goto exit;
	}
	
	/* clear ai buffer */
	ret = ak_ai_clear_frame_buffer(ai_handle);
	if (ret) {
		ak_print_normal("*** set ak_ai_clear_frame_buffer failed. ***\n");
		ak_ai_close(ai_handle);
		goto exit;
	}


	/* 
	 * step 3: start capture frames
	 */
	ret = ak_ai_start_capture(ai_handle);
	if (ret) {
		ak_print_normal("*** ak_ai_start_capture failed. ***\n");
		ak_ai_close(ai_handle);
		goto exit;
	}


	/* 
	 * step 4: start to capture and save pcm data 
	 */
    ai_capture_loop(ai_handle, save_path, save_time);


	/*
	 * step 5: stop capture frames
	 */
	ret = ak_ai_stop_capture(ai_handle);
	if (ret) {
		ak_print_normal("*** ak_ai_stop_capture failed. ***\n");
		ak_ai_close(ai_handle);
		goto exit;
	}


	/*
	 * step 6: close ai
	 */
    ret = ak_ai_close(ai_handle);
	if (ret) {
		ak_print_normal("*** ak_ai_close failed. ***\n");
	}

exit:
	/* exit ai demo */
	ak_print_normal("******** exit ai demo ********\n");

	return ret;
}
