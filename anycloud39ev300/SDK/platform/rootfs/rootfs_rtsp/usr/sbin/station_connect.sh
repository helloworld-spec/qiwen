#! /bin/sh
### BEGIN INIT INFO
# File:				station_connect.sh	
# Description:      wifi station connect to AP 
# Author:			gao_wangsheng
# Email: 			gao_wangsheng@anyka.oa
# Date:				2012-8-2
### END INIT INFO

MODE=$1
GSSID="$2"
SSID=\'\"$GSSID\"\'
GPSK="$3"
PSK=\'\"$GPSK\"\'
KEY=$PSK
KEY_INDEX=$4
KEY_INDEX=${KEY_INDEX:-0}
NET_ID=
PATH=$PATH:/bin:/sbin:/usr/bin:/usr/sbin
cfgfile=/etc/jffs2/anyka_cfg.ini
usage()
{
	echo "Usage: $0 mode(wpa|wep|open) ssid password"
	exit 3
}


check_ssid_valid()
{
	echo "scanning.."
	ap=""
	count=0
	while [ "$ap" == "" ] #-a "$count" -lt "5" ]
	do
		wpa_cli -iwlan0 scan
		ap=`wpa_cli -iwlan0 scan_results |grep $GSSID`
		wpa_cli -iwlan0 scan_results
#		count=`expr $count + 1`
		sleep 1
	done
	echo "Scan results: $ap"
}

check_ssid_exist()
{
	NET_ID=`wpa_cli -iwlan0 list_network\
		| awk 'NR>=2{print $1 "\t" $2}'\
		| grep "$GSSID" | awk '{print $1}'`
	echo $NET_ID
}


finish_station_connect()
{
	sh -c "wpa_cli -iwlan0 set_network $1 scan_ssid 1"
	wpa_cli -iwlan0 enable_network $1
	wpa_cli -iwlan0 select_network $1
#	wpa_cli -iwlan0 save_config
	
	supplicant=`pgrep wpa_supplicant`
	while [ "$supplicant" != "" ] 
	do
		stat=`wpa_cli -iwlan0 status |grep wpa_state`
		if [ "$stat" != "wpa_state=COMPLETED" ]
		then		
			check_ip_and_start
			exit 0
		fi
	
		sleep 1
		supplicant=`pgrep wpa_supplicant`
	done
}

connet_wpa()
{
	NET_ID=""
	check_ssid_exist
	if [ "$NET_ID" = "" ];then
	{
		NET_ID=`wpa_cli -iwlan0 add_network`
		sh -c "wpa_cli -iwlan0 set_network $NET_ID ssid $SSID"
		wpa_cli -iwlan0 set_network $NET_ID key_mgmt WPA-PSK
		sh -c "wpa_cli -iwlan0 set_network $NET_ID psk $PSK"
	}
	elif [ "$GPSK" != "" ];then
	{
		sh -c "wpa_cli -iwlan0 set_network $NET_ID psk $PSK"
	}
	fi

	finish_station_connect $NET_ID
}

connet_wep()
{
	NET_ID=""
	check_ssid_exist
	if [ "$NET_ID" = "" ];then
	{
		NET_ID=`wpa_cli -iwlan0 add_network`
		sh -c "wpa_cli -iwlan0 set_network $NET_ID ssid $SSID"
		wpa_cli -iwlan0 set_network $NET_ID key_mgmt NONE
		sh -c "wpa_cli -iwlan0 set_network $NET_ID wep_key${KEY_INDEX} $KEY"
	}
	elif [ "$GPSK" != "" ];then
	{
		sh -c "wpa_cli -iwlan0 set_network $NET_ID wep_key${KEY_INDEX} $KEY"
	}
	fi

	finish_station_connect $NET_ID
}

connet_open()
{
	NET_ID=""
	check_ssid_exist
	if [ "$NET_ID" = "" ];then
	{
		NET_ID=`wpa_cli -iwlan0 add_network`
		sh -c "wpa_cli -iwlan0 set_network $NET_ID ssid $SSID"
		wpa_cli -iwlan0 set_network $NET_ID key_mgmt NONE
	}
	fi

	finish_station_connect $NET_ID
}

check_ip_and_start ()
{
	status=
	while [ "$status" = "" ] 
	do
		echo "Getting ip address..."
		killall -9 udhcpc
		udhcpc -i wlan0
		status=`ifconfig wlan0 | grep "inet addr:"`
		sleep 1
	done

	if [ -d "/sys/class/net/eth0" ]
	then
	    ifconfig eth0 down
	    ifconfig eth0 up
	fi
	
	/usr/sbin/led.sh blink 2000 200
	echo "wifi connected!"
}

connect_adhoc()
{
	NET_ID=""
	check_ssid_exist
	if [ "$NET_ID" = "" ];then
	{
		wpa_cli ap_scan 2
		NET_ID=`wpa_cli -iwlan0 add_network`
		sh -c "wpa_cli -iwlan0 set_network $NET_ID ssid $SSID"
		wpa_cli -iwlan0 set_network $NET_ID mode 1
		wpa_cli -iwlan0 set_network $NET_ID key_mgmt NONE
	}
	fi

	finish_station_connect $NET_ID
}

check_ssid_ok()
{
	if [ "$GSSID" = "" ]
	then
		echo "Incorrect ssid!"
		usage
	fi
}

check_password_ok()
{
	if [ "$GPSK" = "" ]
	then
		echo "Incorrect password!"
		usage
	fi
}

#
# main:
#

echo $0 $*
case "$MODE" in
	wpa)
		check_ssid_ok
		check_password_ok
		connet_wpa 
		;;
	wep)
		check_ssid_ok
		check_password_ok
		connet_wep 
		;;
	open)
		check_ssid_ok
		connet_open 
		;;
	adhoc)
		check_ssid_ok
		connect_adhoc
		;;
	*)
		usage
		;;
esac
exit 0


