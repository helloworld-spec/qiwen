# Project name

ifdef AK_SPINAND
PROJ = spiboot_39xx_spinand
else
PROJ = spiboot_39xx
endif

TARGET= $(PROJ).bin


# INCLUDE and DEFINE

INCLUDE = -I. 

 
DEFINE		= -DOS_ANYKA=1
ifdef AK_SPINAND
	DEFINE      := $(DEFINE) -DCONFIG_SPI_NAND_FLASH
endif

ifdef AK_SPINAND
    TEXT         = 0x80eFF800
else
	TEXT         = 0x80DFFE00
endif
    


 
#path to build obj 

#COBJ and SOBJ
SRCDIR += .

#CSRCS += $(foreach d, $(SRCDIR), $(wildcard $d/*.c))
#COBJS_T += $(patsubst %.c, $(BUILDPATH)/%.o, $(CSRCS))

#SSRCS += $(foreach d, $(SRCDIR), $(wildcard $d/*.s))
#SOBJS += $(patsubst %.s, $(BUILDPATH)/%.o, $(SSRCS))

SOBJS	= rom.o mmulib.o
COBJS	= boot.o console.o L2.o mmu.o rtc.o spi.o spiflash.o eabi_compat.o
ifdef AK_SPINAND	
    COBJS   += spinand.o
endif


LIBS += $(ARM_LIBC_PATH)/libc.a  $(ARM_LIBGCC_PATH)/libgcc.a

#build rule
.PHONY: target clean 

target:$(TARGET)

$(TARGET): $(PROJ).elf
	$(OBJCOPY) -O binary $< $@

$(PROJ).elf: $(SOBJS) $(COBJS)
	$(LD) $(LDFLAGS) -o $(PROJ).elf $(SOBJS) $(COBJS) $(LIBS)
	#$(OBJDUMP) -h -S -l -D -Mreg-names-raw --show-raw-insn $(PROJ).elf > $(PROJ)_elf.txt

clean : 
	$(RM) $(TARGET) *.elf *.txt *.o  
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

CFLAGS  = -fno-builtin -mlittle-endian -nostdlib -g -O2 $(DEFINE)
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
