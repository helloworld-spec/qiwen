ifeq ($(KBUILD_TOP),)
    ifneq ($(KBUILD_EXTMOD),)
    KBUILD_DIR := $(KBUILD_EXTMOD)
    else
    KBUILD_DIR := $(PWD)
    endif
KBUILD_TOP := $(KBUILD_DIR)/../
endif

include $(KBUILD_TOP)/config.mak


KBUILD_EXTRA_SYMBOLS += $(KBUILD_TOP)/ssvdevice/Module.symvers
KBUILD_EXTRA_SYMBOLS += $(KBUILD_TOP)/hci/Module.symvers

#obj-$(CONFIG_MAC80211) += mac80211.o

# mac80211 objects
mac80211-y := main.c 
mac80211-y += status.c
mac80211-y += driver-ops.c
mac80211-y += sta_info.c
mac80211-y += wep.c
mac80211-y += wpa.c
mac80211-y += scan.c 
mac80211-y += offchannel.c
mac80211-y += ht.c 
mac80211-y += agg-tx.c 
mac80211-y += agg-rx.c
mac80211-y += vht.c 
mac80211-y += ibss.c
mac80211-y += iface.c
mac80211-y += rate.c
mac80211-y += michael.c
mac80211-y += tkip.c
mac80211-y += aes_ccm.c
mac80211-y += aes_gcm.c
mac80211-y += aes_cmac.c
mac80211-y += aes_gmac.c
mac80211-y += cfg.c
mac80211-y += ethtool.c
mac80211-y += rx.c
mac80211-y += spectmgmt.c
mac80211-y += tx.c
mac80211-y += key.c
mac80211-y += util.c
mac80211-y += wme.c
mac80211-y += chan.c
mac80211-y += trace.c
mac80211-y += mlme.c
mac80211-y += tdls.c
mac80211-y += ocb.c
#mac80211-y += lib/rhashtable.c

mac80211-$(CONFIG_MAC80211_LEDS) += led.c
mac80211-$(CONFIG_MAC80211_DEBUGFS) += \
	debugfs.c \
	debugfs_sta.c \
	debugfs_netdev.c \
	debugfs_key.c

mac80211-$(CONFIG_MAC80211_MESH) += \
	mesh.c \
	mesh_pathtbl.c \
	mesh_plink.c \
	mesh_hwmp.c \
	mesh_sync.c \
	mesh_ps.c

mac80211-$(CONFIG_PM) += pm.c

CFLAGS_trace.c := -I$(src)

rc80211_minstrel-y := rc80211_minstrel.c
rc80211_minstrel-$(CONFIG_MAC80211_DEBUGFS) += rc80211_minstrel_debugfs.c

rc80211_minstrel_ht-y := rc80211_minstrel_ht.c
rc80211_minstrel_ht-$(CONFIG_MAC80211_DEBUGFS) += rc80211_minstrel_ht_debugfs.c

mac80211-$(CONFIG_MAC80211_RC_MINSTREL) += $(rc80211_minstrel-y)
mac80211-$(CONFIG_MAC80211_RC_MINSTREL_HT) += $(rc80211_minstrel_ht-y)

KMODULE_NAME=ssv_mac80211
KERN_SRCS=$(mac80211-y)

include $(KBUILD_TOP)/rules.mak
