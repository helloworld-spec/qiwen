CUR_DIR:=$(shell pwd)
LOCAL_INC_DIR := $(CUR_DIR)/include
CFLAGS += $(addprefix -I,$(LOCAL_INC_DIR))

dir_y := common
dir_y += thread
dir_y += ai
dir_y += ao
dir_y += vi
dir_y += drv
dir_y += its
dir_y += vpss
dir_y += ipc_srv
dir_y += tw
dir_y += media_cbs
dir_y += ats
dir_y += vqe

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
