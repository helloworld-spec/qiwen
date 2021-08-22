#ifndef __DRV_UART_H__
#define __DRV_UART_H__

extern int drv_uart_initialize(void);
extern int drv_uart_set_baudrate(int baudrate);
extern int drv_uart_is_kbd_hit(void);
extern char drv_uart_get_char(void);
extern void drv_uart_put_char(char ch);

#endif /* __DRV_UART_H__ */
