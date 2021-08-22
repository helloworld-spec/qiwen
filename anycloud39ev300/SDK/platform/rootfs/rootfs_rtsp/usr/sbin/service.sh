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
	echo "stop network service......"
	killall net_manage.sh
    /usr/sbin/eth_manage.sh stop

	killall anyka_ipc
}

start_service ()
{
	# run daemon process
	daemon
	cmd_serverd

	boot_from=`cat /proc/cmdline | grep nfsroot`
	if [ -z "$boot_from" ];then
		# run net scripts
		echo "start net service......"
		/usr/sbin/net_manage.sh
	else
		echo "## start from nfsroot, do not change ipaddress!"
	fi
	unset boot_from

	# run main service process
	/usr/sbin/anyka_ipc.sh start
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

