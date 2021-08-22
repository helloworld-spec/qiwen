#ifndef _AO_IPCSRV_H_
#define _AO_IPCSRV_H_

enum ao_level_index {
	AO_FINAL_LEVEL,		/* save the final pcm */
	AO_ORIGIN_LEVEL,	/* save the origin pcm */
	AO_RESAMPLE_LEVEL,	/* save the pcm after resample */
	AO_ASLC_LEVEL,		/* save the pcm after aslc */
	AO_EQ_LEVEL,		/* save the pcm after eq */
	AO_MAX_LEVEL
};

/**
 * ao_save_stream_to_file - save pcm file  
 * @buf[IN]: ai data
 * @len[IN]: ai data length
 * @save_level[IN]: ai data save level [0,3]
 * return: 0 success -1 failed
 * notes: 
 */
int ao_save_stream_to_file(unsigned char *buf, int len,
						enum ao_level_index save_level);

/**
 * ai_sys_ipc_register - register to ipc 
 * return: NULL
 * notes: 
 */
void ao_sys_ipc_register(void);

/**
 * ao_sys_ipc_unregister - unregister to ipc 
 * return: NULL
 * notes: 
 */
void ao_sys_ipc_unregister(void);

/**
 * ao_sysipc_bind_handle - bind ao handle 
 * @handle[IN]: audio out opened handle
 * return: NULL
 * notes: 
 */
void ao_sysipc_bind_handle(void *handle);

/**
 * ao_sysipc_unbind_handle - unbind ao handle
 * return: NULL
 * notes: 
 */
void ao_sysipc_unbind_handle(void);

#endif

