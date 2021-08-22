SRCS:= $(wildcard *.c)
OBJS:= $(patsubst %.c, %.o, $(SRCS))
LIB_NAME := libapp_rtsp

ifeq ($(COMPILE_SO), y)
	TARGET := $(LIB_NAME).so
else
	TARGET := $(LIB_NAME).a
endif

all: $(TARGET)

$(TARGET):$(OBJS)
	$(LOCAL_GEN) $(LOCAL_COMMON_FLAG) $@ $^
	$(CP) $(TARGET) $(APP_LIB_DIR)

%.o : %.c
	$(CC) -I$(CURDIR)/../akae/include $(CFLAGS) -c $^ -o $@

install:
	@echo ""

clean:
	rm -f $(TARGET)
	rm -f $(OBJS)	
	$(RM) $(APP_LIB_DIR)/$(TARGET)

.PHONY: $(TARGET) install clean
