#ifndef _VI_IPCSRV_H_
#define _VI_IPCSRV_H_

/* 
 * vi_sys_ipc_register - register vi module 
 * 						 system inter process communication function
 */
void vi_sys_ipc_register(void);

/* 
 * vi_sys_ipc_unregister - unregister vi module 
 * 						 system inter process communication function
 */
void vi_sys_ipc_unregister(void);

/* 
 * vi_sysipc_bind_dev_handle - bind vi handle with device number
 * handle[IN]: vi handle, return by ak_vi_open();
 * dev_no[IN]: device number, same as the argument delivery to ak_vi_open();
 */
void vi_sysipc_bind_dev_handle(void *handle, int dev_no);

/* 
 * vi_sysipc_unbind_dev_handle - unbind vi handle 
 * dev_no[IN]: device number, same as the argument delivery to ak_vi_open();
 */
void vi_sysipc_unbind_dev_handle(int dev_no);

/*
 * vi_save_yuv_to_file - write yuv to file
 * dev_no[IN]: device number, same as calling ak_vi_open(dev_no);
 * buf[IN]: yuv data pointer
 * return: 0 on success, -1 failed
 */
int vi_save_yuv_to_file(int dev_no, unsigned char *buf);

#endif
