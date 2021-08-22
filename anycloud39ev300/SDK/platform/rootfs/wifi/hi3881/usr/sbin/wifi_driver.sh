#! /bin/sh
### BEGIN INIT INFO
# File:				hi3881.sh(wifi_driver.sh)	
# Provides:         hi3881 driver install and uninstall
# Required-Start:   $
# Required-Stop:
# Default-Start:     
# Default-Stop:
# Short-Description:install driver
# Author:			
# Email: 			
# Date:				2020-11-25
### END INIT INFO

PATH=$PATH:/bin:/sbin:/usr/bin:/usr/sbin
MODE=$1

usage()
{
	echo "Usage: $0 station | smartlink | ap | uninstall"
}

station_uninstall()
{
	rmmod hi3881
	rmmod sdio_wifi
}

station_install()
{
    insmod /usr/modules/sdio_wifi.ko
    sleep 1 #### 等待电源稳定后再加载其他驱动	
	insmod /usr/modules/hi3881.ko
}

ap_install()
{
	#install ap mode driver
	echo "install hi3881 ap driver"
    station_install
}

ap_uninstall()
{
	#uninstall ap mode driver
	echo "uninstall hi3881 ap driver"
}

####### main

case "$MODE" in
	station)
		station_install
		;;
	smartlink)
        station_install
		ifconfig wlan0 up
		iwconfig wlan0 mode monitor
		;;	
	ap)
		ap_install
		;;
	uninstall)
		station_uninstall
		ap_uninstall
		;;
	*)
		usage
		;;
esac
exit 0


