#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ak_common.h"
#include "ak_thread.h"
#include "internal_error.h"
#include "ak_error.h"

struct ak_error {
	unsigned char init_flag;
	ak_mutex_t mutex;
	char str[64];
	enum ak_error_type no;
};

static struct ak_error error = {0};

static void init_error(void)
{
	error.init_flag = AK_TRUE;
	error.no = ERROR_TYPE_NO_ERROR;
	memset(error.str, 0x00, sizeof(error.str));
	ak_thread_mutex_init(&(error.mutex));
}

/* the error no. will be overwritten by next error */
int ak_get_error_no(void)
{
	if(!error.init_flag) {
		init_error();
	}
	
	return error.no;
}

/** ak_get_error_str - get error string by errno
 * @error_no[IN]: appointed error no.
 * return: error description of errno
 * note: You can get ONLY ONE error string once.
 */
char* ak_get_error_str(int error_no)
{
	if(!error.init_flag) {
		init_error();
	}

	ak_thread_mutex_lock(&(error.mutex));
	memset(error.str, 0x00, sizeof(error.str));
	switch(error_no) {
	case ERROR_TYPE_NO_ERROR:	//0
		strcpy(error.str, "no error");
		break;
	case ERROR_TYPE_POINTER_NULL:
		strcpy(error.str, "the pointer is NULL");
		break;
	case ERROR_TYPE_INVALID_ARG:
		strcpy(error.str, "invalid argument");
		break;
	case ERROR_TYPE_MALLOC_FAILED:
		strcpy(error.str, "malloc/calloc memory failed");
		break;
	case ERROR_TYPE_PMEM_MALLOC_FAILED:
		strcpy(error.str, "UIO lib malloc pmem memory failed");
		break;
	case ERROR_TYPE_FUNC_NOT_SUPPORT:	//5
		strcpy(error.str, "function not support");
		break;
	case ERROR_TYPE_NOT_INIT:
		strcpy(error.str, "module not init");
		break;
	case ERROR_TYPE_CALL_ORDER_WRONG:
		strcpy(error.str, "call function in wrong order");
		break;
	case ERROR_TYPE_DEV_OPEN_FAILED:
		strcpy(error.str, "device open failed");
		break;
	case ERROR_TYPE_DEV_INIT_FAILED:
		strcpy(error.str, "device init failed");
		break;
	case ERROR_TYPE_DEV_CTRL_FAILED:	//10
		strcpy(error.str, "device I/O control failed");
		break;
	case ERROR_TYPE_DEV_READ_FAILED:
		strcpy(error.str, "device read failed");
		break;
	case ERROR_TYPE_DEV_WRITE_FAILED:
		strcpy(error.str, "device write failed");
		break;
	case ERROR_TYPE_FILE_OPEN_FAILED:
		strcpy(error.str, "file open failed");
		break;
	case ERROR_TYPE_FCNTL_FAILED:
		strcpy(error.str, "file fcntl failed");
		break;
	case ERROR_TYPE_FILE_READ_FAILED:	//15
		strcpy(error.str, "file read failed");
		break;
	case ERROR_TYPE_FILE_WRITE_FAILED:
		strcpy(error.str, "file write failed");
		break;
	case ERROR_TYPE_IOCTL_FAILED:
		strcpy(error.str, "ioctl failed");
		break;
	case ERROR_TYPE_GET_V4L2_PTR_FAILED:
		strcpy(error.str, "get V4L2 ptr failed");
		break;
	case ERROR_TYPE_INVALID_USER:
		strcpy(error.str, "invalid user");
		break;
	case ERROR_TYPE_THREAD_CREATE_FAILED:	//20
		strcpy(error.str, "create thread failed");
		break;
	case ERROR_TYPE_POPEN_FAILED:
		strcpy(error.str, "popen failed");
		break;
	case ERROR_TYPE_MEDIA_LIB_FAILED:
		strcpy(error.str, "call ATC media lib failed");
		break;
	case ERROR_TYPE_NO_DATA:
		strcpy(error.str, "no data");
		break;
	default:
		sprintf(error.str, "Unknow error %d", error_no);
		break;
	}
	ak_thread_mutex_unlock(&(error.mutex));
	
	return error.str;
}

void set_error_no(enum ak_error_type error_no)
{
	if(!error.init_flag) {
		init_error();
	}

	ak_thread_mutex_lock(&(error.mutex));
	error.no = error_no;
	ak_thread_mutex_unlock(&(error.mutex));
}
