#! /bin/sh
### BEGIN INIT INFO
# File:				ssv6x5x.sh(wifi_driver.sh)	
# Provides:         ssv6x5x driver install and uninstall
# Required-Start:   $
# Required-Stop:
# Default-Start:     
# Default-Stop:
# Short-Description:install driver
# Author:			
# Email: 			
# Date:				2018-04-25
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
	rmmod ssv6x5x
	#rmmod otg-hs
	#rmmod sdio_wifi
}

station_install()
{
	#install usb wifi driver(default)
	#insmod /usr/modules/sdio_wifi.ko
	sleep 1 #### 等待电源稳定后再加载其他驱动	
	insmod /usr/modules/otg-hs.ko
	sleep 2 #### 等待otg 向usb host 核心层完成一些注册工作后再加在驱动
	insmod /usr/modules/ssv6x5x.ko stacfgpath=/etc/jffs2/ak3916-wifi.cfg
	sleep 2 #### 等待ssv drv核心层完成一些注册工作后再加在驱动	
}

smartlink_uninstall()
{
	#rm usb wifi driver(default)
	rmmod ssv6x5x
	#rmmod otg-hs
	#rmmod sdio_wifi
}

smartlink_install()
{
  #insmod /usr/modules/sdio_wifi.ko
	#sleep 1 #### 等待电源稳定后再加载其他驱动
	insmod /usr/modules/otg-hs.ko 
	sleep 2 #### 等待otg 向usb host 核心层完成一些注册工作后再加在驱动
	if test -e /usr/modules/ssv6x5x.ko ;then
		insmod /usr/modules/ssv6x5x.ko stacfgpath=/etc/jffs2/ak3916-wifi.cfg
	else
        insmod /usr/modules/ssv6x5x.ko stacfgpath=/etc/jffs2/ak3916-wifi.cfg
	fi
	sleep 2 #### 等待ssv drv核心层完成一些注册工作后再加在驱动	
}

ap_install()
{
	#install ap mode driver
	echo "install ssv6x5x ap driver"
}

ap_uninstall()
{
	#uninstall ap mode driver
	echo "uninstall ssv6x5x ap driver"
}

####### main

case "$MODE" in
	station)
		station_install
		;;
	smartlink)
		smartlink_install
		ifconfig wlan0 down
		iwconfig wlan0 mode monitor
		ifconfig wlan0 up
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


