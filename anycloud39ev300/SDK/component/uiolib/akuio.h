#ifndef __AKUIO_H_
#define __AKUIO_H_

/**
 * akuio_pmem_init - Init pmem
 * @void: 
 * return: 0 success; -1 failed
 */
int akuio_pmem_init(void);

/**
 * akuio_pmem_fini - Free pmem resource
 * @void: 
 * return: 0 success; -1 failed
 */
int akuio_pmem_fini(void);

/**akuio_get_used_pmem
 * @void
 * @return: current used pmem in bytes
 */
unsigned long akuio_get_used_pmem(void);

/**akuio_get_free_pmem
 * @void
 * @return: current free pmem in bytes
 */
unsigned long akuio_get_free_pmem(void);

/**
 * akuio_alloc_pmem - Allocate a piece of physically continuous memory
 * @size: size of physically continuous memory region
 * return: virtual addr, according to the physically continuous memory; 
 *		NULL failed
 */
void* akuio_alloc_pmem(unsigned int size);

/**
 * akuio_free_pmem - Free the appointed physically continuous memory
 * @vaddr: virtual addr, according to the physically continuous memory
 * return: none
 */
void akuio_free_pmem(void *vaddr);

/**
 * akuio_vaddr2paddr - Convert virtual address to physical address
 * @vaddr: virtual addr, according to the physically continuous memory
 * return: the physical address of physically contigous memory; 0 failed
 */
unsigned long akuio_vaddr2paddr(void *vaddr);

/**
 * akuio_lock_timewait - Lock akuio with timeout mode
 * @hw_id: the id of hardware which will be lock
 * return: lock handle; NULL lock akuio failed
 */
void* akuio_lock_timewait(int hw_id);

/**
 * akuio_unlock - Unlock akuio
 * @lock_handle: the handle returned by akuio_lock_xxx()
 * return: 0 success; -1 failed
 * notes: Unlock akuio or de-init pmem failed.
 */
int akuio_unlock(void *lock_handle);

/**
 * akuio_map_regs - Map the register region to application memory space
 * @paddr: the physical address of register region
 * @size: the size of register region
 * return: maped vaddr; NULL failed
 */
void* akuio_map_regs(unsigned int paddr, unsigned int size);

/**
 * akuio_unmap_regs - Unmap the register region from application memory space
 * @vaddr: the pointer returned by akuio_map_regs()
 * @size: the size of register region
 * return: none
 */
void akuio_unmap_regs(void *vaddr, unsigned int size);

/**
 * akuio_sysreg_write - Modify the system register
 * @paddr: the physical address of system register
 * @val: the value which will be set to system register
 * @mask: the valid bit of the value
 * return: none
 */
void akuio_sysreg_write(unsigned int paddr, unsigned int val, unsigned int mask);

/**
 * akuio_wait_irq -  Wait a interruption occur
 * @state:  pointer to store irq state
 * return: none
 */
void akuio_wait_irq(int *state);

/**
 * akuio_clean_invalidate_dcache - clean invalidate DCache if needed
 * @void: 
 * return: none
 * note: Used by Sky platform, Cloud do nothing
 */
void akuio_clean_invalidate_dcache(void);

void akuio_reset_video(void);
#endif
