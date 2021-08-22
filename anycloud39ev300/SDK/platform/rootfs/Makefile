#
#Makefile
#

TEMP_DIR := ./temp_dir
ROOTFS_DIR := $(shell pwd)/rootfs
KO_SRC_DIR := $(shell pwd)/ko/
KO_DST_DIR := $(ROOTFS_DIR)/usr/modules
UTIL_SRC_DIR := $(shell pwd)/utils

BUILD_JFFS2 := ./create_jffs2fs.sh
BUILD_SQAUSHFS := ./create_squashfs.sh
BUILD_YAFFS2 := ./create_yaffs2fs.sh
BUILD_LINK := ./create_linkage.sh

include platform.cfg

all:
	@echo "Do Nothing for rootfs, exit..."

nandimage:
	@echo -e "\033[1;31m""building root.yaffs2 usr.yaffs2 jffs2.yaffs2 file system ...""\033[m"
	@rm -f usr.yaffs2 root.yaffs2 jffs2.yaffs2
	@$(BUILD_YAFFS2)

image:
	@[ -L "$(ROOTFS_DIR)/etc/passwd" ] || mv $(ROOTFS_DIR)/etc/passwd $(ROOTFS_DIR)/etc/jffs2/
	@[ -L "$(ROOTFS_DIR)/etc/shadow" ] || mv $(ROOTFS_DIR)/etc/shadow $(ROOTFS_DIR)/etc/jffs2/
	@[ -L "$(ROOTFS_DIR)/etc/passwd" ] || [ -L "$(ROOTFS_DIR)/etc/shadow" ] || $(BUILD_LINK)
	@echo "\033[1;31m""building usr.jffs2 file system ...""\033[m"
	@rm -f usr.jffs2
	@$(BUILD_JFFS2)
	@echo "\033[1;31m"'building root.sqsh4 usr.sqsh4 file system ...'"\033[m"
	@rm -f usr.sqsh4 root.sqsh4
	@$(BUILD_SQAUSHFS)

install: extract.sh rootfs.tar.gz
	@echo "CONFIG_DANA_SUPPORT=$(CONFIG_DANA_SUPPORT)"
	@echo "CONFIG_RTSP_SUPPORT=$(CONFIG_RTSP_SUPPORT)"
	@echo "CONFIG_UTILS_SUPPORT=$(CONFIG_UTILS_SUPPORT)"
	@rm -fr $(TEMP_DIR)
	@mkdir $(TEMP_DIR)
	@[ -d "rootfs" ] || ./extract.sh
	@[ -d $(KO_SRC_DIR) ] && \
		$(CP) -a $(KO_SRC_DIR)/* $(KO_DST_DIR)

	@for i in $(dir_y); \
	do	\
		$(CP) -a $$i/* $(TEMP_DIR); \
	done

ifeq ($(DEBUG), y)
		@$(CP) -a $(UTIL_SRC_DIR)/usr/bin/dmalloc $(ROOTFS_DIR)/usr/bin
endif

	@$(RM) $(TEMP_DIR)/*.txt
	@$(RM) $(TEMP_DIR)/README
	@$(RM) $(TEMP_DIR)/include
	@find $(TEMP_DIR) -type d -name "\.svn" | xargs rm -fr {}\;
	@find $(TEMP_DIR) -name "*.sh" | xargs chmod u+x;
	@$(CP) -a $(TEMP_DIR)/* $(ROOTFS_DIR)
	@$(RM) $(TEMP_DIR)

clean:
	@rm -fr $(TEMP_DIR)
	@rm -fr rootfs rootfs.initramfs *.yaffs2
	@rm -fr usr.jffs2 usr.sqsh4 root.sqsh4

demo app modules lib:

.PHONY:all install image nandimage clean demo app modules lib
