#ifndef __AK_DETECTOR_H__
#define __AK_DETECTOR_H__



enum detect_type
{
 	USB_DETECTOR =0,     //detect device number 
	SD_DETECTOR,
	GPIO_DETECTOR,
	MAX_DETECTOR
};
/**
 * @brief open  device detector
 * @author Jian_Kui
 * @date 2016-11-23
 * @param [in] dev_id :device ID
 * @return viod *
 * @retval device handle
  */
void *ak_drv_detector_open(int dev_id);

/**
 * @brief detector enable or disnable function
 * @author Jian_Kui
 * @date 2016-11-23
 * @param [in] handle :device handle
 * @param [in] value : 0 :disable ,1 :enable
 * @return int
 * @retval 0 :success
 * @retval -1 :fail 
  */
int ak_drv_detector_mask(void *handle, int value);

/**
 * @brief get detector status function
 * @author Jian_Kui
 * @date 2016-11-23
 * @param [in] handle :device handle
 * @param [in] event : remain link status
 * @return int
 * @retval 0 :success
 * @retval -1 :fail 
  */
int ak_drv_detector_poll_event(void *handle, int *event);

/**
 * @brief wait status change and get detector status function
 * @author Jian_Kui
 * @date 2016-11-23
 * @param [in] handle :device handle
 * @param [in] event : remain link status
  * @param [in] ms : the longst wait time,  ms<0: until charge, ms = 0: no wait ,ms>0: wait no than ms . unit :ms
 * @return int
 * @retval 0 :success
 * @retval -1 :fail 
  */
int ak_drv_detector_wait_event(void *handle, int *event, long ms);

/**
 * @brief close detector device
 * @author Jian_Kui
 * @date 2016-11-23
 * @param [in] handle :device handle
 * @return int
 * @retval 0 :success
 * @retval -1 :fail 
  */
int ak_drv_detector_close(void *handle);





#endif
