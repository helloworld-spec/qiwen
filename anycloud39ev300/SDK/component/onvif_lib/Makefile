KERNEL_DIR = ../../kernel

KERNEL_ISP_CHAR_INC_DIR = $(KERNEL_DIR)/drivers/misc

PLATFORM_DIR = ../../platform
PLATFORM_LIB_DIR = $(PLATFORM_DIR)/libapp/lib/
PLATFORM_LIB_INC_DIR = $(PLATFORM_DIR)/libapp/src/onvif/

CP = cp

#USE_CPP := 1
LOCAL_SRC_CFILES := \
        http_service.c dvs.c rtsp.c

LOCAL_MODULE := libonvif.a
PREFIX := arm-anykav200-linux-uclibcgnueabi-

LOCAL_SHARED_LIBRARIES := $(LOCAL_SHARED_LIBRARIES)
LOCAL_C_INCLUDES := $(KERNEL_DIR)/include \
	$(KERNEL_DIR)/include/plat-anyka	\
	$(KERNEL_DIR)/drivers/misc	
LOCAL_LDFLAGS :=  -Xlinker -Map=output.map
LOCAL_CFLAGS := -MD -nostdlib -D_SVID_SOURCE -D__LINUX__ -D_GNU_SOURCE
LOCAL_ARFLAGS :=
include a_rules.mk
