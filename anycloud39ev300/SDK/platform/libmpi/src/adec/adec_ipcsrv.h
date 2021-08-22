#ifndef _ADEC_IPCSRV_H_
#define _ADEC_IPCSRV_H_

enum adec_level_index{
	AFTER_DEC_LEVEL,	/* save after decode audio(pcm) */
	BEFORE_DEC_LEVEL,	/* save before decode audio */
	ADEC_MAX_LEVEL
};

/**
 * adec_save_stream_to_file - save stream to file  
 * @origin_type[IN]: origin decode type
 * @buf[IN]: adec data
 * @len[IN]: adec data length
 * @save_level[IN]: adec data save level [0,1]
 * return: 0 success -1 failed
 * notes: 
 */
int adec_save_stream_to_file(int origin_type, const unsigned char *buf, int len,
							enum adec_level_index save_level);

/**
 * adec_sys_ipc_register - register to ipc 
 * return: NULL
 * notes: 
 */
void adec_sys_ipc_register(void);

/**
 * adec_sys_ipc_unregister - unregister to ipc 
 * return: NULL
 * notes: 
 */
void adec_sys_ipc_unregister(void);

/**
 * adec_sysipc_bind_handle - bind adec handle 
 * @handle[IN]: audio decode opened handle
 * return: NULL
 * notes: 
 */
void adec_sysipc_bind_handle(void *handle);

/**
 * adec_sysipc_unbind_handle - unbind adec handle 
 * @origin_type[IN]: audio decode origin type
 * return: NULL
 * notes: 
 */
void adec_sysipc_unbind_handle(int origin_type);

#endif

