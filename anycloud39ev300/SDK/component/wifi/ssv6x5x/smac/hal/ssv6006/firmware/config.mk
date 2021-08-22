
ifeq ($(OS),Windows_NT)
CROSS_COMPILER ?= $(ANDES_GCC_ROOT)/bin/nds32le-elf-
# Export GNU_ARM_ROOT_WINDOWS for Lint to search includes files of the tool chain.
GNU_ARM_ROOT_WINDOWS = $(shell cygpath -m $(ANDES_GCC_ROOT))
export GNU_ARM_ROOT_WINDOWS 
ifdef PC_LINT_ROOT
PC_LINT_ROOT_WINDOWS = $(shell cygpath -m $(PC_LINT_ROOT))
export PC_LINT_ROOT_WINDOWS
LINT     = ./utility/lin_cabrio.bat
endif
else
CROSS_COMPILER ?= /opt/Andestech/BSPv420/toolchains/nds32le-elf-mculib-v3/bin/nds32le-elf-
PC_LINT_ROOT_WINDOWS = "c:/lint"
LINT     = ./utility/lintwine
endif

################################################################
# Define the make variables
################################################################
AS	 = $(CROSS_COMPILER)as
LD	 = $(CROSS_COMPILER)ld
CC	 = $(CROSS_COMPILER)gcc
CPP	 = $(CC) -E
AR	 = $(CROSS_COMPILER)ar
STRIP	 = $(CROSS_COMPILER)strip
OBJDUMP	 = $(CROSS_COMPILER)objdump
OBJCOPY	 = $(CROSS_COMPILER)objcopy
RANLIB	 = $(CROSS_COMPILER)RANLIB
RM	 = rm -rf
CP	 = cp -rf
MKDIR	 = mkdir -p
ifeq ($(OS),Windows_NT)
BIN2MIF = utility/bin2mif.exe
CYGPATH = cygpath -w 
else
BIN2MIF = utility/bin2mif
CYGPATH = winepath -w
endif
LOG_CC   = $(TOPDIR)/utility/log-cc
ifndef LINT_FILE_LIST
LINT_FILE_LIST = all_src
endif
LINT_OPT = $(LINT_FILE_LIST)

CONFIG_PATH	= $(TOPDIR)/include/config

PLATFORM_PATH   = $(BSP_PATH)/turismo
LDSCRIPT        = $(PLATFORM_PATH)/turismo.ld

CONFIG_H        = $(CONFIG_PATH)/turismo_cfg.h
HW_HAL_SRC	= $(PLATFORM_PATH)/turismo.c

OS_DEF		:= -DCONFIG_OS_FREERTOS

RTOS_PATH	= $(TOPDIR)/rtos/freertos
BSP_PATH        = $(TOPDIR)/bsp
APP_PATH	= $(TOPDIR)/app/turismo

INCLUDE += -I$(TOPDIR)
INCLUDE += -I$(TOPDIR)/include
INCLUDE += -I$(TOPDIR)/include/config
INCLUDE += -I$(BSP_PATH)
INCLUDE += -I$(BSP_PATH)/turismo
INCLUDE += -I$(BSP_PATH)/CPU_DEF
INCLUDE += -I$(APP_PATH)
INCLUDE += -I$(APP_PATH)/include
INCLUDE += -I$(RTOS_PATH)/kernel/Source/include
INCLUDE += -I$(RTOS_PATH)
INCLUDE += -I$(RTOS_PATH)/hal
INCLUDE += -I$(RTOS_PATH)/nds32
INCLUDE += -I$(TOPDIR)/driver
INCLUDE += -I$(TOPDIR)/libc
INCLUDE += -I$(TOPDIR)/cli
INCLUDE += -I$(TOPDIR)/../../../../include
INCLUDE += -I$(TOPDIR)/../../ssv6006c

ARFLAGS	 = crv

ifeq ($(DEBUG),1)
OPTIM	:= -O0 -g3
else
OPTIM	:= -Os -g3
endif

CPPFLAGS := -include $(CONFIG_H) \
            $(INCLUDE) \
            $(OS_DEF) \
            $(OPTIM) \
            $(OPTFLAGS) $(GLOBAL_DEF)
INPUT_CFLAGS := -Wall -fno-builtin -fomit-frame-pointer -funroll-loops -nostdlib \
	        -fno-strict-aliasing -ffunction-sections \
                $(CFLAGS_EXT)

LD_FLAGS	= -Bstatic -T $(LDSCRIPT) -o $(IMAGE_NAME).adx $(OPTIM) \
                  -Wl,-Map=$(IMAGE_NAME).map -fno-builtin -nostartfiles \
		  -nostdlib -static \


#                  -static -Wl,--gc-sections -Wl,--mno-ex9
#-nostdlib

AFLAGS		:= -fno-builtin


################################################################
export AS LD CC CPP AR STRIP OBJDUMP RANLIB RM
export ARFLAG LDSCRIPT CPPFLAGS LDFLAGS CFLAGS INPUT_CFLAGS
export AFLAGS
################################################################
