#include "anyka_cpu.h"

    IMPORT  irq_dispatch_handler
    IMPORT  fiq_dispatch_handler
    IMPORT  system_error_check

#ifdef AKOS
    IMPORT TCT_Interrupt_Context_Save
    IMPORT TCT_Interrupt_Context_Restore
    IMPORT TCD_Interrupt_Count
#endif

    IMPORT  pc_lr
    
    AREA INTR, CODE, READONLY

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;   Entry of IRQ   ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    EXPORT irq_handler
irq_handler
#ifdef AKOS 
    stmdb   sp!,{r0     - r3}     ;should be executed in NuCleus
    sub r3, lr,#4           ;should be executed in NuCleus
    ldr     r2, =pc_lr
    str     r3, [r2]
    bl      TCT_Interrupt_Context_Save
#else
    stmdb   sp!, {r0-r12, lr}   ;backup the context
#endif

    ;load the status register of interrupt
    ldr     r0, =INT_STATUS_REG
    ldr     r4, [r0]
    ldr     r0, =IRQINT_MASK_REG
    ldr     r5, [r0]

    and    r4, r4, r5

    mov     r0, #0
irq_handler_check_status    ;loop to check all bits of the status register
    mov     r1, #1
    mov     r1, r1, lsl r0
    tst     r4, r1
    bicne   r4, r4, r1
    stmdb   sp!, {r0-r6}
    blne    irq_dispatch_handler
    ldmia   sp!, {r0-r6}
    cmp     r4, #0
    beq     irq_handler_exit
    add     r0, r0, #1
    ;cmp    r0, #INT_STATUS_NBITS       ;check valid status bits

    b       irq_handler_check_status

irq_handler_exit            ;Exit point of IRQ
#ifdef AKOS
    b      TCT_Interrupt_Context_Restore
#else
    ldmia   sp!, {r0-r12, lr}       ;restore the context
    subs    pc, lr, #4              ;return to the interrupt point
#endif
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;   Entry of FIQ   ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    EXPORT fiq_handler
fiq_handler
    stmdb    sp!, {r0-r7, lr}    ;backup the context

    ;load the status register of interrupt
    ldr     r0, =INT_STATUS_REG
    ldr     r9, [r0]
    ldr     r0, =FRQINT_MASK_REG
    ldr     r8, [r0]

    and     r9, r9, r8
    mov     r0, #0
fiq_handler_check_status        ;loop to check all bits of the status register
    mov     r1, #1
    mov     r1, r1, lsl r0
    tst     r9, r1
    bicne   r9, r9, r1
    stmdb   sp!, {r0}
    blne    fiq_dispatch_handler
    ldmia   sp!, {r0}
    cmp     r9, #0
    beq     fiq_handler_exit
    add     r0, r0, #1
    cmp     r0, #INT_STATUS_NBITS   ;check valid status bits

    bne     fiq_handler_check_status

fiq_handler_exit            ;Exit point of FIQ
    ldmia   sp!, {r0-r7, lr}    ;restore the context
    subs    pc, lr, #4          ;return to the interrupt point


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;   Entry of Prefech handler  ;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;@@@@@@@@@@@@@@@@@@@@@   Entry of Undefined handler   @@@@@@@@@@@@@@@@@@@@@@@@@
    EXPORT undefined_handler
undefined_handler
    ;@when undefined error is accured, CPU will arrive here 
    ;@save r0-r12,LR, SPSR,CPSR into the current stack ,and stack of undefined module
    stmdb   sp!, {r0-r12, lr}                           ;@backup the context
    mrs     r0, spsr
    ldr     r1,  =UNDEF_ERROR
    stmdb   sp!, {r0-r1}

    mrs     r0, spsr
    mrs     r1, cpsr
    orr     r0, r0, #0xc0   ;disable I,F bit
    bic     r0, r0, #0x20   ;clear T bit
    msr     cpsr_cxsf, r0
    mov     r2, sp
    msr     cpsr_cxsf, r1

    stmdb   sp!, {r2}       ;save SVC sp to r2

    ;@call c function handle with previous scene
    mov     r0, sp
    ldr     pc, =system_error_check
        
    ldmia   sp!, {r0-r12, lr}                               ;@restore the context
    subs    pc, lr, #4                                      ;       @return to the interrupt point

;@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
;@@@@@@@@@@@@@@@@@@@@@   Entry of Prefech handler  @@@@@@@@@@@@@@@@@@@@@@@@@@@@
    EXPORT prefetch_handler
prefetch_handler
    stmdb   sp!, {r0-r12, lr}                               ;@backup the context
    mrs     r0,  spsr
    ldr     r1,  =PREF_ERROR
    stmdb   sp!, {r0-r1}

    mrs     r0, spsr
    mrs     r1, cpsr
    orr     r0, r0, #0xc0   ;disable I,F bit
    bic     r0, r0, #0x20   ;clear T bit
    msr     cpsr_cxsf, r0
    mov     r2, sp
    msr     cpsr_cxsf, r1

    stmdb   sp!, {r2}       ;save SVC sp to r2

    ;@call c function handle with previous scene
    mov     r0, sp
    ldr     pc, =system_error_check
    
    ldmia   sp!, {r0-r12, lr}                               ;@restore the context
    subs    pc, lr, #4                                      ;       @return to the interrupt point

        
;@@@@@@@@@@@@@@@@@@@@@   Entry of Data abort handler  @@@@@@@@@@@@@@@@@@@@@@@@@
    EXPORT abort_handler
abort_handler
    stmdb   sp!, {r0-r12, lr}       ;@backup the context 14 words
    mrs     r0,  spsr
    ldr     r1,  =ABT_ERROR
    stmdb   sp!, {r0-r1}        ;2words

    mrs     r0, spsr
    mrs     r1, cpsr
    orr     r0, r0, #0xc0   ;disable I,F bit
    bic     r0, r0, #0x20   ;clear T bit
    msr     cpsr_cxsf, r0
    mov     r2, sp
    msr     cpsr_cxsf, r1

    stmdb   sp!, {r2}       ;save SVC sp to r2

    ;@call c function handle with previous scene
    mov     r0, sp
    ldr     pc, =system_error_check

    ldmia   sp!, {r0-r12, lr}                  ;@restore the context
    subs    pc, lr, #8                         ;@return to the interrupt point

;@@@@@@@@@@@@@@@@@@@@@   Entry of irq_mask  @@@@@@@@@@@@@@@@@@@@@@@@@
;@ for AKOS version, check TCD_Interrupt_Count to decide under interrupt mode or not

    EXPORT irq_mask
irq_mask
    stmdb   sp!, {r0, r1}
    
#ifdef AKOS
    ldr     r0, =TCD_Interrupt_Count
    ldr     r1, [r0, #0]                      ; Pickup interrupt counter
    mrs     r0, cpsr
    cmp     r1, #0                             ; under SVC mode ?
    orreq   r0, r0, #0x0080
    msr     cpsr_cxsf, r0
#else
    mrs     r0, cpsr
    mov     r1, r0
    and     r1, r1, #0x1f
    cmp     r1, #0x12
    orrne   r0, r0, #0x0080
    msr     cpsr_cxsf, r0
#endif

    ldmia   sp!, {r0, r1}
    mov     pc, lr

;@@@@@@@@@@@@@@@@@@@@@   Entry of irq_unmask @@@@@@@@@@@@@@@@@@@@@@@@@
    EXPORT irq_unmask
irq_unmask
    stmdb   sp!, {r0, r1}

#ifdef AKOS
    ldr     r0, =TCD_Interrupt_Count
    ldr     r1, [r0, #0]                      ; Pickup interrupt counter
    mrs     r0, cpsr
    cmp     r1, #0                             ; under SVC mode ?
    biceq   r0, r0, #0x0080
    msr     cpsr_cxsf, r0
#else
    mrs     r0, cpsr
    mov     r1, r0
    and     r1, r1, #0x1f
    cmp     r1, #0x12
    bicne   r0, r0, #0x0080
    msr     cpsr_cxsf, r0
#endif
    
    ldmia   sp!, {r0, r1}
    mov     pc, lr
    
    EXPORT cpu_halt
cpu_halt
    stmdb   sp!, {r0-r4, lr}                ;@backup the context
    mov     r0, #0
    mcr     p15, 0, r0, c7, c10, 4          ;@ Drain write buffer
    mcr     p15, 0, r0, c7, c0, 4           ;@ Wait for interrupt
    ldmia   sp!, {r0-r4, lr}                ;@restore the context
    mov     pc, lr


;@@@@@@@@@@@@@@@@@@@@@   Entry of intr_mask_irq_fiq @@@@@@@@@@@@@@@@@@@@@@@@@
    EXPORT intr_mask_irq_fiq
intr_mask_irq_fiq
    stmdb   sp!, {r0,r1}
    
    ldr     r0, =IRQINT_MASK_REG        
    ldr     r1, =0x0
    str     r1, [r0]
    
    ;@disable all fiq interrupt
    ldr     r0, =FRQINT_MASK_REG
    ldr     r1, =0x0
    str     r1, [r0]
    
    ldmia   sp!, {r0,r1}
    mov     pc, lr      

    EXPORT jumpto_L2
;@void jumpto_L2(unsigned long param, unsigned long addr)    
jumpto_L2
    mov pc, r1

    END
