# rootfs global info
WORKDIR:=$(shell pwd)
include $(WORKDIR)/config.mk

ROOTFS_BIN_DIR ?= $(WORKDIR)/rootfs/rootfs/usr/bin
ROOTFS_LIB_DIR ?= $(WORKDIR)/rootfs/rootfs/usr/lib
ROOTFS_SBIN_DIR ?= $(WORKDIR)/rootfs/rootfs/sbin
ROOTFS_CGI_DIR ?= $(WORKDIR)/rootfs/rootfs/usr/www/cgi-bin
MAKE:=make
CP:=cp -f
RM:=rm -rf

export ROOTFS_BIN_DIR ROOTFS_SBIN_DIR ROOTFS_CGI_DIR MAKE CP RM ROOTFS_LIB_DIR WORKDIR

# export all of the platform we supportted
export CONFIG_DANA_SUPPORT CONFIG_RTSP_SUPPORT
export CONFIG_WATCHDOG_ENABLE CONFIG_UTILS_SUPPORT
## wifi
export CONFIG_WIFI_RTL8188EUS CONFIG_WIFI_RTL8188FTV CONFIG_WIFI_RTL8189ETV CONFIG_WIFI_SSV6X5X
export CONFIG_WIFI_RTL8189FTV CONFIG_WIFI_ATBM603X CONFIG_WIFI_HI3881

ONFIG_WIFI_RTL8188EUS ?=
CONFIG_WIFI_RTL8188FTV ?=
CONFIG_WIFI_RTL8189ETV ?=
CONFIG_WIFI_SSV6X5X ?=
CONFIG_WIFI_RTL8189FTV ?=
CONFIG_WIFI_ATBM603X ?=
CONFIG_WIFI_MT3881 ?=

# libs and their include dir
PLAT_LIB_DIR := $(WORKDIR)/libplat/lib
PLAT_INC_DIR := $(WORKDIR)/libplat/include
MPI_LIB_DIR := $(WORKDIR)/libmpi/lib
MPI_INC_DIR := $(WORKDIR)/libmpi/include
APP_LIB_DIR := $(WORKDIR)/libapp/lib
APP_INC_DIR := $(WORKDIR)/libapp/include

PLAT_CONFIG:= CONFIG_DANA_SUPPORT=y CONFIG_RTSP_SUPPORT=y

export PLAT_LIB_DIR PLAT_INC_DIR MPI_LIB_DIR MPI_INC_DIR
export APP_LIB_DIR APP_INC_DIR

# compile toolchain
CROSS_COMPILE ?= arm-anykav200-linux-uclibcgnueabi-

DEBUG := n
CC := $(CROSS_COMPILE)gcc
CXX := $(CROSS_COMPILE)g++

ifeq ("$(origin DBG)", "command line")
	DEBUG= $(DBG)
endif

ifeq ($(DEBUG), y)
	CFLAGS := -O0
	CXXFLAGS := -O0
	STD_LDFLAGS = -lrt -lpthread -ldl -Wl,-rpath=/mnt -rdynamic
	CC += -g
	CXX += -g
else
	CFLAGS := -fno-strict-aliasing -Os
	CXXFLAGS := -fno-strict-aliasing -Os
	STD_LDFLAGS = -lrt -lpthread -ldl -Wl,-rpath=/mnt -rdynamic
endif

CFLAGS += -Wall -Werror -D_GNU_SOURCE -std=c99 -fms-extensions
LD:= $(CC)
LDCC := $(CC)
LDCXX := $(CXX)
STRIP :=$(CROSS_COMPILE)strip

AR := $(CROSS_COMPILE)ar
ARFLAGS := crs
export CROSS_COMPILE CC CXX LD LDCC LDCXX STRIP CFLAGS CXXFLAGS AR ARFLAGS STD_LDFLAGS DEBUG

PLAT_RELEASE := n
COMPILE_SO ?= y
COMPILE_DEMO := y

ifeq ("$(origin SO)", "command line")
	COMPILE_SO = $(SO)
endif

ifeq ("$(origin DEMO)", "command line")
	COMPILE_DEMO = $(DEMO)
endif

# config compile lib type, build .so default

export PLAT_RELEASE COMPILE_SO LOCAL_COMMON_FLAG LOCAL_GEN COMPILE_DEMO

# rootfs dir MUST be placed at first
dir-y := rootfs
dir-y += libplat
dir-y += libmpi
dir-y += libapp
dir-y += apps

FILTER-OUT-MPI:=apps rootfs
MPI_DIR:=$(filter-out $(FILTER-OUT-MPI), $(dir-y))

all:
	@for i in $(dir-y); \
	do \
		$(MAKE) -C $$i; \
		if [ $$? -ne 0 ];then exit 1; fi; \
		echo ""; \
	done

install:
	@for i in $(dir-y);	\
	do	\
		$(MAKE) -C $$i install;	\
		echo ""; \
	done

image:
	@for i in $(dir-y);	\
	do	\
		$(MAKE) -C $$i image; \
	done

nandimage:
	@for i in $(dir-y);	\
	do	\
		$(MAKE) -C $$i nandimage; \
	done

clean:
	@for i in $(dir-y); \
	do \
		$(MAKE) -C $$i clean; \
		echo ""; \
	done
	@for i in $(dir-y); \
	do \
		$(MAKE) -C $$i COMPILE_SO=n clean; \
		echo ""; \
	done

modules:
	@for i in $(MPI_DIR);\
	do \
		$(MAKE) -C $$i COMPILE_SO=y $(PLAT_CONFIG); \
		if [ $$? -ne 0 ];then exit 1; fi \
	done

lib:
	@for i in $(MPI_DIR); \
	do \
		$(MAKE) -C $$i COMPILE_SO=n $(PLAT_CONFIG); \
		if [ $$? -ne 0 ];then exit 1; fi \
	done

app:
	$(MAKE) -C apps COMPILE_SO=$(COMPILE_SO)

demo:
	@for i in $(dir-y);	\
	do	\
		$(MAKE) -C $$i demo COMPILE_SO=$(COMPILE_SO);\
	done

help:
	@echo ""
	@echo " Generic targets:"
	@echo "   all     - Build all targets. Default link the dynamic link libraries"
	@echo "   install - Build file-system and install executable file"
	@echo "   image   - Build take package for update"
	@echo "   clean   - Clean up targets files and temp files"
	@echo ""
	@echo " SDK libraries targets:"
	@echo "   modules - Build all dynamic link libaries in lib* directory"
	@echo "   lib     - Build all statically link libaries in lib* directory"
	@echo ""
	@echo " Particular targets:"
	@echo "   app     - Build all application in apps directory"
	@echo "   demo    - Build all demo in lib* directory"
	@echo ""
	@echo "   Notice: to build app and/or demo, you need build libraries first"
	@echo "           by 'make modules' or 'make lib'"
	@echo ""
	@echo " Command line argumemts:"
	@echo "   make SO=[y/n] ==> y: build dynamic link libaries (*.so)"
	@echo "                 ==> n: build statically link libaries (*.a)"
	@echo "      'y' is default, its equal to 'make' with no argument"
	@echo ""
	@echo "   make DBG=[y/n] ==> y: build debug mode, will link with -g and"
	@echo "                         others debug libaries, and do not strip."
	@echo "                  ==> n: build release mode, will strip exec files"
	@echo "      'n' is default."
	@echo ""

.PHONY: all install image nandimage clean modules lib app demo help
