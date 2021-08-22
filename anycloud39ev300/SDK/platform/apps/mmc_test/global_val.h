#define DEV_MMC           "/dev/mmcblk0"
#define BLK_1G            (2048*1024LL)
#define BLK_128M          (2048*64LL)
#define SEC_SIZE          512LL
#define SIZE_SECTOR       512
#define SIZE_SECTOR_ALIGN 0x1ff
#define NUM_FORMAT_SECTOR 64                                                                        //在删除分区表的时候,从0扇区开始清空的扇区数
#define CHKSZ             14 	//(3G)
#define RANGE             0x100000LL //(128M)
#define COUNT_SECTOR      2048
#define SEC2USEC          1000000
#define MSEC2USEC         1000
#define MSEC2NSEC         1000000
#define LEN_OFFSET        1024
#define LEN_HINT          512
#define LEN_CMD           256
#define LEN_MOUNT         2048
#define LEN_SHELL_RES     4096
#define CMD_MOUNT_STATUS  "mount"
#define CMD_UMOUNT        "umount -l %s"
#define CMD_MOUNT         "mount %s %s"
#define NUM_ASCII         256
#define GIGABYTE          1000000000
#define AK_FALSE          0
#define AK_TRUE           1
#define AK_FAILED        -1
#define AK_SUCCESS        0
#define SECTOR_PER_MB     2048

#define PERCENT_GOOD      0                                                                         //低于此错误率判断为好卡
#define PERCENT_WARNING   5                                                                         //低于此错误率判断为告警卡

#define DEV_MMCBLK0       "/dev/mmcblk0"
#define DEV_MMCBLK0P1     "/dev/mmcblk0p1"
#define LEN_PATH_FILE     255
#define FAT_FIX_TEMPLATE  "A:%s"
#define FAT_FIX_PATH      "/"
#define PROC_MOUNTS       "/proc/mounts"

#define BYTE2MB           1048576
#define KBYTE             1024
#define HINT_MNT_INFO_ERR "CAN'T FIND MOUNT POINT INFO."
#define HINT_TEST_TIMEOUT "READ/WRITE/ERASE TF CARD TIMEOUT."
#define HINT_NO_ERASE     "DISABLE ERASE TF CARD."
#define HINT_VERSION      "1.0.2"

#define TEST_PER_MB       20                                                                        //固定容量测试取点。
#define TEST_NUMBER       128                                                                       //固定的默认测试点数量
#define TEST_SECTOR       1                                                                         //测试扇区数量
#define TEST_TIMEOUT      15                                                                        //测试超时
#define TEST_POINT_TIMEOUT 300                                                                      //单个采集点测试超时
#define TEST_SLEEP        1                                                                         //每测试点的休眠时间
#define TEST_OFFSET       81920                                                                     //测试的起始偏移值
#define TEST_ALIGN        2048                                                                      //扇区对齐的值

#define SPEED_NUM         5                                                                         //采样点数量
#define SPEED_SECTOR      128                                                                       //采样点测速扇区数
#define SPEED_PER_MB      2                                                                         //测速采样点偏移值
#define SPEED_OFFSET      40960                                                                     //测速开始扇区偏移值
#define SPEED_REQUIRE_MB  1                                                                         //测速的写速度大于该数值判断为正常

#define HEX_OUTPUT_VIEW   64

#define SEC_WAIT_PT_FRESH 20                                                                        //等待内核重新加载分区表

#define CMD_SYNC          "sync"
#define CMD_FDISK         "echo -e 'n\np\n1\n\n\nw\n' | fdisk /dev/mmcblk0"
#define CMD_MKFS          "mkfs.vfat %s"
//#define CMD_FDISK_FRESH   "echo -e 'p\n\nv\n\n\nw\n' | fdisk /dev/mmcblk0"
#define CMD_FDISK_FRESH   "fdisk /dev/mmcblk0 << EOF\np\nw\nEOF"
typedef unsigned long long ULL;

enum CARD_STATUS {                                                                                  //坏卡测试
	CARD_STATUS_GOOD = 0,
	CARD_STATUS_WARNING,
	CARD_STATUS_ERROR,
};

enum FIX_STATUS {                                                                                   //文件系统检测
	FIX_STATUS_ERROR   = -1,
	FIX_STATUS_NONEED  =  0,
	FIX_STATUS_SUCCESS =  1,
};

enum FORMAT_STATUS {                                                                                //格式化结果
	FORMAT_STATUS_FAIL  =  0,
	FORMAT_STATUS_SUCCESS =  1,
};

enum RESULT_STATUS {                                                                                //检测结果类型
	RESULT_STATUS_NOTEST = -1,
	RESULT_STATUS_FAIL   =  0,
	RESULT_STATUS_PASS   =  1,
};

enum SPEED_TYPE {
	SPEED_TYPE_MIN = 0,
	SPEED_TYPE_MAX ,
	SPEED_TYPE_AVG ,
	SPEED_TYPE_NUM ,
};

enum CARD_TEST_TYPE {
	CARD_TEST_TYPE_GLOBAL = 0,
	CARD_TEST_TYPE_READ,
	CARD_TEST_TYPE_WRITE,
	CARD_TEST_TYPE_ERASE,
	CARD_TEST_TYPE_REWRITE,
	CARD_TEST_TYPE_NUM,
};

enum TEST_STATUS {
	TEST_STATUS_GLOBAL = 0,
	TEST_STATUS_SPEED,
	TEST_STATUS_CARD,
	TEST_STATUS_FATFS,
	TEST_STATUS_NUM,
};

enum FIX_RESULT {
	FIX_RESULT_ERROR,
	FIX_RESULT_NONEED,
	FIX_RESULT_SUCCESS,
	FIX_RESULT_UNKNOWN,
	FIX_RESULT_NUM,
};

struct card_result {                                                                                //测试结果报告结构
	int ai_status[ TEST_STATUS_NUM ];                                                               //测试结果
	ULL ai_status_us[ TEST_STATUS_NUM ];                                                            //测试用时
	int ai_status_point[ TEST_STATUS_NUM ];                                                         //测试点
	double f_total_size;                                                                            //tf卡容量
	ULL i_total_sector;                                                                             //tf卡扇区数
	double f_speed_size;                                                                            //测速的使用长度
	double f_speed_percent;                                                                         //测速的错误百分比
	double af_speed_write[ SPEED_TYPE_NUM ];                                                        //测速写速度
	double af_speed_read[ SPEED_TYPE_NUM ];                                                         //测速读速度
	int ai_card_test[ CARD_TEST_TYPE_NUM ];                                                         //坏卡测试次数
	int ai_card_error[ CARD_TEST_TYPE_NUM ];                                                        //坏卡测试错误次数
	double af_card_percent[ CARD_TEST_TYPE_NUM ];                                                   //坏卡测试错误百分比
	int i_fatfs_res;                                                                                //文件系统修复的结果
};

#define FIX_HINT_ERROR   "ERROR CARD. CAN'T FIX."
#define FIX_HINT_NONEED  "GOOD CARD. NO NEED FIX."
#define FIX_HINT_SUCCESS "WRONG CARD. FIX SUCCESS."
#define FIX_HINT_UNKNOWN "UNKNOWN CARD."

#define HINT_PASS        "PASS"
#define HINT_FAIL        "FAIL"

#define HINT_MOUNT_SUCCESS "MOUNT SUCCESS"
#define HINT_MOUNT_FAIL    "MOUNT FAIL"

#define HINT_CARD_GOOD    "GOOD CARD"
#define HINT_CARD_WARNING "WARNING CARD"
#define HINT_CARD_ERROR   "ERROR CARD"

#define HINT_TEST_GLOBAL  "GLOBAL"
#define HINT_TEST_READ    "READ"
#define HINT_TEST_WRITE   "WRITE"
#define HINT_TEST_ERASE   "ERASE"
#define HINT_TEST_REWRITE "REWRITE"

extern int gi_fd_dev;
extern int gi_speed_sector;
extern int gi_speed_per_mb;
extern int gi_speed_offset;
extern char gc_prog_run;
extern char gc_view_detail;
extern char gac_hint_status[ ][ LEN_HINT ];
extern char gac_hint_fatfs[ ][ LEN_HINT ];
extern double gf_speed_require;
extern char gc_full_test;
extern int gi_test_sector;
extern int gi_test_timeout;
extern char gc_per_mb;
extern int gi_test_per_mb;
extern int gi_test_offset;
extern int gi_test_num;
extern int gi_test_sleep;
extern char gc_read;
extern char gc_erase;
extern char gc_erase_check;
extern char gc_write;
extern char gc_sector_align;
extern char gc_write_check;
extern char gc_key_value_res;
extern char gc_fatfs_printf;
extern struct card_result g_card_result;
extern int gi_test_point_timeout;
extern char gac_mount_dev[ LEN_MOUNT ];