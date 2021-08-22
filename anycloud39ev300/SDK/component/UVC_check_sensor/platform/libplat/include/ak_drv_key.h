#ifndef         __AK_DRV_KEY_H
#define         __AK_DRV_KEY_H


enum key_stat{
  PRESS = 1,
  RELEASE,
};

struct key_event{
   int code;
   enum key_stat stat;
};
/**
 * @brief   set gpio share pin as gpio,input dir,interrupt mode
 * @author  Panyuyi
 * @date    2016-11-23
 * @param[in]  
 * @param[in]  
 * @return  void *
 * @retval  NULL created failed
 */

void* ak_drv_key_open();

/**
 * @brief   get the state of key
 * @author  Panyuyi
 * @date    2016-11-23
 * @param[in]  handler handler of the key
 * @param[in]  event     refer to KEY_EVENT
 * @param[in]  ms
 * @return int
 * @retval  0 failed
 * @retval  1 successful
 */
	

int ak_drv_key_get_event(void *handle, struct key_event *event,long ms);



/**
 * @brief   close gpio pin
 * @author  Panyuyi
 * @date    2016-11-23
 * @param[in]  handler handler of the key
 * @return int
 * @retval  0 failed
 * @retval  1 successful
 */
 int ak_drv_key_close(void *handle);

#endif
