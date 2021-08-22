dir_y := ai_demo
dir_y += ao_demo
dir_y += vi_demo
dir_y += vpss_demo
dir_y += aec_demo
dir_y += aec_ex_demo
dir_y += tw_demo

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
