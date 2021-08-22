//#include "akuio.h"
#include "libc_mem.h"
#include "ak_common.h"

/**************** DMA memory ****************/
/**
* @BRIEF     Init pmem
* @AUTHOR    She shaohua
* @DATE      2011-11-25
* @PARAM     unsigned long pmem_size : the DMA size must same with kernel CONFIG_VIDEO_RESERVED_MEM_SIZE config
* @RETVAL     0 :  Successfully init pmem
* @RETVAL    <0 :  Init pmem failed.
*/
int akuio_pmem_init(void)
{
	return 0;
}
/**
* @BRIEF     Free pmem resource
* @AUTHOR    She shaohua
* @DATE      2011-11-25
* @PARAM
* @RETURN    int :  if successful return 0, otherwise return nagative
* @RETVAL     0 :  Successfully free pmem resource
* @RETVAL    <0 :  Free pmem resource failed.
*/
int akuio_pmem_fini( void )
{
	return 0;
}


/**
* @BRIEF     Allocate a piece of physically contiguous memory
* @AUTHOR    Jacky Lau
* @DATE      2011-03-15
* @PARAM     [in]  size: size of physically contigous memory region
* @RETURN    void* :  if successful return a pointer of the physically contigous memory,
                     otherwise return NULL
* @RETVAL    !NULL :  Allocation successful.
* @RETVAL     NULL :  Allocation failed.
*/
void *akuio_alloc_pmem(unsigned int size)
{
	return malloc(size);  	
}

/**
* @BRIEF     Free a piece of physically contiguous memory
* @AUTHOR    Jacky Lau
* @DATE      2011-03-15
* @PARAM     [in]  p :  pointer of physically contigous memory
* @RETURN    void
* @RETVAL
*/
void  akuio_free_pmem(void *p)
{
	free(p);
}

/**
* @BRIEF     Convert virtual address to physical address
* @AUTHOR    Jacky Lau
* @DATE      2011-03-15
* @PARAM     [in]  p :  pointer of physically contigous memory
* @RETURN    unsigned long :  if successful return the physical address of physically contigous memory,
                             otherwise return 0
* @RETVAL    !0 :  convertion successful.
* @RETVAL     0 :  convertion failed.
*/
unsigned long akuio_vaddr2paddr(void *p)
{
	return (unsigned long)p;
}

/**
* @BRIEF     Lock akuio with timeout mode
* @AUTHOR    She shaohua
* @DATE      2011-11-25
* @PARAM     [in]  hw_id :  the id of hardware which will be lock
* @RETURN    void * :  if successful return lock handle, otherwise return NULL
* @RETVAL    !NULL :  Successfully lock akuio
* @RETVAL     NULL :  Lock akuio failed.
*/
void *akuio_lock_timewait(int hw_id)
{
	return (void *)hw_id;
}

/**
* @BRIEF     Unlock akuio
* @AUTHOR    She shaohua
* @DATE      2011-11-25
* @PARAM     [in]  *lock_handle :  the handle returned by akuio_lock_xxx()
* @RETURN    int :  if successful return 0, otherwise return nagative
* @RETVAL     0 :  Successfully unlock akuio and free pmem resource
* @RETVAL    <0 :  Unlock akuio or de-init pmem failed.
*/
int   akuio_unlock(void *lock)
{
	return 0;
}

/**
* @BRIEF     Map the register region to application memory space
* @AUTHOR    Jacky Lau
* @DATE      2011-03-15
* @PARAM     [in]  paddr :  the physical address of register region
* @PARAM     [in]  size :  the size of register region
* @RETURN    void* :  if successful return the pointer of register region
                     otherwise return NULL
* @RETVAL    !NULL :  Mapping successful.
* @RETVAL     NULL :  Mapping failed.
*/
void *akuio_map_regs(unsigned int paddr, unsigned int size)
{
	return (void *)paddr;
}
/**
* @BRIEF     Unmap the register region from application memory space
* @AUTHOR    Jacky Lau
* @DATE      2011-03-15
* @PARAM     [in]  vaddr :  the pointer returned by akuio_map_regs()
* @PARAM     [in]  size :  the size of register region
* @RETURN    void
* @RETVAL
*/
void  akuio_unmap_regs(void * vaddr, unsigned int size)
{
	return ;
}
/**
* @BRIEF     Modify the system register
* @AUTHOR    Jacky Lau
* @DATE      2011-03-15
* @PARAM     [in]  paddr :  the physical address of system register
* @PARAM     [in]  val :  the value which will be set to system register
* @PARAM     [in]  mask :  the valid bit of the value
* @RETURN    void
* @RETVAL
*/
#define REG32(_register_)                               (*(volatile unsigned long *)(_register_))
void  akuio_sysreg_write(unsigned int paddr, unsigned int val, unsigned int mask)
{
	unsigned long tmp1,tmp2;

    
	//mask为1的位表示要置值， val是对应位需要置的值
	tmp1 = REG32(paddr) | mask;
	tmp2 = (val | ~mask)  ;
	REG32(paddr) = tmp1 & tmp2; 
	
}
/**
* @BRIEF     Wait a interruption occur
* @AUTHOR    Jacky Lau
* @DATE      2011-03-15
* @PARAM
* @RETURN    void
* @RETVAL
*/
void  akuio_wait_irq()
{
	if (false == codec_wait_finish())
	{
		ak_print_error_ex("timeout!!\n");
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
    MMU_InvalidateDCache();
    
}



