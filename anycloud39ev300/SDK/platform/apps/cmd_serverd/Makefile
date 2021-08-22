# appointed libs and their dir
ifeq ($(COMPILE_SO), y)
	LOCAL_SHARED_LIB += plat_common plat_thread
	PLAT_STATIC_LIB :=
	MPI_STATIC_LIB :=
	APP_STATIC_LIB :=
else
	PLAT_STATIC_LIB := plat_common
	MPI_STATIC_LIB :=
	APP_STATIC_LIB :=
endif

ifeq ($(DEBUG), y)
	PLAT_STATIC_LIB += dmallocthcxx dmallocth
endif

# appointed link lib
LIBS := $(addprefix $(PLAT_LIB_DIR)/,$(patsubst %,lib%.a,$(PLAT_STATIC_LIB)))
LIBS += $(addprefix $(MPI_LIB_DIR)/,$(patsubst %,lib%.a,$(MPI_STATIC_LIB)))
LIBS += $(addprefix $(APP_LIB_DIR)/,$(patsubst %,lib%.a,$(APP_STATIC_LIB)))

LOCAL_LDFLAGS := -L$(PLAT_LIB_DIR) -L$(MPI_LIB_DIR)
LDFLAGS := $(LOCAL_LDFLAGS)

LDFLAGS += $(addprefix -l,$(patsubst lib%,%,$(LOCAL_SHARED_LIB)))
LDFLAGS += $(STD_LDFLAGS)

SRCS := $(wildcard *.c)
OBJS := $(patsubst %.c,%.o,$(SRCS))

TARGET := cmd_serverd

all: $(TARGET)

$(TARGET): $(OBJS)
	$(LD) $^ $(LDFLAGS) -Xlinker "-(" $(LIBS) -Xlinker "-)" -o $@
ifeq ($(DEBUG), n)
	$(STRIP) $@
endif

%.o : %.c
	@$(CC) $(CFLAGS) -c $< -o $@

install:
	@$(CP) $(TARGET) $(ROOTFS_BIN_DIR)

clean:
	$(RM) *.o
	$(RM) $(TARGET)

.PHONY: all install clean
