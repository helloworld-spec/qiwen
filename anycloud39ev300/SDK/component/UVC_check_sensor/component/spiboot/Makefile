# Project name

PROJ = spiboot_39xx

TARGET= $(PROJ).bin


# INCLUDE and DEFINE

INCLUDE = -I. 

 
DEFINE		= -DOS_ANYKA=1
ifdef MTD_NFTL
    DEFINE := $(DEFINE) -DMTD_NFTL=1
endif
    
DEFINE		:= $(DEFINE) -DFREE_MEM_START_ADDR=$(FREE_MEM_START_ADDR)

ifdef DEBUG_OUTPUT
ifeq ($(DEBUG_OUTPUT), Y)
DEFINE		:= $(DEFINE) -DDEBUG_OUTPUT=1
endif
endif

TEXT         = 0x80DFFE00

 
#path to build obj 
#BUILDPATH = build

#COBJ and SOBJ
SRCDIR += .

#CSRCS += $(foreach d, $(SRCDIR), $(wildcard $d/*.c))
#COBJS_T += $(patsubst %.c, $(BUILDPATH)/%.o, $(CSRCS))

#SSRCS += $(foreach d, $(SRCDIR), $(wildcard $d/*.s))
#SOBJS += $(patsubst %.c, $(BUILDPATH)/%.o, $(SSRCS))

SOBJS	= rom.o mmulib.o
COBJS	= main.o uart.o spi.o spiflash.o l2.o fwl_spiflash.o freq.o memapi.o mmu.o eabi_compat.o

ifdef COMPRESS
ifeq ($(COMPRESS), Y)
DEFINE		:= $(DEFINE) -DCOMPRESS=1
LIBS		:= $(LIBS)	 ./inflate.a
COBJS		:= $(COBJS)	 Fwl_compress.o
endif
endif

LIBS += $(ARM_LIBC_PATH)/libc.a  $(ARM_LIBGCC_PATH)/libgcc.a

#build rule
.PHONY: all target prepare clean 

all: prepare target 

prepare: 
ifneq ($(BUILDPATH),)
	@for i in $(SRCDIR); \
	do mkdir -p $(BUILDPATH)/$$i; \
	done
endif
	
target:$(TARGET)

$(TARGET): $(PROJ).elf
	$(OBJCOPY) -O binary $< $@

$(PROJ).elf: $(SOBJS) $(COBJS)
	$(LD) $(LDFLAGS) -o $(PROJ).elf $(SOBJS) $(COBJS) $(LIBS)
	#$(OBJDUMP) -h -S -l -D -Mreg-names-raw --show-raw-insn $(PROJ).elf > $(PROJ)_elf.txt

clean : 
	$(RM) $(TARGET) $(CBOJS) $(SOBJS)
	$(RM) $(BUILDPATH)


# Tools and flags
CROSS_PATH      ?= /opt/arm-anykav200-crosstool/usr
CROSS_PREFIX    ?= arm-anykav200-linux-uclibcgnueabi-
ARM_LIBC_PATH	?=$(CROSS_PATH)/arm-anykav200-linux-uclibcgnueabi/sysroot/usr/lib
ARM_LIBGCC_PATH ?=$(CROSS_PATH)/lib/gcc/arm-anykav200-linux-uclibcgnueabi/4.8.5

CC	= $(CROSS_PREFIX)gcc 
AS	= $(CROSS_PREFIX)as
AR	= $(CROSS_PREFIX)ar
LD	= $(CROSS_PREFIX)ld
RM	= rm -rf
MKDIR	= mkdir

OBJDUMP	= $(CROSS_PREFIX)objdump
OBJCOPY	= $(CROSS_PREFIX)objcopy

CFLAGS	= -fno-builtin -mlittle-endian -mlong-calls -nostdlib -g -O2 $(DEFINE)
ASFLAGS =  -x assembler-with-cpp $(DEFINE)
LDFLAGS = -e_start -EL -N -p -X -Ttext $(TEXT) \
			$(TOOLLDFLAGS) -n -nostartfiles -L. -T boot.ld

# Rules
# --------------------------- s -> o
%.o:%.s
	@echo ---------------------[$<]----------------------------------
	$(CC) -c $(CFLAGS) $(ASFLAGS) -o $@ $<

# ----------------------------- c -> o
%.o:%.c
	@echo ---------------------[$<]----------------------------------
	$(CC) -c $(CFLAGS)  -o $@ $<	
