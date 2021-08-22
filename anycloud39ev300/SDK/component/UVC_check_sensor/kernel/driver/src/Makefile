# Project name

PROJ = libdev_drv

TARGET = $(PROJ).a

KERNELDIR ?= $(shell pwd)/../../../kernel
LIBDIR = $(KERNELDIR)/lib


# Flags

export INCLUDE = -I./include \
                 -I./camera \
                 -I../include \
                 -I$(KERNELDIR)/include \
                 -I$(KERNELDIR)/include/driver \
                 -I$(KERNELDIR)/include/os \
                 -I$(KERNELDIR)/common 


CFLAGS 	+= $(DEFINE)

#Locations

BUILDPATH = build

SRCDIR += .
SRCDIR += camera

CSRCS += $(foreach d, $(SRCDIR), $(wildcard $d/*.c))

COBJS += $(patsubst %.c, $(BUILDPATH)/%.o, $(CSRCS))

#build rule
.PHONY: all target clean


all: prepare target 

prepare:
ifneq ($(BUILDPATH),)
	@for i in $(SRCDIR); \
	do mkdir -p $(BUILDPATH)/$$i; \
	done
endif

target:$(LIBDIR)/$(TARGET)


$(LIBDIR)/$(TARGET): $(COBJS) 
	@echo ---------------------[build $@]----------------------------------	
	$(AR) -rsv $@  $(COBJS) 

clean : 
	$(RM) $(COBJS) $(LIBDIR)/$(TARGET)
	$(RM) -rf $(BUILDPATH)


# Rules
include $(KERNELDIR)/rules.mk

