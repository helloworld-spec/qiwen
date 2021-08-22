SRCS := $(wildcard *.c)
OBJS := $(patsubst %.c,%.o,$(SRCS))
TARGET := updater

ifeq ($(DEBUG), y)
	PLAT_STATIC_LIB += dmallocthcxx dmallocth
	LIBS := $(addprefix $(PLAT_LIB_DIR)/,$(patsubst %,lib%.a,$(PLAT_STATIC_LIB)))
endif

all:$(TARGET)

$(TARGET):$(OBJS)
	$(LD) $(STD_LDFLAGS) -L$(PLAT_LIB_DIR) -lpthread $^ $(LIBS) -o $@
	$(STRIP) $@

%.o : %.c
	$(CC) $(CFLAGS) -c $^ -o $@
	@echo ""

install:
	@$(CP) $(TARGET) $(ROOTFS_SBIN_DIR)

clean:
	$(RM) *.o
	$(RM) $(TARGET)

.PHONY: all clean install
