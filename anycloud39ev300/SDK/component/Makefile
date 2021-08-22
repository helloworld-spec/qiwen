
dir_y:=ispdrv_lib
dir_y+=ispsdk_lib
dir_y+=uiolib

all:
	@for i in $(dir_y);\
	do\
		$(MAKE) -C $$i;\
	done

install:
	@for i in $(dir_y);\
	do\
		$(MAKE) -C $$i install;\
	done

clean:
	@for i in $(dir_y);\
	do\
		$(MAKE) -C $$i clean;\
	done

help:
	@echo "Generic targets:"
	@echo "    all         - Build for target"
	@echo "    install     - To install libs to it's target directory"
	@echo "    clean       - To clean up all target file and temp file"

.PHONY: all install clean
