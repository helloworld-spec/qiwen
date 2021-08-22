#ifndef __TX_MSG_H__
#define __TX_MSG_H__

#include "TXSDKCommonDef.h"

CXX_EXTERN_BEGIN


///////////////////////////////////////////////////////////////////////////
//
//                          【向手机QQ发送消息】
//
//   向手机QQ发送消息是QQ物联智能硬件平台的一个特色通道，借助这个通道，设备可以将
//   重要的信息发给手机QQ，直接展示在手机QQ的消息列表中。
//
//   ATTENTION:
//   要发送这些消息，需要先进入配置平台（iot.qq.com）进行配置，否则QQ物联后台不会进行转发
//
///////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
//
//                          【向手机QQ发送文本消息】
//
///////////////////////////////////////////////////////////////////////////

//callback: 用于确认消息是否送达了QQ物联后台服务器
typedef void (*on_send_text_msg_ret)(unsigned int cookie, int err_code);

/**
 * 接口说明：向手机QQ发送文本消息
 * 参数说明：
 *  msg_id                  动态消息ID，请到配置平台进行注册，没有注册过的ID无法送达手机QQ
 *  text                    要发送的文本
 *  ret_callback            发送结果回调
 *  cookie                  由sdk内部分配唯一值，并会在发送结果回调时回传，用于唯一标识当前调用
 *  to_targetids            指定发给某些target
 *  to_targetids_count      指定发给某些target的count，填0表示发送给所有的绑定者
 */
SDK_API void tx_send_text_msg(int msg_id, char *text, on_send_text_msg_ret ret_callback, unsigned int *cookie, unsigned long long* to_targetids, unsigned int to_targetids_count);


///////////////////////////////////////////////////////////////////////////
//
//                          【向手机QQ发送结构化消息】
//
///////////////////////////////////////////////////////////////////////////
//
//   所谓结构化是一条消息里面包含了多个信息，比如title, timestamp, file_key, UI结构如下：
//            __________________________________________
//           | title                                    |
//           | timestamp                                |
//           |      ______________________________      |
//           |     |                              |     |
//           |     |                              |     |
//           |     |            IMAGE             |     |
//           |     |                              |     |
//           |     |                              |     |
//           |     |______________________________|     |
//           | content                                  |
//           | jump url >>>                             |
//           |__________________________________________|
//
//
typedef struct tag_structuring_msg
{
	int                     msg_id;             // 动态消息ID，请到配置平台进行注册，没有注册过的ID无法送达手机QQ
    char*					file_path;          // '\0'结尾字符串，附带文件的path（可为空）
    char*					thumb_path;         // '\0'结尾字符串，缩略图path（可空）
    char*					title;              // '\0'结尾字符串。结构化消息标题
    char*					digest;             // '\0'结尾字符串，简述文字
    char*					guide_words;        // '\0'结尾字符串，引导文字
	unsigned int            duration;           // 如果是语音消息，该字段用于设置录音时长 单位:秒
	unsigned long long*		to_targetids;       // 指定发给某些target
	unsigned int			to_targetids_count; // 指定发给某些target的count
} structuring_msg;

typedef struct _tx_send_msg_notify
{
    void (*on_file_transfer_progress)(const unsigned int cookie, unsigned long long transfer_progress, unsigned long long max_transfer_progress);
    void (*on_send_structuring_msg_ret)(const unsigned int cookie, int err_code);
} tx_send_msg_notify;

/**
 * 接口说明：向手机QQ发送结构化消息，每条消息都会有唯一的 cookie, tx_send_msg_notify 用于了解发送状态
 */
SDK_API void tx_send_structuring_msg(const structuring_msg *msg, tx_send_msg_notify *notify, unsigned int *cookie);




///////////////////////////////////////////////////////////////////////////
//
//                 向手机QQ发送强提醒通知
//
//////////////////////////////////////////////////////////////////////////
//
//  所谓强提醒通知是一种类似“来电提醒”的强唤醒机制，一般用于一些特定场景，比如智能门铃，等等。
//  在Android平台下，这个效果跟来电提醒没有体验上的区别，所以一定要慎用，在IOS平台下，由于后台运行的限制，
//  提醒实效性依赖APNS（苹果推送机制）的可靠性。
//

//callback: 用于确认消息是否送达了QQ物联后台服务器
typedef void (*on_send_notify_msg_ret)(unsigned int cookie, int err_code);

/**
 * 接口说明：向手机QQ发送强提醒通知，每条消息都会有唯一的 cookie
 * 参数说明：
 *  msg_id                  动态消息ID，请到配置平台进行注册，没有注册过的ID无法送达手机QQ
 *  digest                  显示在通知界面中的提示文字
 *  ret_callback            发送结果回调
 *  cookie                  由sdk内部分配唯一值，并会在发送结果回调时回传，用于唯一标识当前调用
 *  to_targetids            指定发给某些target
 *  to_targetids_count      指定发给某些target的count，填0表示发送给所有的绑定者
 */
SDK_API void tx_send_notify_msg(int msg_id, char* digest, on_send_notify_msg_ret ret_callback, unsigned int *cookie, unsigned long long* to_targetids, unsigned int to_targetids_count);

CXX_EXTERN_END

#endif // __TX_MSG_H__
