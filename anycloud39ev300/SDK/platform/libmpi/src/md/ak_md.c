#include <stdlib.h>
#include <string.h>
#include "internal_error.h"
#include "ak_global.h"
#include "ak_common.h"
#include "ak_thread.h"
#include "ak_vi.h"
#include "ak_venc.h"
#include "ak_vpss.h"
#include "ak_md.h"

#define MPI_MD			    "<mpi_md>"

#define DEFAULT_DIFF 		12000
#define BASE_DIFF 		    5000	//threshold for global mode to judge if one block has move
#define MAX_DIFF			65535
#define MAX_AREA_NUM 		512

enum md_thread_stat{
	MD_THREAD_EXIT = 0,
	MD_THREAD_STOP ,
	MD_THREAD_RUN ,
};

struct hw_md_result {
	ak_mutex_t lock;
	char global;
	char *area;
	char *reserved;
	struct ak_date detect_time;   //when does move detect find?
};

struct hw_md_ctrl {
	int thread_stat;
	ak_pthread_t md_tid;
	ak_sem_t run_sem;		//notify semaphore
	ak_sem_t send_sem;
	void *vi_handle;
	int md_fps;	//msecond. interval time data to detect
	int global_sens;	//global sensivity
	int area_md_flag;	//area mode flag
	unsigned short *area_diff;
	int h_num;
	int v_num;
	int v_size_max;
} ;

static const char *hw_md_version = "libmpi_md V1.0.01";
static struct hw_md_result md_result;
static struct hw_md_ctrl md_ctrl ;
static int md_init_flag = 0;

/**
 * ak_hw_md_get_stat: get motion detection stat params
 * @vi_handle[IN]: vi module handle
 * @md[OUT]: md params
 * return: 0 success, -1 failed
 * notes:
 */
static int md_get_stat(const void *vi_handle, struct vpss_md_info *md)
{
	if (NULL == vi_handle) {
		ak_print_error_ex(MPI_MD "vi_handle is NULL\n");
		return -1;
	}

	/*get isp 3d stat info*/
	int ret = ak_vpss_md_get_stat(vi_handle, md);
	if (0 != ret) {
		ak_print_error_ex(MPI_MD "get 3d nr stat info fail\n");
	}

	return ret;
}

/**
 * md_area_judge: judge function for area mode
 * @md[IN]: md params
 * return: 0 no move, 1 has move
 * notes:
 */
static int md_area_judge(struct vpss_md_info *md)
{
	int i, j, v, h;
	int v_multiple = md_ctrl.v_size_max / md_ctrl.v_num;
	int h_multiple = VPSS_MD_DIMENSION_H_MAX / md_ctrl.h_num;
	int md_trigged = 0;
	unsigned short *stat;
	
#if 0
	unsigned int area_stat;

	/* usr have been redefined area, we should caculate area again */
	for(i = 0; i < md_ctrl.v_num; i++) {
		for(j = 0; j < md_ctrl.h_num; j++) {
			area_stat = 0;
			stat = &(md->stat[i * v_multiple][j * h_multiple]);
			for(v = 0; v < v_multiple; v++)
				for(h = 0; h < h_multiple; h++)
					area_stat += *(stat + v * VPSS_MD_DIMENSION_H_MAX + h);

			area_stat = area_stat / (v_multiple * h_multiple);
			if(area_stat > *(md_ctrl.area_diff + i * md_ctrl.h_num + j)){
				*(md_result.area + i * md_ctrl.h_num + j) = 1;
				md_trigged = 1;
			}
		}
	}
#else
	int cnt, cnt_right;
	int v_num_ex = v_multiple + v_multiple * 2 / 3;
	//int flag = 0;

	/* usr have been redefined area, we should caculate area again */
	for(i = 0; i < md_ctrl.v_num; i++) {
		for(j = 0; j < md_ctrl.h_num; j++) {
			cnt = 0;
			cnt_right = 0;
			
			if (i < md_ctrl.v_num - 1) {
				stat = &(md->stat[i * v_multiple][j * h_multiple]);				
			} else {
				stat = &(md->stat[(i+1) * v_multiple - v_num_ex][j * h_multiple]);
			}

			for(v = 0; v < v_num_ex; v++) {
				for(h = 0; h < h_multiple; h++) {
					if (*(stat + v * VPSS_MD_DIMENSION_H_MAX + h) > *(md_ctrl.area_diff + i * md_ctrl.h_num + j))
						cnt++;
				}
			}

			if ((h_multiple >= 2) && (j < md_ctrl.h_num - 1)) {
				if (i < md_ctrl.v_num - 1) {
					stat = &(md->stat[i * v_multiple][j * h_multiple + h_multiple / 2]);		
				} else {
					stat = &(md->stat[(i+1) * v_multiple - v_num_ex][j * h_multiple + h_multiple / 2]);
				}
				
				for(v = 0; v < v_num_ex; v++) {
					for(h = 0; h < h_multiple; h++) {
						if (*(stat + v * VPSS_MD_DIMENSION_H_MAX + h) > *(md_ctrl.area_diff + i * md_ctrl.h_num + j))
							cnt_right++;
					}
				}
			}
				
			if((cnt > v_num_ex * h_multiple * 3 / 4) 
				|| (cnt_right > v_num_ex * h_multiple * 3 / 4)){
				*(md_result.area + i * md_ctrl.h_num + j) = 1;
				md_trigged = 1;
			}
		}
	}
#endif

	if(md_trigged){
		ak_thread_mutex_lock(&md_result.lock);
		memcpy(md_result.reserved,md_result.area,md_ctrl.h_num * md_ctrl.v_num);
		memset(md_result.area, 0, md_ctrl.h_num * md_ctrl.v_num);
		ak_thread_mutex_unlock(&md_result.lock);
		return 1;
	}

	return 0;
}

/**
 * md_global_judge: judge function for global mode
 * @md[IN]: md params
 * return: 0 no move, 1 has move
 * notes:
 */
static int md_global_judge(struct vpss_md_info *md)
{
	int i, j;
	int md_cnt = 0;

	for(i = 0; i < md_ctrl.v_size_max; i++) {
		for(j = 0; j < VPSS_MD_DIMENSION_H_MAX; j++) {
			if(md->stat[i][j] > BASE_DIFF)
				md_cnt++;
		}
	}

	if (md_cnt > (101 - md_ctrl.global_sens )){
		return 1;
	}

	return 0;
}

/**
 * md_check: judge function
 * @md_ctrl[IN]: md ctrl struct pointer
 * return: 0 no move, 1 has move
 * notes:
 */
static int md_check(struct hw_md_ctrl *md_ctrl)
{
	int ret = 0;
	struct vpss_md_info md = {0};

	if(md_get_stat(&md_ctrl->vi_handle, &md)){
		ak_print_warning_ex(MPI_MD "get md stat fail.\n");
		return 0;
	}
	if(md_ctrl->area_md_flag){
		ret = md_area_judge(&md);
	}else{
		ret = md_global_judge(&md);
	}

	return ret;
}

/**
 * submit - get md result.
 * @md_sec[OUT]:  md trigger time,calendar time,second.
 * @area_md[OUT]:	if enable area md, output area md result.
 * return: 1 - md trigger; 0 - no md;  -1  - fail;
 */
int get_md_result( int *md_sec, char *area_md)
{
	ak_thread_mutex_lock(&md_result.lock);
	/* return area md result */
	if(md_ctrl.area_md_flag && area_md){
		memcpy(area_md,md_result.reserved,md_ctrl.h_num * md_ctrl.v_num);
		memset(md_result.reserved, 0, md_ctrl.h_num * md_ctrl.v_num);
	}

	*md_sec = ak_date_to_seconds(&md_result.detect_time);
	md_result.global = 0;
	ak_thread_mutex_unlock(&md_result.lock);

	return 1;
}

/**
 * md_thread - move detect thread, do detect thing here
 * compare move detect success over 2 times, then one detect message is emit really
 */
static void *md_thread(void *arg)
{
	int move_count = 0;
	int detect_interval ;
	struct ak_timeval md_tv,now_tv;
	ak_thread_set_name("move_detection");

	ak_print_normal_ex(MPI_MD "thread id : %ld\n", ak_thread_get_tid());
	while (md_ctrl.thread_stat) {
		detect_interval = 1000/md_ctrl.md_fps;

		/*
		* stop mode ,don't run to check, wait here
		*/
		do{
			ak_sleep_ms(detect_interval);
			if(	MD_THREAD_STOP == md_ctrl.thread_stat){
				move_count = 0;
			}
		}while(MD_THREAD_STOP == md_ctrl.thread_stat);
		
		if(md_check(&md_ctrl)){
			++move_count;
			if(2 == move_count){
				move_count--;
				if(ak_get_localdate(&md_result.detect_time)){
					memset(&md_result.detect_time, 0x00, sizeof(md_result.detect_time));
				}else{
					ak_get_ostime(&md_tv);
					ak_thread_mutex_lock(&md_result.lock);
					md_result.global = 1;
					ak_thread_mutex_unlock(&md_result.lock);
				}
				ak_thread_sem_post(&md_ctrl.send_sem);
			}
		}else{
			/*
			 * alarm module donot take away md result and it is over 4 second,
			 * clean it
			 */
			if( 1 == md_result.global) {
				ak_get_ostime(&now_tv);
				if((now_tv.sec > (md_tv.sec + 4)) || (now_tv.sec < md_tv.sec )){
					ak_thread_mutex_lock(&md_result.lock);
					md_result.global = 0;
					ak_thread_mutex_unlock(&md_result.lock);
				}
			}
			move_count = 0;
		}
	}

	ak_print_normal_ex(MPI_MD "### thread id: %ld exit ###\n",
	    ak_thread_get_tid());
	ak_thread_exit();
	return NULL;
}

/**
 * submit - md init. set defaut fps ,global sensitivity
 * @vi_handle[IN]:   video in handle.
 *
 * return:  0 - success;  -1  - fail;
 */
int ak_md_init(void *vi_handle)
{
	int ret = AK_SUCCESS;
	struct video_channel_attr attr = {{0}};  // code_updating

	if ( md_init_flag ) {
		ak_print_error_ex(MPI_MD "fail,have been init \n");
		return AK_FAILED;
	}
	if (NULL == vi_handle) {
		set_error_no(ERROR_TYPE_POINTER_NULL);
		ak_print_error_ex(MPI_MD "fail, vi_handle is null\n");
		return AK_FAILED;
	}

	md_ctrl.vi_handle = vi_handle;
	md_ctrl.thread_stat = MD_THREAD_STOP;
	ak_thread_sem_init(&md_ctrl.run_sem, 0);
	ak_thread_sem_init(&md_ctrl.send_sem, 0);
	md_ctrl.global_sens = DEFAULT_DIFF * 100 / MAX_DIFF;
	md_ctrl.md_fps = 10;
	md_ctrl.area_diff = NULL;
	md_result.area = NULL;
	md_result.reserved = NULL;
	ak_thread_mutex_init(&md_result.lock, NULL);

	ak_vi_get_channel_attr(vi_handle, &attr); // code_updating

	if (0 == attr.res[VIDEO_CHN_MAIN].height % VPSS_MD_DIMENSION_V_MAX)
		md_ctrl.v_size_max = VPSS_MD_DIMENSION_V_MAX;
	else
		md_ctrl.v_size_max = 16;

	if (ak_thread_create(&md_ctrl.md_tid, md_thread, NULL,
				ANYKA_THREAD_NORMAL_STACK_SIZE, -1)) {
		ak_print_error_ex(MPI_MD "create move detect thread\n");
	}
	md_init_flag = 1;

	return ret;
}

/**
 * submit -  set how much frame to do md in one second
 * @fps[IN]:     how much frame to do md in one second.
 * return:  0 - success;  -1  - fail;
 */
int ak_md_set_fps( int fps)
{
	if(0 == md_init_flag) {
		set_error_no(ERROR_TYPE_NOT_INIT);
		ak_print_error_ex(MPI_MD "fail,no init\n");
		return AK_FAILED;
	}
	if(fps <= 0){
		set_error_no(ERROR_TYPE_INVALID_ARG);
		ak_print_error_ex(MPI_MD "invalid arg\n");
		return AK_FAILED;
	}
	md_ctrl.md_fps = fps;

	return AK_SUCCESS;
}

/**
 * submit -  get fps
 * @fps[OUT]:   how much frame that md do  in one second.
 * return:  > 0 -  fps ;  -1  - fail;
 */
int ak_md_get_fps( int *fps)
{
	if( 0 == md_init_flag) {
		set_error_no(ERROR_TYPE_NOT_INIT);
		ak_print_error_ex(MPI_MD "fail,no init\n");
		return AK_FAILED;
	}
	if(fps)
		*fps = md_ctrl.md_fps;
	else{
		set_error_no(ERROR_TYPE_POINTER_NULL);
		ak_print_error_ex(MPI_MD "NULL pointer\n");
		return AK_FAILED;
	}

	return md_ctrl.md_fps;
}

/**
 * submit -  set the sensitivity that  md do
 * @sensitivity[IN]:   sensitivity to do global md.
 * return:  0 - success;  -1  - fail;
 */
int ak_md_set_global_sensitivity ( int sensitivity)
{
	ak_print_normal_ex(MPI_MD "sensitivity:%d \n",sensitivity);
	if( 0 == md_init_flag) {
		set_error_no(ERROR_TYPE_NOT_INIT);
		ak_print_error_ex(MPI_MD "fail,no init\n");
		return AK_FAILED;
	}
	if((sensitivity < 1) || (sensitivity > 100)){
		set_error_no(ERROR_TYPE_INVALID_ARG);
		ak_print_error_ex(MPI_MD "invalid arg\n");
		return AK_FAILED;
	}
	md_ctrl.global_sens = sensitivity;
	md_ctrl.area_md_flag = 0;

	return AK_SUCCESS;
}

/**
 * submit -  get global sensitivity
 * @sensitivity[OUT]:   how sensitivity that md do
 * return: > 0 - sensitivity;  -1  - fail;
 */
int ak_md_get_global_sensitivity ( int *sensitivity)
{
	if( 0 == md_init_flag) {
		set_error_no(ERROR_TYPE_NOT_INIT);
		ak_print_error_ex(MPI_MD "fail,no init\n");
		return AK_FAILED;
	}
	if(sensitivity)
		*sensitivity = md_ctrl.global_sens;
	else{
		set_error_no(ERROR_TYPE_POINTER_NULL);
		ak_print_error_ex(MPI_MD "NULL pointer\n");
		return AK_FAILED;
	}
	return md_ctrl.global_sens;
}

/**
 * submit - set dimension area thart md to do.
 * @horizon_num[IN]:   dimension number of horizontal.
 * @vertical_num[IN]:   dimension number of vertial.
 * @area_sensitivity[IN]:   each area sensitivity
 * return:  0 - success;  -1  - fail;
 */
int ak_md_set_area_sensitivity(int horizon_num, int vertical_num,
							int *area_sensitivity)
{
	int i = 0;
	int j = 0;
	int area_sens = 0;

	ak_print_normal_ex(MPI_MD "h_num:%d v_num:%d\n",horizon_num,vertical_num);
	if( 0 == md_init_flag) {
		set_error_no(ERROR_TYPE_NOT_INIT);
		ak_print_error_ex(MPI_MD "fail,not init\n");
		return AK_FAILED;
	}
	if(( horizon_num < 0 || vertical_num < 0)
		|| (0 != (md_ctrl.v_size_max % vertical_num))
		|| (0 != (VPSS_MD_DIMENSION_H_MAX % horizon_num))
		|| (NULL == area_sensitivity)) {
		set_error_no(ERROR_TYPE_INVALID_ARG);
		ak_print_error_ex(MPI_MD "fail\n");
		return AK_FAILED;
	}

	int ret = AK_FAILED;
	if(md_ctrl.area_diff) {
		free(md_ctrl.area_diff);
		md_ctrl.area_diff = NULL;
	}
	md_ctrl.area_diff = (unsigned short*)calloc(sizeof(unsigned short),
		horizon_num * vertical_num);
	if(NULL == md_ctrl.area_diff){
		set_error_no(ERROR_TYPE_MALLOC_FAILED);
		ak_print_error_ex(MPI_MD "calloc md_ctrl.area_diff fail\n");
		goto set_area_end;
	}

	if(md_result.area) {
		free(md_result.area);
		md_result.area = NULL;
	}
	md_result.area = (char *)calloc(1, horizon_num * vertical_num);
	if(NULL == md_result.area){
		set_error_no(ERROR_TYPE_MALLOC_FAILED);
		ak_print_error_ex(MPI_MD "calloc md_result.area fail\n");
		goto set_area_end;
	}

	if(md_result.reserved ) {
		free(md_result.reserved);
		md_result.reserved = NULL;
	}
	md_result.reserved = (char *)calloc(1, horizon_num * vertical_num);
	if(NULL == md_result.reserved){
		set_error_no(ERROR_TYPE_MALLOC_FAILED);
		ak_print_error_ex(MPI_MD "calloc md_result.reserved fail\n");
		goto set_area_end;
	}

	for( i = 0 ; i < vertical_num; i++) {
		for( j = 0 ; j < horizon_num; j++){
			area_sens = *(area_sensitivity + i * horizon_num + j);
			if((area_sens < 1) || (area_sens > 100)){
				ak_print_error_ex(MPI_MD "fail\n");
				goto set_area_end;
			}
			//ak_print_error_ex(MPI_MD "i:%d j:%d\n",i,j);			
			if (1 == area_sens){			
				*(md_ctrl.area_diff + i * horizon_num + j) = MAX_DIFF;
			}
			else if (area_sens <= 10){
			/*map (1-10] to  (MAX_DIFF , MAX_DIFF /5)*/
				*(md_ctrl.area_diff + i * horizon_num + j) = MAX_DIFF / 5 + 
					MAX_DIFF / 50 * 4 *(11 - area_sens);
			}
			else if (area_sens > 90){
			/*map [91-100] to  [MAX_DIFF /500, MAX_DIFF /5000]*/
				*(md_ctrl.area_diff + i * horizon_num + j) = MAX_DIFF / 5000 * (101 - area_sens);
			}
			else {
			/*map [10-90] to  [MAX_DIFF /5, MAX_DIFF /500)*/
				*(md_ctrl.area_diff + i * horizon_num + j) = MAX_DIFF / 400 * (91 - area_sens);
			}
		}
	}

	md_ctrl.v_num = vertical_num;
	md_ctrl.h_num = horizon_num;
	md_ctrl.area_md_flag = 1;
	ret = AK_SUCCESS;

set_area_end:
	if (AK_FAILED == ret) {
		if(md_ctrl.area_diff) {
			free(md_ctrl.area_diff);
			md_ctrl.area_diff = NULL;
		}
		if(md_result.area) {
			free(md_result.area);
			md_result.area = NULL;
		}
		if(md_result.reserved ) {
			free(md_result.reserved);
			md_result.reserved = NULL;
		}
	}

	return ret;
}

/**
 * submit - get dimension max thart md support.
 * @horizon_num[OUT]:   dimension number of horizontal.
 * @vertical_num[OUT]:   dimension number of vertial.
 * return:  0 - success;  -1  - fail;
 */
int ak_md_get_dimension_max( int *horizon_num, int *vertical_num)
{
	if(horizon_num && vertical_num){
		*vertical_num = md_ctrl.v_size_max;
		*horizon_num = VPSS_MD_DIMENSION_H_MAX;
		return AK_SUCCESS;
	}
	set_error_no(ERROR_TYPE_POINTER_NULL);
	ak_print_error_ex(MPI_MD "NULL pointer\n");
	return AK_FAILED;
}

/**
 * submit - get md result.
 * @md_sec[OUT]:  md trigger time,calendar time,second.
 * @area_md[OUT]:	if enable area md, output area md result.
 * @msec[IN]:  msec <0  block mode, ==0 non-block mode, >0 waittime
 * return: 1 - md trigger; 0 - no md;  -1  - fail;
 */
int ak_md_get_result(int *md_sec, char *area_md, int msec)
{
	int ret = 0;

	if (0 == md_init_flag) {
		set_error_no(ERROR_TYPE_NOT_INIT);
		ak_print_error_ex(MPI_MD "fail,not init\n");
		return AK_FAILED;
	}
	if (MD_THREAD_RUN != md_ctrl.thread_stat) {
		set_error_no(ERROR_TYPE_FUNC_NOT_SUPPORT);
		ak_print_error_ex(MPI_MD "fail,not run\n");
		return AK_FAILED;
	}
	if (NULL == md_sec) {
		set_error_no(ERROR_TYPE_POINTER_NULL);
		ak_print_error_ex(MPI_MD "fail,NULL pointer\n");
		return AK_FAILED;
	}
	/*md have been triggered*/
	if (md_result.global) {
		return get_md_result(md_sec, area_md);
	}
	if (0 == msec)
		return 0;
	else if (msec > 0) {
		while (msec > 0) {
			ak_sleep_ms(20);
			if (md_result.global) {
				return get_md_result(md_sec, area_md);
			}
			msec -= 20;
		}
	} else if (msec < 0) {
		ret = ak_thread_sem_wait(&md_ctrl.send_sem);
	}

	/* sem_wait exit normally, md is triggered*/
	if ((0 == ret) && (md_result.global)) {
		return get_md_result(md_sec, area_md);
	}

	/*no md triggered*/
	return 0;
}

/**
 * ak_md_enable - start or stop md .
 * @enable[IN]:  [0,1],  0 -> stop md; 1 -> start md
 * return: 0 - success; otherwise -1;
 */
int ak_md_enable(int enable)
{
	if (0 == md_init_flag) {
		set_error_no(ERROR_TYPE_NOT_INIT);
		ak_print_error_ex(MPI_MD "fail,no init\n");
		return AK_FAILED;
	}
	ak_print_normal_ex(MPI_MD "enable:%d\n", enable);
	if((1 == enable) && (MD_THREAD_STOP == md_ctrl.thread_stat)) {
		md_ctrl.thread_stat = MD_THREAD_RUN;
		//ak_thread_sem_post(&md_ctrl.run_sem);
	} else if ((0 == enable) && (MD_THREAD_RUN == md_ctrl.thread_stat)) {
		md_ctrl.thread_stat = MD_THREAD_STOP;
	}
	return AK_SUCCESS;
}

/*
 * ak_md_destroy - free md resource and quit md .
 * return: none
 */
void ak_md_destroy(void)
{
 	int ret = 0;

	if (0 == md_init_flag) {
		ak_print_error_ex(MPI_MD "fail, no init\n");
		return ;
	}
	md_init_flag = 0;
	if(MD_THREAD_STOP == md_ctrl.thread_stat) {
		md_ctrl.thread_stat = MD_THREAD_EXIT;
		//ak_thread_sem_post(&md_ctrl.run_sem);
	} else
		md_ctrl.thread_stat = MD_THREAD_EXIT;

	ret = ak_thread_join(md_ctrl.md_tid);
	ak_print_normal_ex(MPI_MD "disable, %s\n", ret ? "failed" : "success");
	ak_thread_sem_destroy(&md_ctrl.run_sem);
	ak_thread_sem_destroy(&md_ctrl.send_sem);
	ak_thread_mutex_destroy(&md_result.lock);
	if (md_ctrl.area_diff)
		free(md_ctrl.area_diff);
	if (md_result.area)
		free(md_result.area);
	if (md_result.reserved)
		free(md_result.reserved);
}

const char* ak_md_get_version(void)
{
	return hw_md_version;
}
