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
CONF_FILE=/etc/jffs2/wpa_supplicant.conf
DEV_STATION=wlan0
usage()
{
	echo "Usage: $0 mode(wpa|wep|open) ssid password"
	exit 3
}

#refresh_net()
#{
#	#### remove all connected netword
#	while true
#	do
#		NET_ID=`wpa_cli -iwlan0 list_network\
#			| awk 'NR>=2{print $1}'`
#
#		if [ -n "$NET_ID" ];then
#			wpa_cli -p/var/run/wpa_supplicant remove_network $NET_ID
#		else
#			break
#		fi
#	done
#	wpa_cli -p/var/run/wpa_supplicant ap_scan 1
#}

#station_connect()
#{
#	sh -c "wpa_cli -iwlan0 set_network $1 scan_ssid 1"
#
#	wpa_cli -iwlan0 enable_network $1
#	wpa_cli -iwlan0 select_network $1
#	wpa_cli -iwlan0 save_config
#}

connet_wpa()
{
	#NET_ID=""
	#refresh_net

	#NET_ID=`#wpa_cli -iwlan0 add_network`
	#sh -c "#wpa_cli -iwlan0 set_network $NET_ID ssid $SSID"
	#wpa_cli -iwlan0 set_network $NET_ID key_mgmt WPA-PSK
	#sh -c "#wpa_cli -iwlan0 set_network $NET_ID psk $PSK"

	#station_connect $NET_ID
	sed -i "s/^\\s*ssid=.*/        ssid=\"$GSSID\"/" $CONF_FILE
	sed -i "s/^\\s*#*\\s*key_mgmt=.*/        key_mgmt=WPA-PSK WPA-EAP IEEE8021X NONE/" $CONF_FILE
	sed -i "s/^\\s*#*\\s*pairwise=.*/        pairwise=TKIP CCMP/" $CONF_FILE
	sed -i "s/^\\s*#*\\s*group=.*/        group=CCMP TKIP WEP104 WEP40/" $CONF_FILE
	sed -i "s/^\\s*#*\\s*psk=.*/        psk=\"$GPSK\"/" $CONF_FILE
}

connet_wep()
{
	NET_ID=""
	#refresh_net
	if [ "$NET_ID" = "" ];then
	{
		#NET_ID=`wpa_cli -iwlan0 add_network`
		#sh -c "wpa_cli -iwlan0 set_network $NET_ID ssid $SSID"
		#wpa_cli -iwlan0 set_network $NET_ID key_mgmt NONE
		keylen=$echo${#KEY}

		if [ $keylen != "9" ] && [ $keylen != "17" ];then
		{
			wepkey1=${KEY#*'"'}
			wepkey2=${wepkey1%'"'*};
			KEY=$wepkey2;
			echo $KEY
		}
		fi
		#sh -c "wpa_cli -iwlan0 set_network $NET_ID wep_key${KEY_INDEX} $KEY"
	}
	elif [ "$GPSK" != "" ];then
	{
		keylen=$echo${#KEY}
		if [ $keylen != "9" ] && [ $keylen != "17" ];then
		{
			wepkey1=${KEY#*'"'}
			wepkey2=${wepkey1%'"'*};
			KEY=$wepkey2;
			echo $KEY
		}
		fi
		#sh -c "wpa_cli -iwlan0 set_network $NET_ID wep_key${KEY_INDEX} $KEY"
	}
	fi

	#station_connect $NET_ID
}

connet_open()
{
	#NET_ID=""
	#refresh_net

	#NET_ID=`#wpa_cli -iwlan0 add_network`
	#sh -c "#wpa_cli -iwlan0 set_network $NET_ID ssid $SSID"
	#wpa_cli -iwlan0 set_network $NET_ID key_mgmt NONE

	#station_connect $NET_ID
	sed -i "s/^\\s*ssid=.*/        ssid=\"$GSSID\"/" $CONF_FILE
	sed -i "s/^\\s*#*\\s*key_mgmt=.*/        key_mgmt=NONE/" $CONF_FILE
	sed -i "s/^\\s*#*\\s*pairwise=.*/        #pairwise=TKIP CCMP/" $CONF_FILE
	sed -i "s/^\\s*#*\\s*group=.*/        #group=CCMP TKIP WEP104 WEP40/" $CONF_FILE
	sed -i "s/^\\s*#*\\s*psk=.*/        #psk=\"$GPSK\"/" $CONF_FILE
}

connect_adhoc()
{
	NET_ID=""
#	refresh_net
#	if [ "$NET_ID" = "" ];then
#	{
#		wpa_cli ap_scan 2
#		NET_ID=`wpa_cli -iwlan0 add_network`
#		sh -c "wpa_cli -iwlan0 set_network $NET_ID ssid $SSID"
#		wpa_cli -iwlan0 set_network $NET_ID mode 1
#		wpa_cli -iwlan0 set_network $NET_ID key_mgmt NONE
#	}
#	fi

#	station_connect $NET_ID
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

echo -e "\33[32m$0 $@\33[0m"
#echo $0 $*
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
