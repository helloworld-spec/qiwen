/*
 * Broadcom Proprietary and Confidential. Copyright 2016 Broadcom
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */

/** @file
 */

#include <string.h>
#include "wiced.h"
#include "wiced_network.h"
#include "wiced_utilities.h"
#include "wwd_debug.h"
#include "wwd_assert.h"
#include "lwip/init.h"
#include "lwip/tcpip.h"
#include "netif/etharp.h"
#include "lwip/dhcp.h"
#include "wwd_management.h"
#include "wwd_network.h"
#include "dhcp_server.h"
#include "dns.h"
#include "internal/wiced_internal_api.h"
#include "lwip/dns.h"
#include "wiced_p2p.h"

#if LWIP_NETIF_HOSTNAME
#include "platform_dct.h"

#ifdef NETWORK_CONFIG_APPLICATION_DEFINED
#include "network_config_dct.h"
#else/* #ifdef NETWORK_CONFIG_APPLICATION_DEFINED */
#include "default_network_config_dct.h"
#endif /* #ifdef NETWORK_CONFIG_APPLICATION_DEFINED */

#endif /* LWIP_NETIF_HOSTNAME */

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

#define TIME_WAIT_TCP_SOCKET_DELAY (400)
#define MAXIMUM_IP_ADDRESS_CHANGE_CALLBACKS           (2)

/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/

typedef struct
{
    wiced_ip_address_change_callback_t callback;
    void*                              arg;
} ip_address_change_callback_t;

/******************************************************
 *                 Static Variables
 ******************************************************/

#ifdef WICED_USE_WIFI_STA_INTERFACE
    static struct netif       sta_ip_handle;
    #define STA_IP_HANDLE    &sta_ip_handle
#else
    #define STA_IP_HANDLE    NULL
#endif

#ifdef WICED_USE_WIFI_AP_INTERFACE
    static struct netif      ap_ip_handle;
    #define AP_IP_HANDLE    &ap_ip_handle
#else
    #define AP_IP_HANDLE    NULL
#endif

#ifdef WICED_USE_WIFI_P2P_INTERFACE
    static struct netif       p2p_ip_handle;
    #define P2P_IP_HANDLE    &p2p_ip_handle
#else
    #define P2P_IP_HANDLE    NULL
#endif

#if LWIP_NETIF_HOSTNAME
static wiced_hostname_t    hostname;
#endif /* LWIP_NETIF_HOSTNAME */

struct netif* wiced_ip_handle[ 3 ] =
{
    [WICED_STA_INTERFACE] = STA_IP_HANDLE,
    [WICED_AP_INTERFACE]  = AP_IP_HANDLE,
    [WICED_P2P_INTERFACE] = P2P_IP_HANDLE
};

/* Network objects */
struct dhcp         wiced_dhcp_handle;
static wiced_bool_t        wiced_using_dhcp;
static wiced_dhcp_server_t internal_dhcp_server;

/* IP status callback variables */
static ip_address_change_callback_t wiced_ip_address_change_callbacks[MAXIMUM_IP_ADDRESS_CHANGE_CALLBACKS];

const wiced_ip_address_t const INITIALISER_IPV4_ADDRESS( wiced_ip_broadcast, 0xFFFFFFFF );

wiced_mutex_t            lwip_send_interface_mutex;

/* Wi-Fi power save state */
static uint8_t wifi_powersave_mode = 0;
static uint16_t wifi_return_to_sleep_delay = 0;

/******************************************************
 *               Static Function Declarations
 ******************************************************/

static void tcpip_init_done( void* arg );
static void ip_address_changed_handler( struct netif *netif, ip_addr_t *new_ip );
static void invalidate_all_arp_entries( struct netif *netif );

/* free_entry() is an LwIP function in etharp.c. It is used in invalidate_all_arp_entries() */
extern void free_entry( int i );

/******************************************************
 *               Function Definitions
 ******************************************************/

wiced_result_t wiced_network_init( void )
{
    wwd_result_t result;
    host_semaphore_type_t lwip_done_sema;

    /* Initialize the LwIP system.  */
    WPRINT_NETWORK_INFO(("Initialising LwIP " LwIP_VERSION "\n"));

    ip_networking_inited[0] = WICED_FALSE;
    ip_networking_inited[1] = WICED_FALSE;
    ip_networking_inited[2] = WICED_FALSE;

    /* Create a semaphore to signal when LwIP has finished initialising */
    result = host_rtos_init_semaphore(  &lwip_done_sema );
    if ( result != WWD_SUCCESS )
    {
        /* Could not create semaphore */
        WPRINT_NETWORK_ERROR(("Could not create LwIP init semaphore"));
        return WICED_ERROR;
    }

    /* Initialise LwIP, providing the callback function and callback semaphore */
    tcpip_init( tcpip_init_done, (void*) &lwip_done_sema );
    host_rtos_get_semaphore( &lwip_done_sema, NEVER_TIMEOUT, WICED_FALSE );
    host_rtos_deinit_semaphore( &lwip_done_sema );

    memset(&internal_dhcp_server, 0, sizeof(internal_dhcp_server));

    /* create a mutex for link up and down registrations */
    wiced_rtos_init_mutex( &link_subscribe_mutex );
    /* Create a mutex for UDP and TCP sending with ability to swap a default interface */
    wiced_rtos_init_mutex( &lwip_send_interface_mutex );

    memset( link_up_callbacks_wireless,   0, sizeof( link_up_callbacks_wireless ) );
    memset( link_down_callbacks_wireless, 0, sizeof( link_down_callbacks_wireless ) );
#ifdef WICED_USE_ETHERNET_INTERFACE
    memset( link_up_callbacks_ethernet,   0, sizeof(link_up_callbacks_ethernet ) );
    memset( link_down_callbacks_ethernet, 0, sizeof(link_down_callbacks_ethernet ) );
#endif

#if LWIP_NETIF_HOSTNAME
    /* Get hostname */
    memset( hostname, 0, sizeof(hostname) );
    result = wiced_network_get_hostname( &hostname )
    if( result != WICED_SUCCESS )
    {
        return result;
    }
#endif /* LWIP_NETIF_HOSTNAME */

    wiced_using_dhcp = WICED_FALSE;

    return WICED_SUCCESS;
}

/**
 *  LwIP init complete callback
 *
 *  This function is called by LwIP when initialisation has finished.
 *  A semaphore is posted to allow the startup thread to resume, and to run the app_main function
 *
 * @param arg : the handle for the semaphore to post (cast to a void pointer)
 */

static void tcpip_init_done( void* arg )
{
    host_semaphore_type_t* LwIP_done_sema = (host_semaphore_type_t*) arg;
    host_rtos_set_semaphore( LwIP_done_sema, WICED_FALSE );
}



wiced_result_t wiced_network_deinit( void )
{
    tcpip_deinit( );
    wiced_rtos_deinit_mutex( &link_subscribe_mutex );
    wiced_rtos_deinit_mutex( &lwip_send_interface_mutex );
    return WICED_SUCCESS;
}

wiced_result_t wiced_network_suspend(void)
{
    /* Stop all registered TCP/IP timers */
    if( tcpip_deactivate_tcpip_timeouts() != ERR_OK )
    {
        return WICED_ERROR;
    }
    return WICED_SUCCESS;
}

wiced_result_t wiced_network_resume(void)
{
    /* Resume all TCP/IP timers again */
    if( tcpip_activate_tcpip_timeouts() != ERR_OK )
    {
        return WICED_ERROR;
    }

    return WICED_SUCCESS;
}

wiced_result_t wiced_ip_up( wiced_interface_t interface, wiced_network_config_t config, const wiced_ip_setting_t* ip_settings )
{
    struct ip_addr ipaddr;
    struct ip_addr netmask;
    struct ip_addr gw;
    wiced_bool_t static_ip;

    if ( IP_NETWORK_IS_INITED( interface ) == WICED_TRUE )
    {
        return WICED_SUCCESS;
    }

    static_ip = ( ( config == WICED_USE_STATIC_IP || config == WICED_USE_INTERNAL_DHCP_SERVER) && ip_settings != NULL )? WICED_TRUE : WICED_FALSE;

    /* Enable the network interface */
    if ( static_ip == WICED_TRUE )
    {
        ipaddr.addr  = htonl(ip_settings->ip_address.ip.v4);
        gw.addr      = htonl(ip_settings->gateway.ip.v4);
        netmask.addr = htonl(ip_settings->netmask.ip.v4);
    }
    else
    {
        /* make sure that ip address is zero in order to start dhcp client */
        ip_addr_set_zero( &gw );
        ip_addr_set_zero( &ipaddr );
        ip_addr_set_zero( &netmask );
    }

    if ( NULL == netif_add( &IP_HANDLE(interface), &ipaddr, &netmask, &gw, (void*) (ptrdiff_t) WICED_TO_WWD_INTERFACE( interface ), ethernetif_init, ethernet_input ) )
    {
        WPRINT_NETWORK_ERROR(( "Could not add network interface\n" ));
        return WICED_ERROR;
    }

    /* Register a handler for any address changes */
    netif_set_ipchange_callback( &IP_HANDLE(interface), ip_address_changed_handler );

    if ( ( static_ip == WICED_FALSE ) && ( config == WICED_USE_EXTERNAL_DHCP_SERVER ))
    {
        wiced_ip_address_t dns_server_address;
        uint32_t     address_resolution_timeout = WICED_DHCP_IP_ADDRESS_RESOLUTION_TIMEOUT;
        wiced_bool_t timeout_occured            = WICED_FALSE;

        /* Bring up the network interface */
        netif_set_up( &IP_HANDLE(interface) );
        netif_set_default( &IP_HANDLE(interface) );

        WPRINT_NETWORK_INFO(("Obtaining IP address via DHCP\n"));
        dhcp_set_struct( &IP_HANDLE(interface), &wiced_dhcp_handle );

#if LWIP_NETIF_HOSTNAME
        IP_HANDLE(interface).hostname = &hostname.value;
#endif

        dhcp_start( &IP_HANDLE(interface) );
        while ( wiced_dhcp_handle.state != DHCP_BOUND  && timeout_occured == WICED_FALSE )
        {
            sys_msleep( 10 );
            address_resolution_timeout -= 10;

            if ( address_resolution_timeout <= 0 )
            {
                /* timeout has occured */
                timeout_occured = WICED_TRUE;
            }
        }

        if ( timeout_occured == WICED_TRUE )
        {
            dhcp_stop( &IP_HANDLE(interface) );
            netif_remove( &IP_HANDLE(interface) );
            return WICED_ERROR;
        }

        wiced_using_dhcp = WICED_TRUE;

        /* Check if DNS servers were supplied by the DHCP client */
#if LWIP_DHCP && LWIP_DNS
        if ( (dns_getserver( 0 )).addr != IP_ADDR_ANY->addr )
        {
            u8_t i;
            ip_addr_t lwip_dns_server_addr;

            dns_server_address.version = WICED_IPV4;
            for (i = 0; i < DNS_MAX_SERVERS; i++)
            {
                lwip_dns_server_addr = dns_getserver(i);
                if (lwip_dns_server_addr.addr != IP_ADDR_ANY->addr)
                {
                    dns_server_address.ip.v4 = ntohl(lwip_dns_server_addr.addr);
                    dns_client_add_server_address(dns_server_address);
                }
            }
        }
        else
#endif /* if LWIP_DHCP && LWIP_DNS */
        {
            /* DNS servers were not supplied by DHCP client... */
            /* Add gateway DNS server and Google public DNS server */
            wiced_ip_get_gateway_address( interface, &dns_server_address );
            dns_client_add_server_address( dns_server_address );


            /* Google DNS server is 8.8.8.8 */
            memset( &dns_server_address.ip.v4, 8, sizeof( dns_server_address.ip.v4 ) );
            dns_client_add_server_address( dns_server_address );
        }

        /* Register for IP address change notification */
        /* TODO: Add support for IP address change notification */
    }
    else
    {
        netif_set_up( &IP_HANDLE(interface) );

        igmp_start(&IP_HANDLE(interface));

        /* Check if we should start the DHCP server */
        if ( config == WICED_USE_INTERNAL_DHCP_SERVER )
        {
            wiced_start_dhcp_server(&internal_dhcp_server, interface);
        }
    }

    SET_IP_NETWORK_INITED( interface, WICED_TRUE );

    WPRINT_NETWORK_INFO( ( "Network ready IP: %u.%u.%u.%u\n", (unsigned char) ( ( htonl( IP_HANDLE(interface).ip_addr.addr ) >> 24 ) & 0xff ),
        (unsigned char) ( ( htonl( IP_HANDLE(interface).ip_addr.addr ) >> 16 ) & 0xff ),
        (unsigned char) ( ( htonl( IP_HANDLE(interface).ip_addr.addr ) >>  8 ) & 0xff ),
        (unsigned char) ( ( htonl( IP_HANDLE(interface).ip_addr.addr ) >>  0 ) & 0xff ) ) );

    return WICED_SUCCESS;
}

wiced_result_t wiced_ip_down( wiced_interface_t interface )
{
    if ( IP_NETWORK_IS_INITED( interface ) == WICED_TRUE )
    {
        /* Cleanup DNS client and DHCP server/client depending on interface */
        if ( ( interface == WICED_AP_INTERFACE ) || ( interface == WICED_CONFIG_INTERFACE )
                                                 || ( ( interface == WICED_P2P_INTERFACE ) && ( wwd_wifi_p2p_go_is_up == WICED_TRUE ) ) )
        {
            wiced_stop_dhcp_server( &internal_dhcp_server );

            /* Wait till the time wait sockets get closed */
            sys_msleep(TIME_WAIT_TCP_SOCKET_DELAY);
        }
        else /* STA interface */
        {
            if ( wiced_using_dhcp == WICED_TRUE )
            {
                dhcp_stop( &IP_HANDLE(interface) );
                wiced_using_dhcp = WICED_FALSE;
            }
            dns_client_remove_all_server_addresses();
        }

        /* Delete the network interface */
        netif_remove( &IP_HANDLE(interface) );

        SET_IP_NETWORK_INITED( interface, WICED_FALSE );
    }
    return WICED_SUCCESS;
}
void wiced_network_notify_link_up( wiced_interface_t interface )
{
    /* Notify LwIP that the link is up */
    netif_set_up( &IP_HANDLE(interface) );
}

void wiced_network_notify_link_down( wiced_interface_t interface )
{
    /* Notify LwIP that the link is down*/
    netif_set_down( &IP_HANDLE(interface) );
}

wiced_result_t wiced_wireless_link_down_handler( void* arg )
{
    int i = 0;
    UNUSED_PARAMETER( arg );

    WPRINT_NETWORK_DEBUG( ("Wireless link DOWN!\n\r") );

    if ( wiced_using_dhcp == WICED_TRUE )
    {
        dhcp_stop( &IP_HANDLE(WICED_STA_INTERFACE) );
    }
    /* Wait for a while before the time wait sockets get closed */
    sys_msleep( TIME_WAIT_TCP_SOCKET_DELAY );

    /* Add clearing of ARP cache */
    netifapi_netif_common( &IP_HANDLE(WICED_STA_INTERFACE), (netifapi_void_fn) invalidate_all_arp_entries, NULL );

    /* Inform all subscribers about the link down event */
    for ( i = 0; i < WICED_MAXIMUM_LINK_CALLBACK_SUBSCRIPTIONS; i++ )
    {
        if ( link_down_callbacks_wireless[i] != NULL )
        {
            link_down_callbacks_wireless[i]( );
        }
    }

    /* Kick the radio chip if it's in power save mode in case the link down event is due to missing beacons. Setting the chip to the same power save mode is sufficient. */
    wifi_powersave_mode = wiced_wifi_get_powersave_mode();
    if ( wifi_powersave_mode == PM1_POWERSAVE_MODE )
    {
        wiced_wifi_enable_powersave();
    }
    else if ( wifi_powersave_mode == PM2_POWERSAVE_MODE )
    {
        wifi_return_to_sleep_delay = wiced_wifi_get_return_to_sleep_delay();
        wiced_wifi_enable_powersave_with_throughput( wifi_return_to_sleep_delay );
    }

    return WICED_SUCCESS;
}

wiced_result_t wiced_wireless_link_up_handler( void* arg )
{
    int i = 0;
    ip_addr_t ip_addr;
    wiced_result_t result = WICED_SUCCESS;

    UNUSED_PARAMETER( arg );

    WPRINT_NETWORK_DEBUG(("Wireless link UP!\n\r"));

    if ( wiced_using_dhcp == WICED_TRUE )
    {
        /* Save the current power save state */
        wifi_powersave_mode        = wiced_wifi_get_powersave_mode();
        wifi_return_to_sleep_delay = wiced_wifi_get_return_to_sleep_delay();

        /* Disable power save for the DHCP exchange */
        if ( wifi_powersave_mode != NO_POWERSAVE_MODE )
        {
            wiced_wifi_disable_powersave( );
        }

        /* For DHCP only, we should reset netif IP address. We don't want to re-use previous netif IP address given from previous DHCP session */
        ip_addr_set_zero( &ip_addr );
        netif_set_ipaddr( &IP_HANDLE(WICED_STA_INTERFACE), &ip_addr);

        /* Restart DHCP */
        if ( dhcp_start( &IP_HANDLE(WICED_STA_INTERFACE)) != ERR_OK )
        {
            WPRINT_NETWORK_ERROR( ( "Failed to initiate DHCP transaction\n" ) );
            result = WICED_ERROR;
        }

        /* Wait a little to allow DHCP a chance to complete, but we can't block here */
        host_rtos_delay_milliseconds( 10 );

        if ( wifi_powersave_mode == PM1_POWERSAVE_MODE )
        {
            wiced_wifi_enable_powersave( );
        }
        else if ( wifi_powersave_mode == PM2_POWERSAVE_MODE )
        {
            wiced_wifi_enable_powersave_with_throughput( wifi_return_to_sleep_delay );
        }
    }

    /* Inform all subscribers about an event */
    for ( i = 0; i < WICED_MAXIMUM_LINK_CALLBACK_SUBSCRIPTIONS; i++ )
    {
        if ( link_up_callbacks_wireless[i] != NULL )
        {
            link_up_callbacks_wireless[i]( );
        }
    }

    return result;
}

wiced_result_t wiced_wireless_link_renew_handler( void* arg )
{
    UNUSED_PARAMETER( arg );

    /* TODO : need to invalidate only particular arp instead of all arp entries*/
    netifapi_netif_common( &IP_HANDLE(WICED_STA_INTERFACE), (netifapi_void_fn) invalidate_all_arp_entries, NULL );

    /* TODO: Do a DHCP renew if interface is using external DHCP server */

    if ( wiced_dhcp_handle.state == DHCP_BOUND )
    {
        netifapi_netif_common( &IP_HANDLE(WICED_STA_INTERFACE), (netifapi_void_fn)dhcp_renew, NULL);
    }

    return WICED_SUCCESS;
}

static void ip_address_changed_handler( struct netif *netif, ip_addr_t *new_ip )
{
    uint8_t i;

    UNUSED_PARAMETER( netif );
    UNUSED_PARAMETER( new_ip );

    for ( i = 0; i < MAXIMUM_IP_ADDRESS_CHANGE_CALLBACKS; i++ )
    {
        if ( wiced_ip_address_change_callbacks[i].callback != NULL )
        {
            ( *wiced_ip_address_change_callbacks[i].callback )( wiced_ip_address_change_callbacks[i].arg );
        }
    }
}

wiced_result_t wiced_ip_register_address_change_callback( wiced_ip_address_change_callback_t callback, void* arg )
{
    uint8_t i;
    for ( i = 0; i < MAXIMUM_IP_ADDRESS_CHANGE_CALLBACKS; i++ )
    {
        if ( wiced_ip_address_change_callbacks[i].callback == NULL )
        {
            wiced_ip_address_change_callbacks[i].callback = callback;
            wiced_ip_address_change_callbacks[i].arg = arg;
            return WICED_SUCCESS;
        }
    }

    WPRINT_NETWORK_ERROR( ( "Out of callback storage space\n" ) );

    return WICED_ERROR;
}

wiced_result_t wiced_ip_deregister_address_change_callback( wiced_ip_address_change_callback_t callback )
{
    uint8_t i;
    for ( i = 0; i < MAXIMUM_IP_ADDRESS_CHANGE_CALLBACKS; i++ )
    {
        if ( wiced_ip_address_change_callbacks[i].callback == callback )
        {
            memset( &wiced_ip_address_change_callbacks[i], 0, sizeof( wiced_ip_address_change_callback_t ) );
            return WICED_SUCCESS;
        }
    }

    WPRINT_NETWORK_ERROR( ( "Unable to find callback to deregister\n" ) );

    return WICED_ERROR;
}

/**
 * Remove all ARP table entries of the specified netif.
 * @param netif points to a network interface
 */
static void invalidate_all_arp_entries( struct netif *netif )
{
    u8_t i;
    LWIP_UNUSED_ARG(netif);

    /*free all the entries in arp list */

    for ( i = 0; i < ARP_TABLE_SIZE; ++i )
    {
        free_entry( i );
    }
}
