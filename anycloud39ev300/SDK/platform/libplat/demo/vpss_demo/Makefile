dir_y := effect_demo
dir_y += mask_demo
dir_y += md_demo
dir_y += od_demo

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

