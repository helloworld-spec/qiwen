#include "hal.h"

#include "lcd/lcd.h"
#include "uart/uart.h"
#include "ssp/sspd_rts.h"

#if defined(CONFIG_OS_UCOS_II) || defined(CONFIG_OS_UCOS_III)
#define TaskTS_TASK_PRIORITY	(32)
#elif defined(CONFIG_OS_FREERTOS)
#define TaskTS_TASK_PRIORITY	( tskIDLE_PRIORITY + 2 )
#elif defined(CONFIG_OS_THREADX)
#define TaskTS_TASK_PRIORITY    (31)
#else
#  error "No valid OS is defined!"
#endif

static unsigned int pre_rect_x = -1;
static unsigned int pre_rect_y = -1;
void draw_cross(void *param, int x, int y)
{
	if ((pre_rect_x == -1) || (pre_rect_y == -1) )
		drv_lcd_erase_rect(0, LCD_PANEL_WIDTH, 0, LCD_PANEL_HEIGHT);
	else
		drv_lcd_erase_rect(pre_rect_x-5,10,pre_rect_y-5,10);

	drv_lcd_draw_rect(x - 5, 10, y, 1, 255, 255, 255);
	drv_lcd_draw_rect(x, 1, y - 5, 10, 255, 255, 255);
	pre_rect_x = x;
	pre_rect_y = y;
}
void ts_init(struct ts_dev *ts)
{
	hal_create_semaphore(&ts->sem, 0, 0);

	ts->penirq	= HAL_TRUE;
	ts->penirq_en	= HAL_TRUE;
	ts->event_obj	= &ts->sem;
	ts->event_data	= &ts->data;

#ifndef CONFIG_PLAT_QEMU
	ts->left	= 161;
	ts->right	= 3956;
	ts->top		= 3756;
	ts->bottom	= 161;
#else
	/* These values are measured in Qemu before 
	 * We don't need to calibrate again */
	ts->left	= 287;
	ts->right	= 3788;
	ts->top		= 213;
	ts->bottom	= 3698;
#endif
	ts->lcd_width	= LCD_PANEL_WIDTH;
	ts->lcd_height	= LCD_PANEL_HEIGHT;

	_sspd_rts_init(ts);
}
void TaskTS(void *pdata)
{
	struct ts_dev ts;

	ts_init(&ts);

	DEBUG(1, 0, "Default   : left: %4d, right: %4d, top: %4d, bottom: %4d\n", ts.left, ts.right, ts.top, ts.bottom);
	ts_calibrate(&ts, draw_cross, 1);
	DEBUG(1, 0, "Calibrated: left: %4d, right: %4d, top: %4d, bottom: %4d\n", ts.left, ts.right, ts.top, ts.bottom);

	while (1) {

		int x = 0, y = 0;
		ts_value(&ts, &x, &y);

		draw_cross(NULL, x, y);
		DEBUG(1, 0, "%3d, %3d\n", x, y);
	}
}
void APP_Init(void){

	drv_uart_init();
	hal_init_os();
	drv_lcd_init();

	hal_thread_t thread;
	thread.fn 		= TaskTS;
	thread.name 		= "TaskTS";
	thread.stack_size 	= CONFIG_MINIMAL_STACK_SIZE;
	thread.arg 		= NULL;
	thread.prio 		= TaskTS_TASK_PRIORITY;
	thread.task		= NULL;
	thread.ptos		= NULL;
	hal_create_thread(&thread);
	hal_start_os();

}
