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
#pragma once

#include <stdint.h>

#include "platform_toolchain.h"

#include "wwd_buffer.h"
#include "wwd_constants.h"

#include "wiced_constants.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************
 *                    Callback types
 ******************************************************/

typedef enum
{
    WICED_DEEP_SLEEP_EVENT_ENTER,
    WICED_DEEP_SLEEP_EVENT_CANCEL,
    WICED_DEEP_SLEEP_EVENT_LEAVE,
    WICED_DEEP_SLEEP_EVENT_WLAN_RESUME
} wiced_deep_sleep_event_type_t;

typedef void( *wiced_deep_sleep_event_handler_t )( wiced_deep_sleep_event_type_t event );

/******************************************************
 *                 Platform definitions
 ******************************************************/

#ifdef PLATFORM_DEEP_SLEEP
#define PLATFORM_DEEP_SLEEP_HEADER_INCLUDED
#include "platform_deep_sleep.h"
#endif /* PLATFORM_DEEP_SLEEP */

/******************************************************
 *                      Macros
 ******************************************************/

#ifndef WICED_DEEP_SLEEP_SAVED_VAR
#define WICED_DEEP_SLEEP_SAVED_VAR( var )                                               var
#endif

#ifndef WICED_DEEP_SLEEP_EVENT_HANDLER
#ifdef  __IAR_SYSTEMS_ICC__
#define IAR_ROOT_FUNC __root
#else
#define IAR_ROOT_FUNC
#endif /* __IAR_SYSTEMS_ICC__ */
#define WICED_DEEP_SLEEP_EVENT_HANDLER( func_name ) \
    static IAR_ROOT_FUNC void MAY_BE_UNUSED func_name( wiced_deep_sleep_event_type_t event )
#endif /* ndef WICED_DEEP_SLEEP_EVENT_HANDLER */

#ifndef WICED_DEEP_SLEEP_CALL_EVENT_HANDLERS
#define WICED_DEEP_SLEEP_CALL_EVENT_HANDLERS( cond, event )
#endif

#ifndef WICED_DEEP_SLEEP_IS_WARMBOOT
#define WICED_DEEP_SLEEP_IS_WARMBOOT( )                                                 0
#endif

#ifndef WICED_DEEP_SLEEP_IS_ENABLED
#define WICED_DEEP_SLEEP_IS_ENABLED( )                                                  0
#endif

#ifndef WICED_DEEP_SLEEP_IS_WARMBOOT_HANDLE
#define WICED_DEEP_SLEEP_IS_WARMBOOT_HANDLE( )                                          ( WICED_DEEP_SLEEP_IS_ENABLED( ) && WICED_DEEP_SLEEP_IS_WARMBOOT( ) )
#endif

#ifndef WICED_DEEP_SLEEP_SAVE_PACKETS_NUM
#define WICED_DEEP_SLEEP_SAVE_PACKETS_NUM                                               0
#endif

/******************************************************
 *                    Constants
 ******************************************************/

/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/

/******************************************************
 *                 Global Variables
 ******************************************************/

/******************************************************
 *               Function Declarations
 ******************************************************/

/** Return the time spent during deep sleep.
 *
 * @note Currently this is implemented for 4390x platforms only.
 *
 * @return time in system ticks
 */
uint32_t     wiced_deep_sleep_ticks_since_enter( void );

/** Save packets before going into deep sleep.
 *
 * @note Currently this is implemented for 4390x platforms only.
 *
 * @param[in] buffer             : Pointer to the packet buffer to be saved.
 * @param[in] interface          : The network interface (AP or STA) to which the specified packet belongs.
 *
 * @return WICED_TRUE if the packet buffer is successfully saved, otherwise WICED_FALSE *
 */
wiced_bool_t wiced_deep_sleep_save_packet( wiced_buffer_t buffer, wwd_interface_t interface );

/** Notify application that network interface is ready and push all saved packets up to stack.
 *
 * @note Currently this is implemented for 4390x platforms only.
 *
 * @return void
 */
void         wiced_deep_sleep_set_networking_ready( void );

/** Check whether there are packets pending before going to deep sleep
 *
 * @note Currently this is implemented for 4390x platforms only.
 *
 * @param[in] interface : The network interface (AP or STA) to be checked for pending packets
 *
 * @return WICED_FALSE if any packets pending, otherwise WICED_TRUE *
 */
wiced_bool_t wiced_deep_sleep_is_networking_idle( wiced_interface_t interface );

#ifdef __cplusplus
} /* extern "C" */
#endif
