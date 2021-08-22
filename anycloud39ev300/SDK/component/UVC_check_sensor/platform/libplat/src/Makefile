#
#Makefile
#
CUR_DIR:=$(shell pwd)

dir_y := ai
dir_y += ao
dir_y += common
dir_y += drv
dir_y += thread
dir_y += partition
dir_y += vi
dir_y += vpss
dir_y += its
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

