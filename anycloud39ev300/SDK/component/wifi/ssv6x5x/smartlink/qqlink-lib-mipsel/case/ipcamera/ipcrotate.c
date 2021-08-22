/**
 * 这段代码的目的是用来演示：手Q向设备发送云台及清晰度设置
 *
 * 通过tx_ipcamera_set_callback函数设置相关的回调函数。
 * 
 * 期望的结果是：
 * 设备收到清晰度调整回调时设置指定的清晰度，设备收到云台指令时转动云台
 */
 
#include <stdio.h>
#include <string.h>


// 设置清晰度,只需关注第一个参数definition, definitaion的含义在TXIPCAM.h中定义
int cb_on_set_definition(int definition, char *cur_definition, int cur_definition_length)
{
    printf("==============cb_on_set_definition, definition:%d\n", definition);
    return 0;
}

// 设置云台转动
int cb_on_control_rotate(int rotate_direction, int rotate_degree)
{
    printf("===============cb_on_control_rotate, rotate_direction:%d, rotate_degree:%d\n", rotate_direction, rotate_degree);
    return 0;
}
