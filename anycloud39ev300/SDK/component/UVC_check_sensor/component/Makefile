#
# toolchain
#

##rootfs dir MUST be placed at first
dir-y += akos
dir-y += drvlib
dir-y += ispdrv
dir-y += memlib
dir-y += lwip
dir-y += wifi

all:
	@for i in $(dir-y); \
	do \
		$(MAKE) -C $$i ; \
		if [ $$? -ne 0 ];then \
		    exit 1; \
		fi \
	done
install:
	@for i in $(dir-y); \
	do \
		$(MAKE) -C $$i install; \
	done
	

clean:
	@for i in $(dir-y); \
	do \
		$(MAKE) -C $$i clean; \
	done

.PHONY: all clean
