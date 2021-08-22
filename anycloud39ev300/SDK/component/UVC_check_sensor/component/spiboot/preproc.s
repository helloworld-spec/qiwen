#line 1 "mmulib.s"

@void MMU_EnableICache(void)
   .global MMU_EnableICache
MMU_EnableICache:
   mrc  p15,0,r0,c1,c0,0
   orr  r0,r0,# (1<<12) 
   mcr  p15,0,r0,c1,c0,0
    mov pc,lr 

@void MMU_DisableICache(void)
   .global MMU_DisableICache
MMU_DisableICache:       
   mrc  p15,0,r0,c1,c0,0
   bic  r0,r0,# (1<<12) 
   mcr  p15,0,r0,c1,c0,0
    mov pc,lr 

@void MMU_EnableDCache(void)
   .global MMU_EnableDCache
MMU_EnableDCache:
   mrc  p15,0,r0,c1,c0,0
   orr  r0,r0,# (1<<2) 
   mcr  p15,0,r0,c1,c0,0
    mov pc,lr 

@void MMU_DisableDCache(void)
   .global MMU_DisableDCache
MMU_DisableDCache:       
   mrc  p15,0,r0,c1,c0,0
   bic  r0,r0,# (1<<2) 
   mcr  p15,0,r0,c1,c0,0
    mov pc,lr 


@void MMU_EnableAlignFault(void)
   .global MMU_EnableAlignFault 
MMU_EnableAlignFault:
   mrc  p15,0,r0,c1,c0,0
   orr  r0,r0,# (1<<1) 
   mcr  p15,0,r0,c1,c0,0
    mov pc,lr 

@void MMU_DisableAlignFault(void)
   .global MMU_DisableAlignFault
MMU_DisableAlignFault:
   mrc  p15,0,r0,c1,c0,0
   bic  r0,r0,# (1<<1) 
   mcr  p15,0,r0,c1,c0,0
    mov pc,lr 

@void MMU_EnableMMU(void)
   .global MMU_EnableMMU
MMU_EnableMMU:
   mrc  p15,0,r0,c1,c0,0
   orr  r0,r0,# (1) 
   mcr  p15,0,r0,c1,c0,0
    mov pc,lr 

@void MMU_DisableMMU(void)
   .global MMU_DisableMMU
MMU_DisableMMU:
   mrc  p15,0,r0,c1,c0,0
   bic  r0,r0,# (1) 
   mcr  p15,0,r0,c1,c0,0
    mov pc,lr 

@=========================
@ Set TTBase
@=========================
@void MMU_SetTTBase(int base)
   .global MMU_SetTTBase
MMU_SetTTBase:
   @ro=TTBase
   mcr  p15,0,r0,c2,c0,0
    mov pc,lr 

@=========================
@ Set Domain
@=========================
@void MMU_SetDomain(int domain)
   .global MMU_SetDomain
MMU_SetDomain:
   @ro=domain
   mcr  p15,0,r0,c3,c0,0
    mov pc,lr 

@=========================
@ ICache/DCache functions
@=========================
@void MMU_InvalidateIDCache(void)
   .global MMU_InvalidateIDCache
MMU_InvalidateIDCache:
   mcr  p15,0,r0,c7,c7,0
    mov pc,lr 

@void MMU_InvalidateICache(void)
   .global MMU_InvalidateICache
MMU_InvalidateICache:
   mcr  p15,0,r0,c7,c5,0
    mov pc,lr 

@void MMU_InvalidateICacheMVA(U32 mva)
   .global MMU_InvalidateICacheMVA
MMU_InvalidateICacheMVA:
   @r0=mva
   mcr  p15,0,r0,c7,c5,1
    mov pc,lr 
        
@void MMU_PrefetchICacheMVA(U32 mva)
   .global MMU_PrefetchICacheMVA
MMU_PrefetchICacheMVA:
   @r0=mva
   mcr  p15,0,r0,c7,c13,1
    mov pc,lr 

@void MMU_InvalidateDCacheMVA(U32 mva)
   .global MMU_InvalidateDCacheMVA
MMU_InvalidateDCacheMVA:
   @r0=mva
   mcr  p15,0,r0,c7,c6,1
    mov pc,lr 

@void MMU_CleanDCacheMVA(U32 mva)
   .global MMU_CleanDCacheMVA
MMU_CleanDCacheMVA:
   @r0=mva
   mcr  p15,0,r0,c7,c10,1
    mov pc,lr 

@void MMU_CleanInvalidateDCacheMVA(U32 mva)
   .global MMU_CleanInvalidateDCacheMVA
MMU_CleanInvalidateDCacheMVA:
   @r0=mva
   mcr  p15,0,r0,c7,c14,1
    mov pc,lr 

@void MMU_CleanDCacheIndex(U32 index)
   .global MMU_CleanDCacheIndex
MMU_CleanDCacheIndex:
   @r0=index 
   mcr  p15,0,r0,c7,c10,2
    mov pc,lr 

@void MMU_CleanInvalidateDCacheIndex(U32 index) 
   .global MMU_CleanInvalidateDCacheIndex
MMU_CleanInvalidateDCacheIndex:  
   @r0=index
   mcr  p15,0,r0,c7,c14,2
    mov pc,lr 

@void MMU_WaitForInterrupt(void)
   .global MMU_WaitForInterrupt 
MMU_WaitForInterrupt:
   mcr  p15,0,r0,c7,c0,4
    mov pc,lr 

@===============
@ TLB functions
@===============
@voic MMU_InvalidateTLB(void)
   .global MMU_InvalidateTLB
MMU_InvalidateTLB:
   mcr  p15,0,r0,c8,c7,0
    mov pc,lr 

@void MMU_InvalidateITLB(void)
   .global MMU_InvalidateITLB
MMU_InvalidateITLB:      
   mcr  p15,0,r0,c8,c5,0
    mov pc,lr 

@void MMU_InvalidateITLBMVA(U32 mva)
   .global MMU_InvalidateITLBMVA
MMU_InvalidateITLBMVA:
   @ro=mva
   mcr  p15,0,r0,c8,c5,1
    mov pc,lr 

@void MMU_InvalidateDTLB(void)
   .global MMU_InvalidateDTLB
MMU_InvalidateDTLB:
   mcr p15,0,r0,c8,c6,0
    mov pc,lr 

@void MMU_InvalidateDTLBMVA(U32 mva)
   .global MMU_InvalidateDTLBMVA 
MMU_InvalidateDTLBMVA:
        @r0=mva
   mcr p15,0,r0,c8,c6,1
    mov pc,lr 

@=================
@ Cache lock down
@=================
@void MMU_SetDCacheLockdownBase(U32 base)
   .global MMU_SetDCacheLockdownBase 
MMU_SetDCacheLockdownBase:
   @r0= victim & lockdown base
   mcr  p15,0,r0,c9,c0,0
    mov pc,lr 
        
@void MMU_SetICacheLockdownBase(U32 base)
   .global MMU_SetICacheLockdownBase
MMU_SetICacheLockdownBase:
   @r0= victim & lockdown base
   mcr  p15,0,r0,c9,c0,1
    mov pc,lr 

@=================
@ TLB lock down
@=================
@void MMU_SetDTLBLockdown(U32 baseVictim)
   .global MMU_SetDTLBLockdown
MMU_SetDTLBLockdown:
   @r0= baseVictim
   mcr  p15,0,r0,c10,c0,0
    mov pc,lr 
        
@void MMU_SetITLBLockdown(U32 baseVictim)
   .global MMU_SetITLBLockdown
MMU_SetITLBLockdown:
   @r0= baseVictim
   mcr  p15,0,r0,c10,c0,1
    mov pc,lr 

@============
@ Process ID
@============
@void MMU_SetProcessId(U32 pid)
   .global MMU_SetProcessId
MMU_SetProcessId:
   @r0= pid
   mcr  p15,0,r0,c13,c0,0
    mov pc,lr 
        
        
@void MMU_Clean_All_DCache()
   .global MMU_Clean_All_DCache
MMU_Clean_All_DCache:
   mrc  p15, 0, r15, c7, c10, 3
   bne  MMU_Clean_All_DCache
    mov pc,lr 
   
@void MMU_Clean_Invalidate_Dcache()
   .global MMU_Clean_Invalidate_Dcache
MMU_Clean_Invalidate_Dcache:
   mrc  p15, 0, r15, c7, c14, 3
   bne  MMU_Clean_Invalidate_Dcache
   mov pc,lr 
   
@void MMU_DrainWriteBuffer()
  .global  MMU_DrainWriteBuffer
MMU_DrainWriteBuffer:
  mcr  p15, 0, r0, c7, c10, 4 
  mov pc,lr 

@void MMU_CleanSR()
  .global MMU_CleanSR
MMU_CleanSR:
   mrc  p15,0,r0,c1,c0,0
   bic  r0,r0,#(3<<8)
   mcr  p15,0,r0,c1,c0,0
   mov pc,lr
