/**
* @brief  PTZ control
* 
* @author dengzhou
* @date 2013-04-07
* @param[void] 
* @return T_S32
* @retval 
*/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <string.h>
#include <sys/select.h>
#include <time.h>
#include "PTZControl.h"

#define AK_MOTOR_DEV0 "/dev/ak-motor0"//左右
#define AK_MOTOR_DEV1 "/dev/ak-motor1"//上下

#define MOTOR_MAX_BUF 	(64)

void anyka_print(char *fmt, ...)
{}
/* time out second val */
//int timeout_val;

typedef enum
{
	MT_STOP,
	MT_STEP,
	MT_RUN,
	MT_RUNTO,
	MT_CAL,
}MT_STATUS;

struct ak_motor
{
	int angle;
	
	int running;
	
	pthread_mutex_t lock;
	struct notify_data data;
	int fd;
	MT_STATUS mt_status;
	int step_degree;
	int cw;

	int default_speed;
	int run_speed;
	int runto_speed;
	int hit_speed;
	int step_speed;
	int direction;//0左右   1上下
	
	int nMax_hit;//撞击角度
	int nMax_unhit;//最大不撞击角度
	
	int dg_max;//最大角度
	int dg_min;//最小角度
	int dg_save;//保存设置位置的角度
	int dg_cur;//当前位置角度
	int dg_center;//中间位置角度
};

//角度的范围为[-x,x]，垂直方向上中间角度为-32，水平方向为0
#define MAX_SPEED 200
#define MAX_DG 360 * 3 + 1//最大转动角度，这里定义足够大
#define INVALID_DG	370
#define DEFAULT_LEFT_UNHIT_DG		358
#define DEFAULT_UP_UNHIT_DG			106


#if 0
static struct ak_motor akmotor[] = 
{
	[0] = {
		.fd = -1,
		.mt_status = MT_STOP,
		.step_degree = 8,//这个值或者它的倍数的误差最小
		
		.default_speed = MAX_SPEED,//5,
		.run_speed = MAX_SPEED,//5,
		.runto_speed = MAX_SPEED,//15,
		.hit_speed = MAX_SPEED,//15,
		.step_speed = 15,//15,//8,
		
		.direction = 0,
		.dg_center = 0,
	},
	[1] = {
		.fd = -1,
		.mt_status = MT_STOP,
		.step_degree = 8,
		
		.default_speed = MAX_SPEED,//3,
		.run_speed = MAX_SPEED,//3,
		.runto_speed = MAX_SPEED,//15,
		.hit_speed = MAX_SPEED,//15,
		.step_speed = 15,//15,//8,
		
		.direction = 1,
		.dg_center = -32,
	},
};

#else
static struct ak_motor akmotor[] = 
{
	[0] = {
		.fd = -1,
		.mt_status = MT_STOP,
		.step_degree = 8,
		
		.default_speed = MAX_SPEED,//3,
		.run_speed = MAX_SPEED,//3,
		.runto_speed = MAX_SPEED,//15,
		.hit_speed = MAX_SPEED,//15,
		.step_speed = 15,//15,//8,
		
		.direction = 1,
		.dg_center = -32,
	},
	[1] = {
		.fd = -1,
		.mt_status = MT_STOP,
		.step_degree = 8,
		
		.default_speed = MAX_SPEED,//3,
		.run_speed = MAX_SPEED,//3,
		.runto_speed = MAX_SPEED,//15,
		.hit_speed = MAX_SPEED,//15,
		.step_speed = 15,//15,//8,
		
		.direction = 1,
		.dg_center = -32,
	},
};


#endif



#if 1 /*fanxt*/
static void ak_motor_revise_param(struct ak_motor *motor)
{
	#define DEF_H_UHIT_ANGLE	(356)
	#define DEF_V_UHIT_ANGLE	(104)
	#define MIN_H_UHIT_ANGLE	(350)
	#define MIN_V_UHIT_ANGLE	(100)

	int def_angle;
	int min_angle;

	if (motor == &akmotor[0])
	{
		def_angle = DEF_H_UHIT_ANGLE;
		min_angle = MIN_H_UHIT_ANGLE;
	}
	else
	{
		def_angle = DEF_V_UHIT_ANGLE;
		min_angle = MIN_V_UHIT_ANGLE;
	}

	if ((motor->nMax_hit < min_angle) ||
		(motor->nMax_unhit < min_angle))
	{
		motor->nMax_hit = def_angle;
		motor->nMax_unhit = def_angle - 1;
	}
}

#endif

/**
* @brief  motor turn
* 
* @author dengzhou
* @date 2013-04-07
* @param[void] 
* @return T_S32
* @retval 
*/
static int motor_turn(struct ak_motor *motor, int angle, int is_cw)
{
	if(motor->fd < 0)
		return -1;
	int ret;
	int cmd = is_cw ? AK_MOTOR_TURN_CLKWISE:AK_MOTOR_TURN_ANTICLKWISE;

	motor->angle = angle;
	ret = ioctl(motor->fd, cmd, &angle);	
	if(ret) {
		printf("%s fail. is_cw:%d, angle:%d, ret:%d.\n", __func__, angle, is_cw, ret);
	}
	return ret;
}

/**
* @brief  change speed
* 
* @author dengzhou
* @date 2013-04-07
* @param[void] 
* @return T_S32
* @retval 
*/

static int motor_change_speed(struct ak_motor *motor, int ang_speed)
{
	int ret;	
	int val = ang_speed;
	if(motor->fd < 0)
		return -1;
	ret = ioctl(motor->fd, AK_MOTOR_SET_ANG_SPEED, &val);	
	if(ret) {
		printf("%s fail.  ang_speed:%d, ret:%d.\n", __func__, ang_speed, ret);
	}
	return ret;
}
#if 1   /*fanxt*/
static int motor_get_speed(struct ak_motor *motor)
{
	int ret;	
	int val;

	if(motor->fd < 0)
		return -1;
	ret = ioctl(motor->fd, AK_MOTOR_GET_ANG_SPEED, &val);	
	if(ret) {
		anyka_print("%s fail.  ret:%d.\n", __func__, ret);
		val = 16;
	}

	return val;
}
#endif

int ak_motor_calcu_max_time(struct ak_motor *motor)
{
	#define ROUND_TIME	30
	#define REDUND		5

	int sec;

	sec = motor->angle * ROUND_TIME;
	sec = sec / 360 + ((sec % 360) ? 1:0);

	sec += REDUND;

	if (sec > ROUND_TIME + REDUND)
		sec = ROUND_TIME + REDUND;

	anyka_print("%s angle:%d, time:%ds.\n", __func__, motor->angle, sec);
	return sec;
}

int ak_motor_check_event_timeout(time_t *base, int timeout)
{
	time_t t;

	t = time(0);

	/* 防止系统时间同步和时区同步造成误判 */
	if (t - *base > 1800)
		*base = time(0);

	t -= *base;
	if (t > timeout)
		return 1;

	return 0;
}

/**
* @brief  wait the moto stop
* 
* @author dengzhou
* @date 2013-04-07
* @param[void] 
* @return T_S32
* @retval 
*/
#if 1 /*fanxt*/
static int ak_motor_wait_stop(struct ak_motor *motor, int* remain_angle, int speed)
{
	fd_set fds;
	int ret;
	struct timeval tv;
	time_t t, t2;
	const int timeout = 2000 / (1000 / (1000 / speed / (360 / 64))) + 2;
	if(motor->fd < 0)
		return -1;

	t = time(0);
start:
	t2 = time(0);
	tv.tv_sec	= timeout - (t2 - t);
	tv.tv_usec	= 0;
	FD_ZERO(&fds);
	FD_SET(motor->fd, &fds);
	ret = select(motor->fd+1, &fds, NULL, NULL, &tv);
	if(ret <= 0) {
		anyka_print("wait, ret:%d,\n", ret);
		if (ret == 0)
			anyka_print("%s select timeout.\n", __func__);
		return -1;
	}
	
	ret = read(motor->fd, (void*)&motor->data, sizeof(struct notify_data));
		
	if(!(motor->data.event & AK_MOTOR_EVENT_STOP))
	{
		anyka_print("not stop, %d, %d\n", motor->data.event, motor->data.remain_angle);
		if(motor->data.remain_angle > 0)
		{
			//可能会由于抖动的原因误报，
			//如果出现这种情况就按原来的方向转动剩余角度
			//motor_turn(motor, motor->data.remain_angle, motor->cw);
			if (ak_motor_check_event_timeout(&t, timeout))
			{
				anyka_print("%s event timeout.\n", __func__);
				return -1;
			}

			goto start;
		}	
	}

	*remain_angle = motor->data.remain_angle;
	return 0;
}
#else
static int ak_motor_wait_stop(struct ak_motor *motor, int* remain_angle)
{
	fd_set fds;
	int ret;
	struct timeval tv;
	time_t t;
	const int timeout = ak_motor_calcu_max_time(motor);
	if(motor->fd < 0)
		return -1;

	t = time(0);
start:
	tv.tv_sec	= timeout;
	tv.tv_usec	= 0;
	FD_ZERO(&fds);
	FD_SET(motor->fd, &fds);
	ret = select(motor->fd+1, &fds, NULL, NULL, &tv);
	if(ret <= 0) {
		anyka_print("wait, ret:%d,\n", ret);
		if (ret == 0)
			anyka_print("%s select timeout.\n", __func__);
		return -1;
	}
	
	ret = read(motor->fd, (void*)&motor->data, sizeof(struct notify_data));
		
	if(!(motor->data.event & AK_MOTOR_EVENT_STOP))
	{
		anyka_print("not stop, %d, %d\n", motor->data.event, motor->data.remain_angle);
		if(motor->data.remain_angle > 0)
		{
			//可能会由于抖动的原因误报，
			//如果出现这种情况就按原来的方向转动剩余角度
			motor_turn(motor, motor->data.remain_angle, motor->cw);
			if (ak_motor_check_event_timeout(&t, timeout))
			{
				anyka_print("%s event timeout.\n", __func__);
				return -1;
			}

			goto start;
		}	
	}

	*remain_angle = motor->data.remain_angle;
	return 0;
}
#endif
/**
* @brief motor_wait_hit
* 
* @author dengzhou
* @date 2013-04-07
* @param[void] 
* @return T_S32
* @retval 
*/
#if 1 /*fanxt*/
static int ak_motor_wait_hit(struct ak_motor *motor, int* remain_angle, int speed)
{
	fd_set fds;
	int ret;
	struct timeval tv;
	time_t t, t2;
	const int timeout =ak_motor_calcu_max_time(motor); //2000 / (1000 / (1000 / speed / (360 / 64))) + 2;
	if(motor->fd < 0)
		{
		return -1;
		}

	t = time(0);
start:
	t2 = time(0);
	tv.tv_sec	= timeout - (t2 - t);
	tv.tv_usec	= 0;
	FD_ZERO(&fds);
	FD_SET(motor->fd, &fds);

	ret = select(motor->fd+1, &fds, NULL, NULL, &tv);
	printf("*****************ccptz test %d*******************.\n",ret);
	if(ret <= 0) {
		anyka_print("wait, ret:%d,\n", ret);
		if (ret == 0)
			anyka_print("%s select timeout.\n", __func__);
		return -1;
	}
	
	ret = read(motor->fd, (void*)&motor->data, sizeof(struct notify_data));
	
	if(!(motor->data.event & AK_MOTOR_EVENT_HIT))
	{
		anyka_print("not hit,motor->data.event = %d, %d\n", motor->data.event, motor->data.remain_angle);
		if (ak_motor_check_event_timeout(&t, timeout))
		{
			anyka_print("%s event timeout.\n", __func__);
			return -1;
		}

		goto start;
	}
	
	*remain_angle = motor->data.remain_angle;
	return 0;
}

#else
static int ak_motor_wait_hit(struct ak_motor *motor, int* remain_angle)
{
	fd_set fds;
	int ret;
	struct timeval tv;
	time_t t;
	const int timeout = ak_motor_calcu_max_time(motor);
	if(motor->fd < 0)
		return -1;

	t = time(0);
start:
	tv.tv_sec	= timeout;
	tv.tv_usec	= 0;
	FD_ZERO(&fds);
	FD_SET(motor->fd, &fds);
	ret = select(motor->fd+1, &fds, NULL, NULL, &tv);
	if(ret <= 0) {
		printf("wait, ret:%d,\n", ret);
		if (ret == 0)
			anyka_print("%s select timeout.\n", __func__);
		return -1;
	}
	
	ret = read(motor->fd, (void*)&motor->data, sizeof(struct notify_data));
	
	if(!(motor->data.event & AK_MOTOR_EVENT_HIT))
	{
		anyka_print("not hit,motor->data.event = %d, %d\n", motor->data.event, motor->data.remain_angle);
		if (ak_motor_check_event_timeout(&t, timeout))
		{
			anyka_print("%s event timeout.\n", __func__);
			return -1;
		}

		goto start;
	}
	
	*remain_angle = motor->data.remain_angle;
	return 0;
}
#endif

#if 1  /*fanxt*/
static int ak_motor_wait_hit_stop(struct ak_motor *motor, int* remain_angle, int speed)
{
	fd_set fds;
	int ret;
	struct timeval tv;
	time_t t, t2;
	const int timeout = 2000 / (1000 / (1000 / speed / (360 / 64))) + 2;
	
	if(motor->fd < 0)
		return -1;

	t = time(0);
start:
	t2 = time(0);
	tv.tv_sec	= timeout - (t2 - t);
	tv.tv_usec	= 0;
	FD_ZERO(&fds);
	FD_SET(motor->fd, &fds);
	ret = select(motor->fd+1, &fds, NULL, NULL, &tv);
	if(ret <= 0) {
		anyka_print("wait, ret:%d,\n", ret);
		if (ret == 0)
			anyka_print("%s select timeout.\n", __func__);
		return -1;
	}
	
	ret = read(motor->fd, (void*)&motor->data, sizeof(struct notify_data));



	if (motor->data.event & AK_MOTOR_EVENT_HIT)
	{
		*remain_angle = motor->data.remain_angle;
		return 0;
	}
	else if (motor->data.event & AK_MOTOR_EVENT_STOP)
	{
		*remain_angle = motor->data.remain_angle;
		return -1;
	}
	else
	{
		anyka_print("not hit,motor->data.event = %d, %d\n", motor->data.event, motor->data.remain_angle);
		if (ak_motor_check_event_timeout(&t, timeout))
		{
			anyka_print("%s event timeout.\n", __func__);
			return -1;
		}

		goto start;
	}
	
	*remain_angle = motor->data.remain_angle;
	return 0;
}
#endif
/**
* @brief motor_wait_unhit
* 
* @author dengzhou
* @date 2013-04-07
* @param[void] 
* @return T_S32
* @retval 
*/
#if 1   /*fanxt*/
static int ak_motor_wait_unhit(struct ak_motor *motor, int* remain_angle, int speed)
{
	fd_set fds;
	int ret;
	struct timeval tv;
	time_t t, t2;
	const int timeout = 2000 / (1000 / (1000 / speed / (360 / 64))) + 2;
	if(motor->fd < 0)
		return -1;

	t = time(0);
start:
	t2 = time(0);
	tv.tv_sec	= timeout - (t2 - t);
	tv.tv_usec	= 0;
	FD_ZERO(&fds);
	FD_SET(motor->fd, &fds);
	ret = select(motor->fd+1, &fds, NULL, NULL, &tv);
	if(ret <= 0) {
		anyka_print("wait, ret:%d,\n", ret);
		if (ret == 0)
			anyka_print("%s select timeout.\n", __func__);
		return -1;
	}
	
	ret = read(motor->fd, (void*)&motor->data, sizeof(struct notify_data));
		
	if(!(motor->data.event & AK_MOTOR_EVENT_UNHIT))
	{
		anyka_print("not unhit,motor->data.event = %d,,%d\n", motor->data.event, motor->data.remain_angle);
		if (ak_motor_check_event_timeout(&t, timeout))
		{
			anyka_print("%s event timeout.\n", __func__);
			return -1;
		}

		goto start;
	}
	
	*remain_angle = motor->data.remain_angle;
	return 0;
}
#else
static int ak_motor_wait_unhit(struct ak_motor *motor, int* remain_angle)
{
	fd_set fds;
	int ret;
	struct timeval tv;
	time_t t;
	const int timeout = ak_motor_calcu_max_time(motor);
	if(motor->fd < 0)
		return -1;

	t = time(0);
start:
	tv.tv_sec	= timeout;
	tv.tv_usec	= 0;
	FD_ZERO(&fds);
	FD_SET(motor->fd, &fds);
	ret = select(motor->fd+1, &fds, NULL, NULL, &tv);
	if(ret <= 0) {
		anyka_print("wait, ret:%d,\n", ret);
		if (ret == 0)
			anyka_print("%s select timeout.\n", __func__);
		return -1;
	}
	
	ret = read(motor->fd, (void*)&motor->data, sizeof(struct notify_data));
		
	if(!(motor->data.event & AK_MOTOR_EVENT_UNHIT))
	{
		anyka_print("not unhit,motor->data.event = %d,,%d\n", motor->data.event, motor->data.remain_angle);
		if (ak_motor_check_event_timeout(&t, timeout))
		{
			anyka_print("%s event timeout.\n", __func__);
			return -1;
		}

		goto start;
	}
	
	*remain_angle = motor->data.remain_angle;
	return 0;
}
#endif
/**
* @brief motor turn clkwise
* 
* @author dengzhou
* @date 2013-04-07
* @param[void] 
* @return T_S32
* @retval 
*/

static int ak_motor_turn_clkwise(struct ak_motor *motor)
{
	int rg;
	int speed;
	speed=motor_get_speed(motor);
		
	if(0/*motor->dg_max - motor->dg_cur <= motor->step_degree*/)   /*fanxt*/
	{
		anyka_print("dg_max=%d, dg_cur=%d, %d\n", motor->dg_max, motor->dg_cur, motor->dg_max - motor->dg_cur);
		motor_turn(motor, motor->step_degree, motor->cw);
		ak_motor_wait_hit(motor, &rg, speed);
		motor->dg_cur = motor->dg_max + 1;

//		PTZControlSetPosition(1);
	}
	else
	{
		motor_turn(motor, motor->step_degree, motor->cw);
		ak_motor_wait_stop(motor, &rg, speed);



//		PTZControlSetPosition(1);
	}

	return 0;
}
/**
* @brief motor_wait_unhit
* 
* @author dengzhou
* @date 2013-04-07
* @param[void] 
* @return T_S32
* @retval 
*/

static int ak_motor_turn_anticlkwise(struct ak_motor *motor)
{
	int rg;
	int speed;

	speed = motor_get_speed(motor);

	if(0/*motor->dg_cur - motor->dg_min <= motor->step_degree*/)  /*fanxt*/
	{
		anyka_print("anti:dg_min=%d, dg_cur=%d, %d\n", motor->dg_min, motor->dg_cur, motor->dg_cur - motor->dg_min);
		motor_turn(motor, motor->step_degree, motor->cw);
		ak_motor_wait_hit(motor, &rg, speed);
		motor->dg_cur = motor->dg_min - 1;

//		PTZControlSetPosition(1);
	}
	else
	{
		motor_turn(motor, motor->step_degree, motor->cw);
		ak_motor_wait_stop(motor, &rg, speed);



//		PTZControlSetPosition(1);
	}
	
	return 0;
}

/**
* @brief motor thread
* 
* @author dengzhou
* @date 2013-04-07
* @param[void] 
* @return T_S32
* @retval 
*/

#if 0
static int ak_motor_cal_thread(void* data)
{
	printf("%s\n", __func__);
	//巡航，校准
	struct ak_motor *motor = NULL;
	
	int rg;
	int a,b,c,d;
	int speed;

	speed=0;
	//speed=motor->hit_speed;
	printf("*****************55ptz test *******************.\n");
	//先水平，后垂直
	for(int i = 0; i < 2; i ++)
	{
		motor = &akmotor[i];
		
		if(motor->mt_status != MT_STOP)
			continue;
		if(motor->fd < 0)
			continue;
		
		motor_change_speed(motor, motor->runto_speed);
		motor->mt_status = MT_CAL;
		
		//顺时针转到底
		motor->cw = 1;
		motor_turn(motor, MAX_DG, motor->cw);
		if(ak_motor_wait_hit(motor, &rg,speed) < 0){
			printf("*****************66ptz test *******************.\n");
			//return -1;
		}
		
		a = rg;
		motor->cw = 0;
		//逆时针转到底
		motor_turn(motor, rg, motor->cw);
		if(ak_motor_wait_unhit(motor, &rg, speed) < 0){
			printf("*****************77ptz test *******************.\n");
		//	return -1;
		}
		
		b = rg;
	
	//	motor_turn(motor, rg, motor->cw);   /*fanxt*/
		if(ak_motor_wait_hit(motor, &rg,speed) < 0){
			printf("*****************88ptz test *******************.\n");
		//	return -1;
		}
		
		c = rg;
		//计算出各种角度后顺时针转到中间
		motor->cw = 1;
#if 0    /*fanxt*/
		motor_turn(motor, rg, motor->cw);
		if(ak_motor_wait_unhit(motor, &rg) < 0){
			printf("*****************99ptz test *******************.\n");
			return -1;
		}
#endif		
		d = rg;
		anyka_print("a = %d, b = %d, c = %d, d = %d, ac = %d, bd = %d\n", a,b,c,d, (a - c), (b + d - 2 * c));
	
		motor->nMax_hit = a - c;
	//	motor->nMax_unhit = b + d - 2 * c;
		motor->nMax_unhit = motor->nMax_hit - 4;

		printf("MaxHit=%d\n", motor->nMax_hit);

		ak_motor_revise_param(motor);
	
		motor->dg_max = motor->nMax_unhit / 2;
		motor->dg_min =  - (motor->nMax_unhit) / 2;
		motor->dg_save = motor->dg_center;
		motor->dg_cur = motor->dg_min;
	
		rg = motor->dg_save - motor->dg_cur;
		motor_turn(motor, rg, motor->cw);
		if(ak_motor_wait_hit_stop(motor, &rg, speed)< 0){
			printf("*****************aaptz test *******************.\n");
		//	return -1;
		}
		
		motor->dg_cur = motor->dg_save - rg;
	
		motor->mt_status = MT_STOP;
		motor_change_speed(motor, motor->step_speed);
	}
	
	anyka_print("cal ok\n");
	return 0;
}
#else
static int ak_motor_cal_thread(void* data)
{
	printf("%s\n", __func__);
	//巡航，校准
	struct ak_motor *motor = NULL;
	
	int rg;
	int a,b,c,d;
	int speed;

	speed=0;
	//speed=motor->hit_speed;
	printf("*****************55ptz test *******************.\n");
	//先水平，后垂直
	
	for(int i = 0; i < 2; i ++)
	{
		motor = &akmotor[i];

		if(motor->mt_status != MT_STOP)
			continue;
		if(motor->fd < 0)
			continue;
		
		motor_change_speed(motor, motor->runto_speed);
		motor->mt_status = MT_RUN;
		
		//顺时针转到底
		motor->cw = 1;
		rg = (i==0)?(180):(90);
		motor_turn(motor, rg, motor->cw);
		sleep(3);
	
		motor->cw = 0;
		//逆时针转到底
		motor_turn(motor, rg, motor->cw);
		sleep(3);
		motor->mt_status = MT_STOP;


	}
	
	anyka_print("cal ok\n");
	return 0;
}



#endif

/**
* @brief create thread 
* 
* @author dengzhou
* @date 2013-04-07
* @param[void] 
* @return T_S32
* @retval 
*/

static int ak_motor_cal(struct ak_motor* motor)
{
	//pthread_t th;
	//anyka_pthread_create(&th, ak_motor_cal_thread, motor, ANYKA_THREAD_MIN_STACK_SIZE, -1);
	printf("*****************33ptz test *******************.\n");
	if(ak_motor_cal_thread(NULL) < 0)
		{
		printf("*****************44ptz test *******************.\n");
		return -1;
		}
	return 0;
}

/**
* @brief thread fun
* 
* @author dengzhou
* @date 2013-04-07
* @param[void] 
* @return T_S32
* @retval 
*/
/*
static void* ak_motor_run_thread(void* data)
{
	struct ak_motor *motor = data;
	if(motor->mt_status != MT_STOP)
		return 0;

	motor->mt_status = MT_RUN;
	motor_change_speed(motor, motor->run_speed);
	
	motor->cw = 1;//默认从顺时针转
	
	while(motor->mt_status == MT_RUN)
	{
		if(motor->cw == 1)
		{
			ak_motor_turn_clkwise(motor);
		}
		else
		{
			ak_motor_turn_anticlkwise(motor);
		}
	}
	
	motor_change_speed(motor, motor->step_speed);
	return NULL;
}
*/
static int ak_motor_run(struct ak_motor* motor)
{
	if(motor->fd < 0)
		return -1;
	//pthread_t th;
	//anyka_pthread_create(&th, ak_motor_run_thread, motor, ANYKA_THREAD_MIN_STACK_SIZE, -1);
	return 0;
}
/*
static void* ak_motor_runto_thread(void* data)
{
	struct ak_motor *motor = data;
	if(motor->mt_status != MT_STOP)
		return 0;

	int rg;
	motor->mt_status = MT_RUNTO;
	
	motor_change_speed(motor, motor->runto_speed);
	int dg = motor->dg_save - motor->dg_cur;//计算当前位置和保存位置的角度
	
	if(dg > 0)//判断转动方向
	{
		motor->cw = 1;	
	}
	else if(dg < 0)
	{
		motor->cw = 0;
		dg = -dg;
	}
	anyka_print("cw = %d,angle = %d\n", motor->cw, dg);
	motor_turn(motor, dg, motor->cw);
	ak_motor_wait_stop(motor, &rg);
	motor->dg_cur = motor->dg_save;
	
	motor_change_speed(motor, motor->step_speed);
	motor->mt_status = MT_STOP;
	return NULL;
}
*/
static int ak_motor_runto(struct ak_motor* motor)
{
	if(motor->fd < 0)
		return -1;
	//pthread_t th;
	//anyka_pthread_create(&th,  ak_motor_runto_thread, motor, ANYKA_THREAD_MIN_STACK_SIZE, -1);
	return 0;
}


static int ak_motor_init(struct ak_motor *motor, char *name)
{
	if(!motor)
		return -1;

	motor->fd = open(name, O_RDWR);
	if(motor->fd < 0)
		return -2;
	return 0;
}

int PTZControlInit()
{
	ak_motor_init(&akmotor[0], AK_MOTOR_DEV0);
	ak_motor_init(&akmotor[1], AK_MOTOR_DEV1);
	return ak_motor_cal(NULL);
}

int PTZControlStepUp()
{
	struct ak_motor* motor = &akmotor[1];
	if(motor->mt_status != MT_STOP)
		return -1;
	motor->mt_status = MT_STEP;
	motor->cw = 1;
	
	if(ak_motor_turn_clkwise(motor) < 0)
		return -1;
	
	motor->mt_status = MT_STOP;
	//printf("up:curpos=%d\n", motor->dg_cur);
	return 0;
}
int PTZControlStepDown()
{
	struct ak_motor* motor = &akmotor[1];
	
	if(motor->mt_status != MT_STOP)
		return -1;
	motor->mt_status = MT_STEP;
	motor->cw = 0;
	
	ak_motor_turn_anticlkwise(motor);
	
	motor->mt_status = MT_STOP;
	//printf("down:curpos=%d\n", motor->dg_cur);
	return 0;
}
int PTZControlStepLeft()
{
	struct ak_motor* motor = &akmotor[0];
	if(motor->mt_status != MT_STOP)
		return -1;
	motor->mt_status = MT_STEP;
	motor->cw = 1;
	
	if(ak_motor_turn_clkwise(motor) < 0)
		return -1;
	
	motor->mt_status = MT_STOP;
	//printf("left:curpos=%d\n", motor->dg_cur);
	return 0;
}
int PTZControlStepRight()
{
	struct ak_motor* motor = &akmotor[0];
	
	if(motor->mt_status != MT_STOP)
		return -1;
	motor->mt_status = MT_STEP;
	motor->cw = 0;
	
	ak_motor_turn_anticlkwise(motor);
	
	motor->mt_status = MT_STOP;
	//printf("right:curpos=%d\n", motor->dg_cur);
	return 0;
}
int PTZControlUpDown()
{
	struct ak_motor* pmotor = &akmotor[1];
	anyka_print("up and down:");
	if(pmotor->mt_status == MT_RUN)
	{
		anyka_print("run-->stop\n");
		pmotor->mt_status = MT_STOP;	
	}
	else if(pmotor->mt_status == MT_STOP)
	{
		anyka_print("stop-->run\n");
		ak_motor_run(pmotor);
	}
	
	return 0;
}
int PTZControlLeftRight()
{
	struct ak_motor* pmotor = &akmotor[0];
	anyka_print("left and right:");
	if(pmotor->mt_status == MT_RUN)
	{
		anyka_print("run-->stop\n");
		pmotor->mt_status = MT_STOP;	
	}
	else if(pmotor->mt_status == MT_STOP)
	{
		anyka_print("stop-->run\n");
		ak_motor_run(pmotor);
	}
	return 0;
}

struct ptz_pos posnum[6];

int PTZControlSetPosition(int para1)
{
	akmotor[0].dg_save = akmotor[0].dg_cur;
	akmotor[1].dg_save = akmotor[1].dg_cur;
	
	posnum[para1-1].left = akmotor[0].dg_cur;
	posnum[para1-1].up = akmotor[1].dg_cur;
	
	anyka_print("savepos= %d,,%d\n", akmotor[0].dg_save, akmotor[1].dg_cur);
	//anyka_set_ptz_info((void *)posnum, para1);

	return 0;
}
int PTZControlRunPosition(int para1)
{
	akmotor[0].mt_status = MT_STOP;
	akmotor[1].mt_status = MT_STOP;
	akmotor[0].dg_save = posnum[para1-1].left;
	akmotor[1].dg_save = posnum[para1-1].up;
	ak_motor_runto(&akmotor[0]);
	ak_motor_runto(&akmotor[1]);
	
	return 0;
}

int PTZControlDeinit()
{
	akmotor[0].mt_status = MT_STOP;
	akmotor[1].mt_status = MT_STOP;
	
	if(akmotor[0].fd > 0)
		close(akmotor[0].fd);
	if(akmotor[1].fd > 0)
		close(akmotor[1].fd);
	
	akmotor[0].fd = -1;
	akmotor[1].fd = -1;
	return 0;
}

static void save_stat(char *file, int stat)
{
	char buf[32];

	if (!file)
		return;

	sprintf(buf, "echo \"%d\" > %s", stat, file);
	system(buf);
}

int main(int argc, char **argv)
{
	char *file = NULL;

	if (argc > 1)
		file = argv[2];
//	timeout_val = atoi(argv[1]);
	printf("*****************11ptz test *******************.\n");
	if(PTZControlInit() < 0)
	{
		printf("ptz test failed.\n");
		save_stat(file, -1);
	}

	else
	{
		printf("ptz test ok.\n");
		save_stat(file, 0);
	}

	return 0;
}
