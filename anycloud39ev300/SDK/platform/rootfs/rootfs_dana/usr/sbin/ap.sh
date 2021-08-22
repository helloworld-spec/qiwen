#!/bin/sh

cfgfile="/etc/jffs2/anyka_cfg.ini"

wifi_ap_start()
{
	killall smartlink 2>/dev/null
	killall wpa_supplicant 2>/dev/null

	## check driver
	wifi_driver.sh uninstall
	wifi_driver.sh station

	echo "start wlan0 on ap mode"
	ifconfig wlan0 up

	ssid=`awk 'BEGIN {FS="="}/\[softap\]/{a=1} a==1 && 
	$1~/s_ssid/{gsub(/\"/,"",$2);gsub(/\;.*/, "", $2);
	gsub(/^[[:blank:]]*/,"",$2);print $2}' $cfgfile`
	password=`awk 'BEGIN {FS="="}/\[softap\]/{a=1} a==1 && 
	$1~/s_password/{gsub(/\"/,"",$2);gsub(/\;.*/, "", $2);
	gsub(/^[[:blank:]]*/,"",$2);print $2}' $cfgfile`

	echo "ap :: ssid=$ssid password=$password"
	if [ -z "$ssid" ];then
		ssid="123456"
	fi

	/usr/sbin/device_save.sh name "$ssid"
	/usr/sbin/device_save.sh password "$password"

	if [ -z $password ];then
		/usr/sbin/device_save.sh setwpa 0
	else
		/usr/sbin/device_save.sh setwpa 2
	fi

	hostapd /etc/jffs2/hostapd.conf -B
	ifconfig wlan0 192.168.0.1
	route del default 2>/dev/null
	route add default gw 192.168.0.1 wlan0
	udhcpd /etc/udhcpd.conf
}

wifi_ap_stop()
{
	killall hostapd 2>/dev/null
	killall udhcpd 2>/dev/null
	ifconfig wlan0 down
	wifi_driver.sh uninstall
	route del default 2>/dev/null
}

usage()
{
	echo "$0 start | stop"
}


case $1 in
	start)
		wifi_ap_start
		;;
	stop)
		wifi_ap_stop
		;;
	*)
		usage
		;;
esac
	


