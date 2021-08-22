SRCS := $(wildcard *.c)
OBJS := $(patsubst %.c,%.o,$(SRCS))

CFLAGS += -I./include

.PHONY: all install

all: $(OBJS)
	@mv *.o $(TARGET_DIR)

%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@
	@echo ""

install:
	@echo ""
