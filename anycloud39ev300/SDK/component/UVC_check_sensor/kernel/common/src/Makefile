# Project name

PROJ = libkernel_common

TARGET = $(PROJ).a

KERNELDIR ?= $(shell pwd)/../../../kernel
LIBDIR = $(KERNELDIR)/lib


# Flags

export INCLUDE  =	-I. \
		-I../ \
		-I$(KERNELDIR)/include \
		-I$(KERNELDIR)/include/driver \
		-I$(KERNELDIR)/include/os \


CFLAGS 	+= $(DEFINE)

#Locations

BUILDPATH = build

SRCDIR += .

CSRCS += $(foreach d, $(SRCDIR), $(wildcard $d/*.c))

COBJS += $(patsubst %.c, $(BUILDPATH)/%.o, $(CSRCS))

#build rule
.PHONY: all target clean


all: prepare target 

prepare:
	mkdir -p $(BUILDPATH)

target:$(LIBDIR)/$(TARGET)


$(LIBDIR)/$(TARGET): $(COBJS) 
	@echo ---------------------[build $@]----------------------------------	
	$(AR) -rsv $@  $(COBJS) 

clean : 
	$(RM) $(COBJS) $(LIBDIR)/$(TARGET)
	$(RM) -rf $(BUILDPATH)


# Rules
include $(KERNELDIR)/rules.mk

