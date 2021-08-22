#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "internal_error.h"
#include "ak_common.h"
#include "ak_thread.h"
#include "ak_drv_ptz.h"

#define AK_MOTOR_DEV0 		"/dev/ak-motor0"	/* horizontal left-right */
#define AK_MOTOR_DEV1 		"/dev/ak-motor1"	/* vertical  up-down */
#define AK_MOTOR_DEV_NUM	2

#define CLKWISE 			1		//turn clock-wise
#define ANTICLKWISE 		0		//turn anti-clock-wise

#define MAX_SPEED 			100		//max turnning speed

#define AK_MOTOR_IOC_MAGIC 			'm'			//ioctl magic
#define AK_MOTOR_SET_ANG_SPEED 		_IOW(AK_MOTOR_IOC_MAGIC, 11, int) //set speed
#define AK_MOTOR_GET_ANG_SPEED 		_IOR(AK_MOTOR_IOC_MAGIC, 12, int) //get speed
/* turn clockwise */
#define AK_MOTOR_TURN_CLKWISE 		_IOW(AK_MOTOR_IOC_MAGIC, 13, int) 
/* turn anti-clockwise */
#define AK_MOTOR_TURN_ANTICLKWISE 	_IOW(AK_MOTOR_IOC_MAGIC, 14, int)
/* get hit status */
#define AK_MOTOR_GET_HIT_STATUS 	_IOW(AK_MOTOR_IOC_MAGIC, 15, int)

#define AK_MOTOR_TURN_STOP          _IOW(AK_MOTOR_IOC_MAGIC, 16, int)

#define AK_MOTOR_EVENT_HIT 	(1)
#define AK_MOTOR_EVENT_STOP (1<<2)

/// Anycloud V500 Porting.
/// /* 1. argument for start up */
/*
 * MOTOR_PARM:			set paramters
 */
#define MOTOR_PARM			_IOW(AK_MOTOR_IOC_MAGIC, 0x00, int)

/* 2. argument for real-time */
/*
 * MOTOR_SPEED_STEP:	set speed in step(hz)
 * MOTOR_SPEED_ANGLE:	set speed in angle
 * */
#define MOTOR_SPEED_STEP	_IOW(AK_MOTOR_IOC_MAGIC, 0x20, int)
#define MOTOR_SPEED_ANGLE	_IOW(AK_MOTOR_IOC_MAGIC, 0x21, int)

/* 3. base command */
/*
 * MOTOR_MOVE_LIMIT:	move clkwise or anticlkwise
 *						clkwise: val > 0, anticlkwise: val < 0
 * MOTOR_MOVE_NOLIMIT:	move clkwise or anticlkwise
 *						clkwise: val > 0, anticlkwise: val < 0
 * MOTOR_STOP:			set stop and wait for be stop finish
 * MOTOR_GET_STATUS:	get status
 * */
#define MOTOR_MOVE_LIMIT	_IOW(AK_MOTOR_IOC_MAGIC, 0x40, int)
#define MOTOR_MOVE_NOLIMIT	_IOW(AK_MOTOR_IOC_MAGIC, 0x41, int)
#define MOTOR_STOP			_IOW(AK_MOTOR_IOC_MAGIC, 0x42, int)
#define MOTOR_GET_STATUS	_IOW(AK_MOTOR_IOC_MAGIC, 0x43, int)

/* 4. extern command */
/*
 * MOTOR_RESET:			first go to ACTIVAL boundary then go to middle
 * MOTOR_MIDDLE:		turn to middle directly
 * MOTOR_CRUISE:		turn to the clkwise an anticlkwise until set stop
 * MOTOR_BOUNDARY:		move to ACTIVAL boundary
 * 						in clkwise or anticlkwise until be stop
 * */
#define MOTOR_RESET			_IOW(AK_MOTOR_IOC_MAGIC, 0x60, int)
#define MOTOR_MIDDLE		_IOW(AK_MOTOR_IOC_MAGIC, 0x61, int)
#define MOTOR_CRUISE		_IOW(AK_MOTOR_IOC_MAGIC, 0x62, int)
#define MOTOR_BOUNDARY		_IOW(AK_MOTOR_IOC_MAGIC, 0x63, int)

/*
 * motor_status:
 * @MOTOR_IS_STOP:		motor is stoped now
 * @MOTOR_IS_RUNNING:	motor is running now
 * */
enum motor_status {
	MOTOR_IS_STOP = 0,
	MOTOR_IS_RUNNING,
};

/*
 * struct motor_parm - motor parameters
 * @pos:				the current position
 * @speed_step:			speed of step(hz)
 * @steps_one_circel:	steps one circel
 * @total_steps:		steps motor can run
 * @boundary_steps:		reserved boundary steps
 */
struct motor_parm {
	int pos;
	int speed_step;
	int steps_one_circle;
	int total_steps;
	int boundary_steps;
};


/*
 * motor_message - message
 * @status:				motor working status
 * @pos:				the current position
 * @speed_step:			speed of step(hz)
 * @speed_angle:		speed of angle
 * @steps_one_circel:	steps one circel
 * @total_steps:		steps motor can run
 * @boundary_steps:		reserved boundary steps
 * @attach_timer:		attach to hardware timer
 */
struct motor_message {
	enum motor_status status;
	int pos;
	int speed_step;
	int speed_angle;
	int steps_one_circle;
	int total_steps;
	int boundary_steps;
	int attach_timer;
};






/* notify structure */
struct notify_data {
	int hit_num;
	int event;
	int remain_steps;
};

/* turnning command data */
struct cmd_data {
	int dg;
	int clkwise;
};

/* single motor node */
struct ak_motor {
    int run_flag;	//model run flag
	int fd;			//file describe
	enum ptz_status mt_status;		//motor status

	int step_degree;		//step to degree
	int runto_speed;
	int hit_speed;
	int step_speed;

	/*get this value by selftest*/
	int nMax_hit;

	/// Full Distance Step
	int fulldst_step;

	/// One Cycle 360d for Steps.
	int cycle_step;

	/// The Reset Step Location.
	int reset_step;

	enum ptz_feedback_pin correct_type;

	/* now ptz angle*/
	int dg_cur;
	sem_t ctrl_sem;
	struct cmd_data cmd_data;
};


/**
 * 已校准标识。
 */
static int _calibrated = AK_FALSE;


static const char drv_ptz_version[] = "libplat_drv_ptz V1.2.01";

static struct ak_motor akmotor[] =
{
	/// 水平电机。
	{
		.fd = -1,
		.mt_status = PTZ_WAIT_INIT,
		.step_degree = 16,
		.fulldst_step = 2048,
		.cycle_step = 2048,

		.runto_speed = MAX_SPEED,
		.hit_speed = MAX_SPEED,
		.step_speed = 15,
		.fulldst_step = 2048,
		.cycle_step = 2048,
	},

	/// 垂直电机。
	{
		.fd = -1,
		.mt_status = PTZ_WAIT_INIT,
		.step_degree = 8,

		.runto_speed = MAX_SPEED,
		.hit_speed = MAX_SPEED,
		.step_speed = 15,
	},
};

pthread_t motor_thread[AK_MOTOR_DEV_NUM];


/**
 * 通过 ioctl 获取当前电机位置。
 */
static int __get_motor_step (struct ak_motor *motor, int def) {

	struct motor_message Msg;

	if (NULL == motor) {
		return def;
	}

	if (ioctl (motor->fd, MOTOR_GET_STATUS, &Msg) != 0) {
		return def;
	}

	return Msg.pos;
}


/**
 * 角度转步数。
 */
#define degree2step(__motor, __degree)  ( (__motor)->cycle_step * (__degree) / 360 )

/**
 * 步数转角度。
 */
#define step2degree(__motor, __steps)  ( (__steps) * 360 / (__motor)->cycle_step )

static void motor_revise_param(struct ak_motor *motor)
{
	motor->nMax_hit = motor->fulldst_step * 360 /  motor->cycle_step;
}

static int motor_turn(struct ak_motor *motor, int steps, int is_cw)
{
	if(motor->fd < 0)
		return -1;

	int ret;
	int cmd = is_cw ? AK_MOTOR_TURN_CLKWISE:AK_MOTOR_TURN_ANTICLKWISE;

	ret = ioctl(motor->fd, cmd, &steps);
	if(ret) {
		set_error_no(ERROR_TYPE_DEV_CTRL_FAILED);
		ak_print_error_ex("%s fail. is_cw:%d, steps:%d, ret:%d.\n",
		    __func__, steps, is_cw, ret);
	}

	return ret;
}


static int get_motor_fd (enum ptz_device motor_no) {

	int fd = -1;

	if (PTZ_DEV_H == motor_no) {
		fd = akmotor[0].fd;
	} else if (PTZ_DEV_V == motor_no) {
		fd = akmotor[1].fd;
	} else {
		return -1;
	}

	return fd;
}



/**
 * set_turn_position_info - after motor turn, update position data
 * @motor[IN]:  ptr to motor control struct
 * @is_cw[IN]:  reserved
 * @degree[IN]:  relative degree at horizontal or vertica
 * @steps[IN]:  reserved
 * return: void
 */
static void set_turn_position_info(struct ak_motor *motor,
	int is_cw, int degree, int steps)
{
	if (is_cw) {
		motor->dg_cur += degree;
	} else {
		motor->dg_cur -= degree;
	}
}

/**
 * motor_turn_dg - control motor to turn, update position data
 * @motor[IN]:  ptr to motor control struct
 * @degree[IN]:  relative degree at horizontal or vertica
 * @is_cw[IN]:  1, clockwise; 0, anti-clockwise
 * return: 0, success; -1, failed
 */
static int motor_turn_dg(struct ak_motor *motor, int degree, int is_cw)
{
	if(motor->fd < 0)
		return -1;

	int res_angle = degree;
	int steps;
	int ret;

	//steps = calc_turn_steps(motor, &res_angle, is_cw);
	//if (steps <= 0) {
	//	ak_print_error_ex("[%s] turn steps:%d\n", __func__, steps);
	//	return -1;
	//}
	
	/// Try to Trun to the Max Rate.
	steps = degree2step (motor, degree);

	ret = motor_turn(motor, steps, is_cw);
	if(ret) {
		ak_print_error_ex(" fail. is_cw:%d, degree:%d, ret:%d.\n",
		    degree, is_cw, ret);
	} else {
		set_turn_position_info(motor, is_cw, res_angle, steps);
	}

	return ret;
}

static int motor_change_speed(struct ak_motor *motor, int ang_speed)
{
	int ret;
	int val = ang_speed;

	ret = ioctl(motor->fd, AK_MOTOR_SET_ANG_SPEED, &val);
	if(ret) {
		set_error_no(ERROR_TYPE_DEV_CTRL_FAILED);
		ak_print_error_ex(" fail.  ang_speed:%d, ret:%d.\n", ang_speed, ret);
	}

	return ret;
}

static int motor_check_event_timeout(time_t *base, int timeout)
{
	time_t t;

	t = time(0);

	/*caution : system sync time or zone */
	if (t - *base > 1800)
		*base = time(0);

	t -= *base;
	if (t > timeout)
		return 1;

	return 0;
}

/**
* motor_wait_hit_stop: check motor to stop and get remain steps that not finish
* @motor[IN]:    horizontal or vertical motor
* @remain_steps[IN,OUT]:  as IN, steps to turn; as OUT,remain steps that not finish
* return: 0 - success; otherwise -1;
*/
static int motor_wait_hit_stop(struct ak_motor *motor, int *remain_steps, int speed)
{
	fd_set fds;
	int ret = 0;
	struct timeval tv;
	time_t t, t2;
	int steps_per_second;
	int timeout;

	steps_per_second = degree2step (motor, speed);
	timeout = (*remain_steps) / steps_per_second + 30;

	//ak_print_normal_ex("motor_wait_hit_stop timeout: %d\n", timeout);
    struct notify_data notify;

	t = time(0);
start:
	t2 = time(0);
	tv.tv_sec	= timeout - (t2 - t);
	tv.tv_usec	= 0;
	FD_ZERO(&fds);
	FD_SET(motor->fd, &fds);
	ret = select(motor->fd+1, &fds, NULL, NULL, &tv);
	if(ret <= 0) {
		if (ret == 0)
			ak_print_info_ex(" select timeout.\n");
		return -1;
	}

	/*
	 * get notify data.
	 * if hit or stop, break; otherwise, continue
	 * when exit, get remain step
	 */
	ret = read(motor->fd, (void*)&notify, sizeof(struct notify_data));

	if (notify.event & AK_MOTOR_EVENT_HIT) {
		ret = 0;
	} else if (notify.event & AK_MOTOR_EVENT_STOP) {
		ret = -1;
	} else {
		ak_print_debug("not hit,motor->data.event = %d, %d\n",
			   	notify.event, notify.remain_steps);
		if (motor_check_event_timeout(&t, timeout)) {
			ak_print_info("%s event timeout.\n", __func__);
			return -1;
		}
		goto start;
	}

	*remain_steps = notify.remain_steps;
	return ret;
}

static void get_motor_param_ex(struct ak_motor *motor, enum sc_step *p_scs, int scs_size)
{
	/*test and get motr param*/
	int i;
	int rg;
	int const steps = motor->fulldst_step;
	enum sc_step scs;

	ak_print_debug_ex ("Motor Background Thread ID %08x\n", (unsigned int)ak_thread_get_tid());
	ak_print_debug_ex ("Full Distanse Step %d/%d\r\n", motor->fulldst_step, motor->cycle_step);

	/* let motor run fastly and save time */
	motor_change_speed(motor, motor->hit_speed);
	motor->mt_status = PTZ_SYS_WAIT;

	for (i = 0; i < scs_size; i++) {
		scs = *(p_scs + i);

		switch (scs) {
			case SC_CLKMAX:
				if (!motor_turn(motor, steps, CLKWISE)) {
					rg = steps;
					motor_wait_hit_stop(motor, &rg, motor->hit_speed);
				}
				if (0 == motor->nMax_hit) {
					motor_revise_param(motor);
				}
				
				motor->dg_cur = motor->nMax_hit;
				break;

			case SC_ACLKMAX:
				if (!motor_turn(motor, steps, ANTICLKWISE)) {
					rg = steps;
					motor_wait_hit_stop(motor, &rg, motor->hit_speed);

					ak_print_normal_ex("rg=%d\n", rg);
				}

				/* now can get the max angle that motor can turn */
				motor->nMax_hit = (unsigned int)step2degree (motor, steps - rg);
				ak_print_normal_ex("MaxHit=%d, steps:%d\n", motor->nMax_hit, steps - rg);
				/* set some default param to motor */
				motor_revise_param(motor);

				ak_print_normal_ex("Revised MaxHit=%d\n", motor->nMax_hit);

				/**/
				motor->dg_cur = 0;
				break;

			case SC_CLKMID:
				rg = motor->nMax_hit / 2;
				if (!motor_turn_dg(motor, rg, CLKWISE)) {

					rg = degree2step(motor, rg);

					motor_wait_hit_stop(motor, &rg, motor->hit_speed);
				}
				break;

			case SC_ACLKMID:
				rg = motor->nMax_hit / 2;
				if (!motor_turn_dg(motor, rg, ANTICLKWISE)) {

					rg = degree2step(motor, rg);

					motor_wait_hit_stop(motor, &rg, motor->hit_speed);
				}
				break;

			default:
				break;
		}
	}

	motor->mt_status = PTZ_INIT_OK;
}

/* control moter run and get motor param */
static void get_motor_param(struct ak_motor *motor)
{
	/*test and get motr param*/
	int rg;
	int const steps = motor->fulldst_step;

	ak_print_debug_ex ("Motor Background Thread ID %08x\r\n", (unsigned int)ak_thread_get_tid());
	ak_print_debug_ex ("Full Distanse Step %d/%d\r\n", motor->fulldst_step, motor->cycle_step);

	/* let motor run fastly and save time */
	motor_change_speed(motor, motor->hit_speed);
	motor->mt_status = PTZ_SYS_WAIT;


	if(PTZ_FEEDBACK_PIN_EXIST == motor->correct_type) {
		if (!motor_turn(motor, steps, CLKWISE)) {
			rg = steps;
			motor_wait_hit_stop(motor, &rg, motor->hit_speed);
		}
	}
	rg = 0;

	/* motor turn anti-clockwise  until hit endpoint */
	if (!motor_turn(motor, steps, ANTICLKWISE)) {
		rg = steps;
		motor_wait_hit_stop(motor, &rg, motor->hit_speed);
		//ak_print_normal_ex("rg=%d\n", rg);
	}

	/* now can get the max angle that motor can turn */
	motor->nMax_hit = (unsigned int)step2degree(motor, steps - rg);
	ak_print_normal_ex("MaxHit=%d, steps:%d\n", motor->nMax_hit, steps - rg);
    /* set some default param to motor */
	motor_revise_param(motor);

	ak_print_normal_ex("Revised MaxHit=%d\n", motor->nMax_hit);

	/**/
	motor->dg_cur = 0;

	/*
	 * run to middle of max angle that motor can reach
	 * let ptz at right position
	 */
//	rg = motor->nMax_hit / 2;
	rg = step2degree (motor, motor->reset_step);
	if (!motor_turn_dg(motor, rg, CLKWISE)) {
		rg = degree2step(motor, rg);
		motor_wait_hit_stop(motor, &rg, motor->hit_speed);
	}

	motor->mt_status = PTZ_INIT_OK;
}

/**
 * motor_turn_thread - excute command and control moter run
 * @data[IN]:  reserved
 * return: null
 */
static void* motor_turn_thread(void* data)
{
	struct ak_motor *motor = data;
	int rg;

	ak_print_normal_ex("thread id: %ld\n", ak_thread_get_tid());
	while(motor->run_flag){
		ak_thread_sem_wait(&motor->ctrl_sem);
		if( 0 == motor->run_flag )
			break;
		ak_print_debug_ex("clkwise=%d, angle=%d\n",
		    motor->cmd_data.clkwise,motor->cmd_data.dg);
        motor->mt_status = PTZ_SYS_WAIT;

		/*
		 * if step is long, turn speed should fast ,
		 * otherwise, use normal speed
		 */
		//if(motor->cmd_data.dg > motor->step_degree)
			motor_change_speed(motor, motor->runto_speed);
		//else
			//motor_change_speed(motor, motor->step_speed);
		if (!motor_turn_dg(motor, motor->cmd_data.dg, motor->cmd_data.clkwise)) {
			rg = degree2step(motor, motor->cmd_data.dg);

			motor_wait_hit_stop(motor, &rg, motor->runto_speed);
		}

		motor->mt_status = PTZ_INIT_OK;
	}

	ak_print_normal_ex("### thread id: %ld exit ###\n", ak_thread_get_tid());
	ak_thread_exit();

	return NULL;
}

/* get motor param and create thread to excute command */
static void start_motor(void)
{
    int i=0;

	/*
	 * create thread to calculate motor run angle
	 * support 2 motor and create 2 thread
	 */
	for (i=0; i<AK_MOTOR_DEV_NUM; ++i) {
        if((akmotor[i].mt_status == PTZ_WAIT_INIT) && (akmotor[i].fd >= 0)) {
            get_motor_param(&akmotor[i]);
    	}
	}
	ak_print_info_ex("get motor param OK\n");

	/*
	 * create thread and control motor to run
	 * support 2 motor and create 2 thread
	 */
	for (i=0; i<AK_MOTOR_DEV_NUM; ++i) {
        if(akmotor[i].mt_status == PTZ_INIT_OK) {
            ak_thread_create(&motor_thread[i], motor_turn_thread,
    				&akmotor[i], ANYKA_THREAD_MIN_STACK_SIZE, -1);
        }
	}
}

/* get motor param and create thread to excute command */
static void start_motor_ex(enum sc_step *scs, int scs_size)
{
    int i=0;

	/*
	 * create thread to calculate motor run angle
	 * support 2 motor and create 2 thread
	 */
    if (scs_size > 0) {
		for (i=0; i<AK_MOTOR_DEV_NUM; ++i) {
			if((akmotor[i].mt_status == PTZ_WAIT_INIT) && (akmotor[i].fd >= 0)) {
				get_motor_param_ex(&akmotor[i], scs, scs_size);
			}
		}
    } else {
    	for (i=0; i<AK_MOTOR_DEV_NUM; ++i) {
			akmotor[i].mt_status = PTZ_INIT_OK;
			motor_revise_param (&akmotor[i]);
		}
    }
	ak_print_info_ex("get motor param OK\n");

	/*
	 * create thread and control motor to run
	 * support 2 motor and create 2 thread
	 */
	for (i=0; i<AK_MOTOR_DEV_NUM; ++i) {
        if(akmotor[i].mt_status == PTZ_INIT_OK) {
            ak_thread_create(&motor_thread[i], motor_turn_thread,
    				&akmotor[i], ANYKA_THREAD_MIN_STACK_SIZE, -1);
        }
	}
}

/**
 * open_motor - open motor device
 * @motor[IN]:  ptr to moter control struct
 * @name[IN]:  motor device file which create by kernel
 * return: 0 - success; otherwise -1;
 */
static int open_motor(struct ak_motor *motor, char *name)
{
	motor->fd = open(name, O_RDWR);
	if(motor->fd < 0){
		set_error_no(ERROR_TYPE_DEV_OPEN_FAILED);
		ak_print_error_ex("open %s fail\n",name);
		return -1;
	}

	return 0;
}

/**
 * check_dg - check dg if it is right, fix it
 * @motor[IN]:  ptr to moter control struct
 * @degree[IN]:  relative degree at horizontal or vertical
 * return: 0 - success; otherwise -1;
 */
static int check_dg (struct ak_motor *motor, int degree)
{

	int dg = 0;
    int set_step = degree2step (motor, degree);
    int cur_step = __get_motor_step (motor, set_step); ///< 获取当前位置。
    int step = set_step - cur_step;

    /// 最大最小值限定。
    set_step = set_step >= 0 ? set_step : 0;
    set_step = set_step < motor->fulldst_step ? set_step : motor->fulldst_step - 1;

    /// 算出步数偏差。
    step = set_step - cur_step;
	if (step == 0) {
		/// 不需要动
		return -1;
	}

	/// 转换成角度。
	dg = step2degree (motor, step);
	ak_print_debug_ex ("Move from %d to %d Step %d Degree %d\r\n", cur_step, set_step, step, dg);

	/// 更新指令集。
	motor->cmd_data.clkwise = (dg > 0)? 1:0;
	if (dg < 0) {
		dg = -dg;
	}

	motor->cmd_data.dg = dg;

	return 0;
}

const char* ak_drv_ptz_get_version(void)
{
	return drv_ptz_version;
}

/**
 * ak_drv_ptz_open - init ptz data struct,motor selftest, creat thread for command
 * return: 0 - success; otherwise -1;
 */
int ak_drv_ptz_open(void)
{
	if(_calibrated){
		ak_print_error_ex("already opened.\n");
		return -1;
	}
	if (open_motor(&akmotor[0], AK_MOTOR_DEV0) != 0){
		ak_print_error_ex("open motor fail\n");
	    return -1;
	}
	if (open_motor(&akmotor[1], AK_MOTOR_DEV1) != 0){
		ak_print_error_ex("open motor fail\n");
		close(akmotor[0].fd);
		akmotor[0].fd = -1;
	    return -1;
	}

	return 0;
}





int ak_drv_ptz_setup_step_param (enum ptz_device motor_no, int cycle_step, int fulldst_step, int init_pos) {

	int fd = get_motor_fd (motor_no);
	struct motor_parm Param;
	struct motor_message Msg;

	if (fd < 0) {
		return -1;
	}

	/// 缺省初始位置。
	if (init_pos < 0) {
		init_pos = fulldst_step / 2;
	}

	Param.boundary_steps = 0;
	Param.pos = init_pos;
	Param.speed_step = 400;
	Param.steps_one_circle = cycle_step;
	Param.total_steps = fulldst_step;

	if (ioctl (fd, MOTOR_PARM, &Param) != 0) {
		return -1;
	}

	if (ioctl (fd, MOTOR_GET_STATUS, &Msg) != 0) {
		return -1;
	}

	/// 更新步幅范围。
	akmotor[motor_no].fulldst_step = Msg.total_steps;
	akmotor[motor_no].cycle_step = Msg.steps_one_circle;
	akmotor[motor_no].reset_step = init_pos;

	return 0;
}


int ak_drv_ptz_get_step_pos (enum ptz_device motor_no) {

	int fd = get_motor_fd (motor_no);
	struct motor_message Msg;

	if (fd < 0) {
		return -1;
	}

	if (ioctl (fd, MOTOR_GET_STATUS, &Msg) != 0) {
		return -1;
	}


	return Msg.pos;
}



/**
 * ak_drv_ptz_check_self - motor check it self
 * @pin_type[IN]: feedback pin type in 'enum ptz_feedback_pin'
 * return: 0 success; -1 failed
 */
int ak_drv_ptz_check_self(enum ptz_feedback_pin pin_type)
{
    int ret = AK_FAILED;

	if(akmotor[0].fd > 0){
	    int i = 0;

	    for (i=0; i<AK_MOTOR_DEV_NUM; ++i) {
            akmotor[i].run_flag = 1;
            akmotor[i].correct_type = pin_type;
		    ak_thread_sem_init(&akmotor[i].ctrl_sem, 0);
    	}

		start_motor();

		_calibrated = AK_TRUE; ///< 自检标识。
		ret = AK_SUCCESS;
	}

	return ret;
}

/**
 * ak_drv_ptz_check_self_ex - motor check it self
 * @pin_type[IN]: feedback pin type in 'enum ptz_feedback_pin'
 * @scs[IN]: self check step array
 * @scs_size[IN]: size of self check step array
 * return: 0 success; -1 failed
 */
int ak_drv_ptz_check_self_ex (enum ptz_feedback_pin pin_type, enum sc_step *scs, int scs_size)
{
    int ret = AK_FAILED;

	if(akmotor[0].fd > 0){
	    int i = 0;

	    for (i=0; i<AK_MOTOR_DEV_NUM; ++i) {
            akmotor[i].run_flag = 1;
            akmotor[i].correct_type = pin_type;
		    ak_thread_sem_init(&akmotor[i].ctrl_sem, 0);
    	}

		start_motor_ex(scs, scs_size);

		_calibrated = AK_TRUE;
		ret = AK_SUCCESS;
	}

	return ret;
}

/**
 * ak_drv_ptz_reset_dg: reset degree for calibrate.
 * @motor_no[IN]: motor device No. in 'enum ptz_device'
 * return: 0 success, -1 failed
 */
int ak_drv_ptz_reset_dg(enum ptz_device motor_no)
{
	/* check ptz if inited */
	if(0 == _calibrated){
		set_error_no(ERROR_TYPE_NOT_INIT);
		ak_print_error_ex("not init.\n");
    	return -1;
	}

	/* we support no more than 2 motor */
    if((motor_no < PTZ_DEV_H) || (motor_no >= PTZ_DEV_NUM)){
		set_error_no(ERROR_TYPE_INVALID_ARG);
		ak_print_error_ex("para err\n");
    	return -1;
    }

	akmotor[motor_no].dg_cur = 0;	
	return 0;
}


/**
 * ak_drv_ptz_turn: command motor to turn to the relative position
 * @direct[IN]: ptz turn direction, [left,right,up,down]
 * @degree[IN]: relative degree of horizontal or vertical to current position
 * return: 0 success; -1 failed
 * note: degree of one circle is 360.
 */
int ak_drv_ptz_turn(enum ptz_turn_direction direct, int degree)
{
    int ret = 0;

	/* check ptz if inited */
	if( 0 == _calibrated ){
		set_error_no(ERROR_TYPE_NOT_INIT);
		ak_print_error_ex("not init.\n");
    	return -1;
	}
	if((direct < PTZ_TURN_LEFT) || (direct > PTZ_TURN_DOWN)){
		set_error_no(ERROR_TYPE_INVALID_ARG);
		ak_print_error_ex("para err.\n");
		return -1;
	}

	struct ak_motor* motor = NULL;
    switch(direct){
  	  case PTZ_TURN_LEFT:
        motor = &akmotor[0];
		motor->cmd_data.clkwise = 1;
		break;
      case PTZ_TURN_RIGHT:
         motor = &akmotor[0];
		motor->cmd_data.clkwise = 0;
		break;
      case PTZ_TURN_UP:
        motor = &akmotor[1];
		motor->cmd_data.clkwise = 1;
		break;
      case PTZ_TURN_DOWN:
        motor = &akmotor[1];
		motor->cmd_data.clkwise = 0;
		break;
      default:
        ret = -1;
		break;
  	}

	if(motor != NULL && motor->mt_status != PTZ_INIT_OK){
		//set_error_no(ERROR_TYPE_NOT_INIT);
		ak_print_notice_ex("moter not idle.status:%d\n", motor->mt_status );
		ret = -1;
	} else if( -1 != ret) {
	    motor->cmd_data.dg = degree;
		/* start moter thread to execute cmd */
	    ak_thread_sem_post(&motor->ctrl_sem);
	}

	return ret;
}

/**
 * ak_drv_ptz_turn_to_pos: command motor to turn to the object position
 * @left_degree[IN]:  absolute degree at  horizontal
 * @up_degree[IN]:  absolute degree of horizontal or vertical to start position
 * return: 0 success, -1 failed
 * note: degree of one circle is 360.
 */
int ak_drv_ptz_turn_to_pos(int h_degree, int v_degree)
{
	int ret = 0;

	/* check ptz if inited */
	if( 0 == _calibrated ){
		set_error_no(ERROR_TYPE_NOT_INIT);
		ak_print_error_ex("not init.\n");
    	return -1;
	}
	/* ptz is not idle */
	if(akmotor[0].mt_status != PTZ_INIT_OK
		|| akmotor[1].mt_status != PTZ_INIT_OK) {
		//set_error_no(ERROR_TYPE_NOT_INIT);
		ak_print_notice_ex("not idle\n");
		return -1;
	}

	ak_print_debug_ex ("Turn to (%d, %d)\r\n", h_degree, v_degree);
	if( 0 == check_dg(&akmotor[0], h_degree)) {
		ak_thread_sem_post(&akmotor[0].ctrl_sem);
	}else {
		//set_error_no(ERROR_TYPE_INVALID_ARG);
		//ak_print_error_ex("para err\n");
		ret = -1;
	}

	if( 0 == check_dg(&akmotor[1], v_degree)){
		ak_thread_sem_post(&akmotor[1].ctrl_sem);
	}else{
		ret = -1;
	}

	return ret;
}

int ak_drv_ptz_turn_stop(enum ptz_turn_direction direct)
{

	int ret = -1;
	struct ak_motor* motor = NULL;

	/* check ptz if inited */
	if( 0 == _calibrated ){
		set_error_no(ERROR_TYPE_NOT_INIT);
		return -1;
	}

	if((direct ==PTZ_TURN_LEFT) ||(direct ==PTZ_TURN_RIGHT)) {
		motor = &akmotor[0];
	} else if((direct ==PTZ_TURN_UP) ||(direct ==PTZ_TURN_DOWN)) {
		motor = &akmotor[1];
	} else {
		return -1;
	}

	ret=ioctl(motor->fd,AK_MOTOR_TURN_STOP,NULL);
	if(ret<0)
	{
		ak_print_error_ex("motor set stop fail ret=%d\n",ret);
	}

	return ret ;

}

/**
 * ak_drv_ptz_get_status: get appointed motor status.
 * @motor_no[IN]: motor device No. in 'enum ptz_device'
 * @status[OUT]: motor status
 * return: 0 success, -1 failed
 */
int ak_drv_ptz_get_status(enum ptz_device motor_no, enum ptz_status *status)
{
	/* check ptz if inited */
	if(0 == _calibrated){
		set_error_no(ERROR_TYPE_NOT_INIT);
		ak_print_error_ex("not init.\n");
    	return -1;
	}

	/* we support no more than 2 motor */
    if(!status || (motor_no < PTZ_DEV_H) || (motor_no >= PTZ_DEV_NUM)){
		set_error_no(ERROR_TYPE_INVALID_ARG);
		ak_print_error_ex("para err\n");
    	return -1;
    }

	*status = akmotor[motor_no].mt_status;

	return 0;
}

/**
 * ak_drv_ptz_set_speed: set appointed motor speed.
 * @motor_no[IN]: motor device No. in 'enum ptz_device'
 * @speed[IN]: speed, [15, 100]
 * return: 0 success, -1 failed
 */
int ak_drv_ptz_set_speed(enum ptz_device motor_no, int speed)
{
	int runto_speed = speed;
	
	/* check ptz if inited */
	if(0 == _calibrated){
		set_error_no(ERROR_TYPE_NOT_INIT);
		ak_print_error_ex("not init.\n");
    	return -1;
	}

	/* we support no more than 2 motor */
    if((motor_no < PTZ_DEV_H) || (motor_no >= PTZ_DEV_NUM)){
		set_error_no(ERROR_TYPE_INVALID_ARG);
		ak_print_error_ex("para err\n");
    	return -1;
    }

	if (runto_speed > MAX_SPEED)
		runto_speed = MAX_SPEED;

	if (runto_speed < 15)
		runto_speed = 15;
	
	akmotor[motor_no].runto_speed = runto_speed;	
	return 0;
}


/**
 * ak_drv_ptz_close - close motor, release motor resource
 * return: 0 success, -1 failed
 */
int ak_drv_ptz_close(void)
{
	if(_calibrated){
		/* exit moter thread */
		int i=0;

		for (i=0; i<AK_MOTOR_DEV_NUM; ++i) {
            akmotor[i].run_flag = 0;
    		ak_thread_sem_post(&akmotor[i].ctrl_sem);
    		ak_print_notice_ex("join motor turn %d thread...\n", i);
        	ak_thread_join(motor_thread[i]);
        	ak_print_notice_ex("motor turn %d thread join OK\n", i);

        	if(akmotor[i].fd >= 0) {
                close(akmotor[i].fd);
    		    akmotor[i].fd = -1;
        	}
		}

		_calibrated = 0;
	}

	return AK_SUCCESS;
}
