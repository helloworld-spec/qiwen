CLOUD39EV300平台SDK使用Quick Start Guide
===

## 第一章 Quick Start Guide文档概述
该文档使用对象为产品的软件开发工程师，用于帮助工程师了解SDK相关内容，并快速上手进行开发。
    
## 第二章 首次安装使用SDK
1. SDK包格式及版本説明
SDK是一个压缩包，命名一般如：CLOUD39EV300_SDK_Vx.x.x.tgz，使用命令：tar -zxf CLOUD39EV300_SDK_Vx.x.x.tgz进行解压缩，可以得到一个CLOUD39EV300_SDK_Vx.x.x目录，即是SDK目录。

2. SDK目录介绍
```
CLOUD39EV300_SDK_Vx.x.x
├── doc
│   ├── Cloud39EV300内核配置及修改手册_V1.0.0.pdf
│   └── Cloud39EV300平台用户开发手册_V1.0.0.pdf
├── kernel
├── platform
│   ├── apps
│   │   ├── akipc                          # 主应用程序，负责音视频采集编码、网络发送和本地录像等功能。
│   │   ├── ccli                           # 动态获取和设置应用进程的运行状态
│   │   ├── cmd_serverd                    # 监听和接收其它进程的shell调用请求，并发起对shell命令的调用
│   │   ├── daemon                         # 后台守护程序，负责： 监测anyka_ipc的运行，对异常退出、内存泄露和内核死锁等情况做恢复处理。 T卡插拔检测和按键处理
│   │   ├── disk_repair
│   │   ├── Makefile
│   │   ├── mmc_test                       # TF卡测试程序
│   │   ├── product_test                   # 产测程序，主要完成图像、音频、wifi 和T卡等硬件的测试，以及完成 uid和mac 地址的烧写
│   │   ├── updater                        # 升级程序，可以升级boot、内核、文件系统镜像以及升级包。对于内核的升级，要区分 uImage 和zImage的混升情况，避免出错
│   │   └── version
│   ├── ChangeLog
│   ├── config.mk
│   ├── libapp                             # 应用层
│   ├── libmpi                             # 多媒体层
│   ├── libplat                            # 平台层
│   ├── Makefile
│   └── rootfs                             # 文件系统
├── Quick Start Guide.txt
└── tools
    ├── anyka_uclibc_gcc.tar.bz2           # 交叉编译工具
    ├── arm-anykav200-gdb-7.12.tar.gz      # gdb调试工具
    └── burntool                           # 烧录工具
```        

3. 在Linux服务器上安装并配置交叉编译环境
* 交叉编译工具位于SDK的tools目录，安凯提供的工具链的压缩包名称为anyka_uclibc_gcc.tar.bz2，
* 用root用户权限在根目录下执行下面命令解压
```sh
# tar -Pxvf  anyka_uclibc_gcc.tar.bz2 
```
解压后工具链会安装在目录/opt/arm-anykav200-crosstool/下。工具链使用的是绝对路径，必须放在这个目录下才可正常运行，不能修改目录及文件的名字。
* 设置主机PATH环境变量：用export PATH=$PATH:/opt/arm-anykav200-crosstool/usr/bin添加到系统的PATH变量，也可以添加到系统的启动脚本中，可以修改/etc/environment或/etc/bash.bashrc等文件。
* 工具链确认：安装好后执行arm-anykav200-linux-uclibcgnueabi-gcc  –v 系统能找到命令并显示gcc的版本信息说明工具安装正确。

## 第三章 内核及平台编译配置选项
1. 内核配置选项
在kernel目录下顺序执行：
```sh
	# mkdir ../build
	# make O=../build cloud39ev3_xx_defconfig
	# make O=../build -j4 uImage
```
生成uImage内核镜像文件，在build/arch/arm/boot 路径下。
*	如果直接在kernel目录下执行make clean会删除lib目录下的库文件，建议在kernel的同级目录下，新建一个build目录。
*	make O=../build -j4 uImage 这种方式编译时，是不会编译驱动modules的那些ko文件的，若需要有几种方法：
a、	可以先不加uImage 直接make O=../build -j4 编译出zImage ，同时会编译modules，再用make O=../build -j4 uImage命令编译出uImage。
b、	在make O=../build -j4 uImage命令后面再加上 modules指定目标，make O=../bd -j4 uImage modules。
c、	分开编译，用make O=../bd -j4 modules编译驱动modules，用make O=../build -j4 uImage编译内核。
		
2. 应用程序云平台配置
针对主应用程序anyka_ipc需要进行编译配置，配置文件的路径为platform/config.mk.因不同的云平台的网络配置方式不同，暂时未考虑多云平台的同时运行支持，仅可开启一种云平台支持。运择云平台配置后还要选择是否支持Wi-Fi配置。下面介绍配置项，设置y为开启，n主关闭：
```sh
	CONFIG_DANA_SUPPORT         = y           // 大拿编译配置项（默认）
	CONFIG_RTSP_SUPPORT         = n           // RTSP 功能
	CONFIG_ONVIF_SUPPORT        = n           // ONVIF平台
	CONFIG_ONVIF_AUDIO_SUPPORT  = n           // ONVIF平台是否支持音频输入
```
若platform/config.mk里的配置打开，应用的代码里，相应的宏定义也会被定义。比如platform/config.mk里CONFIG_DANA_SUPORT =y，那么应用代码里的宏CONFIG_DANA_SUPPORT也会被定义，在预处理#ifdef CONFIG_DANA_SUPPORT里面的代码将被编译。
注意：ONVIF配置项和RTSP及DANA配置项冲突，不能同时设为y，但RTSP配置项和DANA配置项不冲突，可以同时为y

3. 文件系统配置
文件系统的配置文件platform/rootfs/platform.cfg会自动根据platform/config.mk里的配置来编译相应目录的云平台的文件系统。在platform/config.mk里配置utils开关，默认不打开。若没有打开，rootfs/utils目录下的调试/测试工具是不会被拷贝到rootfs下的。
```sh
CONFIG_UTILS_SUPPORT = n         //utils配置开关
```

在platform目录下顺序执行：
```sh
	# make clean                      //清理旧的编译结果
	# make                            //编译所有目标，包括动态库
	# make install                    //生成rootfs根文件系统目录内容，包括拷贝已编译好的应用程序
	# make image                      //打包rootfs目录成镜像文件(root.sqsh4、usr.sqsh4、usr.jffs2)
```
最终在rootfs目录下生成root.sqsh4、usr.sqsh4和usr.jffs2三个文件系统镜像文件

## 第四章 系统运行配置
系统运行配置主要包括以下文件
1. anyka_cfg.ini配置文件为主应用程序提供各项配参数
2. danale.conf配置文件保存大拿平台的设备登录信息
3. isp_xx.conf等是ISP参数配置文件
注意：详细参数配置请参考《Cloud39EV300平台用户开发手册_V1.0.0》

## 第五章 安装及升级开发板
1. 烧写镜像文件
注意：以下内容简要介绍了开发板的烧录过程，详细内容请参考《Cloud39EV200平台开发板使用说明》
* 检查底板板上的JP4和JP10跳线是否已配置为USB烧录模式，即：USB_DP->AK_DP，USB_DM->AK_DM。
* 把编译生成的uImage、root.sqsh4、usr.jffs2、usr.sqsh4四个文件拷贝到烧录工具的目录下；
* 运行PDK配套的BurnTool烧录工具。
* 使用USB线连接开发的USB接口到PC USB端口。
* 按USB供电开关接通开发板电源
* 长按底板上的【BOOT】键，同时短按芯片核心板上的【RESET】键，待烧录工具进入烧录模式（对应通道状态变黄色）后再松开【BOOT】键。
* 在烧录工具界面，单击“开始”按钮，执行烧录，等待烧录完成。
* 烧录成功后，短按芯片板上的【RESET】键复位并开机。

2. 开发板镜像升级
注意：可以通过TF卡或者网络进行升级
* 将编译生成的uImage、root.sqsh4、usr.jffs2、usr.sqsh4文件(根据需要放对应的镜像文件即可)通过TF卡或者网络放到开发板上的/tmp目录
* 执行update.sh命令，等待升级完成重启即可。

## 第六章 常用开发环境配置
1. 如何使用NFS文件系统启动
* 在开发阶段，推荐使用NFS作为开发环境，可以省去重新制作和烧写根文件系统的工作。
* 挂载NFS文件系统的操作命令：
```sh
		# mount -t nfs -o nolock -o tcp xx.xx.xx.xx:/your-nfs-path /mnt
```
* 然后就可以在/mnt目录下访问服务器上的文件，并进行开发工作。
	
2. 如何运行停止主程序
注意：默认情况下主程序在/usr/bin目录，可执行文件名为anyka_ipc
停止主程序可以使用两种方法：
* 方法一：使用脚本停止服务，执行service.sh stop命令即可停止正在运行的程序（包括anyka_ipc及daemon等）
* 方法二：使用kill或killall命令直接杀掉进程（注意需要先kill掉daemon进程，然后再kill掉anyka_ipc进程）

启动主程序：
手工启动主程序主要是为了方便使用NFS文件系统进行调试，用户需要通过NFS文件系统将待调试程序挂载到开发板，然后切换到待调试程序所在的目录执行程序，例如：./anyka_ipc &

3. 开启telnet服务
 网络正常后，一般情况下telnet服务已经开启；如未开启，运行命令 telnetd& 就可以启动单板telnet服务，使用telnet即可登录到单板。
	
4. 开启log打印
执行tail -F /var/log/message&开启log打印。