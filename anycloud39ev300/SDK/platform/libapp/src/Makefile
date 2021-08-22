CFLAGS += $(addprefix -I,$(PLAT_INC_DIR_INNER))

dir_y := ini
dir_y += osd_ex
dir_y += dvr
dir_y += rtsp
dir_y += net
dir_y += alarm

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
