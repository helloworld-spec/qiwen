# Project name
PROJ := sky39ev200


#source code Locations
export TOPDIR      = $(shell pwd)
export KERNELDIR   = $(TOPDIR)/kernel
export PLATFORMDIR = $(TOPDIR)/platform

export IMAGEPATH   = $(TOPDIR)/image

include $(KERNELDIR)/board.cfg
include $(TOPDIR)/system.cfg

DEFINE +=-DAK_RTOS
export DEFINE

#demo src
DEMODIR += $(PLATFORMDIR)/libapp/demo/ini
DEMODIR += $(PLATFORMDIR)/libapp/demo/dvr
DEMODIR += $(PLATFORMDIR)/libapp/demo/sysupd
DEMODIR += $(PLATFORMDIR)/libapp/demo/photo
DEMODIR += $(PLATFORMDIR)/libapp/demo/alarm

ifeq ($(LWIP), y)
DEMODIR += $(PLATFORMDIR)/libapp/demo/net 
DEMODIR += $(PLATFORMDIR)/libapp/demo/uvc 
endif

ifeq ($(WIFI), y)
DEMODIR += $(PLATFORMDIR)/libapp/demo/wifi 
endif

ifeq ($(LIBAPP), y)
PLATLIBS += $(shell find $(PLATFORMDIR)/libapp/lib/ -name *.a)
endif

#libakstreamenclib should be placed behand libmpi_venc for link order
ifeq ($(LIBMPI), y)
DEMODIR += $(PLATFORMDIR)/libmpi/demo 
PLATLIBS += $(shell find $(PLATFORMDIR)/libmpi/lib/ -name *.a)
endif

ifeq ($(LIBPLAT), y)
DEMODIR  += $(PLATFORMDIR)/libplat/demo 
PLATLIBS += $(shell find $(PLATFORMDIR)/libplat/lib/ -name *.a)
endif

#main application
ifeq ($(RING), y)
APPDIR += $(PLATFORMDIR)/apps/ring_call
endif
ifeq ($(AKIPC), y)
APPDIR += $(PLATFORMDIR)/apps/akipc
endif

ifeq ($(DEMO_SUPPORT), y)
APPDIR += $(DEMODIR)
endif

CSRCS += $(foreach d, $(APPDIR), $(wildcard $d/*.c))

APPOBJS += $(patsubst %.c, %.o, $(CSRCS))

#export for kernel use
export APPOBJS PLATLIBS


#build rule
.PHONY: all platform apps kernel image release apps_clean clean 

all: prepare platform apps kernel 

prepare:
ifneq ($(BOARD), cc3200bd)
ifneq ($(BOARD), devbd)
	$(error BOARD undefined!  Please use "make BOARD=cc3200bd" or "make BOARD=devbd" to make target on board with cc3200 or board with 8801 wifi)
endif
endif

platform: 
	$(MAKE) -C $(TOPDIR)/platform 
	
kernel: prepare
	$(MAKE) -C $(TOPDIR)/kernel 

apps: prepare
	@for i in $(APPDIR); \
	do \
		$(MAKE) -C $$i ; \
		if [ $$? -ne 0 ]; then\
		  exit 1;\
		fi \
	done

apps_clean:
	for i in $(APPDIR); \
	do \
		$(MAKE) -C $$i  clean; \
	done

	
image:
	$(MAKE) -C $(TOPDIR)/kernel 

release:platform
	$(MAKE) -C $(TOPDIR)/kernel commonlib 
	$(MAKE) -C $(TOPDIR)/kernel driverlib
	mkdir -p ../release_tmp
	rm  -rf ../release_tmp/*
	mkdir -p ../release
	svn export kernel ../release_tmp/kernel
	svn export tools ../release_tmp/tools
	svn export platform ../release_tmp/platform
	svn export rules.mk ../release_tmp/rules.mk
	svn export system.cfg ../release_tmp/system.cfg
	svn export Makefile ../release_tmp/Makefile
	sed -i -e '/.PHONY/{s/platform//;s/release//}' -e '/all:/s/platform//' -e '/platform:/,+1d' -e '/release/,/release_end/d' -e '/platform clean/d' ../release_tmp/Makefile
	sed -i -e '/.PHONY/{s/commonlib//;s/driverlib//}' -e '/all:/{s/commonlib//;s/driverlib//}' -e '/commonlib:/,+1d' -e '/driverlib:/,+1d' -e '/src clean/d' ../release_tmp/kernel/Makefile
	rm -rf ../release_tmp/kernel/common/src
	rm -rf ../release_tmp/kernel/driver/src
	rm -rf ../release_tmp/platform/libapp/src
	rm -rf ../release_tmp/platform/libmpi/src
	rm -rf ../release_tmp/platform/libplat/src
	cp  kernel/lib/*.a ../release_tmp/kernel/lib/
	cp  platform/libmpi/lib/*.a ../release_tmp/platform/libmpi/lib/
	cp  platform/libapp/lib/*.a ../release_tmp/platform/libapp/lib/
	cp  platform/libplat/lib/*.a ../release_tmp/platform/libplat/lib/
	cp -rf ../release_tmp/* ../release
	rm -rf ../release_tmp
#release_end
clean: apps_clean
	$(MAKE) -C $(TOPDIR)/kernel clean
	$(MAKE) -C $(TOPDIR)/platform clean


#Rules
include $(TOPDIR)/rules.mk
