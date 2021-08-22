# Project name

PROJ = libmpi_muxer

TARGET = $(PROJ).a

TOPDIR 	?= $(shell pwd)/../../../..
KERNELDIR ?= $(TOPDIR)/kernel
PLATDIR ?= $(TOPDIR)/platform

LIBDIR = ../../lib


# Flags

INCLUDE    =	-I. \
		-I../include \
		-I../include/akmedialib \
		-I../../include \
		-I$(KERNELDIR)/include \
		-I$(PLATDIR)/libplat/include \
		-I$(PLATDIR)/libplat/src/include 


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
