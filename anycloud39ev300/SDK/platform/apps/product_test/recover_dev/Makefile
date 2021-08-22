SRCS := $(wildcard *.c)
OBJS := $(patsubst %.c,%.o,$(SRCS))

.PHONY: all install

all: $(OBJS)
	@mv *.o $(TARGET_DIR)

%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@
	@echo ""

install:
	@echo ""
