#! /bin/sh
### BEGIN INIT INFO
# File:				net_manage.sh
# Provides:         start ethernet
# Required-Start:   $
# Required-Stop:
# Default-Start:     
# Default-Stop:
# Short-Description 
# Author:			
# Email: 			
# Date:				2013-1-15
### END INIT INFO


#
#main
#

#Do load ethernet module?
if [ ! -d "/sys/class/net/eth0" ]
then
	echo "[net manage] the ethernet module is not initialize"
	exit 1
else
	#ethernet always up
	ifconfig eth0 up
	sleep 3
	while true
	do
	#check whether insert internet cable

	status=`ifconfig eth0 | grep RUNNING`
	if [ -z "$status" ]
	then
		#had inserted internet cable
		sleep 1
	else
		break
	fi
	
	done
	/usr/sbin/eth_manage.sh start  ##for rtsp
fi

exit 0

