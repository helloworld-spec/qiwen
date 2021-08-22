#ifndef _AK_PARTITION_H_
#define _AK_PARTITION_H_

const char* ak_partition_get_version(void);

/** 
 * ak_partition_open - 
 * @partition_name[IN]: 
 * return: opened partition handle; NULL failed; 
 */
void* ak_partition_open(const char *partition_name);


/** 
 * ak_partition_write - 
 * @handle[IN]: opened handle in ak_partition_open
 * @data[IN]: data to write
 * @len[IN]: data len
 * return: 0 success, -1 failed
 */
int ak_partition_write(const void *handle, const char *data, unsigned int len);


/** 
 * ak_partition_read - 
 * @handle[IN]: opened handle in ak_partition_open
 * @data[OUT]: data buf for read
 * @len[IN]: data len
 * return: 0 success, -1 failed
 */
int ak_partition_read(const void *handle, char *data, unsigned int len);


/** 
 * ak_partition_close - 
 * @handle[IN]: opened handle in ak_partition_open
 * return: 0 success, -1 failed
 */
int ak_partition_close(const void *handle);

/**
 * ak_partition_get_dat_size-get partition data size
 * @handle[IN]: opened handle in ak_partition_open
 * return:   success return data_size, fail return 0
 */
unsigned long ak_partition_get_dat_size(void *handle);

/**
 * ak_partition_get_size - get partition size (bytes)
 * @handle[IN]: opened handle in ak_partition_open
 * return:    success return partition size, fail return 0
 */
unsigned long ak_partition_get_size(void *handle);

#endif
