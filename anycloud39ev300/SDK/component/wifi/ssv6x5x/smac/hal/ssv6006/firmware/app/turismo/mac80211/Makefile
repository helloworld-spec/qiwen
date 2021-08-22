ifeq (0,$(MAKELEVEL))
TOPDIR := $(CURDIR)/../../..
export TOPDIR
endif

include $(TOPDIR)/config.mk

INCLUDE         +=
SRC_DIRS        +=
LOCAL_SRCS      += cmd_engine.c smarticomm_firmware.c soft-main.c tx_stuck.c soft_beacon.c 

##############################################################
# make rules
##############################################################
include $(TOPDIR)/rules.mk

