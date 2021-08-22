/**
 * 这段代码的目的是用来演示：设备向手Q发送视频流
 *
 * 首先确保手Q与设备已经绑定，在on_online_status回调函数中判断设备是否登录成功，
 * 在登录成功后调用tx_start_av_service启动SDK的音视频服务，这个函数会设置一些回调，
 * 例如：开始采集视频数据，开始采集音频数据，停止采集视频数据等。
 * 然后SDK内部会启动一个线程处理这些逻辑，这个线程会一直运行直到进程结束。
 * 在手Q向设备请求视频数据时：SDK收到服务器下发的信令，将调用test_start_camera回调函数，
 * 开发者在这个函数中向SDK塞入视频数据，SDK将通过服务器中转（或者打洞直连）的方式将视频数据发送给手Q
 *
 * 为了方便演示，这个例子的视频流从视频文件中获取，不断的从文件中读取数据，通过tx_set_video_data函数发送给对应的手Q。
 * 当然您也可以同时在test_start_mic中通过tx_set_audio_data函数发送音频数据。
 *
 * 期望的结果是：
 * 手Q上有设备发送过来的视频画面
 */

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>

#include "TXSDKCommonDef.h"
#include "TXAudioVideo.h"


/**
 * 为了简单表示，这里只测试一个视频文件
 */
/********************************************************************/
static FILE *fstream = NULL;
static char* s_cData = NULL;
static bool s_bstart = false;

static pthread_t thread_enc_id = 0;
static unsigned long s_dwTotalFrameIndex = 0;

unsigned long _GetTickCount() {
    struct timeval current = {0};
	gettimeofday(&current, NULL);
	return (current.tv_sec*1000 + current.tv_usec/1000);
}

/**
 *  该线程不断的从文件中读取数据，通过tx_set_video_data发送视频数据
 */
static void* EncoderThread(void* pThreadData) {
	while(s_bstart) {

		if(s_bstart) {

			if(fstream &&!feof(fstream)) {
				int iLen = 0;
				int iType = -1;
				fread(&iLen, 1, 4, fstream);
				fread(&iType, 1, 4, fstream);

				if (iLen > 0) {
					fread(s_cData, 1, iLen, fstream);

					static int s_nFrameIndex = 0;
					static int s_gopIndex = -1;

					unsigned char * pcEncData = (unsigned char *)s_cData;
					int nEncDataLen = iLen;
					int nFrameType = 1;
					if(iType == 0) {
						s_nFrameIndex = 0;
						s_gopIndex++;
						nFrameType = 0;
					}
					else {
						s_nFrameIndex++;
						nFrameType = 1;
					}

					if(s_gopIndex == -1) {
						printf("No I Frame s_gopIndex == -1\n");
					}
					else {
						if(nEncDataLen != 0) {
							// 发送视频数据，SDK内部一个线程负责将数据发送给对方
							tx_set_video_data(pcEncData, nEncDataLen, nFrameType, (int)_GetTickCount(), s_gopIndex, s_nFrameIndex, (int)s_dwTotalFrameIndex++, 40);
						}
					}
				}

			}
			else if(fstream && feof(fstream)) {
				fclose(fstream);
				fstream = fopen("test.264", "rb");
			}
		}

		usleep(90000);//90ms
	}

    return 0;
}

/**
 * 开始采集视频的回调
 * 通知摄像头视频链路已经建立，可以通过 tx_set_video_data接口向 AV SDK 填充采集到的视频数据
 * 为了测试，这里是启动了一个线程从文件中读取数据并发送，实际应用中可以启动一个线程去实时采集
 */
bool test_start_camera() {
	printf("###### test_start_camera ###################################### \n");
	if (!fstream) {
		fstream = fopen("test.264", "rb");
	}

	if (!s_cData) {
		s_cData = (char*)malloc(1280*720);
	}

	if (!fstream || !s_cData) {
		return false;
	}

	s_bstart = true;
	int ret = pthread_create(&thread_enc_id, NULL, EncoderThread, NULL);
	if (ret || !thread_enc_id) {
		s_bstart = false;
		return false;
	}

    return true;
}

/**
 * 停止采集视频的回调
 * 通知摄像头视频链路已经断开，可以不用再继续采集视频数据。
 */
bool test_stop_camera() {
	printf("###### test_stop_camera ###################################### \n");
	s_bstart = false;
	if(fstream) {
		fclose(fstream);
		fstream = NULL;
	}

	if(s_cData) {
		free(s_cData);
		s_cData = NULL;
	}

	if(thread_enc_id !=0) {
        pthread_join(thread_enc_id,NULL);
        thread_enc_id = 0;
    }

    return true;
}

/**
 * 视频码率意味着1s产生多少数据，这个参数跟网络带宽的使用直接相关
 * AV SDK 会根据当前的网络情况和Qos信息，给出建议的bitrate，
 * 上层应用可以根据这个建议值设置Camera的各项参数，如帧率、分辨率，量化参数等，从而获得合适的码率
 */
bool test_set_bitrate(int bit_rate) {
    printf("###### test_set_bitrate  ##################################### %d \n", bit_rate);
	return true;
}

/**
 * 如果I帧丢了，那么发再多的P帧也没有多大意义，AV SDK 会在发现这种情况发生的时候主动通知上层重新启动一个I帧
 */
bool test_restart_gop() {
	printf("###### test_restart_gop ###################################### \n");
    return true;
}

/**
 * 开始采集音频的回调
 * 通知麦克风音频链路已经建立，可以通过 tx_set_audio_data 接口向 AV SDK 填充采集到的音频数据
 * 这里只测试视频，所以这里是空实现
 */
bool test_start_mic() {
	printf("###### test_start_mic ###################################### \n");
	return true;
}

/**
 * 停止采集音频的回调
 * 通知摄像头音频链路已经断开，可以不用再继续采集音频数据
 * 这里只测试视频，所以这里是空实现
 */
bool test_stop_mic() {
	printf("###### test_stop_mic ######################################\n");
	return true;
}

/*
 * 智能设备作为音视频的接收方，下面回调用于接收音视频数据
 * 注意：on_recv_audiodata回调出来的tx_audio_encode_param参数里面包含了音频的编码参数
 */
void test_recv_audiodata(tx_audio_encode_param *param, unsigned char *pcEncData, int nEncDataLen)
{
    printf("##### test_recv_audiodata ######################################\n");
}



