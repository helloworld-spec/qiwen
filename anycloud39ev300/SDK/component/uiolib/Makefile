LOCAL_SRC_FILES := akuio.c
PREFIX := arm-anykav200-linux-uclibcgnueabi-

PLATFORM_LIB_DIR := ../../platform/libplat/lib                                
PLATFORM_LIB_INC_DIR := ../../platform/libplat/include_inner/
TARGET_INC := akuio.h

LOCAL_SHARED_LIBRARIES := pthread 
CP := cp -af

LOCAL_MODULE := libakuio
LOCAL_VERSION := 3.1.01

DEBUG := n
ifeq ($(DEBUG), y)
	LOCAL_CFLAGS := -g -O0
else
	LOCAL_CFLAGS := -O2
endif

include so_rules.mk
