# appointed libs and their dir
LOCAL_SHARED_LIB:=

ifeq ($(COMPILE_SO), y)
LOCAL_SHARED_LIB+= plat_common plat_thread app_ini
else
PLAT_STATIC_LIB:= plat_common plat_thread
APP_STATIC_LIB:= app_ini
endif

ifeq ($(DEBUG), y)
	PLAT_STATIC_LIB += dmallocthcxx dmallocth
endif

LIBS := $(addprefix $(PLAT_LIB_DIR)/,$(patsubst %,lib%.a,$(PLAT_STATIC_LIB)))
LIBS += $(addprefix $(APP_LIB_DIR)/,$(patsubst %,lib%.a,$(APP_STATIC_LIB)))

LDFLAGS := $(LOCAL_LDFLAGS)
LDFLAGS += -L$(APP_LIB_DIR)
LDFLAGS += $(addprefix -l,$(patsubst lib%,%,$(LOCAL_SHARED_LIB)))
LDFLAGS += $(STD_LDFLAGS)

SRCS := $(wildcard *.c)
OBJS := $(patsubst %.c,%.o,$(SRCS))
TARGET := ak_ini_demo

all:$(TARGET)

$(TARGET):$(OBJS)
	$(LD) $^ $(LDFLAGS) -Xlinker "-(" $(LIBS) -Xlinker "-)" -o $@
	$(STRIP) $@

%.o : %.c
	$(CC) $(CFLAGS) -c $^ -o $@
	@echo ""

install:
	@echo ""

clean:
	$(RM) *.o
	$(RM) $(TARGET)

.PHONY: all install clean
