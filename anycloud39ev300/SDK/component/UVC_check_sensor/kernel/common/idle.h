#ifndef __IDLE_H__
#define __IDLE_H__

/**
 * @brief 	get cpu idle percent
 * @author 
 * @return 	unsigned long 
 * @retval	unsigned long cpu idle percent
 */ 
unsigned long idle_get_cpu_idle(void);


/**
 * @brief 	get cpu usage percent
 * @author 
 * @return 	unsigned long 
 * @retval	unsigned long cpu usage percent
 */ 
unsigned long idle_get_cpu_usage(void);

/**
 * @brief 	Create A Idle Thread
 * @author 	
 * @return 	bool 
 * @retval	false	Failure
 * @retval	true	Success
 */ 
bool idle_thread_create(void);

/**
 * @brief 	Destroy The Idle Thread
 * @author 	
 * @return 	bool
 * @retval	fasle	Failure
 * @retval	true	Success
 */ 
bool idle_thread_destroy(void);

/**
 * @brief 	Query The Idle Thread Is Running
 * @author 	
 * @return 	bool
 * @retval	false	NO A Idle Thead Is running
 * @retval	true	The Idle Thead Is running
 */ 
bool idle_thread_is_created(void);






#endif
