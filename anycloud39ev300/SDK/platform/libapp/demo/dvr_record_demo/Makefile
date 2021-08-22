# appointed libs and their dir
LOCAL_SHARED_LIB := akuio akmedia akaudiocodec akaudiofilter akv_encode	akispsdk

ifeq ($(COMPILE_SO), y)
LOCAL_SHARED_LIB+= plat_common plat_thread plat_vi plat_vpss  plat_venc_cb
LOCAL_SHARED_LIB+= plat_ai plat_ao plat_its plat_ipcsrv
LOCAL_SHARED_LIB+= mpi_adec mpi_aenc mpi_venc mpi_muxer \
				   mpi_osd mpi_md mpi_aed
LOCAL_SHARED_LIB+=app_ini app_osd_ex app_dvr app_net app_alarm
else
# decide locate what kind of lib
PLAT_STATIC_LIB := plat_common plat_thread plat_vi plat_vpss plat_drv plat_ipcsrv
PLAT_STATIC_LIB += plat_ai plat_ao plat_its plat_venc_cb
ifeq ($(DEBUG), y)
	PLAT_STATIC_LIB += dmallocthcxx dmallocth
endif
MPI_STATIC_LIB := mpi_adec mpi_aenc mpi_venc mpi_muxer mpi_osd mpi_md mpi_aed
APP_STATIC_LIB := app_ini app_osd_ex app_dvr app_net app_alarm
endif

# appointed link lib
LIBS := $(addprefix $(PLAT_LIB_DIR)/,$(patsubst %,lib%.a,$(PLAT_STATIC_LIB)))
LIBS += $(addprefix $(MPI_LIB_DIR)/,$(patsubst %,lib%.a,$(MPI_STATIC_LIB)))
LIBS += $(addprefix $(APP_LIB_DIR)/,$(patsubst %,lib%.a,$(APP_STATIC_LIB)))

LDFLAGS := -L$(PLAT_LIB_DIR) -L$(MPI_LIB_DIR) -L$(APP_LIB_DIR)
LDFLAGS += $(addprefix -l,$(patsubst lib%,%,$(LOCAL_SHARED_LIB)))
LDFLAGS += $(STD_LDFLAGS)

SRCS := $(wildcard *.c)
OBJS := $(patsubst %.c,%.o,$(SRCS))
TARGET := ak_dvr_record_demo
all:$(TARGET)

$(TARGET):$(OBJS)
	$(LDCXX) $^ $(LDFLAGS) -Xlinker "-(" $(LIBS) -Xlinker "-)" -o $@
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
