# Project name

PROJ = libplat_thread

TARGET = $(PROJ).a

TOPDIR 	?= $(shell pwd)/../../../..
KERNELDIR ?= $(TOPDIR)/kernel
PLATDIR ?= $(TOPDIR)/platform

LIBDIR = ../../lib


# Flags

INCLUDE    =	-I. \
		-I../../include \
		-I$(KERNELDIR)/include \
		-I$(KERNELDIR)/include/os \



#Locations

SRCDIR += .

CSRCS += $(foreach d, $(SRCDIR), $(wildcard $d/*.c))

COBJS += $(patsubst %.c, %.o, $(CSRCS))



#build rule
.PHONY: all target install clean

all: target 


install:

target:$(LIBDIR)/$(TARGET)


$(LIBDIR)/$(TARGET): $(COBJS) 
	@echo ---------------------[build $@]----------------------------------	
	$(AR) -rsv $@  $(COBJS) 

clean : 
	$(RM) $(COBJS) $(LIBDIR)/$(TARGET)


# Rules
include $(TOPDIR)/rules.mk
