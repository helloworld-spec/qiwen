
/** @file
 * Defines WWD SDIO functions for AK3918E SDIOH
 */
//wmj-
//#include <typedefs.h>
//#include <osl.h>
#include <stdbool.h>
#include "platform/wwd_platform_interface.h"
#include "platform/wwd_bus_interface.h"
#include "internal/bus_protocols/wwd_bus_protocol_interface.h"
#include "chip_constants.h"

//#include "platform_sdio.h"
#include "wwd_constants.h"
#include "anyka_types.h"
#include "arch_mmc_sd.h"

/******************************************************
 *             Constants
 ******************************************************/

#ifndef WICED_DISABLE_MCU_POWERSAVE
#error "Not support WICED_DISABLE_MCU_POWERSAVE"
#endif /* ifndef  WICED_DISABLE_MCU_POWERSAVE */

#define WICED_USE_WIFI_RESET_PIN
#define WWD_PIN_WIFI_RESET        14
#define GPIO_LEVEL_LOW            0 
#define GPIO_LEVEL_HIGH           1





/******************************************************
 *             Structures
 ******************************************************/

/******************************************************
 *             Variables
 ******************************************************/
//wmj-
//extern sdioh_info_t *glob_sd;

/******************************************************
 *             Function declarations
 ******************************************************/
void sdio_client_check_isr( void* );

/******************************************************
 *             Function definitions
 ******************************************************/
void host_platform_power_wifi( wiced_bool_t power_enabled )
{
    UNUSED_PARAMETER( power_enabled );
}

void host_platform_reset_wifi( wiced_bool_t reset_asserted )
{
#if defined ( WICED_USE_WIFI_RESET_PIN )
		( reset_asserted == WICED_TRUE ) ? gpio_set_pin_level(WWD_PIN_WIFI_RESET, GPIO_LEVEL_LOW) : gpio_set_pin_level(WWD_PIN_WIFI_RESET, GPIO_LEVEL_HIGH);
#else
		UNUSED_PARAMETER( reset_asserted );
#endif

}

wwd_result_t host_platform_bus_init( void )
{
	int ret;
	
	ret = sdio_initial(USE_FOUR_BUS, SDIO_64B_BLOCK);
	
	if(ret == AK_FALSE){
		WPRINT_PLATFORM_ERROR(("sdio initial faild\n"));
		return WWD_SDIO_BUS_UP_FAIL;
	}
/*
	ret = sdio_read_cccr();
	if (ret != 0)
	{
		WPRINT_PLATFORM_ERROR("sdio_read_cccr faild");
		return WWD_SDIO_BUS_UP_FAIL;
	}

	sdio_read_common_cis();
	mmc_sdio_switch_hs(TRUE);
*/	
	
	return WWD_SUCCESS;
#if 0
    if ( platform_sdio_host_init( sdio_client_check_isr ) != WICED_SUCCESS )
    {
        WPRINT_WWD_ERROR(("SDIO Host init FAIL\n"));
        return WWD_SDIO_BUS_UP_FAIL;
    }
    else
    {
        return WWD_SUCCESS;
    }
#endif
}

wwd_result_t host_platform_sdio_enumerate( void )
{
    /* Select card already done in sdioh_attach */
    return WWD_SUCCESS;
}

wwd_result_t host_platform_bus_deinit( void )
{
    return WWD_SUCCESS;
}

//TODO wmj : this function may have some problems
wwd_result_t host_platform_sdio_transfer( wwd_bus_transfer_direction_t direction, sdio_command_t command, sdio_transfer_mode_t mode, sdio_block_size_t block_size, uint32_t argument, /*@null@*/ uint32_t* data, uint16_t data_size, sdio_response_needed_t response_expected, /*@out@*/ /*@null@*/ uint32_t* response )
{
	int ret;
	sdio_cmd_argument_t arg = { .value = argument };
	
	UNUSED_PARAMETER( response_expected );
    UNUSED_PARAMETER( block_size );
	UNUSED_PARAMETER( mode );
	
	if ( command == SDIO_CMD_53 )
    {
    	if(direction == BUS_WRITE)
        {
        	ret =  sdio_write_multi(arg.cmd53.function_number, arg.cmd53.register_address, data_size, arg.cmd53.op_code, data);
        }
		else
		{
			ret =  sdio_read_multi(arg.cmd53.function_number, arg.cmd53.register_address, data_size, arg.cmd53.op_code, data);
			if ( data_size == 2 || data_size == 4 )
    		{
    			if ( response != NULL )
	            {
	                *response = *data;
	            }
	            
			}
		}
    }
	else if( command == SDIO_CMD_52 )
	{
		
		if(direction == BUS_WRITE)
        {
        	ret = sdio_write_byte(arg.cmd52.function_number, arg.cmd52.register_address, arg.cmd52.write_data);
		}
        else
        {
        	ret = sdio_read_byte(arg.cmd52.function_number, arg.cmd52.register_address, response);
        }
	}
	else
	{
		return WWD_WLAN_SDIO_ERROR;
	}

	return(ret == true ? WWD_SUCCESS : WWD_WLAN_SDIO_ERROR);

#if 0
    wiced_result_t wiced_result;

    wiced_result = platform_sdio_host_transfer( direction == BUS_READ ? SDIO_READ : SDIO_WRITE,
                                                command, mode, block_size, argument, data, data_size, response_expected, response );

    return (wiced_result == WICED_SUCCESS ? WWD_SUCCESS : WWD_WLAN_SDIO_ERROR);
#endif
}

wwd_result_t host_platform_enable_high_speed_sdio( void )
{
	int ret;
	unsigned char speed;
	
	sdio_read_byte(BUS_FUNCTION, SDIOD_CCCR_SPEED_CONTROL, &speed);
	speed |= SDIO_SPEED_EHS;
	ret = sdio_write_byte(BUS_FUNCTION, SDIOD_CCCR_SPEED_CONTROL, speed);
	if(ret == true)
	{
		return WWD_SUCCESS;
	}
	else
	{
		return WWD_WLAN_SDIO_ERROR;
	}
		

#if 0
    platform_sdio_host_enable_high_speed();
    return WWD_SUCCESS;
#endif
}

wwd_result_t host_platform_bus_enable_interrupt( void )
{

#if 0
    platform_sdio_host_enable_interrupt();
    return  WWD_SUCCESS;
#endif 
}

wwd_result_t host_platform_bus_disable_interrupt( void )
{

#if 0
    platform_sdio_host_disable_interrupt();
    return  WWD_SUCCESS;
#endif	
}

#ifdef WICED_PLATFORM_MASKS_BUS_IRQ
wwd_result_t host_platform_unmask_sdio_interrupt( void )
{
    return host_platform_bus_enable_interrupt();
}
#endif

void host_platform_bus_buffer_freed( wwd_buffer_dir_t direction )
{
    UNUSED_PARAMETER( direction );
}

/* Client Interrupt handler */
void sdio_client_check_isr( void* user_data )
{
    UNUSED_PARAMETER( user_data );
    host_platform_bus_disable_interrupt();
    wwd_thread_notify_irq();
}
