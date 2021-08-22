#!/bin/sh
# auto compile for cloud39ev300 PDK V1
# usage: ./auto_build.sh boardname platform wifiname tarname
# notes:
#	1. without input param, default is ak3916e_core_bd

version=V1.0.0
cur_dir=`pwd`
build_dir=${cur_dir}
component_dir=${cur_dir}/component
platform_dir=${cur_dir}/platform
tools_dir=
kernel_dir=${cur_dir}/kernel
kernel_out=
bd_config_file=
target_bd=
arg_num=$#

ak3918e_38module="ak3918e_38module"
ak3916e_corebd="ak3916e_corebd"
ak3918e_corebd="ak3918e_corebd"

rtsp_support="rtsp"
dana_support="dana"

rtl8188="rtl8188"
rtl8188_ftv="rtl8188_ftv"
rtl8189_etv="rtl8189_etv"
rtl8189_ftv="rtl8189_ftv"
hi3881="hi3881"
atbm603x="atbm603x"

platform_config=""
target_package_name=""
wifi_config=""
usage_show_once=0
release_version=n

usage()
{
	if [ $usage_show_once -eq 1 ];then
		return
	fi

	echo "Usage: $0 boardname platform wifi tarname"
	echo "Support boardname:
            ak3918e_38module - ak3918ev300 38 module kernel config
            ak3916e_corebd   - ak3916ev300 core board kernel config
            ak3918e_corebd   - ak3918ev300 core board kernel config
	"
	echo "Support platform:
            dana  -	dana cloud platform
            rtsp  - rtsp platform
	"
	echo "Support wifi:
		rtl8188     - realtek 8188-eus usb-wifi
		rtl8188_ftv - realtek 8188-ftv usb-wifi
		rtl8189_etv - realtek 8189-etv sdio-wifi
		rtl8189_ftv - realtek 8189-ftv sdio-wifi
		hi3881		- hi 3881 sdio-wifi
		atbm603x	- atbm6032i usb-wifi
		nowifi		- no need wifi
	"
	echo "Example: $0 ak3918e_38module dana rtl8188 pdk_v1.0_dana_rtl8188"
	usage_show_once=1
}

release_dirs()
{
	read -p "please input release version dir: " version
	echo "you will create dir: ${version}"
	mkdir -p ${version}
}

set_kernel_defconfig()
{
	case $1 in
		${ak3918e_38module})
			bd_config_file=cloud39ev3_co38_sc2235_v1.0.0_defconfig
			;;
		${ak3916e_corebd})
			bd_config_file=cloud39ev3_ak3916ev300_corebd_v1.0.0_defconfig
			;;
		${ak3918e_corebd})
			bd_config_file=cloud39ev3_ak3918ev300_corebd_v1.0.0_defconfig
			;;
		*)
			echo "\033[1;31m""config board fail, will not compile kernel""\033[m"
			usage
			return
			;;
	esac
	kernel_out=${cur_dir}/$1
	target_bd=$1

	echo ""
	echo " kernel compile out dir: ${kernel_out}"
	echo "\033[1;32m"" use kernel defconfig: ${bd_config_file}""\033[m"
	echo " target build directory: ${target_bd}"
	echo ""
}

set_platform_config()
{
	case "$1" in
		${dana_support})
			platform_config="CONFIG_DANA_SUPPORT=y CONFIG_RTSP_SUPPORT=n"
			;;
		${rtsp_support})
			platform_config="CONFIG_RTSP_SUPPORT=y CONFIG_DANA_SUPPORT=n"
			;;
		*)
			platform_config=""
			echo "\033[1;31m"" config platform fail, will not compile it""\033[m"
			usage
			return
			;;
	esac
	echo "\033[1;35m"" platform is $1, config: ${platform_config}""\033[m"
	echo ""
}

set_wifi_config()
{
	case "$1" in
		${rtl8188})
			wifi_config="CONFIG_WIFI_RTL8188EUS=y CONFIG_WIFI_RTL8188FTV=n CONFIG_WIFI_RTL8189ETV=n CONFIG_WIFI_RTL8189FTV=n CONFIG_WIFI_SSV6X5X=n CONFIG_WIFI_HI3881=n CONFIG_WIFI_ATBM603X=n"
			;;
		${rtl8188_ftv})
			wifi_config="CONFIG_WIFI_RTL8188EUS=n CONFIG_WIFI_RTL8188FTV=y CONFIG_WIFI_RTL8189ETV=n CONFIG_WIFI_RTL8189FTV=n CONFIG_WIFI_SSV6X5X=n CONFIG_WIFI_HI3881=n CONFIG_WIFI_ATBM603X=n"
			;;
		${rtl8189_etv})
			wifi_config="CONFIG_WIFI_RTL8188EUS=n CONFIG_WIFI_RTL8188FTV=n CONFIG_WIFI_RTL8189ETV=y CONFIG_WIFI_RTL8189FTV=n CONFIG_WIFI_SSV6X5X=n CONFIG_WIFI_HI3881=n CONFIG_WIFI_ATBM603X=n"
			;;
		${rtl8189_ftv})
			wifi_config="CONFIG_WIFI_RTL8188EUS=n CONFIG_WIFI_RTL8188FTV=n CONFIG_WIFI_RTL8189ETV=n CONFIG_WIFI_RTL8189FTV=y CONFIG_WIFI_SSV6X5X=n CONFIG_WIFI_HI3881=n CONFIG_WIFI_ATBM603X=n"
			;;
		${sv6155})
			wifi_config="CONFIG_WIFI_RTL8188EUS=n CONFIG_WIFI_RTL8188FTV=n CONFIG_WIFI_RTL8189ETV=n CONFIG_WIFI_RTL8189FTV=n CONFIG_WIFI_SSV6X5X=y CONFIG_WIFI_HI3881=n CONFIG_WIFI_ATBM603X=n"
			;;
		${hi3881})
			wifi_config="CONFIG_WIFI_RTL8188EUS=n CONFIG_WIFI_RTL8188FTV=n CONFIG_WIFI_RTL8189ETV=n CONFIG_WIFI_RTL8189FTV=n CONFIG_WIFI_SSV6X5X=n CONFIG_WIFI_HI3881=y CONFIG_WIFI_ATBM603X=n"
			;;
		${atbm603x})
			wifi_config="CONFIG_WIFI_RTL8188EUS=n CONFIG_WIFI_RTL8188FTV=n CONFIG_WIFI_RTL8189ETV=n CONFIG_WIFI_RTL8189FTV=n CONFIG_WIFI_SSV6X5X=n CONFIG_WIFI_HI3881=n CONFIG_WIFI_ATBM603X=y"
			;;
		*)
			wifi_config="CONFIG_WIFI_RTL8188EUS=n CONFIG_WIFI_RTL8188FTV=n CONFIG_WIFI_RTL8189ETV=n CONFIG_WIFI_RTL8189FTV=n CONFIG_WIFI_SSV6X5X=n CONFIG_WIFI_HI3881=n CONFIG_WIFI_ATBM603X=n"
			echo "\033[1;31m"" compile will exclude wifi""\033[m"
			return
			;;
	esac
	echo "\033[1;36m"" wifi config: $1, config name: ${wifi_config}""\033[m"
	echo ""

}

set_tar_package_name()
{
	target_package_name=$1
	echo "\033[1;34m"" target package name: $target_package_name""\033[m"
}

check_compile_error()
{
	error=`grep error tmp_log`
	error1=`grep "´íÎó" tmp_log`
	warnning=`grep warnning tmp_log`

	if [ -n "${error}" ] || [ -n "${error1}" ];then
		echo "\033[1;33m""compile platform capture some error, stopped""\033[m"
		exit 1
	fi

	rm tmp_log
}

platform_make_and_install()
{
	echo "plat release: $release_version"
	make PLAT_RELEASE=${release_version} $1 $2 $3 $4 $5> /dev/null 2>tmp_log -j4
	check_compile_error
	echo "	--- platform compile ok ---"

	make install PLAT_RELEASE=${release_version} $1 $2 $3 $4 $5 $6 $7 $8 $9 ${10} ${11}> /dev/null
	make image > /dev/null
}

rootfs_to_burntool()
{
	cp -rf rootfs/*.sqsh4 ${tools_dir}
	cp -rf rootfs/usr.jffs2 ${tools_dir}
}

make_update_image()
{
	if [ -n "$target_package_name" ];then
		cd ${tools_dir}
		echo "making update target package"
		tar cf "$target_package_name".tar uImage usr.sqsh4 \
			root.sqsh4 usr.jffs2
		if [ $? -eq 0 ];then
			echo "\033[1;33m""make tar ok""\033[m"
			mv $target_package_name.tar ${cur_dir}
		else
			echo "\033[1;31m""make tar error""\033[m"
		fi
	fi
}

make_component()
{
	if [ ! -d ${component_dir} ];then
		release_version=y
		tools_dir=${build_dir}/burntool/
		return
	else
		tools_dir=${build_dir}/tools/burntool/
	fi

	echo "---------- 1. enter component ----------"
	release_version=n
	cd ${component_dir}

	make clean > /dev/null
	make > /dev/null
	if [ $? -ne 0 ];then
		false;
		return;
	fi
	make install > /dev/null
	make clean > /dev/null

	echo "---------- leave componet ----------"
	echo ""
}

make_kernel()
{
	if [ -z "${bd_config_file}" ];then
		return
	fi

	echo "---------- 2. enter kernel ----------"
	cd ${kernel_dir}

	if [ ! -d "${kernel_out}" ];then
		echo "${kernel_out} dir not exist"
		echo "we create it..."
		echo ""
		mkdir -p ${kernel_out}
		make O=${kernel_out} ${bd_config_file}
	else
		echo "${kernel_out} dir exist and has not been configed"
		echo "we compile directly..."
		echo ""
	fi

	echo "	start kernel compiling..."
	make O=${kernel_out} uImage modules > /dev/null 2>tmp_log -j4
	check_compile_error
	echo "	--- kernel compile ok ---"

	cp -rf ${kernel_out}/drivers/media/video/plat-anyka/*.ko ${platform_dir}/rootfs/ko/
	cp -rf ${kernel_out}/drivers/misc/*.ko ${platform_dir}/rootfs/ko/
	cp -rf ${kernel_out}/drivers/usb/gadget/*.ko ${platform_dir}/rootfs/ko/
	cp -rf ${kernel_out}/drivers/usb/gadget/plat-anyka/*.ko ${platform_dir}/rootfs/ko/
	cp -rf ${kernel_out}/drivers/usb/host/plat-anyka/*.ko ${platform_dir}/rootfs/ko/

	cp -rf ${kernel_out}/arch/arm/boot/uImage ${tools_dir}

	echo "---------- leave kernel ----------"
	cd ${build_dir}
	echo ""
}

platform_compile()
{
	platform_make_and_install $platform_config $wifi_config
	rootfs_to_burntool
}

make_platform()
{
	if [ -z "${platform_config}" ];then
		return
	fi

	echo "---------- 4. enter platform ----------"
	cd ${platform_dir}
	make clean PLAT_RELEASE=${release_version} > /dev/null -j4

	echo "---------- compiling ----------"
	platform_compile
	cd ${build_dir}

	echo "---------- leave platform ----------"
	echo ""
}

image_for_update()
{
	echo "---------- 5. enter burntool ----------"
	make_update_image
	echo "---------- leave burntool ----------"
}

#0. set kernel compile defconfig
case ${arg_num} in
	1)
		set_kernel_defconfig $1
		;;
	2)
		set_kernel_defconfig $1
		set_platform_config $2
		;;
	3)
		set_kernel_defconfig $1
		set_platform_config $2
		set_wifi_config $3
		;;
	4)
		set_kernel_defconfig $1
		set_platform_config $2
		set_wifi_config $3
		set_tar_package_name $4
		;;
	*)
		usage
		;;
esac

#1. copy the latest component to kernel and platform
make_component

#2. enter kernel directory, make ko and uImage, then copy to platform & burntool
make_kernel

#3. enter platform directory, do make, copy image to burntool
make_platform

#4. all kind of image for updating
image_for_update

echo "---------- auto compile finished ----------"
date
