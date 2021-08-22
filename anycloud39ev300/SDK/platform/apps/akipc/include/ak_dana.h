#ifndef _AK_DANA_H_
#define _AK_DANA_H_

typedef int (* FP_UPDATE_DVR_PLAN)(void);
typedef int (* FP_REQUEST_FORMAT_CARD)(void);

/**
 * ak_dana_get_version - get dana version
 * return: version string
 */
const char* ak_dana_get_version(void);

/**
 * ak_dana_init: init dana app
 * @vi_handle[IN]: opened vi_handle
 * @ai_handle[IN]: opened ai_handle
 * @ao_handle[IN]: opened ao_handle
 * return: void
 */
void ak_dana_init(void *vi_handle, void *ai_handle, void *ao_handle);

/* dana switch gop len */
void ak_dana_switch_gop(void);

/**
 * ak_dana_exit: exit dana app
 * return: void
 */
void ak_dana_exit(void);

/**
 * dak_dana_get_send_flag: get_send_flag,
 * 						   if memory is enought, send ok;
 * 						   else not send.
 * return: send flag. 1 -> send ok; others -> not send
 */
int ak_dana_get_send_flag(void);

#endif
