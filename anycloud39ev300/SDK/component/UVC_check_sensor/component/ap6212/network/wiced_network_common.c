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


#include <ctype.h>
#include "wiced_management.h"
#include "wiced_framework.h"
#include "wiced_rtos.h"
#include "wiced_deep_sleep.h"
#include "internal/wiced_internal_api.h"
#include "wwd_assert.h"
#ifdef WICED_USE_ETHERNET_INTERFACE
#include "platform_ethernet.h"
#endif /* ifdef WICED_USE_ETHERNET_INTERFACE */

#ifdef NETWORK_CONFIG_APPLICATION_DEFINED
#include "network_config_dct.h"
#else/* #ifdef NETWORK_CONFIG_APPLICATION_DEFINED */
#include "default_network_config_dct.h"
#endif /* #ifdef NETWORK_CONFIG_APPLICATION_DEFINED */

/* IP networking status */
wiced_bool_t ip_networking_inited[WICED_INTERFACE_MAX];
wiced_mutex_t link_subscribe_mutex;

/* Link status callback variables */
wiced_network_link_callback_t link_up_callbacks_wireless[WICED_MAXIMUM_LINK_CALLBACK_SUBSCRIPTIONS];
wiced_network_link_callback_t link_down_callbacks_wireless[WICED_MAXIMUM_LINK_CALLBACK_SUBSCRIPTIONS];

#ifdef WICED_USE_ETHERNET_INTERFACE
wiced_network_link_callback_t link_up_callbacks_ethernet[WICED_MAXIMUM_LINK_CALLBACK_SUBSCRIPTIONS];
wiced_network_link_callback_t link_down_callbacks_ethernet[WICED_MAXIMUM_LINK_CALLBACK_SUBSCRIPTIONS];
#endif

wiced_bool_t wiced_network_is_up( wiced_interface_t interface )
{
    wiced_bool_t result = WICED_FALSE;

#ifdef WICED_USE_ETHERNET_INTERFACE
    if( interface == WICED_ETHERNET_INTERFACE )
    {
        result = platform_ethernet_is_inited();
    }
    else
#else
    wiced_assert("Bad args", interface != WICED_ETHERNET_INTERFACE);
#endif
    {
        result = (wwd_wifi_is_ready_to_transceive( WICED_TO_WWD_INTERFACE(interface) ) == WWD_SUCCESS) ? WICED_TRUE : WICED_FALSE;
    }

    return result;
}

wiced_bool_t wiced_network_is_ip_up( wiced_interface_t interface )
{
    return IP_NETWORK_IS_INITED( interface );
}

wiced_result_t wiced_network_up_default( wiced_interface_t* interface, const wiced_ip_setting_t* ap_ip_settings )
{
    wiced_result_t result;
    platform_dct_network_config_t* dct_network_config;

    /* Read config */
    wiced_dct_read_lock( (void**) &dct_network_config, WICED_TRUE, DCT_NETWORK_CONFIG_SECTION, 0, sizeof(platform_dct_network_config_t) );
    *interface = dct_network_config->interface;
    wiced_dct_read_unlock( dct_network_config, WICED_FALSE );


    /* Bring up the network interface */
    if( *interface == WICED_STA_INTERFACE )
    {
        result = wiced_network_up( *interface, WICED_USE_EXTERNAL_DHCP_SERVER, NULL );
    }
    else if( *interface == WICED_AP_INTERFACE )
    {
        result = wiced_network_up( *interface, WICED_USE_INTERNAL_DHCP_SERVER, ap_ip_settings );
    }
    else if( *interface == WICED_ETHERNET_INTERFACE )
    {
        result = wiced_network_up( *interface, WICED_USE_EXTERNAL_DHCP_SERVER, NULL );
    }
    else
    {
        result = WICED_BADOPTION;
    }
    return result;
}

wiced_result_t wiced_get_default_ready_interface( wiced_interface_t* interface )
{
    if ( !interface )
    {
        return WICED_ERROR;
    }

    if ( wiced_network_is_up( WICED_STA_INTERFACE ) == WICED_TRUE )
    {
        *interface = WICED_STA_INTERFACE;
    }
    else if ( wiced_network_is_up( WICED_AP_INTERFACE ) == WICED_TRUE )
    {
        *interface = WICED_AP_INTERFACE;
    }
    else if ( wiced_network_is_up( WICED_P2P_INTERFACE ) == WICED_TRUE )
    {
        *interface = WICED_P2P_INTERFACE;
    }
#ifdef WICED_USE_ETHERNET_INTERFACE
    else if ( wiced_network_is_up( WICED_ETHERNET_INTERFACE ) == WICED_TRUE )
    {
        *interface = WICED_ETHERNET_INTERFACE;
    }
#endif
    else
    {
        return WICED_ERROR;
    }

    return WICED_SUCCESS;
}

wiced_result_t wiced_wifi_find_best_channel(int32_t band, uint32_t *channel, uint8_t *cca_score)
{
    return wwd_wifi_get_best_cca(band, channel, cca_score, 75); /* 75 ms dwell time by default */
}

wiced_result_t wiced_network_set_hostname( const char* name )
{
    wiced_hostname_t temp_hostname;

    wiced_assert("Bad args", name != NULL);

    memset( &temp_hostname, 0, sizeof( temp_hostname ) );
    strncpy( temp_hostname.value, name, HOSTNAME_SIZE );

    return wiced_dct_write( temp_hostname.value, DCT_NETWORK_CONFIG_SECTION, OFFSETOF( platform_dct_network_config_t, hostname ), sizeof(wiced_hostname_t) );
}

wiced_result_t wiced_network_get_hostname( wiced_hostname_t* hostname )
{
    wiced_assert("Bad args", hostname != NULL);

    /* Read config */
    return wiced_dct_read_with_copy( hostname->value, DCT_NETWORK_CONFIG_SECTION, OFFSETOF( platform_dct_network_config_t, hostname ), sizeof(wiced_hostname_t) );
}

wiced_result_t wiced_network_up( wiced_interface_t interface, wiced_network_config_t config, const wiced_ip_setting_t* ip_settings )
{
    wiced_result_t result = WICED_SUCCESS;

    if ( wiced_network_is_up( interface ) == WICED_FALSE )
    {
        if ( interface == WICED_CONFIG_INTERFACE )
        {
            wiced_config_soft_ap_t* config_ap;
            wiced_result_t retval = wiced_dct_read_lock( (void**) &config_ap, WICED_FALSE, DCT_WIFI_CONFIG_SECTION, OFFSETOF(platform_dct_wifi_config_t, config_ap_settings), sizeof(wiced_config_soft_ap_t) );
            if ( retval != WICED_SUCCESS )
            {
                return retval;
            }

            /* Check config DCT is valid */
            if ( config_ap->details_valid == CONFIG_VALIDITY_VALUE )
            {
                result = wiced_start_ap( &config_ap->SSID, config_ap->security, config_ap->security_key, config_ap->channel );
            }
            else
            {
                wiced_ssid_t ssid =
                {
                    .length =  sizeof("Wiced Config")-1,
                    .value  = "Wiced Config",
                };
                result = wiced_start_ap( &ssid, WICED_SECURITY_OPEN, "", 1 );
            }
            wiced_dct_read_unlock( config_ap, WICED_FALSE );
        }
        else if ( interface == WICED_AP_INTERFACE )
        {
            wiced_config_soft_ap_t* soft_ap;
            result = wiced_dct_read_lock( (void**) &soft_ap, WICED_FALSE, DCT_WIFI_CONFIG_SECTION, OFFSETOF(platform_dct_wifi_config_t, soft_ap_settings), sizeof(wiced_config_soft_ap_t) );
            if ( result != WICED_SUCCESS )
            {
                return result;
            }
            result = (wiced_result_t) wwd_wifi_start_ap( &soft_ap->SSID, soft_ap->security, (uint8_t*) soft_ap->security_key, soft_ap->security_key_length, soft_ap->channel );
            if ( result != WICED_SUCCESS )
            {
                WPRINT_APP_INFO(( "Error: wwd_wifi_start_ap failed\n" ));
                return result;
            }
            wiced_dct_read_unlock( soft_ap, WICED_FALSE );
        }
        else if ( interface == WICED_STA_INTERFACE )
        {
            result = wiced_join_ap( );
        }
#ifdef WICED_USE_ETHERNET_INTERFACE
        else if ( interface == WICED_ETHERNET_INTERFACE )
        {
            result = platform_ethernet_init( );
        }
#endif
        else
        {
            result = WICED_ERROR;
        }
    }

    if ( result != WICED_SUCCESS )
    {
        return result;
    }

    result = wiced_ip_up( interface, config, ip_settings );
    if ( result != WICED_SUCCESS )
    {
        if ( interface == WICED_STA_INTERFACE )
        {
            wiced_leave_ap( interface );
        }
        else if ( interface != WICED_ETHERNET_INTERFACE )
        {
            wiced_stop_ap( );
        }
#ifdef WICED_USE_ETHERNET_INTERFACE
        else if ( interface == WICED_ETHERNET_INTERFACE )
        {
            platform_ethernet_deinit( );
        }
#endif
    }

    return result;
}

wiced_result_t wiced_network_resume_after_deep_sleep( wiced_interface_t interface, wiced_network_config_t config, const wiced_ip_setting_t* ip_settings )
{
    /*
     * Current implementation assumes wiced_network_up() during cold boot joins AP and then bring network interface up.
     * During resuming from deep-sleep same wiced_network_up() is called.
     * It calls wiced_network_is_up() which returns true, and joining is skipped. State returned by wiced_network_is_up() is preserved across deep-sleep.
     * If connection to AP lost, joining be tried.
     */

    if ( !WICED_DEEP_SLEEP_IS_ENABLED( ) )
    {
        wiced_assert( "Deep-sleep is not supported", 0 );
        return WICED_UNSUPPORTED;
    }

    if ( interface != WICED_STA_INTERFACE )
    {
        return WICED_UNSUPPORTED;
    }

    return wiced_network_up( interface, config, ip_settings );
}


/* Bring down the network interface
 *
 * @param interface       : wiced_interface_t, either WICED_AP_INTERFACE or WICED_STA_INTERFACE
 *
 * @return  WICED_SUCCESS : completed successfully
 *
 */
wiced_result_t wiced_network_down( wiced_interface_t interface )
{
    wiced_result_t result;
    result = wiced_ip_down( interface );

    if ( result != WICED_SUCCESS ) {
        return result;
    }

    if ( wiced_network_is_up( interface ) == WICED_TRUE )
    {
        /* Stop Wi-Fi */
        if ( ( interface == WICED_AP_INTERFACE ) || ( interface == WICED_CONFIG_INTERFACE ) )
        {
            wiced_stop_ap( );
        }
        else if ( interface == WICED_P2P_INTERFACE )
        {
            if ( wwd_wifi_p2p_go_is_up )
            {
                wwd_wifi_p2p_go_is_up = WICED_FALSE;
            }
            else
            {
                wiced_leave_ap( interface );
            }
        }
        else if ( interface == WICED_STA_INTERFACE )
        {
            wiced_leave_ap( interface );
        }
#ifdef WICED_USE_ETHERNET_INTERFACE
        else if ( interface == WICED_ETHERNET_INTERFACE )
        {
            platform_ethernet_deinit( );
        }
#endif
        else
        {
            return WICED_ERROR;
        }
    }

    return WICED_SUCCESS;
}



wiced_result_t wiced_network_register_link_callback( wiced_network_link_callback_t link_up_callback, wiced_network_link_callback_t link_down_callback, wiced_interface_t interface )
{
    int i = 0;

    wiced_rtos_lock_mutex( &link_subscribe_mutex );

    /* Find next empty slot among the list of currently subscribed */
    for ( i = 0; i < WICED_MAXIMUM_LINK_CALLBACK_SUBSCRIPTIONS; ++i )
    {
        if ( link_up_callback != NULL && (LINK_UP_CALLBACKS_LIST( interface ))[i] == NULL )
        {
            (LINK_UP_CALLBACKS_LIST( interface ))[i] = link_up_callback;
            link_up_callback = NULL;
        }

        if ( link_down_callback != NULL && (LINK_DOWN_CALLBACKS_LIST( interface ))[i] == NULL )
        {
            (LINK_DOWN_CALLBACKS_LIST( interface ))[i] = link_down_callback;
            link_down_callback = NULL;
        }
    }

    wiced_rtos_unlock_mutex( &link_subscribe_mutex );

    /* Check if we didn't find a place of either of the callbacks */
    if ( (link_up_callback != NULL) || (link_down_callback != NULL) )
    {
        return WICED_ERROR;
    }
    else
    {
        return WICED_SUCCESS;
    }
}

wiced_result_t wiced_network_deregister_link_callback( wiced_network_link_callback_t link_up_callback, wiced_network_link_callback_t link_down_callback, wiced_interface_t interface )
{
    int i = 0;

    wiced_rtos_lock_mutex( &link_subscribe_mutex );

    /* Find matching callbacks */
    for ( i = 0; i < WICED_MAXIMUM_LINK_CALLBACK_SUBSCRIPTIONS; ++i )
    {
        if ( link_up_callback != NULL && (LINK_UP_CALLBACKS_LIST( interface ))[i] == link_up_callback )
        {
            (LINK_UP_CALLBACKS_LIST( interface ))[i] = NULL;
        }

        if ( link_down_callback != NULL && (LINK_DOWN_CALLBACKS_LIST( interface ))[i] == link_down_callback )
        {
            (LINK_DOWN_CALLBACKS_LIST( interface ))[i] = NULL;
        }
    }

    wiced_rtos_unlock_mutex( &link_subscribe_mutex );

    return WICED_SUCCESS;
}

/*
 ******************************************************************************
 * Convert an ipv4 string to a uint32_t.
 *
 * @param     arg  The string containing the value.
 * @param     arg  The structure which will receive the IP address
 *
 * @return    0 if read successfully
 */
int str_to_ip( const char* arg, wiced_ip_address_t* address )
{
    uint32_t* addr = &address->ip.v4;
    uint8_t num = 0;

    arg--;

    *addr = 0;

    do
    {
        uint32_t tmp_val = 0;
        *addr = *addr << 8;
        string_to_unsigned( ++arg, 3, &tmp_val, 0 );
        *addr += (uint32_t) tmp_val;
        while ( ( *arg != '\x00' ) && ( *arg != '.' ) )
        {
            if ( isdigit( (int) *arg ) == WICED_FALSE )
            {
                return -1;
            }
            arg++;
        }
        num++;
    } while ( ( num < 4 ) && ( *arg != '\x00' ) );
    if ( num == 4 )
    {

        address->version = WICED_IPV4;
        return 0;
    }
    return -1;
}


void format_wep_keys( char* wep_key_output, const char* wep_key_data, uint8_t* wep_key_length, wep_key_format_t wep_key_format )
{
    int              a;
    uint8_t          wep_key_entry_size;
    wiced_wep_key_t* wep_key = (wiced_wep_key_t*)wep_key_output;

    /* Setup WEP key 0 */
    wep_key[0].index  = 0;

    if ( wep_key_format == WEP_KEY_HEX_FORMAT )
    {
        wep_key[0].length = *wep_key_length >> 1;
        for ( a = 0; a < wep_key[0].length; ++a )
        {
            uint8_t nibble1 = 0;
            uint8_t nibble2 = 0;
            if ( hexchar_to_nibble( wep_key_data[a*2],     &nibble1 ) == -1 ||
                 hexchar_to_nibble( wep_key_data[a*2 + 1], &nibble2 ) == -1    )
            {
                WPRINT_APP_INFO( ( "Error - invalid hex character function: %s line: %u ", __FUNCTION__, __LINE__ ) );
            }
            wep_key[0].data[a] = (uint8_t)(( nibble1 << 4 ) | nibble2);
        }
    }
    else
    {
        wep_key[0].length = *wep_key_length;
        memcpy( wep_key[0].data, wep_key_data, *wep_key_length );
    }

    /* Calculate the size of each WEP key entry */
    wep_key_entry_size = (uint8_t) ( 2 + *wep_key_length );

    /* Duplicate WEP key 0 for keys 1 to 3 */
    wep_key = (wiced_wep_key_t*)((char*)wep_key + wep_key_entry_size);
    memcpy( wep_key, wep_key_output, wep_key_entry_size );
    wep_key->index = 1;

    wep_key = (wiced_wep_key_t*)((char*)wep_key + wep_key_entry_size);
    memcpy( wep_key, wep_key_output, wep_key_entry_size );
    wep_key->index = 2;

    wep_key = (wiced_wep_key_t*)((char*)wep_key + wep_key_entry_size);
    memcpy( wep_key, wep_key_output, wep_key_entry_size );
    wep_key->index = 3;

    *wep_key_length = (uint8_t) ( 4 * wep_key_entry_size );
}

wiced_bool_t IP_NETWORK_IS_INITED( wiced_interface_t interface )
{
    return (ip_networking_inited[( ( uint8_t ) interface)&3]);
}
