## This makefile use to make release only

PWD:= $(shell pwd)
RELEASE_DIR:= $(PWD)/SDK_Release_Cloud39ev300
COMPILE_SCRIPTS := $(PWD)/auto_build.sh
COMPONENT_DIR:=$(PWD)/component
KERNEL_DIR:=$(PWD)/kernel
PLATFORM_DIR:=$(PWD)/platform
BURNTOOL_DIR:=$(PWD)/tools/burntool

DIR_SUFFIX:=$(notdir $(PWD))
ifeq ("$(DIR_SUFFIX)", "SDK")
	ISPCONF_DIR:=$(PWD)/../Tools/tools/isptool
else
	ISPCONF_DIR:=$(PWD)/tools/isptool
endif

MAKE := make
MKDIR:= mkdir
RM:=rm
CP:=cp -af
SED := sed

release:check_release_dir
	@echo component entry
	@$(MAKE) -C $(COMPONENT_DIR) clean -j2 > /dev/null
	@$(MAKE) -C $(COMPONENT_DIR) -j2 > /dev/null
	@$(MAKE) -C $(COMPONENT_DIR) install -j2 > /dev/null
	@echo "kernel entry"
	@$(CP) $(KERNEL_DIR) $(RELEASE_DIR)
	@echo "platform entry"
	@$(MAKE) -C $(PLATFORM_DIR) clean -j2 > /dev/null
	@$(MAKE) -C $(PLATFORM_DIR) modules -j2 > /dev/null
	@$(MAKE) -C $(PLATFORM_DIR) lib -j2 > /dev/null
	@#echo "ispconf entry"
	@#find $(PLATFORM_DIR)/rootfs -maxdepth 1 -type d -name \
	 #"rootfs_*" -exec $(CP) $(ISPCONF_DIR)/isp_*.conf {}/usr/local \;
	@#echo "ispconf done"
	@$(CP) $(PLATFORM_DIR) $(RELEASE_DIR)
	@find $(RELEASE_DIR)/platform -type d -name "src" | xargs rm -rf {} >/dev/null 2>&1 \;
	@find $(RELEASE_DIR)/platform -type d -name "include_inner" | xargs rm -rf {} >/dev/null 2>&1 \;
	@$(CP) $(BURNTOOL_DIR) $(RELEASE_DIR)
	@find $(RELEASE_DIR)/ -name "*.svn" | xargs rm -rf {} > /dev/null 2>&1 \;
	@$(CP) $(COMPILE_SCRIPTS) $(RELEASE_DIR)
	@$(SED) -i "s/^\\s*PLAT_RELEASE.*/PLAT_RELEASE := y/" $(RELEASE_DIR)/platform/Makefile
	@echo -e "\033[1;32m""make release done, output in $(RELEASE_DIR)""\033[m"

check_release_dir:
	@$(MKDIR) -p $(RELEASE_DIR)
	@$(RM) -rf $(RELEASE_DIR)/*

.PHONY: all modules install image release app demo lib clean help
