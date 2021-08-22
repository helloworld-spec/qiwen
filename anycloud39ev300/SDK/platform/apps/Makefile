CFLAGS += $(addprefix -I,$(PLAT_INC_DIR))
CFLAGS += $(addprefix -I,$(MPI_INC_DIR))
CFLAGS += $(addprefix -I,$(APP_INC_DIR))

dir_y :=
dir_y += updater
dir_y += akipc
dir_y += daemon
dir_y += ccli
dir_y += cmd_serverd
dir_y += mmc_test
dir_y += version
dir_y += drv

ifeq ($(CONFIG_DANA_SUPPORT), y)
	CFLAGS += -DCONFIG_DANA_SUPPORT=1
endif

ifeq ($(CONFIG_RTSP_SUPPORT), y)
	CFLAGS += -DCONFIG_RTSP_SUPPORT=1
endif


ifeq ($(CONFIG_DANA_SUPPORT)_$(CONFIG_RTSP_SUPPORT), y_y)
	dir_y += product_test
	CUR_DIR := $(shell pwd)
	PRODUCT_TEST_DIR := $(CUR_DIR)/product_test/output/
	export PRODUCT_TEST_DIR
endif

all:
	@for i in $(dir_y); \
	do \
		$(MAKE) -C $$i; \
		if [ $$? -ne 0 ];then exit 1; fi \
	done

install:
	@for i in $(dir_y); \
	do \
		$(MAKE) -C $$i install; \
	done

clean:
	@for i in $(dir_y); \
	do \
		$(MAKE) -C $$i clean; \
	done

image:

demo:


.PHONY: all install clean
