#ifndef _MSG_Q_H_
#define _MSG_Q_H_
#include "xrf_api.h"


int msgget(msg_q_t *msg_q, int q_size);
int msgsnd(msg_q_t msgid, void *msgbuf);
int msgrcv(msg_q_t msgid, void **msgbuf, unsigned int timeout);
int msgfree(msg_q_t msg_q);
void msg_q_init(void);

#endif
