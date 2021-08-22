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
		/usr/sbin/net_manage.sh &
	else
		echo "## start from nfsroot, do not change ipaddress!"
	fi
	unset boot_from

	while true                                                                                  #进入循环判断
	do
		sleep 2
		status=`ifconfig eth0 | grep RUNNING`                                               #通过判断RUNNING是否存在,设置当前的网线插入状态
		inet=`ifconfig eth0 | grep "inet addr:"`                                           #判断ip地址是否存在
		if [ -n "$status" ] && [ -n "$inet" ]
		then
			break                                                                       #网络初始化好,则继续主程序的启动
		else
			echo "Wait for the newwork init."                               #网络还没有初始化好
		fi
	done
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

