#
#Makefile
#


##rootfs dir MUST be placed at first
dir_y += libapp
dir_y += libmpi
dir_y += libplat


all: 
	@for i in $(dir_y); \
	do \
		$(MAKE) -C $$i  ; \
		if [ $$? -ne 0 ]; then\
		  exit 1;\
		fi \
	done

install:
	@for i in $(dir_y); \
	do \
		$(MAKE) -C $$i  install; \
	done

clean:
	@for i in $(dir_y); \
	do \
		$(MAKE) -C $$i  clean; \
	done


.PHONY: all install clean
