CUR_DIR := $(shell pwd)
TARGET_DIR := $(CUR_DIR)/output/
export TARGET_DIR

AKIPC_INNER_INC := $(CUR_DIR)/include
CFLAGS += $(addprefix -I,$(AKIPC_INNER_INC))

dir_y := main
dir_y += misc
dir_y += config
dir_y += record

ifeq ($(CONFIG_DANA_SUPPORT), y)
	dir_y += dana
	DANA_LIB_DIR := $(CUR_DIR)/dana/lib
	export DANA_LIB_DIR
endif

dir_y += monitor
dir_y += output

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
	@$(MAKE) -C output clean

.PHONY: all install clean
