#ifndef __AK_KEY_LED_H__
#define __AK_KEY_LED_H__


/**
 * @brief Key function open
 * @pram [in] gpio_num : key -> gpio_num
 * @pram [in] Key_pin  : gpio ->pin
 * @pram [in] IntType  : interrupt triggered condition , 1:high level    0:low level
 * @pram [in] ms  :key put down effective time 
 * @pram [in] key_Int_Handler : effective key interrupt callback func tion address 
 * @author Jiankui
 * @date 2016-12-08
 * @return int
 * @retval 0 success
  * @retval -1 fail
 */
int key_open(unsigned char gpio_num, unsigned char IntType, unsigned long MilliSecs, void (*key_Int_Handler)(void));



/**
 * @brief init led gpio with output function
 * @pram [in] gpio_num : gpio wake up -> gpio_num
 * @author Jiankui
 * @date 2016-12-08
 * @return void
 */
void led_open(unsigned char gpio_num);

/**
 * @brief set led on/off function,befor must led_open function
 * @pram [in] gpio_num : gpio wake up -> gpio_num
 * @pram [in] value :write gpio out date value, 0 : led off,   1:led on
 * @author Jiankui
 * @date 2016-12-08
 * @return void
 */
void led_ctrol(unsigned char gpio_num, unsigned char value);









#endif
