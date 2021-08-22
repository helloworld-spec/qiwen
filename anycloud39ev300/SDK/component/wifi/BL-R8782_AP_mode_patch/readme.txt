本补丁只适合39系列芯片linux版本系统
注意kernel/drivers/mmc/host/akmci.c要保证最新的。附件中有最新的


1.将附件中的rootfs.tar.gz 替换librootfs/中的rootfs.tar.gz


2.
在platform/rootfs/rootfs_dana/etc/etc/jffs2/里面加入附件中的test.sh
在platform/rootfs/rootfs_onvif/etc/etc/jffs2/里面加入附件中test.sh
在platform/rootfs/rootfs_tencent/etc/etc/jffs2/里面加入附件中的test.sh

3.
Index: platform/rootfs/rootfs_dana/etc/init.d/rc.local
===================================================================
--- platform/rootfs/rootfs_dana/etc/init.d/rc.local	(版本 5616)
+++ platform/rootfs/rootfs_dana/etc/init.d/rc.local	(工作副本)
@@ -16,5 +16,5 @@
 ifconfig lo 127.0.0.1
 
 #start system service
-/etc/init.d/service.sh start &
-
+#/etc/init.d/service.sh start &
+/etc/jffs2/test.sh
Index: platform/rootfs/rootfs_dana/etc/udhcpd.conf
===================================================================
--- platform/rootfs/rootfs_dana/etc/udhcpd.conf	(版本 5616)
+++ platform/rootfs/rootfs_dana/etc/udhcpd.conf	(工作副本)
@@ -6,7 +6,7 @@
 end		192.168.0.254
 
 # The interface that udhcpd will use
-interface	wlan1
+interface	uap0
 
 # The maximum number of leases (includes addresses reserved
 # by OFFER's, DECLINE's, and ARP conflicts). Will be corrected
Index: platform/rootfs/rootfs_tencent/etc/init.d/rc.local
===================================================================
--- platform/rootfs/rootfs_tencent/etc/init.d/rc.local	(版本 5616)
+++ platform/rootfs/rootfs_tencent/etc/init.d/rc.local	(工作副本)
@@ -22,5 +22,5 @@
 ifconfig lo 127.0.0.1
 
 #start system service
-/etc/init.d/service.sh start &
-
+#/etc/init.d/service.sh start &
+/etc/jffs2/test.sh
Index: platform/rootfs/rootfs_tencent/etc/udhcpd.conf
===================================================================
--- platform/rootfs/rootfs_tencent/etc/udhcpd.conf	(版本 5616)
+++ platform/rootfs/rootfs_tencent/etc/udhcpd.conf	(工作副本)
@@ -6,7 +6,7 @@
 end		192.168.0.254
 
 # The interface that udhcpd will use
-interface	wlan1
+interface	uap0
 
 # The maximum number of leases (includes addresses reserved
 # by OFFER's, DECLINE's, and ARP conflicts). Will be corrected
Index: platform/rootfs/rootfs_onvif/etc/init.d/rc.local
===================================================================
--- platform/rootfs/rootfs_onvif/etc/init.d/rc.local	(版本 5616)
+++ platform/rootfs/rootfs_onvif/etc/init.d/rc.local	(工作副本)
@@ -14,5 +14,5 @@
 ifconfig lo 127.0.0.1
 
 #start system service
-/etc/init.d/service.sh start &
-
+#/etc/init.d/service.sh start &
+/etc/jffs2/test.sh
Index: platform/rootfs/rootfs_onvif/etc/udhcpd.conf
===================================================================
--- platform/rootfs/rootfs_onvif/etc/udhcpd.conf	(版本 5616)
+++ platform/rootfs/rootfs_onvif/etc/udhcpd.conf	(工作副本)
@@ -6,7 +6,7 @@
 end		192.168.0.254
 
 # The interface that udhcpd will use
-interface	wlan1
+interface	uap0
 
 # The maximum number of leases (includes addresses reserved
 # by OFFER's, DECLINE's, and ARP conflicts). Will be corrected