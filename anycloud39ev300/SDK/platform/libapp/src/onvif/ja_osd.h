#ifndef JA_OSD_H_
#define JA_OSD_H_
#ifdef __cplusplus
extern "C" {
#endif

/**
 * ja_osd_init - create thread to draw osd to screen on ja platform
 * @vi[IN]: vi handle  
 * return: 0 - success, fail return -1.
 */
int ja_osd_init(void *vi);

/**
 * ja_osd_exit - exit osd thread which is for ja platform  
 * return: void.
 */
void ja_osd_exit(void);


/**
 * ja_osd_init_mutex - init ja osd mutex
 * return: void.
 */
void ja_osd_init_mutex(void);

/**
 * ja_osd_destroy_mutex - destroy ja osd mutex
 * return: void.
 */
void ja_osd_destroy_mutex(void);

#ifdef __cplusplus
};
#endif
#endif //JA_OSD_H_

