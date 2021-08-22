SRCS := $(wildcard *.c)
OBJS := $(patsubst %.c,%.o,$(SRCS))
LIB_NAME := libplat_ai

ifeq ($(COMPILE_SO), y)
	TARGET := $(LIB_NAME).so
else
	TARGET := $(LIB_NAME).a
endif

.PHONY: $(TARGET) install

$(TARGET): $(OBJS)
	$(LOCAL_GEN) $(LOCAL_COMMON_FLAG) $@ $^
	$(CP) $(TARGET) $(PLAT_LIB_DIR)
	@echo ""

%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@
	@echo ""
	
clean:	
	$(RM) *.o
	$(RM) $(TARGET)
	$(RM) $(PLAT_LIB_DIR)/$(TARGET)

install:
	@echo ""
