#include "n12_def.h"
#include "cache.h"
#include "string.h"

void n12_dcache_invalidate(void){

#ifdef CONFIG_CPU_DCACHE_ENABLE

	unsigned long end;
	unsigned long cache_line = CACHE_LINE_SIZE(DCACHE);

	end = CACHE_WAY(DCACHE) * CACHE_SET(DCACHE) * CACHE_LINE_SIZE(DCACHE);

	do {
		end -= cache_line;
		__asm__ volatile ("\n\tcctl %0, L1D_IX_INVAL" ::"r" (end));
		end -= cache_line;
		__asm__ volatile ("\n\tcctl %0, L1D_IX_INVAL" ::"r" (end));
		end -= cache_line;
		__asm__ volatile ("\n\tcctl %0, L1D_IX_INVAL" ::"r" (end));
		end -= cache_line;
		__asm__ volatile ("\n\tcctl %0, L1D_IX_INVAL" ::"r" (end));
	} while (end > 0);

	MSYNC(store);
	DSB();
#endif
}

void n12_dcache_flush(void){

#ifdef CONFIG_CPU_DCACHE_ENABLE

#ifndef CONFIG_CPU_DCACHE_WRITETHROUGH
	unsigned long saved_gie;
#endif
	unsigned long end;
	unsigned long cache_line;

	cache_line = CACHE_LINE_SIZE(DCACHE);
	end = CACHE_WAY(DCACHE) * CACHE_SET(DCACHE) * cache_line;

#ifndef CONFIG_CPU_DCACHE_WRITETHROUGH
	GIE_SAVE(&saved_gie);

	do {
		end -= cache_line;
		__asm__ volatile ("\n\tcctl %0, L1D_IX_WB" ::"r" (end));
		__asm__ volatile ("\n\tcctl %0, L1D_IX_INVAL" ::"r" (end));
		end -= cache_line;
		__asm__ volatile ("\n\tcctl %0, L1D_IX_WB" ::"r" (end));
		__asm__ volatile ("\n\tcctl %0, L1D_IX_INVAL" ::"r" (end));
		end -= cache_line;
		__asm__ volatile ("\n\tcctl %0, L1D_IX_WB" ::"r" (end));
		__asm__ volatile ("\n\tcctl %0, L1D_IX_INVAL" ::"r" (end));
		end -= cache_line;
		__asm__ volatile ("\n\tcctl %0, L1D_IX_WB" ::"r" (end));
		__asm__ volatile ("\n\tcctl %0, L1D_IX_INVAL" ::"r" (end));

	} while (end > 0);
	GIE_RESTORE(saved_gie);
#else
	while (end > 0){

		end -= cache_line;
		__asm__ volatile ("\n\tcctl %0, L1D_IX_INVAL" ::"r" (end));
	}
#endif
	MSYNC(store);
	DSB();
#endif
}

void n12_icache_flush(void){

#ifdef CONFIG_CPU_ICACHE_ENABLE
	unsigned long end;
	unsigned long cache_line = CACHE_LINE_SIZE(ICACHE);

	end = CACHE_WAY(ICACHE) * CACHE_SET(ICACHE) * CACHE_LINE_SIZE(ICACHE);

	do {
		end -= cache_line;
		__asm__ volatile ("\n\tcctl %0, L1I_IX_INVAL" ::"r" (end));
		end -= cache_line;
		__asm__ volatile ("\n\tcctl %0, L1I_IX_INVAL" ::"r" (end));
		end -= cache_line;
		__asm__ volatile ("\n\tcctl %0, L1I_IX_INVAL" ::"r" (end));
		end -= cache_line;
		__asm__ volatile ("\n\tcctl %0, L1I_IX_INVAL" ::"r" (end));
	} while (end > 0);
#endif
}

#ifdef CONFIG_CHECK_RANGE_ALIGNMENT
#define chk_range_alignment(start, end, line_size) do {	\
							\
	BUG_ON((start) & ((line_size) - 1));		\
	BUG_ON((end) & ((line_size) - 1));		\
	BUG_ON((start) == (end));			\
							\
} while (0);
#else
#define chk_range_alignment(start, end, line_size)
#endif
/* ================================ D-CACHE =============================== */
/*
 * n12_dcache_clean_range(start, end)
 *
 * For the specified virtual address range, ensure that all caches contain
 * clean data, such that peripheral accesses to the physical RAM fetch
 * correct data.
 */
void n12_dcache_clean_range(unsigned long start, unsigned long end){

#ifdef CONFIG_CPU_DCACHE_ENABLE
#ifndef CONFIG_CPU_DCACHE_WRITETHROUGH

	unsigned long line_size;

	line_size = CACHE_LINE_SIZE(DCACHE);
	chk_range_alignment(start, end, line_size);

	while (end > start){

		__asm__ volatile ("\n\tcctl %0, L1D_VA_WB" ::"r" (start));
		start += line_size;
	}
	MSYNC(store);
	DSB();
#endif
#endif
}

void n12_dma_clean_range(unsigned long start, unsigned long end){

	unsigned long line_size;
	line_size = CACHE_LINE_SIZE(DCACHE);
	start = start & (~(line_size-1));
	end = (end + line_size -1) & (~(line_size-1));
	if (start == end)
		return;

	n12_dcache_clean_range(start, end);
}

/*
 * n12_dcache_invalidate_range(start, end)
 *
 * throw away all D-cached data in specified region without an obligation
 * to write them back. Note however that we must clean the D-cached entries
 * around the boundaries if the start and/or end address are not cache
 * aligned.
 */
void n12_dcache_invalidate_range(unsigned long start, unsigned long end){

#ifdef CONFIG_CPU_DCACHE_ENABLE

	unsigned long line_size;

	line_size = CACHE_LINE_SIZE(DCACHE);
	chk_range_alignment(start, end, line_size);

	while (end > start){

		__asm__ volatile ("\n\tcctl %0, L1D_VA_INVAL" ::"r" (start));
		start += line_size;
	}
#endif
}

void n12_dcache_flush_range(unsigned long start, unsigned long end){

#ifdef CONFIG_CPU_DCACHE_ENABLE
	unsigned long line_size;

	line_size = CACHE_LINE_SIZE(DCACHE);

	while (end > start){
#ifndef CONFIG_CPU_DCACHE_WRITETHROUGH
		__asm__ volatile ("\n\tcctl %0, L1D_VA_WB" ::"r" (start));
#endif
		__asm__ volatile ("\n\tcctl %0, L1D_VA_INVAL" ::"r" (start));
		start += line_size;
	}
#endif
}

void n12_dcache_writeback_range(unsigned long start, unsigned long end){

#ifdef CONFIG_CPU_DCACHE_ENABLE
#ifndef CONFIG_CPU_DCACHE_WRITETHROUGH
	unsigned long line_size;

	line_size = CACHE_LINE_SIZE(DCACHE);

	while (end > start){
		__asm__ volatile ("\n\tcctl %0, L1D_VA_WB" ::"r" (start));
		start += line_size;
	}
#endif
#endif
}
void unaligned_cache_line_move(unsigned char* src, unsigned char* dst, unsigned long len )
{
         int i;
         unsigned char* src_p = (unsigned char*)src;
         unsigned char* dst_p = (unsigned char*)dst;
         for( i = 0 ;i < len; ++i)
                 *(dst_p+i)=*(src_p+i);
}



void n12_dma_inv_range(unsigned long start, unsigned long end){
	unsigned long line_size;
	unsigned long old_start=start;
	unsigned long old_end=end;
	line_size = CACHE_LINE_SIZE(DCACHE);
	unsigned char h_buf[line_size];
	unsigned char t_buf[line_size];
	memset((void*)h_buf,0,line_size);
	memset((void*)t_buf,0,line_size);

	start = start & (~(line_size-1));
	end = (end + line_size -1) & (~(line_size-1));
	if (start == end)
		return;
	if (start != old_start)
	{
		//n12_dcache_flush_range(start, start + line_size);
		unaligned_cache_line_move((unsigned char*)start, h_buf, old_start - start);
	}
	if (end != old_end)
	{
		//n12_dcache_flush_range(end - line_size ,end);
		unaligned_cache_line_move((unsigned char*)old_end, t_buf, end - old_end);

	}
	n12_dcache_invalidate_range(start, end);

	//handle cache line unaligned problem
	if(start != old_start)
		unaligned_cache_line_move(h_buf,(unsigned char*)start, old_start - start);

	if( end != old_end )
		unaligned_cache_line_move(t_buf,(unsigned char*)old_end, end - old_end);

}


void n12_dma_flush_range(unsigned long start, unsigned long end){

	unsigned long line_size;
	line_size = CACHE_LINE_SIZE(DCACHE);
	start = start & (~(line_size-1));
	end = (end + line_size -1 ) & (~(line_size-1));
	if (start == end)
		return;

	n12_dcache_flush_range(start, end);
}

/* ================================ I-CACHE =============================== */
/*
 * n12_icache_invalidate_range(start, end)
 *
 * invalidate a range of virtual addresses from the Icache
 *
 * This is a little misleading, it is not intended to clean out
 * the i-cache but to make sure that any data written to the
 * range is made consistant. This means that when we execute code
 * in that region, everything works as we expect.
 *
 * This generally means writing back data in the Dcache and
 * write buffer and flushing the Icache over that region
 *
 * start: virtual start address
 * end: virtual end address
 */
void n12_icache_invalidate_range(unsigned long start, unsigned long end){

#ifdef CONFIG_CPU_ICACHE_ENABLE

	unsigned long line_size;

	line_size = CACHE_LINE_SIZE(ICACHE);
	//chk_range_alignment(start, end, line_size);
	start &= (~(line_size-1));
	end = ( end + line_size - 1 )&(~(line_size-1)); 
	if (end == start)
		end += line_size;

	while (end > start){

		end -= line_size;
		__asm__ volatile ("\n\tcctl %0, L1I_VA_INVAL" ::"r" (end));
	}
#endif
}
