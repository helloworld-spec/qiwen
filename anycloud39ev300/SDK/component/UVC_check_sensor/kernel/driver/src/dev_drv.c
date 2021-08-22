/**
 * @file dev_drv.c
 * @brief:device driver source file
 *
 * Copyright (C) 2017 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author KeJianping
 * @date 2017-2-09
 * @version 1.0
 */


#include "akos_api.h"
#include "arch_init.h"

#include "dev_drv.h"
#include "device.h"

#ifdef __cpluscplus
extern "C"{
#endif



typedef struct
{
	char *dev_name;
	unsigned long open_timers;
	T_hSemaphore sem;
	unsigned char sem_init_flg;
	long         block;
	T_DEV_INFO *devcie;

}T_DEV_INFO_TABLE;


#define dev_err_printk(fmt, arg...) \
	printk("\n[%s:%d] "fmt, __func__, __LINE__, ##arg)

LIST_HEAD(dev_list);

static T_DEV_INFO_TABLE  dev_info_table[DEV_ID_MAJOR_MAX - DEV_MAJOR_ID_BEGIN + 1][DEV_ID_MINOR_MAX - DEV_MINOR_ID_BEGIN + 1]= {0,};


const char* ak_dev_drv_get_version(void)
{

	return LIBDEV_DRV_VERSION;
}



/** 
 * @brief  alloc  devcie  ID
 *
 * @author KeJianping
 * @date 2017-2-09
 * @param devcie[in] device .
 * @param major[in] 0,自动分配。如果知道了具体的主设备号，就输入主设备号。
 * @param name[in] 设备名字指针，这个需要使用全局变量的地址。.
 * @return  int
 * @retval  < 0 :  failed
 * @retval = 0 : successful
 */
int dev_alloc_id(T_DEV_INFO *devcie, int major,const char* name)
{
	int  tmp_major = major;
	int  tmp_minor = DEV_MINOR_ID_BEGIN;
	int  ret = -1;
	static int major_id = DEV_MAJOR_ID_BEGIN;
	
	if(0 == tmp_major)
	{
		if(major_id < DEV_ID_MAJOR_MAX)
		{
			while( 0 != *(int*)(&dev_info_table[major_id][tmp_minor]))//主设备号没自动分配，而是手动误分配成功了。需要这个判断
			{
				tmp_minor++;
				if(tmp_minor >=  DEV_ID_MINOR_MAX)
				{
					dev_err_printk("devcie id alloc faile.\r\n");
					return ret;
				}
			}
			
			devcie->dev_id   = (major_id << MAJOR_ID_OFFSET)|tmp_minor; //minor = DEV_MINOR_ID_BEGIN;
			devcie->dev_name = (char*)name;
			major_id++;
			ret = 0;
		}
		else
		{
			dev_err_printk("devcie id alloc faile.\r\n");
		}
	}
	else
	{
		if(DEV_MAJOR_ID_BEGIN <= tmp_major && tmp_major < DEV_ID_MAJOR_MAX)
		{
			while( 0 != *(int*)(&dev_info_table[tmp_major][tmp_minor]))
			{
				tmp_minor++;				
				if(tmp_minor >=  DEV_ID_MINOR_MAX)
				{
					dev_err_printk("devcie id alloc faile.\r\n");
					return ret;
				}
			}
			devcie->dev_id   = (tmp_major << MAJOR_ID_OFFSET)|tmp_minor; //minor = DEV_MINOR_ID_BEGIN;
			devcie->dev_name = (char*)name;
			ret = 0;

		}
		else
		{
			dev_err_printk("NO %d major id.\r\n",major);
		}
	}

	return ret;
}



static unsigned long dev_get_major_id(int dev_id)
{
	return (dev_id >> MAJOR_ID_OFFSET)&((1<<MAJOR_ID_OFFSET) - 1);
}

static unsigned long dev_get_minor_id(int dev_id)
{
	return (dev_id)&((1<<MAJOR_ID_OFFSET) - 1);
}


static bool check_dev_id(int dev_id,unsigned long *major,unsigned long *minor )
{

	unsigned long major_id;
	unsigned long minor_id;
	
	major_id = dev_get_major_id(dev_id);
	minor_id = dev_get_minor_id(dev_id);
	
	if(major_id < DEV_MAJOR_ID_BEGIN ||  major_id >= DEV_ID_MAJOR_MAX || minor_id < DEV_MINOR_ID_BEGIN ||minor_id >= DEV_ID_MINOR_MAX)
	{
		dev_err_printk("devcie id err,major_id=%d,minor_id=%d\r\n",major_id,minor_id);
		return false;
	}

	*major = major_id - DEV_MAJOR_ID_BEGIN;
	*minor = minor_id - DEV_MINOR_ID_BEGIN;	
	return true;

}


static void dev_read_notify(int device_id)
{

	unsigned long major_id;
	unsigned long minor_id;
	int ret_sem;
			
	if(!check_dev_id(device_id,&major_id,&minor_id))
	{
		return ;
	}
	if(0 != dev_info_table[major_id][minor_id].block)
	{
		ret_sem = AK_Release_Semaphore(dev_info_table[major_id][minor_id].devcie->drv_handler->read_sem);
		if(AK_SUCCESS != ret_sem)
		{
			dev_err_printk("ID=%d,AK_Release_Semaphore faile,err=%d\r\n",device_id,ret_sem);
		}
	}
}

static void dev_write_notify(int dev_id)
{
	//TBD
	;
}

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
bool dev_drv_reg(int dev_id, T_DEV_INFO *devcie)	
{
	unsigned long major_id;
	unsigned long minor_id;
			
	if(!check_dev_id(dev_id,&major_id,&minor_id))
	{
		return false;
	}

	dev_info_table[major_id][minor_id].dev_name                 = devcie->dev_name;
	dev_info_table[major_id][minor_id].devcie                   = devcie;
	dev_info_table[major_id][minor_id].open_timers              = 0;
	dev_info_table[major_id][minor_id].sem                      = AK_INVALID_SEMAPHORE;	
	dev_info_table[major_id][minor_id].sem_init_flg             = 0;	
	dev_info_table[major_id][minor_id].block             		= 0;	

	
	dev_info_table[major_id][minor_id].devcie->dev_open_flg     = false;
	dev_info_table[major_id][minor_id].devcie->write_mutex_lock = AK_INVALID_SEMAPHORE;
	dev_info_table[major_id][minor_id].devcie->read_mutex_lock 	= AK_INVALID_SEMAPHORE;	
	dev_info_table[major_id][minor_id].devcie->ioctl_mutex_lock = AK_INVALID_SEMAPHORE;	

	dev_info_table[major_id][minor_id].devcie->drv_handler->read_sem       = AK_INVALID_SEMAPHORE;	
	dev_info_table[major_id][minor_id].devcie->drv_handler->write_sem      = AK_INVALID_SEMAPHORE;	
	dev_info_table[major_id][minor_id].devcie->drv_handler->read_complete  = dev_read_notify;	
	dev_info_table[major_id][minor_id].devcie->drv_handler->write_complete = dev_write_notify;	
	
	list_add(&devcie->list,&dev_list);

	return true;

}


static bool dev_mutex_lock_init(unsigned long major,unsigned long minor)
{
	int i;
	T_hSemaphore *pSem = NULL;
	unsigned long major_id = major;
	unsigned long minor_id = minor;
	
	T_hSemaphore * sem[]={
	&dev_info_table[major_id][minor_id].sem,
		
	&dev_info_table[major_id][minor_id].devcie->write_mutex_lock,	
	&dev_info_table[major_id][minor_id].devcie->read_mutex_lock,	
	&dev_info_table[major_id][minor_id].devcie->ioctl_mutex_lock,
	};

	for(i = 0; i < sizeof(sem)/sizeof(sem[0]); i++)
	{
	    pSem = sem[i] ;
	    if(AK_INVALID_SEMAPHORE == *pSem)
	    {
	        *pSem = AK_Create_Semaphore(1, AK_FIFO);
	        if(AK_INVALID_SUSPEND == *pSem)
	        {
	            *pSem = AK_INVALID_SEMAPHORE;
	            dev_err_printk("dev_open:[%d]create smph failed!\r\n",i);						
	            return false;
	        }					
    	}
	}
	
	return true;			
	
}


static bool drv_sem_init(unsigned long major,unsigned long minor)
{

	int i;
	T_hSemaphore *pSem = NULL;
	unsigned long major_id = major;
	unsigned long minor_id = minor;
	
	T_hSemaphore * sem[]={
	&dev_info_table[major_id][minor_id].devcie->drv_handler->read_sem,      
	&dev_info_table[major_id][minor_id].devcie->drv_handler->write_sem, 	
	};

	for(i = 0; i < sizeof(sem)/sizeof(sem[0]); i++)
	{
	    pSem = (T_hSemaphore *)sem[i] ;
	    if(AK_INVALID_SEMAPHORE == *pSem)
	    {
	        *pSem = AK_Create_Semaphore(0, AK_FIFO);
	        if(AK_INVALID_SUSPEND == *pSem)
	        {
	            *pSem = AK_INVALID_SEMAPHORE;
	            dev_err_printk("dev_open:[%d]create smph failed!\r\n",i);						
	            return false;
	        }					
    	}
	}
	
	return true;			
	
}



/** 
 * @brief  open devcie  with  devcie's name
 *
 * open the devcie, and then return devcie's ID. 
 * @author KeJianping
 * @date 2017-2-09
 * @param name[in] devcie's name
 * @return  int
 * @retval  < 0 :  failed
 * @retval  > 0 : successful
 */
int dev_open(const char *name)
{

	T_DEV_INFO *devcie = NULL;
	int ret = -1;
	int ret_sem = -1;
	unsigned long major_id;
	unsigned long minor_id;
	
	if(NULL ==  name)
	{
		dev_err_printk("NO devcie name\r\n");
		return ret;
	}

	list_for_each_entry(devcie,&dev_list,list)
	{
		if (0 == strcmp(devcie->dev_name,name))
		{
			if(!check_dev_id(devcie->dev_id,&major_id,&minor_id))
			{
				return ret;
			}

			if(0 == dev_info_table[major_id][minor_id].sem_init_flg)
			{
				if(!dev_mutex_lock_init(major_id,minor_id) || !drv_sem_init(major_id,minor_id))
				{
					return ret;	
				}
				dev_info_table[major_id][minor_id].sem_init_flg = 1;			
			}
			ret_sem = AK_Obtain_Semaphore(dev_info_table[major_id][minor_id].sem, AK_SUSPEND);
			if(AK_SUCCESS != ret_sem)
			{
				dev_err_printk("ID=%d,AK_Obtain_Semaphore faile,err=%d\r\n",devcie->dev_id,ret_sem);	
				return ret_sem;	
			}
			
			if(!devcie->dev_open_flg && NULL != devcie->drv_handler->drv_open_func)
			{
				ret = devcie->drv_handler->drv_open_func(devcie->dev_id, devcie);

				if(ret > 0)
				{
					devcie->dev_open_flg = true;
				}
				else
				{
					ret_sem = AK_Release_Semaphore(dev_info_table[major_id][minor_id].sem);
					if(AK_SUCCESS != ret_sem)
					{
						dev_err_printk("ID=%d,AK_Release_Semaphore faile,err=%d\r\n",devcie->dev_id,ret_sem);
						return ret_sem;	
					}
					break;
				}
				
			}
			else				
			{
				printk(" %s devcie open %d times\r\n",devcie->dev_name,dev_info_table[major_id][minor_id].open_timers+1);
				ret = devcie->dev_id;
			}
					
			dev_info_table[major_id][minor_id].open_timers++;

			ret_sem = AK_Release_Semaphore(dev_info_table[major_id][minor_id].sem);
			if(AK_SUCCESS != ret_sem)
			{
				dev_err_printk("ID=%d,AK_Release_Semaphore faile,err=%d\r\n",devcie->dev_id,ret_sem);
				return ret_sem;	
			}
			break;
			
		}
	}

	return ret;

}

/** 
 * @brief  close devcie  with  devcie's ID
 *
 * @author KeJianping
 * @date 2017-2-09
 * @param device_id[in] devcie's ID, dev_open() return value.
 * @return  int
 * @retval  < 0 :  failed
 * @retval = 0 : successful
 */
int dev_close(int device_id)
{
	unsigned long major_id;
	unsigned long minor_id;
	T_DEV_INFO *devcie     = NULL;

	int ret = -1;
	int ret_sem;

			
	if(!check_dev_id(device_id,&major_id,&minor_id))
	{
		return ret;
	}

	devcie = (T_DEV_INFO *)(dev_info_table[major_id][minor_id].devcie);

	if(NULL == devcie)
	{
		dev_err_printk("NO devcie\r\n");
		return ret;
	}
		
	ret = 0;
	
	if(devcie->dev_open_flg)
	{
		ret_sem = AK_Obtain_Semaphore(dev_info_table[major_id][minor_id].sem, AK_SUSPEND);
		if(AK_SUCCESS != ret_sem)
		{
			dev_err_printk("ID=%d,AK_Obtain_Semaphore faile,err=%d\r\n",device_id,ret_sem);	
			return ret_sem;	
		}
		
		dev_info_table[major_id][minor_id].open_timers--;//多次打开。最后一次关闭才真正关闭
		if(0 == dev_info_table[major_id][minor_id].open_timers)
		{
			if(NULL != devcie->drv_handler->drv_close_func)
			{
				ret = devcie->drv_handler->drv_close_func(device_id);
			}
			devcie->dev_open_flg = false;
			printk(" %s devcie close.\r\n",devcie->dev_name);
		}
		
		ret_sem = AK_Release_Semaphore(dev_info_table[major_id][minor_id].sem);
		if(AK_SUCCESS != ret_sem)
		{
			dev_err_printk("ID=%d,AK_Release_Semaphore faile,err=%d\r\n",device_id,ret_sem);
			return ret_sem;	
		}
	}

	return ret;

	
}


/** 
 * @brief  read data from devcie's driver.  
 *
 * APP read data. 
 * @author KeJianping
 * @date 2017-2-09
 * @param device_id[in] devcie's ID, dev_open() return value.
 * @param buf[out] buffer to store read data 
 * @param len[in] the length to read
 * @return  int
 * @retval  < 0 :  failed
 * @retval  >= 0 : successful
 */
int dev_read(int device_id,  void *buf, unsigned int len)
{
	unsigned long major_id;
	unsigned long minor_id;
	T_DEV_INFO *devcie     = NULL;

	int ret = -1;
	int ret_sem;


	if(!check_dev_id(device_id,&major_id,&minor_id))
	{
		return ret;
	}

	devcie = (T_DEV_INFO *)(dev_info_table[major_id][minor_id].devcie);

	if(NULL == devcie)
	{
		dev_err_printk("NO devcie\r\n");
		return ret;
	}

	if(devcie->dev_open_flg && NULL != devcie->drv_handler->drv_read_func)
	{
		ret_sem = AK_Obtain_Semaphore(devcie->read_mutex_lock, AK_SUSPEND);
		if(AK_SUCCESS != ret_sem)
		{
			dev_err_printk("ID=%d,AK_Obtain_Semaphore faile,err=%d\r\n",device_id,ret_sem);			
			return ret_sem;	
		}
		
		if(0 != dev_info_table[major_id][minor_id].block)
		{
			if(dev_info_table[major_id][minor_id].block < 0)
			{
				//阻塞
				ret_sem = AK_Obtain_Semaphore(devcie->drv_handler->read_sem, AK_SUSPEND);
				if(AK_SUCCESS != ret_sem)
				{
					dev_err_printk("ID=%d,AK_Obtain_Semaphore faile,err=%d\r\n",device_id,ret_sem);			
				}
			}
			else
			{
				//等待时间，除以5，OS调度时间为5ms，这个固定了，不能根据时间调整

				ret_sem = AK_Obtain_Semaphore(devcie->drv_handler->read_sem, dev_info_table[major_id][minor_id].block/OS_TICK);
				if(AK_SUCCESS != ret_sem)
				{
					if (AK_TIMEOUT == ret_sem)
					{
						dev_err_printk("ID=%d,AK_Obtain_Semaphore TIMEOUT\r\n",device_id);	
					}
					else
					{
						dev_err_printk("ID=%d,AK_Obtain_Semaphore faile,err=%d\r\n",device_id,ret_sem);			
					}
				}

			}	
			dev_info_table[major_id][minor_id].block = 0;	
		}

		ret = devcie->drv_handler->drv_read_func(device_id, buf,len);

		ret_sem = AK_Release_Semaphore(devcie->read_mutex_lock);
		if(AK_SUCCESS != ret_sem)
		{
			dev_err_printk("ID=%d,AK_Release_Semaphore faile,err=%d\r\n",device_id,ret_sem);
			return ret_sem;	
		}
		
	}
	
	return ret;

}

/** 
 * @brief  write data to devcie's driver. 
 *
 * APP write data
 * @author KeJianping
 * @date 2017-2-09
 * @param device_id[in] devcie's ID, dev_open() return value.
 * @param buf[in] buffer to store write data 
 * @param len[in] the length to read
 * @return  int
 * @retval  < 0 :  failed
 * @retval  >= 0 : successful
 */
int dev_write(int device_id, const void *buf, unsigned int len)
{

	unsigned long major_id;
	unsigned long minor_id;
	T_DEV_INFO *devcie     = NULL;

	int ret = -1;
	int ret_sem;
		
	if(!check_dev_id(device_id,&major_id,&minor_id))
	{
		return ret;
	}

	devcie = (T_DEV_INFO *)(dev_info_table[major_id][minor_id].devcie);

	if(NULL == devcie)
	{
		dev_err_printk("NO devcie\r\n");
		return ret;
	}

	if(devcie->dev_open_flg && NULL != devcie->drv_handler->drv_write_func)
	{	
		ret_sem = AK_Obtain_Semaphore(devcie->write_mutex_lock, AK_SUSPEND);
		if(AK_SUCCESS != ret_sem)
		{
			dev_err_printk("ID=%d,AK_Obtain_Semaphore faile,err=%d\r\n",device_id,ret_sem);	
			return ret_sem;	
		}
		
		ret = devcie->drv_handler->drv_write_func(device_id, buf,len);

		ret_sem = AK_Release_Semaphore(devcie->write_mutex_lock);
		if(AK_SUCCESS != ret_sem)
		{
			dev_err_printk("ID=%d,AK_Release_Semaphore faile,err=%d\r\n",device_id,ret_sem);
			return ret_sem;	
		}		
	}
	
	return ret;
	

}

/** 
 * @brief  set IO control command
 *
 * APP set ioctl command
 * @author KeJianping
 * @date 2017-2-09
 * @param device_id[in] devcie's ID, dev_open() return value.
 * @param cmd[in]  commamd to set 
 * @param data[in/out] depends on the command.
 * @return  int
 * @retval  < 0 :  failed
 * @retval  = 0 : successful
 */
int dev_ioctl(int device_id, unsigned long cmd, void *data)
{

	unsigned long major_id;
	unsigned long minor_id;
	T_DEV_INFO *devcie     = NULL;
	long * data_tmp = data;
	int ret = -1;
	int ret_sem;
	int cmd_data;
			
	if(!check_dev_id(device_id,&major_id,&minor_id))
	{
		return ret;
	}

	devcie = (T_DEV_INFO *)(dev_info_table[major_id][minor_id].devcie);

	if(NULL == devcie)
	{
		dev_err_printk("NO devcie\r\n");
		return ret;
	}

	if(devcie->dev_open_flg && NULL != devcie->drv_handler->drv_ioctl_func)
	{

		ret_sem = AK_Obtain_Semaphore(devcie->ioctl_mutex_lock, AK_SUSPEND);
		if(AK_SUCCESS != ret_sem)
		{
			dev_err_printk("ID=%d,AK_Obtain_Semaphore faile,err=%d\r\n",device_id,ret_sem);	
			return ret_sem;	
		}
		//dev_err_printk("=io=cmd=%d, %d\r\n",cmd, *data_tmp);

		if (_IOC_TYPE(cmd) == CMD_BLOCK)
		{
			cmd_data = _IOC_NR(cmd);
			
			switch (cmd_data) {
				case BLOCK_NO:
					dev_info_table[major_id][minor_id].block = 0;
					break;
				case BLOCK_WAIT:
					dev_info_table[major_id][minor_id].block = -1;
					break;					
				case BLOCK_MS:
					dev_info_table[major_id][minor_id].block = *data_tmp;
					break;
				default :
					dev_err_printk("IO command err.\r\n");
					break;
				}
		}
		else
		{
			ret = devcie->drv_handler->drv_ioctl_func(device_id, cmd, data);
		}

		ret_sem = AK_Release_Semaphore(devcie->ioctl_mutex_lock);
		if(AK_SUCCESS != ret_sem)
		{
			dev_err_printk("ID=%d,AK_Release_Semaphore faile,err=%d\r\n",device_id,ret_sem);
			return ret_sem;	
		}
	}
	
	return ret;

}



static int dev_info_table_init(void)
{
	int i,num;
	char *tmp;
	tmp = (char *)dev_info_table;
	num = sizeof(dev_info_table)/sizeof(dev_info_table[0])*sizeof(T_DEV_INFO_TABLE);
	for(i = 0; i < num; i++)
	{
		*tmp = 0;
	}
	return 0;

}

module_init(dev_info_table_init)


#ifdef __cplusplus
}
#endif


