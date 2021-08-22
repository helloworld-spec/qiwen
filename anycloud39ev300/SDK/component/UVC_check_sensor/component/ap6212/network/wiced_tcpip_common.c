/*
 * Copyright 2017, Cypress Semiconductor Corporation or a subsidiary of 
 * Cypress Semiconductor Corporation. All Rights Reserved.
 * 
 * This software, associated documentation and materials ("Software"),
 * is owned by Cypress Semiconductor Corporation
 * or one of its subsidiaries ("Cypress") and is protected by and subject to
 * worldwide patent protection (United States and foreign),
 * United States copyright laws and international treaty provisions.
 * Therefore, you may use this Software only as provided in the license
 * agreement accompanying the software package from which you
 * obtained this Software ("EULA").
 * If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
 * non-transferable license to copy, modify, and compile the Software
 * source code solely for use in connection with Cypress's
 * integrated circuit products. Any reproduction, modification, translation,
 * compilation, or representation of this Software except as specified
 * above is prohibited without the express written permission of Cypress.
 *
 * Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
 * reserves the right to make changes to the Software without notice. Cypress
 * does not assume any liability arising out of the application or use of the
 * Software or any product or circuit described in the Software. Cypress does
 * not authorize its products for use in any products where a malfunction or
 * failure of the Cypress product may reasonably be expected to result in
 * significant property damage, injury or death ("High Risk Product"). By
 * including Cypress's product in a High Risk Product, the manufacturer
 * of such system or application assumes all risk of such use and in doing
 * so agrees to indemnify Cypress against all liability.
 */


#include "wiced_tcpip.h"
#include "wiced_network.h"
#include "wiced_tcpip_tls_api.h"
#include "internal/wiced_internal_api.h"
#include "wwd_assert.h"
#include "dns.h"

static wiced_result_t internal_wiced_tcp_connect_socket_callback     ( void* arg );
static wiced_result_t internal_wiced_tcp_disconnect_socket_callback  ( void* arg );
static wiced_result_t internal_wiced_tcp_receive_socket_callback     ( void* arg );
static wiced_result_t internal_wiced_udp_receive_socket_callback     ( void* arg );

wiced_result_t wiced_tcp_send_packet( wiced_tcp_socket_t* socket, wiced_packet_t* packet )
{
#ifndef WICED_DISABLE_TLS
    if ( socket->tls_context != NULL )
    {
        wiced_tls_encrypt_packet( &socket->tls_context->context, packet );
    }
#endif /* ifndef WICED_DISABLE_TLS */

    return network_tcp_send_packet( socket, packet );
}

wiced_result_t wiced_tcp_send_buffer( wiced_tcp_socket_t* socket, const void* buffer, uint16_t buffer_length )
{
    wiced_packet_t* packet = NULL;
    uint8_t* data_ptr = (uint8_t*)buffer;
    uint8_t* packet_data_ptr;
    uint16_t available_space;
    uint16_t data_length;
    uint16_t fragmented_data_length = 0;
    uint16_t  enc_output_buf_len;
    tls_record_t* record = NULL;
    uint8_t* data_buffer;
    uint8_t* enc_output_buf = NULL;
    uint16_t data_to_write;
    wiced_result_t result;
    wiced_bool_t enc_buf_allocated = WICED_FALSE;
    uint16_t maximum_segment_size;

    wiced_assert("Bad args", socket != NULL);

    UNUSED_PARAMETER(record);
    UNUSED_PARAMETER(enc_output_buf_len);
    UNUSED_PARAMETER(maximum_segment_size);

    WICED_LINK_CHECK_TCP_SOCKET( socket );

    data_length = buffer_length;
    maximum_segment_size = (uint16_t)WICED_MAXIMUM_SEGMENT_SIZE(socket);

    while ( buffer_length != 0 )
    {
        data_buffer = data_ptr;
        fragmented_data_length = data_length;

#ifndef WICED_DISABLE_TLS
        if ( socket->tls_context != NULL )
        {
            data_length = MIN(buffer_length, socket->tls_context->context.ext_max_fragment_length);
            fragmented_data_length = data_length;

            if(data_length > maximum_segment_size)
            {
                /* calculate the exact buffer required to accomodate encrypted data for the given payload size */
                if( ( result = wiced_tls_calculate_encrypt_buffer_length(&socket->tls_context->context,data_length,&enc_output_buf_len ) ) != WICED_SUCCESS )
                {
                    WPRINT_LIB_ERROR( ( "Error in getting encrypt buffer length \n" ) );
                    return result;
                }

                /* add record header in the buffer */
                enc_output_buf_len = (uint16_t)( enc_output_buf_len + sizeof(tls_record_header_t));

                enc_output_buf = (uint8_t*) malloc( enc_output_buf_len );

                if( enc_output_buf == NULL)
                {
                    return WICED_ERROR;
                }

                data_buffer = enc_output_buf;
                enc_buf_allocated = WICED_TRUE;

                /* copy actual application data */
                memcpy(data_buffer + sizeof(tls_record_header_t),data_ptr, data_length);

                record                = (tls_record_t*) data_buffer;
                record->type          =  TLS_MSG_APPLICATION_DATA;
                record->major_version = (uint8_t)socket->tls_context->context.major_ver;
                record->minor_version = (uint8_t)socket->tls_context->context.minor_ver;
                record->length        =  htobe16(data_length);

                /* Encrypt the record */
                result = (wiced_result_t) tls_encrypt_record( &socket->tls_context->context, record, data_length );
                if ( result != WICED_SUCCESS )
                {
                    free( enc_output_buf );
                    return result;
                }

                data_length = enc_output_buf_len;
            }
        }
#endif /* ifndef WICED_DISABLE_TLS */


        /* send TCP data in multiple packets. It will be used when length is more than MTU size */
        while(data_length != 0)
        {
            result = wiced_packet_create_tcp( socket, data_length, &packet, &packet_data_ptr, &available_space );
            if ( result != WICED_TCPIP_SUCCESS )
            {
                if ( socket->tls_context != NULL && enc_buf_allocated == WICED_TRUE)
                {
                    free ( enc_output_buf );
                }
                return result;
            }

#ifndef WICED_DISABLE_TLS
            if(socket->tls_context != NULL)
            {
                if ( socket->tls_context->context.state == SSL_HANDSHAKE_OVER && enc_buf_allocated == WICED_TRUE)
                {
                    /* this doesn't need the extra space for the encryption header that a normal TLS socket would use - remove it */
                    packet_data_ptr -= sizeof(tls_record_header_t);
                    wiced_packet_set_data_start((wiced_packet_t*) packet, packet_data_ptr);
                }
            }
#endif /* ifndef WICED_DISABLE_TLS */

            data_to_write   = MIN(data_length, available_space);
            packet_data_ptr = MEMCAT(packet_data_ptr, (uint8_t*)data_buffer, data_to_write);

            /* Write data */
            wiced_packet_set_data_end( packet, packet_data_ptr );

            /* Update variables */
            data_buffer    += data_to_write;
            data_length  = (uint16_t) ( data_length - data_to_write );
            available_space = (uint16_t) ( available_space - data_to_write );

            /* if TLS is enabled then Don't encrypt the data if its already encrypted in case of fragment is more than MSS */
#ifndef WICED_DISABLE_TLS
            if ( socket->tls_context != NULL && enc_buf_allocated == WICED_FALSE)
            {
                wiced_tls_encrypt_packet( &socket->tls_context->context, packet );
            }
#endif /* ifndef WICED_DISABLE_TLS */

            result = network_tcp_send_packet( socket, packet );
            if ( result != WICED_TCPIP_SUCCESS )
            {
                if ( socket->tls_context != NULL && enc_buf_allocated == WICED_TRUE)
                {
                    free ( enc_output_buf );
                }
                wiced_packet_delete( packet );
                return result;
            }
        }

        if (enc_buf_allocated == WICED_TRUE)
        {
            free(enc_output_buf);
            enc_output_buf = NULL;
        }

        data_ptr += fragmented_data_length;
        enc_buf_allocated = WICED_FALSE;

        buffer_length   = (uint16_t) ( buffer_length - fragmented_data_length );

    }

    return WICED_TCPIP_SUCCESS;
}

wiced_result_t wiced_tcp_receive( wiced_tcp_socket_t* socket, wiced_packet_t** packet, uint32_t timeout )
{
    wiced_assert("Bad args", (socket != NULL) && (packet != NULL) );

    WICED_LINK_CHECK_TCP_SOCKET( socket );

#ifndef WICED_DISABLE_TLS
    if ( socket->tls_context != NULL )
    {
        return wiced_tls_receive_packet( socket, packet, timeout );
    }
    else
#endif /* ifndef WICED_DISABLE_TLS */
    {
        return network_tcp_receive( socket, packet, timeout );
    }
}

wiced_result_t wiced_tcp_stream_init( wiced_tcp_stream_t* tcp_stream, wiced_tcp_socket_t* socket )
{
    tcp_stream->tx_packet                 = NULL;
    tcp_stream->rx_packet                 = NULL;
    tcp_stream->tx_packet_data_length     = 0;
    tcp_stream->socket                    = socket;
    return WICED_TCPIP_SUCCESS;
}

wiced_result_t wiced_tcp_stream_deinit( wiced_tcp_stream_t* tcp_stream )
{
    /* Flush the TX */
    wiced_tcp_stream_flush( tcp_stream );

    /* Flush the RX */
    if ( tcp_stream->rx_packet != NULL )
    {
        wiced_packet_delete( tcp_stream->rx_packet );
        tcp_stream->rx_packet = NULL;
    }
    tcp_stream->tx_packet = NULL;
    tcp_stream->rx_packet = NULL;
    tcp_stream->socket    = NULL;

    return WICED_TCPIP_SUCCESS;
}

wiced_result_t wiced_tcp_stream_write( wiced_tcp_stream_t* tcp_stream, const void* data, uint32_t data_length )
{
    int           max_fragment_length = 0;
    wiced_bool_t  send_packet = WICED_FALSE;

    wiced_assert("Bad args", tcp_stream != NULL);

    WICED_LINK_CHECK_TCP_SOCKET( tcp_stream->socket );

    if (tcp_stream->socket->tls_context != NULL )
    {
        max_fragment_length = tcp_stream->socket->tls_context->context.ext_max_fragment_length;
    }

    while ( data_length != 0 )
    {
        uint16_t amount_to_write;
        uint16_t actual_amount_to_write;

        /* Check if we don't have a packet */
        if ( tcp_stream->tx_packet == NULL )
        {
            wiced_result_t result;
            result = wiced_packet_create_tcp( tcp_stream->socket, (uint16_t) MIN( data_length, 0xffff ), &tcp_stream->tx_packet, &tcp_stream->tx_packet_data , &tcp_stream->tx_packet_space_available );
            if ( result != WICED_TCPIP_SUCCESS )
            {
                return result;
            }
        }

        /* Write data */

        amount_to_write = (uint16_t) MIN( data_length, tcp_stream->tx_packet_space_available );

        if ( tcp_stream->socket->tls_context != NULL )
        {
            actual_amount_to_write = (uint16_t) MIN( amount_to_write, (max_fragment_length - tcp_stream->tx_packet_data_length));
        }
        else
        {
            actual_amount_to_write = amount_to_write;
        }

        tcp_stream->tx_packet_data     = MEMCAT( tcp_stream->tx_packet_data, data, actual_amount_to_write );

        /* Update variables */
        data_length                           = (uint16_t)(data_length - actual_amount_to_write);
        tcp_stream->tx_packet_space_available = (uint16_t) ( tcp_stream->tx_packet_space_available - actual_amount_to_write );
        data                                  = ((char*)data + actual_amount_to_write);

        tcp_stream->tx_packet_data_length = (uint16_t)(actual_amount_to_write + tcp_stream->tx_packet_data_length);

        if ( tcp_stream->socket->tls_context != NULL )
        {
            /* check added for TLS max fragment length extension if packet space available length is 0 or packet data length is matching to fragment length
             * negotiated then we should send packet.
             */
            if ( tcp_stream->tx_packet_space_available == 0 || ( tcp_stream->tx_packet_data_length == max_fragment_length ))
            {
                send_packet = WICED_TRUE;
            }
        }
        else
        {
            if ( tcp_stream->tx_packet_space_available == 0 )
            {
                send_packet = WICED_TRUE;
            }
        }

        /* Check if the packet is full */
        if ( send_packet == WICED_TRUE)
        {
            wiced_result_t result;

            /* Send the packet */
            wiced_packet_set_data_end( tcp_stream->tx_packet, (uint8_t*)tcp_stream->tx_packet_data );
            result = wiced_tcp_send_packet( tcp_stream->socket, tcp_stream->tx_packet );

            tcp_stream->tx_packet_data            = NULL;
            tcp_stream->tx_packet_space_available = 0;

            if ( result != WICED_TCPIP_SUCCESS )
            {
                wiced_packet_delete( tcp_stream->tx_packet );
                tcp_stream->tx_packet = NULL;
                return result;
            }

            tcp_stream->tx_packet_data_length = 0;
            tcp_stream->tx_packet = NULL;
            send_packet = WICED_FALSE;
        }
    }

    return WICED_TCPIP_SUCCESS;
}

wiced_result_t wiced_tcp_stream_write_resource( wiced_tcp_stream_t* tcp_stream, const resource_hnd_t* res_id )
{
    const void*   data;
    uint32_t   res_size;
    wiced_result_t    result;
    uint32_t pos = 0;

    do
    {
        resource_result_t resource_result = resource_get_readonly_buffer ( res_id, pos, 0x7fffffff, &res_size, &data );
        if ( resource_result != RESOURCE_SUCCESS )
        {
            return ( wiced_result_t ) resource_result;
        }

        result = wiced_tcp_stream_write( tcp_stream, data, (uint16_t) res_size );
        resource_free_readonly_buffer( res_id, data );
        if ( result != WICED_SUCCESS )
        {
            return result;
        }
        pos += res_size;
    } while ( res_size > 0 );

    return result;
}

wiced_result_t wiced_tcp_stream_read( wiced_tcp_stream_t* tcp_stream, void* buffer, uint16_t buffer_length, uint32_t timeout )
{
    return wiced_tcp_stream_read_with_count( tcp_stream, buffer, buffer_length, timeout, NULL );
}

wiced_result_t wiced_tcp_stream_read_with_count( wiced_tcp_stream_t* tcp_stream, void* buffer, uint16_t buffer_length, uint32_t timeout, uint32_t* read_count )
{
    WICED_LINK_CHECK_TCP_SOCKET( tcp_stream->socket );

    if ( read_count != NULL )
    {
        *read_count = 0;
    }

    while ( buffer_length != 0 )
    {
        uint16_t amount_to_read;
        uint16_t total_available;
        uint8_t* packet_data     = NULL;
        uint16_t space_available = 0;

        /* Check if we don't have a packet */
        if (tcp_stream->rx_packet == NULL)
        {
            wiced_result_t result;
            result = wiced_tcp_receive(tcp_stream->socket, &tcp_stream->rx_packet, timeout );
            if ( result != WICED_TCPIP_SUCCESS)
            {
                if ( ( read_count != NULL ) && ( *read_count != 0 ) )
                {
                    result = WICED_TCPIP_SUCCESS;
                }
                return result;
            }
        }

        wiced_packet_get_data(tcp_stream->rx_packet, 0, &packet_data, &space_available, &total_available);

        /* Read data */
        amount_to_read = MIN(buffer_length, space_available);
        buffer         = MEMCAT((uint8_t*)buffer, packet_data, amount_to_read);

        if ( read_count != NULL )
        {
            *read_count += amount_to_read;
        }

        /* Update buffer length */
        buffer_length = (uint16_t)(buffer_length - amount_to_read);

        /* Check if we need a new packet */
        if ( amount_to_read == space_available )
        {
            wiced_packet_delete( tcp_stream->rx_packet );
            tcp_stream->rx_packet = NULL;
        }
        else
        {
            /* Otherwise update the start of the data for the next read request */
            wiced_packet_set_data_start(tcp_stream->rx_packet, packet_data + amount_to_read);
        }
    }
    return WICED_TCPIP_SUCCESS;
}

wiced_result_t wiced_tcp_stream_flush( wiced_tcp_stream_t* tcp_stream )
{
    wiced_result_t result = WICED_TCPIP_SUCCESS;

    wiced_assert("Bad args", tcp_stream != NULL && tcp_stream->socket != NULL );

    WICED_LINK_CHECK_TCP_SOCKET( tcp_stream->socket );

    /* Check if there is a packet to send */
    if ( tcp_stream->tx_packet != NULL )
    {
        wiced_packet_set_data_end(tcp_stream->tx_packet, tcp_stream->tx_packet_data);

        result = wiced_tcp_send_packet( tcp_stream->socket, tcp_stream->tx_packet );

        tcp_stream->tx_packet_data            = NULL;
        tcp_stream->tx_packet_space_available = 0;
        tcp_stream->tx_packet_data_length     = 0;
        if ( result != WICED_TCPIP_SUCCESS )
        {
            wiced_packet_delete( tcp_stream->tx_packet );
        }

        tcp_stream->tx_packet = NULL;
    }
    return result;
}

wiced_result_t wiced_hostname_lookup( const char* hostname, wiced_ip_address_t* address, uint32_t timeout_ms )
{
    wiced_assert("Bad args", (hostname != NULL) && (address != NULL));

    WICED_LINK_CHECK( WICED_STA_INTERFACE );

    /* Check if address is a string representation of a IPv4 or IPv6 address i.e. xxx.xxx.xxx.xxx */
    if ( str_to_ip( hostname, address ) == 0 )
    {
        /* yes this is a string representation of a IPv4/6 address */
        return WICED_TCPIP_SUCCESS;
    }

    return dns_client_hostname_lookup( hostname, address, timeout_ms );
}


wiced_result_t internal_defer_tcp_callback_to_wiced_network_thread( wiced_tcp_socket_t* socket, wiced_tcp_socket_callback_t callback  )
{
    event_handler_t defer_function;
    if ( callback == NULL )
    {
        return WICED_SUCCESS;
    }

    if ( ( callback == socket->callbacks.receive ) && ( ( socket->tls_context == NULL ) || ( wiced_tls_is_encryption_enabled( socket ) == WICED_TRUE ) ) )
    {
        /* If TLS is enabled, only notify application on receive after TLS handshake is over */
        defer_function = internal_wiced_tcp_receive_socket_callback;
    }
    else if ( callback == socket->callbacks.connect )
    {
        defer_function = internal_wiced_tcp_connect_socket_callback;
    }
    else if ( callback == socket->callbacks.disconnect )
    {
        defer_function = internal_wiced_tcp_disconnect_socket_callback;
    }
    else
    {
        return WICED_ERROR;
    }

    return wiced_rtos_send_asynchronous_event( WICED_NETWORKING_WORKER_THREAD, defer_function, (void*)socket );
}

static wiced_result_t internal_wiced_tcp_connect_socket_callback( void* arg )
{
    wiced_tcp_socket_t* socket = (wiced_tcp_socket_t*)arg;

    return socket->callbacks.connect( socket, socket->callback_arg );
}

static wiced_result_t internal_wiced_tcp_disconnect_socket_callback( void* arg )
{
    wiced_tcp_socket_t* socket = (wiced_tcp_socket_t*)arg;

    return socket->callbacks.disconnect( socket, socket->callback_arg );
}

static wiced_result_t internal_wiced_tcp_receive_socket_callback( void* arg )
{
    wiced_tcp_socket_t* socket = (wiced_tcp_socket_t*)arg;

    return socket->callbacks.receive( socket, socket->callback_arg );
}


wiced_result_t internal_defer_udp_callback_to_wiced_network_thread( wiced_udp_socket_t* socket )
{
    if ( socket->receive_callback == NULL )
    {
        return WICED_SUCCESS;
    }

    return wiced_rtos_send_asynchronous_event( WICED_NETWORKING_WORKER_THREAD, internal_wiced_udp_receive_socket_callback, (void*)socket );
}

static wiced_result_t internal_wiced_udp_receive_socket_callback( void* arg )
{
    wiced_udp_socket_t* socket = (wiced_udp_socket_t*)arg;

    return socket->receive_callback( socket, socket->callback_arg );
}
