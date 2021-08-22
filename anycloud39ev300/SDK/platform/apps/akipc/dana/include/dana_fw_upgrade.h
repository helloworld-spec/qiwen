#ifndef __DANA_FW_UPGRADE_H__
#define __DANA_FW_UPGRADE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

typedef enum _dana_fw_upgrade_err_e_ {
    DANA_FW_UPGRADE_ERR_NULL            =   0x00000000, // succeeded
    DANA_FW_UPGRADE_ERR_FAILED          =   0x00000001, // failed
    DANA_FW_UPGRADE_ERR_UNKNOWN         =   0x00000002, // 未知错误
    DANA_FW_UPGRADE_ERR_URL_NOT_SUPPORT =   0x00000003, // 不支持URL下载
    DANA_FW_UPGRADE_ERR_FILE_CORRUPTION =   0x00000005, // 文件损坏
    DANA_FW_UPGRADE_ERR_NETWORK         =   0x00000006, // 网络异常
    DANA_FW_UPGRADE_ERR_RW              =   0x00000007, // 文件写失败
    DANA_FW_UPGRADE_ERR_CANCEL          =   0x00000008, // 下载被取消
} dana_fw_upgrade_err_t;



typedef struct _dana_fw_upgrade_callback_funs_s {
    // 新固件回调
    // 如果返回0,模块会开始下载升级固件包, 需要指定save_pathname 包含完整路径的文件名
    uint32_t (*dana_fw_upgrade_new_rom_come)(const uint64_t rom_size, const char *rom_ver, char save_pathname[512]);
    
    // 下载完成回调
    void (*dana_fw_upgrade_rom_download_complete)(const uint32_t code, const char *rom_pathname);
    
    // 确认升级回调
    // 成功下载固件包后App会让用户确认,只有这个回调被调用了,才可以启动升级
    // upgrade_timeout_sec 本次升级所需的时间(可能来自升级包,由设备实现者来实现,这里只需要一个预估时间)
    // 需要保证大多数情况下: 文件替换+设备重启+联网
    void (*dana_fw_upgrade_confirm)(const char *rom_pathname, uint32_t *upgrade_timeout_sec);
} dana_fw_upgrade_callback_funs_t;



bool dana_fw_upgrade_init(const char *danale_path, const dana_fw_upgrade_callback_funs_t *cbs);

bool dana_fw_upgrade_deinit();

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* __DANA_FW_UPGRADE_H__ */
