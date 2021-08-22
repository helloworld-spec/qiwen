#ifndef _VENC_IPCSRV_H_
#define _VENC_IPCSRV_H_

/* 
 * venc_save_stream_to_file - save stream to file
 * chn[IN]: channel number
 * buf[IN]: pointer to buffer
 * len[IN]: indicate buffer's len
 */
int venc_save_stream_to_file(int chn, unsigned char *buf, int len);

/*
 * venc_sys_ipc_register - register message handle
 */
void venc_sys_ipc_register(void);

/*
 * venc_sys_ipc_unregister - unregister message
 */
void venc_sys_ipc_unregister(void);

/* 
 * venc_sysipc_bind_chn_handle - bind handle
 * handle[IN]: video encode handle, return by ak_venc_open();
 * chn[IN]: indicate handle belongs to which channel
 */
void venc_sysipc_bind_chn_handle(void *handle, int chn);

/* 
 * venc_sysipc_unbind_chn_handle - unbind handle
 * chn[IN]: indicate handle belongs to which channel
 */
void venc_sysipc_unbind_chn_handle(int chn);

#endif
