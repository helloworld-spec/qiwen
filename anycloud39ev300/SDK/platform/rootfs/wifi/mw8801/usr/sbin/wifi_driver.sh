#! /bin/sh
### BEGIN INIT INFO
# File:				rtl8188.sh(wifi_driver.sh)	
# Provides:         8188 driver install and uninstall
# Required-Start:   $
# Required-Stop:
# Default-Start:     
# Default-Stop:
# Short-Description:install driver
# Author:			
# Email: 			
# Date:				2015-12-21
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
	rmmod sd8xxx.ko
	rmmod mlan.ko 
	rmmod sdio_wifi
}

station_install()
{
    #install usb wifi driver(default)
    insmod /usr/modules/sdio_wifi.ko
    sleep 1 #### 等待电源稳定后再加载其他驱动	
	insmod /usr/modules/mlan.ko
	sleep 2 
	insmod /usr/modules/sd8801.ko cfg80211_wext=0xf drv_mode=1
}

smartlink_uninstall()
{
	#rm usb wifi driver(default)
	rmmod sd8xxx.ko
	rmmod mlan.ko 
	rmmod sdio_wifi
}

smartlink_install()
{
    insmod /usr/modules/sdio_wifi.ko
	sleep 1 #### 等待电源稳定后再加载其他驱动
	insmod /usr/modules/mlan.ko 
	sleep 2 
	insmod /usr/modules/sd8801.ko cfg80211_wext=0xf drv_mode=1
}

ap_install()
{
	#install ap mode driver
	echo "install mw8801 ap driver"
}

ap_uninstall()
{
	#uninstall ap mode driver
	echo "uninstall mw8801 ap driver"
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


