# Project name
PROJ := sky39ev200


# Flags

include board.cfg

export TOPDIR 	 ?= $(shell pwd)/..

export KERNELDIR := $(shell pwd)

export INCLUDE    =	-I. \
		-I$(KERNELDIR)/include \
		-I$(KERNELDIR)/include/driver \
		-I$(KERNELDIR)/include/os \
		-I$(KERNELDIR)/include/lwip \
		-I$(KERNELDIR)/include/lwip/ipv4 \
		-I$(KERNELDIR)/include/fs \
		-I$(KERNELDIR)/mach/mach-$(chips) \
		-I$(KERNELDIR)/common \
		-I$(KERNELDIR)/fs \
		-I$(KERNELDIR)/include/partition \
		-I$(KERNELDIR)/partition \
		-I$(KERNELDIR)/driver/include 



export DEFINE		:= $(DEFINE) 

LDFLAGS	= -p -X -Map maps.txt -T section.lds

WRAP_FUNC = malloc calloc realloc free

LDFLAGS += $(addprefix --wrap=, $(WRAP_FUNC))

#source code Locations
SRCDIR += common
SRCDIR += init
SRCDIR += mm
SRCDIR += partition

ifeq ($(WIFI), y)
SRCDIR += net/wifi
endif

ifeq ($(LWIP), y)
SRCDIR += net/lwip
endif

ifeq ($(FS), y)
SRCDIR += fs
endif

IMAGEPATH  ?= $(KERNELDIR)/image
BUILDPATH  ?= $(KERNELDIR)/build

CSRCS += $(foreach d, $(SRCDIR), $(wildcard $d/*.c))

COBJS += $(patsubst %.c, $(BUILDPATH)/%.o, $(CSRCS))

COBJS +=  $(BUILDPATH)/mach/mach-$(chips)/mach-$(MACH).o

SOBJS =  $(BUILDPATH)/init/boot.o

BUILDDIR +=  $(SRCDIR)
BUILDDIR += mach/mach-$(chips)


lib-$(LWIP)	+= ./lib/liblwip.a
lib-$(WIFI)	+= ./lib/libwifi.a
lib-$(MEMORY)	+= ./lib/memory.a
lib-$(SYSDRV)	+= ./lib/drv_ak39xx.a
lib-$(AKOS)	+= ./lib/akos.a
lib-$(FS)	+= ./lib/mount.a
lib-$(FS)	+= ./lib/fat32fs.a
lib-$(ISPDRV)	+= ./lib/libispdrv_rtos.a
lib-$(PARTITION) += ./lib/libpartition_uclibc.a

lib-kernel += ./lib/libkernel_common.a
lib-kernel += ./lib/libdev_drv.a

#build rule
.PHONY: all prepare commonlib driverlib target clean 

all: prepare commonlib driverlib target 

prepare:
ifneq ($(BOARD), cc3200bd)
ifneq ($(BOARD), devbd)
	$(error BOARD undefined!  Please use "make BOARD=cc3200bd" or "make BOARD=devbd" to make target on board with cc3200 or board with 8801 wifi)
endif
endif
ifneq ($(BUILDPATH),)
	for i in $(BUILDDIR); \
	do mkdir -p $(BUILDPATH)/$$i; \
	done
endif
	mkdir -p $(IMAGEPATH)

target: $(IMAGEPATH)/$(PROJ).bin

commonlib: 
	make -C common/src

driverlib: 
	make -C driver/src

$(IMAGEPATH)/$(PROJ).bin: $(IMAGEPATH)/$(PROJ).elf 
	@echo ---------------------[build binary]----------------------------------
	$(OBJCOPY) -O binary  $<  $@
	

$(IMAGEPATH)/$(PROJ).elf:  $(SOBJS) $(COBJS) $(lib-kernel) $(lib-y) $(APPOBJS) $(PLATLIBS)
	@echo ---------------------[build elf]----------------------------------
	$(LD) $(LDFLAGS)  -o $@ $(SOBJS) $(COBJS) $(APPOBJS) "-(" $(PLATLIBS) "-)" --whole-archive $(lib-kernel) --no-whole-archive $(lib-y) $(CLIB)
#	$(OBJDUMP)  -h -D -Mreg-names-raw --show-raw-insn $@ > $(IMAGEPATH)/$(PROJ).txt


clean : 
	rm -rf $(BUILDPATH) $(IMAGEPATH)
	make -C common/src clean
	make -C driver/src clean

#Rules
include $(KERNELDIR)/rules.mk
