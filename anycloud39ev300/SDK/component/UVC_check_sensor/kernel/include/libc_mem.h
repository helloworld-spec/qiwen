#ifndef __LIBC_MEM_H__
#define __LIBC_MEM_H__

#include "os_malloc.h"

#define malloc(size)        Fwl_MallocAndTrace((size), ((char*)(__FILE__)), ((unsigned long)__LINE__))
#define remalloc(var, size) Fwl_ReMallocAndTrace((var), (size), ((char*)(__FILE__)), ((unsigned long)__LINE__))
#define calloc(num, size)   Fwl_CallocAndTrace((num),(size), ((char*)(__FILE__)), ((unsigned long)__LINE__))
#define free(var)           Fwl_FreeAndTrace((var), ((char*)(__FILE__)), ((unsigned long)__LINE__)) 

/* use for register call */
static inline void* malloc_cb(unsigned int size)
{	
    return Fwl_MallocAndTrace((size), ((char*)(__FILE__)), ((unsigned long)__LINE__));
}

static inline void* free_cb(void *var)
{
    return Fwl_FreeAndTrace((var), ((char*)(__FILE__)), ((unsigned long)__LINE__)); 
}

#endif //__LIBC_MEM_H__