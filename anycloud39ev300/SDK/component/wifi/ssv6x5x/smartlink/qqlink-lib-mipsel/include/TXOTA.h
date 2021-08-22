#ifndef __OTA_H__
#define __OTA_H__

#include "TXSDKCommonDef.h"

CXX_EXTERN_BEGIN

/*
 * QQ物联智能设备升级接口
 */

 ///////////////////////////////////////////////////////////////////////
 //                                                                   //
 //                    QQ物联智能设备升级接口                         //
 //                                                                   //
 ///////////////////////////////////////////////////////////////////////

 /**
  *  QQ物联升级方案：
  *
  *                                                       |—————————————|
  *                                                       |   server    |
  *                                                       |—————————————|
  *                                                             |
  *                                                         1.query
  *                                                             |
  *      |-----------|                                    |—————————————|
  *      |           |    <---- 2.on_new_pkg_come ------  |             |
  *      |  DEVICE   |    ----- 3.download_progress --->  |     APP     |
  *      |           |    <---- 4.on_update_confirm ----  |             |
  *      |           |                                    |             |
  *      |-----------|    ----- 5.ack_ota_result ------>  |—————————————|
  *
  *
  *    step1. 手机向QQ物联升级服务器查询是否有新的升级信息，升级信息需要您在QQ物联的官网（iot.qq.com）的后台管理配置页面进行配置
  *           后台根据配置信息以及设备初始化时填入的product_version大小来确定是否有版本升级，建议在测试时单独准备两个版本号，不要和正式版本号混用，
  *           以免对正式设备造成影响。
  *    step2. 有设备可用的新固件版本，手机会将查询到的固件包信息通知给设备
  *    step3. 设备下载升级文件，并且实时地将进度通知给手机QQ
  *    step4. 手机等设备将文件下载完成后，会有一个最后的用户确认才会开始更新固件，因为替换文件可能需要一些时间，
  *           之后也有可能要重启设备，一个手机端的确认界面能给用户以心理上的等待预期。
  *    step5. 设备完成文件替换（并且重启之后），需要给手机一个升级结果的反馈，告知手机是否升级成功了
  */
typedef struct _tx_ota_notify
{
    /**
	* step2. 有设备可用的新固件版本，手机会将查询到的固件包信息通知给设备
	* param: pkg_size 新的升级固件包的大小
	* param：title + desc 升级描述信息，如果您的智能设备没有显示屏，可以忽略
	* param: target_version 目标版本的版本号
	* return 0  : 如果您返回 0，sdk将会开始启动升级包下载
	* return 1 : 会提示用户设备端拒绝升级（一般是磁盘剩余空间问题）
	*/
	int (*on_new_pkg_come)(unsigned long long from, unsigned long long pkg_size, const char * title, const char * desc, unsigned int target_version);

	/**
	* step3. 设备下载升级文件，并且实时地将进度通知给手机QQ
	* param：download_size 当前已经下载到的文件大小
	* param: total_size    文件总计大小，您可以用  download_size/total_size 来计算百分比
	*/
	void (*on_download_progress)(unsigned long long download_size, unsigned long long total_size);

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
	void (*on_download_complete)(int ret_code);

	/**
	* step4. 手机等设备将文件下载完成后，会有一个最后的用户确认才会开始更新固件，因为替换文件可能需要一些时间，
    *        之后也有可能要重启设备，一个手机端的确认界面能给用户以心理上的等待预期。
	*
	*        所以您需要在收到这个通知以后，再开始启动固件升级
	*/
	void (*on_update_confirm)();
} tx_ota_notify;

/**
* 初始化 tx_ota_notify
* param: replace_timeout step.4 -> step.5 您希望手机APP最多等待多少时间提示升级超时，您需要保证绝大多数情况下：
*        文件替换 + 设备重启的时间 < replace_timeout, 时间单位:秒
* param: target_pathname 您希望我们把升级包下载到哪个目录下的哪个文件，需要填写带文件名的完整路径
*/
SDK_API int  tx_init_ota(tx_ota_notify * notify, int replace_timeout , char* target_pathname);

/**
*  step5. 设备完成文件替换（并且重启之后），需要给手机一个升级结果的反馈，告知手机是否升级成功了,否则手q会在一段时间
*  		  之后告知用户升级超时，所以请务必实现此接口（如果设备重启，需要在设备上线之后调用）
*  param: ret_code  0表示成功；1表示失败
*  param: err_msg   升级失败的描述文案，升级失败时填写
*/
SDK_API void tx_ack_ota_result(int ret_code, char* err_msg);

CXX_EXTERN_END

#endif
