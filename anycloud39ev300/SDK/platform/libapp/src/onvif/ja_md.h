#ifndef _JA_MD_H_
#define _JA_MD_H_
#include "n1_def.h"

typedef void P_MD_ALARM_CALLBACK(void);

/**
 * ja_md_load - load md struct from file
 * @pmotion[OUT]: md struct
 * return: 0 - success, fail return -1.
 */
int ja_md_load(struct NK_MotionDetection* pmotion);

/**
 * ja_md_store - store md struct to file
 * @pmotion[IN]: md struct
 * return: 0 - success, fail return -1.
 */
int ja_md_store(struct NK_MotionDetection* pmotion);

/**
 * ja_md_set_move_ratio - set md ratio according to md struct
 * @pmotion[IN]: md struct
 * return: void
 */
void ja_md_set_move_ratio(struct NK_MotionDetection* pmotion);

/**
 * ja_md_start_movedetection - start md
 * return: void
 */
void ja_md_start_movedetection(void);

/**
 * ja_md_stop_movedetection - stop md
 * return: void
 */
void ja_md_stop_movedetection(void);

/**
 * ja_md_init - init md module
 * @vi[IN]: opened vi handle
 * @pcallback[IN]: alarm callback function
 * return: void
 */
void ja_md_init(void *vi, P_MD_ALARM_CALLBACK pcallback);

/**
 * ja_md_destroy - destroy md module
 * return: void
 */
void ja_md_destroy(void);

#endif
