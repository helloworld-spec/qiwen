#ifndef __AK_DRV_IRCUT_H__
#define __AK_DRV_IRCUT_H__

enum ircut_status {
	IRCUT_STATUS_DAY = 0,
	IRCUT_STATUS_NIGHT,
	IRCUT_STATUS_NUM
};

/** 
 * ak_drv_ir_init: init ircut ,get ircut control mode 
 * return: 0 - success; otherwise -1; 
 */
int ak_drv_ir_init(void);


/** 
 * ak_drv_ir_get_input_level: get ir status
 * return:  success:0 or 1; fail:-1
 */
int ak_drv_ir_get_input_level(void);

/** 
 * ak_drv_ir_set_cut: set ircut to switch  
 * @value[IN]:  status replace day or night
 * return: 0 - success; otherwise -1; 
 */
int ak_drv_ir_set_ircut(enum ircut_status status);

#endif
