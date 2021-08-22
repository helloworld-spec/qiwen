#ifndef __TX_TV_BARRAGE_H__
#define __TX_TV_BARRAGE_H__

#include "TXDeviceSDK.h"

CXX_EXTERN_BEGIN

enum tx_barrage_msg_element_type
{
    element_none            = 0,
    element_text            = 1,        // 文本消息
    element_face            = 2,        // 表情
    element_image           = 3,        // 图片
    element_audio           = 4,        // 语音消息
};

// 文本消息
typedef struct tag_tx_text_msg_element
{
    char *              msg_text;       // '\0'结尾的字符串
}tx_text_msg_element;

// 表情消息
typedef struct tag_tx_face_msg_element
{
    unsigned int        face_index;
}tx_face_msg_element;

// 图片
typedef struct tag_tx_image_msg_element
{
    char *              image_guid;
    int                 guid_length;
    char *              image_url;      // 原图下载地址 '\0'结尾的字符串
    char *              image_thumb_url;// 缩略图下载地址 '\0'结尾的字符串
}tx_image_msg_element;

// 语音
typedef struct tag_tx_audio_msg_element
{
    unsigned int        voice_switch;
    char *              audio_msg_url;  // 语音消息的下载url '\0'结尾的字符串
}tx_audio_msg_element;

typedef struct tag_tx_barrage_msg_element
{
    int                         msg_ele_type;
    void *                      msg_ele_point;
}tx_barrage_msg_element;

typedef struct tag_tx_barrage_msg
{
    unsigned long long          group_id;           // 群id
    unsigned long long          from_target_id;     // 消息发送者id
    unsigned int                from_target_appid;  // 消息发送者appid
    unsigned int                from_target_instid; // 消息发送者instid
    char *                      group_name;         // 群名字，以'\0'结尾的字符串
    char *                      from_nick;          // 发送者nick，以'\0'结尾的字符串
    char *                      from_group_card;    // 发送者群名片，以'\0'结尾的字符串
    char *                      from_head_url;      // 发送者的头像url，以'\0'结尾的字符串
    tx_barrage_msg_element *    msg_ele_array;      // 消息元素数组
    int                         msg_ele_count;      // 消息元素数组个数
}tx_barrage_msg;


typedef struct tag_tx_barrage_notify
{
	//收到群消息
	void (*on_receive_barrage_msg)(tx_barrage_msg * pMsg);

}tx_barrage_notify;


//设置弹幕相关的回调
SDK_API void tx_set_barrage_notify(tx_barrage_notify * notify);

CXX_EXTERN_END

#endif // __TX_TV_BARRAGE_H__
