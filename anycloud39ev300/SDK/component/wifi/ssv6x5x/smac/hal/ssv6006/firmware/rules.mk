ifeq (_all,$(MAKECMDGOALS))
ifneq (0,$(MAKELEVEL))
ifneq ("$(LOCAL_SRCS) $(LOCAL_ASMS)"," ")
.PHONY: dep
include .depend	
endif
endif
endif

CWD	= "`pwd`"

dep: 	Makefile $(TOPDIR)/config.mk  $(LOCAL_SRCS) $(LOCAL_ASMS)
	@for dir in $(SRC_DIRS) ; do \
		$(MAKE) --no-print-directory -C $$dir dep ; \
	done
	@$(RM) -f .depend 
	@for f in $(LOCAL_SRCS) $(LOCAL_ASMS); do \
		g=`basename $$f | sed -e 's/\(.*\)\.\w/\1.o/'`; \
		$(CC) -M $(CPPFLAGS) -MQ soc_obj/$$g $$f | cat >> .depend ; \
	done


#	@for dir in $(SRC_DIRS) ; do \
#		$(MAKE) --no-print-directory -C $$dir clean ; \

clean:
	@for dir in $(SRC_DIRS) ; do \
		$(MAKE) -C $$dir clean ; \
	done
	@$(RM) $(LOCAL_OBJS)
	@$(RM) .build
	@$(RM) .depend
	@$(RM) image/turismo-sw.*
	@$(RM) $(LINT_OUT) $(LINT_DEF_FILE) $(LINT_SRC_FILE)*


soc_obj/%.o:	%.S
	@if [ ! -d soc_obj ]; then $(MKDIR) soc_obj; fi
ifdef SHOW_CC_OPT
	$(CC) $(CFLAGS) $(AFLAGS) -c -o $@ $<
else
	@s=""; \
	 if [ $(MAKELEVEL) -gt 0 ]; then \
	     c=0; \
	     while [ $$c -lt $(MAKELEVEL) ]; do \
                s="$$s  "; \
                c=$$(($$c + 1)); \
             done; \
	 fi; \
	 echo "$${s} -> $(notdir $@)" 1>&2
	@$(CC) $(CFLAGS) $(AFLAGS) -c -o $@ $<
endif

soc_obj/%.o:	%.c
	@if [ ! -d soc_obj ]; then $(MKDIR) soc_obj; fi
ifdef SHOW_CC_OPT
	$(CC) $(CFLAGS) -c -o $@ $<
else
	@s=""; \
	 if [ $(MAKELEVEL) -gt 0 ]; then \
	     c=1; \
	     while [ $$c -lt $(MAKELEVEL) ]; do \
                s="$$s  "; \
                c=$$(($$c + 1)); \
             done; \
	 fi; \
	 echo "$${s} -> $(notdir $@)" 1>&2
	@$(CC) $(CFLAGS) -c -o $@ $<
endif
	@echo $(CWD)/$< >> $(LINT_SRC_FILE).lnx

#@$(CYGPATH) $(CWD)/$< >> $(LINT_SRC_FILE)

LOCAL_OBJS = $(addprefix soc_obj/, $(subst .S,.o,$(LOCAL_ASMS)) $(subst .c,.o,$(LOCAL_SRCS)))

PHONY += $(SRC_DIRS) _all

$(SRC_DIRS):
	@s=""; \
	 if [ $(MAKELEVEL) -gt 0 ]; then \
	     c=0; \
	     while [ $$c -lt $(MAKELEVEL) ]; do \
                s="$$s  "; \
                c=$$(($$c + 1)); \
             done; \
	 else \
	 echo ""; \
	 fi; \
	 echo "$${s}=> $@ ...";
	@$(MAKE) --no-print-directory -C $@ _all


_all: $(sort $(SRC_DIRS)) $(sort $(LOCAL_OBJS))
	@for f in $(LOCAL_OBJS) ; do \
		echo -n "$(subst $(TOPDIR),.,$(CURDIR)/$$f)" >> $(TOPDIR)/.build ; \
		echo -n " " >> $(TOPDIR)/.build; \
	done

	
.PHONY: $(PHONY)
