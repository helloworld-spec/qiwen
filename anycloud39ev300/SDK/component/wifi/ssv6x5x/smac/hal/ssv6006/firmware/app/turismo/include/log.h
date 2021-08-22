#ifndef _LOG_H_
#define _LOG_H_

// note : CONFIG_LOG_ENABLE is defined in <sim_config.h> or <soc_config.h>
#include <config.h>

// ===============================================================================================
//                                  Level & Modules definition
// ===============================================================================================

// LOG_LEVEL_FATAL:
// -  print "program halt!!!" msg & terminate program immerdiately;
// -  the 'LOG_LEVEL_FATAL' will always execute, ignore the influence of 'module' & 'level' settings.
#define LOG_LEVEL_ON        0x0                 // runtime option : show all level msg
#define LOG_LEVEL_TRACE     0x1
#define LOG_LEVEL_DEBUG     0x2
#define LOG_LEVEL_INFO      0x3
#define LOG_LEVEL_WARN      0x4
#define LOG_LEVEL_FAIL      0x5
#define LOG_LEVEL_ERROR     0x6
#define LOG_LEVEL_FATAL     0x7
#define LOG_LEVEL_OFF       (LOG_LEVEL_FATAL+1) // runtime option : show NO  level msg

// switch to turn on/off the message belongs to the 'module'
// max # of module id define : 0~31
typedef enum {
        LOG_MODULE_EMPTY        = 0,    // 0
        LOG_MODULE_INIT,                // 1  - Initialization
        LOG_MODULE_INT,                 // 2  - Interrupt
        LOG_MODULE_OS,                  // 3  - Operating system
        LOG_MODULE_CMD,                 // 4  - Command Engine
        LOG_MODULE_ALL                  // 5
} LOG_MODULE_E;


// ===============================================================================================
//                                          Global
// ===============================================================================================

//
// note : 
//      The 'sys_init_prnf' is to note that it is in system init stage now. All 'log printf' func 
// can NOT be used during this stage because the log module is NOT inited yet and the 'log mutex' 
// is NOT inited yet.
//
#define sys_init_prnf               printf

#define LOG_OUT_HOST_TERM           0x01    // host terminal
#define LOG_OUT_HOST_FILE           0x02    // host file
#define LOG_OUT_SOC_TERM            0x10    // soc terminal (UART)
#define LOG_OUT_SOC_HOST_TERM       0x20    // soc -> host terminal (DbgView)
#define LOG_OUT_SOC_HOST_FILE       0x40    // soc -> host file

// ===============================================================================================
//                                      log host cmd
// ===============================================================================================
//
// host cmd target 
#define LOG_HCMD_TARGET_HOST            0       // the log hcmd is run in host side
#define LOG_HCMD_TARGET_SOC             1       // the log hcmd is run in soc  side
// host cmd id
#define LOG_HCMD_TAG_ON_FL              1
#define LOG_HCMD_TAG_OFF_FL             2
#define LOG_HCMD_TAG_ON_LEVEL           3
#define LOG_HCMD_TAG_OFF_LEVEL          4
#define LOG_HCMD_TAG_ON_MODULE          5
#define LOG_HCMD_TAG_OFF_MODULE         6
#define LOG_HCMD_MODULE_ON              7
#define LOG_HCMD_MODULE_OFF             8
#define LOG_HCMD_LEVEL_SET              9
#define LOG_HCMD_PRINT_USAGE            10
#define LOG_HCMD_PRINT_STATUS           11
#define LOG_HCMD_PRINT_MODULE           12
#define LOG_HCMD_TEST                   13

// ===============================================================================================
//                                      log soc event
// ===============================================================================================
// abbreviation : 'SEVT' means 'soc event'
//
#define LOG_SEVT_MAX_LEN            (64 + LOG_MAX_PATH)     // reserved len for malloc()

#define LOG_SEVT_STATUS_REPORT      LOG_HCMD_PRINT_STATUS   // = 11

#define LOG_SEVT_PRNF_CFG_SET       20      // soc report the printf config (override-able) to host
#define LOG_SEVT_PRNF_BUF           21
#define LOG_SEVT_PRNF               22

#define LOG_SEVT_EXEC_FAIL          99
#define LOG_SEVT_ACK                100

#if defined(CONFIG_LOG_ENABLE) && (CONFIG_LOG_ENABLE == 1)
#include <log_enabled.h>
#else
#include <log_disabled.h>
#endif

// ===============================================================================================
//                              printf & fatal func & quick macro
// ===============================================================================================
// #include <lib/ssv_lib.h>

#define log_printf          printf

extern void soc_fatal(u32 m, const char *file, u32 line, const char *fmt, ...);
extern void soc_printf(u32 n, u32 m, const char *fmt, ...);

#define LOG_PRINTF_LM(l, m, fmt, ...)   \
    do { /*lint -save -e774 */ \
        if ((l) == LOG_LEVEL_FATAL) \
            fatal_printf(fmt, ##__VA_ARGS__); \
            /*soc_fatal(m, __FILE__, __LINE__, fmt, ##__VA_ARGS__); */ \
        else \
            printf(fmt, ##__VA_ARGS__); \
            /*soc_printf((l), (m), fmt, ##__VA_ARGS__);*/ /*lint -restore */ \
    } while (0) 


// quick macro
#define LOG_PRINTF(fmt, ...)        LOG_PRINTF_LM(LOG_LEVEL_ON,     LOG_MODULE_EMPTY, fmt, ##__VA_ARGS__)
#define LOG_TRACE(fmt,  ...)        LOG_PRINTF_LM(LOG_LEVEL_TRACE,  LOG_MODULE_EMPTY, fmt, ##__VA_ARGS__)
#define LOG_DEBUG(fmt,  ...)        LOG_PRINTF_LM(LOG_LEVEL_DEBUG,  LOG_MODULE_EMPTY, fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt,   ...)        LOG_PRINTF_LM(LOG_LEVEL_INFO,   LOG_MODULE_EMPTY, fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt,   ...)        LOG_PRINTF_LM(LOG_LEVEL_WARN,   LOG_MODULE_EMPTY, fmt, ##__VA_ARGS__)
#define LOG_FAIL(fmt,   ...)        LOG_PRINTF_LM(LOG_LEVEL_FAIL,   LOG_MODULE_EMPTY, fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt,  ...)        LOG_PRINTF_LM(LOG_LEVEL_ERROR,  LOG_MODULE_EMPTY, fmt, ##__VA_ARGS__)
#define LOG_FATAL(fmt,  ...)        LOG_PRINTF_LM(LOG_LEVEL_FATAL,  LOG_MODULE_EMPTY, fmt, ##__VA_ARGS__)
// 'M' means 'module'
#define LOG_PRINTF_M(m, fmt, ...)   LOG_PRINTF_LM(LOG_LEVEL_ON,                    m, fmt, ##__VA_ARGS__)
#define LOG_TRACE_M(m,  fmt, ...)   LOG_PRINTF_LM(LOG_LEVEL_TRACE,                 m, fmt, ##__VA_ARGS__)
#define LOG_DEBUG_M(m,  fmt, ...)   LOG_PRINTF_LM(LOG_LEVEL_DEBUG,                 m, fmt, ##__VA_ARGS__)
#define LOG_INFO_M(m,   fmt, ...)   LOG_PRINTF_LM(LOG_LEVEL_INFO,                  m, fmt, ##__VA_ARGS__)
#define LOG_WARN_M(m,   fmt, ...)   LOG_PRINTF_LM(LOG_LEVEL_WARN,                  m, fmt, ##__VA_ARGS__)
#define LOG_FAIL_M(m,   fmt, ...)   LOG_PRINTF_LM(LOG_LEVEL_FAIL,                  m, fmt, ##__VA_ARGS__)
#define LOG_ERROR_M(m,  fmt, ...)   LOG_PRINTF_LM(LOG_LEVEL_ERROR,                 m, fmt, ##__VA_ARGS__)
#define LOG_FATAL_M(m,  fmt, ...)   LOG_PRINTF_LM(LOG_LEVEL_FATAL,                 m, fmt, ##__VA_ARGS__)


#if (CONFIG_SIM_PLATFORM)
#define T(str)  str
#else
#define T(str) ({static const char s[] __attribute__((section(".dbgmsg"))) = str; s;})
#endif

typedef struct
{
    u32     lvl;
    u32     mod;
    u32     fl;

    u32     prn_tag_lvl;
    u32     prn_tag_mod;
    u32     prn_tag_sprs;
    u32     chk_tag_sprs;   // won't check 'level' & 'module', ONLY used by log_printf().
                            // this is used when log_printf() is inside LOG_TAG_SUPPRESS_ON() & LOG_TAG_SUPPRESS_OFF()
} log_prnf_cfg_st;

// for < LOG_SEVT_PRNF >
typedef struct log_evt_prnf_st_tag
{
    u32     len;        // total length
    u32     fmt_len;    // will always be 4 
    u32     arg_len;    // 0 or 4*N
    
    u32     lvl;
    u32     mod;
    // u32      line;
    // u8       file[LOG_MAX_PATH];     // NOT supported now. need to be done.

    log_prnf_cfg_st     prnf_cfg;   // current prnf cfg on soc side

    u8      buf[0];
} log_evt_prnf_st;

// printf string to SOC_HOST_TERM (defaultly, DbgView in WIN32)
#if (defined _WIN32)
    #define _prnf_soc_host_term     OutputDebugStringA
#else
    #define _prnf_soc_host_term     log_printf
#endif

#define LOG_EVT_PRNF_BUF_MAX    (LOG_DMSG_MAX_ARGS*sizeof(u32) + 32)    // 32 -> rsvd space for safety
#define LOG_EVT_PRNF_MAXLEN     (sizeof(log_evt_prnf_st) + LOG_EVT_PRNF_BUF_MAX)

// ===============================================================================================
//                                          Misc
// ===============================================================================================
extern void LOG_init(bool tag_level, bool tag_mod, u32 level, u32 mod_mask, bool fileline);

#endif  // _LOG_H_

