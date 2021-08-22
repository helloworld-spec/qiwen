
SRCS := $(wildcard *.c)
OBJS:=$(patsubst %.c, %.o, $(SRCS))
PLATFORM_LIB_DIR := ../../platform/libplat/lib
PLATFORM_LIB_INC_DIR := ../../platform/libplat/src/include
DRV_INC_DIR := ../../kernel/include/
DRV_INC_DIR += ../../kernel/include/plat-anyka
DRV_INC_DIR += ../../kernel/drivers/misc
TARGET := libakispsdk.so
TARGET_A := libakispsdk.a
TARGET_INC := ak_isp_sdk.h

DEBUG := n
ifeq ($(DEBUG), y)
	LOCAL_CFLAGS := -g -O0
else
	LOCAL_CFLAGS := -O2
endif

CROSS_COMPILE := arm-anykav200-linux-uclibcgnueabi-
CC := $(CROSS_COMPILE)gcc
AR := $(CROSS_COMPILE)ar
RM := rm -rf
CP := cp -af
CFLAGS := $(LOCAL_CFLAGS) -Wall -Werror $(addprefix -I,$(DRV_INC_DIR))
CFLAGS += -MD -nostdlib
ARFLAGS :=
SHARED_FLAGS := -shared -fPIC

all: $(TARGET) $(TARGET_A)

$(TARGET):$(OBJS)
	$(CC) $(SHARED_FLAGS) $^ -o $@ 

$(TARGET_A):$(OBJS)
	$(AR) crv $@ $^

$(OBJS): %.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

install:
	$(CP) $(TARGET) $(PLATFORM_LIB_DIR)
	$(CP) $(TARGET_INC) $(PLATFORM_LIB_INC_DIR)

clean:
	$(RM) *.o
	$(RM) *.so*
	$(RM) *.d
	$(RM) *.a


.PHONY: all install clean
