ifeq (0,$(MAKELEVEL))
TOPDIR := $(CURDIR)/../../../..
export TOPDIR
endif

include $(TOPDIR)/config.mk

INCLUDE         +=
SRC_DIRS        +=
LOCAL_SRCS      += cli_cmd.c mac_monitor.c

##############################################################
# make rules
##############################################################
include $(TOPDIR)/rules.mk

