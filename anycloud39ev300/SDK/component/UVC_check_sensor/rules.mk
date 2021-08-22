
# Cross Tools and libs
CROSS_PATH      ?= /opt/arm-anykav200-crosstool/usr
CROSS_PREFIX    ?= arm-anykav200-linux-uclibcgnueabi-
ARM_LIBC_PATH	?=$(CROSS_PATH)/arm-anykav200-linux-uclibcgnueabi/sysroot/usr/lib
ARM_LIBGCC_PATH ?=$(CROSS_PATH)/lib/gcc/arm-anykav200-linux-uclibcgnueabi/4.8.5

# Tools
CC           = $(CROSS_PREFIX)gcc
AS           = $(CROSS_PREFIX)as
AR           = $(CROSS_PREFIX)ar
LD           = $(CROSS_PREFIX)ld
RM           = rm -rf
MKDIR        = mkdir
OBJDUMP      = $(CROSS_PREFIX)objdump
OBJCOPY	     = $(CROSS_PREFIX)objcopy


CFLAGS += -std=c99 -mlittle-endian  -fno-builtin -nostdlib -O2 -mlong-calls $(DEFINE) $(INCLUDE)
ASFLAGS += -mlittle-endian -x assembler-with-cpp -O2 

export CLIB := $(ARM_LIBC_PATH)/libm.a $(ARM_LIBC_PATH)/libc.a  $(ARM_LIBGCC_PATH)/libgcc.a


# Rules


# --------------------------- s -> o
%.o:%.s
	@echo ---------------------[build $<]----------------------------------
	$(CC) -c $(ASFLAGS) $(CFLAGS) -o $@ $<

# ----------------------------- c -> o
%.o:%.c
	@echo ---------------------[build $<]----------------------------------
	$(CC) -c $(CFLAGS) $(INCLUDE) -o $@ $<

