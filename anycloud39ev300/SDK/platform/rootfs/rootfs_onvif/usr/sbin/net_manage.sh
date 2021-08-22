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
if [ ! -d "/sys/class/net/eth0" ]                                                                   #判断是否存在eth设备
then
	echo "[net manage] the ethernet module is not initialize"
	exit 1                                                                                      #不存在则退出
else
	ifconfig eth0 up
	cable_last=0                                                                                #初始化默认将上一次网络状态设置为未接入
	while true                                                                                  #进入循环判断
	do
		sleep 2
		status=`ifconfig eth0 | grep RUNNING`                                               #通过判断RUNNING是否存在,设置当前的网线插入状态
		if [ -n "$status" ]
		then
			cable_now=1                                                                 #网线接入则置为1
		else
			cable_now=0                                                                 #网线未接入则置为0
		fi
		#echo "cable_last=$cable_last cable_now=$cable_now"
		if [ $cable_last -lt $cable_now ]                                                   #之前网线状态为未接入,而当前网线状态为接入,则启动eth脚本设置网络参数
		then
			/usr/sbin/eth_manage.sh start                                               #启动以太网连接脚本
		fi
		cable_last=$cable_now                                                               #保存本次的网线接入状态
	done
fi

exit 0