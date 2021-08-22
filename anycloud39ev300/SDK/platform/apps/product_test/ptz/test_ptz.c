
#include "ak_global.h"
#include "ak_common.h"
#include "ak_thread.h"
#include "ak_drv_ptz.h"

#define STEP_MAX 100
/* ptz control info */
struct ak_ptz_ctrl_info {
	unsigned char run_flag;
    sem_t ctrl_sem;
    ak_pthread_t ptz_tid;
	int code;
	int step_count;
};

static struct ak_ptz_ctrl_info ptz_ctrl = {0};

static void* test_ptz_ctrl_thread(void* param)
{
	enum ptz_status status[PTZ_DEV_NUM];
	ak_print_normal_ex("thread id: %ld\n", ak_thread_get_tid());

	ak_drv_ptz_check_self(PTZ_FEEDBACK_PIN_EXIST);

	while (ptz_ctrl.run_flag) {
		sem_wait(&ptz_ctrl.ctrl_sem);

		/* ptz turn one direct and have max distance */
		while(ptz_ctrl.run_flag
			&& (ptz_ctrl.step_count++ < STEP_MAX) ) {
			switch (ptz_ctrl.code) {
			case PTZ_TURN_UP:
			case PTZ_TURN_DOWN:
				ak_drv_ptz_turn(ptz_ctrl.code,8);
				break;
			case PTZ_TURN_LEFT:
			case PTZ_TURN_RIGHT:
				ak_drv_ptz_turn(ptz_ctrl.code,16);
				break;
			default:
				break;
			}
			do{
				/* wait ptz execute cmd */
				ak_sleep_ms(5);
				ak_drv_ptz_get_status(PTZ_DEV_H, &status[PTZ_DEV_H]);
				ak_drv_ptz_get_status(PTZ_DEV_V, &status[PTZ_DEV_V]);
			} while((status[PTZ_DEV_H] != PTZ_INIT_OK)
			    || (status[PTZ_DEV_V] != PTZ_INIT_OK));
		}

		ptz_ctrl.step_count = 0;

	}

	ak_print_normal_ex("### thread id: %ld exit ###\n", ak_thread_get_tid());
	ak_thread_exit();
    return NULL;
}

/**
 * test_ptz_init - int ptz for test
 * return:   0 , success ;  -1 , failed;
 */
int test_ptz_init(void)
{
	if (ptz_ctrl.run_flag) {
		return AK_SUCCESS;
	}

	if(ak_drv_ptz_open() < 0){
		ak_print_error_ex("ak_drv_ptz_open failed!\n");
		return AK_FAILED;
	}

	/* change motor param if you changed your machine mould */
	ak_drv_ptz_set_angle_rate(24/12.0, 21/12.0);
	ak_drv_ptz_set_degree(350, 130);
	//ak_drv_ptz_check_self(PTZ_FEEDBACK_PIN_EXIST);

	ak_thread_sem_init(&ptz_ctrl.ctrl_sem, 0);

	ptz_ctrl.step_count = 0;
	ptz_ctrl.run_flag = AK_TRUE;
	/* create thread to excute ptz cmd */
	int ret = ak_thread_create(&(ptz_ctrl.ptz_tid), test_ptz_ctrl_thread,
        NULL, ANYKA_THREAD_MIN_STACK_SIZE, -1);
	if(ret){
		ak_print_normal_ex("create test_ptz_ctrl_thread failed, ret=%d\n", ret);
	} else {
		ak_print_normal_ex("test ptz control start\n");
	}

	return ret;
}

/**
 * test_ptz_exit - exit ptz for test
 * return:   void;
 */
void test_ptz_exit(void)
{
	if (ptz_ctrl.run_flag) {
		ptz_ctrl.run_flag = AK_FALSE;
		ak_thread_sem_post(&(ptz_ctrl.ctrl_sem));
		ak_thread_join(ptz_ctrl.ptz_tid);
		ak_thread_sem_destroy(&(ptz_ctrl.ctrl_sem));
		ak_print_notice_ex("test_ptz_ctrl_thread join OK\n");
		ak_drv_ptz_close();
	}
}

/**
 * test_set_ptz_cmd - set cmd to ptz  and  ptz run
 * @code[IN]: cmd
 * return:   0 , success ;  -1 , failed;
 */
int test_set_ptz_cmd( int code )
{
    ak_print_normal_ex("ptz code:%d\n", code);
	if (0 == ptz_ctrl.run_flag) {
		ak_print_error_ex("ak_drv_ptz not init!\n");
		return AK_FAILED;
	}
	/*
  	  * PTZ_TURN_LEFT = 1,
  	  * PTZ_TURN_RIGHT,
  	  * PTZ_TURN_UP,
	  * PTZ_TURN_DOWN
	  */
	ptz_ctrl.code = code;
	ptz_ctrl.step_count = 0;
	sem_post(&ptz_ctrl.ctrl_sem);

	return 0;
}


