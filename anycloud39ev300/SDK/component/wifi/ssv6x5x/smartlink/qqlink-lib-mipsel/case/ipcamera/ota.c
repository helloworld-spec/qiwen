/**
 * 这段代码的目的是用来演示：手Q向设备发起ota升级
 *
 * 手q向设备发起ota升级命令，设备通过回调响应命令，详细细节请参考物联文档的ota升级章节：
 * http://iot.open.qq.com/wiki/index.html#!FUNC/OTA.md
 * 需要注意的是设备升级完成之后，无论成功失败请务必调用tx_ack_ota_result给手机QQ一个结果
 */
 
#include <stdio.h>
#include <string.h>

#include "TXSDKCommonDef.h"
#include "TXDeviceSDK.h"
#include "TXFileTransfer.h"


/**
 * 有设备可用的新固件版本，手机会将查询到的固件包信息通知给设备
 * param: pkg_size 新的升级固件包的大小，单位为字节
 * param：title + desc 升级描述信息，如果您的智能设备没有显示屏，可以忽略
 * param: target_version 目标版本的版本号
 * return: 如果返回0，sdk将会开始启动升级包下载
 * 如果返回1 会提示用户设备端拒绝升级（一般是磁盘剩余空间问题）
 */
int cb_on_new_pkg_come(unsigned long long from, unsigned long long pkg_size, const char * title, const char * desc, unsigned int target_version)
{
    //todo
    return 0;
}

/**
 * 设备下载升级文件，并且实时地将进度通知给手机QQ，单位为字节
 * param：download_size 当前已经下载到的文件大小
 * param: total_size文件总计大小，您可以用  download_size/total_size 来计算百分比
 */
void cb_on_download_progress(unsigned long long download_size, unsigned long long total_size)
{
    //todo
}

/**
 * param: ret_code  0表示下载成功，其它表示失败
 * 0   成功
 * 2   未知错误
 * 3   当前请求需要用户验证401
 * 4   下载文件写失败，没有写权限/空间不足/文件路径不正确
 * 5   网络异常
 * 7   升级文件包不存在404
 * 8   服务器当前无法处理请求503
 * 9   下载被手q用户中止
 * 10  参数错误，url不合法
 * 11  升级包md5值校验失败，下载可能被劫持
 */
void cb_on_download_complete(int ret_code)
{
    //todo
}

/**
 * 手机等设备将文件下载完成后，会有一个最后的用户确认才会开始更新固件，因为替换文件可能需要一些时间，
 * 之后也有可能要重启设备，一个手机端的确认界面能给用户以心理上的等待预期。
 *
 * 所以您需要在收到这个通知以后，再开始启动固件升级
 */
void cb_on_update_confirm()
{
    //todo 在这里执行升级操作
}


