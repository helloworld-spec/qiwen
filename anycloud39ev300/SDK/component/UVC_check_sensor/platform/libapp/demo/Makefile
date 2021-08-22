#
#Makefile
#
CUR_DIR:=$(shell pwd)

dir_y := net
dir_y += wifi
dir_y += uvc


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
		$(MAKE) -C $$i  install; \
	done

clean: 
	@for i in $(dir_y); \
	do \
		$(MAKE) -C $$i  clean; \
	done

.PHONY: all install clean 

