#! /bin/sh
### BEGIN INIT INFO
# File:				service.sh
# Provides:         init service
# Required-Start:   $
# Required-Stop:
# Default-Start:
# Default-Stop:
# Short-Description:web service
# Author:			gao_wangsheng
# Email: 			gao_wangsheng@anyka.oa
# Date:				2012-12-27
### END INIT INFO

MODE=$1
TEST_MODE=0
PATH=$PATH:/bin:/sbin:/usr/bin:/usr/sbin

usage()
{
	echo "Usage: $0 start|stop)"
	exit 3
}

stop_service()
{
	killall -12 daemon
	echo "watch dog closed"
	sleep 5
	killall daemon
	killall cmd_serverd

	/usr/sbin/anyka_ipc.sh stop

	echo "stop network service......"
	killall net_manage.sh

    /usr/sbin/eth_manage.sh stop
    /usr/sbin/wifi_manage.sh stop
}

start_service ()
{
	cmd_serverd
	if [ $TEST_MODE = 0 ]; then
		daemon 
		/usr/sbin/anyka_ipc.sh start 
		echo "start net service......"
	else
		product_test & 
		echo "start product test."
	fi

	boot_from=`cat /proc/cmdline | grep nfsroot`
	if [ -z "$boot_from" ];then
		echo "start net service......"
		/usr/sbin/net_manage.sh &
	else
		echo "## start from nfsroot, do not change ipaddress!"
	fi
	unset boot_from
}

restart_service ()
{
	echo "restart service......"
	stop_service
	start_service
}

#
# main:
#
if test -e /etc/jffs2/danale.conf ;then
	TEST_MODE=0
else
	TEST_MODE=1
fi


case "$MODE" in
	start)
		start_service
		;;
	stop)
		stop_service
		;;
	restart)
		restart_service
		;;
	*)
		usage
		;;
esac
exit 0

