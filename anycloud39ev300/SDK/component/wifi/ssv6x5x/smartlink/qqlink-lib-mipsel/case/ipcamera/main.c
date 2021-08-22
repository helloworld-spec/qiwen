/**
 * main.c：如何初始化SDK和设备登陆
 *
 * 主要调用了tx_init_device函数来初始化设备信息以及相关回调，这些信息用于服务器在设备登陆时做验证使用。
 * 在SDK初始化成功后，您应该使用我们最新版的手Q，确保手Q和设备连接上了同一个路由器，并且能够访问互联网，
 * 然后在手Q好友面板”我的设备“中点击”搜索新设备“，将扫描到待绑定的设备，绑定成功后将触发设备的登录逻辑，
 * 这个登录过程将在SDK内部完成后通过设置的回调函数向上层通知。
 *
 * 期望的结果：
 * on_online_status回调的errcode为0
 * on_login_complete回调的new 为 11
 * on_binder_list_change正确返回设备的绑定者
 */

#include <stdio.h>
#include <string.h>

#include "TXSDKCommonDef.h"
#include "TXDeviceSDK.h"
#include "TXOTA.h"
#include "TXFileTransfer.h"
#include "TXIPCAM.h"

//=======================回调接口定义==============================//
extern bool test_start_camera();
extern bool test_stop_camera();
extern bool test_set_bitrate(int bit_rate);
extern void test_recv_audiodata(tx_audio_encode_param *param, unsigned char *pcEncData, int nEncDataLen);
extern bool test_start_mic();
extern bool test_stop_mic();

extern void cb_on_transfer_progress(unsigned long long transfer_cookie, unsigned long long transfer_progress, unsigned long long max_transfer_progress);
extern void cb_on_recv_file(unsigned long long transfer_cookie, const tx_ccmsg_inst_info * inst_info, const tx_file_transfer_info * tran_info);
extern void cb_on_transfer_complete(unsigned long long transfer_cookie, int err_code, tx_file_transfer_info* tran_info);

extern void test_send_pic_alarm();
extern void test_send_audio_alarm();

extern int cb_on_new_pkg_come(unsigned long long from, unsigned long long pkg_size, const char * title, const char * desc, unsigned int target_version);
extern void cb_on_download_progress(unsigned long long download_size, unsigned long long total_size);
extern void cb_on_download_complete(int ret_code);
extern void cb_on_update_confirm();

extern int cb_on_set_definition(int definition, char *cur_definition, int cur_definition_length);
extern int cb_on_control_rotate(int rotate_direction, int rotate_degree);
//===============================================================//

// 标记是否已经启动音视频服务
static bool g_start_av_service = false;

/**
 * 登录完成的通知，errcode为0表示登录成功，其余请参考全局的错误码表
 */
void on_login_complete(int errcode) {
    printf("on_login_complete | code[%d]\n", errcode);
}


/**
 * 在线状态变化通知， 状态（status）取值为 11 表示 在线， 取值为 21 表示  离线
 * old是前一个状态，new是变化后的状态（当前）
 */
void on_online_status(int old, int new) {
    printf("online status: %s\n", 11 == new ? "true" : "false");
    
    // 上线成功，启动音视频服务
    if(11 == new && !g_start_av_service) {
        // 视频通知：手Q发送请求视频信令，SDK收到后将调用 on_start_camera 函数
        // 这里只测试视频数据
        tx_av_callback avcallback = {0};
        avcallback.on_start_camera = test_start_camera;
        avcallback.on_stop_camera  = test_stop_camera;
        avcallback.on_set_bitrate  = test_set_bitrate;
        avcallback.on_recv_audiodata = test_recv_audiodata;
        avcallback.on_start_mic    = test_start_mic;
        avcallback.on_stop_mic     = test_stop_mic;
        
        int ret = tx_start_av_service(&avcallback);
        if (err_null == ret) {
            printf(" >>> tx_start_av_service successed\n");
        }
        else {
            printf(" >>> tx_start_av_service failed [%d]\n", ret);
        }
        
        g_start_av_service = true;
    }
}

/**
 * 绑定者列表变化通知，pBinderList是最新的绑定者列表
 */
void on_binder_list_change(int error_code, tx_binder_info * pBinderList, int nCount)
{
    if (err_null != error_code)
    {
        printf("on_binder_list_change failed, errcode:%d\n", error_code);
        return;
    }
    
    printf("on_binder_list_change, %d binder: \n", nCount);
    int i = 0;
    for (i = 0; i < nCount; ++i )
    {
        printf("binder uin[%llu], nick_name[%s]\n", pBinderList[i].uin, pBinderList[i].nick_name);
    }
}

/**
 * 辅助函数: 从文件读取buffer
 * 这里用于读取 license 和 guid
 * 这样做的好处是不用频繁修改代码就可以更新license和guid
 */
bool readBufferFromFile(char *pPath, unsigned char *pBuffer, int nInSize, int *pSizeUsed) {
	if (!pPath || !pBuffer) {
		return false;
	}

	int uLen = 0;
	FILE * file = fopen(pPath, "rb");
	if (!file) {
	    return false;
	}

	fseek(file, 0L, SEEK_END);
	uLen = ftell(file);
	fseek(file, 0L, SEEK_SET);

	if (0 == uLen || nInSize < uLen) {
		printf("invalide file or buffer size is too small...\n");
		return false;
	}

	*pSizeUsed = fread(pBuffer, 1, uLen, file);

	fclose(file);
	return true;
}

/**
 * 辅助函数：SDK的log输出回调
 * SDK内部调用改log输出函数，有助于开发者调试程序
 */
void log_func(int level, const char* module, int line, const char* message)
{
	printf("%s\n", message);
}

/**
 * SDK初始化
 * 例如：
 * （1）填写设备基本信息
 * （2）打算监听哪些事件，事件监听的原理实际上就是设置各类消息的回调函数，
 * 	针对本案例，我们监听的事情包括：
 *  a.登录，设备状态变更，绑定者列表变更
 *  b.云台转动以及视频清晰度控制
 *  c.收到文件（一般是手q向设备发送的语音文件）
 *  d.ota升级
 */
bool initDevice() {
	// 读取license
	unsigned char license[256] = {0};
	int nLicenseSize = 0;
	if (!readBufferFromFile("./licence.sign.file.txt", license, sizeof(license), &nLicenseSize)) {
		printf("[error]get license from file failed...\n");
		return false;
	}

	// 读取sn
	unsigned char guid[32] = {0};
	int nGUIDSize = 0;
	if(!readBufferFromFile("./GUID_file.txt", guid, sizeof(guid), &nGUIDSize)) {
		printf("[error]get guid from file failed...\n");
		return false;
	}

    //读取服务器公钥，此文件为您从iot.qq.com上注册设备时下载到的xxxx.pem(其中xxxx为您的设备的productid，请将下面代码中
    //的1000000004.pem替换为xxxx.pem)
    unsigned char svrPubkey[256] = {0};
    int nPubkeySize = 0;
    if (!readBufferFromFile("./1700001460.pem", svrPubkey, sizeof(svrPubkey), &nPubkeySize))
    {
        printf("[error]get svrPubkey from file failed...\n");
        return NULL;
    }

    // 设备的基本信息
    tx_device_info info = {0};
    info.os_platform            = "Linux";

    info.device_name            = "demo1";
    info.device_serial_number   = (char*)guid;
    info.device_license         = (char*)license;
    info.product_version        = 1;
    info.network_type           = network_type_wifi;

    //请将1000000004替换为您实际申请到的设备的productid
    info.product_id             = 1700001460;
    info.server_pub_key         = svrPubkey;

    // 设备登录、在线状态、绑定者列表变更的事件通知
    // 注意事项：
    // 如下的这些notify回调函数，都是来自硬件SDK内部的一个线程，所以在这些回调函数内部的代码一定要注意线程安全问题
    // 比如在on_login_complete操作某个全局变量时，一定要考虑是不是您自己的线程也有可能操作这个变量
    tx_device_notify notify      = {0};
    notify.on_login_complete     = on_login_complete;
    notify.on_online_status      = on_online_status;
    notify.on_binder_list_change = on_binder_list_change;

    // SDK初始化目录，写入配置、Log输出等信息
    // 为了了解设备的运行状况，存在上传异常错误日志 到 服务器的必要
    // system_path：SDK会在该目录下写入保证正常运行必需的配置信息，请一定注意：system_path目录下的文件必须永久保存，除
    // 用户按了reset键外，其他情况下一律不能删除该目录下文件，包括重启设备，ota升级等，否则sdk将无法正常工作
    // system_path_capicity：是允许SDK在该目录下最多写入多少字节的数据（最小大小：10K，建议大小：100K）
    // app_path：用于保存运行中产生的log或者crash堆栈
    // app_path_capicity：同上，（最小大小：300K，建议大小：1M）
    // temp_path：可能会在该目录下写入临时文件
    // temp_path_capicity：这个参数实际没有用的，可以忽略
    tx_init_path init_path = {0};
    init_path.system_path = "./";
    init_path.system_path_capicity = 100 * 1024;
    init_path.app_path = "./";
    init_path.app_path_capicity = 1024 * 1024;
    init_path.temp_path = "./";
    init_path.temp_path_capicity = 10 * 1024;

    // 设置log输出函数，如果不想打印log，则无需设置。
    // 建议开发在开发调试阶段开启log，在产品发布的时候禁用log。
    tx_set_log_func(log_func);

    // 初始化SDK，若初始化成功，则内部会启动一个线程去执行相关逻辑，该线程会持续运行，直到收到 exit 调用
	int ret = tx_init_device(&info, &notify, &init_path);
	if (err_null != ret) {
        printf(" >>> tx_init_device failed [%d]\n", ret);
        return false;
	}
    printf(" >>> tx_init_device success\n");
    
    //云台转动以及视频清晰度控制的事件通知
    tx_ipcamera_notify ipcamera_notify = {0};
    ipcamera_notify.on_control_rotate = cb_on_control_rotate;
    ipcamera_notify.on_set_definition = cb_on_set_definition;
    tx_ipcamera_set_callback(&ipcamera_notify);
    
    //收到文件（一般是手q向设备发送的语音文件）的事件通知
    tx_file_transfer_notify fileTransferNotify = {0};
    fileTransferNotify.on_transfer_complete = cb_on_transfer_complete;
    fileTransferNotify.on_transfer_progress = cb_on_transfer_progress;
    fileTransferNotify.on_file_in_come 		= cb_on_recv_file;
    tx_init_file_transfer(fileTransferNotify, "/tmp/ramdisk/");
    
    //ota升级的事件通知
    tx_ota_notify ota_notify = {0};
    ota_notify.on_new_pkg_come 		= cb_on_new_pkg_come;
    ota_notify.on_download_progress = cb_on_download_progress;
    ota_notify.on_download_complete = cb_on_download_complete;
    ota_notify.on_update_confirm 	= cb_on_update_confirm;
    tx_init_ota(&ota_notify, 10*60, "/tmp/update_pkg.tar");
    
	return true;
}

/****************************************************************
*  测试代码：
*
*  （1）while循环的作用仅仅是使 Demo进程不会退出，实际使用SDK时一般不需要
*
*  （2） 输入 "quit" 将会退出当前进程，这段逻辑存在的原因在于：
*     					在某些芯片上，直接用Ctrl+C 退出易产生僵尸进程
*
*  （3）while循环里面的sleep(1)在这里是必须的，因为如果Demo进程后台运行，scanf没有阻塞作用，会导致当前线程跑满CPU
*
*****************************************************************/
int main(int argc, char* argv[]) {
	if ( !initDevice() ) {
		return -1;
	}
	
	// 你可以在做其他相关的事情
	// ...

	char input[100];
	while (scanf("%s", input)) {
		if ( !strcmp(input, "quit") ) {
            //程序退出时调用tx_stop_av_service停止音视频服务
            if (g_start_av_service) {
                tx_stop_av_service();
            }
            
			tx_exit_device();
			break;
		}
        else if ( !strcmp(input, "sendpicalarm") ) {
            test_send_pic_alarm();
        }
        else if ( !strcmp(input, "sendaudioalarm") ) {
            test_send_audio_alarm();
        }
		sleep(1);
	}
	
	return 0;
}
