#include <stddef.h>
#include "os_malloc.h"

void *__wrap_malloc(size_t size)
{    
    return Fwl_MallocAndTrace((size), ((char*)(__FILE__)), ((unsigned long)__LINE__));
}

void *__wrap_calloc(size_t nmemb, size_t size)
{    
    return Fwl_CallocAndTrace(nmemb, size, ((char*)(__FILE__)), ((unsigned long)__LINE__));
}
 
void *__wrap_realloc(void *ptr, size_t size)
{    
	return Fwl_ReMallocAndTrace(ptr, size, ((char*)(__FILE__)), ((unsigned long)__LINE__));
}

void __wrap_free(void *ptr)
{
    return Fwl_FreeAndTrace(ptr, ((char*)(__FILE__)), ((unsigned long)__LINE__)); 
}

