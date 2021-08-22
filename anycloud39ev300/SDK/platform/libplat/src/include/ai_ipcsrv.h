#ifndef _AI_IPCSRV_H_
#define _AI_IPCSRV_H_

enum ai_level_index {
	AI_FINAL_LEVEL,		/* save the final pcm */
	AI_ORIGIN_LEVEL,	/* save the origin pcm */
	AI_RESAMPLE_LEVEL,	/* save the pcm after resample */
	AI_ASLC_LEVEL,		/* save the pcm after aslc */
	AI_MAX_LEVEL
};

/**
 * ai_save_stream_to_file - save pcm file  
 * @buf[IN]: ai data
 * @len[IN]: ai data length
 * @save_level[IN]: ai data save level [0,3]
 * return: 0 success -1 failed
 * notes: 
 */
int ai_save_stream_to_file(unsigned char *buf, int len,
						enum ai_level_index save_level);

/**
 * ai_sys_ipc_register - register to ipc 
 * return: NULL
 * notes: 
 */
void ai_sys_ipc_register(void);

/**
 * ai_sys_ipc_unregister - unregister to ipc 
 * return: NULL
 * notes: 
 */
void ai_sys_ipc_unregister(void);

/**
 * ai_sysipc_bind_handle - bind ai handle 
 * @handle[IN]: audio in opened handle
 * return: NULL
 * notes: 
 */
void ai_sysipc_bind_handle(void *handle);

/**
 * ai_sysipc_unbind_handle - unbind ai handle
 * return: NULL
 * notes: 
 */
void ai_sysipc_unbind_handle(void);

#endif
