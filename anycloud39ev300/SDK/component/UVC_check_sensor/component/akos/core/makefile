# Project name

PROJ = nucleus 

TARGET= akos.a

# Pathes and Locations

ifndef SDKPATH
SDKPATH = vmesdk
endif
ifndef BUILDPATH
BUILDPATH  = build
endif
ifndef M3LIBPATH
M3LIBPATH  = c:/cygwin/tools
endif

# Flags

INCLUDE    = -I. 

TEXT	     = 0x10000000
ENDIANELF	= elf32-little

ifeq ($(CFG),debmemleak)
DEFINE		= -DOS_VME=1 -DDEBUG_TRACE_MEMORY_LEAK=1 -DDEBUG_TRACE_LINE=1
else
DEFINE		= -DOS_VME=1
endif

ifeq ($(CFG),Release)
CFLAGS		= -O2 -apcs /interwork -cpu 4T -W -Ec $(DEFINE) $(INCLUDE)
ASFLAGS         = -apcs /interwork -cpu 4T
else
CFLAGS		= -O2 -g -apcs /interwork -cpu 4T -Ec $(DEFINE) $(INCLUDE)
ASFLAGS         = -g -apcs /interwork -cpu 4T
endif

# Tools
CC	= armcc
AS	= armasm
LD	= armlink
RM	= rm -rf
MKDIR	= mkdir
OBJDUMP	= arm-elf-objdump
OBJCOPY	= arm-elf-objcopy

#Locations
LIBS		= #\
			#$(M3LIBPATH)/H-i686-pc-cygwin/arm-elf/lib/libc.a  \
		 	#$(M3LIBPATH)/H-i686-pc-cygwin/lib/gcc-lib/arm-elf/3.0/libgcc.a

#build rule
.PHONY: clean makedirs maketarget debug release help msi

debug:
	$(MAKE) makedirs BUILDPATH=$(BUILDPATH)/deb  
	$(MAKE) msi
	$(MAKE) maketarget BUILDPATH=$(BUILDPATH)/deb

release:
	$(MAKE) makedirs BUILDPATH=$(BUILDPATH)/rel
	$(MAKE) msi
	$(MAKE) maketarget CFG=Release BUILDPATH=$(BUILDPATH)/rel

debmemleak:
	$(MAKE) makedirs BUILDPATH=$(BUILDPATH)/deb  
	$(MAKE) msi
	$(MAKE) maketarget CFG=debmemleak BUILDPATH=$(BUILDPATH)/deb

maketarget: $(BUILDPATH)/$(TARGET)

$(BUILDPATH): 
	-@$(MKDIR) -p $@

BUILDDIRS = 
$(BUILDDIRS):
	-@$(MKDIR) $@
makedirs: $(BUILDPATH) $(BUILDDIRS)

SOBJS = $(BUILDPATH)/tct.o $(BUILDPATH)/tmt.o  

COBJS =  $(BUILDPATH)/csc.o $(BUILDPATH)/dmc.o $(BUILDPATH)/dmce.o $(BUILDPATH)/dmd.o\
 $(BUILDPATH)/dmf.o $(BUILDPATH)/dmi.o $(BUILDPATH)/dms.o $(BUILDPATH)/erc.o $(BUILDPATH)/erd.o\
 $(BUILDPATH)/eri.o $(BUILDPATH)/evc.o $(BUILDPATH)/evce.o $(BUILDPATH)/evd.o $(BUILDPATH)/evf.o\
 $(BUILDPATH)/evi.o $(BUILDPATH)/hic.o $(BUILDPATH)/hid.o $(BUILDPATH)/hii.o $(BUILDPATH)/inc.o\
 $(BUILDPATH)/ioc.o $(BUILDPATH)/ioce.o $(BUILDPATH)/iod.o $(BUILDPATH)/iof.o $(BUILDPATH)/ioi.o\
 $(BUILDPATH)/lic.o $(BUILDPATH)/lid.o $(BUILDPATH)/mbc.o $(BUILDPATH)/mbce.o $(BUILDPATH)/mbd.o\
 $(BUILDPATH)/mbf.o $(BUILDPATH)/mbi.o $(BUILDPATH)/mbs.o $(BUILDPATH)/mbse.o $(BUILDPATH)/pic.o\
 $(BUILDPATH)/pice.o $(BUILDPATH)/pid.o $(BUILDPATH)/pif.o $(BUILDPATH)/pii.o $(BUILDPATH)/pis.o\
 $(BUILDPATH)/pise.o $(BUILDPATH)/pmc.o $(BUILDPATH)/pmce.o $(BUILDPATH)/pmd.o $(BUILDPATH)/pmf.o\
 $(BUILDPATH)/pmi.o $(BUILDPATH)/quc.o $(BUILDPATH)/quce.o $(BUILDPATH)/qud.o $(BUILDPATH)/quf.o\
 $(BUILDPATH)/qui.o $(BUILDPATH)/qus.o $(BUILDPATH)/quse.o $(BUILDPATH)/rlc.o $(BUILDPATH)/rld.o\
 $(BUILDPATH)/sdc.o $(BUILDPATH)/smc.o $(BUILDPATH)/smce.o $(BUILDPATH)/smd.o $(BUILDPATH)/smf.o\
 $(BUILDPATH)/smi.o $(BUILDPATH)/sms.o $(BUILDPATH)/smse.o $(BUILDPATH)/tcc.o $(BUILDPATH)/tcce.o\
 $(BUILDPATH)/tcd.o $(BUILDPATH)/tcf.o $(BUILDPATH)/tcfe.o $(BUILDPATH)/tci.o $(BUILDPATH)/tcs.o\
 $(BUILDPATH)/tcse.o $(BUILDPATH)/tmc.o $(BUILDPATH)/tmd.o $(BUILDPATH)/tmf.o $(BUILDPATH)/tmi.o\
 $(BUILDPATH)/tms.o $(BUILDPATH)/tmse.o

$(BUILDPATH)/$(TARGET):  $(SOBJS) $(COBJS) $(LIBS)
	@echo ---------------------[build out]----------------------------------	

	
ifeq ($(CFG),Release)
	$(AR) -rsv $(BUILDPATH)/$(TARGET) $(SOBJS) $(COBJS) $(LIBS)
else
	$(AR) -rsv $(BUILDPATH)/$(TARGET) $(SOBJS) $(COBJS) $(LIBS)
endif

clean : 
	-@$(RM) $(BUILDPATH)

help:
	@echo "Usage:   make [TARGET] [VARIABLE=XXX]"
	@echo "TARGET:" 
	@echo "         debug:   Builds a debug version, default target"
	@echo "         release: Builds a release version"
	@echo "         clean:   Remove all created objects "
	@echo "VARIABLES:" 
	@echo "         BUILDPATH: Directory of all build objects,"
	@echo "                    e.g. BUILDPATH = c:/build, default: BUILDPATH=build"
	@echo "                    Don't use a backslash!"

# Rules


# --------------------------- s -> o
$(BUILDPATH)/%.o:%.s
	@echo ---------------------[$<]----------------------------------
	$(AS) $(ASFLAGS) -o $@ $<

# ----------------------------- c -> d
$(BUILDPATH)/%.o:%.c
	@echo ---------------------[$<]----------------------------------
	$(CC) -c $(CFLAGS)  -o $@ $<	

