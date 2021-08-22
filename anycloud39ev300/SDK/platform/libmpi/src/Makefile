CUR_DIR:=$(shell pwd)
LOCAL_INC_DIR := $(CUR_DIR)/include $(PLAT_INC_DIR_INNER)
CFLAGS += $(addprefix -I,$(LOCAL_INC_DIR))

dir_y := venc
dir_y += md
dir_y += aenc
dir_y += adec
dir_y += osd
dir_y += muxer
dir_y += aed

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

.PHONY: all install clean
