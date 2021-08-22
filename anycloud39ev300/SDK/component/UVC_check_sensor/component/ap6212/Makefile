# Project name

PROJ = libwifi

TARGET = $(PROJ).a

TOPDIR 	?= $(shell pwd)

LIBDIR = ../../kernel/lib


# DEFINE an include 

INCLUDE = -Iinclude \
		  -I../../kernel/include \
		  -Inetwork/LwIP/WWD \
		  -Inetwork/LwIP/WWD/AKOS \
		  -Inetwork/LwIP/include \
		  -Inetwork/LwIP/include/ipv4 \
		  -Inetwork/LwIP/include/ipv4/lwip \
		  -Iplatform/GCC \
		  -Iplatform/MCU \
		  -Iplatform/sky39ev200 \
		  -Iplatform/include \
		  -IRTOS \
		  -IRTOS/akos \
		  -IRTOS/akos/include \
		  -Iutilities/TLV \
		  -IWWD \
		  -IWWD/include \
		  -IWWD/include/RTOS \
		  -IWWD/include/network \
		  -IWWD/include/platform \
		  -IWWD/internal \
		  -IWWD/internal/bus_protocols \
		  -IWWD/internal/bus_protocols/SDIO \
          -IWWD/internal/chips \
		  -IWWD/internal/chips/4343x 

# BCM943438WCD1
WLAN_CHIP            := 43438
WLAN_CHIP_REVISION   := A1
WLAN_CHIP_FAMILY     := 4343x

PLATFORM_SUPPORTS_BUTTONS := 1

INTERNAL_MEMORY_RESOURCES = $(ALL_RESOURCES)

# Set the WIFI firmware 
WWD_DEFINES          += WWD_DIRECT_RESOURCES


DEFINES += $(addprefix -D,$(WWD_DEFINES))


#Locations
BUILDPATH  ?= build

SRCDIR += network/LwIP/WWD
SRCDIR += internal
SRCDIR += platform
SRCDIR += platform/MCU
SRCDIR += platform/MCU/AK3918E
SRCDIR += RTOS/akos
SRCDIR += utilities/TLV
SRCDIR += WWD/internal
SRCDIR += WWD/internal/bus_protocols
SRCDIR += WWD/internal/bus_protocols/SDIO
SRCDIR += WWD/internal/chips
SRCDIR += WWD/internal/chips/4343x


CSRCS += $(foreach d, $(SRCDIR), $(wildcard $d/*.c))

COBJS += $(patsubst %.c, $(BUILDPATH)/%.o, $(CSRCS))



#build rule
.PHONY: all prepare target clean install 

all: prepare target install

prepare:
ifneq ($(BUILDPATH),)
	@for i in $(SRCDIR); \
	do mkdir -p $(BUILDPATH)/$$i; \
	done
endif


target: $(BUILDPATH)/$(TARGET)

$(BUILDPATH)/$(TARGET): $(COBJS) 
	@echo ---------------------[build out]----------------------------------	
	$(AR) -rsv $@  $(COBJS) 
	
install: 
	@if [ ! -e $(BUILDPATH)/$(TARGET) ];then\
		echo "\033[1;31m"$(TARGET) is not exist"\033[m";\
		/bin/false;	\
	fi
	@echo "\033[1;32m"install $(BUILDPATH)/$(TARGET) to $(LIBDIR)"\033[m"
	cp $(BUILDPATH)/$(TARGET) $(LIBDIR)

clean: 
	rm -rf $(BUILDPATH)



# Rules
include $(TOPDIR)/rules.mk

