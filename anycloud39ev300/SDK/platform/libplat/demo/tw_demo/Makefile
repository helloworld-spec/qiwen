# appointed libs and their dir
LOCAL_SHARED_LIB:= akaudiofilter

LOCAL_LDFLAGS := -L$(PLAT_LIB_DIR)

ifeq ($(COMPILE_SO), y)
LOCAL_SHARED_LIB += plat_common plat_thread plat_ai plat_tw plat_ipcsrv
LIBS :=
else
LOCAL_STATIC_LIB := plat_common plat_thread plat_ai plat_tw plat_ipcsrv
LIBS := $(addprefix $(PLAT_LIB_DIR)/,$(patsubst %,lib%.a,$(LOCAL_STATIC_LIB)))
endif

LDFLAGS := $(LOCAL_LDFLAGS)
LDFLAGS += $(addprefix -l,$(patsubst lib%,%,$(LOCAL_SHARED_LIB)))
LDFLAGS += $(STD_LDFLAGS) -lm

SRCS := $(wildcard *.c)
OBJS := $(patsubst %.c,%.o,$(SRCS))
TARGET := ak_tw_demo

all:$(TARGET)

$(TARGET):$(OBJS)
	$(LD) $^ $(LDFLAGS) -Xlinker "-(" $(LIBS) -Xlinker "-)" -o $@
	$(STRIP) $@

%.o : %.c
	$(CC) $(CFLAGS) -c $^ -o $@
	@echo ""

install:
	@$(CP) $(TARGET) $(ROOTFS_BIN_DIR)
	@echo ""

clean:
	$(RM) *.o
	$(RM) $(TARGET)

.PHONY: all install clean
