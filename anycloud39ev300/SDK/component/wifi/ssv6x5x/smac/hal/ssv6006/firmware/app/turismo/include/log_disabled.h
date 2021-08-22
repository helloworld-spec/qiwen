#ifndef __LOG_DISABLED_H__
#define __LOG_DISABLED_H__

/*
 *
 */
#define LOG_MODULE_MASK(n)
#define LOG_MODULE_I_MASK(n)

#define LOG_LEVEL_SET(n)

#define LOG_MODULE_TURN_ON(n)
#define LOG_MODULE_TURN_OFF(n)

#define LOG_FILELINE_TURN_ON()
#define LOG_FILELINE_TURN_OFF()

#define LOG_TAG_LEVEL_TURN_ON()
#define LOG_TAG_LEVEL_TURN_OFF()
#define LOG_TAG_LEVEL_TURN(x)

#define LOG_TAG_MODULE_TURN_ON()
#define LOG_TAG_MODULE_TURN_OFF()
#define LOG_TAG_MODULE_TURN(x)

#define LOG_TAG_SUPPRESS_ON()
#define LOG_TAG_SUPPRESS_OFF()


#define LOG_MUTEX_INIT()

#define LOG_MUTEX_LOCK()

#define LOG_MUTEX_UNLOCK()


#define LOG_OUT_DST_IS_OPEN(x)
#define LOG_OUT_DST_OPEN(x)
#define LOG_OUT_DST_CLOSE(x)

#define LOG_OUT_DST_IS_CUR_ON(x)
#define LOG_OUT_DST_CUR_ON(x)
#define LOG_OUT_DST_CUR_OFF(x)

// ===============================================================================================
//                                      log soc event
// ===============================================================================================
// abbreviation : 'SEVT' means 'soc event'
//

// note : We should move HOST_EVENT_XXX() to msgevt.h & .c.
//        For these macros, plz refer to HOST_EVENT_XXX() in cmd_def.h 
#define LOG_EVENT_SEND(ev)

#define LOG_EVENT_SET_LEN(ev, l)

#define LOG_EVENT_ALLOC(ev, evid, l)

#define LOG_EVENT_FREE(ev)

#define LOG_EVENT_DATA_PTR(ev)


// ===============================================================================================
//                                      dbgmsg stripping
// ===============================================================================================

#define t_printf(l, m, fmt, ...)

#endif // __LOG_DISABLED_H__
