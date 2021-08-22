KERNEL_DIR = ../../kernel
KERNEL_LIB_DIR = $(KERNEL_DIR)/lib
KERNEL_LIB_INC_DIR = $(KERNEL_DIR)/include/plat-anyka

CP = cp -af

#USE_CPP := 1
LOCAL_SRC_CFILES := \
        ak39_isp2.c ak39_isp2_3a.c

LOCAL_MODULE := libispdrv.a
PREFIX := arm-anykav200-linux-uclibcgnueabi-

LOCAL_SHARED_LIBRARIES := $(LOCAL_SHARED_LIBRARIES)
LOCAL_C_INCLUDES := $(KERNEL_DIR)/include
LOCAL_LDFLAGS :=  -Xlinker -Map=output.map
LOCAL_CFLAGS := -MD -nostdlib
LOCAL_ARFLAGS :=
include a_rules.mk
