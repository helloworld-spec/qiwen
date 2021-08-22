/**
 * 这段代码的目的是用来演示：设备向手q发送报警消息,主要演示发送图文报警和语音报警
 * 报警消息实际上是动态消息，首先需要在iot.qq.com上注册和关联动态消息，这样物联后台才能正确转发报警消息
 * 具体配置情参考物联文档的第5.3章节：http://iot.open.qq.com/wiki/index.html#!CASE/IP_Camera.md
 */
 
#include <stdio.h>
#include <string.h>

#include "TXSDKCommonDef.h"
#include "TXDeviceSDK.h"
#include "TXMsg.h"

void on_send_alarm_file_progress(const unsigned int cookie, unsigned long long transfer_progress, unsigned long long max_transfer_progress)
{
    printf("on_send_alarm_file_progress, cookie[%u]\n", cookie);
}

void on_send_alarm_msg_ret(const unsigned int cookie, int err_code)
{
    printf("on_send_alarm_msg_ret, cookie[%u] ret[%d]\n", cookie, err_code);
}

/*
 * 发送图文报警，假设您已经在物联官网上配置触发器值为1时发送图文报警
 */
void test_send_pic_alarm()
{
    // 发送图片的结构化消息
    structuring_msg msg = {0};
    msg.msg_id = 1;   //假设您已经在物联官网上配置触发器值为1时发送图文报警
    msg.file_path = "./alarm.png";
    msg.thumb_path = "./thumb.png";
    msg.title = "发现异常";
    msg.digest = "客厅发现异常";
    msg.guide_words = "点击查看";
    
    tx_send_msg_notify notify = {0};
    notify.on_file_transfer_progress = on_send_alarm_file_progress;
    notify.on_send_structuring_msg_ret = on_send_alarm_msg_ret;
    tx_send_structuring_msg(&msg, &notify, 0);
}

/*
 * 发送语音报警，假设您已经在物联官网上配置触发器值为2时发送语音报警
 */
void test_send_audio_alarm()
{
    // 发送语音的结构化消息
    structuring_msg msg = {0};
    msg.msg_id = 2;   //假设您已经在物联官网上配置触发器值为2时发送语音报警
    msg.file_path = "./test.mp3";
    msg.title = "语音报警";
    msg.digest = "收到语音报警";
    msg.guide_words = "点击查看";
    
    tx_send_msg_notify notify = {0};
    notify.on_file_transfer_progress = on_send_alarm_file_progress;
    notify.on_send_structuring_msg_ret = on_send_alarm_msg_ret;
    tx_send_structuring_msg(&msg, &notify, 0);
}






