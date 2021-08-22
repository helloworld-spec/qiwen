SRCS := $(wildcard *.c)
OBJS := $(patsubst %.c,%.o,$(SRCS))
LIB_NAME := libmpi_osd

ifeq ($(COMPILE_SO), y)
	TARGET := $(LIB_NAME).so
else
	TARGET := $(LIB_NAME).a
endif

.PHONY: $(TARGET) install

$(TARGET): $(OBJS)
	$(LOCAL_GEN) $(LOCAL_COMMON_FLAG) $@ $^
	$(CP) $(TARGET) $(MPI_LIB_DIR)
	@echo ""

%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@
	@echo ""
	
clean:	
	$(RM) *.o
	$(RM) $(TARGET)
	$(RM) $(MPI_LIB_DIR)/$(TARGET)

install:
	@echo ""
