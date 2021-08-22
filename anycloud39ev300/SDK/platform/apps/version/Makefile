# appointed libs and their dir
LOCAL_SHARED_LIB := akuio akmedia akaudiocodec akaudiofilter akv_encode akispsdk

ifeq ($(COMPILE_SO), y)
LOCAL_SHARED_LIB += plat_common plat_thread plat_vi plat_vpss plat_drv plat_its \
					plat_ao plat_ai plat_ipcsrv plat_venc_cb plat_tw \
					mpi_aenc mpi_adec mpi_venc mpi_muxer \
					mpi_osd mpi_md mpi_aed app_ini app_dvr app_net\
					app_alarm app_osd_ex app_rtsp akae
PLAT_STATIC_LIB :=
MPI_STATIC_LIB :=
APP_STATIC_LIB :=
else
PLAT_STATIC_LIB := plat_common plat_thread plat_vi plat_vpss plat_drv \
	  			plat_ai plat_ao plat_its plat_ipcsrv plat_venc_cb plat_tw
MPI_STATIC_LIB := mpi_adec mpi_aenc mpi_venc mpi_muxer mpi_osd mpi_md mpi_aed
APP_STATIC_LIB := app_ini app_osd_ex app_dvr app_net app_alarm app_rtsp akae
endif

ifeq ($(DEBUG), y)
	PLAT_STATIC_LIB += dmallocthcxx dmallocth
endif

# appointed link lib
LIBS := $(addprefix $(PLAT_LIB_DIR)/,$(patsubst %,lib%.a,$(PLAT_STATIC_LIB)))
LIBS += $(addprefix $(MPI_LIB_DIR)/,$(patsubst %,lib%.a,$(MPI_STATIC_LIB)))
LIBS += $(addprefix $(APP_LIB_DIR)/,$(patsubst %,lib%.a,$(APP_STATIC_LIB)))

LOCAL_LDFLAGS := -L$(PLAT_LIB_DIR) -L$(MPI_LIB_DIR) -L$(APP_LIB_DIR)
LDFLAGS := $(LOCAL_LDFLAGS)

LDFLAGS += $(addprefix -l,$(patsubst lib%,%,$(LOCAL_SHARED_LIB)))
LDFLAGS += $(STD_LDFLAGS)

SRCS := $(wildcard *.c)
OBJS := $(patsubst %.c,%.o,$(SRCS))

TARGET := ak_lib_version
all: $(TARGET)

$(TARGET): $(OBJS)
	$(LDCXX) $^ $(LDFLAGS) -Xlinker "-(" $(LIBS) -Xlinker "-)" -o $@
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
