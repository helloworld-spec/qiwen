SRCS := $(wildcard *.c)
OBJS := $(patsubst %.c,%.o,$(SRCS))

.PHONY: all install

all: $(OBJS)
	@mv *.o $(TARGET_DIR)
	@echo ""

%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@
	@echo ""

clean:
	$(RM) *.o
	$(RM) $(TARGET)
	$(RM) $(APP_LIB_DIR)/$(TARGET)

install:
	@echo ""
