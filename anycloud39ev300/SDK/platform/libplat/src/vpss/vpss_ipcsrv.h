#ifndef _VPSS_IPCSRV_H_
#define _VPSS_IPCSRV_H_

/*
 * vpss_sys_ipc_register - register vpss module
 * return: void
 */
void vpss_sys_ipc_register(void);

/*
 * vpss_sys_ipc_unregister - unregister vpss module
 * return: void
 */
void vpss_sys_ipc_unregister(void);

/*
 * vpss_sysipc_bind_dev_handle - bind vi handle
 * handle[IN]: vi handle
 * dev_no[IN]: dev no
 * return: void
 */
void vpss_sysipc_bind_dev_handle(void *handle, int dev_no);

/*
 * vpss_sysipc_unbind_dev_handle - unbind vi handle
 * dev_no[IN]: dev no
 * return: void
 */
void vpss_sysipc_unbind_dev_handle(int dev_no);

#endif
