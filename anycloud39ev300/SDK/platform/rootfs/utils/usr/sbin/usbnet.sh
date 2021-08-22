#! /bin/sh
### BEGIN INIT INFO
# File:				usbnet.sh
# Provides:         usbnet driver install and uninstall
# Required-Start:   $
# Required-Stop:
# Default-Start:     
# Default-Stop:
# Short-Description:install driver
# Author:			
# Email: 			
# Date:				2018-07-23
### END INIT INFO

PATH=$PATH:/bin:/sbin:/usr/bin:/usr/sbin
MODE=$1

usage()
{
	echo "Usage: $0 master | slave | uninstall"
}

master_uninstall()
{
	#rm usb net master mode driver
    rmmod asix
	rmmod usbnet
	rmmod otg-hs
}

master_install()
{
    #install usb net master mode driver
    insmod /usr/modules/otg-hs.ko
    sleep 1 	
	insmod /usr/modules/usbnet.ko
	sleep 1 
	insmod /usr/modules/asix.ko
	
	sleep 2
	ifconfig eth0 up
	sleep 1
	udhcpc -i eth0
}

slave_uninstall()
{
	#rm usb net slave mode driver
	rmmod g_ether
	rmmod udc
}

slave_install()
{
    #install usb net slave mode driver
    insmod /usr/modules/udc.ko
	sleep 2
	insmod /usr/modules/g_ether.ko 
	
	sleep 6
	ifconfig eth0 up
	sleep 1
	ifconfig eth0 192.168.1.88 ###set pc ip as 192.168.1.*
}


####### main

case "$MODE" in
	master)
		master_install
		;;
	slave)
		slave_install
		;;
	uninstall)
		master_uninstall
		slave_uninstall
		;;
	*)
		usage
		;;
esac
exit 0


