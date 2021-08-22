# appointed libs and their dir
LOCAL_SHARED_LIB := akuio akmedia akaudiocodec akaudiofilter akv_encode	akispsdk ak_mt

# decide locate what kind of lib
ifeq ($(COMPILE_SO), y)
LOCAL_SHARED_LIB += plat_common plat_thread plat_vi plat_venc_cb \
					plat_vpss plat_drv plat_ai plat_ao plat_its plat_ipcsrv plat_ats\
					mpi_adec mpi_aenc mpi_venc mpi_muxer mpi_osd mpi_md mpi_aed \
					app_dvr app_net app_alarm app_ini app_osd_ex 
PLAT_STATIC_LIB :=
MPI_STATIC_LIB :=
APP_STATIC_LIB :=
else
PLAT_STATIC_LIB := plat_common plat_thread plat_vi plat_vpss plat_drv plat_ipcsrv
PLAT_STATIC_LIB += plat_ai plat_ao plat_its plat_venc_cb plat_ats
MPI_STATIC_LIB := mpi_adec mpi_aenc mpi_venc mpi_muxer mpi_osd mpi_md mpi_aed
APP_STATIC_LIB := app_ini app_osd_ex app_dvr app_net app_alarm
endif

LOCAL_LDFLAGS := -L$(PLAT_LIB_DIR) -L$(MPI_LIB_DIR) -L$(APP_LIB_DIR)

ifeq ($(CONFIG_DANA_SUPPORT), y)
	LOCAL_SHARED_LIB += iconv
	DANA_STATIC_LIB := dana_video_plus
	LOCAL_LDFLAGS += -L$(DANA_LIB_DIR)
endif

# support RTSP
ifeq ($(CONFIG_RTSP_SUPPORT), y)
	ifeq ($(COMPILE_SO), y)
		LOCAL_SHARED_LIB += app_rtsp akae
	else
		APP_STATIC_LIB += app_rtsp akae
	endif
endif

# appointed link lib
LIBS := $(addprefix $(PLAT_LIB_DIR)/,$(patsubst %,lib%.a,$(PLAT_STATIC_LIB)))
LIBS += $(addprefix $(MPI_LIB_DIR)/,$(patsubst %,lib%.a,$(MPI_STATIC_LIB)))
LIBS += $(addprefix $(APP_LIB_DIR)/,$(patsubst %,lib%.a,$(APP_STATIC_LIB)))

ifeq ($(CONFIG_DANA_SUPPORT), y)
	LIBS += $(addprefix $(DANA_LIB_DIR)/,$(patsubst %,lib%.a,$(DANA_STATIC_LIB)))
endif


LDFLAGS := $(LOCAL_LDFLAGS)

LDFLAGS += $(addprefix -l,$(patsubst lib%,%,$(LOCAL_SHARED_LIB)))
LDFLAGS += $(STD_LDFLAGS)

OBJS := $(wildcard *.o)
TARGET := anyka_ipc

all: $(TARGET)

$(TARGET): $(OBJS)
	@echo "CONFIG_DANA_SUPPORT=$(CONFIG_DANA_SUPPORT)"
	@echo "CONFIG_RTSP_SUPPORT=$(CONFIG_RTSP_SUPPORT)"
	$(LDCXX) $^ $(LDFLAGS) -Xlinker "-(" $(LIBS) -Xlinker "-)" -o $@
	$(RM) *.o
ifeq ($(DEBUG), n)
	$(STRIP) $@
endif

install:
	@$(CP) $(TARGET) $(ROOTFS_BIN_DIR)

clean:
	$(RM) *.o
	$(RM) $(TARGET)

.PHONY: all install clean
