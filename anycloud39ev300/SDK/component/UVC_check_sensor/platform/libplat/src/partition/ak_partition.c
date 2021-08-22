/**
 * @file ak_partition.c
 * @brief 
 *
 * This file provides partiton APIs: open partition, write data to partition, read data from partition,close partition
 * Copyright (C) 2016 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author 
 * @date 2016-12-21
 * @version 1.0
 */
#include <stdio.h>

#include "partition_lib.h"
#include "ak_partition.h"

static const char partition_version[] = "libplat_partition V2.0.00";

const char* ak_partition_get_version(void)
{
	return partition_version;
}

/** 
 * ak_partition_open - 
 * @partition_name[IN]: 
 * return: opened partition handle; NULL failed; 
 */
void* ak_partition_open(const char *partition_name)
{
    void *handle = NULL;
	
	handle = partition_open((unsigned char *)partition_name);
	if(NULL == handle)
	{
		return NULL;
	}
	
	return handle;

}

/**
* @brief partition write
* @author 
* @date 2016-12-21
* @param [in] handle is the pointer of file handle
* @param [in] data is the pointer to data
* @param [in] len is data length
* @return int:success return data length, fail return -1
* @version 
*/
int ak_partition_write(const void *handle, const char *data, unsigned int len)
{
	int ret=0;
	
	if((NULL == handle)||(NULL == data)||(0 == len))
	{
		return -1;
	}
	
	ret = partition_write((void *)handle, (unsigned char *)data, len);
	if(ret < 0)
	{
		return -1;
	}
	
    return ret;

}

/**
* @brief partition read
* @author 
* @date 2016-12-21
* @param [in] handle is the pointer of file handle
* @param [in] data is the pointer to buffer
* @param [in] len is data length
* @return int:success return data length, fail return -1
* @version 
*/
int ak_partition_read(const void *handle, char *data, unsigned int len)
{
	int ret = 0;
	
	if((NULL == handle)||(NULL == data)||(0 == len))
	{
		return -1;
	}

	ret = partition_read((void *)handle, data, len);
	if(ret < 0)
	{
		return -1;
	}
	
    return ret;
}

/**
* @brief partition close
* @author 
* @date 2016-12-21
* @param [in] handle is the file handle
* @return int:success return 0, fail return -1
* @version 
*/
int ak_partition_close(const void *handle)
{
    int ret = 0;
	
	if(NULL == handle)
	{
		return -1;
	}
	
	ret = partition_close((void *)handle);
	if(ret < 0)
	{
		return -1;
	}
	
    return ret;
}

/**
 * ak_partition_get_dat_size-get partition data size
 * @handle[IN]: opened handle in ak_partition_open
 * return:   success return data_size, fail return 0
 */
unsigned long ak_partition_get_dat_size(void *handle)
{	 

	 if(NULL == handle)
	 {
		return 0;
	 }
	 
	 return partition_get_data_size(handle); 
}

/**
 * ak_partition_get_size - get partition size (bytes)
 * @handle[IN]: opened handle in ak_partition_open
 * return:    success return partition size, fail return 0
 */
unsigned long ak_partition_get_size(void *handle)
{
	unsigned long ret = 0;

	if(NULL == handle)
	{
		return 0;
	}
	
	ret = partition_get_ksize(handle);
	if(0 == ret)
	{
		return 0;
	}
	ret *= 1024;
	
	return ret; 
}

