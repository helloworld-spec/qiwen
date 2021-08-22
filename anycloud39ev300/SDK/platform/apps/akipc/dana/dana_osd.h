#ifndef _DANA_OSD_H_
#define _DANA_OSD_H_

/**
 * dana_osd_get_ptr - get dana osd info ptr
 * @void: 
 * return: ptr of dana osd
 */
struct osd_ex_info* dana_osd_get_ptr(void);

/**
 * dana_osd_init - create thread to draw osd to screen on dana platform
 * @vi[IN]: vi handle  
 * return: 0 - success, fail return -1.
 */
int dana_osd_init(void *vi);

/**
 * dana_osd_exit - exit osd thread which is for dana platform  
 * return: void.
 */
void dana_osd_exit(void);

#endif
