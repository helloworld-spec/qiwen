# Project name

PROJ = akos

TARGET= $(PROJ).a

LIBDIR = ../../kernel/lib

# INCLUDE and DEFINE

INCLUDE    	= -I. -I./core 

DEFINE		= -DOS_ANYKA=1

ifdef MEMLEAK
ifeq ($(MEMLEAK),1)
TARGET		:= $(PROJ)_ml.a
DEFINE		+= -DDEBUG_TRACE_MEMORY_LEAK=1
endif
endif

DEFINE		+= -DDEBUG_TRACE_TASK_INFO=1

DEFINE		+= -DNU_DEBUG=1 -DNU_ERROR_STRING=1 -DNU_ENABLE_STACK_CHECK=1

#path to build obj 
BUILDPATH = build


#COBJ and SOBJ
#user set the soure file path here
SRCDIR += .
SRCDIR += core

CSRCS += $(foreach d, $(SRCDIR), $(wildcard $d/*.c))
COBJS += $(patsubst %.c, $(BUILDPATH)/%.o, $(CSRCS))

SSRCS_T += $(foreach d, $(SRCDIR), $(wildcard $d/*.s))
SSRCS_ADS += $(foreach d, $(SRCDIR), $(wildcard $d/*_ads.s))
SSRCS = $(filter-out $(SSRCS_ADS), $(SSRCS_T))

SOBJS += $(patsubst %.s, $(BUILDPATH)/%.o, $(SSRCS))


#build rule
.PHONY: all prepare target clean install

all: prepare target 

prepare: 
ifneq ($(BUILDPATH),)
	@for i in $(SRCDIR); \
	do mkdir -p $(BUILDPATH)/$$i; \
	done
endif
	
target: $(BUILDPATH)/$(TARGET)

$(BUILDPATH)/$(TARGET):  $(SOBJS) $(COBJS) 
	@echo ---------------------[build $@]----------------------------------	
	$(AR) -rsv $@ $(COBJS) $(SOBJS)

install :
	@if [ ! -e $(BUILDPATH)/$(TARGET) ];then\
		echo "\033[1;31m"$(TARGET) is not exist"\033[m";\
		/bin/false;	\
	fi
	@echo "\033[1;32m"install $(BUILDPATH)/$(TARGET) to $(LIBDIR)"\033[m"
	cp $(BUILDPATH)/$(TARGET) $(LIBDIR)

clean : 
	$(RM) $(BUILDPATH)


# Cross Tools and flgs
CROSS_PREFIX    ?= arm-anykav200-linux-uclibcgnueabi-

CC		= $(CROSS_PREFIX)gcc
AS	     	= $(CROSS_PREFIX)as
AR           	= $(CROSS_PREFIX)ar
LD	     	= $(CROSS_PREFIX)ld
RM	     	= rm -rf
MKDIR      	= mkdir
OBJDUMP      	= $(CROSS_PREFIX)objdump
OBJCOPY	     	= $(CROSS_PREFIX)objcopy

CFLAGS 	= -mlittle-endian -fno-builtin -nostdlib  -O2  -mlong-calls $(INCLUDE) $(DEFINE)
ASFLAGS = -mlittle-endian -x assembler-with-cpp -O2 

# Rules

# --------------------------- s -> o
$(BUILDPATH)/%.o:%.s
	@echo ---------------------[$<]----------------------------------
	$(CC) -c $(CFLAGS) $(ASFLAGS) -o $@ $<

# ----------------------------- c -> o
$(BUILDPATH)/%.o:%.c
	@echo ---------------------[$<]----------------------------------
	$(CC) -c $(CFLAGS)  -o $@ $<	

