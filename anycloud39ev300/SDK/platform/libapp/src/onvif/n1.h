#ifndef _N1_H_
#define _N1_H_

/**
 * n1_init - init n1sdk
 * @listen_port[IN]: listen port
 * return: void
 */
void n1_init(int listen_port);

/**
 * n1_destroy - destroy n1sdk
 * return: void
 */
void n1_destroy(void);

#endif
