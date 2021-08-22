@//=====================================================================
@//	FILE NAME                                                                                                                  
@////			boot.h
@////
@//// 	This file contains the target processor dependent initialization 
@////    values used in boot.s,tct.s, and tmt.s                       
@////                                                                       
@////
@//// 	Define constants used in low-level initialization.  
@////
@////============================================================================

@////============================================================================
@////	CPU Mode Bit
@////============================================================================

.equ USR_MODE		,		0x10
.equ FIQ_MODE		,		0x11
.equ IRQ_MODE		,		0x12
.equ SVC_MODE		,		0x13
.equ ABT_MODE		,		0x17
.equ UNDEF_MODE		,		0x1B
.equ SYS_MODE		,		0x1F
.equ I_Bit			,		0x80
.equ F_Bit			,		0x40

.equ LOCKOUT			,		0xC0		@//// Interrupt Lockout value
.equ LOCK_MSK		,		0xC0		@//// Interrupt Lockout mask value
.equ MODE_MASK		,		0x1F		@//// Processor Mode Mask

.equ true			,		0
.equ false			,		0
.equ BINARY_CODE		,		1
.equ INIT_CACHE      ,		1
.equ INIT_DATA_SEGMENT  ,		1
.equ JANUS_VSN		,		0
 
@///////////////////////////////////////////////////////////////////////////////
@//  		Memory Remap Control
@//////////////////////////////////////////////////////////////////////////////

.equ REMAP_Cntl_1	,		0x21
.equ REMAP_Cntl_2	,		0x30

@/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
@////  System Control & I/O Base
@//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
@//#define IO_BASE			0xc0000000		//// I/O device Base
.equ IO_BASE			,		0xc0000000      @0x20000000	

@////========== Memory configuration==============================================
.equ MEM_StartAddr		,		0x30
.equ MEM_Bank0EndingAddr	,		0x31
.equ MEM_MaMapType		,		0x36
.equ MEM_MemoryTiming	,		0x37
.equ MEM_SdramModeCtr	,		0x39
@////==========DMA configuration =================================================
.equ DMA_MemAddr			,		0x80
.equ DMA_Len			    ,		0x84
.equ DMA_Remain		    ,		0x86
@////
.equ DMA_DeviceCtr		,		0x88
@//
.equ DevAddrInc_BIT		,		0x4
.equ DevBigEndian_BIT	,		0x2
.equ Dev2Mem_Dir_BIT		,		0x1
.equ DevAddrNotInc		,		0x0
.equ DevLittleEndian		,		0x0
.equ Mem2Dev_Dir			,		0x0
@////
.equ DMA_Status			,		0x8c
@//=
.equ DmaCompleted_BIT	,		0x10
.equ DmaErr_BIT			,		0x08
.equ DmaErrIntEnable_BIT	,		0x04
.equ DmaIntEnable_BIT	,		0x02
.equ DmaRun_BIT			,		0x01
@////============= timer configuration  ===========================================
.equ TIM_Timer3Cnt		,		0xc0
.equ TIM_Timer2Cnt		,		0xc4
.equ TIM_Timer1Cnt		,		0xc8
.equ TIM_Timer0Cnt		,		0xcc
@////
.equ TIMER_Status		,		0xd0
.equ T3_Run_BIT			,		0x8
.equ T2_Run_BIT			,		0x4
.equ T1_Run_BIT			,		0x2
.equ T0_Run_BIT			,		0x1
.equ T3Status_BIT		,		0x8
.equ T2Status_BIT		,		0x4
.equ T1Status_BIT		,		0x2
.equ T0Status_BIT		,		0x1
@////
.equ TIM_Ctr				,		0xd1
@//=
.equ MCLK				,		0x00
.equ MCLK_4				,		0x40
.equ MCLK_16				,		0x80
.equ MCLK_64				,		0xC0
@////
.equ TIM_Mode			,		0xd2
@//=	
.equ T3_AutoReload_BIT	,		0x200000
.equ T2_AutoReload_BIT	,		0x400000
.equ T1_AutoReload_BIT	,		0x200000
.equ T0_AutoReload_BIT	,		0x100000
.equ T3_IntEnable_BIT	,		0x080000
.equ T2_IntEnable_BIT	,		0x040000
.equ T1_IntEnable_BIT	,		0x020000
.equ T0_IntEnable_BIT	,		0x010000

@////===== System Clock=================================================== 	                                

.equ TIME_3_1mSec	,		0x20000
.equ TIME_1_6mSec    ,		0x004E00            	@//// Value of 10ms timer
.equ TIME_9_7mSec	,		0x300000
 												@//// Value od 1sec timer
.equ TIMER_Md_IEn    ,		0x110000
.equ TIMER_Cntl		,		0x00c000		@@@ MCLK/64 = 20MHz/64 =
.equ TIMER_St_En 	,		0x000011 

@////======== interrupt configuration ============================================
.equ INT_IntRegister			,		0x0100		@0x00000014
.equ INT_IRQ_Status			,		0x0104
.equ INT_FIQ_Status			,		0x0108
.equ INT_Type				,		0x010c
.equ INT_MASK				,		0x0110

@INT_STATUS_REG				,		(IO_BASE + 0x00000014)
@STANDBY_REG					,		(IO_BASE + 0x00000034)
@IRQINT_MASK_REG				,		(IO_BASE + 0x00000018)
@FRQINT_MASK_REG				,		(IO_BASE + 0x0000001C)


.equ T3_BIT		,		0x80000000
.equ T2_BIT		,		0x40000000
.equ T1_BIT		,		0x20000000
.equ T0_BIT		,		0x10000000
.equ DMA_BIT		,		0x08000000
.equ INTR8_BIT	,		0x04000000
.equ INTR7_BIT	,		0x02000000
.equ INTR6_BIT	,		0x01000000
.equ INTR5_BIT	,		0x00800000
.equ INTR4_BIT	,		0x00400000
.equ INTR3_BIT	,		0x00200000
.equ INTR2_BIT	,		0x00100000
.equ INTR1_BIT	,		0x00080000
.equ UART_BIT	,		0x00040000
.equ INTR0_BIT	,		0x00020000
.equ GPIO15_0_BIT 	,		0x00010000
.equ GPIO31_BIT	,		0x00008000
.equ GPIO30_BIT	,		0x00004000
.equ GPIO29_BIT	,		0x00002000
.equ GPIO28_BIT	,		0x00001000
.equ GPIO27_BIT	,		0x00000800
.equ GPIO26_BIT	,		0x00000400
.equ GPIO25_BIT	,		0x00000200
.equ GPIO24_BIT	,		0x00000100
.equ GPIO23_BIT	,		0x00000080
.equ GPIO22_BIT	,		0x00000040
.equ GPIO21_BIT	,		0x00000020
.equ GPIO20_BIT	,		0x00000010
.equ GPIO19_BIT	,		0x00000008
.equ GPIO18_BIT	,		0x00000004
.equ GPIO17_BIT	,		0x00000002
.equ GPIO16_BIT	,		0x00000001
.equ GPIO31_16_BIT	,		0x00000000

@////======== Interrupt Vector define ================================


.equ IRQ_TIMER3_VECTOR     	,		16
.equ IRQ_TIMER2_VECTOR     	,		15
.equ IRQ_TIMER1_VECTOR     	,		14
.equ IRQ_TIMER0_VECTOR     	,		13
.equ IRQ_DMA_VECTOR			,		12
.equ IRQ_EXINT8_VECTOR  		,		11
.equ IRQ_EXINT7_VECTOR     	,		10
.equ IRQ_EXINT6_VECTOR   	,		9
.equ IRQ_EXINT5_VECTOR    	,		8
.equ IRQ_EXINT4_VECTOR     	,		7
.equ IRQ_EXINT3_VECTOR     	,		6
.equ IRQ_EXINT2_VECTOR      	,		5
.equ IRQ_EXINT1_VECTOR      	,		4
.equ IRQ_UART_VECTOR			,		3
.equ IRQ_EXINT0_VECTOR     	,		2
.equ IRQ_GPIOHW_VECTOR       ,		1
.equ IRQ_GPIOLW_VECTOR       ,		0
.equ IRQ_UNUSED_VECTOR		,		0


@IRQ_UNUSED_VECTOR		,		0
@IRQ_LCD_VECTOR			,		1	@//0x00000002
@IRQ_GUI_VECTOR			,		2	@//0x00000004
@IRQ_CAMERA_VECTOR		,		3	@//0x00000008
@IRQ_VIDEO_VECTOR		,		4	@//0x00000010
@IRQ_AUDIO_VECTOR		,		5	@//0x00000020
@IRQ_UART_MMC_VECTOR		,		6	@//0x00000040	//1 LSL 6
@IRQ_USBC_VECTOR			,		7	@//0x00000080
@IRQ_HOST_VECTOR			,		8	@//0x00000100	//1 LSL 8
@IRQ_GPIO_TIMER_VECTOR	,		9	@//0x00000200
@IRQ_FLASH_VECTOR		,		10	@//0x00000400
@IRQ_RAM_VECTOR			,		11	@//0x00000800
@IRQ_MMC_BIT				,		12	@//0x00001000
@IRQ_UART1_BIT			,		13	@//0x00002000
@IRQ_UART0_BIT			,		14	@//0x00004000
@IRQ_SPI_BIT				,		15	@//0x00008000
@IRQ_TIMER2_VECTOR		,		16	@//0x00010000	//1 LSL 16



.equ FIQ_TIMER3_VECTOR     	,		16
.equ FIQ_TIMER2_VECTOR     	,		15
.equ FIQ_TIMER1_VECTOR     	,		14
.equ FIQ_TIMER0_VECTOR     	,		13
.equ FIQ_DMA_VECTOR			,		12
.equ FIQ_EXINT8_VECTOR  		,		11
.equ FIQ_EXINT7_VECTOR     	,		10
.equ FIQ_EXINT6_VECTOR   	,		9
.equ FIQ_EXINT5_VECTOR    	,		8
.equ FIQ_EXINT4_VECTOR     	,		7
.equ FIQ_EXINT3_VECTOR     	,		6
.equ FIQ_EXINT2_VECTOR      	,		5
.equ FIQ_EXINT1_VECTOR      	,		4
.equ FIQ_UART_VECTOR			,		3
.equ FIQ_EXINT0_VECTOR     	,		2
.equ FIQ_GPIOHW_VECTOR       ,		1
.equ FIQ_GPIOLW_VECTOR    	,	 	0
.equ FIQ_UNUSED_VECTOR		,		0


@////=========== Uart configuration ==============================================
.equ URT_HiWord		,		0x0404
.equ URT_LoWord		,		0x0400
	
.equ URT_RBR			,		0x0400  @// r
.equ URT_THR			,		0x0400	@// W
.equ URT_DLL			,		0x0400
.equ URT_DLM			,		0x0401
.equ BAUD_RATE_9600_DLL		,		0x0C
.equ BAUD_RATE_9600_DLM		,		0x0000
.equ BAUD_RATE_19200_DLL		,		0x06
.equ BAUD_RATE_19200_DLM		,		0x0000
.equ BAUD_RATE_38400_DLL		,		0x03
.equ BAUD_RATE_38400_DLM		,		0x0000
.equ BAUD_RATE_56000_DLL		,		0x02
.equ BAUD_RATE_56000_DLM		,		0x0000
.equ BAUD_RATE_115200_DLL	 	,		0x01
.equ BAUD_RATE_115200_DLM	 	,		0x0000
.equ URT_IER			,		0x0401
.equ RX_DATA_AVAILABLE_INT_BIT	,		0x0100
.equ TX_HOLDING_REG_EMPTY_INT_BIT   	,		0x0200
.equ RX_LINE_STATUS_INT_BIT		,		0x0400
.equ MODEM_STATUS_INT_BIT		,		0x0800
.equ URT_IIR			,		0x0402
.equ RX_LINE_STATUS_INT		,		0x060000
.equ RX_DATA_AVALIABLE_INT		,		0x040000
.equ CHAR_TIMEOUT_INDICATION_INT	,		0x0C0000
.equ TX_HOLDING_REG_EMPTY_INT	,		0x020000
.equ MODEM_STATUS_INT		,		0x000000
.equ URT_FCR			,		0x0402
.equ FIFO_ENABLE_BIT			,		0x010000
.equ RX_FIFO_RESET_BIT		,		0x020000
.equ TX_FIFO_RESET_BIT		,		0x040000
.equ DMA_MODE_SELECT_BIT		,		0x080000
.equ URT_LCR			,		0x0403
.equ DATA_BITS_5_BIT			,		0x00000000
.equ DATA_BITS_6_BIT			,		0x01000000
.equ DATA_BITS_7_BIT			,		0x02000000
.equ DATA_BITS_8_BIT			,		0x03000000
.equ EVENT_PARITY_BIT		,		0x10000000
.equ STICK_PARITY_BIT		,		0x20000000
.equ SET_BREAK_BIT			,		0x40000000
.equ DIVISOR_LATCH_ACCESS_BIT	,		0x80000000
.equ URT_MCR			,		0x0404
.equ URT_LSR			,		0x0405
.equ DATA_READY_BIT			,		0x0100
.equ OVERRUN_ERR_BIT			,		0x0200
.equ PARITY_ERR_BIT			,		0x0400
.equ FRAMING_ERR_BIT			,		0x0800
.equ BREAK_INT_BIT			,		0x1000
.equ TX_HOLDING_REG_EMPTY_BIT	,		0x2000
.equ TX_EMPTY_BIT			,		0x4000
.equ RCVR_FIFO_ERR_BIT		,		0x8000
.equ URT_MSR			,		0x0406
.equ URT_SCR			,		0x0407

@////======== Gpio configuration =================================================
.equ PIO_DIR			,		0x0200
.equ PIO_ALT			,		0x0204
.equ PIO_SET			,		0x0208
.equ PIO_EGDT		,		0x020c
.equ PIO_REGD		,		0x0210
.equ PIO_FEGD		,		0x0214
.equ PIO_LVL			,		0x0218

@//============================================================================
@// Board Specific Definitions                                      
@//=============================================================================



@////============================================================================
.equ Timer_Priority 		,		2 

@////======== Memory Configuration ===============================================
.equ RAM_Base		,		0x01000000
.equ RAM_Lenght		,		0x01000000	@////16Mbyte

@////======== Stack offset =======================================================
.equ SystemStack		,		0x30000		@//0x20000
.equ AbtStack	   	,		0x14500		@//0x400
.equ UndefStack		,		0xc500		@//0x400
.equ IrqStack		,		0x4500		@//0x1000
.equ FiqStack		,		0x4400		@//0x1000
.equ UserStack		,		0x3ffc		@//0x10000
.equ TimerHISR    	,		0x1ffc		@//0x1000 		//// Define timer HISR stack size

@/*
@0x30000
@0x14500
@0x0C500
@0x04500
@0x04400
@0x03FFC
@0x03000
@*/
@SDRAM_SIZE		,		0x2000000	@// 32Mbyte

@USER_MODE_STACK		,		0x31FD0000 @//(RAM_BASE_ADDR + SDRAM_SIZE - 0x30000)	// reverse (64Kbyte - 16byte) for irq mode
@SVC_MODE_STACK		,		0x31FEBB00 @//(RAM_BASE_ADDR + SDRAM_SIZE - (0xC500+0x8000))	 
@FIQ_MODE_STACK		,		0x31FF3B00 @//(RAM_BASE_ADDR + SDRAM_SIZE - (0x4500+0x8000))
@IRQ_MODE_STACK		,		0x31FFBB00 @//(RAM_BASE_ADDR + SDRAM_SIZE - 0x4500)
@ABORT_MODE_STACK	,		0x31FFBC00 @//(RAM_BASE_ADDR + SDRAM_SIZE - 0x4400)	
@UNDEF_MODE_STACK	,		0x31FFBFFC @0x30FFC004 @//(RAM_BASE_ADDR + SDRAM_SIZE - 0x4000 - 4)	// same as user mode?
@HISR_MODE_STACK		,		0x31FFE000 @//(RAM_BASE_ADDR + SDRAM_SIZE - 0x2000)	// same as user mode?



@ANYKA_CPU_Mode_USR		,		0x10
@ANYKA_CPU_Mode_FIQ		,		0x11
@ANYKA_CPU_Mode_IRQ		,		0x12
@ANYKA_CPU_Mode_SVC		,		0x13
@ANYKA_CPU_Mode_ABT		,		0x17
@ANYKA_CPU_Mode_UNDEF	,		0x1B
@ANYKA_CPU_Mode_SYS		,		0x1F		
@ANYKA_CPU_I_Bit			,		0x80
@ANYKA_CPU_F_Bit			,		0x40

@////======== General =============================================================
.equ CR		,		0x0D
.equ LF		,		0x0A



@////==============================================================================
@////	for Memory Copy
@////============================================================================== 
.equ ROM_BASE		,		0x00000000
.equ Image_RO_Base	,		0x01000000
.equ Image_RO_Size	,		0x00400000		@//// 1Mbyte
.equ Image_RW_Base	,		0x00400000
	
