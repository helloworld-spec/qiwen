#include "ak_thread.h"
#include "ak_common.h"

#include "ja_md.h"
#include "ak_md.h"
#include "ak_onvif_config.h"


#define MD_H_NUM			8
#define MD_V_NUM			8

#define MD_FILE_PATH	"/etc/jffs2/md.conf"


typedef struct
{
    ak_pthread_t			md_tid;		//md thread id
	int						bRunning;	//md tread work flag
	P_MD_ALARM_CALLBACK 	*pfunc;		//alarm callback function
	ak_mutex_t     			filelock;	//n1 md stuct save to file ,or read form file, need lock
	void					*vi_handle;	//opened vi handle
}MD_CTRL;

static MD_CTRL md_ctrl = {0};

/**
 * ja_md_load - load md struct from file
 * @pmotion[OUT]: md struct
 * return: 0 - success, fail return -1.
 */
int ja_md_load(struct NK_MotionDetection* pmotion)
{
	int ret = AK_FAILED;
	FILE *fd = NULL;
	int len = 0;

	if (NULL == pmotion) {
		ak_print_error_ex("param NULL!\n");
		return ret;
	}

	ak_thread_mutex_lock(&md_ctrl.filelock);

	fd = fopen (MD_FILE_PATH, "rb");
	if (NULL == fd) {
		ak_print_error_ex("fd open failed!\n");
		ak_thread_mutex_unlock(&md_ctrl.filelock);
		return ret;
	}

	len = fread(pmotion, 1, sizeof(struct NK_MotionDetection), fd);
	if (len == sizeof(struct NK_MotionDetection))
	{
		ret = AK_SUCCESS;
	}
	else
	{
		ak_print_error_ex("read len err : %d, %d!\n", len, sizeof(struct NK_MotionDetection));
	}

	fclose(fd);

	ak_thread_mutex_unlock(&md_ctrl.filelock);

	return ret;
}

/**
 * ja_md_store - store md struct to file
 * @pmotion[IN]: md struct
 * return: 0 - success, fail return -1.
 */
int ja_md_store(struct NK_MotionDetection* pmotion)
{
	int ret = AK_FAILED;
	FILE *fd = NULL;
	int len = 0;

	if (NULL == pmotion)
	{
		ak_print_error_ex("param NULL!\n");
		return ret;
	}

	ak_thread_mutex_lock(&md_ctrl.filelock);

	fd = fopen (MD_FILE_PATH, "w+b");

	if (NULL == fd)
	{
		ak_print_error_ex("fd open failed!\n");
		ak_thread_mutex_unlock(&md_ctrl.filelock);
		return ret;
	}

	len = fwrite(pmotion, 1, sizeof(struct NK_MotionDetection), fd);


	if (len == sizeof(struct NK_MotionDetection))
	{
		ret = AK_SUCCESS;
	}
	else
	{
		ak_print_error_ex("write len err : %d, %d!\n", len, sizeof(struct NK_MotionDetection));
	}

	fclose(fd);

	ak_thread_mutex_unlock(&md_ctrl.filelock);


	return ret;
}

/**
 * ja_md_set_move_ratio - set md ratio according to md struct
 * @pmotion[IN]: md struct
 * return: void
 */
void ja_md_set_move_ratio(struct NK_MotionDetection* pmotion)
{
	unsigned short i, j, k, cnt = 0,ratios = 1;
	int Sensitivity[MD_H_NUM * MD_V_NUM];
	struct onvif_alarm_config *alarm = onvif_config_get_alarm();

	ak_print_normal_ex("%d, %d, %d, %d, %d\n", alarm->md_level_1,
		alarm->md_level_2, alarm->md_level_3, alarm->md_level_4, alarm->md_level_5);

	if ((alarm->md_level_1 < 1) || (alarm->md_level_1 > 100))
		alarm->md_level_1 = 10;
	if ((alarm->md_level_2 < 1) || (alarm->md_level_2 > 100))
		alarm->md_level_2 = 30;
	if ((alarm->md_level_3 < 1) || (alarm->md_level_3 > 100))
		alarm->md_level_3 = 50;
	if ((alarm->md_level_4 < 1) || (alarm->md_level_4 > 100))
		alarm->md_level_4 = 70;
	if ((alarm->md_level_5 < 1) || (alarm->md_level_5 > 100))
		alarm->md_level_5 = 90;

	switch (pmotion->SensitivityLevel.val)
	{
	case 100:
		ratios = alarm->md_level_5;	//ratios [1, 100], larger value means more sensitve
		break;

	case 90:
		ratios = alarm->md_level_4;
		break;

	case 80:
		ratios = alarm->md_level_3;
		break;

	case 70:
		ratios = alarm->md_level_2;
		break;

	case 60:
		ratios = alarm->md_level_1;
		break;

	default:
		ratios = alarm->md_level_3;
		break;
	}
	
	/*
	 *把32*24的区域转换成8*8的，12小块合并为1大块，若该大块的12小块中有6个及
	 *以上被选，则认为该大块被选。
	 */
	for(i = 0; i < MD_H_NUM * MD_V_NUM; i++) {
		cnt = 0;    	
		for (j=(i/MD_H_NUM)*(24/MD_V_NUM); j<(i/MD_H_NUM)*(24/MD_V_NUM)+(24/MD_V_NUM); j++) {			
			for (k=(i%MD_H_NUM)*(32/MD_H_NUM); k<(i%MD_H_NUM)*(32/MD_H_NUM)+(32/MD_H_NUM); k++)	{				
				if (pmotion->Mask.matrix[j][k])	{					
					cnt++;				
				}			
			}
		}

		if (cnt >= 6) {			
			Sensitivity[i] = ratios;		
		} else {			
			Sensitivity[i] = 1;		
		}
	}

	for (i=0; i<MD_H_NUM * MD_V_NUM; i++) {		
		if (Sensitivity[i] >= ratios) {			
			ak_print_normal("O ");		
		} else {			
			ak_print_normal(". ");		
		}		

		if (7 == i % MD_H_NUM) {			
			ak_print_normal("\n");	
		}	
	}

	ak_md_set_area_sensitivity(MD_H_NUM, MD_V_NUM, Sensitivity);
}

/**
 * ja_md_thread - md check thread
 * @arg[IN]: 
 * return: void
 */
void* ja_md_thread(void *arg)
{
	int md_time = 0;		/*calendar time ,second*/

	ak_print_normal_ex("md thread id: %ld\n", (long int)ak_thread_get_tid());

	do{
		if(1 == ak_md_get_result(&md_time, NULL, 0))
		{
			if (NULL != md_ctrl.pfunc)
				md_ctrl.pfunc();
		}

		ak_sleep_ms(1000);
	}while(md_ctrl.bRunning);

	ak_print_normal_ex("### md thread id: %ld exit ###\n", (long int)ak_thread_get_tid());
	pthread_exit(NULL);
	return NULL;
}


/**
 * ja_md_start_movedetection - start md
 * return: void
 */
void ja_md_start_movedetection(void)
{
    struct NK_MotionDetection motion;

	md_ctrl.bRunning = AK_FALSE;
    ja_md_load(&motion);
    ak_print_normal_ex("[%s]enable=%d, SensitivityLevel=%d\n", __func__, motion.Enabled.val, motion.SensitivityLevel.val);

    if (motion.Enabled.val == 0)
    {
        ak_print_normal_ex("motion detection disabled!!\n");
        return;
    }

	ak_md_init(md_ctrl.vi_handle);
	ja_md_set_move_ratio(&motion);
	ak_md_enable(1);
	md_ctrl.bRunning = AK_TRUE;

	ak_thread_create(&md_ctrl.md_tid, ja_md_thread, NULL, ANYKA_THREAD_MIN_STACK_SIZE, -1);
}

/**
 * ja_md_stop_movedetection - stop md
 * return: void
 */
void ja_md_stop_movedetection(void)
{
	md_ctrl.bRunning = AK_FALSE;
	ak_thread_join(md_ctrl.md_tid);

	ak_md_enable(0);
	ak_md_destroy();
}

/**
 * ja_md_init - init md module
 * @vi[IN]: opened vi handle
 * @pcallback[IN]: alarm callback function
 * return: void
 */
void ja_md_init(void *vi, P_MD_ALARM_CALLBACK pcallback)
{
	struct NK_MotionDetection motion;
	ak_thread_mutex_init(&md_ctrl.filelock, NULL);

	md_ctrl.pfunc = pcallback;
	md_ctrl.vi_handle = vi;
	
	if (AK_SUCCESS != ja_md_load(&motion)) {
		/*md enable default value*/
		motion.Enabled.val = NK_FALSE;
		motion.SensitivityLevel.val = 80;
		motion.SensitivityLevel.min = 0;
		motion.SensitivityLevel.max = 100;
		motion.Mask.width = 32;
		motion.Mask.height = 24;
		int i, j;

		/*md areas, default select all areas*/
		for (i = 0; i < motion.Mask.height; ++i) {
			for (j = 0; j < motion.Mask.width; ++j) {
				motion.Mask.matrix[i][j] = NK_True;
			}
		}
		ja_md_store(&motion);
	}
    ja_md_start_movedetection();
}

/**
 * ja_md_destroy - destroy md module
 * return: void
 */
void ja_md_destroy(void)
{
	if (md_ctrl.bRunning)
		ja_md_stop_movedetection();

	ak_thread_mutex_destroy(&md_ctrl.filelock);
}

