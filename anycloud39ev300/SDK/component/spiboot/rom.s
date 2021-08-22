@On-chip ROM size if 8KB, On-chip RAM is 16KB.

@internal RAM allocate:
@	bios: 0x40003000~0x40003fff
@	user: 0x40000000~0x40003ffc


@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
.global _start
_start:
	 b 	ResetHandler
@password:
	.ascii   "ANYKA_H2"
	.word	0x0		@download size
	.word	0x02		@spi devide
	.word	0x4		@resv
	.word	0x0		@single read mode
	.word	0x6		@use outsideram
#ifdef CONFIG_SPI_NAND_FLASH	
	.word 0x80f00000	@boot start address
	.word	0x0		@use cpu mode

	.skip	356
@	.ascii	"SPIP"
	.skip	4
	.skip	112
	.skip	1536

#else
	.word	0x80e00000	@boot start address
	.word	0x0		@use cpu mode

	.skip	356
	.ascii	"SPIP"
	.skip	112
#endif

ResetHandler:   
@********************************************************************************
  @enable the clock: 
@*************************************************************************************
    ldr		r1,=0x0
    ldr     r0,=0x0800001c
    str     r1,[r0]
@*************************************************************************************

 @release the reset: 
@*************************************************************************************
    ldr		r1,=0x0
    ldr     r0,=0x08000020
    str     r1,[r0]	

    @inital stack
    @@change to User Mode
    mov	r0, #0x13
    msr	cpsr, R0
    ldr	r1,=0x80ff0000
    MOV	SP, R1
    
    @bl to c function
    @ldr pc, =boot_main
	bl boot_main

ResetHandler_stop:
    b   ResetHandler_stop

