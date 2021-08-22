#!/bin/sh
# File:				update.sh	
# Provides:         
# Description:      update zImage&rootfs under dir1/dir2/...
# Author:			xc

# **Update Log**
# Description：		support spi nand flash update 
# Time:				2019-10-24
# Author:			li_cheng

style=`df -T -h | grep -i "dev/root" | awk '{print $2}'`

norflash="squashfs"
nandflash="yaffs2"

if [ ${style} = ${norflash} ];then
	VAR1="uImage"
	VAR2="root.sqsh4"
	VAR3="usr.sqsh4"
	VAR4="usr.jffs2"

	ZMD5="uImage.md5"
	SMD5="usr.sqsh4.md5"
	JMD5="usr.jffs2.md5"
	RMD5="root.sqsh4.md5"
	echo "system file style is squashfs, flash is spi nor flash"
elif [ ${style} = ${nandflash} ];then
	VAR1="zImage"
 	VAR2="root.yaffs2"
 	VAR3="usr.yaffs2"
 	VAR4="jffs2.yaffs2"
	
 	ZMD5="zImage.md5"
 	SMD5="usr.yaffs2.md5"
	JMD5="jffs2.yaffs2.md5"
 	RMD5="root.yaffs2.md5"
	echo "system file style is yaffs2, flash is spi nand flash"
fi

DIR1="/tmp"
DIR2="/mnt"
UPDATE_DIR_TMP=0

update_voice_tip()
{
	echo "play update voice tips"
	ccli misc --tips "/usr/share/anyka_update_device.mp3"
	sleep 3
}

update_check_image()
{
	echo "check update image .........................."

	for target in ${VAR1} ${VAR2} ${VAR3} ${VAR4}
	do
		if [ -e ${DIR1}/${target} ]; then
			echo "find a target ${target}, update in /tmp"
			UPDATE_DIR_TMP=1
			break
		fi	
	done
}

#
# main:
#
echo "stop system service before update....."
killall -15 syslogd
killall -15 klogd
killall -15 tcpsvd

# play update vioce tip
update_voice_tip

# send signal to stop watchdog
killall -12 daemon
sleep 5
# kill apps, MUST use force kill
killall -9 daemon
killall -9 anyka_ipc
killall -9 net_manage.sh
/usr/sbin/wifi_manage.sh stop
killall -9 smartlink

# sleep to wait the program exit
i=5
while [ $i -gt 0 ]
do
	sleep 1

	pid=`pgrep anyka_ipc`
	if [ -z "$pid" ];then
		echo "The main app anyka_ipc has exited !!!"
		break
	fi

	i=`expr $i - 1`
done

if [ $i -eq 0 ];then
	echo "The main app anyka_ipc is still run, we don't do update, reboot now !!!"
	reboot
fi

echo "############ please wait a moment. And don't remove TFcard or power-off #############"

#led blink
/usr/sbin/led.sh blink 50 50

update_check_image

if [ ${style} = ${norflash} ];then 
    if [ $UPDATE_DIR_TMP -ne 1 ];then
        ## copy the image file to /tmp to avoid update fail on TF-card
        for dir in ${VAR1} ${VAR2} ${VAR3} ${VAR4}
        do
            cp ${DIR2}/${dir} /tmp/ 2>/dev/null
            cp ${DIR2}/${dir}.md5 /tmp/ 2>/dev/null
        done
        umount /mnt/ -l
        echo "update use image from /mnt"
    else
        echo "update use image from /tmp"
    fi
    cd ${DIR1}
elif [ ${style} = ${nandflash} ];then 
    echo "update use image from /mnt"
    cd ${DIR2}
fi

#run in ram, start new shell and updater_image.sh
sh /sbin/update_image.sh &



