#! /bin/sh
### BEGIN INIT INFO
# File:				rtl8188ftv.sh(wifi_driver.sh)	
# Provides:         8188 driver install and uninstall
# Required-Start:   $
# Required-Stop:
# Default-Start:     
# Default-Stop:
# Short-Description:install driver
# Author:			
# Email: 			
# Date:				2017-08-07
### END INIT INFO

PATH=$PATH:/bin:/sbin:/usr/bin:/usr/sbin
MODE=$1

usage()
{
	echo "Usage: $0 station | smartlink | ap | uninstall"
}

station_uninstall()
{
	#rm usb wifi driver(default)
	rmmod atbm603x_wifi_usb
	rmmod otg-hs
}

station_install()
{
    #install usb wifi driver(default)
	insmod /usr/modules/otg-hs.ko
	sleep 2 #### 等待otg 向usb host 核心层完成一些注册工作后再加在驱动
	insmod /usr/modules/atbm603x_wifi_usb.ko
}

smartlink_uninstall()
{
	#rm usb wifi driver(default)
	rmmod atbm603x_wifi_usb
	rmmod otg-hs
}

smartlink_install()
{
	insmod /usr/modules/otg-hs.ko 
	sleep 2 #### 等待otg 向usb host 核心层完成一些注册工作后再加在驱动
	insmod /usr/modules/atbm603x_wifi_usb.ko
}

ap_install()
{
	#install ap mode driver
	echo "install rtl8188ftv ap driver"
}

ap_uninstall()
{
	#uninstall ap mode driver
	echo "uninstall rtl8188ftv ap driver"
}

####### main

case "$MODE" in
	station)
		station_install
		;;
	smartlink)
		smartlink_install
		ifconfig wlan0 up
		iwconfig wlan0 mode monitor
		;;	
	ap)
		ap_install
		;;
	uninstall)
		station_uninstall
		smartlink_uninstall
		ap_uninstall
		;;
	*)
		usage
		;;
esac
exit 0


