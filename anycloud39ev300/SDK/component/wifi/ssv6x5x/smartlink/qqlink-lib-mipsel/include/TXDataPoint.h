#ifndef __TX_DATA_POINT_H__
#define __TX_DATA_POINT_H__

#include "TXSDKCommonDef.h"
#include "TXMsg.h"

CXX_EXTERN_BEGIN

///////////////////////////////////////////////////////////////////////////
//
//                 低级接口： 直接处理 DATAPOINT 消息
//
///////////////////////////////////////////////////////////////////////////
//
//   DATAPOINT结构体
//
//  （1）一个 datapoint 中最重要的信息就是 id 和 value
//  （2）QQ物联后台的所有消息都是用 datapoint id 进行约束的，
//  （3）如果 datapoint id 相同，则value采用同样的格式进行组织
//  （4）QQ物联平台中的所有智能设备对同样的  datapoint id 采用同样的解析逻辑，因而可以相互理解
//   

typedef struct tag_tx_data_point
{
    // char * : utf8 with '\0'
    unsigned int            id;
    char *                  value;
    unsigned int            seq;
    unsigned int            ret_code;
}tx_data_point;

// 
typedef struct tag_tx_data_point_notify
{
    void (*on_receive_data_point)(unsigned long long from_client, tx_data_point * data_points, int data_points_count);
} tx_data_point_notify;

// 初始化datapoint
// notify: receive datapoint callback
SDK_API int tx_init_data_point(const tx_data_point_notify *notify);






// 上报datapoint数据 结果callback
typedef void (*on_report_data_point_ret)(unsigned int cookie, int err_code);

// 上报datapoint数据
// [in]     id:             上报的datapoint ID
// [in]     value:          上报的datapoint value
// [out]    cookie:         返回调用cookie
// [in]     ret_callback:   发送结果callback
SDK_API int tx_report_data_point(unsigned int id, char * value, unsigned int * cookie, on_report_data_point_ret ret_callback);

// 上报一组datapoint数据
// [in]     data_points:        上报的datapoint
// [in]     data_points_count:  上报的datapoint count
// [out]    cookie:             返回调用cookie
// [in]     ret_callback:       发送结果callback
SDK_API int tx_report_data_points(tx_data_point * data_points, int data_points_count, unsigned int * cookie, on_report_data_point_ret ret_callback);






typedef void (*on_ack_data_point_ret)(unsigned int cookie, unsigned long long from_client, int err_code);

// 应答收到的datapoint
// [in]    from_client:     datapoint信令来自某个绑定者，       必须和已收到的datapoint时的from_client一致，否则会被过滤
// [in]    id:              应答的datapoint对应的ID，          必须和已收到的datapoint的ID一致，否则会被过滤
// [in]    value:           应答的datapoint的自定义数据
// [in]    seq:             应答的datapoint对应的seq，         必须和已收到的datapoint的seq一致，否则会被过滤
// [out]   cookie:          返回调用cookie
// [in]    ret_callback:    发送结果callback
SDK_API int tx_ack_data_point(unsigned long long from_client, unsigned int id, char * value, unsigned int seq, unsigned int ret_code, unsigned int * cookie, on_ack_data_point_ret ret_callback);

// 应答收到的一组datapoint
// [in]    from_client:         datapoint信令来自某个绑定者，       必须和已收到的datapoint时的from_client一致，否则会被过滤
// [in]    data_points:         应答的datapoint，                 应答的一组datapoint，与接收到的一组datapoint的 ID & seq需要一致，若存在 frome_client & ID & seq 不存在收到的datapoint记录，会被过滤
// [in]    data_points_count:   datapoint count
// [out]   cookie:              返回调用cookie
// [in]    ret_callback:        发送结果callback
SDK_API int tx_ack_data_points(unsigned long long from_client, tx_data_point * data_points, int data_points_count, unsigned int * cookie, on_ack_data_point_ret ret_callback);


CXX_EXTERN_END

#endif // __TX_DATA_POINT_H__
