#ifndef _DANA_DEBUG_H_
#define _DANA_DEBUG_H_

#ifdef __cplusplus
extern "C" {
#endif

void dbg_on();
void dbg_off();

void dbg(const char *msg, ...);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* _DANA_DEBUG_H_ */
