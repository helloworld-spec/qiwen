
SRCS := $(wildcard $(CURDIR)/src/*.c)
OBJS := $(patsubst %.c,%.o,$(SRCS))

DEMOS := $(wildcard $(CURDIR)/demo/*_demo)
LIB_NAME := libplat_drv

ifeq ($(COMPILE_SO), y)
	TARGET := $(LIB_NAME).so
else
	TARGET := $(LIB_NAME).a
endif

.PHONY: install all demo

all: $(TARGET) demo


$(TARGET): $(OBJS)
	$(AR) $(ARFLAGS) $@ $^

%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@


demo: $(DEMOS)
	@for i in $^; \
	do \
		$(MAKE) -C $$i; \
		if [ $$? -ne 0 ];then exit 1; fi \
	done

	
clean:	
	$(RM) *.o
	$(RM) $(TARGET)

install:
	@echo ""
