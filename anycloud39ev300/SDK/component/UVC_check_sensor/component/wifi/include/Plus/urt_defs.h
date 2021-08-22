/*************************************************************************/
/*                                                                       */
/*        Copyright (c) 1999-2000  Accelerated Technology, Inc.          */
/*                                                                       */
/* PROPRIETARY RIGHTS of Accelerated Technology are involved in the      */
/* subject matter of this material.  All manufacturing, reproduction,    */
/* use, and sales rights pertaining to this subject matter are governed  */
/* by the license agreement.  The recipient of this software implicitly  */
/* accepts the terms of the license.                                     */
/*                                                                       */
/*************************************************************************/

/*************************************************************************/
/*                                                                       */
/* FILE NAME                                            VERSION          */
/*                                                                       */
/*      urt_defs.h                                PLUS/SNDS100 1.11.3    */ 
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      UART                                                             */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains constant definitions and function macros      */
/*      for the UART module.                                             */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*    M. Kyle Craig , Accelerated Technology, Inc.                       */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*      none                                                             */
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/*      none                                                             */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*      NAME            DATE                    REMARKS                  */
/*                                                                       */
/*    Bobby Iden     12-21-99   Created initial version 1.11.1 for the   */
/*                              the Samsung SNDS100 with the KS32C50100. */ 
/*    D. Phillips   01-18-2000  Updated port to new structuring          */
/*                               scheme                                  */
/*    D. Phillips   03-26-2000  Released version 1.11.3                  */
/*    							                 */
/*************************************************************************/

#ifndef URT_DEFS
#define URT_DEFS


#define UART1	1
#define UART2	2
 
#define UART1_BASE  0x03FFD000
#define UART2_BASE  0x03FFE000

/* UART error codes */

#define UART_INVALID_PARITY     -1
#define UART_INVALID_DATA_BITS  -2
#define UART_INVALID_STOP_BITS  -3
#define UART_INVALID_BAUD       -4
#define UART_INVALID_COM_PORT   -5

typedef struct UART_INIT_STRUCT
{
    unsigned  int  com_port;
    unsigned  int  word_length;
    unsigned  int  stop_bits;
    unsigned  int  parity;
    unsigned  int  set_break;
    unsigned  int  baud_rate;

} UART_INIT;

#define     BUFFER_SIZE        100
#define     UART_BUFFER_FULL    -1
#define     UART_BUFFER_DATA    -2
#define     UART_BUFFER_EMPTY   -3

typedef struct BUFFER_STRUCT
{
    char    *head;
    char    *tail;
    char    *read;
    char    *write;
    INT     status;
    char    buffer[BUFFER_SIZE];
} BUFFER;


/* Layout of the KS32C50100 UART Registers from the Reg Base @ 0x03FF0000
**
** + 0xD000 - UART 0 Line Control              (ULCON0)
** + 0xD004 - Channel 0 Control reg            (UCON0)
** + 0xD008 - Channel 0 Status reg             (USTAT0)
** + 0xD00C - Channel 0 Tx Holding reg         (UTXBUF0)
** + 0xD010 - Channel 0 Rx Receive reg         (URXBUF0)
** + 0xD014 - Channel 0 Baud Rate Divisor reg  (UBRDIV0)
**
** + 0xE000 - UART 0 Line Control              (ULCON0)
** + 0xE004 - Channel 0 Control reg            (UCON0)
** + 0xE008 - Channel 0 Status reg             (USTAT0)
** + 0xE00C - Channel 0 Tx Holding reg         (UTXBUF0)
** + 0xE010 - Channel 0 Rx Receive reg         (URXBUF0)
** + 0xE014 - Channel 0 Baud Rate Divisor reg  (UBRDIV0)
**
************************************************************************/

#define ULCON0                  0x03FFD000
#define ULCON1                  0x03FFE000
/* UART Line Control Defines */
#define Data_Length_Mask        0x00000003  /* Bits 1-0 Control Word size */
#define Data_Length_8           0x00000003  /* 1 1 = 8  */
#define Data_Length_7           0x00000002  /* 1 0 = 7  */
#define Data_Length_6           0x00000001  /* 0 1 = 6  */
#define Data_Length_5           0x00000000  /* 0 0 = 5  */

#define Stop_Bits_Mask          0x00000002  /* Bit 2 is Number of Stop Bits */
#define One_Stop_Bit            0x00000000  /* 0 = 1    */
#define Two_Stop_Bits           0x00000002  /* 1 = 2    */      

#define Parity_Mask 		    0x00000038  /* Bits 5 4 3 Control Parity    */
#define Parity_None		    	0x00000000  /* 0 0 0 = No Parity            */
#define Parity_Odd	    	    0x00000020  /* 1 0 0 = Odd                  */
#define Parity_Even 		    0x00000028  /* 1 0 1 = Even                 */

/* We will use the External UART Clock (UCLK = 29.4912 MHz) */ 
#define Serial_Clock    	    0x00000040  /* Bit Set = External (UCLK)    */
#define Infra_Red_Mode    	    0x00000080  /* Bit Set = Infrared Mode On   */


#define UCON0                   0x03FFD004
#define UCON1                   0x03FFe004
/* UART Control Defines */
#define ENABLE_TX_RX		    0x00000009  /* This is the "normal" mode    */
#define Status_Irq			    0x00000004  /* Bit 2 = 1 ENAB Rx Status IRQ */
#define Assert_DSR			    0x00000020  /* Bit 5 = 1 DSR Asserted       */
#define Send_Break			    0x00000040  /* Bit 6 = 1 To send Break      */
#define No_Break			    0x00000000  /* Bit 6 = 0 Do NOT send Break  */
#define Loop_Back			    0x00000080  /* Bit 7 = 1 For Loopback Tests */


#define USTAT0                  0x03FFD008
#define USTAT1                  0x03FFE008
/* UART Status Defines */
#define RX_ERROR                0x00000007  /* OVERRUN / PARITY / FRAMING   */ 
#define Uart_Overrun_Err	    0x00000001
#define Uart_Parity_Err		    0x00000002
#define Uart_Frame_Err		    0x00000004
#define Uart_Break_Detected	    0x00000008
#define Uart_DTR_Detect 	    0x00000010  /* Bit 4 = 0 = DTR High; 1 = Low */
#define Uart_Rx_Data_Ready	    0x00000020  /* Bit 5 = 1 = Rx Data is ready  */
#define Uart_Tx_Data_Ready	    0x00000040  /* Bit 6 = 1 = Tx Buffer Empty   */
#define Uart_Tx_Complete	    0x00000080  /* Bit 7 = 1 = Tx Complete       */
                                            /* Bit 7 = 0 = Tx In Progress    */

#define UTXBUF0                 0x03FFD00C  /* UART 0 Tx Buffer  */
#define URXBUF0                 0x03FFD010  /* UART 0 Rx Buffer  */
#define UTXBUF1                 0x03FFE00C  /* UART 1 Tx Buffer  */
#define URXBUF1                 0x03FFE010  /* UART 1 Rx Buffer  */
               


/* UART BAUD Rate Divisor Regs */
#define UBRDIV0				    0x03FFD014
#define UBRDIV1				    0x03FFE014

/* Values for the SNDS100's 29.4912 Mhz (UCLK)
** NOTE: Bits 3-->0 are Divide by 16 or 1 Control --  Use 0 here for 
**       Divide by 1!
**************************************************************************/  
#define Baud_Rate_Mask  0x0000FFF0 /* Bits 15 -> 4 are Baud rate Divisor */
#define Baud9600        0x00000BF0
#define Baud19200       0x000005F0
#define Baud38400       0x000002F0
#define Baud57600       0x000001F0
#define Baud115200      0x000000F0
#define Baud230400      0x00000070
#define Baud460800      0x00000030


/* Interrupt Controller Registers - All have 1 bit for each of the 21 IRQs  */
#define INTMOD          0x03FF4000  /* Set Mode to IRQ (0) or FIQ (1)       */ 
#define INTPND          0x03FF4004  /* IRQ Pending = 1 -- Write 1 to reset! */ 
/* NOTE: The IRQ Mask Reg Bits are set to 0 to Enable IRQ!  Bit 21 is a
**       "Master" Control for ALL IRQs and must be set to 0 for ANY IRQs to 
**        be processed!
*****************************************************************************/
#define INTMSK          0x03FF4008  

/* These registers allow for customizing the Interrupt structure
** for special applications and for Testing the IRQ functions!    
** Refer to the Samsung KS32C50100 RISC Microcontroller User's Manual for
** information on these registers.  */
#define INTPRI0     0x400C
#define INTPRI1		0x4010
#define INTPRI2		0x4014
#define INTPRI3		0x4018
#define INTPRI4		0x401C
#define INTPRI5		0x4020
#define INTOFFSET	0x4024
#define INTPNDPRI	0x4028
#define INTPNDTST	0x402C

/* IRQ Bit Assignments for the KS32C50100 Controller */
#define	UART0_TX_INT		0x000010
#define	UART0_RX_INT	    0x000020  /* This is also for the Rx Error IRQ */
#define	UART1_TX_INT		0x000040
#define	UART1_RX_INT    	0x000080  /* This is also for the Rx Error IRQ */
#define	GDMA0_INT   		0x000100
#define	GDMA1_INT	    	0x000200
#define	TIMER0_INT		    0x000400
#define	TIMER1_INT		    0x000800
#define	HDLCTxA_INT	    	0x001000
#define	HDLCRxA_INT		    0x002000
#define	HDLCTxB_INT 		0x004000
#define	HDLCRxB_INT	    	0x008000
#define	MASTER_INT	    	0x200000  /* Master IRQ Mask bit */

#endif /* EOF for urt_defs.h */