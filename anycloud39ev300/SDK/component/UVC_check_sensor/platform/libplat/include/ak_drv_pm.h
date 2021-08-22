#ifndef         __AK_DRV_PM_H
#define         __AK_DRV_PM_H


/**
 * @brief   get vbat value
 * @author  
 * @date    2016-11-23
 * @param[in]  bat voltage 
 * @return  int *
 * @retval  0 no bat
 * @retval  value bat_value
 */
int ak_drv_pm_get_bat_value(int *voltage);

#endif