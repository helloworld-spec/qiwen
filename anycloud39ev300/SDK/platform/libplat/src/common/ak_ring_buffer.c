#include <stdlib.h>
#include <string.h>

#include "internal_error.h"

#include "ak_common.h"
#include "ak_thread.h"
#include "ak_ring_buffer.h"

#define AK_RB_DEBUG		0

struct ak_rb_t {
	int remain;				//free buffer len
	int read;				//current read offset
	int write;				//current write offset
	int total_len;			//buffer total len
	ak_mutex_t data_mutex;
	unsigned char *data;
};

/** 
 * ak_rb_init - init appointed size ring buffer
 * @rb_size[IN]: appointed ring buffer size
 * return: new ring buffer handle; NULL failed
 */
void* ak_rb_init(const unsigned int rb_size)
{
	struct ak_rb_t *rb = NULL;
	
	if (rb_size > 0) {
		rb = (struct ak_rb_t *)calloc(1, sizeof(struct ak_rb_t));
		if (rb) {
			rb->data = (unsigned char *)calloc(1, rb_size); 
			if (rb->data) {
				rb->total_len = rb_size;
				ak_thread_mutex_init(&(rb->data_mutex), NULL);
				ak_print_notice_ex("init rb_size=%d ring buffer OK\n", rb_size);
			} else {
				free(rb);
				rb = NULL;
				
				ak_print_error_ex("calloc ring buffer failed, rb_size=%d\n",
					rb_size);
				set_error_no(ERROR_TYPE_MALLOC_FAILED);
			}
		} else {
			ak_print_error_ex("calloc ring buffer struct failed\n");
			set_error_no(ERROR_TYPE_MALLOC_FAILED);
		}
	}

	return rb;
}

/** 
 * ak_rb_reset - reset ring buffer, all of the data will be lost
 * @rb_handle[IN]: ring buffer handle
 * return: 0 success; -1 failed
 */
int ak_rb_reset(void *rb_handle)
{
	if (!rb_handle) {
		return AK_FAILED;
	}

	struct ak_rb_t *rb = (struct ak_rb_t *)rb_handle;

	ak_thread_mutex_lock(&(rb->data_mutex));
	rb->remain = 0;
	rb->read = 0;
	rb->write = 0;
	if (rb->data) {
		memset(rb->data, 0x00, rb->total_len);
	}
	ak_thread_mutex_unlock(&(rb->data_mutex));

	return AK_SUCCESS;
}

/** 
 * ak_rb_get_data_len - get ring buffer current data len
 * @rb_handle[IN]: ring buffer handle
 * return: >=0 current data len; -1 failed
 */
int ak_rb_get_data_len(void *rb_handle)
{
	if (!rb_handle) {
		return AK_FAILED;
	}

	struct ak_rb_t *rb = (struct ak_rb_t *)rb_handle;
	
	ak_thread_mutex_lock(&(rb->data_mutex));
	int data_len = rb->remain;
	ak_thread_mutex_unlock(&(rb->data_mutex));
	
	return data_len;
}

/** 
 * ak_rb_read - read ring buffer data
 * @rb_handle[IN]: ring buffer handle
 * @to[OUT]: read data buffer
 * @read_len[IN]: read data len
 * return: >=0 real read len; -1 failed
 */
int ak_rb_read(void *rb_handle, unsigned char *to, int read_len)
{
	if (!rb_handle) {
		return AK_FAILED;
	}

	struct ak_rb_t *rb = (struct ak_rb_t *)rb_handle;
	
	ak_thread_mutex_lock(&(rb->data_mutex));
	if (read_len > rb->remain) {
		read_len = rb->remain;
	}

#if AK_RB_DEBUG
	ak_print_normal_ex("333, write=%d, read=%d, remain=%d\n",
		rb->write, rb->read, rb->remain);
#endif		
		
	int tail_space = (rb->total_len - rb->read);
	if (tail_space >= read_len) {
		memcpy(to, &(rb->data[rb->read]), read_len);
		rb->read += read_len;
	} else {
		if (tail_space > 0) {
			memcpy(to, &(rb->data[rb->read]), tail_space);
		}
		
		memcpy(&to[tail_space], rb->data, (read_len - tail_space));
		rb->read = read_len - tail_space;
	}

	rb->remain -= read_len;

#if AK_RB_DEBUG	
	ak_print_normal_ex("444, write=%d, read=%d, remain=%d\n\n",
		rb->write, rb->read, rb->remain);
#endif

	ak_thread_mutex_unlock(&(rb->data_mutex));

	return read_len;
}

/** 
 * ak_rb_read - read ring buffer data
 * @rb_handle[IN]: ring buffer handle
 * @from[IN]: write data buffer
 * @write_len[IN]: write data len
 * return: >=0 real read len; -1 failed
 */
int ak_rb_write(void *rb_handle, const unsigned char *from, int write_len)
{
	if (!rb_handle) {
		return AK_FAILED;
	}

	struct ak_rb_t *rb = (struct ak_rb_t *)rb_handle;
	
	if (write_len > rb->total_len) {
		ak_print_error_ex("save size over, total_len=%d, write_len=%d\n",
			rb->total_len, write_len);
		return AK_FAILED;
	}

#if AK_RB_DEBUG
	ak_print_normal_ex("111, write=%d, read=%d, remain=%d\n",
		rb->write, rb->read, rb->remain);
#endif

	ak_thread_mutex_lock(&(rb->data_mutex));
	int tail_space = (rb->total_len - rb->write);
	if (tail_space >= write_len) {
		memcpy(&(rb->data[rb->write]), from, write_len);
		rb->write += write_len;
	} else {
		if (tail_space > 0) {
			memcpy(&(rb->data[rb->write]), from, tail_space);
		}

		memcpy(rb->data, &from[tail_space], (write_len - tail_space));
		rb->write = write_len - tail_space;
	}
	
	rb->remain += write_len;

	/* ring buffer is full */
	if(rb->remain >= rb->total_len) {
		/* data had been overwritten, keep read=write */
		rb->read = rb->write;
		rb->remain = rb->total_len;
    }

#if AK_RB_DEBUG    
    ak_print_normal_ex("222, write=%d, read=%d, remain=%d\n\n",
		rb->write, rb->read, rb->remain);
#endif

	ak_thread_mutex_unlock(&(rb->data_mutex));

	return AK_SUCCESS;
}

/** 
 * ak_rb_release - init appointed size ring buffer
 * @rb_handle[IN]: ring buffer handle
 * return: 0 success; -1 failed
 */
int ak_rb_release(void *rb_handle)
{
	if (!rb_handle) {
		return AK_FAILED;
	}

	struct ak_rb_t *rb = (struct ak_rb_t *)rb_handle;
	
	if (rb->data) {
		free(rb->data);
		rb->data = NULL;
	}

	ak_thread_mutex_destroy(&(rb->data_mutex));
	ak_print_info_ex("rb_size=%d, release ring buffer OK\n", rb->total_len);
	free(rb);
	rb = NULL;

	return AK_SUCCESS;
}
