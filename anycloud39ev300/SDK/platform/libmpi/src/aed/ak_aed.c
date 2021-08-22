#include "list.h"
#include "ak_common.h"
#include "ak_thread.h"
#include "ak_global.h"
#include "ak_ai.h"
#include "ak_aenc.h"
#include "ak_aed.h"
#include "internal_error.h"

#define MPI_AED			        "<mpi_aed>"

struct aed_ctrl_t {
	char 		init_flg;		//init flag
	char 		run_flg;		//enable flag
	char 		event_result;	//event trigger flag
	void 		*ai_handle;		//audio input handle
	int 		threshold;		//event trigger judge value
	long 		interval;		//check interval time: ms
	unsigned long long 	trigger_ts;	//trigger time: ms
	ak_aed_cb 			event_report_cb;	//callback
	ak_pthread_t 		tid;				//thread id
	ak_sem_t 			aed_sem;			//work semaphore
};

static struct aed_ctrl_t aed_ctrl = {0};
static const char aed_version[] = "libmpi_aed V1.0.00";

static unsigned long long aed_get_data_occupy_ts(struct list_head *head)
{
	unsigned long long diff_ms = 0;
	struct aenc_entry *first, *last;

	if (head) {
		first = list_first_entry(head, struct aenc_entry, list);
		last  = list_last_entry(head, struct aenc_entry, list);
		diff_ms = last->stream.ts - first->stream.ts;
	}

	return diff_ms;
}

static void aed_report(void *data, int len, unsigned long long ts)
{
	aed_ctrl.event_result = AK_TRUE;
	aed_ctrl.trigger_ts = ts;
	if (aed_ctrl.event_report_cb)
		aed_ctrl.event_report_cb(data, len);
}

/*
 * do ture aed things
 */
static int aed_calc(void *pbuf, int len, int value)
{
    short *pdata = (signed short *)pbuf;
    int count = len / 32;
    int i;
    int average = 0;
    short temp;
    int dcavr = 0;
    static int dcvalue = 0;
    static int checkcnt = 0;
	int ret = AK_FALSE;

    /* caculte direct_current value */
    if (checkcnt < 2)  {
        /* 防止开始录音打压不稳定，开始两帧不参与直流偏移的计算 */
        checkcnt ++;
    } else if (checkcnt <= 20) {
    /* 考虑到硬件速度，只让3到20帧的数据参与直流偏移的计算 */
        checkcnt ++;
        for (i = 0; i < len / 2; i++) {
            dcavr += pdata[i];
        }
        dcavr /= (signed)(len / 2);
        dcavr += dcvalue;
        dcvalue = (signed short)(dcavr / 2);
    }
    /* spot check data value */
    for (i = 0; i < count; i++) {
        temp = pdata[i * 16];
        temp = (signed short)(temp - dcvalue);
        if (temp < 0) {
            average += (-temp);
        } else {
            average += temp;
        }
    }
    average /= count;
    if (average < value) {
		//ak_print_info_ex(MPI_AED "no aed, avrg: %d, value: %d\n",
				//average, value);
        ret = AK_FALSE;
    } else {
		ak_print_info_ex(MPI_AED "aed, avrg: %d, value: %d\n",
				average, value);
        ret = AK_TRUE;
    }

	return ret;
}

static void aed_check_report(struct aenc_entry *entry)
{
	if (entry) {
		/* do aed check */
		if (aed_calc(entry->stream.data, entry->stream.len, aed_ctrl.threshold)) {
			/* event report */
			aed_report(entry->stream.data, entry->stream.len, entry->stream.ts);
		}
	}
}

static int aed_open_pcm_stream(void **aenc, void **aenc_stream)
{
	int ret = AK_FAILED;
	struct pcm_param params = {0};

	ret = ak_ai_get_params(aed_ctrl.ai_handle, &params);
	if (ret) {
		ak_print_error_ex(MPI_AED "get ai param fail, exit thread\n");
		goto exit;
	} else {
		/* should almost run to here */
		struct audio_param aenc_param = {0};

		aenc_param.type = AK_AUDIO_TYPE_PCM;
		aenc_param.sample_rate = params.sample_rate;
		aenc_param.sample_bits = params.sample_bits;
		aenc_param.channel_num = params.channel_num;

		/* open aend for pcm stream */
		*aenc = ak_aenc_open(&aenc_param);
		if (!*aenc) {
			ak_print_error_ex(MPI_AED "open aenc for type pcm failed\n");
			goto exit;
		}
		/* requst pcm stream */
		*aenc_stream = ak_aenc_request_stream(aed_ctrl.ai_handle, *aenc);
		if (!*aenc_stream) {
			ak_print_error_ex(MPI_AED "request pcm failed\n");
			ak_aenc_close(*aenc);
			*aenc = NULL;
			goto exit;
		}
		ret = AK_SUCCESS;
	}
exit:
	return ret;
}

static void *aed_thread(void *arg)
{
	/* set thread information */
	long int tid = ak_thread_get_tid();
	ak_print_normal_ex(MPI_AED "thread id : %ld\n", tid);
	ak_thread_set_name("aed_thread");

	while (aed_ctrl.init_flg) {
		/* wait action signal */
		ak_thread_sem_wait(&aed_ctrl.aed_sem);
		/* start aenc pcm encode to get pcm data */
		void *aenc_pcm = NULL;
		void *aenc_stream = NULL;

		if (aed_open_pcm_stream(&aenc_pcm, &aenc_stream))
			break;
		/*
		 * all prepare for audio pcm stream ready,
		 * now can get pcm stream and do audio event detection
		 */
		struct list_head stream_list;
		INIT_LIST_HEAD(&stream_list);

		/* run aed work */
		while (aed_ctrl.run_flg) {
			static unsigned long long last_check_ts = 0;
			unsigned long long diff_ms = 0, sleep_time = 0;
			int interval_ms = aed_ctrl.interval;

			if (interval_ms <= 0)
				interval_ms = 500;	/* default 500ms check once time */

			/* get raw pcm data */
			if (!ak_aenc_get_stream(aenc_stream, &stream_list)) {
				/* data occupy time calculate */
				diff_ms = aed_get_data_occupy_ts(&stream_list);

				struct aenc_entry *entry, *n;
				/* to iterate list */
				list_for_each_entry_safe(entry, n, &stream_list, list) {
					/* time pass calculate, -5 is to fix average error */
					if ((entry->stream.ts - last_check_ts) >= (interval_ms - 5)) {
						last_check_ts = entry->stream.ts;
						/* do detection */
						aed_check_report(entry);
					}
					/* use ok, release resource */
					ak_aenc_release_stream(entry);
				}
				/* for set detection interval */
				sleep_time = interval_ms - diff_ms % interval_ms;
				ak_sleep_ms(sleep_time);
			} else {
				ak_sleep_ms(interval_ms);
			}
		}
		if (aenc_stream) {
			ak_aenc_cancel_stream(aenc_stream);
			aenc_stream = NULL;
			ak_aenc_close(aenc_pcm);
			aenc_pcm = NULL;
		}
	}
	/* thread exit */
	ak_thread_exit();
	return NULL;
}

/**
 * ak_aed_set_interval - set detection interval time
 * ms[IN]: detection interval time in micro secord
 * return: 0 success, -1 failed
 */
int ak_aed_set_interval(long interval)
{
	int ret = AK_FAILED;

	if (aed_ctrl.init_flg) {
		aed_ctrl.interval = interval;
		ret = AK_SUCCESS;
	} else {
		ak_print_error_ex(MPI_AED "please init aed module first\n");
		set_error_no(ERROR_TYPE_NOT_INIT);
	}

	return ret;
}

/**
 * ak_aed_get_result - get aed result
 * trigger_ts[OUT]: store trigger time, ms
 * return: 1 -> event trigger; otherwise -> no event
 */
int ak_aed_get_result(unsigned long long *trigger_ts)
{
	int result = AK_FALSE;

	/* check param */
	if (!trigger_ts) {
		ak_print_error_ex(MPI_AED "invalid trigger_ts\n");
		set_error_no(ERROR_TYPE_INVALID_ARG);
		return result;
	}
	/* flag check */
	if (aed_ctrl.init_flg && aed_ctrl.run_flg) {
		/* set return value */
		result = aed_ctrl.event_result;
		*trigger_ts = aed_ctrl.trigger_ts;

		/* clean inner valure */
		if (aed_ctrl.event_result) {
			aed_ctrl.event_result = AK_FALSE;
			aed_ctrl.trigger_ts = 0;
		}
	} else {
		ak_print_error_ex(MPI_AED "please init and trigger aed module first\n");
		set_error_no(ERROR_TYPE_NOT_INIT);
	}

	return result;
}

/**
 * ak_aed_enable - set aed param
 * @param[IN]: aed param, more detail see ak_aed.h
 * return: 0 - success; otherwise -1;
 */
int ak_aed_set_param(struct ak_aed_param *param)
{
	/* check param */
	if (!param) {
		ak_print_error_ex(MPI_AED "invalid param\n");
		set_error_no(ERROR_TYPE_INVALID_ARG);
		return AK_FAILED;
	}

	/* check module init */
	if (!aed_ctrl.init_flg) {
		ak_print_error_ex(MPI_AED "please init and trigger aed module first\n");
		set_error_no(ERROR_TYPE_NOT_INIT);
		return AK_FAILED;
	}

	/* set valur */
	aed_ctrl.event_report_cb = param->aed_cb;
	aed_ctrl.interval = param->interval;
	aed_ctrl.threshold = param->threshold;

	return AK_SUCCESS;
}

/**
 * ak_aed_enable - start or stop aed.
 * @enable[IN]:  [0,1],  0 -> stop aed; 1 -> start aed
 * return: 0 - success; otherwise -1;
 */
int ak_aed_enable(int enable)
{
	/* check module init */
	if (!aed_ctrl.init_flg) {
		set_error_no(ERROR_TYPE_NOT_INIT);
		ak_print_error_ex(MPI_AED "fail, no init\n");
		return AK_FAILED;
	}

	ak_print_normal_ex(MPI_AED "enable: %d\n", enable);
	if (enable)
		aed_ctrl.run_flg = AK_TRUE;
	else
		aed_ctrl.run_flg = AK_FALSE;

	/* send signal to start aed */
	ak_thread_sem_post(&aed_ctrl.aed_sem);

	return AK_SUCCESS;
}


/*
 * ak_aed_init - audio event detection init
 * ai_handle[IN]: auido input handle
 * return: AK_SUCCESS on success, AK_FAILED on failed
 */
int ak_aed_init(void *ai_handle)
{
	int ret = AK_FAILED;

	/* check param */
	if (!ai_handle) {
		set_error_no(ERROR_TYPE_POINTER_NULL);
		ak_print_error_ex(MPI_AED "invalid handle\n");
		return ret;
	}

	/* incase of reentry */
	if (aed_ctrl.init_flg) {
		ak_print_error_ex(MPI_AED "auido event detection has opened\n");
		set_error_no(ERROR_TYPE_EBUSY);
		return ret;
	} else {
		aed_ctrl.init_flg = AK_TRUE;
		ak_print_normal_ex(MPI_AED "init audio event detection\n");
	}

	/* set param */
	aed_ctrl.ai_handle = ai_handle;
	ak_thread_sem_init(&aed_ctrl.aed_sem, 0);

	/* create module main thread */
	ret = ak_thread_create(&aed_ctrl.tid, aed_thread, NULL,
			ANYKA_THREAD_NORMAL_STACK_SIZE, -1);
	if (ret) {
		aed_ctrl.init_flg = AK_FAILED;
		ak_thread_sem_destroy(&aed_ctrl.aed_sem);
	}

	return ret;
}

/*
 * ak_aed_exit - exit audio event detection
 * return: AK_SUCCESS on success, AK_FAILED on failed
 */
int ak_aed_exit(void)
{
	int ret = AK_SUCCESS;

	ak_print_info_ex(MPI_AED "entry ...\n");
	/* destroy resources */
	if (aed_ctrl.init_flg) {
		aed_ctrl.run_flg = 0;
		aed_ctrl.init_flg = 0;
		ak_thread_sem_post(&aed_ctrl.aed_sem);
   		ret = ak_thread_join(aed_ctrl.tid);
		ak_thread_sem_destroy(&aed_ctrl.aed_sem);
		ak_print_info_ex(MPI_AED "leave ...\n");
	}

	return ret;
}

/*
 * ak_aed_get_version - get version
 * return: pointer to version string
 */
const char *ak_aed_get_version(void)
{
	return aed_version;
}
