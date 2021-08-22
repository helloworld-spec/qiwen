@On-chip ROM size if 8KB, On-chip RAM is 16KB.

@internal RAM allocate:
@	bios: 0x40003000~0x40003fff
@	user: 0x40000000~0x40003ffc


@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
.global _start
_start:
	 b 		ResetHandler
password:
    .ascii   "ANYKA_H2"
		
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		
flash_information:
	.word	0x1			@flash_type
	.word	0x5000		@readdata_len
	.word   0x4         @clock_divider
	.word	0x3			@addr_cyc
	.word	0x0         @spi_buswidth
	.word	0x3         @spi_qe_time	
	.word	0x6         @use_outsideram
	
	.word	0x30000200      @copy_address
	
	.word	0x2002d004      @reg_addr2
	.word	0x155889cb      @set_value2
	
	.word	0x2002d000      @reg_addr3
	.word	0xe0170000      @set_value3
	
	.word	0x2002d000      @reg_addr4
	.word	0xe0120400      @set_value4
	
	.word	0x2002d000      @reg_addr5
	.word	0xe0110000      @set_value5
	
	.word	0x2002d000      @reg_addr6
	.word	0xe0110000      @set_value6
	
	.word	0x2002d000      @reg_addr7
	.word	0xe0110000      @set_value7
	
	.word	0x2002d000      @reg_addr8
	.word	0xe0110000      @set_value8     
	
	.word	0x2002d000      @reg_addr9
	.word	0xe0110000      @set_value9   
	
	.word	0x2002d000      @reg_addr10
	.word	0xe0110000      @set_value10  
	
	.word	0x2002d000  	@reg_addr11
	.word	0xe0110000      @set_value11    
	
	.word	0x2002d000      @reg_addr12
	.word	0xe0110000      @set_value12  
	
	.word	0x2002d000      @reg_addr13
	.word	0xe0100033      @set_value13  
	
	.word	0x2002d000  	@reg_addr14
	.word	0xe0170000      @set_value14    
	
	.word	0x66668888      @reg_addr15
	.word	0x0000000a      @set_value15  
	
	.word	0x88888888  	@reg_addr16
	.word	0x00000000      @set_value16    
	
	
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@	

    .skip   348

ResetHandler:   
    b ResetStart

.global spiflash_param
    .ascii      "SPIP"
spiflash_param:
    .word       0x1740c8        @id
    .word       0x00080000      @total size
    .word       256             @page size
    .word       256             @program size
    .word       0x10000         @erase size
    .word       10000000        @clock
    .byte       0x1             @flag
    .byte       0x0             @protect mask
    .byte       0x0             @resv1
    .byte       0x0             @resv2
    .skip   32                  @name
    
ResetStart:
@********************************************************************************
  @enable the clock: 
@*************************************************************************************
    ldr	    r1,= 0xFEFFFD7F  // 0xFEFFFD7F
    ldr     r0,=0x0800001c   
    str     r1,[r0]

@*************************************************************************************

 @release the reset: 
@*************************************************************************************
    ldr	    r1,=0xFFFD5F   // 0xFEFFFF7F   0xFEFFFD7F   0xFEFFFD5F
    ldr     r0,=0x08000020    
    str     r1,[r0]	    

@disable  interrupt
	ldr     r1,=0x0
	ldr     r0,=0x08000024
	str     r1,[r0]
	
	ldr     r0, =0x08000028
	str     r1,[r0]
	
	ldr		r0, =0x0800002c
	str     r1,[r0]


    ldr r0, =_bss_start
    ldr r1, =_end
    mov r3, #0

clean_loop:
    str r3, [r0], #4
    cmp r0, r1	
    bne clean_loop	
	

    @inital stack
    @@change to User Mode
    mov	r0, #0x13
    msr	cpsr, R0
    ldr	r1,=0x80ff0000  @
    MOV	SP, R1
    @bl to c function
    ldr pc, =CMain

ResetHandler_stop:
    b   ResetHandler_stop

