#ifndef __MEMORY_H__
#define __MEMORY_H__

void wifi_freeAndTrace(void * pData, char * file_name, int line);
void * wifi_mallocAndTrace(unsigned int  size, char * file_name, int line );
void *wifi_callocAndTrace(unsigned int count, unsigned int size ,char * file_name, int line);


//#define mem_malloc(SIZE) Fwl_Malloc(SIZE)
//#define mem_calloc(SIZE, COUNT) mem_calloc_ex((SIZE), (COUNT))
//#define mem_free(X) Fwl_Free(X)

#define wifi_malloc(size)           wifi_mallocAndTrace((size),__FILE__,__LINE__)
#define wifi_calloc(n, size)         wifi_callocAndTrace((n),(size),__FILE__,__LINE__)
#define wifi_free(x)             wifi_freeAndTrace(x,__FILE__,__LINE__)

#define mem_malloc(SIZE)         wifi_malloc(SIZE)
#define mem_calloc(COUNT, SIZE)  wifi_calloc((COUNT), (SIZE))
#define mem_free(X)              wifi_free(X)


//#define malloc 		mem_malloc
//#define free 		mem_free

#endif /*__LWIP_MEM_H__*/
