#
#Makefile
#
CUR_DIR:=$(shell pwd)

dir_y := adec
dir_y += aenc
dir_y += md
dir_y += osd
dir_y += venc
dir_y += muxer
dir_y += login


all: 
	@for i in $(dir_y); \
	do \
		$(MAKE) -C $$i ; \
		if [ $$? -ne 0 ]; then\
		  exit 1;\
		fi \
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

