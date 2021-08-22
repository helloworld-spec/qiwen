
/**
 * @file 
 * @brief:
 *
 * This file provides 
 * Copyright (C) 2017 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author 
 * @date 2017-2-09
 * @version 1.0
 */

#include "print.h"
#include <string.h>  //for "list.h"  size_t
#include "list.h"
#include "ioctl.h"

#ifndef __DEVICE_H__
#define __DEVICE_H__

#define OS_TICK (5)


#define DEV_MAJOR_ID_BEGIN 100
#define DEV_MINOR_ID_BEGIN 0

#define DEV_ID_MAJOR_MAX (DEV_MAJOR_ID_BEGIN + 30)
#define DEV_ID_MINOR_MAX (DEV_MINOR_ID_BEGIN + 4)


#define PLATFORM_DEV_MAX_NB  ((DEV_ID_MAJOR_MAX - DEV_MAJOR_ID_BEGIN + 1)*(DEV_ID_MINOR_MAX - DEV_MINOR_ID_BEGIN+ 1))


#define MAJOR_ID_OFFSET  (16)
#define MINORMASK	((1U << MAJOR_ID_OFFSET) - 1)

#define MAJOR(dev)	((unsigned int) ((dev) >> MAJOR_ID_OFFSET))
#define MINOR(dev)	((unsigned int) ((dev) & MINORMASK))



typedef long    T_DrvMutex;
typedef long    T_DrvSem;
typedef void    (*f_Drv_Complete)(int dev_id);   



typedef struct
{
	/*
	* device_id:高16位位主设备号，低16位为次设备号，组成唯一的设备号
	*/
	int  device_id;


	/*
	* drv_open_func :
	*
	* dev_id  : 设备号
	* data     : 设备驱动需要传入参数的结构体地址。如uart的波特率等
	*
	*return    : 成功返回设备号
	*              失败返回负数，具体哪种错误参考系统的Error Code
	*/
	int            (*drv_open_func) (int dev_id, void *data);

	/*
	* drv_close_func :
	* dev_id  : 设备号
	*
	*return    : 成功返回0
	*              失败返回-1
	*/
    int            (*drv_close_func)(int dev_id);

	/*
	* drv_read_func:
	* dev_id  : 设备号
	* data   : 存放数据的地址
	* len     : 读数据的长度
	*
	*return  : 成功返回已读有效数据的长度
	*            失败返回负数，具体哪种错误参考系统的Error Code
	*/	
    int            (*drv_read_func) (int dev_id, void *data, unsigned int len);

	/*
	* drv_write_func:
	* dev_id  : 设备号
	* data   : 存放数据的地址
	* len     : 写数据的长度
	*
	*return  : 成功返回已写有效数据的长度
	*            失败返回负数，具体哪种错误参考系统的Error Code
	*/
    int            (*drv_write_func)(int dev_id, const void  *data, unsigned int len);

	/*
	* drv_ioctl_func:
	* dev_id  : 设备号	
	* cmd   : ioctl命令。
	*           bit31-30  : 读写属性.
	*             0      0   : 没有参数
	*             0      1   : 写
	*             1      0   : 读
	*             1      1   : 读写
	*
	*		bit29-16  : 参数个数
	*		bit15-8    :对于一类设备驱动的命令，要一一对应
	*	       bit 7-0    :每一种命令对应相应功能
	* 		
	*data   : iocltl命令参数的结构体地址
	*
	*return  : 成功返回根据实际的驱动设备而定(待改进)
	*            失败返回负数，具体哪种错误参考系统的Error Code
	*/	
	int            (*drv_ioctl_func)(int dev_id, unsigned long cmd, void *data);
	
	/*
	*read_sem:信号量用于异步通知
	*/	
	T_DrvSem       read_sem;

	/*
	*read_complete:注册回调函数，实现异步通知，函数参数为read_sem
	*/		
	f_Drv_Complete read_complete;

	/*
	*read_sem:信号量用于异步通知
	*/		
	T_DrvSem       write_sem;
	
	/*
	*write_complete:注册回调函数，实现异步通知，函数参数为write_sem
	*/	
	f_Drv_Complete write_complete;

	/*
	*具体设备驱动需要额外的驱动操作或参数等的集合
	*/		
	void		   *drv_data;	

	/*
	*驱动对应的设备。
	*/		
	void		   *devcie;	
	
}
T_DEV_DRV_HANDLER;





typedef struct
{
	/*
	* dev_open_flg:设备是否打开。
	*/
	volatile bool dev_open_flg; 
	
	/*
	* dev_name:指向设备名字符串的指针(设备名称尽量简洁，字符太多直接影响匹配的时间)
	*/	
	char *dev_name;	
	
	/*
	* dev_id:高16位位主设备号，低16位为次设备号，组成唯一的设备号
	*/
	int dev_id; 

	/*
	*read_mutex_lock:用于临界资源的保护
	*/
	T_DrvMutex     read_mutex_lock;
	
	/*
	*write_mutex_lock:用于临界资源的保护
	*/	
	T_DrvMutex     write_mutex_lock;

	/*
	*ioctl_mutex_lock:用于临界资源的保护
	*/	
	T_DrvMutex     ioctl_mutex_lock;
	
	/*
	* dev_data:具体每个设备的私有数据。
	*/	
	void *dev_data;

	/*
	* drv_handler:指向设备的驱动结构
	*/		
    T_DEV_DRV_HANDLER *drv_handler;

	/*
	* list:设备链表节点
	*/		
	struct list_head list;
}T_DEV_INFO;



/** 
 * @brief  alloc  devcie  ID
 *
 * @author KeJianping
 * @date 2017-2-09
 * @param devcie[in] device .
 * @param major[in] 0, or you know the major number.
 * @param name[in] devcie's name.
 * @return  int
 * @retval  < 0 :  failed
 * @retval = 0 : successful
 */
int dev_alloc_id(T_DEV_INFO *devcie, int major, const char* name);


/** 
 * @brief    devcie  register
 *
 * @author KeJianping
 * @date 2017-2-09
 * @param dev_id[in] device  ID.
 * @param devcie[in] device.
 * @return  int
 * @retval  = 0 :  failed
 * @retval  = 1 : successful
 */
bool dev_drv_reg(int dev_id, T_DEV_INFO *devcie);

 
#endif // #ifndef __DEVICE_H__


