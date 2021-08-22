#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/prctl.h>

#include "ak_common.h"
#include "ak_thread.h"

static const char thread_version[] = "libplat_thread V1.1.00";

const char* ak_thread_get_version(void)
{
	return thread_version;
}

/**
 * ak_thread_create - create a posix thread.
 * @thread_id[OUT]: thread id
 * @func[IN]: thread function
 * @arg[IN]: argument for thread
 * @stack_size[IN]: thread stack size
 * @priority[IN]: thread priority, default -1
 * return: 0 success; -1 failed
 */
int ak_thread_create(ak_pthread_t *id, thread_func func, void *arg,
		int stack_size, int priority)
{
	int ret = AK_FAILED;
	ak_pthread_attr_t sched_attr;

	/* Init thread attribute */
	ret = pthread_attr_init(&sched_attr);
	if (ret) {
		ak_print_error_ex("pthread_attr_init, ret: %d, %s\n",
			ret, strerror(ret));
		return AK_FAILED;
	}

	if (priority != -1) {
		ret = pthread_attr_setinheritsched(&sched_attr, PTHREAD_EXPLICIT_SCHED);
		if (ret) {
			ak_print_error_ex("setinheritsched, ret: %d, %s\n",
				ret, strerror(ret));
			goto pthread_create_end;
		}

		ret = pthread_attr_setschedpolicy(&sched_attr, SCHED_RR);
		if (ret) {
			ak_print_error_ex("setschedpolicy, ret: %d, %s\n",
				ret, strerror(ret));
			goto pthread_create_end;
		}

		struct sched_param sched_par;
		sched_par.sched_priority = priority;
		ret = pthread_attr_setschedparam(&sched_attr, &sched_par);
		if (ret) {
			ak_print_error_ex("setschedparam, ret: %d, %s\n",
				ret, strerror(ret));
			goto pthread_create_end;
		}
	}

	ret = pthread_attr_setstacksize(&sched_attr, stack_size);
	if (ret) {
		ak_print_error_ex("setstacksize, ret: %d, %s\n", ret, strerror(ret));
		goto pthread_create_end;
	}

	ret = pthread_create(id, &sched_attr, func, arg);
	if (ret) {
		ak_print_error_ex("pthread_create, ret: %d, %s\n", ret, strerror(ret));
		goto pthread_create_end;
	}
	ret = AK_SUCCESS;

pthread_create_end:
	pthread_attr_destroy(&sched_attr);
	return ret;
}

int ak_thread_join(ak_pthread_t id)
{
	return pthread_join(id, NULL);
}

int ak_thread_detach(ak_pthread_t id)
{
    return pthread_detach(id);
}

int ak_thread_cancel(ak_pthread_t id)
{
	return pthread_cancel(id);
}

void ak_thread_exit(void)
{
	pthread_exit(NULL);
}

/* get pthread id, should include the <sys/syscall.h> head file */
long int ak_thread_get_tid(void)
{
	return syscall(SYS_gettid);
}

int ak_thread_mutex_init(ak_mutex_t *mutex, ak_mutexattr_t *attr)
{
	return pthread_mutex_init(mutex, attr);
}

int ak_thread_mutex_lock(ak_mutex_t *mutex)
{
	return pthread_mutex_lock(mutex);
}

int ak_thread_mutex_unlock(ak_mutex_t *mutex)
{
	return pthread_mutex_unlock(mutex);
}

/**
 * ak_thread_sem_init - init sem with init value
 * @sem[IN]: pointer to semaphore which you want to post
 * @value[IN]: init value
 * return: 0 success; -1 failed
 */
int ak_thread_mutex_destroy(ak_mutex_t *mutex)
{
	return pthread_mutex_destroy(mutex);
}

/**
 * ak_thread_sem_init - init sem with init value
 * @sem[IN]: pointer to semaphore which you want to post
 * @value[IN]: init value
 * return: 0 success; -1 failed
 */
int ak_thread_sem_init(ak_sem_t *sem, unsigned int value)
{
	return sem_init(sem, 0, value);
}

/**
 * ak_thread_sem_post - post sem, give waiter a wake up signal
 * @sem[IN]: pointer to semaphore which you want to post
 * return: 0 success; -1 failed
 */
int ak_thread_sem_post(ak_sem_t *sem)
{
	return sem_post(sem);
}

/**
 * ak_thread_sem_wait - wait sem, block
 * @sem[IN]: pointer to semaphore which you want to wait
 * return: 0 success; -1 failed
 */
int ak_thread_sem_wait(ak_sem_t *sem)
{
	return sem_wait(sem);
}

/**
 * ak_thread_sem_destroy - destroy sem
 * @sem[IN]: pointer to semaphore which you want to destroy
 * return: 0 success; -1 failed
 */
int ak_thread_sem_destroy(ak_sem_t *sem)
{
	return sem_destroy(sem);
}

/* ak_thread_sem_trywait - try wait semaphore */
int ak_thread_sem_trywait(ak_sem_t *sem)
{
	return sem_trywait(sem);
}

/* ak_thread_sem_timedwait- wait semaphore timeout version */
int ak_thread_sem_timedwait(ak_sem_t *sem, const struct timespec *abs_timeout)
{
	return sem_timedwait(sem, abs_timeout);
}

/**
 * ak_thread_set_name - set thread name
 * @name[IN]: pointer to 'thread name' no longer than 15B
 * return: 0 success; -1 failed
 */
int ak_thread_set_name(const char *name)
{
	if (!name) {
		ak_print_error_ex("invalid name\n");
	}

	char th_name[16] = {0};
	strncpy(th_name, name, 15);

	return prctl(PR_SET_NAME, th_name);
}

/*
 * ak_thread_cond_init - init thread condition value
 */
int ak_thread_cond_init(ak_cond_t *cond)
{
	return pthread_cond_init(cond, NULL);
}

/* ak_thread_cond_destroy - destroy thread condition value */
int ak_thread_cond_destroy(ak_cond_t *cond)
{
	return pthread_cond_destroy(cond);
}

/* ak_thread_cond_wait - wait thread condition */
int ak_thread_cond_wait(ak_cond_t *cond, ak_mutex_t *mutex)
{
	return pthread_cond_wait(cond, mutex);
}

/* ak_thread_cond_timewait - wait thread condition timeout version */
int ak_thread_cond_timedwait(ak_cond_t *cond, ak_mutex_t *mutex,
	   	const struct timespec *abstime)
{
	return pthread_cond_timedwait(cond, mutex, abstime);
}

/* ak_thread_cond_signal - thread condition notify */
int ak_thread_cond_signal(ak_cond_t *cond)
{
	return pthread_cond_signal(cond);
}

/* ak_thread_cond_broadcast - thread condition broadcast notify */
int ak_thread_cond_broadcast(ak_cond_t *cond)
{
	return pthread_cond_broadcast(cond);
}

/* ak_thread_mutexattr_init - init thread mutexattribute */
int ak_thread_mutexattr_init(ak_mutexattr_t *attr)
{
	return pthread_mutexattr_init(attr);
}

/* ak_thread_mutexattr_destroy - destroy thread mutexattribute */
int ak_thread_mutexattr_destroy(ak_mutexattr_t *attr)
{
	return pthread_mutexattr_destroy(attr);
}

/* ak_thread_mutexattr_settype - set attribute type */
int ak_thread_mutexattr_settype(ak_mutexattr_t *attr, int kind)
{
	return pthread_mutexattr_settype(attr, kind);
}

/* ak_thread_mutexattr_gettype - get attribute type */
int ak_thread_mutexattr_gettype(const ak_mutexattr_t *attr, int *kind)
{
	return pthread_mutexattr_gettype(attr, kind);
}

