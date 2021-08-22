# appointed libs and their dir
LOCAL_SHARED_LIB := akuio akmedia akaudiocodec akaudiofilter akv_encode akispsdk plat_venc_cb

# decide locate what kind of lib
ifeq ($(COMPILE_SO), y)
	LOCAL_SHARED_LIB += plat_common plat_thread plat_vi plat_ipcsrv \
						plat_vpss plat_drv plat_ai plat_ao plat_its plat_venc_cb \
						mpi_adec mpi_aenc mpi_venc mpi_muxer mpi_osd mpi_md \
						app_dvr app_net app_ini
	PLAT_STATIC_LIB :=
	MPI_STATIC_LIB :=
	APP_STATIC_LIB :=
else
	PLAT_STATIC_LIB := plat_common plat_thread plat_vi plat_vpss plat_drv plat_ipcsrv
	PLAT_STATIC_LIB += plat_ai plat_ao plat_its plat_venc_cb 
	MPI_STATIC_LIB := mpi_adec mpi_aenc mpi_venc mpi_muxer mpi_osd mpi_md
	APP_STATIC_LIB := app_ini app_dvr app_net
endif

# support RTSP
ifeq ($(COMPILE_SO), y)
	LOCAL_SHARED_LIB += app_rtsp akae
else
	APP_STATIC_LIB += app_rtsp akae
endif

# appointed link lib
LIBS := $(addprefix $(PLAT_LIB_DIR)/,$(patsubst %,lib%.a,$(PLAT_STATIC_LIB)))
LIBS += $(addprefix $(MPI_LIB_DIR)/,$(patsubst %,lib%.a,$(MPI_STATIC_LIB)))
LIBS += $(addprefix $(APP_LIB_DIR)/,$(patsubst %,lib%.a,$(APP_STATIC_LIB)))

LOCAL_LDFLAGS := -L$(PLAT_LIB_DIR) -L$(MPI_LIB_DIR) -L$(APP_LIB_DIR)

LDFLAGS := $(LOCAL_LDFLAGS)
LDFLAGS += $(addprefix -l,$(patsubst lib%,%,$(LOCAL_SHARED_LIB)))
LDFLAGS += $(STD_LDFLAGS)

OBJS := $(wildcard *.o)

TARGET := product_test

all: $(TARGET)

$(TARGET): $(OBJS)
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
