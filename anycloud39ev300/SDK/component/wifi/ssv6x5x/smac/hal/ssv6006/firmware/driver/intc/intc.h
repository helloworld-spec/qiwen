#ifndef __INTC_H__
#define __INTC_H__

#include <types.h>

void intc_init (void);
void intc_config (u32 int_index);
void intc_mask_all (void);
void intc_mask (u32 mask);
void intc_clear_all (void);
s32 intc_status (void);
s32 intc_raw_status (void);
void intc_int_mask (u32 int_no);
void intc_int_unmask (u32 int_no);
void intc_int_clear (u32 int_no);
s32  intc_int_status (u32 int_no);
s32  intc_int_raw_status (u32 int_no);

void intc_group2_mask (u32 mask);
void intc_group2_enable (u32 peri_int_no, void *irq_func);
void intc_group2_disable (u32 peri_int_no);
s32 intc_group2_status (void);

void intc_group15_mask (u32 mask);
void intc_group15_enable (u32 peri_int_no, void *irq_func);
void intc_group15_disable (u32 peri_int_no);
s32 intc_group15_status (void);

void intc_group31_mask (u32 mask);
void intc_group31_enable (u32 peri_int_no, void *irq_func);
void intc_group31_disable (u32 peri_int_no);
s32 intc_group31_status (void);

#endif // __INTC_H__
