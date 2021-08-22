#ifndef __INI_H__
#define __INI_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "kernel.h"
#include "ak_partition.h"

struct file_struct{
char save;
char *buf;
unsigned int buf_size;
unsigned int buf_tmp;
void *fd;
} ;

/**
 * malloc_INI_parti_buf - malloc MAX INI buffer
 * @file_id: the file handle
 * @buf_len: len of the buffer info
 * return: success:the pointer of buffer  fail:NULL
 
 */
static inline void* malloc_INI_parti_buf(void *file_id, unsigned int *buf_len)
{
	char *buf = NULL;
	
	if(NULL == file_id)
	{
		return NULL;
	}
	
	*buf_len = ak_partition_get_size(file_id);
	if(0 == *buf_len)
	{
		return NULL;
	}
	
	buf = (char *)malloc(*buf_len);
	if(NULL == buf)
	{   
		return NULL;
	}
	memset(buf, 0x00, *buf_len);
	
	return (void*)buf;
}

static inline int put_line_data_to_buf(char *ini_buf, unsigned long buf_len, char *line_data, unsigned int data_len, unsigned int *buf_tem)
{
	if((NULL == ini_buf) || (NULL == line_data) || (0 == buf_len) || (0 == data_len) || (*buf_tem > buf_len))
	{
		return -1;
	}

	if(buf_len >= (*buf_tem+data_len))
	{	
		memcpy(ini_buf+(*buf_tem), line_data, data_len);	
		*buf_tem = *buf_tem + data_len;
	}
	else
	{
		memcpy(ini_buf+(*buf_tem), line_data, (buf_len-*buf_tem));
		*buf_tem = buf_len;
	}

	return 0;
}

static inline void* get_file_all_data(void *file_id, unsigned long f_size)
{
	int ret = 0;
	char *f_buf = NULL;

	if((NULL == file_id) || (0 == f_size))
	{
		return NULL;
	}
	
	f_buf = (char *)malloc(f_size);
	if(f_buf == NULL)
	{   
		return NULL;
	}
	
	memset(f_buf, 0x00, f_size);
	ret = ak_partition_read(file_id, f_buf, f_size);
	if(ret < 0)
	{	
		free(f_buf);
		f_buf = NULL;
		return NULL;
	}
	
	return (void*)f_buf;
}

static inline int get_file_line_data(char *f_buf, unsigned int *f_tem, char *line_buf, unsigned int line_len, unsigned int file_size)
{
   char tem =0;
   unsigned long len = *f_tem;
   unsigned int i = 0;
   
   if((NULL == f_buf) || (NULL == line_buf) || (0 == line_len) || ( 0 == file_size) || (len > file_size))
   {
		return -1;
   }
   
   while(1)
   {
		tem = *(f_buf+len);
	    *(line_buf+i) = tem;
		len++;
		i++;
		
		if('\r' == tem)
		{	
		    if('\n' != (*(f_buf+len))) //mac os file
		    {
				break;
		    }
				
			continue;
		}
		
		if(('\n' == tem) || (len > file_size))
		{	
			break;
		}
		
		if(i > line_len)
		{
			return -1;
		}
   }

	if(len > file_size)
	{
		len = file_size;
	}

	*f_tem = len;
	
	return 0;
}

static inline void * open_file_w(const char *file_name)
{
	struct file_struct *file_id = NULL;

    file_id =(struct file_struct *)malloc(sizeof(struct file_struct));
	if(NULL == file_id)
	{
		return NULL;
	}
	
    file_id->buf = NULL;
	file_id->fd = NULL;
	file_id->buf_size =0;
	file_id->buf_tmp = 0;
	file_id->save =0;
	
	file_id->fd = ak_partition_open(file_name);
	if(NULL == file_id->fd)
	{
	    free(file_id);
		return NULL;
	}
		
	file_id->buf = (char *)malloc_INI_parti_buf(file_id->fd, &file_id->buf_size);
	if(NULL == file_id->buf)
	{
		ak_partition_close(file_id->fd);
		file_id->fd = NULL;
		free(file_id);
		return NULL;
	}
	
	return (void *)file_id;
}

static inline void * open_file_r(const char *file_name)
{
	struct file_struct *file_id = NULL;
	
	file_id =(struct file_struct *)malloc(sizeof(struct file_struct));
	if(NULL == file_id)
	{
		return NULL;
	}
	
    file_id->buf = NULL;
	file_id->fd = NULL;
	file_id->buf_size = 0;
	file_id->buf_tmp = 0;
	file_id->save =0;
	
	file_id->fd = ak_partition_open(file_name);
	if(NULL == file_id->fd)
	{
	    free(file_id);
		return NULL;
	}

	file_id->buf_size = ak_partition_get_dat_size(file_id->fd);
	file_id->buf = get_file_all_data(file_id->fd, file_id->buf_size);
	if(NULL == file_id->buf)
	{
		ak_partition_close(file_id->fd);
		file_id->fd = NULL;
		free(file_id);
		return NULL;
	}
	
	return (void *)file_id;
}

static inline void  write_file(char *file_buf, void *file_id)
{	
	struct file_struct *fp = file_id;
	
	if((NULL == file_id) || (NULL == file_buf))
    {
		return;
	}
	
	fp->save = 1;
	put_line_data_to_buf(fp->buf, fp->buf_size, file_buf, strlen(file_buf), &fp->buf_tmp);
}

static inline void * read_file(char *line_data , int line_data_len, void *file_id)
{	
    int ret=0;
	struct file_struct *fp = file_id;
	
	if((NULL == file_id))
    {
		return NULL;
	}
   
    ret = get_file_line_data(fp->buf, &fp->buf_tmp, line_data, line_data_len, fp->buf_size);
	if(ret < 0)
	{
		return NULL;
	}

	return (void *)1;
}

static inline void  close_file(void *file_id)
{
	struct file_struct *fp = file_id;
	
	if(NULL == file_id)
    {
		return;
	}
	
	//save data
	if((NULL != file_id) && (NULL != fp->fd) && (NULL != fp->buf) && (0 != fp->buf_tmp) && (fp->save))
	{
		ak_partition_write(fp->fd, fp->buf, fp->buf_tmp);
	}
	
	if(NULL != fp->buf)
	{
		free(fp->buf);
		fp->buf = NULL;
	}
	
	if(NULL != fp->fd)
	{
		ak_partition_close(fp->fd);	
		fp->fd = NULL;
	}
	
	free(fp);
}

static inline int file_eof(void *file_id)
{
    struct file_struct *fp = file_id;
	
	if(NULL == file_id)
    {
		return 1;
	}
	
	if(fp->buf_tmp == fp->buf_size)
	{
		return 1;
	}
	
	return 0;
}

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif // __INI_H__
