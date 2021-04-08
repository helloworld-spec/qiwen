### 磁盘分区
### json

### udhcpc udhcpd

```
基于DHCP(动态主机配置协议)的C-S模型服务
udhcpc: DHCP客户端，
udhcpd: DHCP服务端

```

### access()

```c
#include <unistd.h>
int access(const char *pathname, int mode);
mode指定access的作用，取值如下
F_OK，值为0，判断文件是否存在
X_OK，值为1，判断对文件是否可执行权限
W_OK, 值为2，判断对文件是否有写权限
R_OK, 值为4，判断对文件是否有读权限
后面三种可以用'|'，一起使用
```

###  find 文件查找命令
    按文件名查找文件
    用法：find 目录 -name "[文件名]"

###  grep 查找字符段命令
    目录中查找指定字符串
    -i 忽略大小写
    用法: 
    	1、grep -r "[字符串]" 目录
    	2、cat .config | grep make

###  cd - 回到上一次的目录
    回到上一次的目录

###  readelf	查看静态库版本信息
    查看静态库版本信息命令
    readelf ./libtuya_ipc.a -p .comment

### | 管道符号
    两个 linux 命令中间加一个 | .
    把前面一个原本要输出到屏幕的数据当作是后一个命令的标准输入
    如：history | grep date 指从 history 这条命令运行的结果中显示包含 “date” 

###  cut 切割命令
    分段取内容
    如 
    cat /proc/net/rtl8188fu/wlan0/rx_signal | grep rssi | cut -d ":" -f 2
    把目录的文件
    原本要把文件的内容输出到屏幕，管道命令使得输出结果作为 grep 命令的标准
    输入，grep 命令查找包含 'rssi' 的内容，然后 cut 命令分段截取：过程如下
    结果按 : 分段，提取第二段

###     which 搜索命令所在路径及别名

```
[root@localhost ~]: which pwd
[root@localhost ~]: /usr/bin/pwd
```

###  whereis  搜索命令所在的路径以及帮助文档所在的位置

```
[root@localhost ~]: whereis ls
ls: /bin/ls /usr/share/man/man1/ls.1.gz /usr/share/man/man1/ls.1posix.gz
```

###   mount 挂载命令

```
将/dev/hda1挂在/mnt下
mount /dev/hda1 /mnt
```

###   dmesg 显示计算机启动时候的信息

```
在计算机启动时候会打印很多信息，我们可以从中获取很多系统信息，
dmesg | grep cpu  //查看关于cpu字样的信息
```

### netstat 监控TCP/IP网络工具

```
可以显示路由表、实际的网络连接以及每一个网络接口设备的状态信息
-a 显示所有连线中的socket
-n 显示直接使用ip地址，而不通过域名服务器
-t 显示TCP传输协议的连线状态
-u 显示UDP传输协议的连线状态
-p 显示正在使用Socket的程序识别码和程序名称
[root@anyka ~]$ netstat -antp
Active Internet connections (servers and established)
Proto  Recv-Q  Send-Q Local    Address   Foreign Address  State   PID/Program name    
tcp        0      0 0.0.0.0:6789     0.0.0.0:*    LISTEN      471/anyka_ipc
tcp        0      0 0.0.0.0:6790     0.0.0.0:*    LISTEN      471/anyka_ipc
tcp        0      0 0.0.0.0:6791     0.0.0.0:*    LISTEN      471/anyka_ipc
tcp        0      0 0.0.0.0:554      0.0.0.0:*    LISTEN      471/anyka_ipc
tcp        0      0 127.0.0.1:8782   0.0.0.0:*    LISTEN      464/cmd_serverd
tcp        0      0 0.0.0.0:8090     0.0.0.0:*    LISTEN      471/anyka_ipc
```

### telnet 

```
Telnet是进行远程登陆的标准协议和主要方式，它为用户提供了本地计算机上完成远程主机工作的能力。
可以用telnet命令来测试端口号是否正常还是关闭状态
telnet [ip] [端口]
```

### 可重入函数

```
可重入函数主要用于多任务环境中，一个可重入的函数简单来说就是可以被中断的函数，也就是说，可以在这个函数执行的任何时刻中断它，转入系统调度下去执行另外一段代码，而返回控制时不会出现什么错误；而不可重入的函数由于使用了一些系统资源，比如全局变量区，中断向量表，所以它被中断的话，可能会出现问题，这类函数是不能运行在多任务环境下的
```

### free

```
显示系统内存的使用情况，包括物理内存、交换内存(swap)和内核缓冲区内存
[root@anyka /var/log]$ free
              total        used        free      shared  buff/cache   available
Mem:          59752        7608       49064          24        3080       47872
Swap:             0           0           0
Mem:内存使用情况
Swap：交换空间使用情况
total：显示内存总的可用物理内存和交换空间大小
used：显示已经被使用的物理内存和交换空间
free:显示还有多少物理内存和交换空间可使用
shared：显示被共享使用的物理内存大小
buff/cache：显示被buffer和cache使用的物理内存大小
available:显示还可以被应用程序使用的物理内存大小
```

