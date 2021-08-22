/*FIXME:use for test*/
test_str:
	.ascii "test aimer uboot"

.globl test_strprint
test_strprint:
	mov	r11, lr

    mov	r1, #0x13
    msr	cpsr, r1
    ldr	r2,=0x80ff0000
    mov sp, r2

	bl test_init_serial
	adr r0, test_str
	bl test_print

	/* back to arch calling code */
	mov	lr, r11
	mov pc, lr

.globl test_led
test_led:
	mov	r11, lr

	cmp r0, #0
	bne led2

led1:
	/*light on gpio 30*/
	ldr r2, =0x20170000
	mov r1, #(1<<30)
	str r1, [r2]

	ldr r2, =0x20170008
	mov r1, #(1<<30)
	str r1, [r2]
	b led_out

led2: 
	/*light on gpio 57*/
	ldr r2, =0x20170004
	mov r1, #(1<<25)
	str r1, [r2]

	ldr r2, =0x2017000c
	mov r1, #(1<<25)
	str r1, [r2]

led_out:
	/* back to arch calling code */
	mov	lr, r11
	mov pc, lr


