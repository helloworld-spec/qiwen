/**************************************************************************
*
*      Copyright (c) 2001 by Accelerated Technology, Inc.
*
* PROPRIETARY RIGHTS of Accelerated Technology are involved in the subject
* matter of this material.  All manufacturing, reproduction, use and sales
* rights pertaining to this subject matter are governed by the license
* agreement.  The recipient of this software implicity accepts the terms
* of the license.
*
***************************************************************************
***************************************************************************
*
* FILE NAME                                       VERSION                  
*
*  sdc.c                                  Nucleus PLUS\ARM 6/7/9\ADS 1.13.22 
*
* DESCRIPTION
*
*  This file contains the Serial Driver specific functions.
*
* DATA STRUCTURES
*
*  SD_PORT *       :   An array of pointers to serial port structures.
*
* FUNCTIONS
*
*  SDC_Init_Port
*  SDC_Date_Ready
*  SDC_Put_String
*  SDC_LISR
*  SDC_Get_Char
*  SDC_Put_Char
*  SDC_Set_Baud_Rate
*
* DEPENDENCIES
*
*  nucleus.h
*  sd_defs.h
*  sd_extr.h
*   target.h
* protocol.h
*  externs.h
*      ppp.h
*
* HISTORY
*
*         NAME            DATE                    REMARKS
*      C. Sheppard     10-15-2001         Released version 1.13.22
*                                          Fixed spr 211, 249
****************************************************************************/

#include "nucleus.h"
#include "sd_defs.h"
#include "sd_extr.h"

#ifdef NU_ENABLE_PPP

#include "net\target.h"
#include "net\inc\externs.h"
#include "net\inc\tcp_errs.h"
#include "ppp\inc\ppp.h"

#endif /* NU_ENABLE_PPP */

extern NU_MEMORY_POOL   System_Memory;
extern void ERC_System_Error(int);

/* Define a small array to hold pointers to the two UART data
   structures. This is used by the LISR to find the correct
   data structure for the interrupt being handled. */
SD_PORT         *SDC_Port_List[SD_MAX_UARTS];


/************** Begin Port Specific Section ****************/
UINT8   SDC_TX_Int_Flag=0;
/*************** End Port Specific Section ****************/

/* Define prototypes for functions local to this module. */
VOID    SDC_Set_Baud_Rate(UINT32, SD_PORT *);

/***************************************************************************
* FUNCTION
*
*    SDC_Init_Port
*
* DESCRIPTION
*
*    This function intializes the COM port that will be used for PPP
*    communications.
*
*
* INPUTS
*
*    SD_PORT *     :   device initialization structure.
*
* OUTPUTS
*
*    STATUS        :   Returns NU_SUCCESS if successful initialization,
*                      else a negative value is returned.
*
****************************************************************************/
STATUS  SDC_Init_Port(SD_PORT *uart)
{

STATUS      status = NU_SUCCESS;
INT32       int_level,          /* old interrupt level */
            tInt;
CHAR        sem_name[8];
static INT  num_ports = 0;
VOID        (*old_lisr)(INT);   /* old LISR */



    if (uart->communication_mode == SERIAL_MODE)
    {

        /* Check for max allowed UARTS. */
        if (num_ports >= SD_MAX_UARTS)

           /* We have already initialized the max allowed UARTS. */
           status = NU_UART_LIST_FULL;
    }

    if (status != NU_SUCCESS)
        return (status);

    /* Check the supplied parity */
    else if ((uart->parity != SD_PARITY_NONE) &&
             (uart->parity != SD_PARITY_EVEN) &&
             (uart->parity != SD_PARITY_ODD))

        /* The supplied parity is not valid */
        status = NU_INVALID_PARITY;

    /* Check the supplied number of data bits */
    else if ((uart->data_bits != SD_DATA_BITS_6) &&
             (uart->data_bits != SD_DATA_BITS_7) &&
             (uart->data_bits != SD_DATA_BITS_8))

        /* The supplied data bits value is not valid */
        status = NU_INVALID_DATA_BITS;

    /* Check the supplied number of stop bits */
    else if ((uart->stop_bits != SD_STOP_BITS_1) &&
             (uart->stop_bits != SD_STOP_BITS_2))

        /* The supplied stop bits value is not valid */
        status = NU_INVALID_STOP_BITS;

    /* Verify the baud rate is within acceptable range */
    else if ((uart->baud_rate < 1200) || (uart->baud_rate > 115200))

        /* The baud rate is out of range */
        status = NU_INVALID_BAUD;

    /************** Begin Port Specific Section ****************/

    /* Validate the com port. */
    else if ((uart->com_port == SD_UART_A) ||
             (uart->com_port == SD_UART_B))
    {
        /* Handle UARTA */
        if (uart->com_port == SD_UART_A)
        {
            /* Set the vector inside this structure */
            uart->vector = SD_UART_A_VECTOR;

            /* Set the base address for this UART. */
            uart->base_address = SD_UART_A_BASE;
        }
        else    /* Otherwise handle UARTB. */
        {
            /* Set the vector inside this structure */
            uart->vector = SD_UART_B_VECTOR;

            /* Set the base address for this UART. */
            uart->base_address = SD_UART_B_BASE;
        }
    }
    else

    /************** End Port Specific Section **************/

        /* Not a supported port. */
        status = NU_INVALID_COM_PORT;

    if (uart->communication_mode == SERIAL_MODE)
    {
        /* Make sure the port was valid and the LISR was
           registered. Then create the semaphore used to make
           the SD_Put_String service thread safe. */
        if (status == NU_SUCCESS)
        {
            /* Allocate memory for the semaphore control block. */
           status = NU_Allocate_Memory (&System_Memory,(VOID**) &uart->sd_semaphore, sizeof(NU_SEMAPHORE),
                                        NU_NO_SUSPEND);

            for(tInt=0; tInt < sizeof(NU_SEMAPHORE); tInt++)
               SD_OUTBYTE((CHAR *)uart->sd_semaphore + tInt, 0x00);

            if (status == NU_SUCCESS)
            {

                /* Build the name. */
                sem_name[0] = 's';
                sem_name[1] = 'e';
                sem_name[2] = 'r';
                sem_name[3] = 'i';
                sem_name[4] = 'a';
                sem_name[5] = 'l';
                sem_name[6] = '_';
                sem_name[7] = (CHAR)(0x30 + num_ports);

                status = NU_Create_Semaphore (uart->sd_semaphore, sem_name,
                                              1, NU_FIFO);
            }
        }

        /* Make sure all the above was completed. Then store off this
           UART stucture and initialize the chip. */
        if (status == NU_SUCCESS)
        {
            SDC_Port_List[num_ports++] = uart;

        }
    }

    if (status == NU_SUCCESS)
    {
        /* Allocate memory for the data buffers. PPP only requires a TX
           buffer so the allocation will be a little different for PPP mode. */
        if (uart->communication_mode == SERIAL_MODE)
        {
            status = NU_Allocate_Memory (&System_Memory,(VOID**) &uart->tx_buffer,
                                        (2 * uart->sd_buffer_size),
                                        NU_NO_SUSPEND);

            /* Set the RX buffer to just past the TX buffer. */
            uart->rx_buffer = (CHAR *)(uart->tx_buffer + uart->sd_buffer_size);
        }
        else
        {
            status = NU_Allocate_Memory (&System_Memory,(VOID**) &uart->tx_buffer,
                                        uart->sd_buffer_size,
                                        NU_NO_SUSPEND);
        }

        if (status == NU_SUCCESS)
        {
            /* Setup the RX SD buffer */
            uart->rx_buffer_read = uart->rx_buffer_write = 0;

            uart->rx_buffer_status = NU_BUFFER_EMPTY;

            /* Setup the TX SD buffer */
            uart->tx_buffer_read = uart->tx_buffer_write = 0;

            uart->tx_buffer_status = NU_BUFFER_EMPTY;
        }

    }

    if (status == NU_SUCCESS)
    {
        /* Disable interrupts */
        int_level = NU_Local_Control_Interrupts(NU_DISABLE_INTERRUPTS);

        /* Initialize the UART */

        /************** Begin Port Specific Section *************/

        /* Reset the TX/RX FIFOs */
        SD_OUTDWORD(uart->base_address + FCR_OFFSET, (UINT32)FCR_FIFO_Reset);

        /* Setup baud rate */
        SDC_Set_Baud_Rate(uart->baud_rate, uart);

        /* Set the modem control register. Set DTR, RTS to output to LOW,
        and set INT output pin to normal operating mode     */
        SD_OUTDWORD (uart->base_address + MCR_OFFSET,
                    (UINT32)( MCR_DTR_Low | MCR_RTS_Low | MCR_Interrupt_En));


        /* Setup parity, data bits, and stop bits */
        SD_OUTDWORD (uart->base_address + LCR_OFFSET,
                    (UINT32)(LCR_NO_Break|uart->parity | uart->data_bits | uart->stop_bits ));

        /* Setup Fifo trigger level and enable FIFO */
        SD_OUTDWORD (uart->base_address + FCR_OFFSET,
                     (UINT32)(FCR_RCVR_Trig_level | FCR_FIFO_Enabled));

        /* Register the interrupt handler for the UART receiver */
        if (uart->com_port == SD_UART_A)
            status = NU_Register_LISR(SD_UART_A_VECTOR, SDC_LISR, &old_lisr);
        else
            status = NU_Register_LISR(SD_UART_B_VECTOR, SDC_LISR, &old_lisr);


        if (status == NU_SUCCESS)
        {

            /* Enable the RX interrupts */
            SD_OUTDWORD (uart->base_address + IER_OFFSET, (UINT32)IER_Rx_Holding_Reg );

            /* Enable the UART interrupts */
            if (uart->com_port == SD_UART_A)
                *IRQEnable |= UART_A_RXINT;
            else
                *IRQEnable |= UART_B_RXINT;

        }


        /************** End Port Specific Section *************/


        /* Initialize the error counters. */
        uart->parity_errors   =
        uart->frame_errors    =
        uart->overrun_errors  =
        uart->busy_errors     =
        uart->general_errors  = 0;

        /* Restore interrupts to previous level */
        NU_Local_Control_Interrupts(int_level);
    }

    return (status);
}
/***************************************************************************
* FUNCTION
*
*    SDC_Put_Char
*
* DESCRIPTION
*
*    This writes a character out to the serial port.
*
* INPUTS
*
*    UINT8 :   Character to to be written to the serial port.
*    SD_PORT *     :   Serial port to send the char to.
*
* OUTPUTS
*
*    none
*
****************************************************************************/
VOID  SDC_Put_Char(UINT8 ch, SD_PORT *uart)
{
    INT     intmask,i;
    UINT8   temp_byte;

    if (uart->communication_mode == SERIAL_MODE)
    {

        /* If the buffer is full wait for it to empty a little. */
        do
        {

        /* If the global interrupt flag is set, ensure the interrupt
           is turned-on.  This interrupt could be disabled due to the
           ANGEL monitor operating with IRQs */

        }while (uart->tx_buffer_status == NU_BUFFER_FULL);


        /* Disable interrupts */
        intmask = NU_Local_Control_Interrupts(NU_DISABLE_INTERRUPTS);

        /* Check the transmit buffer status. If it has data already
           just add this byte to the buffer. */
         if (( uart->tx_buffer_status == NU_BUFFER_DATA) ||
            ( uart->tx_buffer_status == NU_BUFFER_FULL) )
        {

            /* Add byte to buffer. */
            uart->tx_buffer[uart->tx_buffer_write++] = ch;

            /* Check for wrap of buffer. */
            if(uart->tx_buffer_write == uart->sd_buffer_size)
                uart->tx_buffer_write = 0;

            /* Check for full buffer. */
            if (uart->tx_buffer_write == uart->tx_buffer_read)
                uart->tx_buffer_status = NU_BUFFER_FULL;

            /* Restore interrupts to previous level */
            NU_Local_Control_Interrupts(intmask);
        }

        else
        {
            /* Otherwise send the data. */

            /* Restore interrupts to previous level */
            NU_Local_Control_Interrupts(intmask);

            /* Add byte to buffer. */
            uart->tx_buffer[uart->tx_buffer_write++] = ch;

            /* Check for wrap of buffer. */
            if(uart->tx_buffer_write == uart->sd_buffer_size)
                uart->tx_buffer_write = 0;

            /* Set status */
            uart->tx_buffer_status = NU_BUFFER_DATA;

            /**************** Begin Port Specific Section **************/

            /* Wait until the transmitter buffer is empty */
            while (!(SD_INDWORD (uart->base_address + LSR_OFFSET) & (UINT32)LSR_Tx_Hold_Empty));

            /* Transmit the character */
            SD_OUTDWORD (uart->base_address + THR_OFFSET, (UINT32)ch);

            /* Enable the TX interrupts */
            temp_byte = (UINT8)SD_INDWORD (uart->base_address + IER_OFFSET);
            temp_byte |= IER_Tx_Holding_Reg;
            SDC_TX_Int_Flag = 1;            /* Global flag used to ensure TX interrupt works
                                               when using ANGEL monitor with IRQ */
            SD_OUTDWORD (uart->base_address + IER_OFFSET, (UINT32)temp_byte);

        }

    }  /* endif mode */
    else
    {
        /* Wait until the transmitter buffer is empty */
        while (!(SD_INDWORD (uart->base_address + LSR_OFFSET) & (UINT32)LSR_Tx_Hold_Empty));

        /* Transmit the character */
          SD_OUTDWORD (uart->base_address + THR_OFFSET, (UINT32)ch);

#ifndef PPP_POLLED_TX

            /* Enable the TX interrupts */
            i = (UINT8)SD_INDWORD (uart->base_address + IER_OFFSET);
            i |= IER_Tx_Holding_Reg;
            SD_OUTDWORD (uart->base_address + IER_OFFSET, (UINT32)i);

#endif /* PPP_POLLED_TX */
    }

        /***************** End Port Specific Section ***************/

}

/***************************************************************************
* FUNCTION
*
*    SDC_RX_LISR
*
* DESCRIPTION
*
*    This is the entry function for the receive ISR that services the UART
*    in the SH4.  The SH4 has 4 interrupts per UART.
*
* INPUTS
*
*    INT         :   Interrupt vector
*
* OUTPUTS
*
*    none
*
****************************************************************************/
VOID  SDC_LISR(INT vector)
{

SD_PORT         *uart;
CHAR            receive;
UINT8           status;
UINT8           vector_found = NU_FALSE;
UINT8           ier_val;
#ifdef NU_ENABLE_PPP
DV_DEVICE_ENTRY *device;
#endif

    for(receive = 0 ; (SDC_Port_List[receive] != NU_NULL) &&
        (receive < SD_MAX_UARTS) && !vector_found ; receive++)
    {
    /* See if we found one. Better have since we got an interrupt
        from one. */
        if (SDC_Port_List[receive] -> vector == vector)
        {
            /* Point our local structure to it. */
            uart = SDC_Port_List[receive];
            vector_found = NU_TRUE;
        }
    }

#ifdef  NU_ENABLE_PPP

        /* Find the device for this interrupt */
        if ( (device = DEV_Get_Dev_For_Vector(vector)) != NU_NULL)
        {
            /* Get the address of the uart structure for this device. */
            uart = &((PPP_LAYER *) device->ppp_layer)->uart;
            vector_found = NU_TRUE;
        }

#endif /* NU_ENABLE_PPP */

    if (vector_found == NU_TRUE)
    {


        /**************** Begin Port Specific Section **************/
        status = (UINT8)SD_INDWORD (uart->base_address + ISR_OFFSET);

        /* Check for a receive interrupt. */
        if (status & ISR_Rx_Rdy)
        {

            status = (UINT8)SD_INDWORD (uart->base_address + LSR_OFFSET);

            /* Process every character in the receive FIFO */
            do
            {

                if (status & (LSR_Framing_Error | LSR_Parity_Error))
                {
                    if (status & LSR_Framing_Error)
                    {
                        uart->frame_errors++;
                    }

                    if (status & LSR_Parity_Error)
                    {
                        uart->parity_errors++;
                    }
                }

                else
                {
                    /* If an overrun has occured, log it */
                    if (status & LSR_Overrun_Error)
                    {
                        uart->overrun_errors++;
                    }

                    switch(uart->communication_mode)
                    {
                        case SERIAL_MODE:
                             /* Put char into buffer */
                             receive = (UINT8)SD_INDWORD (uart->base_address + RHR_OFFSET);

                             if (uart->rx_buffer_status != NU_BUFFER_FULL)
                             {
                                /* Put the character into the buffer */
                                uart->rx_buffer[uart->rx_buffer_write++] = receive;

                                /* Check for wrap of buffer. */
                                if(uart->rx_buffer_write == uart->sd_buffer_size)
                                    uart->rx_buffer_write = 0;

                                /* Set status field based on latest character */
                                if (uart->rx_buffer_write == uart->rx_buffer_read)
                                    uart->rx_buffer_status = NU_BUFFER_FULL;
                                else
                                    uart->rx_buffer_status = NU_BUFFER_DATA;
                             }
                             else
                                uart->busy_errors++;
                             break;

#ifdef NU_ENABLE_PPP
                        /* call PPP processing functions */

                        case MDM_NETWORK_COMMUNICATION:
                             /* Call this devices receive routine */
                             device->dev_receive(device);
                             break;

                        case MDM_TERMINAL_COMMUNICATION:
                             default:
                             MDM_Receive(device);
                             break;
#endif /* NU_ENABLE_PPP */
                    }

                }
              status = (UINT8)SD_INDWORD (uart->base_address + LSR_OFFSET);
            } while (status & LSR_Rx_Data_Ready);
        }

        status = (UINT8)SD_INDWORD (uart->base_address + IER_OFFSET);

        if (status & IER_Tx_Holding_Reg)
        {
            if (uart->communication_mode == SERIAL_MODE)
            {
                /* Bump the read pointer past the byte that was just
                   transmitted. */
                ++(uart->tx_buffer_read);

                /* Check for wrap of buffer. */
                if(uart->tx_buffer_read == uart->sd_buffer_size)
                    uart->tx_buffer_read = 0;

                /* Update the status. */
                if (uart->tx_buffer_write == uart->tx_buffer_read)
                {
                    uart->tx_buffer_status = NU_BUFFER_EMPTY;

                    /* Since it is now empty disable the TX interrupt! */
                    ier_val =  (UINT8)SD_INDWORD (uart->base_address + IER_OFFSET);
                    SDC_TX_Int_Flag = 0;
                    SD_OUTDWORD (uart->base_address + IER_OFFSET, (UINT32)ier_val & ~IER_Tx_Holding_Reg);
                 }

                else
                {
                    /* Send the next byte in the queue. */
                    SD_OUTDWORD (uart->base_address + THR_OFFSET,
                          (UINT32)uart->tx_buffer[uart->tx_buffer_read]);

                    /* Update the status. */
                    uart->tx_buffer_status = NU_BUFFER_DATA;
                }
            }
#ifdef NU_ENABLE_PPP
            else
            {
#ifndef PPP_POLLED_TX
               /* Check for a transmit interrupt. */
               /* Is there another byte in the TX buffer to send? */
               if (uart->tx_buffer_read != uart->tx_buffer_write)
               {
                   /* Send the next byte in the queue. */
                   SD_OUTDWORD (uart->base_address + THR_OFFSET,
                             (UINT32)uart->tx_buffer[uart->tx_buffer_read++]);

                   /* Check for wrap of buffer. */
                   uart->tx_buffer_read %= uart->sd_buffer_size;
               }

               else
               {

                    /* Since it is now empty disable the TX interrupt! */
                    ier_val =  (UINT8) SD_INDWORD (uart->base_address + IER_OFFSET);
                    ier_val ^= IER_Tx_Holding_Reg;
                    SD_OUTDWORD (uart->base_address + IER_OFFSET, (UINT32)ier_val);

                   /* Only activate the HISR if we are tranmitting
                      network data. */
                   if (uart->communication_mode == MDM_NETWORK_COMMUNICATION)
                   {
                       /* Add this device to the list of PPP devices that have finished
                          sending a packet. */
                       _ppp_tx_dev_ptr_queue [_ppp_tx_dev_ptr_queue_write++] = device;

                       /* Activate the HISR that will take care of processing the
                          next packet in queue, if one is ready. */
                       NU_Activate_HISR (&PPP_TX_HISR);

                       /* Check for wrap of ring buffer. */
                       _ppp_tx_dev_ptr_queue_write %= PPP_MAX_TX_QUEUE_PTRS;
                    }
                }
#endif /* PPP_POLLED_TX */
            }
#endif /* NU_ENABLE_PPP */
        }

        /**************** End Port Specific Section **************/

        /* No port is associated with the vector */
    }
    else
    {
        ERC_System_Error(NU_UNHANDLED_INTERRUPT);
    }
}


/****************************************************************************
* FUNCTION
*
*    SDC_Set_Baud_Rate
*
* DESCRIPTION
*
*    This function sets the UART buad rate.
*
* INPUTS
*
*    UINT32      :  The new baud rate.
*    SD_PORT *     :  Serial port to set the baud rate.
*
* OUTPUTS
*
*    none
*
****************************************************************************/
VOID  SDC_Set_Baud_Rate(UINT32 baud_rate, SD_PORT *uart)
{

    UINT32  baud_value;

    /**************** Begin Port Specific Section **************/
    /* First, compute the baudrate divisor. */
    baud_value = 115200L / baud_rate;

    /* Set the divisor latch bit. */
    SD_OUTDWORD (uart->base_address + LCR_OFFSET,(UINT32)LCR_Divisor_Latch);

    /* First write the LSB of the baud rate divisor. */
    SD_OUTDWORD (uart->base_address + DLL_OFFSET, (UINT8)baud_value);

    /* Now write the MSB of the baud rate divisor. */
    SD_OUTDWORD (uart->base_address + DLM_OFFSET,(UINT8)(baud_value >> 8));

    /* Now that the baud rate has been set turn off Divisor Latch
    to reference data in line control */
    SD_OUTDWORD (uart->base_address + LCR_OFFSET,
        (UINT32)((UINT8)SD_INDWORD(uart->base_address +LCR_OFFSET) & ~LCR_Divisor_Latch));

   /**************** End Port Specific Section ****************/

}
/****************************************************************************
* FUNCTION
*
*    SDC_Get_Char
*
* DESCRIPTION
*
*    This function reads the last received character from the UART.
*
* INPUTS
*
*    SD_PORT *      :   Serial port to get the char from.
*
* OUTPUTS
*
*    CHAR  :  Character read
*
****************************************************************************/
CHAR  SDC_Get_Char(SD_PORT *uart)
{

CHAR    ch = NU_NULL;

    if (uart->communication_mode == SERIAL_MODE)
    {
        if ((uart->rx_buffer_status == NU_BUFFER_FULL) ||
            (uart->rx_buffer_status == NU_BUFFER_DATA))
        {
            /* Store the character to be returned */
            ch = uart->rx_buffer[uart->rx_buffer_read++];

            /* If read pointer is at end, wrap it around */
            if (uart->rx_buffer_read == uart->sd_buffer_size)
                uart->rx_buffer_read = 0;

            /* Set the status to reflect removal of the character */
            if (uart->rx_buffer_write == uart->rx_buffer_read)
                uart->rx_buffer_status = NU_BUFFER_EMPTY;
            else
                uart->rx_buffer_status = NU_BUFFER_DATA;
        }

        return (ch);
    } /* endif mode */

#ifdef NU_ENABLE_PPP
    else if (uart->communication_mode == MDM_TERMINAL_COMMUNICATION ||
             uart->communication_mode == MDM_NETWORK_COMMUNICATION)

    /**************** Begin Port Specific Section **************/

             return ((UINT8)SD_INDWORD (uart->base_address + RHR_OFFSET));

    /**************** End Port Specific Section ****************/

#endif /* NU_ENABLE_PPP */

    /* Execution should never reach this point, this return was added
       in response to the 'implicit return' compiler warning */

    return (ch);
}

/****************************************************************************
* FUNCTION
*
*    SDC_Carrier
*
* DESCRIPTION
*
*    This function checks for a carrier.
*
* INPUTS
*
*    none
*
* OUTPUTS
*
*    STATUS    :  The status of the detection.
*
****************************************************************************/
STATUS SDC_Carrier(SD_PORT *uart)
{
    return (NU_TRUE);
}

/****************************************************************************
 Note: All functions below this point are generic and should not require
       any changes to support other UARTS.
 ****************************************************************************/

/****************************************************************************
* FUNCTION
*
*    SDC_Put_String
*
* DESCRIPTION
*
*    This writes a null-terminated string out to the serial port.
*
* INPUTS
*
*    CHAR *        :   String to be written to the serial port.
*    SD_PORT *     :   Serial port to send the string to.
*
* OUTPUTS
*
*    none
*
****************************************************************************/
VOID SDC_Put_String(CHAR *str, SD_PORT *uart)
{
    STATUS status;
    /* Grab the semaphore so that strings between threads
       do not get mixed. */
    if ((status = NU_Obtain_Semaphore(uart->sd_semaphore, NU_NO_SUSPEND)) == NU_SUCCESS)
    {

        /* Send out the string. */
        for (; *str != 0; str++)
            SDC_Put_Char(*str, uart);

        /* Allow other threads to use this service. */
        NU_Release_Semaphore (uart->sd_semaphore);
    }
    else
    {
        status = 0;
    }

}


/****************************************************************************
* FUNCTION
*
*    SDC_Data_Ready
*
* DESCRIPTION
*
*    This function checks to see if there are any characters in the
*    receive buffer.  A status value is returned indicating whether
*    characters are present in the receive buffer.
*
* INPUTS
*
*    SD_PORT *      :   Serial port to check for data.
*
* OUTPUTS
*
*    STATUS                                The status indicates the
*                                          presence of characters.
*
****************************************************************************/
STATUS SDC_Data_Ready(SD_PORT *port)
{
    /* Check the status. */
    if((port->rx_buffer_status == NU_BUFFER_FULL) ||
       (port->rx_buffer_status == NU_BUFFER_DATA))

        return (NU_TRUE);

    else

        return (NU_FALSE);
}

/****************************************************************************
* FUNCTION
*
*    SDC_Change_Communication_Mode
*
* DESCRIPTION
*
*    This function switches the serial port between terminal mode and
*    network mode.  The mode affects how incoming characters are directed.
*
* INPUTS
*
*    INT      :  The mode of operation desired.
*
* OUTPUTS
*
*    none
*
****************************************************************************/
VOID SDC_Change_Communication_Mode(INT mode, SD_PORT *uart)
{
    uart->communication_mode = mode;

} /* SDC_Change_Communication_Mode */

/****************************************************************************
* FUNCTION
*
*    SDC_Reset
*
* DESCRIPTION
*
*    This function intializes the data variables associated with a UART
*
* INPUTS
*
*    SD_PORT      * :   Serial port to reset
*
* OUTPUTS
*
*    STATUS      :   Returns URT_SUCCESS if successful initialization,
*                    else a negative value is returned.
*
****************************************************************************/
VOID SDC_Reset (SD_PORT *uart)
{
    /* Ini the error counters */
    uart->frame_errors   = 0;
    uart->overrun_errors = 0;
    uart->parity_errors  = 0;
    uart->busy_errors    = 0;
    uart->general_errors = 0;
}

/***************************************************************************
* FUNCTION
*
*    URT_Init_Port
*
* DESCRIPTION
*
*    This function intializes the data variables associated with a UART
*
* INPUTS
*
*    SD_PORT      * :   Serial port to reset
*
* OUTPUTS
*
*    STATUS      :   Returns URT_SUCCESS if successful initialization,
*                    else a negative value is returned.
*
****************************************************************************/
#ifdef NU_ENABLE_PPP
STATUS  URT_Init_Port(DV_DEVICE_ENTRY *device)
{
    SD_PORT   *uart;
    STATUS    ret_status;

    /* Get a pointer to the UART layer of this device. */
    uart = &((PPP_LAYER *) device->ppp_layer)->uart;

    /* Init the serial port, copy init parameters from the device
       structure. */
    uart->com_port              = device->dev_com_port;
    uart->baud_rate             = device->dev_baud_rate;
    uart->data_bits             = device->dev_data_bits;
    uart->stop_bits             = device->dev_stop_bits;
    uart->parity                = device->dev_parity;
    uart->data_mode             = device->dev_data_mode;
    uart->vector                = device->dev_vect;
    uart->driver_options        = device->dev_driver_options;
    uart->communication_mode    = MDM_TERMINAL_COMMUNICATION;
    uart->sd_buffer_size        = (2 * (PPP_MTU + PPP_FCS_SIZE +
                                    PPP_MAX_PROTOCOL_SIZE + PPP_MAX_ADDR_CONTROL_SIZE));

    /* Init the port */
    ret_status = NU_SD_Init_Port (uart);

    if (ret_status == NU_SUCCESS)
    {
        /* Copy the vector back into the device entry just in case
           the UART driver changed it. */
        device->dev_vect = uart->vector;
    }

    return (ret_status);

}
#endif /* NU_ENABLE_PPP */


