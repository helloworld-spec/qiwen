#ifndef __LOG_DISABLED_H__
#define __LOG_DISABLED_H__

#include <types.h>
#include <cmd_def.h>
#include <msgevt.h>


#define     LOG_DMSG_FILENAME   "ssv6200-uart.dmsg"
#define     LOG_MAX_TAG         16
#define     LOG_MAX_PATH            256
#define     LOG_MAX_FILENAME        48      // without directory path
#define     LOG_DMSG_MAX_ARGS   16

// global variables for acceleration
#define     PRN_BUF_MAX         256
#define     PRN_TAG_MAX         LOG_MAX_TAG
#define     ACC_BUF_MAX         (2*PRN_BUF_MAX + 3*PRN_TAG_MAX)
#define     LOG_FRMBUF_MAX      (16 * SIZE_1KB)
// extern u8    g_log_frmbuf[LOG_FRMBUF_MAX];

const char* g_log_lvl_tag[LOG_LEVEL_OFF];
const char* g_log_mod_tag[LOG_MODULE_ALL];

/*
 *
 */
#define LOG_MODULE_MASK(n)          (((n) == LOG_MODULE_ALL) ? 0xffffffffU :   (0x00000001U << (int)(n)))
#define LOG_MODULE_I_MASK(n)        (((n) == LOG_MODULE_ALL) ? 0x00000000U : (~(0x00000001U << (int)(n))))

#define LOG_LEVEL_SET(n)            { LOG_MUTEX_LOCK(); g_log_prnf_cfg.lvl = (n);                   LOG_MUTEX_UNLOCK(); }

#define LOG_MODULE_TURN_ON(n)       { LOG_MUTEX_LOCK(); g_log_prnf_cfg.mod |= LOG_MODULE_MASK(n);   LOG_MUTEX_UNLOCK(); }
#define LOG_MODULE_TURN_OFF(n)      { LOG_MUTEX_LOCK(); g_log_prnf_cfg.mod &= LOG_MODULE_I_MASK(n); LOG_MUTEX_UNLOCK(); }

#define LOG_FILELINE_TURN_ON()      { LOG_MUTEX_LOCK(); g_log_prnf_cfg.fl = 1;                      LOG_MUTEX_UNLOCK(); }
#define LOG_FILELINE_TURN_OFF()     { LOG_MUTEX_LOCK(); g_log_prnf_cfg.fl = 0;                      LOG_MUTEX_UNLOCK(); }

#define LOG_TAG_LEVEL_TURN_ON()     { LOG_MUTEX_LOCK(); g_log_prnf_cfg.prn_tag_lvl = true;          LOG_MUTEX_UNLOCK(); }
#define LOG_TAG_LEVEL_TURN_OFF()    { LOG_MUTEX_LOCK(); g_log_prnf_cfg.prn_tag_lvl = false;         LOG_MUTEX_UNLOCK(); }
#define LOG_TAG_LEVEL_TURN(x)       { LOG_MUTEX_LOCK(); g_log_prnf_cfg.prn_tag_lvl = (x);           LOG_MUTEX_UNLOCK(); }

#define LOG_TAG_MODULE_TURN_ON()    { LOG_MUTEX_LOCK(); g_log_prnf_cfg.prn_tag_mod = true;          LOG_MUTEX_UNLOCK(); }
#define LOG_TAG_MODULE_TURN_OFF()   { LOG_MUTEX_LOCK(); g_log_prnf_cfg.prn_tag_mod = false;         LOG_MUTEX_UNLOCK(); }
#define LOG_TAG_MODULE_TURN(x)      { LOG_MUTEX_LOCK(); g_log_prnf_cfg.prn_tag_mod = (x);           LOG_MUTEX_UNLOCK(); }

#define LOG_TAG_SUPPRESS_ON()       { LOG_MUTEX_LOCK(); g_log_prnf_cfg.prn_tag_sprs = true;         LOG_MUTEX_UNLOCK(); }
#define LOG_TAG_SUPPRESS_OFF()      { LOG_MUTEX_LOCK(); g_log_prnf_cfg.prn_tag_sprs = false;        LOG_MUTEX_UNLOCK(); }


// ===============================================================================================
//                                          log mutex
// ===============================================================================================
// just for debugging use
#define     LOG_MUTEX_DBGMSG    0
#define     LOG_MUTEX_DBG       0

#if (LOG_MUTEX_DBG == 0)
#define _os_mutex           OsMutex
#define _os_mutex_init      OS_MutexInit
#define _os_mutex_lock      OS_MutexLock
#define _os_mutex_unlock    OS_MutexUnLock
#else
#define _os_mutex           u32
#define _os_mutex_init(x)   (g_log_mutex = 0)
#define _os_mutex_lock(x)   (g_log_mutex++)
#define _os_mutex_unlock(x) (g_log_mutex--)
#endif


_os_mutex           g_log_mutex;
u32                 g_dbg_mutex;
//if (!gOsFromISR)
#define LOG_MUTEX_INIT()            \
{                                   \
    if (LOG_MUTEX_DBGMSG)   printf("%s() #%d: log mutex init   = %d\n\r", __FUNCTION__, __LINE__, (g_dbg_mutex = 0)); \
    _os_mutex_init(&g_log_mutex);   \
}
#define LOG_MUTEX_LOCK()            \
{                                   \
    if (!gOsFromISR)                \
        _os_mutex_lock(g_log_mutex);\
    if (LOG_MUTEX_DBGMSG)   printf("%s() #%d: log mutex lock   = %d\n\r", __FUNCTION__, __LINE__, (++g_dbg_mutex)); \
}
#define LOG_MUTEX_UNLOCK()          \
{                                   \
    if (LOG_MUTEX_DBGMSG)   printf("%s() #%d: log mutex unlock = %d\n\r", __FUNCTION__, __LINE__, (--g_dbg_mutex)); \
    if (!gOsFromISR)                \
    _os_mutex_unlock(g_log_mutex);  \
}

// ===============================================================================================
//                          log prnf cfg & output stream descriptor
// ===============================================================================================
extern log_prnf_cfg_st  g_log_prnf_cfg;

#define LOG_OUT_DST_IS_OPEN(x)      (g_log_out.dst_open & (x))
#define LOG_OUT_DST_OPEN(x)         { LOG_MUTEX_LOCK(); g_log_out.dst_open |= (x);      LOG_MUTEX_UNLOCK(); }
#define LOG_OUT_DST_CLOSE(x)        { LOG_MUTEX_LOCK(); g_log_out.dst_open &= (~(x));   LOG_MUTEX_UNLOCK(); }

#define LOG_OUT_DST_IS_CUR_ON(x)    (g_log_out.dst_cur & (x))
#define LOG_OUT_DST_CUR_ON(x)       { LOG_MUTEX_LOCK(); g_log_out.dst_cur |= (x);       LOG_MUTEX_UNLOCK(); }
#define LOG_OUT_DST_CUR_OFF(x)      { LOG_MUTEX_LOCK(); g_log_out.dst_cur &= (~(x));    LOG_MUTEX_UNLOCK(); }

// #define LOG_OUT_FILE_MODE        "a+"    // mode for fopen()
#define LOG_OUT_FILE_MODE       "w" // mode for fopen()

typedef struct
{
    u8      dst_open;           // opened  destination
    u8      dst_cur;            // current destination
    void    *fp;                // the log file ptr  on host side
    u8      path[LOG_MAX_PATH]; // the log file path on host side
} log_out_desc_st;
extern log_out_desc_st  g_log_out;

#if (CONFIG_SIM_PLATFORM)
#define LOG_out_dst_open        _sim_out_dst_open
#define LOG_out_dst_close       _sim_out_dst_close
#define LOG_out_dst_turn_on     _sim_out_dst_turn_on
#define LOG_out_dst_turn_off    _sim_out_dst_turn_off
#else
#define LOG_out_dst_open        _soc_out_dst_open
#define LOG_out_dst_close       _soc_out_dst_close
#define LOG_out_dst_turn_on     _soc_out_dst_turn_on
#define LOG_out_dst_turn_off    _soc_out_dst_turn_off
#endif

extern bool LOG_out_init(void);
extern bool LOG_out_dst_open(u8 _dst, const u8 *_path);
extern bool LOG_out_dst_close(u8 _dst);
extern bool LOG_out_dst_turn_on(u8 _dst);
extern bool LOG_out_dst_turn_off(u8 _dst);
extern void LOG_out_desc_dump(void);

// < log host cmd > (for now, fixed length)
typedef struct
{
    u8      target;
    u8      cmd;
    u8      data;
} log_hcmd_st;

extern void     LOG_hcmd_dump(log_hcmd_st *hcmd);
extern bool     LOG_soc_exec_hcmd(log_hcmd_st *hcmd);


// ===============================================================================================
//                                      log soc event
// ===============================================================================================
// abbreviation : 'SEVT' means 'soc event'
//

typedef struct
{
    u16     len;    // total length
    u16     cmd;
    u8      data[0];    
} log_soc_evt_st;

#define LOG_SEVT_CMD_DATA_OFFSET        (sizeof(log_soc_evt_st))


// note : We should move HOST_EVENT_XXX() to msgevt.h & .c.
//        For these macros, plz refer to HOST_EVENT_XXX() in cmd_def.h 
#define LOG_EVENT_SEND(ev)                          \
{                                                   \
    msg_evt_post(MBOX_CMD_ENGINE, (ev));            \
}
#define LOG_EVENT_SET_LEN(ev, l)                    \
{                                                   \
    ((HDR_HostEvent *)((ev)->MsgData))->len =       \
    (l) + sizeof(HDR_HostEvent);                    \
}
#define LOG_EVENT_ALLOC(ev, evid, l)                \
{                                                   \
    (ev) = _log_evt_alloc(evid, l);                 \
    ASSERT(ev);                                     \
}
#define LOG_EVENT_FREE(ev)                          \
{                                                   \
    _log_evt_free(ev);                              \
}
#define LOG_EVENT_DATA_PTR(ev)                      ((HDR_HostEvent *)((ev)->MsgData))->dat

extern MsgEvent*    _log_evt_alloc(ssv6xxx_soc_event hEvtID, s32 size);
extern void         _log_evt_free(MsgEvent *MsgEvt);


// for < LOG_SEVT_PRNF_BUF >
typedef struct
{
    u16     len;                // total length
    u8      dst_open;           // opened  destination
    u8      dst_cur;            // current destination
    u16     fmt_len;
    u16     arg_len;
    u8      buf[0];
} log_evt_prnf_buf_st;

// < LOG_SEVT_STATUS_REPORT >
typedef struct  
{
    log_prnf_cfg_st     prnf_cfg;
    log_out_desc_st     out_desc;
} log_evt_status_report_st;


// ===============================================================================================
//                                      dbgmsg stripping
// ===============================================================================================
#if (CONFIG_SIM_PLATFORM)

#define LOG_DMSG_MAX_ARGS   16

// struct for dbgmsg stripping
typedef struct
{
    bool    is_load;
    FILE    *fp;
    u32     size;       // file size, including the last 4-bytes addr
    u32     addr;       // [.dmsg] section addr for g_log_dmsg
                        // unused               for g_log_dbin
    u8      *buf;
} log_dbgmsg_st;
extern log_dbgmsg_st    g_log_dmsg;
extern log_dbgmsg_st    g_log_dbin; 

extern bool  LOG_dmsg_load(char path[256]);
// extern bool  LOG_dbin_load(char path[256]);
#endif


extern u8   g_magic_str[8];
extern u32  g_magic_u32;

// for < LOG_SEVT_PRNF_CFG_SET >
#define log_evt_prnf_cfg_set_st     log_prnf_cfg_st

#define t_printf(l, m, fmt, ...)        _t_printf(l, m, fmt, ##__VA_ARGS__, g_magic_str, g_magic_u32)

extern bool log_host_exec_evt_prnf(log_evt_prnf_st *prnf);
extern void _t_printf(u32 n, u32 m, const char *fmt, ...);

/* ================================= static functions  ======================================== */
//
// void _log_evt_prn_buf_dump(log_evt_prnf_buf_st *pkt);
// void _log_prnf_cfg_dump(log_prnf_cfg_st *p);
// void _log_out_dst_open_dump(u8 dst_open);
// void _log_out_dst_cur_dump(u8 dst_cur);
// void _log_out_desc_dump(log_out_desc_st *p);
// void _log_soc_send_evt_prn_buf(const char *buf);
// void _log_soc_send_evt_prnf_cfg_set(const u8 *p);
// void _log_tag_lvl_str(const u32 n, u8 str[LOG_MAX_TAG]);
// u32  _log_tag_mod_str(const u32 m, u8 str[LOG_MODULE_ALL][LOG_MAX_TAG]);

#endif