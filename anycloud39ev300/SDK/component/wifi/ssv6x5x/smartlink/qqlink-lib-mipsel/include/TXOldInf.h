#ifndef __TX_OLD_INTERFACE_H__
#define __TX_OLD_INTERFACE_H__

#include "TXSDKCommonDef.h"
#include "TXDataPoint.h"

CXX_EXTERN_BEGIN

/////////////////////////////////////////////////////////////////////////
//
//  向手机QQ发送CC datapoint消息，早期接口，由于没有很好的解决多个绑定者的问题，后续会逐步弃用
//
////////////////////////////////////////////////////////////////////////

//消息 接收方/发送方 信息
typedef struct tag_tx_ccmsg_inst_info
{
    unsigned long long          target_id;          // ccmsg target id for send to / from
    unsigned int                appid;
    unsigned int                instid;
    unsigned int                platform;           // 指定平台
    unsigned int                open_appid;         // 开平分配给第三方app的appid
    unsigned int                productid;          //
    unsigned int                sso_bid;            // SSO终端管理分配的appid
    char *                      guid;               // 设备的唯一标识
    int                         guid_len;
}tx_ccmsg_inst_info;


// 发送C2C datapoint数据
typedef void (*on_send_cc_data_point_ret)(unsigned int cookie, unsigned long long to_client, int err_code);
/**
* 接口说明：发送Client to Client data point 消息
*/
SDK_API int tx_send_cc_data_point(unsigned long long to_client, unsigned int id, char * value, unsigned int * cookie, on_send_cc_data_point_ret ret_callback);
/**
* 接口说明：发送Client to Client data point 消息
*/
SDK_API int tx_send_cc_data_points(unsigned long long to_client, tx_data_point * data_points, int data_points_count, unsigned int * cookie, on_send_cc_data_point_ret ret_callback);

CXX_EXTERN_END

#endif // __TX_OLD_INTERFACE_H__
