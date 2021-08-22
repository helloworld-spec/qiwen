/*
 * Broadcom Proprietary and Confidential. Copyright 2016 Broadcom
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */

#include "network/wwd_buffer_interface.h"
#include "network/wwd_network_constants.h"
#include "platform/wwd_bus_interface.h"
#include "lwip/netbuf.h"
#include "lwip/pbuf.h"
#include "lwip/memp.h"
#include <string.h> /* for NULL */
#include "wwd_assert.h"
#include "RTOS/wwd_rtos_interface.h"

#include "wiced_utilities.h"


//wmj+ 
#define MEMP_PBUF_POOL_TX   0
#define MEMP_PBUF_POOL_RX   1
#define MEMP_PBUF           2




void memp_free_notify( unsigned int type );

wwd_result_t wwd_buffer_init( /*@null@*/ /*@unused@*/ void * native_arg )
{
    UNUSED_PARAMETER( native_arg );

    return WWD_SUCCESS;
}


wwd_result_t host_buffer_check_leaked( void )
{
    //wmj-
#if 0
    wiced_assert( "pbuf TX pool Buffer leakage", memp_in_use( MEMP_PBUF_POOL_TX ) == 0 );
    wiced_assert( "pbuf RX pool Buffer leakage", memp_in_use( MEMP_PBUF_POOL_RX ) == 0 );
    wiced_assert( "pbuf ref/rom Buffer leakage", memp_in_use( MEMP_PBUF ) == 0 );
#endif
	return WWD_SUCCESS;
}

wwd_result_t internal_host_buffer_get( wiced_buffer_t * buffer, wwd_buffer_dir_t direction, unsigned short size, unsigned long timeout_ms )
{
    UNUSED_PARAMETER( direction );

    wiced_assert("Error: Invalid buffer size\n", size != 0);

    *buffer = NULL;

    if ( size > (unsigned short) WICED_LINK_MTU )
    {
        WPRINT_NETWORK_DEBUG(("Attempt to allocate a buffer larger than the MTU of the link\n"));
        /*@-compdef@*/ /* Lint: buffer is not allocated in error case */
        return WWD_BUFFER_UNAVAILABLE_PERMANENT;
        /*@+compdef@*/
    }

    do
    {
        *buffer = pbuf_alloc( PBUF_RAW, size, ( direction == WWD_NETWORK_RX ) ? PBUF_POOL_RX : PBUF_POOL_TX );
        if ( *buffer != NULL )
        {
            break;
        }
        host_rtos_delay_milliseconds( 1 );

    } while ( timeout_ms-- > 0);

    if ( *buffer == NULL )
    {
#if 0
        WPRINT_NETWORK_DEBUG(("Failed to allocate packet buffer\n"));
#endif /* if 0 */
        /*@-compdef@*/ /* Lint: buffer is not allocated in this case */
        return WWD_BUFFER_UNAVAILABLE_TEMPORARY;
        /*@+compdef@*/
    }

    /*@-compdef@*/ /* Lint does not realise allocation has occurred */
    return WWD_SUCCESS;
    /*@+compdef@*/
}

wwd_result_t host_buffer_get( /*@special@*/ /*@out@*/ wiced_buffer_t* buffer, wwd_buffer_dir_t direction, unsigned short size, wiced_bool_t wait ) /*@allocates *buffer@*/  /*@defines **buffer@*/
{
    unsigned long wait_option = ( wait == WICED_TRUE ) ? (unsigned long) WICED_NEVER_TIMEOUT : 0;
    return internal_host_buffer_get(buffer, direction, size, wait_option);
}

void host_buffer_release( /*@only@*/ wiced_buffer_t buffer, wwd_buffer_dir_t direction )
{
    UNUSED_PARAMETER( direction );

    wiced_assert("Error: Invalid buffer\n", buffer != NULL);
    (void) pbuf_free( buffer ); /* Ignore returned number of freed segments since TCP packets will still be referenced by LWIP after relase by WICED */
}

/*@exposed@*/ uint8_t* host_buffer_get_current_piece_data_pointer( /*@temp@*/ wiced_buffer_t buffer )
{
    wiced_assert("Error: Invalid buffer\n", buffer != NULL);
    return (uint8_t*) buffer->payload;
}

uint16_t host_buffer_get_current_piece_size( /*@temp@*/ wiced_buffer_t buffer )
{
    wiced_assert("Error: Invalid buffer\n", buffer != NULL);
    return (uint16_t) buffer->len;
}

/*@exposed@*/ /*@dependent@*/ /*@null@*/ wiced_buffer_t host_buffer_get_next_piece( /*@dependent@*/ wiced_buffer_t buffer )
{
    wiced_assert("Error: Invalid buffer\n", buffer != NULL);
    return buffer->next;
}

wwd_result_t host_buffer_add_remove_at_front( wiced_buffer_t * buffer, int32_t add_remove_amount )
{
    wiced_assert("Error: Invalid buffer\n", buffer != NULL);
    if ( (u8_t) 0 != pbuf_header( *buffer, (s16_t) ( -add_remove_amount ) ) )
    {
        WPRINT_NETWORK_DEBUG(("Failed to move pointer - usually because not enough space at front of buffer\n"));
        return WWD_BUFFER_POINTER_MOVE_ERROR;
    }

    return WWD_SUCCESS;
}


wwd_result_t host_buffer_set_size( /*@temp@*/ wiced_buffer_t buffer, unsigned short size )
{
     if ( size > (unsigned short) WICED_LINK_MTU )
        {
            WPRINT_NETWORK_ERROR(("Attempt to set a length larger than the MTU of the link\n"));
            /*@-unreachable@*/ /* Reachable after hitting assert */
            return WWD_BUFFER_SIZE_SET_ERROR;
            /*@+unreachable@*/
        }
          buffer->tot_len = size;
         buffer->len = size;

         return WWD_SUCCESS;
}


void memp_free_notify( unsigned int type )
{
    if ( type == MEMP_PBUF_POOL_TX )
    {
        host_platform_bus_buffer_freed( WWD_NETWORK_TX );
    }
    else if ( type == MEMP_PBUF_POOL_RX )
    {
        host_platform_bus_buffer_freed( WWD_NETWORK_RX );
    }
}
