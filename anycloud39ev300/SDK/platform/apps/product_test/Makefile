CUR_DIR:=$(shell pwd)
LOCAL_INC_DIR := $(CUR_DIR)/include $(PLAT_INC_DIR_INNER) $(AKIPC_INNER_INC)
CFLAGS += $(addprefix -I,$(LOCAL_INC_DIR))

TARGET_DIR := $(CUR_DIR)/output/
export TARGET_DIR

dir_y := main
dir_y += net
dir_y += audio
dir_y += ircut
dir_y += recover_dev
dir_y += mmc
dir_y += wifi
dir_y += ptz
dir_y += mac
dir_y += pushid
dir_y += config
dir_y += voice_tip
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
