/**
 * @FILENAME akuio.c
 * @BRIEF This file provide the APIs implement of libakuio.
 * Copyright (C) 2011 Anyka(Guangzhou) Microelectronics Technology CO.,LTD.
 * @AUTHOR Jacky Lau
 * @DATE 2011-03-15
 * @VERSION 0.1
 * @VERSION 0.2, add use and free list to manage alloc and free.
 *			huang_haitao, 2017.02.08
 * @REF Please refer to akuio.h
 */

#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include "include/linux/akuio_driver.h"
#include "list.h"
#include "log.h"
#include "akuio.h"
#include "ion.h"

#define NONE          		"\033[m"          //close
#define YELLOW        		"\033[1;33m"		//yellow

#define LOG_TAG 			"[libakuio]"
#define MODULE_TAG 			"libakuio"

#define TRACE_DMA        	1
#define TRACE_UIO        	1
#define TRACE_BITS_WRITE 	0
#define LIST_MOTHED_DEBUG	0
#define VA2PA_MATCH			0
#define PMEM_USE_INFO		0

#define LEFT_ALIGN(a,b)  ((a/b)*b)
#define RIGHT_ALIGN(a,b) ((a+b-1)/b*b)

#define PMEM_FILE 			"/dev/ion"
#define UIO_VCODEC_DEV_NAME "/dev/uio0"
#define RESERVE_SZ_PATH		"/sys/ak_info_dump/reserved_mem_size"

struct pmem_t {
	unsigned long size;		//current size	
	unsigned long paddr;	//physical addr	
	void *vaddr;			//virtual addr	
	bool enable;	
	struct list_head list;	//list head
};

/* Device Registers mapping */
struct uio_t {
	unsigned int uio_index;
	unsigned int map_index;
	int          uio_fd;
	unsigned int paddr;
	void         *vaddr;
	unsigned int offset;
	unsigned int size;
	struct list_head list;
};

/* free list node info */
struct free_pmem_node {
	unsigned long length;		//make sure (length = end_offset - start_offset)
	unsigned long start_offset; // start offset of using pmem
	unsigned long end_offset; 	// end offset of using pmem
	struct list_head list;		//list head
};

struct pmem_info {
	int fd;
	void *lock_handle;
	unsigned long paddr;
	void *vaddr;
	unsigned long length;
	unsigned long original_offset; 	// pmem stack pointer
	unsigned long cur_offset; 		// current offset of malloc pmem
	struct list_head free_list;		// free pmem list
	struct list_head using_list;	// using pmem list
	int init;						// pmem initializing counter
};

static struct pmem_info pmem = {0};
static const char akuio_version[] = "libakuio.so V3.1.01";

static pthread_mutex_t pmem_mutex = PTHREAD_MUTEX_INITIALIZER;

/* for device driver support */
static int uio0_fd = 0;
static struct list_head uio0_list_head;
static unsigned int remmap_count=0;

static struct ion_allocation_data iad;
static struct ion_fd_data ifd;

/** get_available_pmem: get available pmem, we just check in free list
 * @malloc_size[IN]: malloc size
 * @using_pmem[OUT]: using pmem info
 * return: NULL failed, otherwize free pmem node
 */
static int get_available_pmem(unsigned int malloc_size, struct pmem_t *using_pmem)
{
	struct free_pmem_node *node = NULL;
	struct list_head *ptr = NULL;
	struct list_head *pn = NULL;
	
	list_for_each_safe(ptr, pn, &(pmem.free_list)) {
		node = list_entry(ptr, struct free_pmem_node, list);
		if((NULL != node) && (malloc_size <= node->length)) {
#if LIST_MOTHED_DEBUG	
			printf("\t *** get_available_pmem node=0x%lX, node->length=%ld ***\n", 
				(unsigned long)node, node->length);
			printf("\t *** node->start_offset=%ld ***\n", node->start_offset);
#endif
			
			using_pmem->size = malloc_size;
			using_pmem->paddr = pmem.paddr + node->start_offset;
			using_pmem->vaddr = pmem.vaddr + node->start_offset;
#if TRACE_DMA
			LOGD ("%s: %ubytes, paddr=0x%08lx, vaddr=%p\n", __func__, 
				using_pmem->size, using_pmem->paddr, using_pmem->vaddr);
#endif
		
			/* add to pmem_list */
			using_pmem->enable = true;
			list_add_tail(&(using_pmem->list), &(pmem.using_list));
	
			node->length -= malloc_size;
			node->start_offset += malloc_size;
			if(0 == node->length) {
				list_del_init(ptr);
                free(node);
                node = NULL;
			}
			
			return 0;
		}
	}

	LOGE ("no space for allocating %ubytes of pmem\n", malloc_size);
	return -1;
}

/** adjust_free_pmem_list: adjust free pmem list after akuio_free_pmem
 * @void
 * return: none
 * notes: list node may join into a large free space each other.
 */
static void adjust_free_pmem_list(void)
{
	unsigned char match = 0;
	struct free_pmem_node *out_node = NULL;
	struct list_head *out_ptr = NULL;
	struct list_head *out_pn = NULL;
	struct free_pmem_node *in_node = NULL;
	struct list_head *in_ptr = NULL;
	struct list_head *in_pn = NULL;
	
	/* find sequence free space and join to a big free space */
	list_for_each_safe(out_ptr, out_pn, &(pmem.free_list))
	{
		out_node = list_entry(out_ptr, struct free_pmem_node, list);
		if(NULL == out_node)
		{
			continue;
		}

		match = 0;
		list_for_each_safe(in_ptr, in_pn, &(pmem.free_list))
		{
			in_node = list_entry(in_ptr, struct free_pmem_node, list);
			if(NULL == in_node)
			{
				continue;
			}

			/* the same node */
			if(out_node == in_node)
			{
				continue;
			}
			
#if LIST_MOTHED_DEBUG		
			printf("\t *** out_node->start_offset=%ld ***\n", out_node->start_offset);
			printf("\t *** out_node->end_offset=%ld ***\n", out_node->end_offset);
			printf("\t *** out_node->length=%ld ***\n", out_node->length);
			printf("\t *** in_node->start_offset=%ld ***\n", in_node->start_offset);
			printf("\t *** in_node->end_offset=%ld ***\n", in_node->end_offset);
			printf("\t *** in_node->length=%ld ***\n", in_node->length);
#endif				
			if((out_node->start_offset + out_node->length) == in_node->start_offset)
			{		
				/* join before current in_node free space */
				in_node->start_offset = out_node->start_offset;
				in_node->length += out_node->length;
				match = 1;
				break;
			}

			if(out_node->start_offset == in_node->end_offset)
			{			
				/* join after current in_node free space */
				in_node->end_offset += out_node->length;
				in_node->length += out_node->length;
				match = 1;
				break;
			}
		}

		if(match)
		{
			match = 0;
			list_del_init(out_ptr);
			free(out_node);
			out_node = NULL;
		}
	}
}

/** recovery_pmem: recovery pmem when akuio_free_pmem
 * @free_pmem[INT]: current free pmem info
 * return: 0 success, otherwize failed
 */
static int recovery_pmem(struct pmem_t *free_pmem)
{
	if(NULL == free_pmem)
	{
		return -1;
	}
	
	unsigned long start_offset = 0;
	struct free_pmem_node *node = NULL;
	struct list_head *ptr = NULL;
	struct list_head *pn = NULL;

	start_offset = free_pmem->paddr - pmem.paddr;
#if LIST_MOTHED_DEBUG	
	printf("*** recovery_pmem free start_offset=%ld ***\n", start_offset);
	printf("*** paddr=0x%lX, vaddr=%p, size=%ld ***\n", 
		free_pmem->paddr, free_pmem->vaddr, free_pmem->size);
#endif		
	
	/* find sequence free space and join to a big free space */
	list_for_each_safe(ptr, pn, &(pmem.free_list))
	{
		node = list_entry(ptr, struct free_pmem_node, list);
		if(NULL != node)
		{
#if LIST_MOTHED_DEBUG		
			printf("\t *** node->start_offset=%ld ***\n", node->start_offset);
			printf("\t *** node->end_offset=%ld ***\n", node->end_offset);
			printf("\t *** start_offset + free_pmem->size=%ld ***\n\n", 
				(start_offset + free_pmem->size));
#endif				
			if((start_offset + free_pmem->size) == node->start_offset)
			{
#if LIST_MOTHED_DEBUG			
				printf("\t *** join before current free space ***\n");
#endif				
				/* join before current free space */
				node->start_offset = start_offset;
				node->length += free_pmem->size;
				return 0;
			}

			if(start_offset == node->end_offset)
			{
#if LIST_MOTHED_DEBUG			
				printf("\t *** join after current free space ***\n");
#endif				
				/* join after current free space */
				node->end_offset += free_pmem->size;
				node->length += free_pmem->size;
				return 0;
			}
		}
	}

#if LIST_MOTHED_DEBUG
	printf("\t *** new free space add to free list ***\n");
#endif

	/* new free space */
	node = malloc(sizeof(struct free_pmem_node));
	if(NULL != node)
	{
		node->start_offset = start_offset;
		node->end_offset = start_offset + free_pmem->size;
		node->length = free_pmem->size;
		list_add_tail(&(node->list), &(pmem.free_list));
	}

#if LIST_MOTHED_DEBUG
	/* dump list */
	list_for_each_entry(node, &(pmem.free_list), list)
	{
		printf("*** free_list node=0x%lX, start_offset=%ld, "
			"end_offset=%ld, length=%ld ***\n", 
			(unsigned long)node,
			node->start_offset, node->end_offset, node->length);
	}
#endif	
	
	return 0;
}

static void release_uio_resource(struct uio_t *uio, unsigned int size)
{
#if TRACE_UIO
  	LOGD ("%s: unmap %p(total size: %i), size = %i\n", __func__, 
  		uio->vaddr, uio->size, size);
#endif

	if(-1 == munmap(uio->vaddr, size)) {
		LOGE("##CALL munmap FAILED3,uio->vaddr[%p]size[%d]\n", uio->vaddr,size);
	}

    if((uio->paddr >= 0x08000000) && (uio->paddr <= 0x08000017)) {
        if(remmap_count > 0) {
            close(uio->uio_fd);
            remmap_count--;
            LOGD("uio1 common register munmap! remmap_count=%d\n", remmap_count);
        }
    }

    if (uio->size == size) {
        list_del_init(&(uio->list));
        free(uio);
        uio = NULL;
    } else {
        LOGW ("unmap partial register region\n");
        uio->vaddr += size;
        uio->size -= size;
    }
}

/**akuio_get_used_pmem
 * @void
 * @return: current used pmem in bytes
 */
unsigned long akuio_get_used_pmem(void)
{
	unsigned long total = 0;
	struct pmem_t *entry = NULL;
	
	pthread_mutex_lock(&pmem_mutex);
	list_for_each_entry(entry, &(pmem.using_list), list) {
		total += entry->size;
		LOGE("*** used entry=0x%lX, vaddr=0x%lX, paddr=0x%lX, size=%ld ***\n", 
			(unsigned long)entry,
			(unsigned long)(entry->vaddr), 
			(unsigned long)(entry->paddr), 
			entry->size);
	}	
	pthread_mutex_unlock(&pmem_mutex);
	
	return total;
}

/**akuio_get_free_pmem
 * @void
 * @return: current free pmem in bytes
 */
unsigned long akuio_get_free_pmem(void)
{
	unsigned long total = 0;
	struct free_pmem_node *node = NULL;
	
	pthread_mutex_lock(&pmem_mutex);
	list_for_each_entry(node, &(pmem.free_list), list) {
		total += node->length;
#if PMEM_USE_INFO		
		LOGE("*** free node=0x%lX, start_offset=%ld, "
			"end_offset=%ld, length=%ld ***\n", 
			(unsigned long)node,
			node->start_offset, node->end_offset, node->length);
#endif			
	}	
	pthread_mutex_unlock(&pmem_mutex);
	
	return total;
}

/**
 * akuio_pmem_init - Init pmem
 * @void: 
 * return: 0 success; -1 failed
 */
int akuio_pmem_init(void)
{
    LOGD ("+ %s pmem.init=%d", __func__, pmem.init);

    pthread_mutex_lock(&pmem_mutex);
    if(pmem.init > 0) {
        pmem.init++;
        LOGD("%s: already inited, pmem.init=%d\n", 
        	__func__, pmem.init);
        pthread_mutex_unlock( &pmem_mutex );
        LOGD("- %s\n", __func__);
        return 0;
    }
    
	/* get pmem_size */
	char buf[100] = {0};
	FILE *fp = fopen(RESERVE_SZ_PATH, "r");
	if (!fp) {
		printf ("====open %s error: %s=========\n", PMEM_FILE, strerror(errno));
		goto _pmem_err_0;
	}
	fread(buf, 1, sizeof(buf), fp);
	fclose(fp);
	fp = NULL;

	unsigned long pmem_size = strtol(buf, NULL, 16);
	printf(YELLOW"\n----- %s -----\n"NONE, akuio_version);
	printf("[%s][%s] kernel config reserve mem size: %lu, %s\n", 
		MODULE_TAG, PMEM_FILE, pmem_size, buf);

	pmem.fd = open(PMEM_FILE, O_RDONLY);
	if (pmem.fd < 0) {
		printf ("====open %s error: %s=========\n", PMEM_FILE, strerror(errno));
		goto _pmem_err_0;
	}

	if(fcntl(pmem.fd , F_SETFD, FD_CLOEXEC) == -1) {
		fprintf(stderr, "[%s;%s]error:%s\n", __FILE__, __func__, strerror(errno));
	}

	pmem.length = pmem_size;
	iad.len = pmem.length;
	iad.flags = ION_HEAP_TYPE_CARVEOUT | ION_HEAP_TYPE_SYSTEM_CONTIG;
	iad.align = 4096;
	iad.heap_id_mask = 0xFF;
	if (ioctl(pmem.fd, ION_IOC_ALLOC, &iad) < 0) {
		fprintf(stderr, "ION: alloc %d bytes for process %d failed\n",
				iad.len, getpid());
		goto _pmem_err_1;
	}

	fprintf(stdout, "[%s]ION: alloc %d bytes for process %d\n",
		MODULE_TAG, iad.len, getpid());

	ifd.handle = iad.handle;
	if (ioctl(pmem.fd, ION_IOC_MAP, &ifd) < 0) {
		fprintf(stderr, "ION: yield fd for iad 0x%x failed!\n", (unsigned int)&iad);
		goto _pmem_err_1;
	}
	
	fprintf(stdout, "[%s]ION: yield %d for iad 0x%x, len: %d\n", 
		MODULE_TAG, ifd.fd, (unsigned int)&iad, iad.len);

	pmem.vaddr = mmap(0, iad.len, (PROT_READ | PROT_WRITE),  MAP_SHARED, 
		ifd.fd, 0);
	if (MAP_FAILED == pmem.vaddr) {
		LOGE ("map %s error: %s\n", PMEM_FILE, strerror(errno));
		goto _pmem_err_1;
	}
	LOGI ("PMEM VADDR: %p\n", pmem.vaddr);

	pmem.original_offset = 0;
	pmem.paddr = 0x80000000;	
	LOGI ("PMEM_GET_PHYS: 0x%08lx\n", pmem.paddr);
	
	/* always show this message */
    printf("\n--------------- pmem info ---------------\n");
    printf("    PMEM_GET_TOTAL_SIZE: %lu bytes\n", pmem.length);
    printf("    PMEM_GET_PHYS: 0x%08lX\n", pmem.paddr);
    printf("    pmem.vaddr: %p\n", pmem.vaddr);
    printf("--------------- info End ---------------\n\n");
    
	uio0_fd = open(UIO_VCODEC_DEV_NAME, O_RDWR);
	if (uio0_fd < 0) {
		LOGE ("UIO0device open failed: %s\n", strerror(errno));
		goto _pmem_err_1;
	}

	pmem.init++;
    pmem.cur_offset = pmem.original_offset;
    INIT_LIST_HEAD(&(pmem.free_list));
	INIT_LIST_HEAD(&(pmem.using_list));
	INIT_LIST_HEAD(&uio0_list_head);

	/* available free space */
	struct free_pmem_node *node = malloc(sizeof(struct free_pmem_node));
	if(NULL != node) {
		node->start_offset = pmem.original_offset;
		node->end_offset = pmem.length;
		node->length = (node->end_offset - node->start_offset);
		list_add_tail(&(node->list), &(pmem.free_list));
	}
	
#if LIST_MOTHED_DEBUG
	/* dump list */
	list_for_each_entry(node, &(pmem.free_list), list) {
		printf("*** free_list node=0x%lX, start_offset=%ld, "
			"end_offset=%ld, length=%ld ***\n", 
			(unsigned long)node,
			node->start_offset, node->end_offset, node->length);
	}
#endif

    pthread_mutex_unlock(&pmem_mutex);
    LOGD("- %s  pmem.init=%d\n", __func__, pmem.init);
	printf("####akuio pmem init success###\n");
	
    return 0;

    //Error handling:
_pmem_err_1:
    close(pmem.fd);
_pmem_err_0:
    pthread_mutex_unlock(&pmem_mutex);
    LOGD("- %s error", __func__);
    
    return -1;
}

/**
 * akuio_pmem_fini - Free pmem resource
 * @void: 
 * return: 0 success; -1 failed
 */
int akuio_pmem_fini(void)
{
    LOGD ("+ %s pmem.init=%d\n", __func__, pmem.init);

    pthread_mutex_lock(&pmem_mutex);
    if (pmem.init > 1) {
        pmem.init--;
        LOGD ("%s: refrence count>1, pmem.init=%d\n", 
        	__func__, pmem.init);
        pthread_mutex_unlock(&pmem_mutex);
        LOGD ("- %s\n", __func__);
        return 0;
    }

	
	if(!list_empty(&(pmem.using_list))) {
		LOGW("pmem.using_list is not empty!\n");
	}
	
    struct pmem_t *entry = NULL;
	struct list_head *ptr = NULL;
	struct list_head *pn = NULL;

	list_for_each_safe(ptr, pn, &(pmem.using_list)) {
		entry = list_entry(ptr, struct pmem_t, list);
		if(NULL != entry) {
            free(entry);
            entry = NULL;
		}
		list_del_init(ptr);
	}

	if(-1 == munmap(pmem.vaddr, pmem.length)) {
        LOGE("##CALL munmap FAILED2, pmem.vaddr[%p], pmem.length[%ld]", 
        	pmem.vaddr, pmem.length);
    }

	close(pmem.fd);
    pmem.init--;
    pthread_mutex_unlock(&pmem_mutex);
    LOGD ("- %s pmem.init=%d\n", __func__, pmem.init);

    return 0;
}

/**************** DMA memory ****************/
/* use pmem driver for physical memory allocation. */
/**
 * akuio_alloc_pmem - Allocate a piece of physically continuous memory
 * @size: size of physically continuous memory region
 * return: virtual addr, according to the physically continuous memory; 
 *		NULL failed
 */
void* akuio_alloc_pmem(unsigned int size)
{
    pthread_mutex_lock(&pmem_mutex);
    if(pmem.init <= 0) {
        pthread_mutex_unlock(&pmem_mutex);
        LOGE("%s error: akuio not init\n", __func__);
        return NULL;
    }

#if TRACE_DMA
	LOGD ("%s: %ubytes\n", __func__, size);
	LOGD ("pmem.fd = %i\n", pmem.fd);
#endif

	struct pmem_t *entry = malloc(sizeof(struct pmem_t));
	if(NULL == entry) {
		LOGE ("malloc entry is NULL\n");
		pthread_mutex_unlock(&pmem_mutex);
		return NULL;
	}
	
	size = RIGHT_ALIGN(size, 32);
	if(get_available_pmem(size, entry) < 0) {
		free(entry);
		entry = NULL;
		pthread_mutex_unlock(&pmem_mutex);
		akuio_get_used_pmem();
		akuio_get_free_pmem();
		return NULL;
	}

#if LIST_MOTHED_DEBUG
	/* dump list */
	struct pmem_t *tmp_entry = NULL;

	/* find in old list */
	list_for_each_entry(tmp_entry, &(pmem.using_list), list) {
		printf("\t *** tmp_entry=0x%lX, vaddr=0x%lX, paddr=0x%lX ***\n", 
			(unsigned long)tmp_entry,
			(unsigned long)(tmp_entry->vaddr), 
			(unsigned long)(tmp_entry->paddr));
	}
#endif	
	
	pthread_mutex_unlock(&pmem_mutex);

#if PMEM_USE_INFO
	akuio_get_used_pmem();
	akuio_get_free_pmem();
#endif
	
	return entry->vaddr;
}

/**
 * akuio_free_pmem - Free the appointed physically continuous memory
 * @vaddr: virtual addr, according to the physically continuous memory
 * return: none
 */
void akuio_free_pmem(void *vaddr)
{
	if(NULL == vaddr) {
		LOGE ("%s input arg is NULL\n", __func__);
		return;
	}

#if LIST_MOTHED_DEBUG
	printf("\t --- entering %s ---\n", __func__);
#endif

	pthread_mutex_lock(&pmem_mutex);
	if(pmem.init <= 0) {
		pthread_mutex_unlock(&pmem_mutex);
		LOGE("%s error: akuio not init\n", __func__);
		return ;
	}

#if TRACE_DMA
	LOGD ("%s(): %p\n", __func__, vaddr);
#endif

#if LIST_MOTHED_DEBUG
	printf("\t *** find free vaddr=0x%p, sizeof(struct pmem_t)=%d ***\n", 
		vaddr, sizeof(struct pmem_t));
#endif		

	struct pmem_t *entry = NULL;
	struct list_head *ptr = NULL;
	struct list_head *pn = NULL;
	
	list_for_each_safe(ptr, pn, &(pmem.using_list)) {
		entry = list_entry(ptr, struct pmem_t, list);
		if((NULL != entry) && (entry->vaddr == vaddr)) {
#if LIST_MOTHED_DEBUG		
			printf("\t *** match free vaddr=0x%p, entry=0x%lX ***\n", 
				vaddr, (unsigned long)entry);
#endif				
			list_del_init(ptr);
			break;
		}
		entry = NULL;
	}

	if(entry == NULL) {
		LOGE ("%s can't find %p in dma memory list\n", __func__, vaddr);
		pthread_mutex_unlock(&pmem_mutex);
		return;
	}

	recovery_pmem(entry);
	adjust_free_pmem_list();
	free(entry);
	entry = NULL;
	
	pthread_mutex_unlock(&pmem_mutex);

#if PMEM_USE_INFO
	akuio_get_used_pmem();
	akuio_get_free_pmem();
#endif

#if LIST_MOTHED_DEBUG
	printf("\t --- leaving akuio_free_pmem ---\n\n");
#endif
}

/**
 * akuio_vaddr2paddr - Convert virtual address to physical address
 * @vaddr: virtual addr, according to the physically continuous memory
 * return: the physical address of physically contigous memory; 0 failed
 */
unsigned long akuio_vaddr2paddr(void *vaddr)
{
	if(NULL == vaddr) {
		LOGE ("%s input arg is NULL\n", __func__);
		return 0;
	}
	
	pthread_mutex_lock(&pmem_mutex);
	if (pmem.init <= 0) {
		pthread_mutex_unlock(&pmem_mutex);
		LOGE("%s error: akuio not init\n", __func__);
		return 0;
	}

	struct pmem_t *entry = NULL;
	struct list_head *ptr = NULL;
	struct list_head *pn = NULL;
	
	list_for_each_safe(ptr, pn, &(pmem.using_list)) {
		entry = list_entry(ptr, struct pmem_t, list);
		if((NULL != entry) 
			&& (vaddr >= entry->vaddr) 
 			&& (vaddr < (entry->vaddr+entry->size))) {
#if VA2PA_MATCH		
			printf("[%s] *** match, vaddr=0x%p, entry=0x%lX, vaddr=0x%lX, size=%ld ***\n\n", 
				__func__, vaddr, (unsigned long)entry, 
				(unsigned long)(entry->vaddr), entry->size);
#endif				
			break;
		}
		entry = NULL;
	}

	if(NULL == entry) {
		LOGE ("%s can't find %p in dma memory list\n", __func__, vaddr);
		pthread_mutex_unlock(&pmem_mutex);    
		return 0;
	}

#if TRACE_DMA
	static int x = 0;

	if (x < 30) {
		LOGD ("%s: vaddr=%p, paddr=0x%08lx\n", __func__, 
			vaddr, pmem.paddr + (vaddr - pmem.vaddr));
		x++;
	}
#endif

	pthread_mutex_unlock(&pmem_mutex);
	
	return pmem.paddr + (vaddr - pmem.vaddr);
}

/**
 * akuio_lock_timewait - Lock akuio with timeout mode
 * @hw_id: the id of hardware which will be lock
 * return: lock handle; NULL lock akuio failed
 */
void* akuio_lock_timewait(int hw_id)
{
    return NULL;
}

/**
 * akuio_unlock - Unlock akuio
 * @lock_handle: the handle returned by akuio_lock_xxx()
 * return: 0 success; -1 failed
 * notes: Unlock akuio or de-init pmem failed.
 */
int akuio_unlock(void *lock_handle)
{
    return 0;
}

/**
 * akuio_map_regs - Map the register region to application memory space
 * @paddr: the physical address of register region
 * @size: the size of register region
 * return: maped vaddr; NULL failed
 */
void* akuio_map_regs(unsigned int paddr, unsigned int size)
{
    FILE *fd;
    unsigned int uio_addr;
    unsigned int uio_offset;
    int j = 0;
    bool found = false;
    char path[1024] = {0};

#if TRACE_UIO
    LOGD ("%s : paddr=0x%08x, size=%u\n", __func__, paddr, size);
#endif

    for (j = 0; j < 256 && found == false; j++) {
        sprintf (path, "/sys/class/uio/uio0/maps/map%u/addr", j);
        fd = fopen (path, "r");
        if (!fd) {
        	break;
        }

        fscanf (fd, "0x%x", &uio_addr);
        if (uio_addr == paddr)
        	found = true;

        fclose (fd);
    }

    if (found == false) {
        LOGE ("Can't found uio mapping for 0x%08x\n", paddr);
        return NULL;
    }

    j--;

    LOGD("[akuio_map_regs]:/sys/class/uio/uio0/maps/map%d/offset\n",  j);
    sprintf (path, "/sys/class/uio/uio0/maps/map%u/offset",  j);

    fd = fopen (path, "r");
    if (fd){
        fscanf (fd, "0x%x", &uio_offset);
        fclose (fd);
    } else {
        uio_offset = 0;
    }

    struct uio_t *uio = malloc (sizeof(struct uio_t));
    if (uio == NULL) {
        LOGE ("Alloc uio struct error: %s", strerror(errno));
        return NULL;
    }

    uio->uio_index = 0;
    uio->map_index = j;
    uio->uio_fd    =  uio0_fd ;
    uio->paddr     = paddr;
    uio->vaddr     = mmap (NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED,
    	                uio->uio_fd, uio->map_index*getpagesize());
    uio->offset    = uio_offset;
    uio->size      = size;

	int io_fd = -1;
    if(uio->vaddr == MAP_FAILED) {
        if(uio->paddr>=0x08000000 && uio->paddr<=0x08000017){
            io_fd = open ("/dev/uio0", O_RDWR); /* must O_RDWR */
            if(io_fd < 0){
                LOGE ("[akuio_map_regs]:UIO0 device open failed: %s\n", strerror(errno));
                return NULL;
            }

            remmap_count++;
            LOGD("uio1 common register re-mmap!remmap_count=%d\n", remmap_count);
            uio->uio_fd = io_fd;
            uio->vaddr = mmap (NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED,
                                io_fd, uio->map_index*getpagesize());
        }
    }

	if(fcntl(uio->uio_fd, F_SETFD, FD_CLOEXEC) == -1) {
		fprintf(stderr, "[%s;%s]error:%s\n", __FILE__, __func__, strerror(errno));
	}

    if (uio->vaddr == MAP_FAILED) {
        LOGE ("mmap() failed: %s\n", strerror(errno));
        free (uio);
        return NULL;
    }

	list_add_tail(&(uio->list), &uio0_list_head);

#if TRACE_UIO
	LOGD ("%s: vaddr=%p, offset = %u\n", __func__, uio->vaddr, uio->offset);
#endif

	return (uio->vaddr + uio->offset);
}

/**
 * akuio_unmap_regs - Unmap the register region from application memory space
 * @vaddr: the pointer returned by akuio_map_regs()
 * @size: the size of register region
 * return: none
 */
void akuio_unmap_regs(void *vaddr, unsigned int size)
{
	LOGD ("Enter %s\n", __func__);
#if TRACE_UIO
	LOGD ("%s: vaddr=%p\n", __func__, vaddr);
#endif

	bool found = false;
	struct uio_t *uio = NULL;
	struct uio_t *ptr = NULL;
	
    /* Firstly search it in uio0_list_head. */
    list_for_each_entry_safe(uio, ptr, &uio0_list_head, list) {
		if (uio && (uio->vaddr + uio->offset) == vaddr) {
            found = true;
            release_uio_resource(uio, size);
            break;
        }
    }

    if (!found) {
        LOGW ("WARNING: %s can't find uio mapping for %p\n", __func__, vaddr);
        return;
    }
    LOGD ("Exit %s\n", __func__);
}

/**
 * akuio_sysreg_write - Modify the system register
 * @paddr: the physical address of system register
 * @val: the value which will be set to system register
 * @mask: the valid bit of the value
 * return: none
 */
void akuio_sysreg_write(unsigned int paddr, unsigned int val, unsigned int mask)
{
#if TRACE_BITS_WRITE
    LOGD ("%s: 0x%08x, 0x%08x, 0x%08x\n", __func__, paddr, val, mask);
#endif

    struct akuio_sysreg_write_t reg_write;

    reg_write.paddr = paddr;
    reg_write.val   = val;
    reg_write.mask  = mask;

    if(uio0_fd && (ioctl(uio0_fd, AKUIO_SYSREG_WRITE, &reg_write)>=0)){
        LOGD("uio0 akuio_sysreg_write OK.\n");
    } else {
        LOGE("===akuio_sysreg_write occur error: %s========uio0_fd:%x\n", 
        	strerror(errno), uio0_fd);
    }
}

/**
 * akuio_wait_irq -  Wait a interruption occur
 * @state:  pointer to store irq state
 * return: none
 */
void akuio_wait_irq(int *state)
{
	int ret = ioctl(uio0_fd, AKUIO_WAIT_IRQ, state);
	if (ret < 0) {
		LOGE("%s: ioctl AKUIO_WAIT_IRQ error: %s\n", __func__, strerror(errno));
	}
}

/**
 * akuio_clean_invalidate_dcache - clean invalidate DCache if needed
 * @void: 
 * return: none
 * note: Used by Sky platform, Cloud do nothing
 */
void akuio_clean_invalidate_dcache(void)
{
}

/*
 * akuio_reset_video - reset system video mode 
 * return void;
 */
void akuio_reset_video(void)
{
	int ret = ioctl(uio0_fd, AKUIO_VIDEO_RESET);
	if (ret < 0) {
		LOGE("%s: ioctl AKUIO_VIDEO_RESET error: %s\n", __func__, strerror(errno));
	}
}
