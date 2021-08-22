#!/bin/sh

#	第一个参数指定要打包目录
#	第二个参数指定输出文件名
#	第三个参数指定打包方式
mkdir -p tmpfs
cp -af rootfs/* tmpfs
rm -rf tmpfs/usr/* tmpfs/etc/jffs2/*

./mksquashfs tmpfs/ root.sqsh4 -noappend -comp xz
./mksquashfs rootfs/usr usr.sqsh4 -noappend -comp xz

rm -rf tmpfs
