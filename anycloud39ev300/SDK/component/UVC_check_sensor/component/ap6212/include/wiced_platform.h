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

/** @file
 *  Defines functions that access platform specific peripherals
 *
 */

#pragma once

//wmj-
//#include "wiced_result.h"
#include "wiced_utilities.h"
#include "wwd_constants.h"
#include "platform_peripheral.h"
#include "platform.h" /* This file is unique for each platform */
#include "platform_dct.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************
 * @cond                Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

#define WICED_I2C_START_FLAG                    (1U << 0)
#define WICED_I2C_REPEATED_START_FLAG           (1U << 1)
#define WICED_I2C_STOP_FLAG                     (1U << 2)

#define WICED_GPIO_NONE ((wiced_gpio_t)0x7fffffff)

/******************************************************
 *                   Enumerations
 ******************************************************/

typedef enum
{
    WICED_ACTIVE_LOW   = 0,
    WICED_ACTIVE_HIGH  = 1,
} wiced_active_state_t;

typedef enum
{
    WICED_LED_OFF   = 0,        /**< OFF    */
    WICED_LED_ON,               /**< ON     */
} wiced_led_state_t;

typedef enum
{
    WICED_LED_INDEX_1 = 0,
    WICED_LED_INDEX_2,
    WICED_LED_INDEX_3,
    WICED_LED_INDEX_4,

    WICED_LED_INDEX_MAX /* denotes end of list - do not use */
} wiced_led_index_t;
/******************************************************
 *                 Type Definitions
 ******************************************************/

typedef platform_pin_config_t                   wiced_gpio_config_t;

typedef platform_gpio_irq_trigger_t             wiced_gpio_irq_trigger_t;

typedef platform_gpio_irq_callback_t            wiced_gpio_irq_handler_t;

typedef platform_uart_config_t                  wiced_uart_config_t;

typedef platform_i2c_bus_address_width_t        wiced_i2c_bus_address_width_t;

typedef platform_i2c_speed_mode_t               wiced_i2c_speed_mode_t;

typedef platform_i2c_message_t                  wiced_i2c_message_t;

typedef platform_spi_message_segment_t          wiced_spi_message_segment_t;

typedef platform_rtc_time_t                     wiced_rtc_time_t;

typedef platform_spi_slave_config_t             wiced_spi_slave_config_t;

typedef platform_spi_slave_transfer_direction_t wiced_spi_slave_transfer_direction_t;

typedef platform_spi_slave_transfer_status_t    wiced_spi_slave_transfer_status_t;

typedef platform_spi_slave_command_t            wiced_spi_slave_command_t;

typedef platform_spi_slave_data_buffer_t        wiced_spi_slave_data_buffer_t;

/******************************************************
 *                    Structures
 ******************************************************/

/**
 * Specifies details of an external I2C slave device which is connected to the WICED system
 */
typedef struct
{
    wiced_i2c_t                   port;          /**< Which I2C peripheral of the platform to use for the I2C device being specified */
    uint16_t                      address;       /**< the address of the device on the I2C bus */
    wiced_i2c_bus_address_width_t address_width; /**< Indicates the number of bits that the slave device uses for addressing */
    uint8_t                       flags;         /**< Flags that change the mode of operation for the I2C port See WICED/platform/include/platform_peripheral.h  I2C flags constants */
    wiced_i2c_speed_mode_t        speed_mode;    /**< speed mode the device operates in */
} wiced_i2c_device_t;


/**
 * Specifies details of an external SPI slave device which is connected to the WICED system
 */
typedef struct
{
    wiced_spi_t                   port;          /**< Which SPI peripheral of the platform to use for the SPI device being specified */
    wiced_gpio_t                  chip_select;   /**< Which hardware pin to use for Chip Select of the SPI device being specified */
    uint32_t                      speed;         /**< SPI device access speed in Hertz */
    uint8_t                       mode;          /**< Mode of operation for SPI port See WICED/platform/include/platform_peripheral.h  SPI mode constants */
    uint8_t                       bits;          /**< Number of data bits - usually 8, 16 or 32 */
} wiced_spi_device_t;

/******************************************************
 *                     Variables
 ******************************************************/

#ifdef WICED_PLATFORM_INCLUDES_SPI_FLASH
extern const wiced_spi_device_t wiced_spi_flash;
#endif

/******************************************************
 * @endcond           Function Declarations
 ******************************************************/

/*****************************************************************************/
/** @defgroup platform       Platform functions
 *
 *  WICED hardware platform functions
 */
/*****************************************************************************/

/*****************************************************************************/
/** @addtogroup uart       UART
 *  @ingroup platform
 *
 * Universal Asynchronous Receiver Transmitter (UART) Functions
 *
 *
 *  @{
 */
/*****************************************************************************/

/** Initialises a UART interface
 *
 * Prepares an UART hardware interface for communications
 *
 * @param[in]  uart               : The interface which should be initialised
 * @param[in]  config             : UART configuration structure
 * @param[in]  optional_rx_buffer : Pointer to an optional RX ring buffer
 *
 * @return    WICED_SUCCESS : on success.
 * @return    WICED_ERROR   : if an error occurred with any step
 */
wiced_result_t wiced_uart_init( wiced_uart_t uart, const wiced_uart_config_t* config, wiced_ring_buffer_t* optional_rx_buffer );


/** Deinitialises a UART interface
 *
 * @param[in]  uart : The interface which should be deinitialised
 *
 * @return    WICED_SUCCESS : on success.
 * @return    WICED_ERROR   : if an error occurred with any step
 */
wiced_result_t wiced_uart_deinit( wiced_uart_t uart );


/** Transmit data on a UART interface
 *
 * @param[in]  uart     : The UART interface
 * @param[in]  data     : Pointer to the start of data
 * @param[in]  size     : Number of bytes to transmit
 *
 * @return    WICED_SUCCESS : on success.
 * @return    WICED_ERROR   : if an error occurred with any step
 */
wiced_result_t wiced_uart_transmit_bytes( wiced_uart_t uart, const void* data, uint32_t size );


/** Receive data on a UART interface
 *
 * @param[in]     uart    : The UART interface
 * @param[out]    data    : Pointer to the buffer which will store incoming data
 * @param[in,out] size    : Number of bytes to receive, function return in same parameter number of actually received bytes
 * @param[in]     timeout : Timeout in milliseconds
 *
 * @return    WICED_SUCCESS : on success.
 * @return    WICED_ERROR   : if an error occurred with any step
 */
wiced_result_t wiced_uart_receive_bytes( wiced_uart_t uart, void* data, uint32_t* size, uint32_t timeout );

/** @} */
/*****************************************************************************/
/** @addtogroup spi       SPI
 *  @ingroup platform
 *
 * Serial Peripheral Interface (SPI) Functions
 *
 *
 *  @{
 */
/*****************************************************************************/

/** Initialises the SPI interface for a given SPI device
 *
 * Prepares a SPI hardware interface for communication as a master
 *
 * @param[in]  spi : The SPI device to be initialised
 *
 * @return    WICED_SUCCESS : on success.
 * @return    WICED_ERROR   : if the SPI device could not be initialised
 */
wiced_result_t wiced_spi_init( const wiced_spi_device_t* spi );


/** Transmits data to a SPI device
 *
 * @param[in]  spi                : The SPI device to be initialised
 * @param[in]  segments           : A pointer to an array of segments
 * @param[in]  number_of_segments : The number of segments to transfer
 *
 * @return    WICED_SUCCESS : on success.
 * @return    WICED_ERROR   : if an error occurred
 */
wiced_result_t wiced_spi_transmit( const wiced_spi_device_t* spi, const wiced_spi_message_segment_t* segments, uint16_t number_of_segments );


/** Transmits and/or receives data from a SPI device
 *
 * @param[in]  spi                : The SPI device to be initialised
 * @param[in]  segments           : A pointer to an array of segments
 * @param[in]  number_of_segments : The number of segments to transfer
 *
 * @return    WICED_SUCCESS : on success.
 * @return    WICED_ERROR   : if an error occurred
 */
wiced_result_t wiced_spi_transfer( const wiced_spi_device_t* spi, const wiced_spi_message_segment_t* segments, uint16_t number_of_segments );


/** De-initialises a SPI interface
 *
 * Turns off a SPI hardware interface
 *
 * @param[in]  spi : The SPI device to be de-initialised
 *
 * @return    WICED_SUCCESS : on success.
 * @return    WICED_ERROR   : if an error occurred
 */
wiced_result_t wiced_spi_deinit( const wiced_spi_device_t* spi );


/** Initialises a SPI slave interface
 *
 * @param[in]  spi    : The SPI slave interface to be initialised
 * @param[in]  config : SPI slave configuration
 *
 * @return @ref wiced_result_t
 */
wiced_result_t wiced_spi_slave_init( wiced_spi_t spi, const wiced_spi_slave_config_t* config );


/** De-initialises a SPI slave interface
 *
 * @param[in]  spi : The SPI slave interface to be de-initialised
 *
 * @return @ref wiced_result_t
 */
wiced_result_t wiced_spi_slave_deinit( wiced_spi_t spi );


/** Receive command from the remote SPI master
 *
 * @param[in]   spi         : The SPI slave interface
 * @param[out]  command     : Pointer to the variable which will contained the received command
 * @param[in]   timeout_ms  : Timeout in milliseconds
 *
 * @return @ref wiced_result_t
 */
wiced_result_t wiced_spi_slave_receive_command( wiced_spi_t spi, wiced_spi_slave_command_t* command, uint32_t timeout_ms );


/** Transfer data to/from the remote SPI master
 *
 * @param[in]  spi         : The SPI slave interface
 * @param[in]  direction   : Transfer direction
 * @param[in]  buffer      : The buffer which contain the data to transfer
 * @param[in]  timeout_ms  : timeout in milliseconds
 *
 * @return @ref wiced_result_t
 */
wiced_result_t wiced_spi_slave_transfer_data( wiced_spi_t spi, wiced_spi_slave_transfer_direction_t direction, wiced_spi_slave_data_buffer_t* buffer, uint32_t timeout_ms );


/** Send an error status over the SPI slave interface
 *
 * @param[in]  spi          : The SPI slave interface
 * @param[in]  error_status : SPI slave error status
 *
 * @return @ref wiced_result_t
 */
wiced_result_t wiced_spi_slave_send_error_status( wiced_spi_t spi, wiced_spi_slave_transfer_status_t error_status );


/** Generate an interrupt on the SPI slave interface
 *
 * @param[in]  spi               : The SPI slave interface
 * @param[in]  pulse_duration_ms : Interrupt pulse duration in milliseconds
 *
 * @return @ref wiced_result_t
 */
wiced_result_t wiced_spi_slave_generate_interrupt( wiced_spi_t spi, uint32_t pulse_duration_ms );


/** @} */
/*****************************************************************************/
/** @addtogroup i2c       I2C
 *  @ingroup platform
 *
 * Inter-IC bus (I2C) Functions
 *
 *
 *  @{
 */
/*****************************************************************************/

/** Initialises an I2C interface
 *
 * Prepares an I2C hardware interface for communication as a master
 *
 * @param[in]  device : The device for which the i2c port should be initialised
 *
 * @return    WICED_SUCCESS : on success.
 * @return    WICED_ERROR   : if an error occurred during initialisation
 */
wiced_result_t wiced_i2c_init( const wiced_i2c_device_t* device );


/** Checks whether the device is available on a bus or not
 *
 * @param[in]  device  : The i2c device to be probed
 * @param[in]  retries : The number of times to attempt to probe the device
 *
 * @return    WICED_TRUE : device is found.
 * @return    WICED_FALSE: device is not found
 */
wiced_bool_t wiced_i2c_probe_device( const wiced_i2c_device_t* device, int retries );


/** Initialize the wiced_i2c_message_t structure for i2c tx transaction
 *
 * @param[out] message          : Pointer to a message structure, this should be a valid pointer
 * @param[in]  tx_buffer        : Pointer to a tx buffer that is already allocated
 * @param[in]  tx_buffer_length : Number of bytes to transmit
 * @param[in]  retries          : The number of times to attempt send a message in case it can't not be sent
 * @param[in]  disable_dma      : If true, disables the dma for current tx transaction. You may find it useful to switch off dma for short tx messages.
 *                     If you set this flag to 0, then you should make sure that the device flags was configured with I2C_DEVICE_USE_DMA. If the
 *                     device doesn't support DMA, the message will be transmitted with no DMA.
 *
 * @return    WICED_SUCCESS : message structure was initialised properly.
 * @return    WICED_BADARG  : one of the arguments is given incorrectly
 */
wiced_result_t wiced_i2c_init_tx_message( wiced_i2c_message_t* message, const void* tx_buffer, uint16_t tx_buffer_length, uint16_t retries, wiced_bool_t disable_dma );


/** Initialize the wiced_i2c_message_t structure for i2c rx transaction
 *
 * @param[out] message          : Pointer to a message structure, this should be a valid pointer
 * @param[in]  rx_buffer        : Pointer to an rx buffer that is already allocated
 * @param[in]  rx_buffer_length : Number of bytes to receive
 * @param[in]  retries          : The number of times to attempt receive a message in case device doesnt respond
 * @param[in]  disable_dma      : If true, disables the dma for current rx transaction. You may find it useful to switch off dma for short rx messages.
 *                     If you set this flag to 0, then you should make sure that the device flags was configured with I2C_DEVICE_USE_DMA. If the
 *                     device doesn't support DMA, the message will be received not using DMA.
 *
 * @return    WICED_SUCCESS : message structure was initialised properly.
 * @return    WICED_BADARG  : one of the arguments is given incorrectly
 */
wiced_result_t wiced_i2c_init_rx_message( wiced_i2c_message_t* message, void* rx_buffer, uint16_t rx_buffer_length, uint16_t retries, wiced_bool_t disable_dma );


/** Initialize the wiced_i2c_message_t structure for i2c combined transaction
 *
 * @param[out]  message         : Pointer to a message structure, this should be a valid pointer
 * @param[in]  tx_buffer        : Pointer to a tx buffer that is already allocated
 * @param[in]  rx_buffer        : Pointer to an rx buffer that is already allocated
 * @param[in]  tx_buffer_length : Number of bytes to transmit
 * @param[in]  rx_buffer_length : Number of bytes to receive
 * @param[in]  retries          : The number of times to attempt receive a message in case device doesnt respond
 * @param[in]  disable_dma      : If true, disables the dma for current rx transaction. You may find it useful to switch off dma for short rx messages.
 *                     If you set this flag to 0, then you should make sure that the device flags was configured with I2C_DEVICE_USE_DMA. If the
 *                     device doesn't support DMA, the message will be received not using DMA.
 *
 * @return    WICED_SUCCESS : message structure was initialised properly.
 * @return    WICED_BADARG  : one of the arguments is given incorrectly
 */
wiced_result_t wiced_i2c_init_combined_message( wiced_i2c_message_t* message, const void* tx_buffer, void* rx_buffer, uint16_t tx_buffer_length, uint16_t rx_buffer_length, uint16_t retries, wiced_bool_t disable_dma );


/** Transmits and/or receives data over an I2C interface
 *
 * @param[in]  device             : The i2c device to communicate with
 * @param[in]  message            : A pointer to a message (or an array of messages) to be transmitted/received
 * @param[in]  number_of_messages : The number of messages to transfer. [1 .. N] messages
 *
 * @return    WICED_SUCCESS : on success.
 * @return    WICED_ERROR   : if an error occurred during message transfer
 */
wiced_result_t wiced_i2c_transfer(  const wiced_i2c_device_t* device, wiced_i2c_message_t* message, uint16_t number_of_messages );


/** Read data over an I2C interface
 *
 * @param[in]  device             : The i2c device to communicate with
 * @param[in]  flags              : Bitwise flags to control i2c data transfers (WICED_I2C_XXX_FLAG)
 * @param[out] buffer             : Pointer to a buffer to hold received data
 * @param[in]  buffer_length      : Length in bytes of the buffer
 *
 * @return    WICED_SUCCESS : on success
 * @return    WICED_ERROR   : if an error occurred during message transfer
 */
wiced_result_t wiced_i2c_read( const wiced_i2c_device_t* device, uint16_t flags, void* buffer, uint16_t buffer_length );


/** Write data over an I2C interface
 *
 * @param[in] device             : The i2c device to communicate with
 * @param[in] flags              : Bitwise flags to control i2c data transfers (WICED_I2C_XXX_FLAG)
 * @param[in] buffer             : Pointer to a buffer with data to write
 * @param[in] buffer_length      : Length in bytes of the buffer
 *
 * @return    WICED_SUCCESS : on success
 * @return    WICED_ERROR   : if an error occurred during message transfer
 */
wiced_result_t wiced_i2c_write( const wiced_i2c_device_t* device, uint16_t flags, const void* buffer, uint16_t buffer_length );


/** Deinitialises an I2C device
 *
 * @param[in]  device : The device for which the i2c port should be deinitialised
 *
 * @return    WICED_SUCCESS : on success.
 * @return    WICED_ERROR   : if an error occurred during deinitialisation
 */
wiced_result_t wiced_i2c_deinit( const wiced_i2c_device_t* device );

/** @} */
/*****************************************************************************/
/** @addtogroup adc       ADC
 *  @ingroup platform
 *
 * Analog to Digital Converter (ADC) Functions
 *
 *
 *  @{
 */
/*****************************************************************************/

/** Initialises an ADC interface
 *
 * Prepares an ADC hardware interface for sampling
 *
 * @param[in] adc            : The interface which should be initialised
 * @param[in] sampling_cycle : Sampling period in number of ADC clock cycles. If the
 *                         MCU does not support the value provided, the closest
 *                         supported value is used.
 *
 * @return    WICED_SUCCESS : on success.
 * @return    WICED_ERROR   : if an error occurred with any step
 */
wiced_result_t wiced_adc_init( wiced_adc_t adc, uint32_t sampling_cycle );


/** Takes a single sample from an ADC interface
 *
 * Takes a single sample from an ADC interface
 *
 * @param[in]  adc    : The interface which should be sampled
 * @param[out] output : Pointer to a variable which will receive the sample
 *
 * @return    WICED_SUCCESS : on success.
 * @return    WICED_ERROR   : if an error occurred with any step
 */
wiced_result_t wiced_adc_take_sample( wiced_adc_t adc, uint16_t* output );


/** Takes multiple samples from an ADC interface
 *
 * Takes multiple samples from an ADC interface and stores them in
 * a memory buffer
 *
 * @param[in]  adc           : The interface which should be sampled
 * @param[out] buffer        : A memory buffer which will receive the samples
 *                             Each sample will be uint16_t little endian.
 * @param[in]  buffer_length : Length in bytes of the memory buffer.
 *
 *
 * @return    WICED_SUCCESS : on success.
 * @return    WICED_ERROR   : if an error occurred with any step
 */
wiced_result_t wiced_adc_take_sample_stream( wiced_adc_t adc, void* buffer, uint16_t buffer_length );


/** De-initialises an ADC interface
 *
 * Turns off an ADC hardware interface
 *
 * @param[in]  adc : The interface which should be de-initialised
 *
 * @return    WICED_SUCCESS : on success.
 * @return    WICED_ERROR   : if an error occurred with any step
 */
wiced_result_t wiced_adc_deinit( wiced_adc_t adc );

/** @} */
/*****************************************************************************/
/** @addtogroup gpio       GPIO
 *  @ingroup platform
 *
 * General Purpose Input/Output pin (GPIO) Functions
 *
 *
 *  @{
 */
/*****************************************************************************/

/** Initialises a GPIO pin
 *
 * Prepares a GPIO pin for use.
 *
 * @param[in] gpio          : The gpio pin which should be initialised
 * @param[in] configuration : A structure containing the required
 *                            gpio configuration
 *
 * @return    WICED_SUCCESS : on success.
 * @return    WICED_ERROR   : if an error occurred with any step
 */
wiced_result_t wiced_gpio_init( wiced_gpio_t gpio, wiced_gpio_config_t configuration );

/** De-initialises a GPIO pin
 *
 * Clears a GPIO pin from use.
 *
 * @param[in] gpio          : The gpio pin which should be de-initialised
 *
 * @return    WICED_SUCCESS : on success.
 * @return    WICED_ERROR   : if an error occurred with any step
 */
wiced_result_t wiced_gpio_deinit( wiced_gpio_t gpio );

/** Sets an output GPIO pin high
 *
 * Sets an output GPIO pin high. Using this function on a
 * gpio pin which is set to input mode is undefined.
 *
 * @param[in] gpio          : The gpio pin which should be set high
 *
 * @return    WICED_SUCCESS : on success.
 * @return    WICED_ERROR   : if an error occurred with any step
 */
wiced_result_t wiced_gpio_output_high( wiced_gpio_t gpio );


/** Sets an output GPIO pin low
 *
 * Sets an output GPIO pin low. Using this function on a
 * gpio pin which is set to input mode is undefined.
 *
 * @param[in] gpio          : The gpio pin which should be set low
 *
 * @return    WICED_SUCCESS : on success.
 * @return    WICED_ERROR   : if an error occurred with any step
 */
wiced_result_t wiced_gpio_output_low( wiced_gpio_t gpio );


/** Get the state of an input GPIO pin
 *
 * Get the state of an input GPIO pin. Using this function on a
 * gpio pin which is set to output mode will return an undefined value.
 *
 * @param[in] gpio          : The gpio pin which should be read
 *
 * @return    WICED_TRUE  : if high
 * @return    WICED_FALSE : if low
 */
wiced_bool_t   wiced_gpio_input_get( wiced_gpio_t gpio );


/** Enables an interrupt trigger for an input GPIO pin
 *
 * Enables an interrupt trigger for an input GPIO pin.
 * Using this function on an uninitialized gpio pin or
 * a gpio pin which is set to output mode is undefined.
 *
 * @param[in] gpio    : The gpio pin which will provide the interrupt trigger
 * @param[in] trigger : The type of trigger (rising/falling edge, high/low level)
 * @param[in] handler : A function pointer to the interrupt handler
 * @param[in] arg     : An argument that will be passed to the
 *                      interrupt handler
 *
 * @return    WICED_SUCCESS : on success.
 * @return    WICED_ERROR   : if an error occurred with any step
 */
wiced_result_t wiced_gpio_input_irq_enable( wiced_gpio_t gpio, wiced_gpio_irq_trigger_t trigger, wiced_gpio_irq_handler_t handler, void* arg );


/** Disables an interrupt trigger for an input GPIO pin
 *
 * Disables an interrupt trigger for an input GPIO pin.
 * Using this function on a gpio pin which has not been set up
 * using @ref wiced_gpio_input_irq_enable is undefined.
 *
 * @param[in] gpio    : The gpio pin which provided the interrupt trigger
 *
 * @return    WICED_SUCCESS : on success.
 * @return    WICED_ERROR   : if an error occurred with any step
 */
wiced_result_t wiced_gpio_input_irq_disable( wiced_gpio_t gpio );

/**
 * Set status of an LED (Off or On)
 *
 * NOTES: LED initialization is done in platform_init_external_devices() in each platform.c file
 *        Platform code knows the polarity of the LEDs and sets the output high/low as appropriate.
 *        If there are no LEDs on a platform, provide a dummy function in platform.c.
 *
 * @param[in]   led_index  : wiced_led_t LED
 * @param[in]   off_on     : WICED_LED_OFF or WICED_LED_ON
 *
 * @return  WICED_SUCCESS
 *          WICED_BADARG
 */
wiced_result_t wiced_led_set_state(wiced_led_index_t led_index, wiced_led_state_t off_on );

/** @} */
/*****************************************************************************/
/** @addtogroup pwm       PWM
 *  @ingroup platform
 *
 * Pulse-Width Modulation (PWM) Functions
 *
 *
 *  @{
 */
/*****************************************************************************/

/** Initialises a PWM pin
 *
 * Prepares a Pulse-Width Modulation pin for use.
 * Does not start the PWM output (use @ref wiced_pwm_start).
 *
 * @param[in] pwm        : The PWM interface which should be initialised
 * @param[in] frequency  : Output signal frequency in Hertz
 * @param[in] duty_cycle : Duty cycle of signal as a floating-point percentage (0.0 to 100.0)
 *
 * @return    WICED_SUCCESS : on success.
 * @return    WICED_ERROR   : if an error occurred with any step
 */
wiced_result_t wiced_pwm_init( wiced_pwm_t pwm, uint32_t frequency, float duty_cycle );


/** Starts PWM output on a PWM interface
 *
 * Starts Pulse-Width Modulation signal output on a PWM pin
 *
 * @param[in] pwm        : The PWM interface which should be started
 *
 * @return    WICED_SUCCESS : on success.
 * @return    WICED_ERROR   : if an error occurred with any step
 */
wiced_result_t wiced_pwm_start( wiced_pwm_t pwm );


/** Stops output on a PWM pin
 *
 * Stops Pulse-Width Modulation signal output on a PWM pin
 *
 * @param[in] pwm        : The PWM interface which should be stopped
 *
 * @return    WICED_SUCCESS : on success.
 * @return    WICED_ERROR   : if an error occurred with any step
 */
wiced_result_t wiced_pwm_stop( wiced_pwm_t pwm );

/** @} */
/*****************************************************************************/
/** @addtogroup watchdog       Watchdog
 *  @ingroup platform
 *
 * Watchdog Timer Functions
 *
 *
 *  @{
 */
/*****************************************************************************/

/** Kick the system watchdog.
 *
 * Resets (kicks) the timer of the system watchdog.
 * This MUST be done done by the App regularly.
 *
 * @return    WICED_SUCCESS : on success.
 * @return    WICED_ERROR   : if an error occurred with any step
 */
wiced_result_t wiced_watchdog_kick( void );

/** @} */
/*****************************************************************************/
/** @addtogroup mcupowersave       Powersave
 *  @ingroup platform
 *
 * MCU Powersave Functions
 *
 *
 *  @{
 */
/*****************************************************************************/

/** Enables the MCU to enter powersave mode.
 *
 * @warning   If the MCU drives the sleep clock input pin of the WLAN chip,   \n
 *            ensure the 32kHz clock output from the MCU is maintained while  \n
 *            the MCU is in powersave mode. The WLAN sleep clock reference is \n
 *            typically configured in the file:                               \n
 *            <WICED-SDK>/include/platforms/<PLATFORM_NAME>/platform.h
 * @return    void
 */
void wiced_platform_mcu_enable_powersave( void );


/** Stops the MCU entering powersave mode.
 *
 * @return    void
 */
void wiced_platform_mcu_disable_powersave( void );

/** @} */

/**
 * This function will return the value of time read from the on board CPU real time clock. Time value must be given in the format of
 * the structure wiced_rtc_time_t
 *
 * @param[out] time        : Pointer to a time structure
 *
 * @return    WICED_SUCCESS : on success.
 * @return    WICED_ERROR   : if an error occurred with any step
 */
wiced_result_t wiced_platform_get_rtc_time( wiced_rtc_time_t* time );


/**
 * This function will set MCU RTC time to a new value. Time value must be given in the format of
 * the structure wiced_rtc_time_t
 *
 * @param[in] time        : Pointer to a time structure
 *
 * @return    WICED_SUCCESS : on success.
 * @return    WICED_ERROR   : if an error occurred with any step
 */
wiced_result_t wiced_platform_set_rtc_time( const wiced_rtc_time_t* time );


/**
 * Enable the 802.1AS time functionality.
 *
 * @return    WICED_SUCCESS : on success.
 * @return    WICED_ERROR   : if an error occurred with any step
 */
wiced_result_t wiced_time_enable_8021as(void);


/**
 * Disable the 802.1AS time functionality.
 *
 * @return    WICED_SUCCESS : on success.
 * @return    WICED_ERROR   : if an error occurred with any step
 */
wiced_result_t wiced_time_disable_8021as(void);


/**
 * Read the 802.1AS time.
 *
 * Retrieve the origin timestamp in the last sync message, correct for the
 * intervening interval and return the corrected (master) time in seconds + nanoseconds.
 *
 * @note If no sync messages have been received, master time and local time will be the same.
 *
 * @param[out] master_secs     : The master time seconds value.
 * @param[out] master_nanosecs : The master time nanoseconds value.
 * @param[out] local_secs      : The local time seconds value.
 * @param[out] local_nanosecs  : The local time nanoseconds value.
 *
 * @return    WICED_SUCCESS : on success.
 * @return    WICED_ERROR   : if an error occurred with any step
 */
wiced_result_t wiced_time_read_8021as(uint32_t *master_secs, uint32_t *master_nanosecs,
                                      uint32_t *local_secs, uint32_t *local_nanosecs);


/**
 * Read the 802.1AS time along with the I2S-driven audio time
 *
 * Retrieve the origin timestamp in the last sync message, correct for the
 * intervening interval and return the corrected time in seconds + nanoseconds.
 * Also retrieve the corresponding time from the audio timer.
 *
 * @note If no sync messages have been received, master time and local time will be the same.
 *
 * @param[out] master_secs     : The master time seconds value.
 * @param[out] master_nanosecs : The master time nanoseconds value.
 * @param[out] local_secs      : The local time seconds value.
 * @param[out] local_nanosecs  : The local time nanoseconds value.
 * @param[out[ audio_time_hi   : The high I2S audio timer value from the ASCU block.
 * @param[out[ audio_time_lo   : The low I2S audio timer value from the ASCU block.
 *
 * @return    WICED_SUCCESS : on success.
 * @return    WICED_ERROR   : if an error occurred with any step
 */
wiced_result_t wiced_time_read_8021as_with_audio(uint32_t *master_secs, uint32_t *master_nanosecs,
                                                 uint32_t *local_secs, uint32_t *local_nanosecs,
                                                 uint32_t *audio_time_hi, uint32_t *audio_time_lo);


/**
 * Enable audio timer
 *
 * @param[in] audio_frame_count : Audio timer interrupts period expressed in number of audio samples/frames
 *
 * @return    WICED_SUCCESS     : on success.
 * @return    WICED_ERROR       : if an error occurred with any step
 */
wiced_result_t wiced_audio_timer_enable        ( uint32_t audio_frame_count );


/**
 * Disable audio timer
 *
 * @return    WICED_SUCCESS     : on success.
 * @return    WICED_ERROR       : if an error occurred with any step
 */
wiced_result_t wiced_audio_timer_disable       ( void );


/**
 * Wait for audio timer frame sync event
 *
 * @param[in]  timeout_msecs     : Timeout value in msecs; WICED_NO_WAIT or WICED_WAIT_FOREVER otherwise.
 *
 * @return    WICED_SUCCESS     : on success.
 * @return    WICED_ERROR       : if an error occurred with any step
 */
wiced_result_t wiced_audio_timer_get_frame_sync( uint32_t timeout_msecs );


/**
 * Read audio timer value (tick count)
 *
 * @param[out]  time_hi           : Upper 32-bit of 64-bit audio timer ticks
 * @param[out]  time_lo           : Lower 32-bit of 64-bit audio timer ticks
 *
 * @return    WICED_SUCCESS     : on success.
 * @return    WICED_ERROR       : if an error occurred with any step
 */
wiced_result_t wiced_audio_timer_get_time      ( uint32_t *time_hi, uint32_t *time_lo );


/**
 * Get audio timer resolution (ticks per second)
 *
 * @param[in]   audio_sample_rate : Audio sample rate
 * @param[out]  ticks_per_sec     : Returned audio timer resolution
 *
 * @return    WICED_SUCCESS     : on success.
 * @return    WICED_ERROR       : if an error occurred with any step
 */
wiced_result_t wiced_audio_timer_get_resolution( uint32_t audio_sample_rate, uint32_t *ticks_per_sec );

#ifdef __cplusplus
} /*extern "C" */
#endif
