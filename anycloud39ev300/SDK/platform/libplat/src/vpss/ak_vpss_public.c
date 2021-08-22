#include "vpss_ipcsrv.h"

static const char vpss_version[] = "libplat_vpss V1.2.1";


/**
 * ak_vpss_get_version - get vpss lib version
 * return: version string
 * notes: 
 */
const char *ak_vpss_get_version(void)
{
	return vpss_version;
}

/**
 * ak_vpss_init - vpss module init
 * @vi_handle[IN]: opened vi handle
 * @dev_no[IN]: dev number
 * return: 
 * notes: 
 */
void ak_vpss_init(void *vi_handle, int dev_no)
{
	vpss_sys_ipc_register();
	vpss_sysipc_bind_dev_handle(vi_handle, dev_no);
}


/**
 * ak_vpss_destroy - vpss module destroy
 * @dev_no[IN]: dev number
 * return: 
 * notes: 
 */
void ak_vpss_destroy(int dev_no)
{
	vpss_sysipc_unbind_dev_handle(dev_no);
	vpss_sys_ipc_unregister();
}

