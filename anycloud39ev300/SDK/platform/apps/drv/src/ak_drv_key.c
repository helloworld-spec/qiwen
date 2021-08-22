#include <unistd.h>
#include <fcntl.h>
#include <linux/input.h>
#include "internal_error.h"
#include "ak_common.h"
#include "ak_drv_key.h"

#define KEY_INPUT_DEV_PATH "/dev/input/event0"

static const char drv_key_version[] = "libplat_drv_key V1.0.00";
static int key_handle = -1;

const char* ak_drv_key_get_version(void)
{
	return drv_key_version;
}

/**
 * ak_drv_key_open: open key driver
 * return: key handle , or NULL;
 */
void *ak_drv_key_open()
{
	if (key_handle < 0) {
		key_handle = open(KEY_INPUT_DEV_PATH, O_RDONLY);
		if (key_handle < 0) {
			set_error_no(ERROR_TYPE_DEV_OPEN_FAILED);
			ak_print_error("Open key dev fail\n");
			return NULL;
		}
	}

	return &key_handle;
}

/**
 * ak_drv_key_get_event: get key event .
 * @handle[IN]:  key handle
 * @key[OUT]:  key event
 * @ms[IN]:  time out .
 * return: 0 - success; otherwise -1;
 */
int ak_drv_key_get_event(void *handle, struct key_event *key, long ms)
{
    int fd = -1;
    int ret = 0, rdbyte;
	fd_set readfds;
	struct input_event key_event[64], *event;
	struct timeval timeout;

	/* arguments check */
	if ((NULL == handle || NULL == key)) {
		set_error_no(ERROR_TYPE_POINTER_NULL);
		ak_print_error_ex("para err\n");
		return -1;
	}

	if (handle != &key_handle) {
		set_error_no(ERROR_TYPE_INVALID_ARG);
		ak_print_error_ex("para err\n");
		return -1;
	}

	/*
	 * check wait time, deal with it differently.
	 * 0, no wait; > 0, wait
	 */
	fd= *(int *)handle;
	FD_ZERO(&readfds);
	FD_SET(fd, &readfds);

	/* block mode */
	if (ms == 0) {
		ret = select(FD_SETSIZE, &readfds, NULL, NULL, NULL);
		if (ret < 1) {
			ak_print_error("select error\n");
		}
	} else { /* unblock mode */
        timeout.tv_sec = ms / 1000;
		timeout.tv_usec = (ms % 1000) * 1000;
        ret = select(FD_SETSIZE, &readfds, NULL, NULL, &timeout);
	}

	/* To check whether the fd's status has changed */
	if (FD_ISSET(fd, &readfds)) {
		/** read the event to the buf **/
		rdbyte = read(fd, key_event, sizeof(struct input_event) * sizeof(key_event));
		/* parse the event */
		int i;
		for (i = 0; (i < rdbyte / sizeof(struct input_event)); i++) {
			event = (struct input_event *)&key_event[i];
			ak_print_info("i:%d type = %d, code = %d, value = %d!\n",
					i, event->type, event->code, event->value);
			/* filter others event */
			if (EV_KEY != event->type) {
				continue;
			}
			/* get key event */
			key->code = event->code;
			key->stat = (1 == event->value) ? PRESS : RELEASE;
		}
		ret = 0;
	} else
		ret = -1;

	return ret;
}

/**
 * ak_drv_key_close: close key .
 * @handle[IN]:  key handle
 * return: 0 - success; otherwise -1;
 */
int ak_drv_key_close(void *handle)
{
	int fd = -1;
	/* argument check */
	if ((handle == NULL) || (handle && (handle != &key_handle))) {
		set_error_no(ERROR_TYPE_INVALID_ARG);
		ak_print_error_ex("para err\n");
		return -1;
	}
	fd= *(int *)handle;
	/* close device */
	close(fd);
	key_handle = -1;

	return 0;
}
