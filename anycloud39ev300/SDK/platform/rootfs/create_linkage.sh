#!/bin/sh
#create passwd and shadow file link

ROOTFS=./rootfs
TARGET_DIR=$ROOTFS/etc

echo "create passwd and shadow file link"
if [ -d "$ROOTFS" ];then
	if [ -d  "$TARGET_DIR" ];then
		cd $TARGET_DIR
		if [ ! -L "passwd" ] && [ -e "jffs2/passwd" ];then
			ln -s jffs2/passwd passwd
			echo "create passwd link success"
		else
			echo "passwd link file exit, do nothing"
		fi

		if [ ! -L "shadow" ] && [ -e "jffs2/shadow" ];then
			ln -s jffs2/shadow shadow
			echo "create shadow link success"
		else
			echo "shadow link file exit, do nothing"
		fi
		
		cd ../../
	fi
fi
