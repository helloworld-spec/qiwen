##############################################################
# Global Variables
##############################################################
TOPDIR          := $(CURDIR)

export TOPDIR

ifeq ($(OS),Windows_NT)
TOPDIR_WIN := $(shell cygpath -m $(TOPDIR))
else
TOPDIR_WIN := $(TOPDIR)
endif

export TOPDIR_WIN

include ./config.mk

##############################################################
# Local Variables
##############################################################
IMAGE_DIR       := image
DOT_CONFIG      := .config
CONFIG_DIR      := defconfig
CONFIG_TARGET   := defconfig
MAKE_LOG        := make.log
LINT_DEF_FILE   := $(TOPDIR)/defs.lnt
LINT_SRC_FILE   := $(TOPDIR)/all_src.lnt
LINT_OUT        := lint_out.txt
STRIP_SH        := $(TOPDIR)/scripts/strip_dmsg.sh
TARGET_DIR      := ../../../../image ../../image /lib/firmware

export LINT_SRC_FILE

conf_exist      := $(shell [ ! -f "$(DOT_CONFIG)" ] || echo "y" )

ifeq ($(TARGET),)
include defconfig/6006_fpga.cfg
#$(DOT_CONFIG)
#ifeq ($(conf_exist),y)
TARGET          = $(CONFIG_TARGET)
HCI_TYPE	= $(CONFIG_HCI)
IMAGE_NAME	= $(addprefix $(IMAGE_DIR)/$(TARGET)-, $(HCI_TYPE))
TARGET_FLAGS	= $(CONFIG_CFLAGS)

CPPFLAGS += -D$(TARGET_FLAGS)
#endif
endif

CFLAGS   = $(CPPFLAGS) $(INPUT_CFLAGS)

CONFIG_FILES    := $(addprefix $(CONFIG_DIR)/, \
                   $(subst $(CONFIG_TARGET),,$(shell ls -1 $(TOPDIR)/$(CONFIG_DIR))))
TARGET_LIST     := $(shell cat $(CONFIG_FILES) | \
                     grep -e '^CONFIG_TARGET[[:blank:]]*=[[:blank:]]*' | \
                     sed 's/=/ /g' | awk '{ print $$2 }')

##############################################################
# Source directories to Make
##############################################################
SRC_DIRS        := bsp
SRC_DIRS        += rtos
SRC_DIRS        += libc
SRC_DIRS        += driver
SRC_DIRS        += app
# SRC_DIRS	+= cli

.PHONY: all config target_list target_check dep binary

#all: dep target_check $(if $(TARGET), $(sort $(SRC_DIRS)), )
all: dep target_check $(if $(TARGET), $(SRC_DIRS), )
	@$(MAKE) --no-print-directory -C $(TOPDIR) binary

ifeq (binary,$(MAKECMDGOALS))	

all_objs := $(shell cat .build)

binary: $(IMAGE_NAME).bin
	@if [ -f $< ]; then \
	    echo "\n$< is updated.\n"; \
	fi

$(IMAGE_NAME).bin: $(all_objs) $(LDSCRIPT)
	@if [ "$(TARGET)" = "" ] ; then \
	    echo "No Target is specified !!"; \
	    exit 0; \
	fi
#	@$(RM) version.o verion.c
#	@./ver_firmware.pl > version.c
#	@echo "const char *compiler = \"$(shell $(CC) --version | sed -n '1p')\";\n" >> version.c
#	@$(CC) -c version.c -o version.o
	@echo "\nLinking $(TARGET) as $(IMAGE_NAME).adx...\n"
#	$(CC) $(CFLAGS) $(LD_FLAGS) -Wl,--start-group $(all_objs) version.o -Wl,--end-group 
	$(CC) $(CFLAGS) $(LD_FLAGS) -Wl,--start-group $(all_objs) -Wl,--end-group 
	@$(OBJCOPY) -O binary $(IMAGE_NAME).adx $(IMAGE_NAME).bin
	@$(OBJDUMP) -D -S $(IMAGE_NAME).adx > $(IMAGE_NAME).asm
	@echo "Done.\n"
#	#@${BIN2MIF} $(IMAGE_NAME).bin $(IMAGE_NAME).mif 32
#	@cd $(TOPDIR) && $(STRIP_SH)

endif

scratch: .config
	@$(RM) $(MAKE_LOG)
	@echo "Cleaning ..."
	@$(MAKE) clean
	@echo "Building ..."
	@echo "CFLAGS = $(CFLAGS)"
	@echo $(INCLUDE) $(GLOBAL_DEF) > defs.lnt
	@$(MAKE) all 2>&1 | $(LOG_CC) $(MAKE_LOG)
	#@$(MAKE) install

.config: config

config:
	@if [ "$(TARGET)" != "" ] ; then \
	    for f in $(CONFIG_FILES) ; do \
		if [ "$$f" = "$(CONFIG_DIR)/$(TARGET)" ] ; then \
		    echo "copy $$f to $(DOT_CONFIG) ..."; \
		    $(CP) $$f $(DOT_CONFIG); \
		fi; \
	    done ; \
	elif [ -f $(CONFIG_DIR)/$(CONFIG_TARGET) ] ; then \
	    $(CP) $(CONFIG_DIR)/$(CONFIG_TARGET) $(DOT_CONFIG); \
	    echo "Copy $(CONFIG_DIR)/$(CONFIG_TARGET) to $(DOT_CONFIG) ..."; \
	else \
	    echo "Usage: make config TARGET=<TARGET_NAME)"; \
	    echo "       OR"; \
	    echo "       copy your target config file to $(CONFIG_DIR)/$(CONFIG_TARGET)"; \
	fi


target_list:
	@$(TOPDIR)/scripts/show-targets.sh $(TOPDIR)/defconfig/

target_check:
	@if [ "$(TARGET)" = "" ]; then \
	    echo "Please 'make config' first!!"; \
	    exit 0; \
	fi
	@$(RM) -rf .build
	@echo "make target '$(TARGET)'"

mrproper: clean
	@$(RM) .config
	@$(RM) $(MAKE_LOG) 

install:
	@for dir in $(TARGET_DIR); do \
	    if [ "$$dir" != "" -a -d "$$dir" ]; then \
	        echo "Copy $(IMAGE_NAME).bin to $$dir"; \
	        cp $(IMAGE_NAME).bin $$dir; \
	    fi; \
	done

lint:
	@$(RM) $(LINT_OUT) $(LINT_SRC_FILE)
	@cygpath_exec=`echo $(CYGPATH) | cut -d ' ' -f1`; which $$cygpath_exec > /dev/null || (echo No $$cygpath_exec; exit 1;)
	@for lnx_src in `cat $(LINT_SRC_FILE).lnx`; do\
	    $(CYGPATH) $$lnx_src >> $(LINT_SRC_FILE); \
	 done
	@echo $(GLOBAL_DEF) >  $(LINT_DEF_FILE)
	@for inc in $(subst -I,,$(INCLUDE)); do \
	    echo -n '-I' >> $(LINT_DEF_FILE); \
	    $(CYGPATH) $$inc >> $(LINT_DEF_FILE); \
	done
ifdef SHOW_LINT
	$(LINT) $(LINT_OPT)
else
	$(LINT) $(LINT_OPT) -os"($(LINT_OUT))"
endif

##############################################################
# Make rules
##############################################################
include ./rules.mk

