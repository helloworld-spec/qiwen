ifeq (0,$(MAKELEVEL))
TOPDIR := $(CURDIR)/../..
export TOPDIR
endif

include $(TOPDIR)/config.mk

INCLUDE         +=
SRC_DIRS        += mac80211
SRC_DIRS        += lib
SRC_DIRS        += cli
LOCAL_SRCS      += app_main.c
LOCAL_SRCS      += msgevt.c
LOCAL_SRCS      += pbuf.c
LOCAL_SRCS      += soc_global.c
#LOCAL_SRCS      += soc_log.c 

##############################################################
# make rules
##############################################################
include $(TOPDIR)/rules.mk

