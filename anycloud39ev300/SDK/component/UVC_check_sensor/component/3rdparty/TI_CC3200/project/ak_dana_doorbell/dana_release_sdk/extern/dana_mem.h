#ifndef _DANA_MEM_H_
#define _DANA_MEM_H_

#include "dana_base.h"
#include "stdarg.h"

#ifdef __cplusplus
extern "C"
{
#endif


extern void *dana_malloc(size_t size);

extern void *dana_calloc(size_t nmemb, size_t size);

extern void dana_free(void *ptr);

extern void *dana_memcpy(void *dest, const void *src, size_t n);

extern void *dana_memset(void *s, int c, size_t n);

extern uint32_t dana_strlen(const char *s);

extern int32_t dana_atoi(const char *s);

extern int32_t dana_vsnprintf(char *s, uint32_t size, const char *template, va_list ap);

extern void  dana_printf(char *args,...) ;                   
//TODO strlen atoi printf



#ifdef __cplusplus
extern "C"
{
#endif
#endif
