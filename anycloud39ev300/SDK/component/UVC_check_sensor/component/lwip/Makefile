# Project name

PROJ = liblwip

TARGET = $(PROJ).a

TOPDIR 	?= $(shell pwd)

LIBDIR = ../../kernel/lib

# Flags

INCLUDE    =	-I. \
		-Iinclude \
		-Iinclude/lwip \
		-Iinclude/arch \
		-Iinclude/ipv4/lwip \
		-Iinclude/netif \
		-Iinclude/posix \
		-Iinclude/posix/sys


#Locations
BUILDPATH  ?= build

SRCDIR += api 
SRCDIR += core
SRCDIR += core/ipv4
SRCDIR += core/snmp
SRCDIR += netif
SRCDIR += netif/ppp


CSRCS += $(foreach d, $(SRCDIR), $(wildcard $d/*.c))

COBJS += $(patsubst %.c, $(BUILDPATH)/%.o, $(CSRCS))



#build rule
.PHONY: all prepare target clean install

all: prepare target

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

