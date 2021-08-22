#ifndef __AK_LED_H__
#define __AK_LED_H__

/**
 * @brief LED device open
 * @author Jian_Kui
 * @date 2016-11-23
 * @param [in] id is led id
 * @return viod *
 * @retval device handle
  */
void * ak_drv_led_open(int id);

/**
 * @brief LED blink function
 * @author Jian_Kui
 * @date 2016-11-23
 * @param [in] handle  is led device handle
 * @param [in] on_time :led on time
 * @param [in] off_time :led off time
 * @return int
 * @retval 0, success
 * @retval -1, fail
  */
int ak_drv_led_blink(void *handle, int on_time, int off_time);

/**
 * @brief LED on function
 * @author Jian_Kui
 * @date 2016-11-23
 * @param [in] handle  is led device handle
 * @return int
 * @retval 0, success
 * @retval -1, fail
  */
int ak_drv_led_on(void *handle);

/**
 * @brief LED off function
 * @author Jian_Kui
 * @date 2016-11-23
 * @param [in] handle  is led device handle
 * @return int
 * @retval 0, success
 * @retval -1, fail
  */
int ak_drv_led_off(void *handle);

/**
 * @brief close LED device
 * @author Jian_Kui
 * @date 2016-11-23
 * @param [in] handle  is led device handle
 * @return int
  * @retval 0, success
  * @retval -1, fail
  */
int ak_drv_led_close(void *handle);

#endif 

