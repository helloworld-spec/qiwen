#
# Copyright 2017, Cypress Semiconductor Corporation or a subsidiary of 
 # Cypress Semiconductor Corporation. All Rights Reserved.
 # This software, including source code, documentation and related
 # materials ("Software"), is owned by Cypress Semiconductor Corporation
 # or one of its subsidiaries ("Cypress") and is protected by and subject to
 # worldwide patent protection (United States and foreign),
 # United States copyright laws and international treaty provisions.
 # Therefore, you may use this Software only as provided in the license
 # agreement accompanying the software package from which you
 # obtained this Software ("EULA").
 # If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
 # non-transferable license to copy, modify, and compile the Software
 # source code solely for use in connection with Cypress's
 # integrated circuit products. Any reproduction, modification, translation,
 # compilation, or representation of this Software except as specified
 # above is prohibited without the express written permission of Cypress.
 #
 # Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
 # EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
 # WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
 # reserves the right to make changes to the Software without notice. Cypress
 # does not assume any liability arising out of the application or use of the
 # Software or any product or circuit described in the Software. Cypress does
 # not authorize its products for use in any products where a malfunction or
 # failure of the Cypress product may reasonably be expected to result in
 # significant property damage, injury or death ("High Risk Product"). By
 # including Cypress's product in a High Risk Product, the manufacturer
 # of such system or application assumes all risk of such use and in doing
 # so agrees to indemnify Cypress against all liability.
#

NAME := WWD_for_$(subst /,_,$(BUS))_$(RTOS)

GLOBAL_INCLUDES := . \
                   include \
                   include/network \
                   include/RTOS \
                   internal/bus_protocols/$(subst .,/,$(BUS)) \
                   internal/chips/$(WLAN_CHIP_FAMILY)


$(NAME)_SOURCES := internal/wwd_thread.c \
                   internal/wwd_ap_common.c \
                   internal/wwd_thread_internal.c \
                   internal/wwd_sdpcm.c \
                   internal/wwd_internal.c \
                   internal/wwd_management.c \
                   internal/wwd_wifi.c \
                   internal/wwd_logging.c \
                   internal/wwd_debug.c \
                   internal/wwd_eapol.c \
                   internal/bus_protocols/wwd_bus_common.c \
                   internal/bus_protocols/$(subst .,/,$(BUS))/wwd_bus_protocol.c
ifdef NO_WICED_API
$(NAME)_SOURCES += ../internal/wiced_crypto.c # Pull in wiced_crypto_get_random() implementation from WICED/internal
endif


$(NAME)_CHECK_HEADERS := \
                         internal/wwd_ap.h \
                         internal/wwd_ap_common.h \
                         internal/wwd_bcmendian.h \
                         internal/wwd_internal.h \
                         internal/wwd_logging.h \
                         internal/wwd_sdpcm.h \
                         internal/wwd_thread.h \
                         internal/wwd_thread_internal.h \
                         internal/bus_protocols/wwd_bus_protocol_interface.h \
                         internal/bus_protocols/$(subst .,/,$(BUS))/wwd_bus_protocol.h \
                         internal/chips/$(WLAN_CHIP_FAMILY)/chip_constants.h \
                         include/wwd_assert.h \
                         include/wwd_constants.h \
                         include/wwd_debug.h \
                         include/wwd_events.h \
                         include/wwd_management.h \
                         include/wwd_poll.h \
                         include/wwd_structures.h \
                         include/wwd_wifi.h \
                         include/wwd_wlioctl.h \
                         include/Network/wwd_buffer_interface.h \
                         include/Network/wwd_network_constants.h \
                         include/Network/wwd_network_interface.h \
                         include/platform/wwd_bus_interface.h \
                         include/platform/wwd_platform_interface.h \
                         include/platform/wwd_resource_interface.h \
                         include/platform/wwd_sdio_interface.h \
                         include/platform/wwd_spi_interface.h \
                         include/RTOS/wwd_rtos_interface.h


ifdef WICED_USE_WIFI_P2P_INTERFACE
GLOBAL_DEFINES += WICED_USE_WIFI_P2P_INTERFACE
ifeq ($(WLAN_CHIP),43362)
WLAN_CHIP_BIN_TYPE:=-p2p
else
WLAN_CHIP_BIN_TYPE:=
endif
else
WLAN_CHIP_BIN_TYPE:=
endif

ifndef NO_WIFI_FIRMWARE
WIFI_FIRMWARE_LOCATION ?= WIFI_FIRMWARE_IN_RESOURCES
ifeq ($(WIFI_FIRMWARE_LOCATION), WIFI_FIRMWARE_IN_RESOURCES)
$(NAME)_RESOURCES += firmware/$(WLAN_CHIP)/$(WLAN_CHIP)$(WLAN_CHIP_REVISION)$(WLAN_CHIP_BIN_TYPE).bin
endif
ifeq ($(WLAN_CHIP),43362)
GLOBAL_DEFINES += FIRMWARE_WITH_PMK_CALC_SUPPORT
endif
endif

ifeq ($(WLAN_CHIP_FAMILY),4343x)
ifeq ($(BUS),SPI)
GLOBAL_DEFINES += WWD_SPI_IRQ_FALLING_EDGE
endif
endif

ifeq ($(WLAN_CHIP_FAMILY),4334x)
ifeq ($(BUS),SPI)
GLOBAL_DEFINES += WWD_DISABLE_SAVE_RESTORE
endif
endif

ifeq ($(WLAN_CHIP),)
$(error ERROR: WLAN_CHIP must be defined in your platform makefile)
endif

ifeq ($(WLAN_CHIP_REVISION),)
$(error ERROR: WLAN_CHIP_REVISION must be defined in your platform makefile)
endif

ifeq ($(WLAN_CHIP_FAMILY),)
$(error ERROR: WLAN_CHIP_FAMILY must be defined in your platform makefile)
endif

ifeq ($(HOST_OPENOCD),)
$(error ERROR: HOST_OPENOCD must be defined in your platform makefile)
endif

$(NAME)_SOURCES += internal/chips/$(WLAN_CHIP_FAMILY)/wwd_ap.c \
                   internal/chips/$(WLAN_CHIP_FAMILY)/wwd_chip_specific_functions.c

$(NAME)_CFLAGS  = $(COMPILER_SPECIFIC_PEDANTIC_CFLAGS)

$(NAME)_COMPONENTS += utilities/TLV
