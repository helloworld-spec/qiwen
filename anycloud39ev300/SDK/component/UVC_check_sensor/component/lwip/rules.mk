
# Cross Tools and libs
CROSS_PREFIX    ?= arm-anykav200-linux-uclibcgnueabi-

# Tools
CC         = $(CROSS_PREFIX)gcc
AS	  = $(CROSS_PREFIX)as
AR	  = $(CROSS_PREFIX)ar
LD	  = $(CROSS_PREFIX)ld
RM         = rm -rf
MKDIR      = mkdir
OBJDUMP    = $(CROSS_PREFIX)objdump
OBJCOPY	  = $(CROSS_PREFIX)objcopy


CFLAGS 	+= -mlittle-endian  -fno-builtin -nostdlib -O2 -mlong-calls $(INCLUDE) $(DEFINE)
ASFLAGS += -mlittle-endian -x assembler-with-cpp -O2 

# Rules

# --------------------------- s -> o
$(BUILDPATH)/%.o:%.s
	@echo ---------------------[build $<]----------------------------------
	$(CC) -c $(ASFLAGS) $(CFLAGS) -o $@ $<

# ----------------------------- c -> o
$(BUILDPATH)/%.o:%.c
	@echo ---------------------[build $<]----------------------------------
	$(CC) -c $(CFLAGS) -o $@ $<

