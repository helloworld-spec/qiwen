# appointed libs and their dir
LOCAL_SHARED_LIB:= akispsdk akuio

ifeq ($(COMPILE_SO), y)
LOCAL_SHARED_LIB+= plat_common plat_thread plat_vpss plat_vi plat_ipcsrv
else
PLAT_STATIC_LIB:= plat_common plat_vpss plat_vi plat_thread plat_ipcsrv
APP_STATIC_LIB:=
endif

ifeq ($(DEBUG), y)
	PLAT_STATIC_LIB += dmallocthcxx dmallocth
endif

CFLAGS += $(addprefix -I,$(PLAT_INC_DIR_INNER))

LIBS := $(addprefix $(PLAT_LIB_DIR)/,$(patsubst %,lib%.a,$(PLAT_STATIC_LIB)))
LIBS += $(addprefix $(APP_LIB_DIR)/,$(patsubst %,lib%.a,$(APP_STATIC_LIB)))

LDFLAGS := $(LOCAL_LDFLAGS)
LDFLAGS += -L$(APP_LIB_DIR)
LDFLAGS += $(addprefix -l,$(patsubst lib%,%,$(LOCAL_SHARED_LIB)))
LDFLAGS += $(STD_LDFLAGS)

SRCS := $(wildcard *.c)
OBJS := $(patsubst %.c,%.o,$(SRCS))
TARGET := ak_vpss_demo

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
