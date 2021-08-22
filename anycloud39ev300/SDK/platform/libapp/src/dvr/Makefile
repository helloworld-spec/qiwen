SRCS := $(wildcard *.c)
OBJS := $(patsubst %.c,%.o,$(SRCS))
LIB_NAME := libapp_dvr
LIB_TAR_DIR := $(APP_LIB_DIR)
CFLAGS += -I./include

ifeq ($(COMPILE_SO), y)
	TARGET := $(LIB_NAME).so
	FAT_OBJS := libakfat.a
else
	TARGET := $(LIB_NAME).a
	FAT_OBJS := $(shell $(LOCAL_GEN) x libakfat.a && ls *.o)
endif

.PHONY: $(TARGET) install

$(TARGET): $(OBJS) $(FAT_OBJS)
	$(LOCAL_GEN) $(LOCAL_COMMON_FLAG) $@ $^
	$(CP) $(TARGET) $(APP_LIB_DIR)
	@echo ""

%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) *.o
	$(RM) $(TARGET)
	$(RM) $(APP_LIB_DIR)/$(TARGET)

install:
	@echo ""
