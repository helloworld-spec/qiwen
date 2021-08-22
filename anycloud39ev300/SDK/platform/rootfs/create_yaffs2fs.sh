#!/bin/sh

#	第一个参数指定要打包目录
#	第二个参数指定输出文件名
#	第三个参数指定打包方式
mkdir -p tmpfs
cp -af rootfs/* tmpfs
rm -rf tmpfs/usr/* tmpfs/etc/jffs2/*

./mkyaffs2image tmpfs/ rootfs.yaffs2 2048
./mkyaffs2image rootfs/usr usr.yaffs2 2048
./mkyaffs2image rootfs/etc/jffs2 jffs2.yaffs2 2048

rm -rf tmpfs
