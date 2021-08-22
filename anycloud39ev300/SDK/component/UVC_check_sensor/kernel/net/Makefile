# Project name

PROJ = net

TARGET = $(PROJ).a

TOPDIR 	?= $(shell pwd)/../../..
KERNELDIR ?= $(shell pwd)/../.. 

LIBDIR = $(KERNELDIR)/lib


# Flags

INCLUDE    =	-I. \
		-I$(KERNELDIR)/include \
		-I$(KERNELDIR)/include/driver \
		-I./include \
		-I./lwip/include \
		-I./lwip/include/lwip \
		-I./lwip/include/ipv4/lwip \
		-I./wifi/include 



#Locations
BUILDPATH  ?= build

SRCDIR += demo
SRCDIR += usr


CSRCS += $(foreach d, $(SRCDIR), $(wildcard $d/*.c))

COBJS += $(patsubst %.c, $(BUILDPATH)/%.o, $(CSRCS))



#build rule
.PHONY: all prepare target clean

all: prepare target 

prepare:
ifneq ($(BUILDPATH),)
	@for i in $(SRCDIR); \
	do mkdir -p $(BUILDPATH)/$$i; \
	done
endif


target:$(BUILDPATH)/$(TARGET)


$(BUILDPATH)/$(TARGET): $(COBJS) 
	@echo ---------------------[build out]----------------------------------	
	$(AR) -rsv $(BUILDPATH)/$(TARGET)  $(COBJS) 
	cp $(BUILDPATH)/$(TARGET)  $(LIBDIR) 


clean : 
	$(RM) $(BUILDPATH)


# Rules
include $(TOPDIR)/rules.mk


